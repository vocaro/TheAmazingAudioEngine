//
//  AEAudioMixerBuffer.h
//  The Amazing Audio Engine
//
//  Created by Michael Tyson on 12/04/2012.
//  Copyright (c) 2012 A Tasty Pixel. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

/*!
 * A source identifier, for use with [AEAudioMixerBufferEnqueue](@ref AEAudioMixerBuffer::AEAudioMixerBufferEnqueue).
 *
 * This can be anything you like, as long as it is not NULL, and is unique to each source.
 */
typedef void* AEAudioMixerBufferSource;

/*!
 * Source render callback
 *
 *      This is called by AEAudioMixerBuffer when audio for the source is required, if you have provided callbacks
 *      for the source via [AEAudioMixerBufferSetSourceCallbacks](@ref AEAudioMixerBuffer::AEAudioMixerBufferSetSourceCallbacks).
 *
 * @param source            The source. This can be anything you like, as long as it is not NULL, and is unique to each source.
 * @param frames            The number of frames required.
 * @param audio             The audio buffer list - audio should be copied into the provided buffers. May be NULL, in which case your render callback should simply discard the requested audio.
 * @param userInfo          The opaque pointer passed to [AEAudioMixerBufferSetSourceCallbacks](@ref AEAudioMixerBuffer::AEAudioMixerBufferSetSourceCallbacks).
 */
typedef void (*AEAudioMixerBufferSourceRenderCallback) (AEAudioMixerBufferSource  source,
                                                        UInt32                    frames,
                                                        AudioBufferList          *audio,
                                                        void                     *userInfo);

/*!
 * Source peek callback
 *
 *      This is called by AEAudioMixerBuffer when it needs to know the status of the source, if you have
 *      provided callbacks for the source via
 *      [AEAudioMixerBufferSetSourceCallbacks](@ref AEAudioMixerBuffer::AEAudioMixerBufferSetSourceCallbacks).
 *
 * @param source            The source. This can be anything you like, as long as it is not NULL, and is unique to each source.
 * @param outTimestamp      On output, the timestamp of the next audio from the source.
 * @param userInfo          The opaque pointer passed to [AEAudioMixerBufferSetSourceCallbacks](@ref AEAudioMixerBuffer::AEAudioMixerBufferSetSourceCallbacks).
 * @return The number of available frames
 */
typedef UInt32 (*AEAudioMixerBufferSourcePeekCallback) (AEAudioMixerBufferSource  source,
                                                        uint64_t                 *outTimestamp,
                                                        void                     *userInfo);


/*!
 * Mixer buffer
 *
 *  This class performs mixing of multiple audio sources, using the timestamps corresponding
 *  to each audio packet from each source to synchronise all sources together.
 *
 *  To use it, create an instance, passing in the AudioStreamBasicDescription of your audio,
 *  then provide data for each source by calling @link AEAudioMixerBufferEnqueue @endlink. Or,
 *  provide callbacks for one or more sources with @link AEAudioMixerBufferSetSourceCallbacks @endlink,
 *  which will cause this class to call your callbacks when data is needed.
 *
 *  Then, call @link AEAudioMixerBufferDequeue @endlink to consume mixed and synchronised audio
 *  ready for playback, recording, etc.
 */
@interface AEAudioMixerBuffer : NSObject

/*!
 * Initialiser
 *
 * @param audioDescription  The AudioStreamBasicDescription defining the audio format used
 */
- (id)initWithAudioDescription:(AudioStreamBasicDescription)audioDescription;

/*!
 * Enqueue audio
 *
 *  Feed the buffer with audio blocks. Identify each source via the `source` parameter. You
 *  may use any identifier you like - pointers, numbers, etc (just cast to AEAudioMixerBufferSource).
 *
 *  When you enqueue audio from a new source (that is, the `source` value is one that hasn't been
 *  seen before, this class will automatically reconfigure itself to start mixing the new source.
 *  However, this will happen at some point in the near future, not immediately, so one or two buffers
 *  may be lost. If this is a problem, then call this function first on the main thread, for each source,
 *  with a NULL audio buffer, and a lengthInFrames value of 0.
 *
 * @param mixerBuffer    The mixer buffer.
 * @param source         The audio source. This can be anything you like, as long as it is not NULL, and is unique to each source.
 * @param audio          The audio buffer list.
 * @param lengthInFrames The length of audio.
 * @param hostTime       The timestamp, in host ticks, associated with the audio.
 */
void AEAudioMixerBufferEnqueue(AEAudioMixerBuffer *mixerBuffer, AEAudioMixerBufferSource source, AudioBufferList *audio, UInt32 lengthInFrames, UInt64 hostTime);

/*!
 * Assign callbacks for a source
 *
 *  Rather than providing audio for a source using @link AEAudioMixerBufferEnqueue @endlink, you may
 *  provide callbacks which will be called by the mixer as required. You must either provide audio via
 *  @link AEAudioMixerBufferEnqueue @endlink, or via this method, but never both.
 *
 * @param renderCallback    The render callback, used to receive audio.
 * @param peekCallback      The peek callback, used to get info about the source's buffer status.
 * @param userInfo          An opaque pointer that will be provided to the callbacks.
 * @param source            The audio source.
 */
- (void)setRenderCallback:(AEAudioMixerBufferSourceRenderCallback)renderCallback peekCallback:(AEAudioMixerBufferSourcePeekCallback)peekCallback userInfo:(void *)userInfo forSource:(AEAudioMixerBufferSource)source;

/*!
 * Dequeue audio
 *
 *  Call this function to receive synchronised and mixed audio.
 *
 * @param mixerBuffer       The mixer buffer.
 * @param bufferList        The buffer list to write audio to. The mData pointers 
 *                          may be NULL, in which case an internal buffer will be provided.
 *                          You may also pass a NULL value, which will simply discard the given
 *                          number of frames.
 * @param ioLengthInFrames  On input, the number of frames of audio to dequeue. On output, 
 *                          the number of frames returned.
 */
void AEAudioMixerBufferDequeue(AEAudioMixerBuffer *mixerBuffer, AudioBufferList *bufferList, UInt32 *ioLengthInFrames);

/*!
 * Dequeue a single source
 *
 *  Normally not used, but if you wish to simply use this class to synchronise the audio across
 *  a number of sources, rather than mixing the sources together also, then this function allows you
 *  to access the synchronized audio for each source.
 *
 *  Do not use this function together with AEAudioMixerBufferDequeue.
 *
 * @param mixerBuffer       The mixer buffer.
 * @param source            The audio source.
 * @param bufferList        The buffer list to write audio to. The mData pointers 
 *                          may be NULL, in which case an internal buffer will be provided.
 * @param ioLengthInFrames  On input, the number of frames of audio to dequeue. On output, 
 *                          the number of frames returned.
 */
void AEAudioMixerBufferDequeueSingleSource(AEAudioMixerBuffer *mixerBuffer, AEAudioMixerBufferSource source, AudioBufferList *bufferList, UInt32 *ioLengthInFrames);

/*!
 * Peek the audio buffer
 *
 *  Use this to determine how much audio is currently buffered, and the corresponding next timestamp.
 *
 * @param mixerBuffer       The mixer buffer
 * @param outNextTimestamp  If not NULL, the timestamp in host ticks of the next available audio
 * @return Number of frames of available audio, in the specified audio format.
 */
UInt32 AEAudioMixerBufferPeek(AEAudioMixerBuffer *mixerBuffer, uint64_t *outNextTimestamp);

/*!
 * Set a different AudioStreamBasicDescription for a source
 */
- (void)setAudioDescription:(AudioStreamBasicDescription*)audioDescription forSource:(AEAudioMixerBufferSource)source;

/*!
 * Set volume for source
 */
- (void)setVolume:(float)volume forSource:(AEAudioMixerBufferSource)source;

/*!
 * Get volume for source
 */
- (float)volumeForSource:(AEAudioMixerBufferSource)source;

/*!
 * Set pan for source
 */
- (void)setPan:(float)pan forSource:(AEAudioMixerBufferSource)source;

/*!
 * Get pan for source
 */
- (float)panForSource:(AEAudioMixerBufferSource)source;

/*!
 * Force the mixer to unregister a source
 *
 *  After this function is called, the mixer will have reconfigured to stop
 *  mixing the given source. If callbacks for the source were provided, these
 *  will never be called again after this function returns.
 *
 *  Use of this function is entirely optional - the mixer buffer will automatically
 *  unregister sources it is no longer receiving audio for, and will clean up when
 *  deallocated.
 *
 * @param source            The audio source.
 */
- (void)unregisterSource:(AEAudioMixerBufferSource)source;

@end
