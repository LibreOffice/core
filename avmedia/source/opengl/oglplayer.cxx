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
{
}

OGLPlayer::~OGLPlayer()
{
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
    OString sFileName = OUStringToOString(m_sURL.copy(m_sURL.lastIndexOf("/")+1),RTL_TEXTENCODING_UTF8);
    aJsonFile.filename = (char*)sFileName.getStr();
    if( !lcl_LoadFile(&aJsonFile, m_sURL) )
        return false;

    m_pHandle = gltf_renderer_init(&aJsonFile);

    if( !m_pHandle || !m_pHandle->files )
        return false;

    // Load external resources
    for( size_t i = 0; i < m_pHandle->size; ++i )
    {
        glTFFile* pFile = m_pHandle->files[i];
        if( pFile && pFile->filename )
        {
            const OUString sFilesURL = m_sURL.copy(0,m_sURL.lastIndexOf("/")+1) +
                OStringToOUString(OString(pFile->filename),RTL_TEXTENCODING_UTF8);
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
                    return false;
            }
        }
    }
    return true;
}

void SAL_CALL OGLPlayer::start() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Start playing of gltf model (see com::sun::star::media::XPlayer)
}

void SAL_CALL OGLPlayer::stop() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Stop playing of gltf model (see com::sun::star::media::XPlayer)
}

sal_Bool SAL_CALL OGLPlayer::isPlaying() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Check whether gltf model is played by now (see com::sun::star::media::XPlayer)
    return false;
}

double SAL_CALL OGLPlayer::getDuration() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Get gltf's duration (see com::sun::star::media::XPlayer)
    return 0.0;
}

void SAL_CALL OGLPlayer::setMediaTime( double /*fTime*/ ) throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Set player to the specified point (see com::sun::star::media::XPlayer)
}

double SAL_CALL OGLPlayer::getMediaTime() throw ( ::com::sun::star::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Get player current time position (see com::sun::star::media::XPlayer)
    return 0.0;
}

double SAL_CALL OGLPlayer::getRate() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Get the speed of stream reading (see com::sun::star::media::XPlayer)
    return 1.0;
}

void SAL_CALL OGLPlayer::setPlaybackLoop( sal_Bool /*bSet*/ ) throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Set the playes replay itself when it ends  (see com::sun::star::media::XPlayer)
}

sal_Bool SAL_CALL OGLPlayer::isPlaybackLoop() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Check whether playing will restart after it ends  (see com::sun::star::media::XPlayer)
    return false;
}

void SAL_CALL OGLPlayer::setVolumeDB( sal_Int16 /*nVolumDB*/ ) throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Set audio volume (see com::sun::star::media::XPlayer)
}

sal_Int16 SAL_CALL OGLPlayer::getVolumeDB() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Get audio volume (see com::sun::star::media::XPlayer)
    return 0;
}

void SAL_CALL OGLPlayer::setMute( sal_Bool /*bSet*/ ) throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Set volume to 0 (see com::sun::star::media::XPlayer)
}

sal_Bool SAL_CALL OGLPlayer::isMute() throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    // TODO: Get whether volume is set to 0 by setMute (see com::sun::star::media::XPlayer)
    return false;
}

awt::Size SAL_CALL OGLPlayer::getPreferredPlayerWindowSize() throw ( uno::RuntimeException, std::exception )
{
    return awt::Size( 480, 360 ); // TODO: It will be good for OpenGL too?
}

uno::Reference< media::XPlayerWindow > SAL_CALL OGLPlayer::createPlayerWindow( const uno::Sequence< uno::Any >& /*aArguments*/ )
     throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );
    OGLWindow* pWindow = new OGLWindow(*this);
    return uno::Reference< media::XPlayerWindow >( pWindow );
}

uno::Reference< media::XFrameGrabber > SAL_CALL OGLPlayer::createFrameGrabber()
     throw ( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_aContext.init();
    //TODO: Use real values instead of constants.
    m_pHandle->viewport.x = 0;
    m_pHandle->viewport.y = 0;
    m_pHandle->viewport.width = 800;
    m_pHandle->viewport.height = 600;
    gltf_renderer_set_content(m_pHandle);
    OGLFrameGrabber *pFrameGrabber = new OGLFrameGrabber( m_pHandle );
    return uno::Reference< media::XFrameGrabber >( pFrameGrabber );;
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

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
