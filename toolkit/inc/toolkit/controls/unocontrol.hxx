/*************************************************************************
 *
 *  $RCSfile: unocontrol.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#define _TOOLKIT_CONTROLS_UNOCONTROL_HXX_


#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XView_HPP_
#include <com/sun/star/awt/XView.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>


struct UnoControlComponentInfos
{
    sal_Bool    bVisible;
    sal_Bool    bEnable;
    long        nX, nY, nWidth, nHeight;
    sal_uInt16  nFlags;
    float       nZoomX, nZoomY;

    UnoControlComponentInfos()
    {
        bVisible = sal_True;
        bEnable = sal_True;
        nX = nY = nWidth = nHeight = 0;
        nFlags = 0; // POSSIZE_POSSIZE;
        nZoomX = nZoomY = 1.0f;
    }
};

//  ----------------------------------------------------
//  class UnoControl
//  ----------------------------------------------------
class UnoControl :  public ::com::sun::star::awt::XControl,
                    public ::com::sun::star::awt::XWindow,
                    public ::com::sun::star::awt::XView,
                    public ::com::sun::star::beans::XPropertiesChangeListener,
                    public ::com::sun::star::lang::XServiceInfo,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::cppu::OWeakAggObject
{
private:
    ::osl::Mutex    maMutex;

protected:
    EventListenerMultiplexer        maDisposeListeners;
    WindowListenerMultiplexer       maWindowListeners;
    FocusListenerMultiplexer        maFocusListeners;
    KeyListenerMultiplexer          maKeyListeners;
    MouseListenerMultiplexer        maMouseListeners;
    MouseMotionListenerMultiplexer  maMouseMotionListeners;
    PaintListenerMultiplexer        maPaintListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >      mxPeer;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >       mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >    mxModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >        mxGraphics;

    sal_Bool                            mbDisposePeer;
    sal_Bool                            mbUpdatingModel;
    sal_Bool                            mbRefeshingPeer;
    sal_Bool                            mbDesignMode;

    UnoControlComponentInfos            maComponentInfos;

    ::osl::Mutex&                                                               GetMutex() { return maMutex; }

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >          getParentPeer() const;
    void                                                                        updateFromModel();
    sal_Bool                                                                    IsUpdatingModel() const { return mbUpdatingModel; }
    void                                                                        StartUpdatingModel()    { mbUpdatingModel = sal_True; }
    void                                                                        EndUpdatingModel()      { mbUpdatingModel = sal_False; }
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >      ImplGetCompatiblePeer( sal_Bool bAcceptExistingPeer );
    virtual void                                                                ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );
    virtual void                                                                PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );

public:
                UnoControl();
                ~UnoControl();

    virtual ::rtl::OUString GetComponentServiceName();

    // ::com::sun::star::uno::XAggregation
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakAggObject::release(); }

    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::beans::XPropertiesChangeListener
    void SAL_CALL propertiesChange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setVisible( sal_Bool Visible ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setEnable( sal_Bool Enable ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFocus(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XView
    sal_Bool SAL_CALL setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& aDevice ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setZoom( float fZoomX, float fZoomY ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    void SAL_CALL setContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Context ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getContext(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};




#endif // _TOOLKIT_CONTROLS_UNOCONTROL_HXX_

