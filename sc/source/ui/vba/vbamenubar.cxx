#include "vbamenubar.hxx"
#include "vbamenus.hxx"
#include <ooo/vba/XCommandBarControls.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenuBar::ScVbaMenuBar( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBar >& xCommandBar ) throw( uno::RuntimeException ) : MenuBar_BASE( xParent, xContext ), m_xCommandBar( xCommandBar )
{
}

uno::Any SAL_CALL
ScVbaMenuBar::Menus( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< XCommandBarControls > xCommandBarControls( m_xCommandBar->Controls( uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XMenus > xMenus( new ScVbaMenus( this, mxContext, xCommandBarControls ) );
    if( aIndex.hasValue() )
    {
        return xMenus->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xMenus );
}

rtl::OUString&
ScVbaMenuBar::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMenuBar") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaMenuBar::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.MenuBar" ) );
    }
    return aServiceNames;
}
