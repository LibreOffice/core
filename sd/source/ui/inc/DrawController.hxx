/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawController.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 14:16:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_DRAW_CONTROLLER_HXX
#define SD_DRAW_CONTROLLER_HXX

#include "ViewShell.hxx"

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
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#include <tools/weakbase.hxx>
#include <memory>
#include <vector>

class SfxViewShell;
class SdXImpressDocument;

namespace {

typedef ::cppu::ImplInheritanceHelper4 <
    SfxBaseController,
    ::com::sun::star::view::XSelectionSupplier,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::drawing::XDrawView,
    ::com::sun::star::view::XSelectionChangeListener
    > DrawControllerInterfaceBase;

class BroadcastHelperOwner
{
public:
    BroadcastHelperOwner (::osl::Mutex& rMutex) : maBroadcastHelper(rMutex) {};
    ::cppu::OBroadcastHelper maBroadcastHelper;
};


} // end of anonymous namespace.


namespace sd {

class DrawSubController;
class ViewShellBase;
class ViewShell;
class View;


/** The DrawController is the UNO controller for Impress and Draw.  It
    relies objects that implement the DrawSubController interface for view
    specific behaviour.  The life time of the DrawController is roughly that
    of ViewShellBase.
*/
class DrawController
    : public DrawControllerInterfaceBase,
      private BroadcastHelperOwner,
      public ::cppu::OPropertySetHelper
{
public:
    enum properties
    {
        PROPERTY__BEGIN = 0,
        PROPERTY_WORKAREA = PROPERTY__BEGIN,
        PROPERTY_CURRENTPAGE,
        PROPERTY_MASTERPAGEMODE,
        PROPERTY_LAYERMODE,
        PROPERTY_ACTIVE_LAYER,
        PROPERTY_ZOOMTYPE,
        PROPERTY_ZOOMVALUE,
        PROPERTY_VIEWOFFSET,
        PROPERTY__END
    };

    /** Create a new DrawController object for the given ViewShellBase.
    */
    DrawController (ViewShellBase& rBase) throw();

    virtual ~DrawController (void) throw();

    /** Replace the currently used sub controller with the given one.  This
        new sub controller is used from now on for the view (that is the
        main view shell to be precise) specific tasks.  Call this method
        with a suitable sub controller whenever the view shell in the center
        pane is exchanged.
        @param pSubController
            The ViewShell specific sub controller or NULL when (temporarily
            while switching to another one) there is no ViewShell displayed
            in the center pane.
    */
    void SetSubController (::std::auto_ptr<DrawSubController> pSubController);

    ::com::sun::star::awt::Rectangle GetVisArea (void) const;

    /** Call this method when the VisArea has changed.
    */
    void FireVisAreaChanged (const Rectangle& rVisArea) throw();

    /** Call this method when the selection has changed.
    */
    void FireSelectionChangeListener (void) throw();

    /** Call this method when the edit mode has changed.
    */
    void FireChangeEditMode (bool bMasterPageMode) throw();

    /** Call this method when the layer mode has changed.
    */
    void FireChangeLayerMode (bool bLayerMode) throw();

    /** Call this method when there is a new current page.
    */
    void FireSwitchCurrentPage (SdPage* pCurrentPage) throw();

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

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
    /** This method must return the name to index table. This table
        contains all property names and types of this object.
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    virtual void FillPropertyTable (
        ::std::vector< ::com::sun::star::beans::Property>& rProperties);

    /**
     * The same as getFastProperyValue, but return the value through
     * rValue and nHandle is always valid.
     */
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const;

    /** Convert the value rValue and return the result in rConvertedValue and the
        old value in rOldValue.
        After this call the vetoable listeners are notified.

        @param rConvertedValue
            The converted value. Only set if return is true.
        @param rOldValue
            The old value. Only set if return is true.
        @param nHandle
            The handle of the proberty.
        @return
            <TRUE/> if the value is converted successfully.
        @throws IllegalArgumentException
     */
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);

    /** The same as setFastProperyValue, but no exception is thrown and nHandle
        is always valid. You must not broadcast the changes in this method.
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

    Rectangle maLastVisArea;
    ::tools::WeakReference<SdrPage> mpCurrentPage;
    bool mbMasterPageMode;
    bool mbLayerMode;

    /** This flag indicates whether the called DrawController is being
        disposed or already has been disposed.
    */
    bool mbDisposing;

    ::std::auto_ptr< ::cppu::IPropertyArrayHelper> mpPropertyArrayHelper;

    /** The current sub controller.  May be NULL.
    */
    ::std::auto_ptr<DrawSubController> mpSubController;

    /** Send an event to all relevant property listeners that a
        property has changed its value.  The fire() method of the
        OPropertySetHelper is wrapped by this method to handle
        exceptions thrown by called listeners.
    */
    void FirePropertyChange (
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue);
};

} // end of namespace sd

#endif
