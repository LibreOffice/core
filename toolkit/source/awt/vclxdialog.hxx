#ifndef LAYOUT_AWT_VCLXDIALOG_HXX
#define LAYOUT_AWT_VCLXDIALOG_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/awt/XDialog2.hpp>

#include <com/sun/star/awt/XSimpleTabController.hpp>
#include "../layout/bin.hxx"

#include <com/sun/star/awt/MaxChildrenException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "forward.hxx"

using namespace toolkit;

//........................................................................
namespace layoutimpl
{
//........................................................................

//====================================================================
//= VCLXDialog
//====================================================================

typedef ::cppu::ImplHelper3 < ::com::sun::star::awt::XTopWindow,
                              ::com::sun::star::awt::XSystemDependentWindowPeer,
                              ::com::sun::star::awt::XDialog2
                              >   VCLXDialog_Base;

class VCLXDialog :public VCLXWindow
                 ,public VCLXDialog_Base
                 ,public Bin
{
public:
    VCLXDialog();

    // ::com::sun::star::awt::XDialog2
    void SAL_CALL endDialog( sal_Int32 nResult ) throw(::com::sun::star::uno::RuntimeException);

protected:
    ~VCLXDialog( );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException);

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

    // ::com::sun::star::awt::XSystemDependendtWindowPeer
    ::com::sun::star::uno::Any SAL_CALL getWindowHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL toFront(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL toBack(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& xMenu ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDialog
    void SAL_CALL setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getTitle(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL endExecute() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    void resizedCb();

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &/*aIds*/ )
    {
    }
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) { return ImplGetPropertyIds( aIds ); }

private:
    VCLXDialog( const VCLXDialog& );            // never implemented
    VCLXDialog& operator=( const VCLXDialog& ); // never implemented
    bool bRealized, bResizeSafeguard;

    css::uno::Reference< css::awt::XLayoutUnit > mxLayoutUnit;
};

//........................................................................
} // namespace layoutimpl
//........................................................................

#endif /*LAYOUT_AWT_VCLXDIALOG_HXX*/
