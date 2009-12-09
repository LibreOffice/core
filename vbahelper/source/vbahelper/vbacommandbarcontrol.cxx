/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "vbacommandbarcontrol.hxx"
#include "vbacommandbarcontrols.hxx"
#include <vbahelper/vbahelper.hxx>
#include <svx/msvbahelper.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaCommandBarControl::ScVbaCommandBarControl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xSettings, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl ) throw (css::uno::RuntimeException) : CommandBarControl_BASE( xParent, xContext ), pCBarHelper( pHelper ), m_sResourceUrl( sResourceUrl ), m_xCurrentSettings( xSettings ), m_xBarSettings( xBarSettings ), m_nPosition( 0 ), m_bTemporary( sal_True )
{
}

ScVbaCommandBarControl::ScVbaCommandBarControl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xSettings, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl, sal_Int32 nPosition, sal_Bool bTemporary ) throw (css::uno::RuntimeException) : CommandBarControl_BASE( xParent, xContext ), pCBarHelper( pHelper ), m_sResourceUrl( sResourceUrl ), m_xCurrentSettings( xSettings ), m_xBarSettings( xBarSettings ), m_nPosition( nPosition ), m_bTemporary( bTemporary )
{
    m_xCurrentSettings->getByIndex( nPosition ) >>= m_aPropertyValues;
}

void ScVbaCommandBarControl::ApplyChange() throw ( uno::RuntimeException )
{
    uno::Reference< container::XIndexContainer > xIndexContainer( m_xCurrentSettings, uno::UNO_QUERY_THROW );
    xIndexContainer->replaceByIndex( m_nPosition, uno::makeAny( m_aPropertyValues ) );
    pCBarHelper->ApplyChange( m_sResourceUrl, m_xBarSettings );
}

::rtl::OUString SAL_CALL
ScVbaCommandBarControl::getCaption() throw ( uno::RuntimeException )
{
    // "Label" always empty
    rtl::OUString sCaption;
    getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii("Label") ) >>= sCaption;
    return sCaption;
}

void SAL_CALL
ScVbaCommandBarControl::setCaption( const ::rtl::OUString& _caption ) throw (uno::RuntimeException)
{
    rtl::OUString sCaption = _caption.replace('&','~');
    setPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii("Label"), uno::makeAny( sCaption ) );
    ApplyChange();
}

::rtl::OUString SAL_CALL
ScVbaCommandBarControl::getOnAction() throw (uno::RuntimeException)
{
    rtl::OUString sCommandURL;
    getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii("CommandURL") ) >>= sCommandURL;
    return sCommandURL;
}

void SAL_CALL
ScVbaCommandBarControl::setOnAction( const ::rtl::OUString& _onaction ) throw (uno::RuntimeException)
{
    // get the current model
    uno::Reference< frame::XModel > xModel( pCBarHelper->getModel() );
    VBAMacroResolvedInfo aResolvedMacro = ooo::vba::resolveVBAMacro( getSfxObjShell( xModel ), _onaction, true );
    if ( aResolvedMacro.IsResolved() )
    {
        rtl::OUString aCommandURL = ooo::vba::makeMacroURL( aResolvedMacro.ResolvedMacro() );
        OSL_TRACE(" ScVbaCommandBarControl::setOnAction: %s", rtl::OUStringToOString( aCommandURL, RTL_TEXTENCODING_UTF8 ).getStr() );
        setPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii("CommandURL"), uno::makeAny( aCommandURL ) );
        ApplyChange();
    }
}

::sal_Bool SAL_CALL
ScVbaCommandBarControl::getVisible() throw (uno::RuntimeException)
{
    sal_Bool bVisible = sal_True;
    uno::Any aValue = getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii("IsVisible") );
    if( aValue.hasValue() )
        aValue >>= bVisible;
    return bVisible;
}
void SAL_CALL
ScVbaCommandBarControl::setVisible( ::sal_Bool _visible ) throw (uno::RuntimeException)
{
    uno::Any aValue = getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii("IsVisible") );
    if( aValue.hasValue() )
    {
        setPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii("IsVisible"), uno::makeAny( _visible ) );
        ApplyChange();
    }
}

::sal_Bool SAL_CALL
ScVbaCommandBarControl::getEnabled() throw (uno::RuntimeException)
{
    sal_Bool bEnabled = sal_True;
    if( m_xParentMenu.is() )
    {
        // currently only the menu in the MenuBat support Enable/Disable
        // FIXME: how to support the menu item in Toolbar
        bEnabled = m_xParentMenu->isItemEnabled( m_xParentMenu->getItemId( sal::static_int_cast< sal_Int16 >( m_nPosition ) ) );
    }
    else
    {
        // emulated with Visible
        bEnabled = getVisible();
    }
    return bEnabled;
}

void SAL_CALL
ScVbaCommandBarControl::setEnabled( sal_Bool _enabled ) throw (uno::RuntimeException)
{
    if( m_xParentMenu.is() )
    {
        // currently only the menu in the MenuBat support Enable/Disable
        m_xParentMenu->enableItem( m_xParentMenu->getItemId( sal::static_int_cast< sal_Int16 >( m_nPosition ) ), _enabled );
    }
    else
    {
        // emulated with Visible
        setVisible( _enabled );
    }
}

void SAL_CALL
ScVbaCommandBarControl::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    if( m_xCurrentSettings.is() )
    {
        uno::Reference< container::XIndexContainer > xIndexContainer( m_xCurrentSettings, uno::UNO_QUERY_THROW );
        xIndexContainer->removeByIndex( m_nPosition );

        pCBarHelper->ApplyChange( m_sResourceUrl, m_xBarSettings );
    }
}

uno::Any SAL_CALL
ScVbaCommandBarControl::Controls( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // only Popup Menu has controls
    uno::Reference< container::XIndexAccess > xSubMenu;
    getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_CONTAINER ) ) >>= xSubMenu;
    if( !xSubMenu.is() )
        throw uno::RuntimeException();

    uno::Reference< awt::XMenu > xMenu;
    if( m_xParentMenu.is() )
    {
        sal_Int16 nItemId = m_xParentMenu->getItemId( sal::static_int_cast< sal_Int16 >( m_nPosition ) );
        xMenu.set( m_xParentMenu->getPopupMenu( nItemId ), uno::UNO_QUERY );
    }

    uno::Reference< XCommandBarControls > xCommandBarControls( new ScVbaCommandBarControls( this, mxContext, xSubMenu, pCBarHelper, m_xBarSettings, m_sResourceUrl, xMenu ) );
    if( aIndex.hasValue() )
    {
        return xCommandBarControls->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xCommandBarControls );
}

rtl::OUString&
ScVbaCommandBarControl::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCommandBarControl") );
    return sImplName;
}

uno::Sequence<rtl::OUString>
ScVbaCommandBarControl::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBarControl" ) );
    }
    return aServiceNames;
}

//////////// ScVbaCommandBarPopup //////////////////////////////
ScVbaCommandBarPopup::ScVbaCommandBarPopup( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xSettings, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl, sal_Int32 nPosition, sal_Bool bTemporary, const css::uno::Reference< css::awt::XMenu >& xMenu ) throw (css::uno::RuntimeException) : CommandBarPopup_BASE( xParent, xContext, xSettings, pHelper, xBarSettings, sResourceUrl )
{
    m_nPosition = nPosition;
    m_bTemporary = bTemporary;
    m_xCurrentSettings->getByIndex( m_nPosition ) >>= m_aPropertyValues;
    m_xParentMenu = xMenu;
}

rtl::OUString&
ScVbaCommandBarPopup::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCommandBarPopup") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaCommandBarPopup::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBarPopup" ) );
    }
    return aServiceNames;
}

//////////// ScVbaCommandBarButton //////////////////////////////
ScVbaCommandBarButton::ScVbaCommandBarButton( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xSettings, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl, sal_Int32 nPosition, sal_Bool bTemporary, const css::uno::Reference< css::awt::XMenu >& xMenu ) throw (css::uno::RuntimeException) : CommandBarButton_BASE( xParent, xContext, xSettings, pHelper, xBarSettings, sResourceUrl )
{
    m_nPosition = nPosition;
    m_bTemporary = bTemporary;
    m_xCurrentSettings->getByIndex( m_nPosition ) >>= m_aPropertyValues;
    m_xParentMenu = xMenu;
}

rtl::OUString&
ScVbaCommandBarButton::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCommandBarButton") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaCommandBarButton::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBarButton" ) );
    }
    return aServiceNames;
}
