#ifndef SD_DRAW_CONTROLLER_HXX
#define SD_DRAW_CONTROLLER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
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
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

class SfxViewShell;

namespace {

class DrawControllerMutexOwner
{
protected:
    osl::Mutex maMutex;
};

typedef ::cppu::ImplHelper4 <
    ::com::sun::star::view::XSelectionSupplier,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::awt::XWindow,
    ::com::sun::star::view::XSelectionChangeListener
    > DrawControllerInterfaceBase;

} // end of anonymous namespace.


namespace sd {

class DrawSubController;
class ViewShellBase;


/** Main controller for all sub-shells that are stacked onto a single
    view shell.

    <p>With the stacked sub-shells it is not possible to have one
    controller for every sub-shell.  Instead there is this generic
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
    : public DrawControllerMutexOwner,
      public ::cppu::OBroadcastHelper,
      public ::cppu::OPropertySetHelper,
      public SfxBaseController,
      public DrawControllerInterfaceBase
{
public:
    DrawController (ViewShellBase& rBase) throw();
    virtual ~DrawController (void) throw();

    /** This method is designed to be called from a
        DrawSubController that has already prepared to Anys to given
        to the broadcast helper.
    */
    void FireVisAreaChanged (
        const ::com::sun::star::uno::Any& rNewVisArea,
        const ::com::sun::star::uno::Any& rOldVisArea) throw();

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
        sal_Int32 nHandle) const;


private:
    ViewShellBase& mrBase;

    /** This flag indicates that an object is either disposing or has
        already been disposed.
    */
    bool mbDisposing;

    /** The currently active sub-controller.  Most of the calls will
        be forwarded to this object.
    */
    DrawSubController* mpSubController;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>
        getWindow (void);

    /** Returns the currently active-subcontroller.  When the member
        variable mpSubController is set then its content is returned.
        Otherwise it is set to the sub-controller of the currently
        active sub-shell.
    */
    DrawSubController* GetSubController (void);
};

} // end of namespace sd

#endif
