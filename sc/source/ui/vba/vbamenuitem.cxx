#include "vbamenuitem.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenuItem::ScVbaMenuItem( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBarControl >& xCommandBarControl ) throw( uno::RuntimeException ) : MenuItem_BASE( xParent, xContext ), m_xCommandBarControl( xCommandBarControl )
{
}

::rtl::OUString SAL_CALL
ScVbaMenuItem::getCaption() throw ( uno::RuntimeException )
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenuItem::setCaption( const ::rtl::OUString& _caption ) throw (uno::RuntimeException)
{
    m_xCommandBarControl->setCaption( _caption );
}

::rtl::OUString SAL_CALL
ScVbaMenuItem::getOnAction() throw ( uno::RuntimeException )
{
    return m_xCommandBarControl->getOnAction();
}

void SAL_CALL
ScVbaMenuItem::setOnAction( const ::rtl::OUString& _onaction ) throw (uno::RuntimeException)
{
    m_xCommandBarControl->setOnAction( _onaction );
}

void SAL_CALL
ScVbaMenuItem::Delete( ) throw (script::BasicErrorException, uno::RuntimeException)
{
    m_xCommandBarControl->Delete();
}

rtl::OUString&
ScVbaMenuItem::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMenuItem") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaMenuItem::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.MenuItem" ) );
    }
    return aServiceNames;
}
