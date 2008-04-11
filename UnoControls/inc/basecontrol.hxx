/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basecontrol.hxx,v $
 * $Revision: 1.7 $
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

#define UNO3_ANY                                    ::com::sun::star::uno::Any
#define UNO3_EVENTOBJECT                            ::com::sun::star::lang::EventObject
#define UNO3_MUTEX                                  ::osl::Mutex
#define UNO3_OCOMPONENTHELPER                       ::cppu::OComponentHelper
#define UNO3_OUSTRING                               ::rtl::OUString
#define UNO3_PAINTEVENT                             ::com::sun::star::awt::PaintEvent
#define UNO3_RECTANGLE                              ::com::sun::star::awt::Rectangle
#define UNO3_REFERENCE                              ::com::sun::star::uno::Reference
#define UNO3_RUNTIMEEXCEPTION                       ::com::sun::star::uno::RuntimeException
#define UNO3_SEQUENCE                               ::com::sun::star::uno::Sequence
#define UNO3_SIZE                                   ::com::sun::star::awt::Size
#define UNO3_TYPE                                   ::com::sun::star::uno::Type
#define UNO3_WINDOWDESCRIPTOR                       ::com::sun::star::awt::WindowDescriptor
#define UNO3_XCONTROL                               ::com::sun::star::awt::XControl
#define UNO3_XCONTROLMODEL                          ::com::sun::star::awt::XControlModel
#define UNO3_XEVENTLISTENER                         ::com::sun::star::lang::XEventListener
#define UNO3_XFOCUSLISTENER                         ::com::sun::star::awt::XFocusListener
#define UNO3_XGRAPHICS                              ::com::sun::star::awt::XGraphics
#define UNO3_XKEYLISTENER                           ::com::sun::star::awt::XKeyListener
#define UNO3_XMOUSELISTENER                         ::com::sun::star::awt::XMouseListener
#define UNO3_XMOUSEMOTIONLISTENER                   ::com::sun::star::awt::XMouseMotionListener
#define UNO3_XMULTISERVICEFACTORY                   ::com::sun::star::lang::XMultiServiceFactory
#define UNO3_XPAINTLISTENER                         ::com::sun::star::awt::XPaintListener
#define UNO3_XSERVICEINFO                           ::com::sun::star::lang::XServiceInfo
#define UNO3_XTOOLKIT                               ::com::sun::star::awt::XToolkit
#define UNO3_XVIEW                                  ::com::sun::star::awt::XView
#define UNO3_XWINDOW                                ::com::sun::star::awt::XWindow
#define UNO3_XWINDOWLISTENER                        ::com::sun::star::awt::XWindowListener
#define UNO3_XWINDOWPEER                            ::com::sun::star::awt::XWindowPeer
#define UNO3_XINTERFACE                             ::com::sun::star::uno::XInterface

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
    UNO3_MUTEX m_aMutex ;
};

//____________________________________________________________________________________________________________
//  classes
//____________________________________________________________________________________________________________

class BaseControl   : public UNO3_XSERVICEINFO
                    , public UNO3_XPAINTLISTENER
                    , public UNO3_XWINDOWLISTENER
                    , public UNO3_XVIEW
                    , public UNO3_XWINDOW
                    , public UNO3_XCONTROL
                    , public IMPL_MutexContainer
                    , public UNO3_OCOMPONENTHELPER
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

    BaseControl( const UNO3_REFERENCE< UNO3_XMULTISERVICEFACTORY >& xFactory );

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

    virtual UNO3_ANY SAL_CALL queryInterface( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual UNO3_SEQUENCE< UNO3_TYPE > SAL_CALL getTypes() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      get implementation id
        @descr      This ID is neccessary for UNO-caching. If there no ID, cache is disabled.
                    Another way, cache is enabled.

        @seealso    XTypeProvider

        @param      -

        @return     ID as Sequence of byte

        @onerror    A RuntimeException is thrown.
    */

    virtual UNO3_SEQUENCE< sal_Int8 > SAL_CALL getImplementationId() throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL setDelegator( const UNO3_REFERENCE< UNO3_XINTERFACE >& xDelegator ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_ANY SAL_CALL queryAggregation( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual sal_Bool SAL_CALL supportsService( const UNO3_OUSTRING& sServiceName ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_OUSTRING SAL_CALL getImplementationName() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_SEQUENCE< UNO3_OUSTRING > SAL_CALL getSupportedServiceNames() throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL dispose() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addEventListener( const UNO3_REFERENCE< UNO3_XEVENTLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeEventListener( const UNO3_REFERENCE< UNO3_XEVENTLISTENER >& xListener   ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL createPeer( const UNO3_REFERENCE< UNO3_XTOOLKIT >& xToolkit, const UNO3_REFERENCE< UNO3_XWINDOWPEER >& xParent ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setContext( const UNO3_REFERENCE< UNO3_XINTERFACE >& xContext ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL setModel( const UNO3_REFERENCE< UNO3_XCONTROLMODEL >& xModel ) throw( UNO3_RUNTIMEEXCEPTION ) = 0 ;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setDesignMode( sal_Bool bOn ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_REFERENCE< UNO3_XINTERFACE > SAL_CALL getContext() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_REFERENCE< UNO3_XCONTROLMODEL > SAL_CALL getModel() throw( UNO3_RUNTIMEEXCEPTION ) = 0;

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_REFERENCE< UNO3_XWINDOWPEER > SAL_CALL getPeer() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_REFERENCE< UNO3_XVIEW > SAL_CALL getView() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL isDesignMode() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL isTransparent() throw( UNO3_RUNTIMEEXCEPTION );

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
                                        sal_Int16   nFlags  ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setEnable( sal_Bool bEnable ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setFocus() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_RECTANGLE SAL_CALL getPosSize() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addWindowListener( const UNO3_REFERENCE< UNO3_XWINDOWLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addFocusListener( const UNO3_REFERENCE< UNO3_XFOCUSLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addKeyListener( const UNO3_REFERENCE< UNO3_XKEYLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addMouseListener( const UNO3_REFERENCE< UNO3_XMOUSELISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addMouseMotionListener( const UNO3_REFERENCE< UNO3_XMOUSEMOTIONLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addPaintListener( const UNO3_REFERENCE< UNO3_XPAINTLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeWindowListener( const UNO3_REFERENCE< UNO3_XWINDOWLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeFocusListener( const UNO3_REFERENCE< UNO3_XFOCUSLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeKeyListener( const UNO3_REFERENCE< UNO3_XKEYLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeMouseListener( const UNO3_REFERENCE< UNO3_XMOUSELISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeMouseMotionListener( const UNO3_REFERENCE< UNO3_XMOUSEMOTIONLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removePaintListener( const UNO3_REFERENCE< UNO3_XPAINTLISTENER >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

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
                                sal_Int32   nY  ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL setGraphics( const UNO3_REFERENCE< UNO3_XGRAPHICS >& xDevice ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL setZoom(  float   fZoomX  ,
                                    float   fZoomY  ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_REFERENCE< UNO3_XGRAPHICS > SAL_CALL getGraphics() throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual UNO3_SIZE SAL_CALL getSize() throw( UNO3_RUNTIMEEXCEPTION );

    //________________________________________________________________________________________________________
    //  UNO3_XEVENTLISTENER
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL disposing( const UNO3_EVENTOBJECT& rSource    ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL windowPaint( const UNO3_PAINTEVENT& rEvent ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL windowResized( const UNO3_WINDOWEVENT& aEvent ) throw( UNO3_RUNTIMEEXCEPTION );
    virtual void SAL_CALL windowMoved( const UNO3_WINDOWEVENT& aEvent ) throw( UNO3_RUNTIMEEXCEPTION );
    virtual void SAL_CALL windowShown( const UNO3_EVENTOBJECT& aEvent ) throw( UNO3_RUNTIMEEXCEPTION );
    virtual void SAL_CALL windowHidden( const UNO3_EVENTOBJECT& aEvent ) throw( UNO3_RUNTIMEEXCEPTION );

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

    static const UNO3_SEQUENCE< UNO3_OUSTRING > impl_getStaticSupportedServiceNames();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    static const UNO3_OUSTRING impl_getStaticImplementationName();

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

    const UNO3_REFERENCE< UNO3_XMULTISERVICEFACTORY > impl_getMultiServiceFactory();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    const UNO3_REFERENCE< UNO3_XWINDOW > impl_getPeerWindow();

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    const UNO3_REFERENCE< UNO3_XGRAPHICS > impl_getGraphicsPeer();

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

    virtual UNO3_WINDOWDESCRIPTOR* impl_getWindowDescriptor( const UNO3_REFERENCE< UNO3_XWINDOWPEER >& xParentPeer );

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
                            const   UNO3_REFERENCE< UNO3_XGRAPHICS >&   xGraphics   );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void impl_recalcLayout( const UNO3_WINDOWEVENT& aEvent );

    /**_______________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    UNO3_REFERENCE< UNO3_XINTERFACE > impl_getDelegator();

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

    UNO3_REFERENCE< UNO3_XMULTISERVICEFACTORY >     m_xFactory              ;
    UNO3_REFERENCE< UNO3_XINTERFACE >               m_xDelegator            ;
    OMRCListenerMultiplexerHelper*                  m_pMultiplexer          ;   // multiplex events
    UNO3_REFERENCE< UNO3_XINTERFACE >               m_xMultiplexer          ;
    UNO3_REFERENCE< UNO3_XINTERFACE >               m_xContext              ;
    UNO3_REFERENCE< UNO3_XWINDOWPEER >              m_xPeer                 ;
    UNO3_REFERENCE< UNO3_XWINDOW >                  m_xPeerWindow           ;
    UNO3_REFERENCE< UNO3_XGRAPHICS >                m_xGraphicsView         ;   // graphics for ::com::sun::star::awt::XView-operations
    UNO3_REFERENCE< UNO3_XGRAPHICS >                m_xGraphicsPeer         ;   // graphics for painting on a peer
    sal_Int32                                       m_nX                    ;   // Position ...
    sal_Int32                                       m_nY                    ;
    sal_Int32                                       m_nWidth                ;   // ... and size of window
    sal_Int32                                       m_nHeight               ;
    sal_Bool                                        m_bVisible              ;   // Some state flags
    sal_Bool                                        m_bInDesignMode         ;
    sal_Bool                                        m_bEnable               ;

};  // class BaseControl

}   // namespace unocontrols

#endif  // ifndef _UNOCONTROLS_BASECONTROL_CTRL_HXX
