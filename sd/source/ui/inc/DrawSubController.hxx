#ifndef SD_DRAW_SUB_CONTROLLER_HXX
#define SD_DRAW_SUB_CONTROLLER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
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
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif


class SdPage;
class SdXImpressDocument;
class Window;

namespace {

class DrawSubControllerMutexOwner
{
protected:
    osl::Mutex maMutex;
};

typedef ::cppu::WeakImplHelper4 <
    ::com::sun::star::view::XSelectionSupplier,
    ::com::sun::star::drawing::XDrawView,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::awt::XWindow
    > DrawSubControllerInterfaceBase;

} // end of anonymous namespace.


namespace sd {

class View;
class ViewShell;

class DrawSubController
    : public DrawSubControllerMutexOwner,
      public ::cppu::OBroadcastHelper,
      public ::cppu::OPropertySetHelper,
      public DrawSubControllerInterfaceBase
{
public:
    DrawSubController (::sd::View& rView, ::sd::ViewShell& rViewShell) throw();
    virtual ~DrawSubController (void) throw();

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
        getWindow (void);

    ::com::sun::star::awt::Rectangle GetVisArea (void) const;

    /** Call this method to tell the sub-controller that the vis area
        has changed.
    */
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

    // XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& URL, const ::rtl::OUString& TargetFrameName, sal_Int32 SearchFlags ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& Requests ) throw (::com::sun::star::uno::RuntimeException);

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // XDrawView
    virtual void SAL_CALL setCurrentPage( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getCurrentPage(  ) throw(::com::sun::star::uno::RuntimeException);

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

protected:
    View& mrView;

    ViewShell& mrViewShell;

    /** This flag indicates that an object is either disposing or has
        already been disposed.
    */
    bool mbDisposing;

    /** This method must return the name to index table. This table
        contains all property names and types of this object.
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

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
    /**
     * The same as getFastProperyValue, but return the value through
     * rValue and nHandle is always valid.
     */
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const;

    SdXImpressDocument* getModel() const throw();

    /** Send an event to all relevant property listeners that a
        property has changed its value.  The fire() method of the
        OPropertySetHelper is wrapped by this method to handle
        exceptions thrown by called listeners.
    */
    void FirePropertyChange (
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue);

protected:
    Rectangle maLastVisArea;
};

} // end of namespace sd

#endif
