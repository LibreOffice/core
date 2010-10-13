#include "vbamenu.hxx"
#include "vbamenuitems.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenu::ScVbaMenu( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBarControl >& xCommandBarControl ) throw( uno::RuntimeException ) : Menu_BASE( xParent, xContext ), m_xCommandBarControl( xCommandBarControl )
{
}

::rtl::OUString SAL_CALL
ScVbaMenu::getCaption() throw ( uno::RuntimeException )
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenu::setCaption( const ::rtl::OUString& _caption ) throw (uno::RuntimeException)
{
    m_xCommandBarControl->setCaption( _caption );
}

void SAL_CALL
ScVbaMenu::Delete( ) throw (script::BasicErrorException, uno::RuntimeException)
{
    m_xCommandBarControl->Delete();
}

uno::Any SAL_CALL
ScVbaMenu::MenuItems( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< XCommandBarControls > xCommandBarControls( m_xCommandBarControl->Controls( uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XMenuItems > xMenuItems( new ScVbaMenuItems( this, mxContext, xCommandBarControls ) );
    if( aIndex.hasValue() )
    {
        return xMenuItems->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xMenuItems );
}

rtl::OUString&
ScVbaMenu::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMenu") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaMenu::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Menu" ) );
    }
    return aServiceNames;
}
