/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "vbacommandbarcontrol.hxx"
#include "vbacommandbarcontrols.hxx"
#include <vbahelper/vbahelper.hxx>
#include <filter/msfilter/msvbahelper.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaCommandBarControl::ScVbaCommandBarControl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xSettings, const VbaCommandBarHelperRef& pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const OUString& sResourceUrl ) : CommandBarControl_BASE( xParent, xContext ), pCBarHelper( pHelper ), m_sResourceUrl( sResourceUrl ), m_xCurrentSettings( xSettings ), m_xBarSettings( xBarSettings ), m_nPosition( 0 )
{
}

void ScVbaCommandBarControl::ApplyChange()
{
    uno::Reference< container::XIndexContainer > xIndexContainer( m_xCurrentSettings, uno::UNO_QUERY_THROW );
    xIndexContainer->replaceByIndex( m_nPosition, uno::makeAny( m_aPropertyValues ) );
    pCBarHelper->ApplyTempChange( m_sResourceUrl, m_xBarSettings );
}

OUString SAL_CALL
ScVbaCommandBarControl::getCaption()
{
    // "Label" always empty
    OUString sCaption;
    getPropertyValue( m_aPropertyValues, "Label" ) >>= sCaption;
    return sCaption;
}

void SAL_CALL
ScVbaCommandBarControl::setCaption( const OUString& _caption )
{
    OUString sCaption = _caption.replace('&','~');
    setPropertyValue( m_aPropertyValues, "Label" , uno::makeAny( sCaption ) );
    ApplyChange();
}

OUString SAL_CALL
ScVbaCommandBarControl::getOnAction()
{
    OUString sCommandURL;
    getPropertyValue( m_aPropertyValues, "CommandURL" ) >>= sCommandURL;
    return sCommandURL;
}

void SAL_CALL
ScVbaCommandBarControl::setOnAction( const OUString& _onaction )
{
    // get the current model
    uno::Reference< frame::XModel > xModel( pCBarHelper->getModel() );
    MacroResolvedInfo aResolvedMacro = ooo::vba::resolveVBAMacro( getSfxObjShell( xModel ), _onaction, true );
    if ( aResolvedMacro.mbFound )
    {
        OUString aCommandURL = ooo::vba::makeMacroURL( aResolvedMacro.msResolvedMacro );
        SAL_INFO("vbahelper", "ScVbaCommandBarControl::setOnAction: " << aCommandURL);
        setPropertyValue( m_aPropertyValues, "CommandURL" , uno::makeAny( aCommandURL ) );
        ApplyChange();
    }
}

sal_Bool SAL_CALL
ScVbaCommandBarControl::getVisible()
{
    bool bVisible = true;
    uno::Any aValue = getPropertyValue( m_aPropertyValues, ITEM_DESCRIPTOR_ISVISIBLE );
    if( aValue.hasValue() )
        aValue >>= bVisible;
    return bVisible;
}
void SAL_CALL
ScVbaCommandBarControl::setVisible( sal_Bool _visible )
{
    uno::Any aValue = getPropertyValue( m_aPropertyValues, ITEM_DESCRIPTOR_ISVISIBLE );
    if( aValue.hasValue() )
    {
        setPropertyValue( m_aPropertyValues, ITEM_DESCRIPTOR_ISVISIBLE , uno::makeAny( _visible ) );
        ApplyChange();
    }
}

sal_Bool SAL_CALL
ScVbaCommandBarControl::getEnabled()
{
    bool bEnabled = true;

    uno::Any aValue = getPropertyValue( m_aPropertyValues, ITEM_DESCRIPTOR_ENABLED );
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
ScVbaCommandBarControl::setEnabled( sal_Bool _enabled )
{
    uno::Any aValue = getPropertyValue( m_aPropertyValues, ITEM_DESCRIPTOR_ENABLED );
    if( aValue.hasValue() )
    {
        setPropertyValue( m_aPropertyValues, ITEM_DESCRIPTOR_ENABLED , uno::makeAny( _enabled ) );
        ApplyChange();
    }
    else
    {
        // emulated with Visible
        setVisible( _enabled );
    }
}

sal_Bool SAL_CALL
ScVbaCommandBarControl::getBeginGroup()
{
    // TODO: need to check if the item before this item is of type 'separator'
    //#STUB
    return false;
}

void SAL_CALL
ScVbaCommandBarControl::setBeginGroup( sal_Bool _begin )
{
    if( getBeginGroup() != _begin )
    {
        // TODO: need to insert or remove an item of type 'separator' before this item
    }
}

void SAL_CALL
ScVbaCommandBarControl::Delete(  )
{
    if( m_xCurrentSettings.is() )
    {
        uno::Reference< container::XIndexContainer > xIndexContainer( m_xCurrentSettings, uno::UNO_QUERY_THROW );
        xIndexContainer->removeByIndex( m_nPosition );

        pCBarHelper->ApplyTempChange( m_sResourceUrl, m_xBarSettings );
    }
}

uno::Any SAL_CALL
ScVbaCommandBarControl::Controls( const uno::Any& aIndex )
{
    // only Popup Menu has controls
    uno::Reference< container::XIndexAccess > xSubMenu;
    getPropertyValue( m_aPropertyValues, ITEM_DESCRIPTOR_CONTAINER ) >>= xSubMenu;
    if( !xSubMenu.is() )
        throw uno::RuntimeException();

    uno::Reference< XCommandBarControls > xCommandBarControls( new ScVbaCommandBarControls( this, mxContext, xSubMenu, pCBarHelper, m_xBarSettings, m_sResourceUrl ) );
    if( aIndex.hasValue() )
    {
        return xCommandBarControls->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xCommandBarControls );
}

OUString
ScVbaCommandBarControl::getServiceImplName()
{
    return OUString("ScVbaCommandBarControl");
}

uno::Sequence<OUString>
ScVbaCommandBarControl::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.CommandBarControl";
    }
    return aServiceNames;
}

// ScVbaCommandBarPopup
ScVbaCommandBarPopup::ScVbaCommandBarPopup( const css::uno::Reference< ov::XHelperInterface >& xParent,
                                            const css::uno::Reference< css::uno::XComponentContext >& xContext,
                                            const css::uno::Reference< css::container::XIndexAccess >& xSettings,
                                            const VbaCommandBarHelperRef& pHelper,
                                            const css::uno::Reference< css::container::XIndexAccess >& xBarSettings,
                                            const OUString& sResourceUrl,
                                            sal_Int32 nPosition )
    : CommandBarPopup_BASE( xParent, xContext, xSettings, pHelper, xBarSettings, sResourceUrl )
{
    m_nPosition = nPosition;
    m_xCurrentSettings->getByIndex( m_nPosition ) >>= m_aPropertyValues;
}

OUString
ScVbaCommandBarPopup::getServiceImplName()
{
    return OUString("ScVbaCommandBarPopup");
}

uno::Sequence<OUString>
ScVbaCommandBarPopup::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.CommandBarPopup";
    }
    return aServiceNames;
}

// ScVbaCommandBarButton
ScVbaCommandBarButton::ScVbaCommandBarButton( const css::uno::Reference< ov::XHelperInterface >& xParent,
                                              const css::uno::Reference< css::uno::XComponentContext >& xContext,
                                              const css::uno::Reference< css::container::XIndexAccess >& xSettings,
                                              const VbaCommandBarHelperRef& pHelper,
                                              const css::uno::Reference< css::container::XIndexAccess >& xBarSettings,
                                              const OUString& sResourceUrl,
                                              sal_Int32 nPosition )
    : CommandBarButton_BASE( xParent, xContext, xSettings, pHelper, xBarSettings, sResourceUrl )
{
    m_nPosition = nPosition;
    m_xCurrentSettings->getByIndex( m_nPosition ) >>= m_aPropertyValues;
}

OUString
ScVbaCommandBarButton::getServiceImplName()
{
    return OUString("ScVbaCommandBarButton");
}

uno::Sequence<OUString>
ScVbaCommandBarButton::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.CommandBarButton";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
