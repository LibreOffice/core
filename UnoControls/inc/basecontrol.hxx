/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_UNOCONTROLS_INC_BASECONTROL_HXX
#define INCLUDED_UNOCONTROLS_INC_BASECONTROL_HXX

#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/FocusEvent.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XActivateListener.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/PaintEvent.hpp>
#include <com/sun/star/awt/InputEvent.hpp>
#include <com/sun/star/awt/KeyGroup.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/KeyFunction.hpp>
#include <com/sun/star/awt/FocusChangeReason.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XFileDialog.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XProgressMonitor.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XMessageBox.hpp>
#include <com/sun/star/awt/XTextEditField.hpp>
#include <com/sun/star/awt/Style.hpp>
#include <com/sun/star/awt/XTimeField.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XSpinField.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/awt/XNumericField.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XTextArea.hpp>
#include <com/sun/star/awt/XImageButton.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XCurrencyField.hpp>
#include <com/sun/star/awt/XPatternField.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/XTabController.hpp>
#include <com/sun/star/awt/XVclContainer.hpp>
#include <com/sun/star/awt/XDateField.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XProgressBar.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/component.hxx>

#include "multiplexer.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

//  "namespaces"

namespace unocontrols{

//  macros

#define TRGB_COLORDATA(TRANSPARENCE,RED,GREEN,BLUE) \
        ((sal_Int32)(((sal_uInt32)((sal_uInt8)(BLUE))))|(((sal_uInt32)((sal_uInt8)(GREEN)))<<8)|(((sal_uInt32)((sal_uInt8)(RED)))<<16)|(((sal_uInt32)((sal_uInt8)(TRANSPARENCE)))<<24))

//  structs

struct IMPL_MutexContainer
{
    // Is necassery to initialize "BaseControl" and make this class thread-safe.
    ::osl::Mutex m_aMutex;
};

class BaseControl   : public ::com::sun::star::lang::XServiceInfo
                    , public ::com::sun::star::awt::XPaintListener
                    , public ::com::sun::star::awt::XWindowListener
                    , public ::com::sun::star::awt::XView
                    , public ::com::sun::star::awt::XWindow
                    , public ::com::sun::star::awt::XControl
                    , public IMPL_MutexContainer
                    , public ::cppu::OComponentHelper
{

//  public methods

public:

    //  construct/destruct

    /**_______________________________________________________________________________________________________
    */

    BaseControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    /**_______________________________________________________________________________________________________
    */

    virtual ~BaseControl();

    //  XInterface

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() override;

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() override;

    //  XTypeProvider

    /**_______________________________________________________________________________________________________
        @short      get information about supported interfaces
        @seealso    XTypeProvider
        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
        @short      get implementation id
        @descr      This ID is necessary for UNO-caching. If there no ID, cache is disabled.
                    Another way, cache is enabled.

        @seealso    XTypeProvider
        @return     ID as Sequence of byte

        @onerror    A RuntimeException is thrown.
    */

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XAggregation

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setDelegator(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDelegator
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XServiceInfo

    /**_______________________________________________________________________________________________________
    */

    virtual sal_Bool SAL_CALL supportsService(
        const OUString& sServiceName
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XComponent

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL addEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL removeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XControl

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL createPeer(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& xToolkit,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setContext(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xContext
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual sal_Bool SAL_CALL setModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override = 0;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setDesignMode( sal_Bool bOn ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getContext()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override = 0;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual sal_Bool SAL_CALL isDesignMode() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual sal_Bool SAL_CALL isTransparent() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XWindow

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setEnable( sal_Bool bEnable ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setFocus() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL addWindowListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL addFocusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL addKeyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener )
    throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL addMouseListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL addMouseMotionListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL addPaintListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL removeWindowListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL removeFocusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL removeKeyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL removeMouseListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL removeMouseMotionListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL removePaintListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XView

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL draw( sal_Int32   nX  ,
                                sal_Int32   nY  ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual sal_Bool SAL_CALL setGraphics(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xDevice
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL setZoom(  float   fZoomX  ,
                                    float   fZoomY  ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  ::com::sun::star::lang::XEventListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& rSource
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XPaintListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowPaint(
        const ::com::sun::star::awt::PaintEvent& rEvent
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XWindowListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  impl but public method to register service

    /**_______________________________________________________________________________________________________
    */

    static const ::com::sun::star::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();

    /**_______________________________________________________________________________________________________
    */

    static const OUString impl_getStaticImplementationName();

//  protected methods

protected:
    using OComponentHelper::disposing;

    /**_______________________________________________________________________________________________________
    */

    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > impl_getComponentContext() { return m_xComponentContext;}

    /**_______________________________________________________________________________________________________
    */

    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > impl_getPeerWindow() { return m_xPeerWindow;}

    /**_______________________________________________________________________________________________________
    */

    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > impl_getGraphicsPeer() { return m_xGraphicsPeer;}

    /**_______________________________________________________________________________________________________
    */

    const sal_Int32& impl_getWidth() { return m_nWidth;}

    /**_______________________________________________________________________________________________________
    */

    const sal_Int32& impl_getHeight() { return m_nHeight;}

    /**_______________________________________________________________________________________________________
    */

    virtual ::com::sun::star::awt::WindowDescriptor* impl_getWindowDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParentPeer
    );

    /**_______________________________________________________________________________________________________
    */

    virtual void impl_paint(        sal_Int32               nX          ,
                                    sal_Int32               nY          ,
                            const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >&   xGraphics   );

    /**_______________________________________________________________________________________________________
    */

    virtual void impl_recalcLayout( const ::com::sun::star::awt::WindowEvent& aEvent );

    /**_______________________________________________________________________________________________________
    */

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > impl_getDelegator() { return m_xDelegator;}

//  private methods

private:

    /**_______________________________________________________________________________________________________
    */

    void impl_releasePeer();

    /**_______________________________________________________________________________________________________
    */

    OMRCListenerMultiplexerHelper* impl_getMultiplexer();

//  private variables

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xComponentContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xDelegator;
    OMRCListenerMultiplexerHelper*                  m_pMultiplexer;   // multiplex events
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xMultiplexer;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >              m_xPeer;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                  m_xPeerWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >                m_xGraphicsView;   // graphics for ::com::sun::star::awt::XView-operations
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >                m_xGraphicsPeer;   // graphics for painting on a peer
    sal_Int32                                       m_nX;   // Position ...
    sal_Int32                                       m_nY;
    sal_Int32                                       m_nWidth;   // ... and size of window
    sal_Int32                                       m_nHeight;
    bool                                        m_bVisible;   // Some state flags
    bool                                        m_bInDesignMode;
    bool                                        m_bEnable;

};  // class BaseControl

}   // namespace unocontrols

#endif // INCLUDED_UNOCONTROLS_INC_BASECONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
