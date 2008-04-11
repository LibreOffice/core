#ifndef LAYOUT_AWT_VCLXTABCONTROLLER_HXX
#define LAYOUT_AWT_VCLXTABCONTROLLER_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include "../layout/container.hxx"

#include <com/sun/star/awt/MaxChildrenException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "forward.hxx"

class TabControl;
using namespace toolkit;

//........................................................................
namespace layoutimpl
{
//........................................................................

//====================================================================
//= VCLXTabControl
//====================================================================

typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XSimpleTabController
                                >   VCLXTabControl_Base;

class VCLXTabControl :public VCLXWindow
                     ,public VCLXTabControl_Base
                     ,public Container
{
public:
    VCLXTabControl();

protected:
    ~VCLXTabControl( );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException);

    virtual SAL_CALL void draw( sal_Int32 nX, sal_Int32 nY ) throw        (::com::sun::star::uno::RuntimeException);

    // XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getActiveTabID(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL addChild(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >& Child )
        throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::awt::MaxChildrenException);
    virtual void SAL_CALL removeChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >& Child )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference
                                             < ::com::sun::star::awt::XLayoutConstrains > > SAL_CALL getChildren()
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL allocateArea( const ::com::sun::star::awt::Rectangle &rArea )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw(::com::sun::star::uno::RuntimeException);

    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getChildProperties(
        const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    // unimplemented:
    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException)
    { return false; }
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 /*nWidth*/ )
    throw(css::uno::RuntimeException)
    { return maRequisition.Height; }

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

private:
    VCLXTabControl( const VCLXTabControl& );            // never implemented
    VCLXTabControl& operator=( const VCLXTabControl& ); // never implemented

    // Each tab page needs an unique id.
    int mnNextTabId;

    // Maps page ids to child references
    friend class TabChildProps;
    struct ChildData
    {
        rtl::OUString aTitle;
        css::awt::Size aRequisition;
        css::uno::Reference< css::awt::XLayoutConstrains > xChild;
        css::uno::Reference< css::beans::XPropertySet >    xProps;
        bool isVisible();
    };
    std::vector< ChildData * > maChildren;  // some may be null
    int mnChildrenNb;

    // FIXME: we might want to use a Multiplexer
    std::list< ::com::sun::star::uno::Reference
               < ::com::sun::star::awt::XTabListener > > mxTabListeners;

    inline TabControl *getTabControl( ) const throw (::com::sun::star::uno::RuntimeException);
};

//........................................................................
} // namespacetoolkit
//........................................................................

#endif /*LAYOUT_AWT_VCLXTABCONTROLLER_HXX*/
