#ifndef LAYOUT_AWT_VCLXSCROLLER_HXX
#define LAYOUT_AWT_VCLXSCROLLER_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include "../layout/bin.hxx"

#include <com/sun/star/awt/MaxChildrenException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "forward.hxx"

class ScrollBar;
class FixedImage;
using namespace toolkit;

//........................................................................
namespace layoutimpl
{
//........................................................................

//====================================================================
//= VCLXScroller
//====================================================================

class VCLXScroller :public VCLXWindow
                   ,public Bin
{
public:
    VCLXScroller( );

protected:
    ~VCLXScroller( );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( const ::com::sun::star::awt::Rectangle &rArea )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw(::com::sun::star::uno::RuntimeException);

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

private:
    VCLXScroller( const VCLXScroller& );            // never implemented
    VCLXScroller& operator=( const VCLXScroller& ); // never implemented

    // because the underlying window is only setup-ed after construction, init
    // scrollbars at play-time
    void ensureScrollBars();

    FixedImage *mpContent;  // dummy
    ScrollBar *mpHorScrollBar, *mpVerScrollBar;
    DECL_LINK( ScrollHdl, ScrollBar* );
};

//........................................................................
} // namespacetoolkit
//........................................................................

#endif /*LAYOUT_AWT_VCLXSCROLLER_HXX*/
