/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "vbacommandbarcontrol.hxx"
#include "vbacommandbarcontrols.hxx"
#include <vbahelper/vbahelper.hxx>
#include <filter/msfilter/msvbahelper.hxx>

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
    MacroResolvedInfo aResolvedMacro = ooo::vba::resolveVBAMacro( getSfxObjShell( xModel ), _onaction, true );
    if ( aResolvedMacro.mbFound )
    {
        rtl::OUString aCommandURL = ooo::vba::makeMacroURL( aResolvedMacro.msResolvedMacro );
        OSL_TRACE(" ScVbaCommandBarControl::setOnAction: %s", rtl::OUStringToOString( aCommandURL, RTL_TEXTENCODING_UTF8 ).getStr() );
        setPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii("CommandURL"), uno::makeAny( aCommandURL ) );
        ApplyChange();
    }
}

::sal_Bool SAL_CALL
ScVbaCommandBarControl::getVisible() throw (uno::RuntimeException)
{
    sal_Bool bVisible = sal_True;
    uno::Any aValue = getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ISVISIBLE ) );
    if( aValue.hasValue() )
        aValue >>= bVisible;
    return bVisible;
}
void SAL_CALL
ScVbaCommandBarControl::setVisible( ::sal_Bool _visible ) throw (uno::RuntimeException)
{
    uno::Any aValue = getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ISVISIBLE ) );
    if( aValue.hasValue() )
    {
        setPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ISVISIBLE ), uno::makeAny( _visible ) );
        ApplyChange();
    }
}

::sal_Bool SAL_CALL
ScVbaCommandBarControl::getEnabled() throw (uno::RuntimeException)
{
    sal_Bool bEnabled = sal_True;

    uno::Any aValue = getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ENABLED ) );
    if( aValue.hasValue() )
    {
        aValue >>= bEnabled;
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
    uno::Any aValue = getPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ENABLED ) );
    if( aValue.hasValue() )
    {
        setPropertyValue( m_aPropertyValues, rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ENABLED ), uno::makeAny( _enabled ) );
        ApplyChange();
    }
    else
    {
        // emulated with Visible
        setVisible( _enabled );
    }
}

::sal_Bool SAL_CALL
ScVbaCommandBarControl::getBeginGroup() throw (css::uno::RuntimeException)
{
    // TODO: need to check if the item before this item is of type 'separator'
    return sal_False;
}

void SAL_CALL
ScVbaCommandBarControl::setBeginGroup( ::sal_Bool _begin ) throw (css::uno::RuntimeException)
{
    if( getBeginGroup() != _begin )
    {
        // TODO: need to insert or remove an item of type 'separator' before this item
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

    uno::Reference< XCommandBarControls > xCommandBarControls( new ScVbaCommandBarControls( this, mxContext, xSubMenu, pCBarHelper, m_xBarSettings, m_sResourceUrl ) );
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
ScVbaCommandBarPopup::ScVbaCommandBarPopup( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xSettings, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl, sal_Int32 nPosition, sal_Bool bTemporary ) throw (css::uno::RuntimeException) : CommandBarPopup_BASE( xParent, xContext, xSettings, pHelper, xBarSettings, sResourceUrl )
{
    m_nPosition = nPosition;
    m_bTemporary = bTemporary;
    m_xCurrentSettings->getByIndex( m_nPosition ) >>= m_aPropertyValues;
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
ScVbaCommandBarButton::ScVbaCommandBarButton( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xSettings, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl, sal_Int32 nPosition, sal_Bool bTemporary ) throw (css::uno::RuntimeException) : CommandBarButton_BASE( xParent, xContext, xSettings, pHelper, xBarSettings, sResourceUrl )
{
    m_nPosition = nPosition;
    m_bTemporary = bTemporary;
    m_xCurrentSettings->getByIndex( m_nPosition ) >>= m_aPropertyValues;
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
