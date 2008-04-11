#ifndef LAYOUT_AWT_VCLXSPLITTER_HXX
#define LAYOUT_AWT_VCLXSPLITTER_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include "../layout/container.hxx"

#include <com/sun/star/awt/MaxChildrenException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "forward.hxx"

class Splitter;
using namespace toolkit;

//........................................................................
namespace layoutimpl
{
//........................................................................

//====================================================================
//= VCLXSplitter
//====================================================================

class VCLXSplitter :public VCLXWindow
                   ,public Container
{
public:
    VCLXSplitter( bool bHorizontal );

protected:
    ~VCLXSplitter( );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException);

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
    VCLXSplitter( const VCLXSplitter& );            // never implemented
    VCLXSplitter& operator=( const VCLXSplitter& ); // never implemented

    // Maps page ids to child references
    friend class SplitterChildProps;
    struct ChildData
    {
        sal_Bool bShrink;

        css::uno::Reference< css::awt::XLayoutConstrains > xChild;
        css::uno::Reference< css::beans::XPropertySet >    xProps;
        bool isVisible();
    };
    ChildData maChildren[ 2 ];  // some may be null

    float mnHandleRatio;
    bool mbHandlePressed;

    DECL_LINK( HandleMovedHdl, Splitter* );
    bool mbHorizontal;
    Splitter *mpSplitter;
    void ensureSplitter();
};

//........................................................................
} // namespacetoolkit
//........................................................................

#endif /*LAYOUT_AWT_VCLXSPLITTER_HXX*/
