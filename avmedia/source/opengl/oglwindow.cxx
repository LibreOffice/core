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
using namespace libgltf;

namespace avmedia { namespace ogl {

OGLWindow::OGLWindow( glTFHandle& rHandle, const rtl::Reference<OpenGLContext> &rContext, vcl::Window& rEventHandlerParent )
    : m_rHandle( rHandle )
    , m_xContext( rContext )
    , m_rEventHandler( rEventHandlerParent )
    , m_bVisible ( false )
    , m_aLastMousePos(Point(0,0))
    , m_bIsOrbitMode( false )
{
}

OGLWindow::~OGLWindow()
{
    dispose();
}

void SAL_CALL OGLWindow::update() throw (css::uno::RuntimeException, std::exception)
{
    m_xContext->makeCurrent();
    int nRet = gltf_prepare_renderer(&m_rHandle);
    if( nRet != 0 )
    {
        SAL_WARN("avmedia.opengl", "Error occurred while preparing for rendering! Error code: " << nRet);
        return;
    }
    gltf_renderer(&m_rHandle);
    gltf_complete_renderer(&m_rHandle);
    m_xContext->swapBuffers();
}

sal_Bool SAL_CALL OGLWindow::setZoomLevel( css::media::ZoomLevel /*eZoomLevel*/ ) throw (css::uno::RuntimeException, std::exception)
{
    return false;
}

css::media::ZoomLevel SAL_CALL OGLWindow::getZoomLevel() throw (css::uno::RuntimeException, std::exception)
{
    return media::ZoomLevel_ORIGINAL;
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
    aRet[0] = "com.sun.star.media.Window_OpenGL";
    return aRet;
}

void SAL_CALL OGLWindow::dispose() throw (uno::RuntimeException, std::exception)
{
    assert(m_rEventHandler.GetParent());
    m_rEventHandler.GetParent()->RemoveEventListener( LINK(this, OGLWindow, FocusGrabber));
    m_rEventHandler.RemoveEventListener( LINK(this, OGLWindow, CameraHandler));
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
    if( m_rHandle.viewport.x != nX || m_rHandle.viewport.x != nY ||
        m_rHandle.viewport.width != nWidth || m_rHandle.viewport.height != nHeight )
    {
        m_xContext->setWinSize(Size(nWidth,nHeight));
        m_rHandle.viewport.x = nX;
        m_rHandle.viewport.y = nY;
        m_rHandle.viewport.width = nWidth;
        m_rHandle.viewport.height = nHeight;
    }
}

awt::Rectangle SAL_CALL OGLWindow::getPosSize()
    throw (uno::RuntimeException, std::exception)
{
    return awt::Rectangle(m_rHandle.viewport.x, m_rHandle.viewport.y,
                          m_rHandle.viewport.width, m_rHandle.viewport.height);
}

void SAL_CALL OGLWindow::setVisible( sal_Bool bSet )
    throw (uno::RuntimeException, std::exception)
{
    assert(m_rEventHandler.GetParent());
    if( bSet && !m_bVisible )
    {
        m_rEventHandler.GetParent()->AddEventListener( LINK(this, OGLWindow, FocusGrabber));
        m_rEventHandler.AddEventListener( LINK(this, OGLWindow, CameraHandler));
        m_rEventHandler.GrabFocus();
    }
    else if( !bSet )
    {
        m_rEventHandler.GetParent()->RemoveEventListener( LINK(this, OGLWindow, FocusGrabber));
        m_rEventHandler.RemoveEventListener( LINK(this, OGLWindow, CameraHandler));
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

IMPL_LINK_TYPED(OGLWindow, FocusGrabber, VclWindowEvent&, rEvent, void)
{
    if( rEvent.GetId() == VCLEVENT_WINDOW_MOUSEMOVE )
    {
        MouseEvent* pMouseEvt = static_cast<MouseEvent*>(rEvent.GetData());
        if(pMouseEvt)
        {
            const Point& rMousePos = pMouseEvt->GetPosPixel();
            const Rectangle aWinRect(m_rEventHandler.GetPosPixel(),m_rEventHandler.GetSizePixel());
            // Grab focus to the OpenGL window when mouse pointer is over it
            if( aWinRect.IsInside(rMousePos) )
            {
                if ( !m_rEventHandler.HasFocus() )
                {
                    m_rEventHandler.GrabFocus();
                }
            }
            // Move focus to the document when mouse is not over the OpenGL window
            else if ( m_rEventHandler.HasFocus() )
            {
                m_rEventHandler.GrabFocusToDocument();
            }
        }
    }
}

IMPL_LINK_TYPED(OGLWindow, CameraHandler, VclWindowEvent&, rEvent, void)
{
    if( rEvent.GetId() == VCLEVENT_WINDOW_KEYINPUT )
    {
        KeyEvent* pKeyEvt = static_cast<KeyEvent*>(rEvent.GetData());
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
                    gltf_get_camera_pos(&m_rHandle, &vEye,&vView,&vUp);
                    float fModelSize =(float)gltf_get_model_size(&m_rHandle);

                    glm::vec3 vMove = vView-vEye;
                    vMove = glm::normalize(vMove);
                    vMove *= 25.0f;
                    glm::vec3 vStrafe = glm::cross(vMove, vUp);
                    vStrafe = glm::normalize(vStrafe);
                    vStrafe *= 25.0f;
                    glm::vec3 vMup = vUp * 25.0f;

                    if( !m_bIsOrbitMode )
                    {
                        if(nCode == KEY_E)vMoveBy += vMup*(0.0005f*fModelSize);
                        if(nCode == KEY_Q)vMoveBy -= vMup*(0.0005f*fModelSize);
                        if(nCode == KEY_W)vMoveBy += vMove*(0.0005f*fModelSize);
                        if(nCode == KEY_S)vMoveBy -= vMove*(0.0005f*fModelSize);
                        if(nCode == KEY_A)vMoveBy -= vStrafe*(0.0005f*fModelSize);
                        if(nCode == KEY_D)vMoveBy += vStrafe*(0.0005f*fModelSize);
                    }
                    else
                    {
                        bool bZoomIn = false;
                        bool bZoomOut = false;
                        if(nCode == KEY_E)
                        {
                            vMoveBy += vMove*(0.0005f*fModelSize);
                            bZoomIn = true;
                        }
                        if(nCode == KEY_Q)
                        {
                            vMoveBy -= vMove*(0.0005f*fModelSize);
                            bZoomOut = true;
                        }

                        // Limit zooming in orbit mode
                        float fCameraDistFromModelGlobe = glm::length(vEye + vMoveBy - vView) - fModelSize / 2.0f;
                        if ((fCameraDistFromModelGlobe < 0.5 * fModelSize && bZoomIn ) ||
                            (fCameraDistFromModelGlobe > 2 * fModelSize && bZoomOut ))
                        {
                            vMoveBy = glm::vec3(0.0);
                        }
                    }
                }
                gltf_renderer_move_camera(&m_rHandle, vMoveBy.x, vMoveBy.y, vMoveBy.z, 0.0001);

                if( m_bIsOrbitMode )
                {
                    long nDeltaX = 0;
                    long nDeltaY = 0;
                    if (nCode == KEY_W)
                    {
                        nDeltaY -= 1;
                    }
                    if (nCode == KEY_S)
                    {
                        nDeltaY += 1;
                    }
                    if (nCode == KEY_A)
                    {
                        nDeltaX -= 1;
                    }
                    if (nCode == KEY_D)
                    {
                        nDeltaX += 1;
                    }
                    float fSensitivity = 50.0;
                    gltf_renderer_rotate_model(&m_rHandle, nDeltaX*fSensitivity, nDeltaY*fSensitivity, 0.0);
                }
            }
            else if(nCode == KEY_M)
            {
                if(m_bIsOrbitMode)
                {
                    gltf_orbit_mode_stop(&m_rHandle);
                    m_bIsOrbitMode = false;
                }
                else
                {
                    gltf_orbit_mode_start(&m_rHandle);
                    m_bIsOrbitMode = true;
                }
            }
            else if(nCode == KEY_F)
            {
                gltf_render_FPS_enable(&m_rHandle);
            }
        }
    }
    else if( rEvent.GetId() == VCLEVENT_WINDOW_MOUSEBUTTONDOWN )
    {
        MouseEvent* pMouseEvt = static_cast<MouseEvent*>(rEvent.GetData());
        if(pMouseEvt && pMouseEvt->IsLeft() && pMouseEvt->GetClicks() == 1)
        {
            m_aLastMousePos = pMouseEvt->GetPosPixel();
        }
    }
    else if( rEvent.GetId() == VCLEVENT_WINDOW_MOUSEMOVE )
    {
        if ( !m_rEventHandler.HasFocus() )
        {
            m_rEventHandler.GrabFocus();
        }
        MouseEvent* pMouseEvt = static_cast<MouseEvent*>(rEvent.GetData());
        if(pMouseEvt && pMouseEvt->IsLeft() && m_aLastMousePos != Point(0,0))
        {
            const Point& aCurPos = pMouseEvt->GetPosPixel();
            float fSensitivity = std::min(m_rHandle.viewport.width, m_rHandle.viewport.height);
            if (fSensitivity == 0.0)
                fSensitivity = 1.0;
            else
                fSensitivity = 540.0 / fSensitivity;


            long nDeltaX = m_aLastMousePos.X()-aCurPos.X();
            long nDeltaY = m_aLastMousePos.Y()-aCurPos.Y();
            if( m_bIsOrbitMode )
            {
                fSensitivity *= 5;
                gltf_renderer_rotate_model(&m_rHandle, (float)nDeltaX*fSensitivity, (float)nDeltaY*fSensitivity, 0.0);
            }
            else
            {
                // Filter out too small deltas to avoid rewrite rotation parameter with 0
                // before rotation is done
                if( nDeltaX != 0 || nDeltaY != 0 )
                    gltf_renderer_rotate_camera(&m_rHandle, (float)nDeltaX*fSensitivity, (float)nDeltaY*fSensitivity, 0.0);
            }
            m_aLastMousePos = aCurPos;
        }
    }
    else if( rEvent.GetId() == VCLEVENT_WINDOW_MOUSEBUTTONUP )
    {
        MouseEvent* pMouseEvt = static_cast<MouseEvent*>(rEvent.GetData());
        if(pMouseEvt && pMouseEvt->IsLeft() && pMouseEvt->GetClicks() == 1)
        {
            m_aLastMousePos = Point(0,0);
        }
    }
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
