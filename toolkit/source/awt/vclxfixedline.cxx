#include "vclxfixedline.hxx"
#include "toolkit/helper/property.hxx"
#include <tools/debug.hxx>
#include <vcl/fixed.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/awt/PosSize.hpp>

using namespace toolkit;
//........................................................................
namespace layoutimpl
{
//........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

//====================================================================
//= VCLXFixedLine
//====================================================================
DBG_NAME( VCLXFixedLine )
//--------------------------------------------------------------------
VCLXFixedLine::VCLXFixedLine()
: VCLXWindow()
{
    DBG_CTOR( VCLXFixedLine, NULL );
}

//--------------------------------------------------------------------
VCLXFixedLine::~VCLXFixedLine()
{
    DBG_DTOR( VCLXFixedLine, NULL );
}

//--------------------------------------------------------------------
//    IMPLEMENT_FORWARD_XINTERFACE1( VCLXFixedLine, VCLXWindow )

//--------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER1( VCLXFixedLine, VCLXWindow )

//--------------------------------------------------------------------
void SAL_CALL VCLXFixedLine::dispose( ) throw(RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = *this;
    }

    VCLXWindow::dispose();
}

//--------------------------------------------------------------------
::com::sun::star::awt::Size SAL_CALL VCLXFixedLine::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    return awt::Size( 8, 8 );
}

//--------------------------------------------------------------------
void VCLXFixedLine::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    ::vos::OClearableGuard aGuard( GetMutex() );

    switch ( _rVclWindowEvent.GetId() )
    {
        default:
            aGuard.clear();
            VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
            break;
    }
}

//--------------------------------------------------------------------
void SAL_CALL VCLXFixedLine::setProperty( const ::rtl::OUString& PropertyName, const Any &Value ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
                VCLXWindow::setProperty( PropertyName, Value );
        }
    }
}

//--------------------------------------------------------------------
Any SAL_CALL VCLXFixedLine::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Any aReturn;
    if ( GetWindow() )
    {
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
                aReturn = VCLXWindow::getProperty( PropertyName );
        }
    }
    return aReturn;
}

//........................................................................
} // namespace toolkit
//........................................................................
