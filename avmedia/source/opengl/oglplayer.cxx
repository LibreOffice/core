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

#include <cassert>

using namespace com::sun::star;

namespace avmedia { namespace ogl {

OGLPlayer::OGLPlayer()
    : Player_BASE(m_aMutex)
    , m_pHandle(NULL)
    , m_pOGLWindow(NULL)
    , m_bIsRendering(false)
{
}

OGLPlayer::~OGLPlayer()
{
    osl::MutexGuard aGuard(m_aMutex);
    if( m_pHandle )
    {
        for (size_t i = 0; i < m_pHandle->size && m_pHandle->files[i].buffer; ++i)
        {
            if (m_pHandle->files[i].type != GLTF_JSON)
            {
                delete [] m_pHandle->files[i].buffer;
            }
        }
        m_aContext.makeCurrent();
        gltf_renderer_release(m_pHandle);
    }
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
    osl::MutexGuard aGuard(m_aMutex);

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

    delete [] aJsonFile.buffer;

    if( !m_pHandle || !m_pHandle->files )
    {
        SAL_WARN("avmedia.opengl", "gltf_renderer_init returned an invalid glTFHandle");
        return false;
    }

    // Load external resources
    for( size_t i = 0; i < m_pHandle->size; ++i )
    {
        glTFFile& rFile = m_pHandle->files[i];
        if( rFile.filename )
        {
            const OUString sFilesURL =
                INetURLObject::GetAbsURL(m_sURL,OStringToOUString(OString(rFile.filename),RTL_TEXTENCODING_UTF8));
            if( rFile.type == GLTF_IMAGE )
            {
                // Load images as bitmaps
                GraphicFilter aFilter;
                Graphic aGraphic;
                if( aFilter.ImportGraphic(aGraphic, INetURLObject(sFilesURL)) != GRFILTER_OK )
                {
                    rFile.buffer = 0;
                    rFile.imagewidth = 0;
                    rFile.imageheight = 0;
                    SAL_WARN("avmedia.opengl", "Can't load texture file: " + sFilesURL);
                    return false;
                }
                BitmapEx aBitmapEx = aGraphic.GetBitmapEx();
                aBitmapEx.Mirror(BMP_MIRROR_VERT);
                rFile.buffer = (char*)OpenGLHelper::ConvertBitmapExToRGBABuffer(aBitmapEx);
                rFile.imagewidth = aBitmapEx.GetSizePixel().Width();
                rFile.imageheight = aBitmapEx.GetSizePixel().Height();
            }
            else if( rFile.type == GLTF_BINARY || rFile.type == GLTF_GLSL )
            {
                if( !lcl_LoadFile(&rFile, sFilesURL) )
                {
                    rFile.buffer = 0;
                    rFile.size = 0;
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
    assert(m_pHandle);

    if(!m_pOGLWindow)
        return;

    // gltf_animation_start play animation from the time 0.0,
    // but OGLPlayer::start used as play from that time where it was stopped before
    double fTime = gltf_animation_get_time(m_pHandle);
    gltf_animation_start(m_pHandle);
    gltf_animation_set_time(m_pHandle, fTime);
    m_aTimer.Start();
    m_bIsRendering = true;
}

void SAL_CALL OGLPlayer::stop() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
    m_aTimer.Stop();
    gltf_animation_stop(m_pHandle);
    m_bIsRendering = false;
}

sal_Bool SAL_CALL OGLPlayer::isPlaying() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
    // Here isPlaying means model is rendered in the window and
    // able to interact with the user (e.g. moving camera)
    if( getDuration() > 0.0 )
        return gltf_animation_is_playing(m_pHandle);
    else
        return m_bIsRendering;
}

double SAL_CALL OGLPlayer::getDuration() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
    return gltf_animation_get_duration(m_pHandle);
}

void SAL_CALL OGLPlayer::setMediaTime( double fTime ) throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
    gltf_animation_set_time(m_pHandle, fTime);
}

double SAL_CALL OGLPlayer::getMediaTime() throw ( ::com::sun::star::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
    return gltf_animation_get_time(m_pHandle);
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
    assert(m_pHandle);
    gltf_animation_set_looping(m_pHandle, (int)bSet);
}

sal_Bool SAL_CALL OGLPlayer::isPlaybackLoop() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
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

static bool lcl_CheckOpenGLRequirements()
{
    float fVersion = OpenGLHelper::getGLVersion();
    if( fVersion >= 3.3 )
    {
        return true;
    }
    else if( fVersion >= 3.0 )
    {
        return glewIsSupported("GL_ARB_sampler_objects");
    }
    else if( fVersion >= 2.1 )
    {
        return glewIsSupported("GL_ARB_sampler_objects GL_ARB_framebuffer_object GL_ARB_vertex_array_object");
    }

    return false;
}

uno::Reference< media::XPlayerWindow > SAL_CALL OGLPlayer::createPlayerWindow( const uno::Sequence< uno::Any >& rArguments )
     throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    assert( rArguments.getLength() >= 3 );
    assert(m_pHandle);

    sal_IntPtr pIntPtr = 0;
    rArguments[ 2 ] >>= pIntPtr;
    SystemChildWindow *pChildWindow = reinterpret_cast< SystemChildWindow* >( pIntPtr );

    if( !pChildWindow )
    {
        SAL_WARN("avmedia.opengl", "Failed to get the SystemChildWindow for rendering!");
        return uno::Reference< media::XPlayerWindow >();
    }
    assert(pChildWindow->GetParent());

    if( !m_aContext.init(pChildWindow) )
    {
        SAL_WARN("avmedia.opengl", "Context initialization failed");
        return uno::Reference< media::XPlayerWindow >();
    }

    if( !lcl_CheckOpenGLRequirements() )
    {
        SAL_WARN("avmedia.opengl", "Your platform does not have the minimal OpenGL requiremenets!");
        return uno::Reference< media::XPlayerWindow >();
    }

    Size aSize = pChildWindow->GetSizePixel();
    m_aContext.setWinSize(aSize);
    m_pHandle->viewport.x = 0;
    m_pHandle->viewport.y = 0;
    m_pHandle->viewport.width = aSize.Width();
    m_pHandle->viewport.height = aSize.Height();

    // TODO: In libgltf different return values are defined (for different errors)
    // but these error codes are not part of the library interface
    int nRet = gltf_renderer_set_content(m_pHandle);
    if( nRet != 0 )
    {
        SAL_WARN("avmedia.opengl", "Error occured while parsing *.json file! Error code: " << nRet);
        return uno::Reference< media::XPlayerWindow >();
    }
    // The background color is white by default, but we need to separate the
    // OpenGL window from the main window so set background color to grey
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    m_pOGLWindow = new OGLWindow(*m_pHandle, m_aContext, *pChildWindow->GetParent());
    return uno::Reference< media::XPlayerWindow >( m_pOGLWindow );
}

uno::Reference< media::XFrameGrabber > SAL_CALL OGLPlayer::createFrameGrabber()
     throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);

    if( !m_aContext.init() )
    {
        SAL_WARN("avmedia.opengl", "Offscreen context initialization failed");
        return uno::Reference< media::XFrameGrabber >();
    }

    if( !lcl_CheckOpenGLRequirements() )
    {
        SAL_WARN("avmedia.opengl", "Your platform does not have the minimal OpenGL requiremenets!");
        return uno::Reference< media::XFrameGrabber >();
    }

    m_pHandle->viewport.x = 0;
    m_pHandle->viewport.y = 0;
    m_pHandle->viewport.width = getPreferredPlayerWindowSize().Width;
    m_pHandle->viewport.height = getPreferredPlayerWindowSize().Height;

    int nRet = gltf_renderer_set_content(m_pHandle);
    if( nRet != 0 )
    {
        SAL_WARN("avmedia.opengl", "Error occured while parsing *.json file! Error code: " << nRet);
        return uno::Reference< media::XFrameGrabber >();
    }
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    OGLFrameGrabber *pFrameGrabber = new OGLFrameGrabber( *m_pHandle );
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
        osl::MutexGuard aGuard(m_aMutex);
        assert(m_pOGLWindow);
        m_pOGLWindow->update();
    }

    return 0;
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
