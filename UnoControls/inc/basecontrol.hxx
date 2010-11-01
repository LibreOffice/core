/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UNOCONTROLS_BASECONTROL_CTRL_HXX
#define _UNOCONTROLS_BASECONTROL_CTRL_HXX

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

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
#include <com/sun/star/awt/MessageBoxCommand.hpp>
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

//____________________________________________________________________________________________________________
//  includes of my project
//____________________________________________________________________________________________________________
#include "multiplexer.hxx"

//____________________________________________________________________________________________________________
//  "namespaces"
//____________________________________________________________________________________________________________

namespace unocontrols{

#define CSS_UNO     ::com::sun::star::uno
#define CSS_AWT     ::com::sun::star::awt
#define CSS_LANG    ::com::sun::star::lang

//____________________________________________________________________________________________________________
//  macros
//____________________________________________________________________________________________________________

#define TRGB_COLORDATA(TRANSPARENCE,RED,GREEN,BLUE) \
        ((sal_Int32)(((sal_uInt32)((sal_uInt8)(BLUE))))|(((sal_uInt32)((sal_uInt8)(GREEN)))<<8)|(((sal_uInt32)((sal_uInt8)(RED)))<<16)|(((sal_uInt32)((sal_uInt8)(TRANSPARENCE)))<<24))

//____________________________________________________________________________________________________________
//  structs
//____________________________________________________________________________________________________________

struct IMPL_MutexContainer
{
    // Is necassery to initialize "BaseControl" and make this class thread-safe.
    ::osl::Mutex m_aMutex ;
};

//____________________________________________________________________________________________________________
//  classes
//____________________________________________________________________________________________________________

class BaseControl   : public CSS_LANG::XServiceInfo
                    , public CSS_AWT::XPaintListener
                    , public CSS_AWT::XWindowListener
                    , public CSS_AWT::XView
                    , public CSS_AWT::XWindow
                    , public CSS_AWT::XControl
                    , public IMPL_MutexContainer
                    , public ::cppu::OComponentHelper
{
//____________________________________________________________________________________________________________
//  public methods
//____________________________________________________________________________________________________________

public:

    //________________________________________________________________________________________________________
    //  construct/destruct
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    BaseControl( const CSS_UNO::Reference< CSS_LANG::XMultiServiceFactory >& xFactory );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual ~BaseControl();

    //________________________________________________________________________________________________________
    //  XInterface
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual CSS_UNO::Any SAL_CALL queryInterface(
        const CSS_UNO::Type& aType
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw();

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw();

    //________________________________________________________________________________________________________
    //  XTypeProvider
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      get information about supported interfaces
        @descr      -

        @seealso    XTypeProvider

        @param      -

        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual CSS_UNO::Sequence< CSS_UNO::Type > SAL_CALL getTypes()
        throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      get implementation id
        @descr      This ID is neccessary for UNO-caching. If there no ID, cache is disabled.
                    Another way, cache is enabled.

        @seealso    XTypeProvider

        @param      -

        @return     ID as Sequence of byte

        @onerror    A RuntimeException is thrown.
    */

    virtual CSS_UNO::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XAggregation
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setDelegator(
        const CSS_UNO::Reference< CSS_UNO::XInterface >& xDelegator
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Any SAL_CALL queryAggregation(
        const CSS_UNO::Type& aType
    ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XServiceInfo
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL supportsService(
        const ::rtl::OUString& sServiceName
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XComponent
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL dispose() throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addEventListener(
        const CSS_UNO::Reference< CSS_LANG::XEventListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeEventListener(
        const CSS_UNO::Reference< CSS_LANG::XEventListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XControl
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL createPeer(
        const CSS_UNO::Reference< CSS_AWT::XToolkit >& xToolkit,
        const CSS_UNO::Reference< CSS_AWT::XWindowPeer >& xParent
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setContext(
        const CSS_UNO::Reference< CSS_UNO::XInterface >& xContext
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL setModel(
        const CSS_UNO::Reference< CSS_AWT::XControlModel >& xModel
    ) throw( CSS_UNO::RuntimeException ) = 0 ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setDesignMode( sal_Bool bOn ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Reference< CSS_UNO::XInterface > SAL_CALL getContext()
        throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Reference< CSS_AWT::XControlModel > SAL_CALL getModel()
        throw( CSS_UNO::RuntimeException ) = 0;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Reference< CSS_AWT::XWindowPeer > SAL_CALL getPeer()
        throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Reference< CSS_AWT::XView > SAL_CALL getView()
        throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL isDesignMode() throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL isTransparent() throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XWindow
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setEnable( sal_Bool bEnable ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setFocus() throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_AWT::Rectangle SAL_CALL getPosSize() throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addWindowListener(
        const CSS_UNO::Reference< CSS_AWT::XWindowListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addFocusListener(
        const CSS_UNO::Reference< CSS_AWT::XFocusListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addKeyListener(
        const CSS_UNO::Reference< CSS_AWT::XKeyListener >& xListener )
    throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addMouseListener(
        const CSS_UNO::Reference< CSS_AWT::XMouseListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addMouseMotionListener(
        const CSS_UNO::Reference< CSS_AWT::XMouseMotionListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addPaintListener(
        const CSS_UNO::Reference< CSS_AWT::XPaintListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeWindowListener(
        const CSS_UNO::Reference< CSS_AWT::XWindowListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeFocusListener(
        const CSS_UNO::Reference< CSS_AWT::XFocusListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeKeyListener(
        const CSS_UNO::Reference< CSS_AWT::XKeyListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeMouseListener(
        const CSS_UNO::Reference< CSS_AWT::XMouseListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeMouseMotionListener(
        const CSS_UNO::Reference< CSS_AWT::XMouseMotionListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removePaintListener(
        const CSS_UNO::Reference< CSS_AWT::XPaintListener >& xListener
    ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XView
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL draw( sal_Int32   nX  ,
                                sal_Int32   nY  ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL setGraphics(
        const CSS_UNO::Reference< CSS_AWT::XGraphics >& xDevice
    ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setZoom(  float   fZoomX  ,
                                    float   fZoomY  ) throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_UNO::Reference< CSS_AWT::XGraphics > SAL_CALL getGraphics()
        throw( CSS_UNO::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_AWT::Size SAL_CALL getSize() throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  CSS_LANG::XEventListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL disposing(
        const CSS_LANG::EventObject& rSource
    ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XPaintListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowPaint(
        const CSS_AWT::PaintEvent& rEvent
    ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  XWindowListener
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL windowResized( const CSS_AWT::WindowEvent& aEvent ) throw( CSS_UNO::RuntimeException );
    virtual void SAL_CALL windowMoved( const CSS_AWT::WindowEvent& aEvent ) throw( CSS_UNO::RuntimeException );
    virtual void SAL_CALL windowShown( const CSS_LANG::EventObject& aEvent ) throw( CSS_UNO::RuntimeException );
    virtual void SAL_CALL windowHidden( const CSS_LANG::EventObject& aEvent ) throw( CSS_UNO::RuntimeException );

    //________________________________________________________________________________________________________
    //  impl but public method to register service
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    static const CSS_UNO::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    static const ::rtl::OUString impl_getStaticImplementationName();

//____________________________________________________________________________________________________________
//  protected methods
//____________________________________________________________________________________________________________

protected:
    using OComponentHelper::disposing;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    const CSS_UNO::Reference< CSS_LANG::XMultiServiceFactory > impl_getMultiServiceFactory();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    const CSS_UNO::Reference< CSS_AWT::XWindow > impl_getPeerWindow();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    const CSS_UNO::Reference< CSS_AWT::XGraphics > impl_getGraphicsPeer();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    const sal_Int32& impl_getWidth();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    const sal_Int32& impl_getHeight();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual CSS_AWT::WindowDescriptor* impl_getWindowDescriptor(
        const CSS_UNO::Reference< CSS_AWT::XWindowPeer >& xParentPeer
    );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void impl_paint(        sal_Int32               nX          ,
                                    sal_Int32               nY          ,
                            const   CSS_UNO::Reference< CSS_AWT::XGraphics >&   xGraphics   );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void impl_recalcLayout( const CSS_AWT::WindowEvent& aEvent );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    CSS_UNO::Reference< CSS_UNO::XInterface > impl_getDelegator();

//____________________________________________________________________________________________________________
//  private methods
//____________________________________________________________________________________________________________

private:

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void impl_releasePeer();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    OMRCListenerMultiplexerHelper* impl_getMultiplexer();

//____________________________________________________________________________________________________________
//  private variables
//____________________________________________________________________________________________________________

private:

    CSS_UNO::Reference< CSS_LANG::XMultiServiceFactory >    m_xFactory              ;
    CSS_UNO::Reference< CSS_UNO::XInterface >               m_xDelegator            ;
    OMRCListenerMultiplexerHelper*                  m_pMultiplexer          ;   // multiplex events
    CSS_UNO::Reference< CSS_UNO::XInterface >               m_xMultiplexer          ;
    CSS_UNO::Reference< CSS_UNO::XInterface >               m_xContext              ;
    CSS_UNO::Reference< CSS_AWT::XWindowPeer >              m_xPeer                 ;
    CSS_UNO::Reference< CSS_AWT::XWindow >                  m_xPeerWindow           ;
    CSS_UNO::Reference< CSS_AWT::XGraphics >                m_xGraphicsView         ;   // graphics for CSS_AWT::XView-operations
    CSS_UNO::Reference< CSS_AWT::XGraphics >                m_xGraphicsPeer         ;   // graphics for painting on a peer
    sal_Int32                                       m_nX                    ;   // Position ...
    sal_Int32                                       m_nY                    ;
    sal_Int32                                       m_nWidth                ;   // ... and size of window
    sal_Int32                                       m_nHeight               ;
    sal_Bool                                        m_bVisible              ;   // Some state flags
    sal_Bool                                        m_bInDesignMode         ;
    sal_Bool                                        m_bEnable               ;

};  // class BaseControl

// The namespace aliaes are only used in this header
#undef CSS_UNO
#undef CSS_AWT
#undef CSS_LANG

}   // namespace unocontrols

#endif  // ifndef _UNOCONTROLS_BASECONTROL_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
