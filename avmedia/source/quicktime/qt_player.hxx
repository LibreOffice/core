/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef QT_PLAYER_HXX
#define QT_PLAYER_HXX

#include <osl/conditn.h>
#include "qt_common.hxx"

#include "com/sun/star/media/XPlayer.hdl"

namespace avmedia { namespace quicktime {

/*
// ----------
// - Player -
// ----------
*/

class Player : public ::cppu::WeakImplHelper2< ::com::sun::star::media::XPlayer,
                                               ::com::sun::star::lang::XServiceInfo >
{
public:

    Player( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
    ~Player();

    bool create( const ::rtl::OUString& rURL );

//    void processMessage( GstMessage *message );

    // XPlayer
    virtual void SAL_CALL start(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL stop(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isPlaying(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getDuration(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMediaTime( double fTime ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getMediaTime(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStopTime( double fTime ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getStopTime(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRate( double fRate ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getRate(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isPlaybackLoop(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMute( sal_Bool bSet ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isMute(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getVolumeDB(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getPreferredPlayerWindowSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow > SAL_CALL createPlayerWindow( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XFrameGrabber > SAL_CALL createFrameGrabber(  ) throw (::com::sun::star::uno::RuntimeException);
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    QTMovie* getMovie();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMgr;

    ::rtl::OUString         maURL;

    QTMovie                 *mpMovie;      // the Movie object
  /* GST
    sal_Bool                mbFakeVideo;
  */
    float                   mnUnmutedVolume;
    double                  mnStopTime;

    sal_Bool                mbMuted;
    sal_Bool                mbLooping;
    sal_Bool                mbInitialized;

    long                    mnWindowID;
    long                    mnDuration;
    int                     mnWidth;
    int                     mnHeight;

    long                    mnVersion;
    oslCondition            maSizeCondition;
};

} // namespace quicktime
} // namespace avmedia

#endif // QT_PLAYER_HXX

