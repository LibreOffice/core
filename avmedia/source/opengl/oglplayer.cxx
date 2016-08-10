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
using namespace libgltf;

namespace avmedia { namespace ogl {

OGLPlayer::OGLPlayer()
    : Player_BASE(m_aMutex)
    , m_pHandle(nullptr)
    , m_xContext(OpenGLContext::Create())
    , m_pOGLWindow(nullptr)
    , m_bIsRendering(false)
{
}

OGLPlayer::~OGLPlayer()
{
    osl::MutexGuard aGuard(m_aMutex);
    if( m_pHandle )
    {
        m_xContext->makeCurrent();
        gltf_renderer_release(m_pHandle);
        m_xContext->dispose();
    }
    releaseInputFiles();
}

static bool lcl_LoadFile( glTFFile* io_pFile, const OUString& rURL)
{
    SvFileStream aStream( rURL, StreamMode::READ );
    if( !aStream.IsOpen() )
        return false;

    const sal_Int64 nBytes = aStream.remainingSize();
    char* pBuffer = new char[nBytes];
    aStream.ReadBytes(pBuffer, nBytes);
    aStream.Close();

    io_pFile->buffer = pBuffer;
    io_pFile->size = nBytes;

    return true;
}

bool OGLPlayer::create( const OUString& rURL )
{
    osl::MutexGuard aGuard(m_aMutex);

    m_sURL = rURL;

    // Convert URL to a system path
    const INetURLObject aURLObj(m_sURL);
    const std::string sFilePath = OUStringToOString( aURLObj.getFSysPath(INetURLObject::FSYS_DETECT), RTL_TEXTENCODING_UTF8 ).getStr();

    // Load *.json file and init renderer
    m_pHandle = gltf_renderer_init(sFilePath, m_vInputFiles);

    if( !m_pHandle )
    {
        SAL_WARN("avmedia.opengl", "gltf_renderer_init returned an invalid glTFHandle");
        return false;
    }

    // Load external resources
    for(glTFFile & rFile : m_vInputFiles)
    {
        if( !rFile.filename.empty() )
        {
            const OUString sFilesURL =
                INetURLObject::GetAbsURL(m_sURL,OStringToOUString(OString(rFile.filename.c_str()),RTL_TEXTENCODING_UTF8));
            if( rFile.type == GLTF_IMAGE )
            {
                // Load images as bitmaps
                GraphicFilter aFilter;
                Graphic aGraphic;
                if( aFilter.ImportGraphic(aGraphic, INetURLObject(sFilesURL)) != GRFILTER_OK )
                {
                    rFile.buffer = nullptr;
                    rFile.imagewidth = 0;
                    rFile.imageheight = 0;
                    SAL_WARN("avmedia.opengl", "Can't load texture file: " + sFilesURL);
                    return false;
                }
                BitmapEx aBitmapEx = aGraphic.GetBitmapEx();
                rFile.buffer = new char[4 * aBitmapEx.GetSizePixel().Width() * aBitmapEx.GetSizePixel().Height()];
                OpenGLHelper::ConvertBitmapExToRGBATextureBuffer(aBitmapEx, reinterpret_cast<sal_uInt8*>(rFile.buffer), true);
                rFile.imagewidth = aBitmapEx.GetSizePixel().Width();
                rFile.imageheight = aBitmapEx.GetSizePixel().Height();
            }
            else if( rFile.type == GLTF_BINARY || rFile.type == GLTF_GLSL )
            {
                if( !lcl_LoadFile(&rFile, sFilesURL) )
                {
                    rFile.buffer = nullptr;
                    rFile.size = 0;
                    SAL_WARN("avmedia.opengl", "Can't load glTF file: " + sFilesURL);
                    return false;
                }
            }
        }
    }

    // Set timer
    m_aTimer.SetTimeout(8); // is 125fps enough for anyone ?
    m_aTimer.SetPriority(SchedulerPriority::HIGH_IDLE);
    m_aTimer.SetTimeoutHdl(LINK(this,OGLPlayer,TimerHandler));

    return true;
}

void OGLPlayer::releaseInputFiles()
{
    for (size_t i = 0; i < m_vInputFiles.size() && m_vInputFiles[i].buffer; ++i)
    {
        delete [] m_vInputFiles[i].buffer;
        m_vInputFiles[i].buffer = nullptr;
    }
    m_vInputFiles.clear();
}

void SAL_CALL OGLPlayer::start() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);

    if(!m_pOGLWindow)
        return;

    gltf_animation_resume(m_pHandle);
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

double SAL_CALL OGLPlayer::getMediaTime() throw ( css::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
    return gltf_animation_get_time(m_pHandle);
}

void SAL_CALL OGLPlayer::setPlaybackLoop( sal_Bool bSet ) throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
    gltf_animation_set_looping(m_pHandle, bSet);
}

sal_Bool SAL_CALL OGLPlayer::isPlaybackLoop() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);
    return gltf_animation_get_looping(m_pHandle);
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
    return OpenGLHelper::getGLVersion() >= 3.0;
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

    if( !m_xContext->init(pChildWindow) )
    {
        SAL_WARN("avmedia.opengl", "Context initialization failed");
        return uno::Reference< media::XPlayerWindow >();
    }

    if( !m_xContext->supportMultiSampling() )
    {
        SAL_WARN("avmedia.opengl", "Context does not support multisampling!");
        return uno::Reference< media::XPlayerWindow >();
    }

    if( !lcl_CheckOpenGLRequirements() )
    {
        SAL_WARN("avmedia.opengl", "Your platform does not have the minimal OpenGL requiremenets!");
        return uno::Reference< media::XPlayerWindow >();
    }

    Size aSize = pChildWindow->GetSizePixel();
    m_xContext->setWinSize(aSize);
    m_pHandle->viewport.x = 0;
    m_pHandle->viewport.y = 0;
    m_pHandle->viewport.width = aSize.Width();
    m_pHandle->viewport.height = aSize.Height();

    // TODO: Use the error codes to print a readable error message
    int nRet = gltf_renderer_set_content(m_pHandle, m_vInputFiles);
    releaseInputFiles();
    if( nRet != 0 )
    {
        SAL_WARN("avmedia.opengl", "Error occurred while setting up the scene! Error code: " << nRet);
        return uno::Reference< media::XPlayerWindow >();
    }
    // The background color is white by default, but we need to separate the
    // OpenGL window from the main window so set background color to grey
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    m_pOGLWindow = new OGLWindow(*m_pHandle, m_xContext, *pChildWindow->GetParent());
    return uno::Reference< media::XPlayerWindow >( m_pOGLWindow );
}

uno::Reference< media::XFrameGrabber > SAL_CALL OGLPlayer::createFrameGrabber()
     throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    assert(m_pHandle);

    if( !m_xContext->init() )
    {
        SAL_WARN("avmedia.opengl", "Offscreen context initialization failed");
        return uno::Reference< media::XFrameGrabber >();
    }

    if( !m_xContext->supportMultiSampling() )
    {
        SAL_WARN("avmedia.opengl", "Context does not support multisampling!");
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

    int nRet = gltf_renderer_set_content(m_pHandle, m_vInputFiles);
    releaseInputFiles();
    if( nRet != 0 )
    {
        SAL_WARN("avmedia.opengl", "Error occurred while setting up the scene! Error code: " << nRet);
        return uno::Reference< media::XFrameGrabber >();
    }
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    OGLFrameGrabber *pFrameGrabber = new OGLFrameGrabber( *m_pHandle );
    return uno::Reference< media::XFrameGrabber >( pFrameGrabber );
}

OUString SAL_CALL OGLPlayer::getImplementationName()
     throw ( css::uno::RuntimeException, std::exception )
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
    return { "com.sun.star.media.Player_OpenGL" };
}

IMPL_LINK_TYPED(OGLPlayer,TimerHandler,Timer*,pTimer,void)
{
    if (pTimer == &m_aTimer)
    {
        osl::MutexGuard aGuard(m_aMutex);
        assert(m_pOGLWindow);
        m_pOGLWindow->update();
    }
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
