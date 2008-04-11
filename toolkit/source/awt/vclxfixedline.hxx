#ifndef LAYOUT_AWT_VCLXFIXEDLINE_HXX
#define LAYOUT_AWT_VCLXFIXEDLINE_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include "forward.hxx"

/* We just provide our own FixedLine, because default has no width... */

class FixedLine;
using namespace toolkit;

//........................................................................
namespace layoutimpl
{
//........................................................................

//====================================================================
//= VCLXFixedLine
//====================================================================

class VCLXFixedLine :public VCLXWindow
{
public:
    VCLXFixedLine();

protected:
    ~VCLXFixedLine( );

    // XInterface
//        DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw(::com::sun::star::uno::RuntimeException);

    // VclWindowPeer
    virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // VCLXWindow
    void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

private:
    VCLXFixedLine( const VCLXFixedLine& );            // never implemented
    VCLXFixedLine& operator=( const VCLXFixedLine& ); // never implemented
};

//........................................................................
} // namespacetoolkit
//........................................................................

#endif /*LAYOUT_AWT_VCLXFIXEDLINE_HXX*/
