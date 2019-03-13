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

#include <sal/config.h>
#include <sal/log.hxx>

#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>

#include <vcl/commandinfoprovider.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/decoview.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <sfx2/sfxhelp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/stritem.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/viewdataentry.hxx>
#include <tools/diagnose_ex.h>
#include <toolkit/helper/vclunohelper.hxx>

#include <algorithm>
#include <strings.hrc>

#include <acccfg.hxx>
#include <cfg.hxx>
#include <SvxMenuConfigPage.hxx>
#include <SvxToolbarConfigPage.hxx>
#include <SvxConfigPageHelper.hxx>
#include "eventdlg.hxx"
#include <dialmgr.hxx>

#include <unotools/configmgr.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/FileSystemStorageFactory.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/ImageManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <comphelper/documentinfo.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/processfactory.hxx>

#include <dlgname.hxx>

namespace uno = com::sun::star::uno;
namespace frame = com::sun::star::frame;
namespace lang = com::sun::star::lang;
namespace container = com::sun::star::container;
namespace beans = com::sun::star::beans;
namespace graphic = com::sun::star::graphic;

#if OSL_DEBUG_LEVEL > 1

void printPropertySet(
    const OUString& prefix,
    const uno::Reference< beans::XPropertySet >& xPropSet )
{
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();

    uno::Sequence< beans::Property > aPropDetails =
        xPropSetInfo->getProperties();

    SAL_WARN("cui", "printPropertySet: " << aPropDetails.getLength() << " properties" );

    for ( sal_Int32 i = 0; i < aPropDetails.getLength(); ++i )
    {
        OUString tmp;
        sal_Int32 ival;

        uno::Any a = xPropSet->getPropertyValue( aPropDetails[i].Name );

        if ( a >>= tmp )
        {
            SAL_WARN("cui", prefix << ": Got property: " << aPropDetails[i].Name << tmp);
        }
        else if ( ( a >>= ival ) )
        {
            SAL_WARN("cui", prefix << ": Got property: " << aPropDetails[i].Name << " = " << ival);
        }
        else
        {
            SAL_WARN("cui", prefix << ": Got property: " << aPropDetails[i].Name << " of type " << a.getValueTypeName());
        }
    }
}

void printProperties(
    const OUString& prefix,
    const uno::Sequence< beans::PropertyValue >& aProp )
{
    for ( sal_Int32 i = 0; i < aProp.getLength(); ++i )
    {
        OUString tmp;

        aProp[i].Value >>= tmp;

        SAL_WARN("cui", prefix << ": Got property: " << aProp[i].Name << " = " << tmp);
    }
}

void printEntries(SvxEntries* entries)
{
    for (auto const& entry : *entries)
    {
        SAL_WARN("cui", "printEntries: " << entry->GetName());
    }
}

#endif

bool
SvxConfigPage::CanConfig( const OUString& aModuleId )
{
    return !(aModuleId == "com.sun.star.script.BasicIDE" || aModuleId == "com.sun.star.frame.Bibliography");
}

static VclPtr<SfxTabPage> CreateSvxMenuConfigPage( TabPageParent pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxMenuConfigPage>::Create(pParent, *rSet);
}

static VclPtr<SfxTabPage> CreateSvxContextMenuConfigPage( TabPageParent pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxMenuConfigPage>::Create(pParent, *rSet, false);
}

static VclPtr<SfxTabPage> CreateKeyboardConfigPage( TabPageParent pParent, const SfxItemSet* rSet )
{
       return VclPtr<SfxAcceleratorConfigPage>::Create(pParent, *rSet);
}

static VclPtr<SfxTabPage> CreateSvxToolbarConfigPage( TabPageParent pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxToolbarConfigPage>::Create(pParent, *rSet);
}

static VclPtr<SfxTabPage> CreateSvxEventConfigPage( TabPageParent pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxEventConfigPage>::Create(pParent, *rSet, SvxEventConfigPage::EarlyInit());
}

/******************************************************************************
 *
 * SvxConfigDialog is the configuration dialog which is brought up from the
 * Tools menu. It includes tabs for customizing menus, toolbars, events and
 * key bindings.
 *
 *****************************************************************************/
SvxConfigDialog::SvxConfigDialog(weld::Window * pParent, const SfxItemSet* pInSet)
    : SfxTabDialogController(pParent, "cui/ui/customizedialog.ui", "CustomizeDialog", pInSet)
{
    SvxConfigPageHelper::InitImageType();

    AddTabPage("menus", CreateSvxMenuConfigPage, nullptr);
    AddTabPage("toolbars", CreateSvxToolbarConfigPage, nullptr);
    AddTabPage("contextmenus", CreateSvxContextMenuConfigPage, nullptr);
    AddTabPage("keyboard", CreateKeyboardConfigPage, nullptr);
    AddTabPage("events", CreateSvxEventConfigPage, nullptr);

    const SfxPoolItem* pItem =
        pInSet->GetItem( pInSet->GetPool()->GetWhich( SID_CONFIG ) );

    if ( pItem )
    {
        OUString text = static_cast<const SfxStringItem*>(pItem)->GetValue();

        if (text.startsWith( ITEM_TOOLBAR_URL ) )
        {
            SetCurPageId("toolbars");
        }
    }
}

void SvxConfigDialog::SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    m_xFrame = xFrame;

    if (!SvxConfigPageHelper::showKeyConfigTabPage( xFrame ))
        RemoveTabPage("keyboard");
}

void SvxConfigDialog::PageCreated(const OString &rId, SfxTabPage& rPage)
{
    if (rId == "menus" || rId == "keyboard" ||
        rId == "toolbars" || rId == "contextmenus")
    {
        rPage.SetFrame(m_xFrame);
    }
    else if (rId == "events")
    {
        dynamic_cast< SvxEventConfigPage& >( rPage ).LateInit( m_xFrame );
    }
}

/******************************************************************************
 *
 * The SaveInData class is used to hold data for entries in the Save In
 * ListBox controls in the menu and toolbar tabs
 *
 ******************************************************************************/

// Initialize static variable which holds default XImageManager
uno::Reference< css::ui::XImageManager>* SaveInData::xDefaultImgMgr = nullptr;

SaveInData::SaveInData(
    const uno::Reference< css::ui::XUIConfigurationManager >& xCfgMgr,
    const uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool isDocConfig )
        :
            bModified( false ),
            bDocConfig( isDocConfig ),
            bReadOnly( false ),
            m_xCfgMgr( xCfgMgr ),
            m_xParentCfgMgr( xParentCfgMgr )
{
    m_aSeparatorSeq.realloc( 1 );
    m_aSeparatorSeq[0].Name  = ITEM_DESCRIPTOR_TYPE;
    m_aSeparatorSeq[0].Value <<= css::ui::ItemType::SEPARATOR_LINE;

    if ( bDocConfig )
    {
        uno::Reference< css::ui::XUIConfigurationPersistence >
            xDocPersistence( GetConfigManager(), uno::UNO_QUERY );

        bReadOnly = xDocPersistence->isReadOnly();
    }

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();

    uno::Reference< container::XNameAccess > xNameAccess(
        css::frame::theUICommandDescription::get(xContext) );

    xNameAccess->getByName( aModuleId ) >>= m_xCommandToLabelMap;

    if ( !m_xImgMgr.is() )
    {
        m_xImgMgr.set( GetConfigManager()->getImageManager(), uno::UNO_QUERY );
    }

    if ( !IsDocConfig() )
    {
        // If this is not a document configuration then it is the settings
        // for the module (writer, calc, impress etc.) Use this as the default
        // XImageManager instance
        xDefaultImgMgr = &m_xImgMgr;
    }
    else
    {
        // If this is a document configuration then use the module image manager
        // as default.
        if ( m_xParentCfgMgr.is() )
        {
            m_xParentImgMgr.set( m_xParentCfgMgr->getImageManager(), uno::UNO_QUERY );
            xDefaultImgMgr = &m_xParentImgMgr;
        }
    }
}

uno::Reference<graphic::XGraphic> SaveInData::GetImage(const OUString& rCommandURL)
{
    uno::Reference< graphic::XGraphic > xGraphic =
        SvxConfigPageHelper::GetGraphic( m_xImgMgr, rCommandURL );

    if (!xGraphic.is() && xDefaultImgMgr != nullptr && (*xDefaultImgMgr).is())
    {
        xGraphic = SvxConfigPageHelper::GetGraphic( (*xDefaultImgMgr), rCommandURL );
    }

    return xGraphic;
}

bool SaveInData::PersistChanges(
    const uno::Reference< uno::XInterface >& xManager )
{
    bool result = true;

    try
    {
        if ( xManager.is() && !IsReadOnly() )
        {
            uno::Reference< css::ui::XUIConfigurationPersistence >
                xConfigPersistence( xManager, uno::UNO_QUERY );

            if ( xConfigPersistence->isModified() )
            {
                xConfigPersistence->store();
            }
        }
    }
    catch ( css::io::IOException& )
    {
        result = false;
    }

    return result;
}

/******************************************************************************
 *
 * The MenuSaveInData class extends SaveInData and provides menu specific
 * load and store functionality.
 *
 ******************************************************************************/

// Initialize static variable which holds default Menu data
MenuSaveInData* MenuSaveInData::pDefaultData = nullptr;

MenuSaveInData::MenuSaveInData(
    const uno::Reference< css::ui::XUIConfigurationManager >& cfgmgr,
    const uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool isDocConfig )
    :
        SaveInData( cfgmgr, xParentCfgMgr, aModuleId, isDocConfig ),
        m_aMenuResourceURL(
            ITEM_MENUBAR_URL  ),
        m_aDescriptorContainer(
            ITEM_DESCRIPTOR_CONTAINER  )
{
    try
    {
        OUString url( ITEM_MENUBAR_URL  );
        m_xMenuSettings = GetConfigManager()->getSettings( url, false );
    }
    catch ( container::NoSuchElementException& )
    {
        // will use menu settings for the module
    }

    // If this is not a document configuration then it is the settings
    // for the module (writer, calc, impress etc.). These settings should
    // be set as the default to be used for SaveIn locations that do not
    // have custom settings
    if ( !IsDocConfig() )
    {
        SetDefaultData( this );
    }
}

MenuSaveInData::~MenuSaveInData()
{
}

SvxEntries*
MenuSaveInData::GetEntries()
{
    if ( pRootEntry == nullptr )
    {
        pRootEntry.reset( new SvxConfigEntry( "MainMenus", OUString(), true, /*bParentData*/false) );

        if ( m_xMenuSettings.is() )
        {
            LoadSubMenus( m_xMenuSettings, OUString(), pRootEntry.get(), false );
        }
        else if ( GetDefaultData() != nullptr )
        {
            // If the doc has no config settings use module config settings
            LoadSubMenus( GetDefaultData()->m_xMenuSettings, OUString(), pRootEntry.get(), false );
        }
    }

    return pRootEntry->GetEntries();
}

void
MenuSaveInData::SetEntries( std::unique_ptr<SvxEntries> pNewEntries )
{
    pRootEntry->SetEntries( std::move(pNewEntries) );
}

void SaveInData::LoadSubMenus( const uno::Reference< container::XIndexAccess >& xMenuSettings,
    const OUString& rBaseTitle, SvxConfigEntry const * pParentData, bool bContextMenu )
{
    SvxEntries* pEntries = pParentData->GetEntries();

    // Don't access non existing menu configuration!
    if ( !xMenuSettings.is() )
        return;

    for ( sal_Int32 nIndex = 0; nIndex < xMenuSettings->getCount(); ++nIndex )
    {
        uno::Reference< container::XIndexAccess >   xSubMenu;
        OUString                aCommandURL;
        OUString                aLabel;

        sal_uInt16 nType( css::ui::ItemType::DEFAULT );
        sal_Int32 nStyle(0);

        bool bItem = SvxConfigPageHelper::GetMenuItemData( xMenuSettings, nIndex,
            aCommandURL, aLabel, nType, nStyle, xSubMenu );

        if ( bItem )
        {
            bool bIsUserDefined = true;

            if ( nType == css::ui::ItemType::DEFAULT )
            {
                uno::Any a;
                try
                {
                    a = m_xCommandToLabelMap->getByName( aCommandURL );
                    bIsUserDefined = false;
                }
                catch ( container::NoSuchElementException& )
                {
                    bIsUserDefined = true;
                }

                bool bUseDefaultLabel = false;
                // If custom label not set retrieve it from the command
                // to info service
                if ( aLabel.isEmpty() )
                {
                    bUseDefaultLabel = true;
                    uno::Sequence< beans::PropertyValue > aPropSeq;
                    if ( a >>= aPropSeq )
                    {
                        OUString aMenuLabel;
                        for ( sal_Int32 i = 0; i < aPropSeq.getLength(); ++i )
                        {
                            if ( bContextMenu )
                            {
                                if ( aPropSeq[i].Name == "PopupLabel" )
                                {
                                    aPropSeq[i].Value >>= aLabel;
                                    break;
                                }
                                else if ( aPropSeq[i].Name == "Label" )
                                {
                                    aPropSeq[i].Value >>= aMenuLabel;
                                }
                            }
                            else if ( aPropSeq[i].Name == "Label" )
                            {
                                aPropSeq[i].Value >>= aLabel;
                                break;
                            }
                        }
                        if ( aLabel.isEmpty() )
                            aLabel = aMenuLabel;
                    }
                }

                SvxConfigEntry* pEntry = new SvxConfigEntry(
                    aLabel, aCommandURL, xSubMenu.is(), /*bParentData*/false );

                pEntry->SetStyle( nStyle );
                pEntry->SetUserDefined( bIsUserDefined );
                if ( !bUseDefaultLabel )
                    pEntry->SetName( aLabel );

                pEntries->push_back( pEntry );

                if ( xSubMenu.is() )
                {
                    // popup menu
                    OUString subMenuTitle( rBaseTitle );

                    if ( !subMenuTitle.isEmpty() )
                    {
                        subMenuTitle += aMenuSeparatorStr;
                    }
                    else
                    {
                        pEntry->SetMain();
                    }

                    subMenuTitle += SvxConfigPageHelper::stripHotKey( aLabel );

                    LoadSubMenus( xSubMenu, subMenuTitle, pEntry, bContextMenu );
                }
            }
            else
            {
                SvxConfigEntry* pEntry = new SvxConfigEntry;
                pEntry->SetUserDefined( bIsUserDefined );
                pEntries->push_back( pEntry );
            }
        }
    }
}

bool MenuSaveInData::Apply()
{
    bool result = false;

    if ( IsModified() )
    {
        // Apply new menu bar structure to our settings container
        m_xMenuSettings.set( GetConfigManager()->createSettings(), uno::UNO_QUERY );

        uno::Reference< container::XIndexContainer > xIndexContainer (
            m_xMenuSettings, uno::UNO_QUERY );

        uno::Reference< lang::XSingleComponentFactory > xFactory (
            m_xMenuSettings, uno::UNO_QUERY );

        Apply( xIndexContainer, xFactory );

        try
        {
            if ( GetConfigManager()->hasSettings( m_aMenuResourceURL ) )
            {
                GetConfigManager()->replaceSettings(
                    m_aMenuResourceURL, m_xMenuSettings );
            }
            else
            {
                GetConfigManager()->insertSettings(
                    m_aMenuResourceURL, m_xMenuSettings );
            }
        }
        catch ( css::uno::Exception& )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("cui.customize", "caught some other exception saving settings " << exceptionToString(ex));
        }

        SetModified( false );

        result = PersistChanges( GetConfigManager() );
    }

    return result;
}

void MenuSaveInData::Apply(
    uno::Reference< container::XIndexContainer > const & rMenuBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory )
{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();

    for (auto const& entryData : *GetEntries())
    {
        uno::Sequence< beans::PropertyValue > aPropValueSeq =
            SvxConfigPageHelper::ConvertSvxConfigEntry(entryData);

        uno::Reference< container::XIndexContainer > xSubMenuBar(
            rFactory->createInstanceWithContext( xContext ),
            uno::UNO_QUERY );

        sal_Int32 nIndex = aPropValueSeq.getLength();
        aPropValueSeq.realloc( nIndex + 1 );
        aPropValueSeq[nIndex].Name = m_aDescriptorContainer;
        aPropValueSeq[nIndex].Value <<= xSubMenuBar;
        rMenuBar->insertByIndex(
            rMenuBar->getCount(), uno::Any( aPropValueSeq ));
        ApplyMenu( xSubMenuBar, rFactory, entryData );
    }
}

void SaveInData::ApplyMenu(
    uno::Reference< container::XIndexContainer > const & rMenuBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory,
    SvxConfigEntry* pMenuData )
{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();

    for (auto const& entry : *pMenuData->GetEntries())
    {
        if (entry->IsPopup())
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                SvxConfigPageHelper::ConvertSvxConfigEntry(entry);

            uno::Reference< container::XIndexContainer > xSubMenuBar(
                rFactory->createInstanceWithContext( xContext ),
                    uno::UNO_QUERY );

            sal_Int32 nIndex = aPropValueSeq.getLength();
            aPropValueSeq.realloc( nIndex + 1 );
            aPropValueSeq[nIndex].Name = ITEM_DESCRIPTOR_CONTAINER;
            aPropValueSeq[nIndex].Value <<= xSubMenuBar;

            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::Any( aPropValueSeq ));

            ApplyMenu( xSubMenuBar, rFactory, entry );
            entry->SetModified( false );
        }
        else if (entry->IsSeparator())
        {
            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::Any( m_aSeparatorSeq ));
        }
        else
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                SvxConfigPageHelper::ConvertSvxConfigEntry(entry);
            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::Any( aPropValueSeq ));
        }
    }
    pMenuData->SetModified( false );
}

void
MenuSaveInData::Reset()
{
    try
    {
        GetConfigManager()->removeSettings( m_aMenuResourceURL );
    }
    catch ( const css::uno::Exception& )
    {}

    PersistChanges( GetConfigManager() );

    pRootEntry.reset();

    try
    {
        m_xMenuSettings = GetConfigManager()->getSettings(
            m_aMenuResourceURL, false );
    }
    catch ( container::NoSuchElementException& )
    {
        // will use default settings
    }
}

ContextMenuSaveInData::ContextMenuSaveInData(
    const css::uno::Reference< css::ui::XUIConfigurationManager >& xCfgMgr,
    const css::uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId, bool bIsDocConfig )
    : SaveInData( xCfgMgr, xParentCfgMgr, aModuleId, bIsDocConfig )
{
    css::uno::Reference< css::uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    css::uno::Reference< css::container::XNameAccess > xConfig( css::ui::theWindowStateConfiguration::get( xContext ) );
    xConfig->getByName( aModuleId ) >>= m_xPersistentWindowState;
}

ContextMenuSaveInData::~ContextMenuSaveInData()
{
}

OUString ContextMenuSaveInData::GetUIName( const OUString& rResourceURL )
{
    if ( m_xPersistentWindowState.is() )
    {
        css::uno::Sequence< css::beans::PropertyValue > aProps;
        try
        {
            m_xPersistentWindowState->getByName( rResourceURL ) >>= aProps;
        }
        catch ( const css::uno::Exception& )
        {}

        for ( const auto& aProp : aProps )
        {
            if ( aProp.Name == ITEM_DESCRIPTOR_UINAME )
            {
                OUString aResult;
                aProp.Value >>= aResult;
                return aResult;
            }
        }
    }
    return OUString();
}

SvxEntries* ContextMenuSaveInData::GetEntries()
{
    if ( !m_pRootEntry )
    {
        std::unordered_map< OUString, bool > aMenuInfo;

        m_pRootEntry.reset( new SvxConfigEntry( "ContextMenus", OUString(), true, /*bParentData*/false ) );
        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aElementsInfo;
        try
        {
            aElementsInfo = GetConfigManager()->getUIElementsInfo( css::ui::UIElementType::POPUPMENU );
        }
        catch ( const css::lang::IllegalArgumentException& )
        {}

        for ( const auto& aElement : aElementsInfo )
        {
            OUString aUrl;
            for ( const auto& aElementProp : aElement )
            {
                if ( aElementProp.Name == ITEM_DESCRIPTOR_RESOURCEURL )
                {
                    aElementProp.Value >>= aUrl;
                    break;
                }
            }

            css::uno::Reference< css::container::XIndexAccess > xPopupMenu;
            try
            {
                xPopupMenu = GetConfigManager()->getSettings( aUrl, false );
            }
            catch ( const css::uno::Exception& )
            {}

            if ( xPopupMenu.is() )
            {
                // insert into std::unordered_map to filter duplicates from the parent
                aMenuInfo.emplace(  aUrl, true );

                OUString aUIMenuName = GetUIName( aUrl );
                if ( aUIMenuName.isEmpty() )
                    // Menus without UI name aren't supposed to be customized.
                    continue;

                SvxConfigEntry* pEntry = new SvxConfigEntry( aUIMenuName, aUrl, true, /*bParentData*/false );
                pEntry->SetMain();
                m_pRootEntry->GetEntries()->push_back( pEntry );
                LoadSubMenus( xPopupMenu, aUIMenuName, pEntry, true );
            }
        }

        // Retrieve also the parent menus, to make it possible to configure module menus and save them into the document.
        css::uno::Reference< css::ui::XUIConfigurationManager > xParentCfgMgr = GetParentConfigManager();
        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aParentElementsInfo;
        try
        {
            if ( xParentCfgMgr.is() )
                aParentElementsInfo = xParentCfgMgr->getUIElementsInfo( css::ui::UIElementType::POPUPMENU );
        }
        catch ( const css::lang::IllegalArgumentException& )
        {}

        for ( const auto& aElement : aParentElementsInfo )
        {
            OUString aUrl;
            for ( const auto& aElementProp : aElement )
            {
                if ( aElementProp.Name == ITEM_DESCRIPTOR_RESOURCEURL )
                {
                    aElementProp.Value >>= aUrl;
                    break;
                }
            }

            css::uno::Reference< css::container::XIndexAccess > xPopupMenu;
            try
            {
                if ( aMenuInfo.find( aUrl ) == aMenuInfo.end() )
                    xPopupMenu = xParentCfgMgr->getSettings( aUrl, false );
            }
            catch ( const css::uno::Exception& )
            {}

            if ( xPopupMenu.is() )
            {
                OUString aUIMenuName = GetUIName( aUrl );
                if ( aUIMenuName.isEmpty() )
                    continue;

                SvxConfigEntry* pEntry = new SvxConfigEntry( aUIMenuName, aUrl, true, true );
                pEntry->SetMain();
                m_pRootEntry->GetEntries()->push_back( pEntry );
                LoadSubMenus( xPopupMenu, aUIMenuName, pEntry, true );
            }
        }
        std::sort( m_pRootEntry->GetEntries()->begin(), m_pRootEntry->GetEntries()->end(), SvxConfigPageHelper::EntrySort );
    }
    return m_pRootEntry->GetEntries();
}

void ContextMenuSaveInData::SetEntries( std::unique_ptr<SvxEntries> pNewEntries )
{
    m_pRootEntry->SetEntries( std::move(pNewEntries) );
}

bool ContextMenuSaveInData::HasURL( const OUString& rURL )
{
    SvxEntries* pEntries = GetEntries();
    for ( const auto& pEntry : *pEntries )
        if ( pEntry->GetCommand() == rURL )
            return true;

    return false;
}

bool ContextMenuSaveInData::HasSettings()
{
    return m_pRootEntry && !m_pRootEntry->GetEntries()->empty();
}

bool ContextMenuSaveInData::Apply()
{
    if ( !IsModified() )
        return false;

    SvxEntries* pEntries = GetEntries();
    for ( const auto& pEntry : *pEntries )
    {
        if ( pEntry->IsModified() || SvxConfigPageHelper::SvxConfigEntryModified( pEntry ) )
        {
            css::uno::Reference< css::container::XIndexContainer > xIndexContainer( GetConfigManager()->createSettings(), css::uno::UNO_QUERY );
            css::uno::Reference< css::lang::XSingleComponentFactory > xFactory( xIndexContainer, css::uno::UNO_QUERY );
            ApplyMenu( xIndexContainer, xFactory, pEntry );

            const OUString& aUrl = pEntry->GetCommand();
            try
            {
                if ( GetConfigManager()->hasSettings( aUrl ) )
                    GetConfigManager()->replaceSettings( aUrl, xIndexContainer );
                else
                    GetConfigManager()->insertSettings( aUrl, xIndexContainer );
            }
            catch ( const css::uno::Exception& )
            {}
        }
    }
    SetModified( false );
    return PersistChanges( GetConfigManager() );
}

void ContextMenuSaveInData::Reset()
{
    SvxEntries* pEntries = GetEntries();
    for ( const auto& pEntry : *pEntries )
    {
        try
        {
            GetConfigManager()->removeSettings( pEntry->GetCommand() );
        }
        catch ( const css::uno::Exception& e )
        {
            SAL_WARN("cui.customize", "Exception caught while resetting context menus: " << e);
        }
    }
    PersistChanges( GetConfigManager() );
    m_pRootEntry.reset();
}

void ContextMenuSaveInData::ResetContextMenu( const SvxConfigEntry* pEntry )
{
    try
    {
        GetConfigManager()->removeSettings( pEntry->GetCommand() );
    }
    catch ( const css::uno::Exception& e )
    {
        SAL_WARN("cui.customize", "Exception caught while resetting context menu: " << e);
    }
    PersistChanges( GetConfigManager() );
    m_pRootEntry.reset();
}

void SvxMenuEntriesListBox::CreateDropDown()
{
    int nWidth = m_xControl->get_text_height() / 2;
    m_xDropDown->SetOutputSizePixel(Size(nWidth, nWidth));
    DecorationView aDecoView(m_xDropDown.get());
    aDecoView.DrawSymbol(tools::Rectangle(Point(0, 0), Size(nWidth, nWidth)),
                         SymbolType::SPIN_RIGHT, m_xDropDown->GetTextColor(),
                         DrawSymbolFlags::NONE);
}

/******************************************************************************
 *
 * SvxMenuEntriesListBox is the listbox in which the menu items for a
 * particular menu are shown. We have a custom listbox because we need
 * to add drag'n'drop support from the Macro Selector and within the
 * listbox
 *
 *****************************************************************************/
SvxMenuEntriesListBox::SvxMenuEntriesListBox(std::unique_ptr<weld::TreeView> xControl, SvxConfigPage* pPg)
    : m_xControl(std::move(xControl))
    , m_xDropDown(m_xControl->create_virtual_device())
    , pPage(pPg)
{
    CreateDropDown();
    m_xControl->connect_key_press(LINK(this, SvxMenuEntriesListBox, KeyInputHdl));
}

SvxMenuEntriesListBox::~SvxMenuEntriesListBox()
{
}

IMPL_LINK(SvxMenuEntriesListBox, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    vcl::KeyCode keycode = rKeyEvent.GetKeyCode();

    // support DELETE for removing the current entry
    if ( keycode == KEY_DELETE )
    {
        pPage->DeleteSelectedContent();
    }
    // support CTRL+UP and CTRL+DOWN for moving selected entries
    else if ( keycode.GetCode() == KEY_UP && keycode.IsMod1() )
    {
        pPage->MoveEntry( true );
    }
    else if ( keycode.GetCode() == KEY_DOWN && keycode.IsMod1() )
    {
        pPage->MoveEntry( false );
    }
    else
    {
        return false; // pass on to default handler
    }
    return true;
}

/******************************************************************************
 *
 * SvxConfigPage is the abstract base class on which the Menu and Toolbar
 * configuration tabpages are based. It includes methods which are common to
 * both tabpages to add, delete, move and rename items etc.
 *
 *****************************************************************************/
SvxConfigPage::SvxConfigPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/menuassignpage.ui", "MenuAssignPage", &rSet)
    , m_aUpdateDataTimer("UpdateDataTimer")
    , bInitialised(false)
    , pCurrentSaveInData(nullptr)
    , m_xSearchEdit(m_xBuilder->weld_entry("searchEntry"))
    , m_xCommandCategoryListBox(new CommandCategoryListBox(m_xBuilder->weld_combo_box("commandcategorylist")))
    , m_xFunctions(new CuiConfigFunctionListBox(m_xBuilder->weld_tree_view("functions")))
    , m_xDescriptionFieldLb(m_xBuilder->weld_label("descriptionlabel"))
    , m_xDescriptionField(m_xBuilder->weld_text_view("desc"))
    , m_xTopLevelListBox(m_xBuilder->weld_combo_box("toplevellist"))
    , m_xMoveUpButton(m_xBuilder->weld_button("up"))
    , m_xMoveDownButton(m_xBuilder->weld_button("down"))
    , m_xSaveInListBox(m_xBuilder->weld_combo_box("savein"))
    , m_xInsertBtn(m_xBuilder->weld_menu_button("insert"))
    , m_xModifyBtn(m_xBuilder->weld_menu_button("modify"))
    , m_xResetBtn(m_xBuilder->weld_button("defaultsbtn"))
    , m_xAddCommandButton(m_xBuilder->weld_button("add"))
    , m_xRemoveCommandButton(m_xBuilder->weld_button("remove"))
{
    m_xTopLevelListBox->connect_changed(LINK(this, SvxMenuConfigPage, SelectElementHdl));

    weld::TreeView& rTreeView = m_xFunctions->get_widget();
    Size aSize(rTreeView.get_approximate_digit_width() * 40, rTreeView.get_height_rows(8));
    m_xFunctions->set_size_request(aSize.Width(), aSize.Height());
    m_xDescriptionField->set_size_request(aSize.Width(), m_xDescriptionField->get_height_rows(3));

    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, SvxConfigPage, ImplUpdateDataHdl));
    m_aUpdateDataTimer.SetDebugName( "SvxConfigPage UpdateDataTimer" );
    m_aUpdateDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    m_xSearchEdit->connect_changed(LINK(this, SvxConfigPage, SearchUpdateHdl));
    m_xSearchEdit->connect_focus_out(LINK(this, SvxConfigPage, FocusOut_Impl));

    rTreeView.connect_row_activated(LINK(this, SvxConfigPage, FunctionDoubleClickHdl));
    rTreeView.connect_changed(LINK(this, SvxConfigPage, SelectFunctionHdl));
}

IMPL_LINK_NOARG(SvxConfigPage, SelectElementHdl, weld::ComboBox&, void)
{
    SelectElement();
}

SvxConfigPage::~SvxConfigPage()
{
    disposeOnce();
}

void SvxConfigPage::Reset( const SfxItemSet* )
{
    // If we haven't initialised our XMultiServiceFactory reference
    // then Reset is being called at the opening of the dialog.

    // Load menu configuration data for the module of the currently
    // selected document, for the currently selected document, and for
    // all other open documents of the same module type
    if ( !bInitialised )
    {
        sal_Int32 nPos = 0;
        uno::Reference < css::ui::XUIConfigurationManager > xCfgMgr;
        uno::Reference < css::ui::XUIConfigurationManager > xDocCfgMgr;

        uno::Reference< uno::XComponentContext > xContext(
            ::comphelper::getProcessComponentContext(), uno::UNO_QUERY_THROW );

        m_xFrame = GetFrame();
        OUString aModuleId = GetFrameWithDefaultAndIdentify( m_xFrame );

        // replace %MODULENAME in the label with the correct module name
        uno::Reference< css::frame::XModuleManager2 > xModuleManager(
            css::frame::ModuleManager::create( xContext ));
        OUString aModuleName = SvxConfigPageHelper::GetUIModuleName( aModuleId, xModuleManager );

        uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier >
            xModuleCfgSupplier( css::ui::theModuleUIConfigurationManagerSupplier::get(xContext) );

        // Set up data for module specific menus
        SaveInData* pModuleData = nullptr;

        try
        {
            xCfgMgr =
                xModuleCfgSupplier->getUIConfigurationManager( aModuleId );

            pModuleData = CreateSaveInData( xCfgMgr,
                                            uno::Reference< css::ui::XUIConfigurationManager >(),
                                            aModuleId,
                                            false );
        }
        catch ( container::NoSuchElementException& )
        {
        }

        if ( pModuleData != nullptr )
        {
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pModuleData)));
            m_xSaveInListBox->append(sId, utl::ConfigManager::getProductName() + " " + aModuleName);
        }

        // try to retrieve the document based ui configuration manager
        OUString aTitle;
        uno::Reference< frame::XController > xController =
            m_xFrame->getController();
        if ( CanConfig( aModuleId ) && xController.is() )
        {
            uno::Reference< frame::XModel > xModel( xController->getModel() );
            if ( xModel.is() )
            {
                uno::Reference< css::ui::XUIConfigurationManagerSupplier >
                    xCfgSupplier( xModel, uno::UNO_QUERY );

                if ( xCfgSupplier.is() )
                {
                    xDocCfgMgr = xCfgSupplier->getUIConfigurationManager();
                }
                aTitle = ::comphelper::DocumentInfo::getDocumentTitle( xModel );
            }
        }

        SaveInData* pDocData = nullptr;
        if ( xDocCfgMgr.is() )
        {
            pDocData = CreateSaveInData( xDocCfgMgr, xCfgMgr, aModuleId, true );

            if ( !pDocData->IsReadOnly() )
            {
                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pDocData)));
                m_xSaveInListBox->append(sId, aTitle);
            }
        }

        // if an item to select has been passed in (eg. the ResourceURL for a
        // toolbar) then try to select the SaveInData entry that has that item
        bool bURLToSelectFound = false;
        if ( !m_aURLToSelect.isEmpty() )
        {
            if ( pDocData && pDocData->HasURL( m_aURLToSelect ) )
            {
                m_xSaveInListBox->set_active(nPos);
                pCurrentSaveInData = pDocData;
                bURLToSelectFound = true;
            }
            else if ( pModuleData && pModuleData->HasURL( m_aURLToSelect ) )
            {
                m_xSaveInListBox->set_active(0);
                pCurrentSaveInData = pModuleData;
                bURLToSelectFound = true;
            }
        }

        if ( !bURLToSelectFound )
        {
            // if the document has menu configuration settings select it
            // it the SaveIn listbox, otherwise select the module data
            if ( pDocData != nullptr && pDocData->HasSettings() )
            {
                m_xSaveInListBox->set_active(nPos);
                pCurrentSaveInData = pDocData;
            }
            else
            {
                m_xSaveInListBox->set_active(0);
                pCurrentSaveInData = pModuleData;
            }
        }

#ifdef DBG_UTIL
        DBG_ASSERT( pCurrentSaveInData, "SvxConfigPage::Reset(): no SaveInData" );
#endif

        if ( CanConfig( aModuleId ) )
        {
            // Load configuration for other open documents which have
            // same module type
            uno::Sequence< uno::Reference< frame::XFrame > > aFrameList;
            try
            {
                uno::Reference< frame::XDesktop2 > xFramesSupplier = frame::Desktop::create(
                    xContext );

                uno::Reference< frame::XFrames > xFrames =
                    xFramesSupplier->getFrames();

                aFrameList = xFrames->queryFrames(
                    frame::FrameSearchFlag::ALL & ~frame::FrameSearchFlag::SELF );

            }
            catch( const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("cui.customize");
            }

            for ( sal_Int32 i = 0; i < aFrameList.getLength(); ++i )
            {
                uno::Reference < frame::XFrame > xf = aFrameList[i];

                if ( xf.is() && xf != m_xFrame )
                {
                    OUString aCheckId;
                    try{
                        aCheckId = xModuleManager->identify( xf );
                    } catch(const uno::Exception&)
                        { aCheckId.clear(); }

                    if ( aModuleId == aCheckId )
                    {
                        // try to get the document based ui configuration manager
                        OUString aTitle2;
                        uno::Reference< frame::XController > xController_ =
                            xf->getController();

                        if ( xController_.is() )
                        {
                            uno::Reference< frame::XModel > xModel(
                                xController_->getModel() );

                            if ( xModel.is() )
                            {
                                uno::Reference<
                                    css::ui::XUIConfigurationManagerSupplier >
                                        xCfgSupplier( xModel, uno::UNO_QUERY );

                                if ( xCfgSupplier.is() )
                                {
                                    xDocCfgMgr =
                                        xCfgSupplier->getUIConfigurationManager();
                                }
                                aTitle2 = ::comphelper::DocumentInfo::getDocumentTitle( xModel );
                            }
                        }

                        if ( xDocCfgMgr.is() )
                        {
                            SaveInData* pData = CreateSaveInData( xDocCfgMgr, xCfgMgr, aModuleId, true );

                            if ( pData && !pData->IsReadOnly() )
                            {
                                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pData)));
                                m_xSaveInListBox->append(sId, aTitle2);
                            }
                        }
                    }
                }
            }
        }

        m_xSaveInListBox->connect_changed(
            LINK( this, SvxConfigPage, SelectSaveInLocation ) );

        bInitialised = true;

        Init();
    }
    else
    {
        if ( QueryReset() == RET_YES )
        {
            // Reset menu configuration for currently selected SaveInData
            GetSaveInData()->Reset();

            Init();
        }
    }
}

OUString SvxConfigPage::GetFrameWithDefaultAndIdentify( uno::Reference< frame::XFrame >& _inout_rxFrame )
{
    OUString sModuleID;
    try
    {
        uno::Reference< uno::XComponentContext > xContext(
            ::comphelper::getProcessComponentContext() );

        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(
            xContext );

        if ( !_inout_rxFrame.is() )
            _inout_rxFrame = xDesktop->getActiveFrame();

        if ( !_inout_rxFrame.is() )
        {
            _inout_rxFrame = xDesktop->getCurrentFrame();
        }

        if ( !_inout_rxFrame.is() && SfxViewFrame::Current() )
            _inout_rxFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();

        if ( !_inout_rxFrame.is() )
        {
            SAL_WARN( "cui.customize", "SvxConfigPage::GetFrameWithDefaultAndIdentify(): no frame found!" );
            return sModuleID;
        }

        uno::Reference< css::frame::XModuleManager2 > xModuleManager(
                css::frame::ModuleManager::create( xContext ) );

        try
        {
            sModuleID = xModuleManager->identify( _inout_rxFrame );
        }
        catch ( const frame::UnknownModuleException& )
        {
        }

    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("cui.customize");
    }

    return sModuleID;
}

OUString SvxConfigPage::GetScriptURL() const
{
    OUString result;

    SfxGroupInfo_Impl *pData = reinterpret_cast<SfxGroupInfo_Impl*>(m_xFunctions->get_selected_id().toInt64());
    if (pData)
    {
        if  (   ( pData->nKind == SfxCfgKind::FUNCTION_SLOT ) ||
                ( pData->nKind == SfxCfgKind::FUNCTION_SCRIPT ) ||
                ( pData->nKind == SfxCfgKind::GROUP_STYLES )    )
        {
            result = pData->sCommand;
        }
    }

    return result;
}

OUString SvxConfigPage::GetSelectedDisplayName()
{
    return m_xFunctions->get_selected_text();
}

bool SvxConfigPage::FillItemSet( SfxItemSet* )
{
    bool result = false;

    for (int i = 0, nCount = m_xSaveInListBox->get_count(); i < nCount; ++i)
    {
        SaveInData* pData =
            reinterpret_cast<SaveInData*>(m_xSaveInListBox->get_id(i).toInt64());

        result = pData->Apply();
    }
    return result;
}

IMPL_LINK_NOARG(SvxConfigPage, SelectSaveInLocation, weld::ComboBox&, void)
{
    pCurrentSaveInData = reinterpret_cast<SaveInData*>(m_xSaveInListBox->get_active_id().toInt64());
    Init();
}

void SvxConfigPage::ReloadTopLevelListBox( SvxConfigEntry const * pToSelect )
{
    int nSelectionPos = m_xTopLevelListBox->get_active();
    m_xTopLevelListBox->clear();

    if ( GetSaveInData() && GetSaveInData()->GetEntries() )
    {
        for (auto const& entryData : *GetSaveInData()->GetEntries())
        {
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(entryData)));
            m_xTopLevelListBox->append(sId, SvxConfigPageHelper::stripHotKey(entryData->GetName()));

            if (entryData == pToSelect)
                nSelectionPos = m_xTopLevelListBox->get_count() - 1;

            AddSubMenusToUI( SvxConfigPageHelper::stripHotKey( entryData->GetName() ), entryData );
        }
    }
#ifdef DBG_UTIL
    else
    {
        DBG_ASSERT( GetSaveInData(), "SvxConfigPage::ReloadTopLevelListBox(): no SaveInData" );
        DBG_ASSERT( GetSaveInData()->GetEntries() ,
            "SvxConfigPage::ReloadTopLevelListBox(): no SaveInData entries" );
    }
#endif

    nSelectionPos = (nSelectionPos != -1 && nSelectionPos < m_xTopLevelListBox->get_count()) ?
        nSelectionPos : m_xTopLevelListBox->get_count() - 1;

    m_xTopLevelListBox->set_active(nSelectionPos);
    SelectElement();
}

void SvxConfigPage::AddSubMenusToUI(
    const OUString& rBaseTitle, SvxConfigEntry const * pParentData )
{
    for (auto const& entryData : *pParentData->GetEntries())
    {
        if (entryData->IsPopup())
        {
            OUString subMenuTitle = rBaseTitle + aMenuSeparatorStr + SvxConfigPageHelper::stripHotKey(entryData->GetName());

            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(entryData)));
            m_xTopLevelListBox->append(sId, subMenuTitle);

            AddSubMenusToUI( subMenuTitle, entryData );
        }
    }
}

SvxEntries* SvxConfigPage::FindParentForChild(
    SvxEntries* pRootEntries, SvxConfigEntry* pChildData )
{
    for (auto const& entryData : *pRootEntries)
    {

        if (entryData == pChildData)
        {
            return pRootEntries;
        }
        else if (entryData->IsPopup())
        {
            SvxEntries* result =
                FindParentForChild( entryData->GetEntries(), pChildData );

            if ( result != nullptr )
            {
                return result;
            }
        }
    }
    return nullptr;
}

int SvxConfigPage::AddFunction(int nTarget, bool bFront, bool bAllowDuplicates)
{
    OUString aURL = GetScriptURL();
    SvxConfigEntry* pParent = GetTopLevelSelection();

    if ( aURL.isEmpty() || pParent == nullptr )
    {
        return -1;
    }

    OUString aDisplayName;
    OUString aModuleId = vcl::CommandInfoProvider::GetModuleIdentifier( m_xFrame );

    if ( typeid(*pCurrentSaveInData) == typeid(ContextMenuSaveInData) )
        aDisplayName = vcl::CommandInfoProvider::GetPopupLabelForCommand( aURL, aModuleId );
    else if ( typeid(*pCurrentSaveInData) == typeid(MenuSaveInData) )
        aDisplayName = vcl::CommandInfoProvider::GetMenuLabelForCommand( aURL, aModuleId );
    else
        aDisplayName = vcl::CommandInfoProvider::GetLabelForCommand( aURL, aModuleId );

    SvxConfigEntry* pNewEntryData =
        new SvxConfigEntry( aDisplayName, aURL, false, /*bParentData*/false );
    pNewEntryData->SetUserDefined();

    if ( aDisplayName.isEmpty() )
        pNewEntryData->SetName( GetSelectedDisplayName() );

    // check that this function is not already in the menu
    if ( !bAllowDuplicates )
    {
        for (auto const& entry : *pParent->GetEntries())
        {
            if ( entry->GetCommand() == pNewEntryData->GetCommand() )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetDialogFrameWeld(),
                                                          VclMessageType::Info, VclButtonsType::Ok, CuiResId(RID_SVXSTR_MNUCFG_ALREADY_INCLUDED)));
                xBox->run();
                delete pNewEntryData;
                return -1;
            }
        }
    }

    return InsertEntry(pNewEntryData, nTarget, bFront);
}

int SvxConfigPage::InsertEntry(
    SvxConfigEntry* pNewEntryData,
    int nTarget,
    bool bFront)
{
    SvxConfigEntry* pTopLevelSelection = GetTopLevelSelection();

    if (pTopLevelSelection == nullptr)
        return -1;

    // Grab the entries list for the currently selected menu
    SvxEntries* pEntries = pTopLevelSelection->GetEntries();

    int nNewEntry = -1;
    int nCurEntry =
        nTarget != -1 ? nTarget : m_xContentsListBox->get_selected_index();

    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pNewEntryData)));

    if (bFront)
    {
        pEntries->insert( pEntries->begin(), pNewEntryData );
        m_xContentsListBox->insert(0, sId);
        nNewEntry = 0;
    }
    else if (nCurEntry == -1 || nCurEntry == m_xContentsListBox->n_children() - 1)
    {
        pEntries->push_back( pNewEntryData );
        m_xContentsListBox->insert(-1, sId);
        nNewEntry = m_xContentsListBox->n_children() - 1;
    }
    else
    {
        SvxConfigEntry* pEntryData =
            reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(nCurEntry).toInt64());

        SvxEntries::iterator iter = pEntries->begin();
        SvxEntries::const_iterator end = pEntries->end();

        // Advance the iterator to the data for currently selected entry
        sal_uInt16 nPos = 0;
        while (*iter != pEntryData && ++iter != end)
        {
            ++nPos;
        }

        // Now step past it to the entry after the currently selected one
        ++iter;
        ++nPos;

        // Now add the new entry to the UI and to the parent's list
        if ( iter != end )
        {
            pEntries->insert( iter, pNewEntryData );
            m_xContentsListBox->insert(nPos, sId);
            nNewEntry = nPos;
        }
    }

    if (nNewEntry != -1)
    {
        m_xContentsListBox->select(nNewEntry);
        m_xContentsListBox->scroll_to_row(nNewEntry);

        GetSaveInData()->SetModified();
        GetTopLevelSelection()->SetModified();
    }

    return nNewEntry;
}

void SvxConfigPage::InsertEntryIntoUI(SvxConfigEntry* pNewEntryData, int nPos, int nStartCol)
{
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pNewEntryData)));

    m_xContentsListBox->set_id(nPos, sId);

    if (pNewEntryData->IsSeparator())
    {
        m_xContentsListBox->set_text(nPos, "----------------------------------", nStartCol + 1);
    }
    else
    {
        auto xImage = GetSaveInData()->GetImage(pNewEntryData->GetCommand());
        if (xImage.is())
            m_xContentsListBox->set_image(nPos, xImage, nStartCol);
        OUString aName = SvxConfigPageHelper::stripHotKey( pNewEntryData->GetName() );
        m_xContentsListBox->set_text(nPos, aName, nStartCol + 1);
    }

    if (nStartCol == 0)  // menus
    {
        if (pNewEntryData->IsPopup() || pNewEntryData->GetStyle() & css::ui::ItemStyle::DROP_DOWN)
            m_xContentsListBox->set_dropdown(nPos, nStartCol + 2);
        else
            m_xContentsListBox->set_image(nPos, nullptr, nStartCol + 2);
    }
}

IMPL_LINK(SvxConfigPage, MoveHdl, weld::Button&, rButton, void)
{
    MoveEntry(&rButton == m_xMoveUpButton.get());
}

IMPL_LINK_NOARG(SvxConfigPage, FunctionDoubleClickHdl, weld::TreeView&, void)
{
    if (m_xAddCommandButton->get_sensitive())
        m_xAddCommandButton->clicked();
}

IMPL_LINK_NOARG(SvxConfigPage, SelectFunctionHdl, weld::TreeView&, void)
{
    // Store the tooltip of the description field at first run
    static const OUString sDescTooltip = m_xDescriptionField->get_tooltip_text();

    // GetScriptURL() returns a non-empty string if a
    // valid command is selected on the left box
    bool bIsValidCommand = !GetScriptURL().isEmpty();

    // Enable/disable Add and Remove buttons depending on current selection
    if (bIsValidCommand)
    {
        m_xAddCommandButton->set_sensitive(true);
        m_xRemoveCommandButton->set_sensitive(true);

        m_xDescriptionField->set_text(m_xFunctions->GetHelpText(false));
    }
    else
    {

        m_xAddCommandButton->set_sensitive(false);
        m_xRemoveCommandButton->set_sensitive(false);

        m_xDescriptionField->set_text("");
    }

    // Disable the description field and its label if the local help is not installed
    // And inform the user via tooltips
    if ( !SfxHelp::IsHelpInstalled() )
    {
        m_xDescriptionField->set_sensitive(false);
        m_xDescriptionFieldLb->set_sensitive(false);
        m_xDescriptionField->set_tooltip_text( sDescTooltip );
        m_xDescriptionFieldLb->set_tooltip_text( sDescTooltip );
    }
    else
    {
        m_xDescriptionField->set_sensitive(true);
        m_xDescriptionFieldLb->set_sensitive(true);
        m_xDescriptionField->set_tooltip_text("");
        m_xDescriptionFieldLb->set_tooltip_text("");
    }
}

IMPL_LINK_NOARG(SvxConfigPage, ImplUpdateDataHdl, Timer*, void)
{
    OUString aSearchTerm(m_xSearchEdit->get_text());
    m_xCommandCategoryListBox->categorySelected(m_xFunctions.get(), aSearchTerm, GetSaveInData());
}

IMPL_LINK_NOARG(SvxConfigPage, SearchUpdateHdl, weld::Entry&, void)
{
    m_aUpdateDataTimer.Start();
}

IMPL_LINK_NOARG(SvxConfigPage, FocusOut_Impl, weld::Widget&, void)
{
    if (m_aUpdateDataTimer.IsActive())
    {
        m_aUpdateDataTimer.Stop();
        m_aUpdateDataTimer.Invoke();
    }
}

void SvxConfigPage::MoveEntry(bool bMoveUp)
{
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();

    int nSourceEntry = rTreeView.get_selected_index();
    int nTargetEntry = -1;
    int nToSelect = -1;

    if (nSourceEntry == -1)
    {
        return;
    }

    if ( bMoveUp )
    {
        // Move Up is just a Move Down with the source and target reversed
        nTargetEntry = nSourceEntry;
        nSourceEntry = nTargetEntry - 1;
        nToSelect = nSourceEntry;
    }
    else
    {
        nTargetEntry = nSourceEntry + 1;
        nToSelect = nTargetEntry;
    }

    if (MoveEntryData(nSourceEntry, nTargetEntry))
    {
        rTreeView.swap(nSourceEntry, nTargetEntry);
        rTreeView.select(nToSelect);
        rTreeView.scroll_to_row(nToSelect);

        UpdateButtonStates();
    }
}

bool SvxConfigPage::MoveEntryData(int nSourceEntry, int nTargetEntry)
{
    //#i53677#
    if (nSourceEntry == -1 || nTargetEntry == -1)
    {
        return false;
    }

    // Grab the entries list for the currently selected menu
    SvxEntries* pEntries = GetTopLevelSelection()->GetEntries();

    SvxConfigEntry* pSourceData =
        reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(nSourceEntry).toInt64());

    SvxConfigEntry* pTargetData =
        reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(nTargetEntry).toInt64());

    if ( pSourceData != nullptr && pTargetData != nullptr )
    {
        // remove the source entry from our list
        SvxConfigPageHelper::RemoveEntry( pEntries, pSourceData );

        SvxEntries::iterator iter = pEntries->begin();
        SvxEntries::const_iterator end = pEntries->end();

        // advance the iterator to the position of the target entry
        while (*iter != pTargetData && ++iter != end) ;

        // insert the source entry at the position after the target
        pEntries->insert( ++iter, pSourceData );

        GetSaveInData()->SetModified();
        GetTopLevelSelection()->SetModified();

        return true;
    }

    return false;
}

SvxMainMenuOrganizerDialog::SvxMainMenuOrganizerDialog(
    weld::Window* pParent, SvxEntries* entries,
    SvxConfigEntry const * selection, bool bCreateMenu )
    : GenericDialogController(pParent, "cui/ui/movemenu.ui", "MoveMenuDialog")
    , m_xMenuBox(m_xBuilder->weld_widget("namebox"))
    , m_xMenuNameEdit(m_xBuilder->weld_entry("menuname"))
    , m_xMenuListBox(m_xBuilder->weld_tree_view("menulist"))
    , m_xMoveUpButton(m_xBuilder->weld_button("up"))
    , m_xMoveDownButton(m_xBuilder->weld_button("down"))
{
    m_xMenuListBox->set_size_request(-1, m_xMenuListBox->get_height_rows(12));

    // Copy the entries list passed in
    if ( entries != nullptr )
    {
        mpEntries.reset( new SvxEntries );
        for (auto const& entry : *entries)
        {
            m_xMenuListBox->append(OUString::number(reinterpret_cast<sal_uInt64>(entry)),
                                   SvxConfigPageHelper::stripHotKey(entry->GetName()));
            mpEntries->push_back(entry);
            if (entry == selection)
            {
                m_xMenuListBox->select(m_xMenuListBox->n_children() - 1);
            }
        }
    }

    if ( bCreateMenu )
    {
        // Generate custom name for new menu
        OUString prefix = CuiResId( RID_SVXSTR_NEW_MENU );

        OUString newname = SvxConfigPageHelper::generateCustomName( prefix, entries );
        OUString newurl = SvxConfigPageHelper::generateCustomMenuURL( mpEntries.get() );

        SvxConfigEntry* pNewEntryData =
            new SvxConfigEntry( newname, newurl, true, /*bParentData*/false );
        pNewEntryData->SetName( newname );
        pNewEntryData->SetUserDefined();
        pNewEntryData->SetMain();

        m_sNewMenuEntryId = OUString::number(reinterpret_cast<sal_uInt64>(pNewEntryData));
        m_xMenuListBox->append(m_sNewMenuEntryId,
                               SvxConfigPageHelper::stripHotKey(pNewEntryData->GetName()));
        m_xMenuListBox->select(m_xMenuListBox->n_children() - 1);

        if (mpEntries)
            mpEntries->push_back(pNewEntryData);

        m_xMenuNameEdit->set_text(newname);
        m_xMenuNameEdit->connect_changed(LINK(this, SvxMainMenuOrganizerDialog, ModifyHdl));
    }
    else
    {
        // hide name label and textfield
        m_xMenuBox->hide();
        // change the title
        m_xDialog->set_title(CuiResId(RID_SVXSTR_MOVE_MENU));
    }

    m_xMenuListBox->connect_changed(LINK(this, SvxMainMenuOrganizerDialog, SelectHdl));

    m_xMoveUpButton->connect_clicked(LINK( this, SvxMainMenuOrganizerDialog, MoveHdl));
    m_xMoveDownButton->connect_clicked(LINK( this, SvxMainMenuOrganizerDialog, MoveHdl));
}

SvxMainMenuOrganizerDialog::~SvxMainMenuOrganizerDialog()
{
}

IMPL_LINK_NOARG(SvxMainMenuOrganizerDialog, ModifyHdl, weld::Entry&, void)
{
    // if the Edit control is empty do not change the name
    if (m_xMenuNameEdit->get_text().isEmpty())
    {
        return;
    }

    SvxConfigEntry* pNewEntryData = reinterpret_cast<SvxConfigEntry*>(m_sNewMenuEntryId.toUInt64());
    pNewEntryData->SetName(m_xMenuNameEdit->get_text());

    const int nNewMenuPos = m_xMenuListBox->find_id(m_sNewMenuEntryId);
    const int nOldSelection = m_xMenuListBox->get_selected_index();
    m_xMenuListBox->remove(nNewMenuPos);
    m_xMenuListBox->insert(nNewMenuPos, pNewEntryData->GetName(), &m_sNewMenuEntryId, nullptr, nullptr);
    m_xMenuListBox->select(nOldSelection);
}

IMPL_LINK_NOARG(SvxMainMenuOrganizerDialog, SelectHdl, weld::TreeView&, void)
{
    UpdateButtonStates();
}

void SvxMainMenuOrganizerDialog::UpdateButtonStates()
{
    // Disable Up and Down buttons depending on current selection
    const int nSelected = m_xMenuListBox->get_selected_index();
    m_xMoveUpButton->set_sensitive(nSelected > 0);
    m_xMoveDownButton->set_sensitive(nSelected != -1 && nSelected < m_xMenuListBox->n_children() - 1);
}

IMPL_LINK( SvxMainMenuOrganizerDialog, MoveHdl, weld::Button&, rButton, void )
{
    int nSourceEntry = m_xMenuListBox->get_selected_index();
    if (nSourceEntry == -1)
        return;

    int nTargetEntry;

    if (&rButton == m_xMoveDownButton.get())
    {
        nTargetEntry = nSourceEntry + 1;
    }
    else
    {
        // Move Up is just a Move Down with the source and target reversed
        nTargetEntry = nSourceEntry - 1;
    }

    OUString sId = m_xMenuListBox->get_id(nSourceEntry);
    OUString sEntry = m_xMenuListBox->get_text(nSourceEntry);
    m_xMenuListBox->remove(nSourceEntry);
    m_xMenuListBox->insert(nTargetEntry, sEntry, &sId, nullptr, nullptr);
    m_xMenuListBox->select(nTargetEntry);

    UpdateButtonStates();
}

SvxConfigEntry* SvxMainMenuOrganizerDialog::GetSelectedEntry()
{
    const int nSelected(m_xMenuListBox->get_selected_index());
    if (nSelected == -1)
        return nullptr;
    return reinterpret_cast<SvxConfigEntry*>(m_xMenuListBox->get_id(nSelected).toUInt64());
}

SvxConfigEntry::SvxConfigEntry( const OUString& rDisplayName,
                                const OUString& rCommandURL, bool bPopup, bool bParentData )
    : nId( 1 )
    , aLabel(rDisplayName)
    , aCommand(rCommandURL)
    , bPopUp(bPopup)
    , bStrEdited( false )
    , bIsUserDefined( false )
    , bIsMain( false )
    , bIsParentData( bParentData )
    , bIsModified( false )
    , bIsVisible( true )
    , nStyle( 0 )
{
    if (bPopUp)
    {
        mpEntries.reset( new SvxEntries );
    }
}

SvxConfigEntry::~SvxConfigEntry()
{
    if (mpEntries)
    {
        for (auto const& entry : *mpEntries)
        {
            delete entry;
        }
    }
}

bool SvxConfigEntry::IsMovable()
{
    return !IsPopup() || IsMain();
}

bool SvxConfigEntry::IsDeletable()
{
    return !IsMain() || IsUserDefined();
}

bool SvxConfigEntry::IsRenamable()
{
    return !IsMain() || IsUserDefined();
}

ToolbarSaveInData::ToolbarSaveInData(
    const uno::Reference < css::ui::XUIConfigurationManager >& xCfgMgr,
    const uno::Reference < css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool docConfig ) :

    SaveInData              ( xCfgMgr, xParentCfgMgr, aModuleId, docConfig ),
    m_aDescriptorContainer  ( ITEM_DESCRIPTOR_CONTAINER  )

{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    // Initialize the m_xPersistentWindowState variable which is used
    // to get the default properties of system toolbars such as name
    uno::Reference< container::XNameAccess > xPWSS = css::ui::theWindowStateConfiguration::get( xContext );

    xPWSS->getByName( aModuleId ) >>= m_xPersistentWindowState;
}

ToolbarSaveInData::~ToolbarSaveInData()
{
}

sal_Int32 ToolbarSaveInData::GetSystemStyle( const OUString& rResourceURL )
{
    sal_Int32 result = 0;

    if ( rResourceURL.startsWith( "private" ) &&
         m_xPersistentWindowState.is() &&
         m_xPersistentWindowState->hasByName( rResourceURL ) )
    {
        try
        {
            uno::Sequence< beans::PropertyValue > aProps;
            uno::Any a( m_xPersistentWindowState->getByName( rResourceURL ) );

            if ( a >>= aProps )
            {
                for ( sal_Int32 i = 0; i < aProps.getLength(); ++i )
                {
                    if ( aProps[ i ].Name == ITEM_DESCRIPTOR_STYLE )
                    {
                        aProps[i].Value >>= result;
                        break;
                    }
                }
            }
        }
        catch ( uno::Exception& )
        {
            // do nothing, a default value is returned
        }
    }

    return result;
}

void ToolbarSaveInData::SetSystemStyle(
    const uno::Reference< frame::XFrame >& xFrame,
    const OUString& rResourceURL,
    sal_Int32 nStyle )
{
    // change the style using the API
    SetSystemStyle( rResourceURL, nStyle );

    // this code is a temporary hack as the UI is not updating after
    // changing the toolbar style via the API
    uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    vcl::Window *window = nullptr;

    uno::Reference< beans::XPropertySet > xPropSet( xFrame, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        uno::Any a = xPropSet->getPropertyValue( "LayoutManager" );
        a >>= xLayoutManager;
    }

    if ( xLayoutManager.is() )
    {
        uno::Reference< css::ui::XUIElement > xUIElement =
            xLayoutManager->getElement( rResourceURL );

        // check reference before we call getRealInterface. The layout manager
        // can only provide references for elements that have been created
        // before. It's possible that the current element is not available.
        uno::Reference< css::awt::XWindow > xWindow;
        if ( xUIElement.is() )
            xWindow.set( xUIElement->getRealInterface(), uno::UNO_QUERY );

        window = VCLUnoHelper::GetWindow( xWindow ).get();
    }

    if ( window != nullptr && window->GetType() == WindowType::TOOLBOX )
    {
        ToolBox* toolbox = static_cast<ToolBox*>(window);

        if ( nStyle == 0 )
        {
            toolbox->SetButtonType( ButtonType::SYMBOLONLY );
        }
        else if ( nStyle == 1 )
        {
            toolbox->SetButtonType( ButtonType::TEXT );
        }
        if ( nStyle == 2 )
        {
            toolbox->SetButtonType( ButtonType::SYMBOLTEXT );
        }
    }
}

void ToolbarSaveInData::SetSystemStyle(
    const OUString& rResourceURL,
    sal_Int32 nStyle )
{
    if ( rResourceURL.startsWith( "private" ) &&
         m_xPersistentWindowState.is() &&
         m_xPersistentWindowState->hasByName( rResourceURL ) )
    {
        try
        {
            uno::Sequence< beans::PropertyValue > aProps;

            uno::Any a( m_xPersistentWindowState->getByName( rResourceURL ) );

            if ( a >>= aProps )
            {
                for ( sal_Int32 i = 0; i < aProps.getLength(); ++i )
                {
                    if ( aProps[ i ].Name == ITEM_DESCRIPTOR_STYLE )
                    {
                        aProps[ i ].Value <<= nStyle;
                        break;
                    }
                }
            }

            uno::Reference< container::XNameReplace >
                xNameReplace( m_xPersistentWindowState, uno::UNO_QUERY );

            xNameReplace->replaceByName( rResourceURL, uno::Any( aProps ) );
        }
        catch ( uno::Exception& )
        {
            // do nothing, a default value is returned
            SAL_WARN("cui.customize", "Exception setting toolbar style");
        }
    }
}

OUString ToolbarSaveInData::GetSystemUIName( const OUString& rResourceURL )
{
    OUString result;

    if ( rResourceURL.startsWith( "private" ) &&
         m_xPersistentWindowState.is() &&
         m_xPersistentWindowState->hasByName( rResourceURL ) )
    {
        try
        {
            uno::Sequence< beans::PropertyValue > aProps;
            uno::Any a( m_xPersistentWindowState->getByName( rResourceURL ) );

            if ( a >>= aProps )
            {
                for ( sal_Int32 i = 0; i < aProps.getLength(); ++i )
                {
                    if ( aProps[ i ].Name == ITEM_DESCRIPTOR_UINAME )
                    {
                        aProps[ i ].Value >>= result;
                    }
                }
            }
        }
        catch ( uno::Exception& )
        {
            // do nothing, an empty UIName will be returned
        }
    }

    if ( rResourceURL.startsWith( ".uno" ) &&
         m_xCommandToLabelMap.is() &&
         m_xCommandToLabelMap->hasByName( rResourceURL ) )
    {
        uno::Any a;
        try
        {
            a = m_xCommandToLabelMap->getByName( rResourceURL );

            uno::Sequence< beans::PropertyValue > aPropSeq;
            if ( a >>= aPropSeq )
            {
                for ( sal_Int32 i = 0; i < aPropSeq.getLength(); ++i )
                {
                    if ( aPropSeq[i].Name == ITEM_DESCRIPTOR_LABEL )
                    {
                        aPropSeq[i].Value >>= result;
                    }
                }
            }
        }
        catch ( uno::Exception& )
        {
            // not a system command name
        }
    }

    return result;
}

SvxEntries* ToolbarSaveInData::GetEntries()
{
    typedef std::unordered_map<OUString, bool > ToolbarInfo;

    ToolbarInfo aToolbarInfo;

    if ( pRootEntry == nullptr )
    {

        pRootEntry.reset( new SvxConfigEntry( "MainToolbars", OUString(), true, /*bParentData*/false) );

        uno::Sequence< uno::Sequence < beans::PropertyValue > > info =
            GetConfigManager()->getUIElementsInfo(
                css::ui::UIElementType::TOOLBAR );

        for ( sal_Int32 i = 0; i < info.getLength(); ++i )
        {
            uno::Sequence< beans::PropertyValue > props = info[ i ];

            OUString url;
            OUString systemname;
            OUString uiname;

            for ( sal_Int32 j = 0; j < props.getLength(); ++j )
            {
                if ( props[ j ].Name == ITEM_DESCRIPTOR_RESOURCEURL )
                {
                    props[ j ].Value >>= url;
                    systemname = url.copy( url.lastIndexOf( '/' ) + 1 );
                }
                else if ( props[ j ].Name == ITEM_DESCRIPTOR_UINAME )
                {
                    props[ j ].Value >>= uiname;
                }
            }

            try
            {
                uno::Reference< container::XIndexAccess > xToolbarSettings =
                    GetConfigManager()->getSettings( url, false );

                if ( uiname.isEmpty() )
                {
                    // try to get the name from m_xPersistentWindowState
                    uiname = GetSystemUIName( url );

                    if ( uiname.isEmpty() )
                    {
                        uiname = systemname;
                    }
                }

                SvxConfigEntry* pEntry = new SvxConfigEntry(
                    uiname, url, true, /*bParentData*/false );

                pEntry->SetMain();
                pEntry->SetStyle( GetSystemStyle( url ) );


                // insert into std::unordered_map to filter duplicates from the parent
                aToolbarInfo.emplace( systemname, true );

                OUString custom(CUSTOM_TOOLBAR_STR);
                if ( systemname.startsWith( custom ) )
                {
                    pEntry->SetUserDefined();
                }
                else
                {
                    pEntry->SetUserDefined( false );
                }

                pRootEntry->GetEntries()->push_back( pEntry );

                LoadToolbar( xToolbarSettings, pEntry );
            }
            catch ( container::NoSuchElementException& )
            {
                // TODO, handle resourceURL with no settings
            }
        }

        uno::Reference< css::ui::XUIConfigurationManager > xParentCfgMgr = GetParentConfigManager();
        if ( xParentCfgMgr.is() )
        {
            // Retrieve also the parent toolbars to make it possible
            // to configure module toolbars and save them into the document
            // config manager.
            uno::Sequence< uno::Sequence < beans::PropertyValue > > info_ =
                xParentCfgMgr->getUIElementsInfo(
                    css::ui::UIElementType::TOOLBAR );

            for ( sal_Int32 i = 0; i < info_.getLength(); ++i )
            {
                uno::Sequence< beans::PropertyValue > props = info_[ i ];

                OUString url;
                OUString systemname;
                OUString uiname;

                for ( sal_Int32 j = 0; j < props.getLength(); ++j )
                {
                    if ( props[ j ].Name == ITEM_DESCRIPTOR_RESOURCEURL )
                    {
                        props[ j ].Value >>= url;
                        systemname = url.copy( url.lastIndexOf( '/' ) + 1 );
                    }
                    else if ( props[ j ].Name == ITEM_DESCRIPTOR_UINAME )
                    {
                        props[ j ].Value >>= uiname;
                    }
                }

                // custom toolbars of the parent are not visible in the document layer
                OUString custom(CUSTOM_TOOLBAR_STR);
                if ( systemname.startsWith( custom ) )
                    continue;

                // check if toolbar is already in the document layer
                ToolbarInfo::const_iterator pIter = aToolbarInfo.find( systemname );
                if ( pIter == aToolbarInfo.end() )
                {
                    aToolbarInfo.emplace( systemname, true );

                    try
                    {
                        uno::Reference< container::XIndexAccess > xToolbarSettings =
                            xParentCfgMgr->getSettings( url, false );

                        if ( uiname.isEmpty() )
                        {
                            // try to get the name from m_xPersistentWindowState
                            uiname = GetSystemUIName( url );

                            if ( uiname.isEmpty() )
                            {
                                uiname = systemname;
                            }
                        }

                        SvxConfigEntry* pEntry = new SvxConfigEntry(
                            uiname, url, true, true );

                        pEntry->SetMain();
                        pEntry->SetStyle( GetSystemStyle( url ) );

                        if ( systemname.startsWith( custom ) )
                        {
                            pEntry->SetUserDefined();
                        }
                        else
                        {
                            pEntry->SetUserDefined( false );
                        }

                        pRootEntry->GetEntries()->push_back( pEntry );

                        LoadToolbar( xToolbarSettings, pEntry );
                    }
                    catch ( container::NoSuchElementException& )
                    {
                        // TODO, handle resourceURL with no settings
                    }
                }
            }
        }

        std::sort( GetEntries()->begin(), GetEntries()->end(), SvxConfigPageHelper::EntrySort );
    }

    return pRootEntry->GetEntries();
}

void
ToolbarSaveInData::SetEntries( std::unique_ptr<SvxEntries> pNewEntries )
{
    pRootEntry->SetEntries( std::move(pNewEntries) );
}

bool
ToolbarSaveInData::HasURL( const OUString& rURL )
{
    for (auto const& entry : *GetEntries())
    {
        if (entry->GetCommand() == rURL)
        {
            return !entry->IsParentData();
        }
    }
    return false;
}

bool ToolbarSaveInData::HasSettings()
{
    // return true if there is at least one toolbar entry
    return !GetEntries()->empty();
}

void ToolbarSaveInData::Reset()
{
    // reset each toolbar by calling removeSettings for its toolbar URL
    for (auto const& entry : *GetEntries())
    {
        try
        {
            const OUString& url = entry->GetCommand();
            GetConfigManager()->removeSettings( url );
        }
        catch ( uno::Exception& )
        {
            // error occurred removing the settings
            // TODO - add error dialog in future?
        }
    }

    // persist changes to toolbar storage
    PersistChanges( GetConfigManager() );

    // now delete the root SvxConfigEntry the next call to GetEntries()
    // causes it to be reinitialised
    pRootEntry.reset();

    // reset all icons to default
    try
    {
        GetImageManager()->reset();
        PersistChanges( GetImageManager() );
    }
    catch ( uno::Exception& )
    {
        SAL_WARN("cui.customize", "Error resetting all icons when resetting toolbars");
    }
}

bool ToolbarSaveInData::Apply()
{
    // toolbar changes are instantly applied
    return false;
}

void ToolbarSaveInData::ApplyToolbar(
    uno::Reference< container::XIndexContainer > const & rToolbarBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory,
    SvxConfigEntry const * pToolbarData )
{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();

    for (auto const& entry : *pToolbarData->GetEntries())
    {
        if (entry->IsPopup())
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                SvxConfigPageHelper::ConvertToolbarEntry(entry);

            uno::Reference< container::XIndexContainer > xSubMenuBar(
                rFactory->createInstanceWithContext( xContext ),
                    uno::UNO_QUERY );

            sal_Int32 nIndex = aPropValueSeq.getLength();
            aPropValueSeq.realloc( nIndex + 1 );
            aPropValueSeq[nIndex].Name = m_aDescriptorContainer;
            aPropValueSeq[nIndex].Value <<= xSubMenuBar;
            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::Any( aPropValueSeq ));

            ApplyToolbar(xSubMenuBar, rFactory, entry);
        }
        else if (entry->IsSeparator())
        {
            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::Any( m_aSeparatorSeq ));
        }
        else
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                SvxConfigPageHelper::ConvertToolbarEntry(entry);

            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::Any( aPropValueSeq ));
        }
    }
}

void ToolbarSaveInData::ApplyToolbar( SvxConfigEntry* pToolbar )
{
    // Apply new toolbar structure to our settings container
    uno::Reference< container::XIndexAccess > xSettings(
        GetConfigManager()->createSettings(), uno::UNO_QUERY );

    uno::Reference< container::XIndexContainer > xIndexContainer (
        xSettings, uno::UNO_QUERY );

    uno::Reference< lang::XSingleComponentFactory > xFactory (
        xSettings, uno::UNO_QUERY );

    ApplyToolbar( xIndexContainer, xFactory, pToolbar );

    uno::Reference< beans::XPropertySet > xProps(
        xSettings, uno::UNO_QUERY );

    if ( pToolbar->IsUserDefined() )
    {
        xProps->setPropertyValue(
            ITEM_DESCRIPTOR_UINAME,
            uno::Any( pToolbar->GetName() ) );
    }

    try
    {
        if ( GetConfigManager()->hasSettings( pToolbar->GetCommand() ) )
        {
            GetConfigManager()->replaceSettings(
                pToolbar->GetCommand(), xSettings );
        }
        else
        {
            GetConfigManager()->insertSettings(
                pToolbar->GetCommand(), xSettings );
            if ( pToolbar->IsParentData() )
                pToolbar->SetParentData( false );
        }
    }
    catch ( css::uno::Exception const & )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("cui.customize", "caught exception saving settings " << exceptionToString(ex));
    }

    PersistChanges( GetConfigManager() );
}

void ToolbarSaveInData::CreateToolbar( SvxConfigEntry* pToolbar )
{
    // show the new toolbar in the UI also
    uno::Reference< container::XIndexAccess >
        xSettings( GetConfigManager()->createSettings(), uno::UNO_QUERY );

    uno::Reference< beans::XPropertySet >
        xPropertySet( xSettings, uno::UNO_QUERY );

    xPropertySet->setPropertyValue(
            ITEM_DESCRIPTOR_UINAME,
            uno::Any( pToolbar->GetName() ) );

    try
    {
        GetConfigManager()->insertSettings( pToolbar->GetCommand(), xSettings );
    }
    catch ( css::uno::Exception const & )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("cui.customize", "caught exception saving settings " << exceptionToString(ex));
    }

    GetEntries()->push_back( pToolbar );

    PersistChanges( GetConfigManager() );
}

void ToolbarSaveInData::RemoveToolbar( SvxConfigEntry* pToolbar )
{
    try
    {
        OUString url = pToolbar->GetCommand();
        GetConfigManager()->removeSettings( url );
        SvxConfigPageHelper::RemoveEntry( GetEntries(), pToolbar );
        delete pToolbar;

        PersistChanges( GetConfigManager() );

        // remove the persistent window state data
        css::uno::Reference< css::container::XNameContainer > xNameContainer(
            m_xPersistentWindowState, css::uno::UNO_QUERY_THROW );

        xNameContainer->removeByName( url );
    }
    catch ( uno::Exception& )
    {
        // error occurred removing the settings
    }
}

void ToolbarSaveInData::RestoreToolbar( SvxConfigEntry* pToolbar )
{
    OUString url = pToolbar->GetCommand();

    // Restore of toolbar is done by removing it from
    // its configuration manager and then getting it again
    bool bParentToolbar = pToolbar->IsParentData();

    // Cannot restore parent toolbar
    if ( bParentToolbar )
        return;

    try
    {
        GetConfigManager()->removeSettings( url );
        pToolbar->GetEntries()->clear();
        PersistChanges( GetConfigManager() );
    }
    catch ( uno::Exception& )
    {
        // if an error occurs removing the settings then just return
        return;
    }

    // Now reload the toolbar settings
    try
    {
        uno::Reference< container::XIndexAccess > xToolbarSettings;
        if ( IsDocConfig() )
        {
            xToolbarSettings = GetParentConfigManager()->getSettings( url, false );
            pToolbar->SetParentData();
        }
        else
            xToolbarSettings = GetConfigManager()->getSettings( url, false );

        LoadToolbar( xToolbarSettings, pToolbar );

        // After reloading, ensure that the icon is reset of each entry
        // in the toolbar
        uno::Sequence< OUString > aURLSeq( 1 );
        for (auto const& entry : *pToolbar->GetEntries())
        {
            aURLSeq[ 0 ] = entry->GetCommand();

            try
            {
                GetImageManager()->removeImages( SvxConfigPageHelper::GetImageType(), aURLSeq );
            }
            catch ( uno::Exception& )
            {
                SAL_WARN("cui.customize", "Error restoring icon when resetting toolbar");
            }
        }
        PersistChanges( GetImageManager() );
    }
    catch ( container::NoSuchElementException& )
    {
        // cannot find the resource URL after removing it
        // so no entry will appear in the toolbar list
    }
}

void ToolbarSaveInData::LoadToolbar(
    const uno::Reference< container::XIndexAccess >& xToolbarSettings,
    SvxConfigEntry const * pParentData )
{
    SvxEntries*         pEntries            = pParentData->GetEntries();

    for ( sal_Int32 nIndex = 0; nIndex < xToolbarSettings->getCount(); ++nIndex )
    {
        OUString                aCommandURL;
        OUString                aLabel;
        bool                bIsVisible;
        sal_Int32               nStyle;

        sal_uInt16 nType( css::ui::ItemType::DEFAULT );

        bool bItem = SvxConfigPageHelper::GetToolbarItemData( xToolbarSettings, nIndex, aCommandURL,
            aLabel, nType, bIsVisible, nStyle );

        if ( bItem )
        {
            bool bIsUserDefined = true;

            if ( nType == css::ui::ItemType::DEFAULT )
            {
                uno::Any a;
                try
                {
                    a = m_xCommandToLabelMap->getByName( aCommandURL );
                    bIsUserDefined = false;
                }
                catch ( container::NoSuchElementException& )
                {
                    bIsUserDefined = true;
                }

                bool bUseDefaultLabel = false;
                // If custom label not set retrieve it from the command
                // to info service
                if ( aLabel.isEmpty() )
                {
                    bUseDefaultLabel = true;
                    uno::Sequence< beans::PropertyValue > aPropSeq;
                    if ( a >>= aPropSeq )
                    {
                        for ( sal_Int32 i = 0; i < aPropSeq.getLength(); ++i )
                        {
                            if ( aPropSeq[i].Name == "Name" )
                            {
                                aPropSeq[i].Value >>= aLabel;
                                break;
                            }
                        }
                    }
                }

                SvxConfigEntry* pEntry = new SvxConfigEntry(
                    aLabel, aCommandURL, false, /*bParentData*/false );

                pEntry->SetUserDefined( bIsUserDefined );
                pEntry->SetVisible( bIsVisible );
                pEntry->SetStyle( nStyle );

                if ( !bUseDefaultLabel )
                    pEntry->SetName( aLabel );

                pEntries->push_back( pEntry );
            }
            else
            {
                SvxConfigEntry* pEntry = new SvxConfigEntry;
                pEntry->SetUserDefined( bIsUserDefined );
                pEntries->push_back( pEntry );
            }
        }
    }
}

SvxNewToolbarDialog::SvxNewToolbarDialog(weld::Window* pWindow, const OUString& rName)
    : GenericDialogController(pWindow, "cui/ui/newtoolbardialog.ui", "NewToolbarDialog")
    , m_xEdtName(m_xBuilder->weld_entry("edit"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
    , m_xSaveInListBox(m_xBuilder->weld_combo_box("savein"))
{
    m_xEdtName->set_text(rName);
    m_xEdtName->select_region(0, -1);
}

SvxNewToolbarDialog::~SvxNewToolbarDialog()
{
}

/*******************************************************************************
*
* The SvxIconSelectorDialog class
*
*******************************************************************************/
SvxIconSelectorDialog::SvxIconSelectorDialog(weld::Window *pWindow,
    const uno::Reference< css::ui::XImageManager >& rXImageManager,
    const uno::Reference< css::ui::XImageManager >& rXParentImageManager)
    : GenericDialogController(pWindow, "cui/ui/iconselectordialog.ui", "IconSelector")
    , m_xImageManager(rXImageManager)
    , m_xParentImageManager(rXParentImageManager)
    , m_xTbSymbol(new SvtValueSet(m_xBuilder->weld_scrolled_window("symbolswin")))
    , m_xTbSymbolWin(new weld::CustomWeld(*m_xBuilder, "symbolsToolbar", *m_xTbSymbol))
    , m_xFtNote(m_xBuilder->weld_label("noteLabel"))
    , m_xBtnImport(m_xBuilder->weld_button("importButton"))
    , m_xBtnDelete(m_xBuilder->weld_button("deleteButton"))
{
    typedef std::unordered_map< OUString, bool > ImageInfo;

    m_nExpectedSize = 16;
    if (SvxConfigPageHelper::GetImageType() & css::ui::ImageType::SIZE_LARGE)
        m_nExpectedSize = 24;
    else if (SvxConfigPageHelper::GetImageType() & css::ui::ImageType::SIZE_32)
        m_nExpectedSize = 32;

    if ( m_nExpectedSize != 16 )
    {
        m_xFtNote->set_label(SvxConfigPageHelper::replaceSixteen(m_xFtNote->get_label(), m_nExpectedSize));
    }

    m_xTbSymbol->SetStyle(m_xTbSymbol->GetStyle() | WB_VSCROLL);
    m_xTbSymbol->SetColCount(11);
    m_xTbSymbol->SetLineCount(5);
    m_xTbSymbol->SetItemWidth(m_nExpectedSize);
    m_xTbSymbol->SetItemHeight(m_nExpectedSize);
    m_xTbSymbol->SetExtraSpacing(6);
    Size aSize(m_xTbSymbol->CalcWindowSizePixel(Size(m_nExpectedSize, m_nExpectedSize), 11, 5));
    m_xTbSymbol->set_size_request(aSize.Width(), aSize.Height());

    uno::Reference< uno::XComponentContext > xComponentContext =
        ::comphelper::getProcessComponentContext();

    m_xGraphProvider.set( graphic::GraphicProvider::create( xComponentContext ) );

    uno::Reference< css::util::XPathSettings > xPathSettings =
        css::util::thePathSettings::get( xComponentContext );


    OUString aDirectory = xPathSettings->getUserConfig();

    sal_Int32 aCount = aDirectory.getLength();

    if ( aCount > 0 )
    {
        sal_Unicode aChar = aDirectory[ aCount-1 ];
        if ( aChar != '/')
        {
            aDirectory += "/";
        }
    }
    else
    {
        m_xBtnImport->set_sensitive(false);
    }

    aDirectory += "soffice.cfg/import";

    uno::Reference< lang::XSingleServiceFactory > xStorageFactory(
          css::embed::FileSystemStorageFactory::create( xComponentContext ) );

    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[ 0 ] <<= aDirectory;
    aArgs[ 1 ] <<= css::embed::ElementModes::READWRITE;

    uno::Reference< css::embed::XStorage > xStorage(
        xStorageFactory->createInstanceWithArguments( aArgs ), uno::UNO_QUERY );

    uno::Sequence<uno::Any> aProp(comphelper::InitAnyPropertySequence(
    {
        {"UserConfigStorage", uno::Any(xStorage)},
        {"OpenMode", uno::Any(css::embed::ElementModes::READWRITE)}
    }));
    m_xImportedImageManager = css::ui::ImageManager::create( xComponentContext );
    m_xImportedImageManager->initialize(aProp);

    ImageInfo aImageInfo1;
    uno::Sequence< OUString > names;
    if ( m_xImportedImageManager.is() )
    {
        names = m_xImportedImageManager->getAllImageNames( SvxConfigPageHelper::GetImageType() );
        for ( sal_Int32 n = 0; n < names.getLength(); ++n )
            aImageInfo1.emplace( names[n], false );
    }

    uno::Sequence< OUString > name( 1 );
    for (auto const& elem : aImageInfo1)
    {
        name[ 0 ] = elem.first;
        uno::Sequence< uno::Reference< graphic::XGraphic> > graphics = m_xImportedImageManager->getImages( SvxConfigPageHelper::GetImageType(), name );
        if ( graphics.getLength() > 0 )
        {
            m_aGraphics.push_back(graphics[0]);
            Image img = Image(graphics[0]);
            m_xTbSymbol->InsertItem(m_aGraphics.size(), img, elem.first);
        }
    }

    ImageInfo                 aImageInfo;

    if ( m_xParentImageManager.is() )
    {
        names = m_xParentImageManager->getAllImageNames( SvxConfigPageHelper::GetImageType() );
        for ( sal_Int32 n = 0; n < names.getLength(); ++n )
            aImageInfo.emplace( names[n], false );
    }

    names = m_xImageManager->getAllImageNames( SvxConfigPageHelper::GetImageType() );
    for ( sal_Int32 n = 0; n < names.getLength(); ++n )
    {
        ImageInfo::iterator pIter = aImageInfo.find( names[n] );
        if ( pIter != aImageInfo.end() )
            pIter->second = true;
        else
            aImageInfo.emplace( names[n], true );
    }

    // large growth factor, expecting many entries
    for (auto const& elem : aImageInfo)
    {
        name[ 0 ] = elem.first;

        uno::Sequence< uno::Reference< graphic::XGraphic> > graphics;
        try
        {
            if (elem.second)
                graphics = m_xImageManager->getImages( SvxConfigPageHelper::GetImageType(), name );
            else
                graphics = m_xParentImageManager->getImages( SvxConfigPageHelper::GetImageType(), name );
        }
        catch ( uno::Exception& )
        {
            // can't get sequence for this name so it will not be
            // added to the list
        }

        if ( graphics.getLength() > 0 )
        {
            Image img = Image(graphics[0]);
            if (!img.GetBitmapEx().IsEmpty())
            {
                m_aGraphics.push_back(graphics[0]);
                m_xTbSymbol->InsertItem(m_aGraphics.size(), img, elem.first);
            }
        }
    }

    m_xBtnDelete->set_sensitive( false );
    m_xTbSymbol->SetSelectHdl( LINK(this, SvxIconSelectorDialog, SelectHdl) );
    m_xBtnImport->connect_clicked( LINK(this, SvxIconSelectorDialog, ImportHdl) );
    m_xBtnDelete->connect_clicked( LINK(this, SvxIconSelectorDialog, DeleteHdl) );
}

SvxIconSelectorDialog::~SvxIconSelectorDialog()
{
}

uno::Reference< graphic::XGraphic> SvxIconSelectorDialog::GetSelectedIcon()
{
    uno::Reference<graphic::XGraphic> result;

    sal_uInt16 nId = m_xTbSymbol->GetSelectedItemId();

    if (nId)
    {
        result = m_aGraphics[nId - 1];
    }

    return result;
}

IMPL_LINK_NOARG(SvxIconSelectorDialog, SelectHdl, SvtValueSet*, void)
{
    sal_uInt16 nId = m_xTbSymbol->GetSelectedItemId();

    if (!nId)
    {
        m_xBtnDelete->set_sensitive(false);
        return;
    }

    OUString aSelImageText = m_xTbSymbol->GetItemText(nId);
    if (m_xImportedImageManager->hasImage(SvxConfigPageHelper::GetImageType(), aSelImageText))
    {
        m_xBtnDelete->set_sensitive(true);
    }
    else
    {
        m_xBtnDelete->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxIconSelectorDialog, ImportHdl, weld::Button&, void)
{
    sfx2::FileDialogHelper aImportDialog(
        css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
        FileDialogFlags::Graphic | FileDialogFlags::MultiSelection, m_xDialog.get());

    // disable the link checkbox in the dialog
    uno::Reference< css::ui::dialogs::XFilePickerControlAccess >
        xController( aImportDialog.GetFilePicker(), uno::UNO_QUERY);
    if ( xController.is() )
    {
        xController->enableControl(
            css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK,
            false);
    }

    aImportDialog.SetCurrentFilter(
        "PNG - Portable Network Graphic");

    if ( ERRCODE_NONE == aImportDialog.Execute() )
    {
        uno::Sequence< OUString > paths = aImportDialog.GetMPath();
        ImportGraphics ( paths );
    }
}

IMPL_LINK_NOARG(SvxIconSelectorDialog, DeleteHdl, weld::Button&, void)
{
    OUString message = CuiResId( RID_SVXSTR_DELETE_ICON_CONFIRM );

    std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(m_xDialog.get(),
                                               VclMessageType::Warning, VclButtonsType::OkCancel,
                                               message));
    if (xWarn->run() == RET_OK)
    {
        sal_uInt16 nId = m_xTbSymbol->GetSelectedItemId();

        OUString aSelImageText = m_xTbSymbol->GetItemText( nId );
        uno::Sequence< OUString > URLs { aSelImageText };
        m_xTbSymbol->RemoveItem(nId);
        m_xImportedImageManager->removeImages( SvxConfigPageHelper::GetImageType(), URLs );
        uno::Reference< css::ui::XUIConfigurationPersistence >
            xConfigPersistence( m_xImportedImageManager, uno::UNO_QUERY );
        if ( xConfigPersistence.is() && xConfigPersistence->isModified() )
        {
            xConfigPersistence->store();
        }
    }
}

bool SvxIconSelectorDialog::ReplaceGraphicItem(
    const OUString& aURL )
{
    uno::Sequence< OUString > URLs(1);
    uno::Sequence< uno::Reference<graphic::XGraphic > > aImportGraph( 1 );
    uno::Reference< css::ui::XUIConfigurationPersistence >
        xConfigPer( m_xImportedImageManager, uno::UNO_QUERY );

    uno::Reference< graphic::XGraphic > xGraphic;
    uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
    aMediaProps[0].Name = "URL";
    aMediaProps[0].Value <<= aURL;

    css::awt::Size aSize;
    bool bOK = false;
    try
    {
        xGraphic = m_xGraphProvider->queryGraphic( aMediaProps );

        uno::Reference< beans::XPropertySet > props =
            m_xGraphProvider->queryGraphicDescriptor( aMediaProps );
        uno::Any a = props->getPropertyValue( "SizePixel" );
        a >>= aSize;
        if (0 == aSize.Width || 0 == aSize.Height)
            return false;
        else
            bOK = true;
    }
    catch ( uno::Exception& )
    {
        return false;
    }

    bool   bResult( false );
    size_t nCount = m_xTbSymbol->GetItemCount();
    for (size_t n = 0; n < nCount; ++n)
    {
        sal_uInt16 nId = m_xTbSymbol->GetItemId( n );

        if ( m_xTbSymbol->GetItemText( nId ) == aURL )
        {
            try
            {
                // replace/insert image with provided URL
                size_t nPos = nId - 1;
                assert(nPos == m_xTbSymbol->GetItemPos(nId));
                m_xTbSymbol->RemoveItem(nId);
                aMediaProps[0].Value <<= aURL;

                Image aImage( xGraphic );
                if ( bOK && ((aSize.Width != m_nExpectedSize) || (aSize.Height != m_nExpectedSize)) )
                {
                    BitmapEx aBitmap = aImage.GetBitmapEx();
                    BitmapEx aBitmapex = BitmapEx::AutoScaleBitmap(aBitmap, m_nExpectedSize);
                    aImage = Image( aBitmapex);
                }
                m_xTbSymbol->InsertItem(nId, aImage, aURL, nPos); //modify

                m_aGraphics[nPos] = Graphic(aImage.GetBitmapEx()).GetXGraphic();

                URLs[0] = aURL;
                aImportGraph[ 0 ] = xGraphic;
                m_xImportedImageManager->replaceImages( SvxConfigPageHelper::GetImageType(), URLs, aImportGraph );
                xConfigPer->store();

                bResult = true;
                break;
            }
            catch ( css::uno::Exception& )
            {
                break;
            }
        }
    }

    return bResult;
}

namespace
{
    OUString ReplaceIconName(const OUString& rMessage)
    {
        OUString name;
        OUString message = CuiResId( RID_SVXSTR_REPLACE_ICON_WARNING );
        OUString placeholder("%ICONNAME" );
        sal_Int32 pos = message.indexOf( placeholder );
        if ( pos != -1 )
        {
            name = message.replaceAt(
                pos, placeholder.getLength(), rMessage );
        }
        return name;
    }

    class SvxIconReplacementDialog
    {
    private:
        std::unique_ptr<weld::MessageDialog> m_xQueryBox;
    public:
        SvxIconReplacementDialog(weld::Window *pParent, const OUString& rMessage, bool bYestoAll)
            : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Warning, VclButtonsType::NONE, ReplaceIconName(rMessage)))
        {
            m_xQueryBox->set_title(CuiResId(RID_SVXSTR_REPLACE_ICON_CONFIRM));
            m_xQueryBox->add_button(Button::GetStandardText(StandardButtonType::Yes), 2);
            if (bYestoAll)
                m_xQueryBox->add_button(CuiResId(RID_SVXSTR_YESTOALL), 5);
            m_xQueryBox->add_button(Button::GetStandardText(StandardButtonType::No), 4);
            m_xQueryBox->add_button(Button::GetStandardText(StandardButtonType::Cancel), 6);
            m_xQueryBox->set_default_response(2);
        }
        short run() { return m_xQueryBox->run(); }
    };
}

void SvxIconSelectorDialog::ImportGraphics(
    const uno::Sequence< OUString >& rPaths )
{
    uno::Sequence< OUString > rejected( rPaths.getLength() );
    sal_Int32 rejectedCount = 0;

    sal_uInt16 ret = 0;
    sal_Int32 aIndex;
    OUString aIconName;

    if ( rPaths.getLength() == 1 )
    {
        if ( m_xImportedImageManager->hasImage( SvxConfigPageHelper::GetImageType(), rPaths[0] ) )
        {
            aIndex = rPaths[0].lastIndexOf( '/' );
            aIconName = rPaths[0].copy( aIndex+1 );
            SvxIconReplacementDialog aDlg(m_xDialog.get(), aIconName, false);
            ret = aDlg.run();
            if ( ret == 2 )
            {
                ReplaceGraphicItem( rPaths[0] );
            }
        }
        else
        {
            if ( !ImportGraphic( rPaths[0] ) )
            {
                rejected[0] = rPaths[0];
                rejectedCount = 1;
            }
        }
    }
    else
    {
        OUString aSourcePath( rPaths[0] );
        if ( rPaths[0].lastIndexOf( '/' ) != rPaths[0].getLength() -1 )
            aSourcePath = rPaths[0] + "/";

        for ( sal_Int32 i = 1; i < rPaths.getLength(); ++i )
        {
            OUString aPath = aSourcePath + rPaths[i];
            if ( m_xImportedImageManager->hasImage( SvxConfigPageHelper::GetImageType(), aPath ) )
            {
                aIndex = rPaths[i].lastIndexOf( '/' );
                aIconName = rPaths[i].copy( aIndex+1 );
                SvxIconReplacementDialog aDlg(m_xDialog.get(), aIconName, true);
                ret = aDlg.run();
                if ( ret == 2 )
                {
                    ReplaceGraphicItem( aPath );
                }
                else if ( ret == 5 )
                {
                    for ( sal_Int32 k = i; k < rPaths.getLength(); ++k )
                    {
                        aPath = aSourcePath + rPaths[k];
                        bool bHasReplaced = ReplaceGraphicItem( aPath );

                        if ( !bHasReplaced )
                        {
                            bool result = ImportGraphic( aPath );
                            if ( !result )
                            {
                                rejected[ rejectedCount ] = rPaths[i];
                                ++rejectedCount;
                            }
                        }
                    }
                    break;
                }
            }
            else
            {
                bool result = ImportGraphic( aSourcePath + rPaths[i] );
                if ( !result )
                {
                    rejected[ rejectedCount ] = rPaths[i];
                    ++rejectedCount;
                }
            }
        }
    }

    if ( rejectedCount != 0 )
    {
        OUStringBuffer message;
        OUString fPath;
        if (rejectedCount > 1)
              fPath = rPaths[0].copy(8) + "/";
        for ( sal_Int32 i = 0; i < rejectedCount; ++i )
        {
            message.append(fPath).append(rejected[i]).append("\n");
        }

        SvxIconChangeDialog aDialog(m_xDialog.get(), message.makeStringAndClear());
        aDialog.run();
    }
}

bool SvxIconSelectorDialog::ImportGraphic( const OUString& aURL )
{
    bool result = false;

    uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
    aMediaProps[0].Name = "URL";

    uno::Reference< graphic::XGraphic > xGraphic;
    css::awt::Size aSize;
    aMediaProps[0].Value <<= aURL;
    try
    {
        uno::Reference< beans::XPropertySet > props =
            m_xGraphProvider->queryGraphicDescriptor( aMediaProps );

        uno::Any a = props->getPropertyValue("SizePixel");

        xGraphic = m_xGraphProvider->queryGraphic( aMediaProps );
        if ( xGraphic.is() )
        {
            bool bOK = true;

            a >>= aSize;
            if ( 0 == aSize.Width || 0 == aSize.Height )
                bOK = false;

            Image aImage( xGraphic );

            if ( bOK && ((aSize.Width != m_nExpectedSize) || (aSize.Height != m_nExpectedSize)) )
            {
                BitmapEx aBitmap = aImage.GetBitmapEx();
                BitmapEx aBitmapex = BitmapEx::AutoScaleBitmap(aBitmap, m_nExpectedSize);
                aImage = Image( aBitmapex);
            }
            if ( bOK && !!aImage )
            {
                m_aGraphics.push_back(Graphic(aImage.GetBitmapEx()).GetXGraphic());
                m_xTbSymbol->InsertItem(m_aGraphics.size(), aImage, aURL);

                uno::Sequence<OUString> aImportURL { aURL };
                uno::Sequence< uno::Reference<graphic::XGraphic > > aImportGraph( 1 );
                aImportGraph[ 0 ] = xGraphic;
                m_xImportedImageManager->insertImages( SvxConfigPageHelper::GetImageType(), aImportURL, aImportGraph );
                uno::Reference< css::ui::XUIConfigurationPersistence >
                xConfigPersistence( m_xImportedImageManager, uno::UNO_QUERY );

                if ( xConfigPersistence.is() && xConfigPersistence->isModified() )
                {
                    xConfigPersistence->store();
                }

                result = true;
            }
            else
            {
                SAL_WARN("cui.customize", "could not create Image from XGraphic");
            }
        }
        else
        {
                SAL_WARN("cui.customize", "could not get query XGraphic");
        }
    }
    catch( uno::Exception const & )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("cui.customize", "Caught exception importing XGraphic: " << exceptionToString(ex));
    }
    return result;
}

/*******************************************************************************
*
* The SvxIconChangeDialog class added for issue83555
*
*******************************************************************************/
SvxIconChangeDialog::SvxIconChangeDialog(weld::Window *pWindow, const OUString& rMessage)
    : MessageDialogController(pWindow, "cui/ui/iconchangedialog.ui", "IconChange", "grid")
    , m_xLineEditDescription(m_xBuilder->weld_text_view("addrTextview"))
{
    m_xLineEditDescription->set_size_request(m_xLineEditDescription->get_approximate_digit_width() * 48,
                                             m_xLineEditDescription->get_text_height() * 8);
    m_xLineEditDescription->set_text(rMessage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
