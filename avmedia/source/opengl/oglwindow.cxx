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
    , m_aLastMousePos(Point())
{
}

OGLWindow::~OGLWindow()
{
    dispose();
}

void SAL_CALL OGLWindow::update() throw (css::uno::RuntimeException, std::exception)
{
    m_pContext->makeCurrent();
    gltf_prepare_renderer(m_pHandle);
    gltf_renderer(m_pHandle);
    gltf_complete_renderer(m_pHandle);
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
    m_pEventHandler->GetParent()->RemoveEventListener( LINK(this, OGLWindow, FocusGrabber));
    m_pEventHandler->RemoveEventListener( LINK(this, OGLWindow, CameraHandler));
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
            // Grab focus to the OpenGL window when mouse pointer is over it
            if( aWinRect.IsInside(rMousePos) )
            {
                if ( !m_pEventHandler->HasFocus() )
                {
                    m_pEventHandler->GrabFocus();
                }
            }
            // Move focus to the document when mouse is not over the OpenGL window
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
            const sal_uInt16 nCode = pKeyEvt->GetKeyCode().GetCode();
            if (nCode == KEY_Q || nCode == KEY_E ||
                nCode == KEY_A || nCode == KEY_D ||
                nCode == KEY_W || nCode == KEY_S )
            {
                // Calculate movement
                glm::vec3 vMoveBy;
                {
                    glm::vec3 vEye;
                    glm::vec3 vView;
                    glm::vec3 vUp;
                    gltf_get_camera_pos(m_pHandle, &vEye,&vView,&vUp);
                    float fModelSize =(float)gltf_get_model_size(m_pHandle);

                    glm::vec3 vMove = vView-vEye;
                    vMove = glm::normalize(vMove);
                    vMove *= 25.0f;
                    glm::vec3 vStrafe = glm::cross(vView-vEye, vUp);
                    vStrafe = glm::normalize(vStrafe);
                    vStrafe *= 25.0f;
                    glm::vec3 vMup = glm::cross(vView-vEye,glm::vec3(1.f,0.f,0.f) );
                    vMup = glm::normalize(vMup);
                    vMup *= 25.0f;

                    if(nCode == KEY_Q)vMoveBy += vMove*(0.0005f*fModelSize);
                    if(nCode == KEY_E)vMoveBy -= vMove*(0.0005f*fModelSize);
                    if(nCode == KEY_A)vMoveBy -= vStrafe*(0.0005f*fModelSize);
                    if(nCode == KEY_D)vMoveBy += vStrafe*(0.0005f*fModelSize);
                    if(nCode == KEY_W)vMoveBy -= vMup*(0.0005f*fModelSize);
                    if(nCode == KEY_S)vMoveBy += vMup*(0.0005f*fModelSize);
                }
                gltf_renderer_move_camera(m_pHandle, vMoveBy.x, vMoveBy.y, vMoveBy.z, 0.0);
                update();
            }
        }
    }
    // TODO: Clean this mess up after libgltf gets a working camera handling
     else if( pEvent->GetId() == VCLEVENT_WINDOW_KEYUP )
    {
        KeyEvent* pKeyEvt = (KeyEvent*)pEvent->GetData();
        if(pKeyEvt)
        {
            const sal_uInt16 nCode = pKeyEvt->GetKeyCode().GetCode();
            if (nCode == KEY_Q || nCode == KEY_E ||
                nCode == KEY_A || nCode == KEY_D ||
                nCode == KEY_W || nCode == KEY_S )
            {
                gltf_renderer_move_camera(m_pHandle, 0.0, 0.0, 0.0, 0.0);
            }
        }
    }
    else if( pEvent->GetId() == VCLEVENT_WINDOW_MOUSEBUTTONDOWN )
    {
        MouseEvent* pMouseEvt = (MouseEvent*)pEvent->GetData();
        if(pMouseEvt && pMouseEvt->IsLeft() && pMouseEvt->GetClicks() == 1)
        {
            m_aLastMousePos = pMouseEvt->GetPosPixel();
        }
    }
    else if( pEvent->GetId() == VCLEVENT_WINDOW_MOUSEMOVE )
    {
        if ( !m_pEventHandler->HasFocus() )
        {
            m_pEventHandler->GrabFocus();
        }
        MouseEvent* pMouseEvt = (MouseEvent*)pEvent->GetData();
        if(pMouseEvt && pMouseEvt->IsLeft())
        {
            const Point& aCurPos = pMouseEvt->GetPosPixel();
            float fSensitivity = std::min(m_pHandle->viewport.width, m_pHandle->viewport.height);
            if (fSensitivity == 0.0)
                fSensitivity = 1.0;
            else
                fSensitivity = 540.0 / fSensitivity;

            long nDeltaX = m_aLastMousePos.X()-aCurPos.X();
            long nDeltaY = aCurPos.Y()-m_aLastMousePos.Y();
            // TODO: It seems this method just moves the camera but not rotate it.
            gltf_renderer_rotate_camera(m_pHandle, (float)nDeltaX*fSensitivity, (float)nDeltaY*fSensitivity, 0.0, 0.0);
            update();

            m_aLastMousePos = aCurPos;
        }
    }
    return 0;
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
