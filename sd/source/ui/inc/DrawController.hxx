/*************************************************************************
 *
 *  $RCSfile: DrawController.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:53:56 $
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

#ifndef SD_DRAW_CONTROLLER_HXX
#define SD_DRAW_CONTROLLER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _SFX_SFXBASECONTROLLER_HXX_
#include <sfx2/sfxbasecontroller.hxx>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#include <memory>
#include <vector>

class SfxViewShell;
class SdXImpressDocument;

namespace {

typedef ::cppu::ImplHelper5 <
    ::com::sun::star::view::XSelectionSupplier,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::drawing::XDrawView,
    ::com::sun::star::awt::XWindow,
    ::com::sun::star::view::XSelectionChangeListener
    > DrawControllerInterfaceBase;

} // end of anonymous namespace.


namespace sd {

class DrawSubController;
class ViewShellBase;
class ViewShell;
class View;

/** Main controller for all sub-shells that are stacked on a single
    view shell.

    <p>With the stacked sub-shells it is not possible to have one
    controller for all sub-shell.  Instead there is this generic
    controller which handles general requests regarding dispatching
    and selection.  It concentrates on the currently active/focused
    sub-shell.  In order to offer more specific access to each of the
    sub-shells associated whith one of the panes of a multi-pane GUI,
    it provides access to sub-controllers.  Each sub-controller
    represents a single sub-shell.  So, if you want to have a slot
    dispatched or want to manipulate the selection of a specific pane,
    you get the sub-controller of that pane and use that as
    controller.</p>
*/
class DrawController
    : public SfxBaseController,
      public ::cppu::OBroadcastHelper,
      public ::cppu::OPropertySetHelper,
      public DrawControllerInterfaceBase
{
public:
    enum properties
    {
        PROPERTY__BEGIN = 0,
        PROPERTY_WORKAREA = PROPERTY__BEGIN,
        PROPERTY__END
    };

    DrawController (
        ViewShellBase& rBase,
        ViewShell& rViewShell,
        View& rView) throw();
    virtual ~DrawController (void) throw();

    ::com::sun::star::awt::Rectangle GetVisArea (void) const;

    virtual void FireVisAreaChanged (const Rectangle& rVisArea) throw();

    /** Call this method to tell the sub-controller that the selection
        has changed.
    */
    virtual void FireSelectionChangeListener (void) throw();

    DECLARE_XINTERFACE();
    DECLARE_XTYPEPROVIDER();

    // XComponent
    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);


    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // XWindow
    virtual void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool Visible ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEnable( sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);


    // XDrawView
    virtual void SAL_CALL
        setCurrentPage (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage >& xPage)
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XDrawPage > SAL_CALL
        getCurrentPage (void)
        throw(::com::sun::star::uno::RuntimeException);


    // lang::XEventListener
    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


    // view::XSelectionChangeListener
    virtual void  SAL_CALL
        selectionChanged (const ::com::sun::star::lang::EventObject& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    View& mrView;

    ViewShell& mrViewShell;

    Rectangle maLastVisArea;

    /** This method must return the name to index table. This table
        contains all property names and types of this object.
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    /** Send an event to all relevant property listeners that a
        property has changed its value.  The fire() method of the
        OPropertySetHelper is wrapped by this method to handle
        exceptions thrown by called listeners.
    */
    void FirePropertyChange (
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue);

    SdXImpressDocument* GetModel (void) const throw();

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::Property>& GetPropertyTable (void);
    virtual void FillPropertyTable (
        ::std::vector< ::com::sun::star::beans::Property>& rProperties);

    /**
     * The same as getFastProperyValue, but return the value through
     * rValue and nHandle is always valid.
     */
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const;

    /**
     * Converted the value rValue and return the result in rConvertedValue and the
     * old value in rOldValue. A IllegalArgumentException is thrown.
     * The method is not implemented in this class. After this call the vetoable
     * listeners are notified.
     *
     * @param rConvertedValue the converted value. Only set if return is true.
     * @param rOldValue the old value. Only set if return is true.
     * @param nHandle the handle of the proberty.
     * @return true if the value converted.
     */
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);

    /**
     * The same as setFastProperyValue, but no exception is thrown and nHandle
     * is always valid. You must not broadcast the changes in this method.<BR>
     * <B>You type is correct you need't test it.</B>
     */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    /** When the called object has been disposed already this method throws
        a Disposed exception and does not return.
    */
    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);

private:
    ViewShellBase& mrBase;

    /** This flag indicates that an object is either disposing or has
        already been disposed.
    */
    bool mbDisposing;

    ::std::auto_ptr< ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::Property> > mpProperties;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>
        GetWindow (void);
};

} // end of namespace sd

#endif
