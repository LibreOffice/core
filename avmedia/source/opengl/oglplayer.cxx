/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oglplayer.hxx"
#include "oglframegrabber.hxx"
#include "oglwindow.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/urlobj.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

using namespace com::sun::star;

namespace avmedia { namespace ogl {

OGLPlayer::OGLPlayer()
    : Player_BASE(m_aMutex)
    , m_pHandle(NULL)
{
}

OGLPlayer::~OGLPlayer()
{
    gltf_renderer_release(m_pHandle);
}

static bool lcl_LoadFile( glTFFile* io_pFile, const OUString& rURL)
{
    SvFileStream aStream( rURL, STREAM_READ );
    if( !aStream.IsOpen() )
        return false;

    const sal_Int64 nBytes = aStream.remainingSize();
    char* pBuffer = new char[nBytes];
    aStream.Read( pBuffer, nBytes );
    aStream.Close();

    io_pFile->buffer = pBuffer;
    io_pFile->size = nBytes;

    return true;
}

bool OGLPlayer::create( const OUString& rURL )
{
    m_sURL = rURL;

    // Load *.json file and init renderer
    glTFFile aJsonFile;
    aJsonFile.type = GLTF_JSON;
    OString sFileName = OUStringToOString(INetURLObject(m_sURL).GetLastName(),RTL_TEXTENCODING_UTF8);
    aJsonFile.filename = (char*)sFileName.getStr();
    if( !lcl_LoadFile(&aJsonFile, m_sURL) )
    {
        SAL_WARN("avmedia.opengl", "Can't load *.json file: " + sFileName);
        return false;
    }

    m_pHandle = gltf_renderer_init(&aJsonFile);

    if( !m_pHandle || !m_pHandle->files )
    {
        SAL_WARN("avmedia.opengl", "gltf_renderer_init returned an invalid glTFHandle");
        return false;
    }

    // Load external resources
    for( size_t i = 0; i < m_pHandle->size; ++i )
    {
        glTFFile* pFile = m_pHandle->files[i];
        if( pFile && pFile->filename )
        {
            const OUString sFilesURL =
                INetURLObject::GetAbsURL(m_sURL,OStringToOUString(OString(pFile->filename),RTL_TEXTENCODING_UTF8));
            if( pFile->type == GLTF_IMAGE )
            {
                // Load images as bitmaps
                GraphicFilter aFilter;
                Graphic aGraphic;
                aFilter.ImportGraphic(aGraphic, INetURLObject(sFilesURL));
                const BitmapEx aBitmapEx = aGraphic.GetBitmapEx();
                pFile->buffer = (char*)OpenGLHelper::ConvertBitmapExToRGBABuffer(aBitmapEx);
                pFile->imagewidth = aBitmapEx.GetSizePixel().Width();
                pFile->imageheight = aBitmapEx.GetSizePixel().Height();
            }
            else if( pFile->type == GLTF_BINARY || pFile->type == GLTF_GLSL )
            {
                if( !lcl_LoadFile(pFile, sFilesURL) )
                {
                    SAL_WARN("avmedia.opengl", "Can't load glTF file: " + sFilesURL);
                    return false;
                }
            }
        }
    }

    // Set timer
    m_aTimer.SetTimeout(10);
    m_aTimer.SetTimeoutHdl(LINK(this,OGLPlayer,TimerHandler));
    return true;
}

void SAL_CALL OGLPlayer::start() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    gltf_animation_start(m_pHandle);
    m_aTimer.Start();
}

void SAL_CALL OGLPlayer::stop() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    gltf_animation_stop(m_pHandle);
    m_aTimer.Stop();
}

sal_Bool SAL_CALL OGLPlayer::isPlaying() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    return (sal_Bool)gltf_animation_is_playing(m_pHandle);
}

double SAL_CALL OGLPlayer::getDuration() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    return gltf_animation_get_duration(m_pHandle);
}

void SAL_CALL OGLPlayer::setMediaTime( double fTime ) throw ( uno::RuntimeException, std::exception )
{
    // TODO: doesn't work, but cause problem in playing
    osl::MutexGuard aGuard(m_aMutex);
    (void) fTime;
    //gltf_animation_set_time(m_pHandle, fTime);
}

double SAL_CALL OGLPlayer::getMediaTime() throw ( ::com::sun::star::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    return 0.0; //gltf_animation_get_time(m_pHandle);
}

double SAL_CALL OGLPlayer::getRate() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // Seems unused
    return 1.0;
}

void SAL_CALL OGLPlayer::setPlaybackLoop( sal_Bool bSet ) throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    gltf_animation_set_looping(m_pHandle, (int)bSet);
}

sal_Bool SAL_CALL OGLPlayer::isPlaybackLoop() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    return (sal_Bool)gltf_animation_get_looping(m_pHandle);
}

void SAL_CALL OGLPlayer::setVolumeDB( sal_Int16 /*nVolumDB*/ ) throw ( uno::RuntimeException, std::exception )
{
    // OpenGL models have no sound.
}

sal_Int16 SAL_CALL OGLPlayer::getVolumeDB() throw ( uno::RuntimeException, std::exception )
{
    // OpenGL models have no sound.
    return 0;
}

void SAL_CALL OGLPlayer::setMute( sal_Bool /*bSet*/ ) throw ( uno::RuntimeException, std::exception )
{
    // OpenGL models have no sound.
}

sal_Bool SAL_CALL OGLPlayer::isMute() throw ( uno::RuntimeException, std::exception )
{
    // OpenGL models have no sound.
    return false;
}

awt::Size SAL_CALL OGLPlayer::getPreferredPlayerWindowSize() throw ( uno::RuntimeException, std::exception )
{
    return awt::Size( 480, 360 );
}

uno::Reference< media::XPlayerWindow > SAL_CALL OGLPlayer::createPlayerWindow( const uno::Sequence< uno::Any >& rArguments )
     throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    assert( rArguments.getLength() >= 3 );

    sal_IntPtr pIntPtr = 0;
    rArguments[ 2 ] >>= pIntPtr;
    SystemChildWindow *pChildWindow = reinterpret_cast< SystemChildWindow* >( pIntPtr );

    if( !m_aContext.init(pChildWindow) )
    {
        SAL_WARN("avmedia.opengl", "Context initialization failed");
        return uno::Reference< media::XPlayerWindow >();
    }

    Size aSize = pChildWindow->GetSizePixel();
    m_aContext.setWinSize(aSize);
    m_pHandle->viewport.x = 0;
    m_pHandle->viewport.y = 0;
    m_pHandle->viewport.width = aSize.Width();
    m_pHandle->viewport.height = aSize.Height();
    gltf_renderer_set_content(m_pHandle);
    m_pOGLWindow = new OGLWindow(m_pHandle, &m_aContext, pChildWindow);
    return uno::Reference< media::XPlayerWindow >( m_pOGLWindow );
}

uno::Reference< media::XFrameGrabber > SAL_CALL OGLPlayer::createFrameGrabber()
     throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);

    if( !m_aContext.init() )
    {
        SAL_WARN("avmedia.opengl", "Offscreen context initialization failed");
        return uno::Reference< media::XFrameGrabber >();
    }

    m_pHandle->viewport.x = 0;
    m_pHandle->viewport.y = 0;
    m_pHandle->viewport.width = getPreferredPlayerWindowSize().Width;
    m_pHandle->viewport.height = getPreferredPlayerWindowSize().Height;
    gltf_renderer_set_content(m_pHandle);
    OGLFrameGrabber *pFrameGrabber = new OGLFrameGrabber( m_pHandle );
    return uno::Reference< media::XFrameGrabber >( pFrameGrabber );
}

OUString SAL_CALL OGLPlayer::getImplementationName()
     throw ( ::com::sun::star::uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.avmedia.Player_OpenGL");
}

sal_Bool SAL_CALL OGLPlayer::supportsService( const OUString& rServiceName )
     throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL OGLPlayer::getSupportedServiceNames()
     throw ( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = OUString("com.sun.star.media.Player_OpenGL");
    return aRet;
}

IMPL_LINK(OGLPlayer,TimerHandler,Timer*,pTimer)
{
    if (pTimer == &m_aTimer)
    {
        m_pOGLWindow->update();
        m_aTimer.Start();
    }

    return 0;
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
