/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oglwindow.hxx"
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;

namespace avmedia { namespace ogl {

OGLWindow::OGLWindow( glTFHandle* pHandle, OpenGLContext* pContext, SystemChildWindow* pChildWindow )
    : m_pHandle( pHandle )
    , m_pContext( pContext )
    , m_pEventHandler( pChildWindow->GetParent() )
    , m_bVisible ( false )
    , meZoomLevel( media::ZoomLevel_ORIGINAL )
{
}

OGLWindow::~OGLWindow()
{
}

void SAL_CALL OGLWindow::update() throw (css::uno::RuntimeException, std::exception)
{
    m_pContext->makeCurrent();
    gltf_renderer_set_content(m_pHandle);
    gltf_prepare_renderer(m_pHandle);
    gltf_renderer(m_pHandle);
    gltf_complete_renderer();
    m_pContext->swapBuffers();
}

sal_Bool SAL_CALL OGLWindow::setZoomLevel( css::media::ZoomLevel eZoomLevel ) throw (css::uno::RuntimeException, std::exception)
{
    bool bRet = false;

    if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
        media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
    {
        if( eZoomLevel != meZoomLevel )
        {
            meZoomLevel = eZoomLevel;
        }
        bRet = true;
    }
    // TODO: set zoom level, not just store this value
    return bRet;
}

css::media::ZoomLevel SAL_CALL OGLWindow::getZoomLevel() throw (css::uno::RuntimeException, std::exception)
{
    return meZoomLevel;
}

void SAL_CALL OGLWindow::setPointerType( sal_Int32 ) throw (css::uno::RuntimeException, std::exception)
{
}

OUString SAL_CALL OGLWindow::getImplementationName() throw (css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.avmedia.Window_OpenGL");
}

sal_Bool SAL_CALL OGLWindow::supportsService( const OUString& rServiceName ) throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL OGLWindow::getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = OUString("com.sun.star.media.Window_OpenGL");
    return aRet;
}

void SAL_CALL OGLWindow::dispose() throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addEventListener( const uno::Reference< lang::XEventListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeEventListener( const uno::Reference< lang::XEventListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::setPosSize( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 /*nFlags*/ )
    throw (uno::RuntimeException, std::exception)
{
    if( m_pHandle->viewport.x != nX || m_pHandle->viewport.x != nY ||
        m_pHandle->viewport.width != nWidth || m_pHandle->viewport.height != nHeight )
    {
        m_pContext->setWinSize(Size(nWidth,nHeight));
        m_pHandle->viewport.x = nX;
        m_pHandle->viewport.y = nY;
        m_pHandle->viewport.width = nWidth;
        m_pHandle->viewport.height = nHeight;
        if( m_bVisible )
        {
            update();
        }
    }
}

awt::Rectangle SAL_CALL OGLWindow::getPosSize()
    throw (uno::RuntimeException, std::exception)
{
    return awt::Rectangle(m_pHandle->viewport.x, m_pHandle->viewport.y,
                          m_pHandle->viewport.width, m_pHandle->viewport.height);
}

void SAL_CALL OGLWindow::setVisible( sal_Bool bSet )
    throw (uno::RuntimeException, std::exception)
{
    if( bSet && !m_bVisible )
    {
        update();
        m_pEventHandler->GetParent()->AddEventListener( LINK(this, OGLWindow, FocusGrabber));
        m_pEventHandler->AddEventListener( LINK(this, OGLWindow, CameraHandler));
        m_pEventHandler->GrabFocus();
    }
    else if( !bSet )
    {
        m_pEventHandler->GetParent()->RemoveEventListener( LINK(this, OGLWindow, FocusGrabber));
        m_pEventHandler->RemoveEventListener( LINK(this, OGLWindow, CameraHandler));
    }
    m_bVisible = bSet;
}

void SAL_CALL OGLWindow::setEnable( sal_Bool )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::setFocus()
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addWindowListener( const uno::Reference< awt::XWindowListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeWindowListener( const uno::Reference< awt::XWindowListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addFocusListener( const uno::Reference< awt::XFocusListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeFocusListener( const uno::Reference< awt::XFocusListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addKeyListener( const uno::Reference< awt::XKeyListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeKeyListener( const uno::Reference< awt::XKeyListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addMouseListener( const uno::Reference< awt::XMouseListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeMouseListener( const uno::Reference< awt::XMouseListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addPaintListener( const uno::Reference< awt::XPaintListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removePaintListener( const uno::Reference< awt::XPaintListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

IMPL_LINK(OGLWindow, FocusGrabber, VclWindowEvent*, pEvent)
{
    assert(m_pEventHandler);
    if( pEvent->GetId() == VCLEVENT_WINDOW_MOUSEMOVE )
    {
        MouseEvent* pMouseEvt = (MouseEvent*)pEvent->GetData();
        if(pMouseEvt)
        {
            const Point& rMousePos = pMouseEvt->GetPosPixel();
            const Rectangle aWinRect(m_pEventHandler->GetPosPixel(),m_pEventHandler->GetSizePixel());
            if( aWinRect.IsInside(rMousePos) )
            {
                if ( !m_pEventHandler->HasFocus() )
                {
                    m_pEventHandler->GrabFocus();
                }
            }
            else if ( m_pEventHandler->HasFocus() )
            {
                m_pEventHandler->GrabFocusToDocument();
            }
        }
    }

    return 0;
}

IMPL_LINK(OGLWindow, CameraHandler, VclWindowEvent*, pEvent)
{
    if( pEvent->GetId() == VCLEVENT_WINDOW_KEYINPUT )
    {
        KeyEvent* pKeyEvt = (KeyEvent*)pEvent->GetData();
        if(pKeyEvt)
        {
            sal_uInt16 nCode = pKeyEvt->GetKeyCode().GetCode();
            m_pContext->makeCurrent();

            // Calculate movement
            glm::vec3 vMoveBy;
            {
                glm::vec3 vEye;
                glm::vec3 vView;
                glm::vec3 vUp;
                gltf_get_camera_pos(&vEye,&vView,&vUp);
                float fModelSize =(float)gltf_get_model_size();

                glm::vec3 vMove = vView-vEye;
                vMove = glm::normalize(vMove);
                vMove *= 25.0f;
                glm::vec3 vStrafe = glm::cross(vView-vEye, vUp);
                vStrafe = glm::normalize(vStrafe);
                vStrafe *= 25.0f;
                glm::vec3 vMup = glm::cross(vView-vEye,glm::vec3(1.f,0.f,0.f) );
                vMup = glm::normalize(vMup);
                vMup *= 25.0f;

                if(nCode == KEY_Q)vMoveBy += vMove*(0.1f*fModelSize);
                if(nCode == KEY_E)vMoveBy -= vMove*(0.1f*fModelSize);
                if(nCode == KEY_A)vMoveBy -= vStrafe*(0.1f*fModelSize);
                if(nCode == KEY_D)vMoveBy += vStrafe*(0.1f*fModelSize);
                if(nCode == KEY_W)vMoveBy -= vMup*(0.1f*fModelSize);
                if(nCode == KEY_S)vMoveBy += vMup*(0.1f*fModelSize);
            }

            gltf_renderer_move_camera(vMoveBy.x,vMoveBy.y,vMoveBy.z,10.0);
            gltf_prepare_renderer(m_pHandle);
            gltf_renderer(m_pHandle);
            gltf_complete_renderer();
            m_pContext->swapBuffers();
        }
    }
    return 0;
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
