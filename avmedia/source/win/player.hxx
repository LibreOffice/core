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



#ifndef _PLAYER_HXX
#define _PLAYER_HXX

#include "wincommon.hxx"

#include "com/sun/star/media/XPlayer.hdl"

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>

struct IGraphBuilder;
struct IBaseFilter;
struct IMediaControl;
struct IMediaEventEx;
struct IMediaSeeking;
struct IMediaPosition;
struct IBasicAudio;
struct IBasicVideo;
struct IVideoWindow;
struct IDDrawExclModeVideo;
struct IDirectDraw;
struct IDirectDrawSurface;

namespace avmedia { namespace win {

// ----------
// - Player -
// ----------
typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::media::XPlayer,
                                          ::com::sun::star::lang::XServiceInfo > Player_BASE;

class Player :  public cppu::BaseMutex,
                public Player_BASE
{
public:

                        Player( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
                        ~Player();

    bool                create( const ::rtl::OUString& rURL );

    void                setNotifyWnd( int nNotifyWnd );
    void                setDDrawParams( IDirectDraw* pDDraw, IDirectDrawSurface* pDDrawSurface );
    long                processEvent();

    const IVideoWindow* getVideoWindow() const;

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

    // ::cppu::OComponentHelper
    virtual void SAL_CALL disposing(void);

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMgr;

    ::rtl::OUString         maURL;
    IGraphBuilder*          mpGB;
    IBaseFilter*            mpOMF;
    IMediaControl*          mpMC;
    IMediaEventEx*          mpME;
    IMediaSeeking*          mpMS;
    IMediaPosition*         mpMP;
    IBasicAudio*            mpBA;
    IBasicVideo*            mpBV;
    IVideoWindow*           mpVW;
    IDDrawExclModeVideo*    mpEV;
    long                    mnUnmutedVolume;
    int                     mnFrameWnd;

    sal_Bool                mbMuted;
    sal_Bool                mbLooping;
    sal_Bool                mbAddWindow;

    void                    ImplLayoutVideoWindow();
};

} // namespace win
} // namespace avmedia

#endif // _PLAYER_HXX
