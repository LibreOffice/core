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

#include <memory>
#include <svtools/popupmenucontrollerbase.hxx>
#include <svx/SmartTagItem.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/menu.hxx>

const sal_uInt16 MN_ST_INSERT_START = 500;

namespace {

class SmartTagMenuController : public svt::PopupMenuControllerBase
{
public:
    explicit SmartTagMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    void FillMenu();
    DECL_LINK( MenuSelect, Menu*, bool );
    struct InvokeAction
    {
        css::uno::Reference< css::smarttags::XSmartTagAction > m_xAction;
        css::uno::Reference< css::container::XStringKeyMap > m_xSmartTagProperties;
        sal_uInt32 m_nActionID;
        InvokeAction( css::uno::Reference< css::smarttags::XSmartTagAction > const & xAction,
                      css::uno::Reference< css::container::XStringKeyMap > const & xSmartTagProperties,
                      sal_uInt32 nActionID ) : m_xAction( xAction ), m_xSmartTagProperties( xSmartTagProperties ), m_nActionID( nActionID ) {}
    };
    std::vector< InvokeAction > m_aInvokeActions;
    std::unique_ptr< const SvxSmartTagItem > m_pSmartTagItem;
};

}

SmartTagMenuController::SmartTagMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::PopupMenuControllerBase( rxContext )
{
}

void SmartTagMenuController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    resetPopupMenu( m_xPopupMenu );

    css::uno::Sequence< css::beans::PropertyValue > aProperties;
    if ( !rEvent.IsEnabled || !( rEvent.State >>= aProperties ) )
        return;

    css::uno::Sequence< css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > > > aActionComponents;
    css::uno::Sequence< css::uno::Sequence< sal_Int32 > > aActionIndices;
    css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > > aStringKeyMaps;
    css::uno::Reference< css::text::XTextRange > xTextRange;
    css::uno::Reference< css::frame::XController > xController;
    css::lang::Locale aLocale;
    OUString aApplicationName;
    OUString aRangeText;

    for ( const auto& aProperty : std::as_const(aProperties) )
    {
        if ( aProperty.Name == "ActionComponents" )
            aProperty.Value >>= aActionComponents;
        else if ( aProperty.Name == "ActionIndices" )
            aProperty.Value >>= aActionIndices;
        else if ( aProperty.Name == "StringKeyMaps" )
            aProperty.Value >>= aStringKeyMaps;
        else if ( aProperty.Name == "TextRange" )
            aProperty.Value >>= xTextRange;
        else if ( aProperty.Name == "Controller" )
            aProperty.Value >>= xController;
        else if ( aProperty.Name == "Locale" )
            aProperty.Value >>= aLocale;
        else if ( aProperty.Name == "ApplicationName" )
            aProperty.Value >>= aApplicationName;
        else if ( aProperty.Name == "RangeText" )
            aProperty.Value >>= aRangeText;
    }
    m_pSmartTagItem.reset( new SvxSmartTagItem( 0, aActionComponents, aActionIndices, aStringKeyMaps, xTextRange, xController, aLocale, aApplicationName, aRangeText ) );
    FillMenu();
}

void SmartTagMenuController::FillMenu()
{
    if ( !m_pSmartTagItem )
        return;

    sal_uInt16 nMenuId = 1;
    sal_uInt16 nSubMenuId = MN_ST_INSERT_START;

    VCLXMenu* pAwtMenu = comphelper::getFromUnoTunnel<VCLXMenu>( m_xPopupMenu );
    PopupMenu* pVCLMenu = static_cast< PopupMenu* >( pAwtMenu->GetMenu() );

    const css::uno::Sequence< css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > > >& rActionComponentsSequence = m_pSmartTagItem->GetActionComponentsSequence();
    const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& rActionIndicesSequence = m_pSmartTagItem->GetActionIndicesSequence();
    const css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& rStringKeyMaps = m_pSmartTagItem->GetStringKeyMaps();
    const css::lang::Locale& rLocale = m_pSmartTagItem->GetLocale();
    const OUString aApplicationName = m_pSmartTagItem->GetApplicationName();
    const OUString aRangeText = m_pSmartTagItem->GetRangeText();
    const css::uno::Reference< css::text::XTextRange >& xTextRange = m_pSmartTagItem->GetTextRange();
    const css::uno::Reference< css::frame::XController >& xController = m_pSmartTagItem->GetController();

    for ( sal_Int32 i = 0; i < rActionComponentsSequence.getLength(); ++i )
    {
        css::uno::Reference< css::container::XStringKeyMap > xSmartTagProperties = rStringKeyMaps[i];

        // Get all actions references associated with the current smart tag type
        const css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > >& rActionComponents = rActionComponentsSequence[i];
        const css::uno::Sequence< sal_Int32 >& rActionIndices = rActionIndicesSequence[i];

        if ( !rActionComponents.hasElements() || !rActionIndices.hasElements() )
            continue;

        // Ask first entry for the smart tag type caption
        css::uno::Reference< css::smarttags::XSmartTagAction > xFirstAction = rActionComponents[0];

        if ( !xFirstAction.is() )
            continue;

        const sal_Int32 nSmartTagIndex = rActionIndices[0];
        const OUString aSmartTagType = xFirstAction->getSmartTagName( nSmartTagIndex );
        const OUString aSmartTagCaption = xFirstAction->getSmartTagCaption( nSmartTagIndex, rLocale );

        // No sub-menus if there's only one smart tag type listed
        PopupMenu* pSubMenu = pVCLMenu;
        if ( 1 < rActionComponentsSequence.getLength() )
        {
            pVCLMenu->InsertItem( nMenuId, aSmartTagCaption );
            VclPtrInstance<PopupMenu> pMenu;
            pSubMenu = pMenu;
            pVCLMenu->SetPopupMenu( nMenuId++, pSubMenu );
        }
        pSubMenu->SetSelectHdl( LINK( this, SmartTagMenuController, MenuSelect ) );

        // Sub-menu starts with smart tag caption and separator
        const OUString aSmartTagCaption2 = aSmartTagCaption + ": " + aRangeText;
        pSubMenu->InsertItem( nMenuId++, aSmartTagCaption2, MenuItemBits::NOSELECT );
        pSubMenu->InsertSeparator();

        // Add subitem for every action reference for the current smart tag type
        for ( const auto& xAction : rActionComponents )
        {
            for ( sal_Int32 j = 0; j < xAction->getActionCount( aSmartTagType, xController, xSmartTagProperties ); ++j )
            {
                const sal_uInt32 nActionID = xAction->getActionID( aSmartTagType, j, xController );
                OUString aActionCaption = xAction->getActionCaptionFromID( nActionID,
                                                                           aApplicationName,
                                                                           rLocale,
                                                                           xSmartTagProperties,
                                                                           aRangeText,
                                                                           OUString(),
                                                                           xController,
                                                                           xTextRange );

                pSubMenu->InsertItem( nSubMenuId++, aActionCaption );
                InvokeAction aEntry( xAction, xSmartTagProperties, nActionID );
                m_aInvokeActions.push_back( aEntry );
            }
        }
    }

    sal_Int16 nItemCount = m_xPopupMenu->getItemCount();
    if (nItemCount > 0)
    {
        static const OUStringLiteral aCommand = u".uno:AutoCorrectDlg?OpenSmartTag:bool=true";
        m_xPopupMenu->insertSeparator(nItemCount++);
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(aCommand, m_aModuleName);
        m_xPopupMenu->insertItem(nMenuId, vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties),
                                 0, nItemCount);
        m_xPopupMenu->setCommand(nMenuId, aCommand);
    }
}

IMPL_LINK( SmartTagMenuController, MenuSelect, Menu*, pMenu, bool )
{
    if ( !m_pSmartTagItem )
        return false;

    sal_uInt16 nMyId = pMenu->GetCurItemId();
    if ( nMyId < MN_ST_INSERT_START )
        return false;

    nMyId -= MN_ST_INSERT_START;

    // Compute SmartTag lib index and action index
    css::uno::Reference< css::smarttags::XSmartTagAction > xSmartTagAction = m_aInvokeActions[nMyId].m_xAction;

    // Execute action
    if ( xSmartTagAction.is() )
    {
        xSmartTagAction->invokeAction( m_aInvokeActions[nMyId].m_nActionID,
                                       m_pSmartTagItem->GetApplicationName(),
                                       m_pSmartTagItem->GetController(),
                                       m_pSmartTagItem->GetTextRange(),
                                       m_aInvokeActions[nMyId].m_xSmartTagProperties,
                                       m_pSmartTagItem->GetRangeText(),
                                       OUString(),
                                       m_pSmartTagItem->GetLocale() );
    }
    return false;
}

OUString SmartTagMenuController::getImplementationName()
{
    return "com.sun.star.comp.svx.SmartTagMenuController";
}

css::uno::Sequence< OUString > SmartTagMenuController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.PopupMenuController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_SmartTagMenuController_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence< css::uno::Any > const & )
{
    return cppu::acquire( new SmartTagMenuController( xContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
