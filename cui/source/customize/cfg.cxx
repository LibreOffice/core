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
#include <typeinfo>

#include <utility>
#include <vcl/stdtext.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/weld.hxx>
#include <vcl/decoview.hxx>
#include <vcl/virdev.hxx>

#include <sfx2/minfitem.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <algorithm>
#include <strings.hrc>

#include <acccfg.hxx>
#include <cfg.hxx>
#include <CustomNotebookbarGenerator.hxx>
#include <SvxMenuConfigPage.hxx>
#include <SvxToolbarConfigPage.hxx>
#include <SvxNotebookbarConfigPage.hxx>
#include <SvxConfigPageHelper.hxx>
#include "eventdlg.hxx"
#include <dialmgr.hxx>

#include <unotools/configmgr.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/FileSystemStorageFactory.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/ImageManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <comphelper/documentinfo.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>
#include <config_features.h>

namespace uno = css::uno;
namespace frame = css::frame;
namespace lang = css::lang;
namespace container = css::container;
namespace beans = css::beans;
namespace graphic = css::graphic;

#if OSL_DEBUG_LEVEL > 1

void printPropertySet(
    const OUString& prefix,
    const uno::Reference< beans::XPropertySet >& xPropSet )
{
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();

    const uno::Sequence< beans::Property >& aPropDetails =
        xPropSetInfo->getProperties();

    SAL_WARN("cui", "printPropertySet: " << aPropDetails.getLength() << " properties" );

    for ( beans::Property const & aPropDetail  : aPropDetails )
    {
        OUString tmp;
        sal_Int32 ival;

        uno::Any a = xPropSet->getPropertyValue( aPropDetail.Name );

        if ( a >>= tmp )
        {
            SAL_WARN("cui", prefix << ": Got property: " << aPropDetail.Name << tmp);
        }
        else if ( ( a >>= ival ) )
        {
            SAL_WARN("cui", prefix << ": Got property: " << aPropDetail.Name << " = " << ival);
        }
        else
        {
            SAL_WARN("cui", prefix << ": Got property: " << aPropDetail.Name << " of type " << a.getValueTypeName());
        }
    }
}

void printProperties(
    const OUString& prefix,
    const uno::Sequence< beans::PropertyValue >& aProp )
{
    for (beans::PropertyValue const & aPropVal : aProp)
    {
        OUString tmp;

        aPropVal.Value >>= tmp;

        SAL_WARN("cui", prefix << ": Got property: " << aPropVal.Name << " = " << tmp);
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
SvxConfigPage::CanConfig( std::u16string_view aModuleId )
{
    return aModuleId != u"com.sun.star.script.BasicIDE" && aModuleId != u"com.sun.star.frame.Bibliography";
}

static std::unique_ptr<SfxTabPage> CreateSvxMenuConfigPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<SvxMenuConfigPage>(pPage, pController, *rSet);
}

static std::unique_ptr<SfxTabPage> CreateSvxContextMenuConfigPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<SvxMenuConfigPage>(pPage, pController, *rSet, false);
}

static std::unique_ptr<SfxTabPage> CreateKeyboardConfigPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
       return std::make_unique<SfxAcceleratorConfigPage>(pPage, pController, *rSet);
}

static std::unique_ptr<SfxTabPage> CreateSvxNotebookbarConfigPage(weld::Container* pPage, weld::DialogController* pController,
                                                         const SfxItemSet* rSet)
{
    return std::make_unique<SvxNotebookbarConfigPage>(pPage, pController, *rSet);
}

static std::unique_ptr<SfxTabPage> CreateSvxToolbarConfigPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<SvxToolbarConfigPage>(pPage, pController, *rSet);
}

static std::unique_ptr<SfxTabPage> CreateSvxEventConfigPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<SvxEventConfigPage>(pPage, pController, *rSet, SvxEventConfigPage::EarlyInit());
}

/******************************************************************************
 *
 * SvxConfigDialog is the configuration dialog which is brought up from the
 * Tools menu. It includes tabs for customizing menus, toolbars, events and
 * key bindings.
 *
 *****************************************************************************/
SvxConfigDialog::SvxConfigDialog(weld::Window * pParent, const SfxItemSet* pInSet)
    : SfxTabDialogController(pParent, u"cui/ui/customizedialog.ui"_ustr, u"CustomizeDialog"_ustr, pInSet)
{
    SvxConfigPageHelper::InitImageType();

    AddTabPage(u"menus"_ustr, CreateSvxMenuConfigPage, nullptr);
    AddTabPage(u"toolbars"_ustr, CreateSvxToolbarConfigPage, nullptr);
    AddTabPage(u"notebookbar"_ustr, CreateSvxNotebookbarConfigPage, nullptr);
    AddTabPage(u"contextmenus"_ustr, CreateSvxContextMenuConfigPage, nullptr);
    AddTabPage(u"keyboard"_ustr, CreateKeyboardConfigPage, nullptr);
    AddTabPage(u"events"_ustr, CreateSvxEventConfigPage, nullptr);

    if (const SfxPoolItem* pItem = pInSet->GetItem(SID_CONFIG))
    {
        OUString text = static_cast<const SfxStringItem*>(pItem)->GetValue();
        if (text.startsWith( ITEM_TOOLBAR_URL ) )
            SetCurPageId(u"toolbars"_ustr);
        else if (text.startsWith( ITEM_EVENT_URL) )
            SetCurPageId(u"events"_ustr);
    }
#if HAVE_FEATURE_SCRIPTING
    else if (pInSet->GetItemIfSet(SID_MACROINFO))
    {
        // for the "assign" button in the Basic Macros chooser automatically switch
        // to the keyboard tab in which this macro will be pre-selected for assigning
        // to a keystroke
        SetCurPageId(u"keyboard"_ustr);
    }
#endif
}

void SvxConfigDialog::ActivatePage(const OUString& rPage)
{
    SfxTabDialogController::ActivatePage(rPage);
    GetResetButton()->set_visible(rPage != "keyboard");
}

void SvxConfigDialog::SetFrame(const css::uno::Reference<css::frame::XFrame>& xFrame)
{
    m_xFrame = xFrame;
    OUString aModuleId = SvxConfigPage::GetFrameWithDefaultAndIdentify(m_xFrame);

    if (aModuleId != "com.sun.star.text.TextDocument" &&
        aModuleId != "com.sun.star.sheet.SpreadsheetDocument" &&
        aModuleId != "com.sun.star.presentation.PresentationDocument" &&
        aModuleId != "com.sun.star.drawing.DrawingDocument")
        RemoveTabPage(u"notebookbar"_ustr);

    if (aModuleId == "com.sun.star.frame.StartModule")
        RemoveTabPage(u"keyboard"_ustr);
}

void SvxConfigDialog::PageCreated(const OUString &rId, SfxTabPage& rPage)
{
    if (rId == "menus" || rId == "keyboard" || rId == "notebookbar"
        || rId == "toolbars" || rId == "contextmenus")
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
    uno::Reference< css::ui::XUIConfigurationManager > xCfgMgr,
    uno::Reference< css::ui::XUIConfigurationManager > xParentCfgMgr,
    const OUString& aModuleId,
    bool isDocConfig )
        :
            bModified( false ),
            bDocConfig( isDocConfig ),
            bReadOnly( false ),
            m_xCfgMgr(std::move( xCfgMgr )),
            m_xParentCfgMgr(std::move( xParentCfgMgr )),
            m_aSeparatorSeq{ comphelper::makePropertyValue(ITEM_DESCRIPTOR_TYPE,
                                                           css::ui::ItemType::SEPARATOR_LINE) }
{
    if ( bDocConfig )
    {
        uno::Reference< css::ui::XUIConfigurationPersistence >
            xDocPersistence( GetConfigManager(), uno::UNO_QUERY );

        bReadOnly = xDocPersistence->isReadOnly();
    }

    const uno::Reference<uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();

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
        m_xMenuSettings = GetConfigManager()->getSettings( ITEM_MENUBAR_URL, false );
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
        pRootEntry.reset( new SvxConfigEntry( u"MainMenus"_ustr, OUString(), true, /*bParentData*/false) );

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
                        for (const beans::PropertyValue& prop : aPropSeq)
                        {
                            if ( bContextMenu )
                            {
                                if ( prop.Name == "PopupLabel" )
                                {
                                    prop.Value >>= aLabel;
                                    break;
                                }
                                else if ( prop.Name == "Label" )
                                {
                                    prop.Value >>= aMenuLabel;
                                }
                            }
                            else if ( prop.Name == "Label" )
                            {
                                prop.Value >>= aLabel;
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
        m_xMenuSettings = GetConfigManager()->createSettings();

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
            TOOLS_WARN_EXCEPTION("cui.customize", "caught some other exception saving settings");
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
    const uno::Reference<uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();

    for (auto const& entryData : *GetEntries())
    {
        uno::Sequence< beans::PropertyValue > aPropValueSeq =
            SvxConfigPageHelper::ConvertSvxConfigEntry(entryData);

        uno::Reference< container::XIndexContainer > xSubMenuBar(
            rFactory->createInstanceWithContext( xContext ),
            uno::UNO_QUERY );

        sal_Int32 nIndex = aPropValueSeq.getLength();
        aPropValueSeq.realloc( nIndex + 1 );
        auto pPropValueSeq = aPropValueSeq.getArray();
        pPropValueSeq[nIndex].Name = m_aDescriptorContainer;
        pPropValueSeq[nIndex].Value <<= xSubMenuBar;
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
    const uno::Reference<uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();

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
            auto pPropValueSeq = aPropValueSeq.getArray();
            pPropValueSeq[nIndex].Name = ITEM_DESCRIPTOR_CONTAINER;
            pPropValueSeq[nIndex].Value <<= xSubMenuBar;

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
    const css::uno::Reference< css::uno::XComponentContext >& xContext( comphelper::getProcessComponentContext() );
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

        for (const auto& aProp : aProps)
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

        m_pRootEntry.reset( new SvxConfigEntry( u"ContextMenus"_ustr, OUString(), true, /*bParentData*/false ) );
        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aElementsInfo;
        try
        {
            aElementsInfo = GetConfigManager()->getUIElementsInfo( css::ui::UIElementType::POPUPMENU );
        }
        catch ( const css::lang::IllegalArgumentException& )
        {}

        for (const auto& aElement : aElementsInfo)
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

        for (const auto& aElement : aParentElementsInfo)
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
            css::uno::Reference< css::container::XIndexContainer > xIndexContainer = GetConfigManager()->createSettings();
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
        catch ( const css::uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("cui.customize", "Exception caught while resetting context menus");
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
    catch ( const css::uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("cui.customize", "Exception caught while resetting context menu");
    }
    PersistChanges( GetConfigManager() );
    m_pRootEntry.reset();
}

void SvxMenuEntriesListBox::CreateDropDown()
{
    int nWidth = (m_xControl->get_text_height() * 3) / 4;
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
    , m_pPage(pPg)
{
    m_xControl->enable_toggle_buttons(weld::ColumnToggleType::Check);
    CreateDropDown();
    m_xControl->connect_key_press(LINK(this, SvxMenuEntriesListBox, KeyInputHdl));
    m_xControl->connect_query_tooltip(LINK(this, SvxMenuEntriesListBox, QueryTooltip));
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
        m_pPage->DeleteSelectedContent();
    }
    // support CTRL+UP and CTRL+DOWN for moving selected entries
    else if ( keycode.GetCode() == KEY_UP && keycode.IsMod1() )
    {
        m_pPage->MoveEntry( true );
    }
    else if ( keycode.GetCode() == KEY_DOWN && keycode.IsMod1() )
    {
        m_pPage->MoveEntry( false );
    }
    else
    {
        return false; // pass on to default handler
    }
    return true;
}

IMPL_LINK(SvxMenuEntriesListBox, QueryTooltip, const weld::TreeIter&, rIter, OUString)
{
    SvxConfigEntry *pEntry = weld::fromId<SvxConfigEntry*>(m_xControl->get_id(rIter));
    if (!pEntry || pEntry->GetCommand().isEmpty())
        return OUString();
    const OUString sCommand(pEntry->GetCommand());
    OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(m_pPage->GetFrame()));
    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(sCommand, aModuleName);
    OUString sTooltipLabel = vcl::CommandInfoProvider::GetTooltipForCommand(sCommand, aProperties,
                                                                            m_pPage->GetFrame());
    return CuiResId(RID_CUISTR_COMMANDLABEL) + ": " + pEntry->GetName().replaceFirst("~", "") + "\n" +
            CuiResId(RID_CUISTR_COMMANDNAME) + ": " + sCommand + "\n" +
            CuiResId(RID_CUISTR_COMMANDTIP) + ": " + sTooltipLabel.replaceFirst("~", "");
}

/******************************************************************************
 *
 * SvxConfigPage is the abstract base class on which the Menu and Toolbar
 * configuration tabpages are based. It includes methods which are common to
 * both tabpages to add, delete, move and rename items etc.
 *
 *****************************************************************************/
SvxConfigPage::SvxConfigPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/menuassignpage.ui"_ustr, u"MenuAssignPage"_ustr, &rSet)
    , m_aUpdateDataTimer( "SvxConfigPage UpdateDataTimer" )
    , bInitialised(false)
    , pCurrentSaveInData(nullptr)
    , m_xCommandCategoryListBox(new CommandCategoryListBox(m_xBuilder->weld_combo_box(u"commandcategorylist"_ustr)))
    , m_xFunctions(new CuiConfigFunctionListBox(m_xBuilder->weld_tree_view(u"functions"_ustr)))
    , m_xCategoryLabel(m_xBuilder->weld_label(u"categorylabel"_ustr))
    , m_xDescriptionFieldLb(m_xBuilder->weld_label(u"descriptionlabel"_ustr))
    , m_xDescriptionField(m_xBuilder->weld_text_view(u"desc"_ustr))
    , m_xLeftFunctionLabel(m_xBuilder->weld_label(u"leftfunctionlabel"_ustr))
    , m_xSearchEdit(m_xBuilder->weld_entry(u"searchEntry"_ustr))
    , m_xSearchLabel(m_xBuilder->weld_label(u"searchlabel"_ustr))
    , m_xCustomizeLabel(m_xBuilder->weld_label(u"customizelabel"_ustr))
    , m_xTopLevelListBox(m_xBuilder->weld_combo_box(u"toplevellist"_ustr))
    , m_xMoveUpButton(m_xBuilder->weld_button(u"up"_ustr))
    , m_xMoveDownButton(m_xBuilder->weld_button(u"down"_ustr))
    , m_xSaveInListBox(m_xBuilder->weld_combo_box(u"savein"_ustr))
    , m_xCustomizeBox(m_xBuilder->weld_widget(u"customizebox"_ustr))
    , m_xInsertBtn(m_xBuilder->weld_menu_button(u"insert"_ustr))
    , m_xModifyBtn(m_xBuilder->weld_menu_button(u"modify"_ustr))
    , m_xResetBtn(m_xBuilder->weld_button(u"defaultsbtn"_ustr))
    , m_xCommandButtons(m_xBuilder->weld_widget(u"arrowgrid"_ustr))
    , m_xAddCommandButton(m_xBuilder->weld_button(u"add"_ustr))
    , m_xRemoveCommandButton(m_xBuilder->weld_button(u"remove"_ustr))
{
    CustomNotebookbarGenerator::getFileNameAndAppName(m_sAppName, m_sFileName);

    m_xTopLevelListBox->connect_changed(LINK(this, SvxConfigPage, SelectElementHdl));

    weld::TreeView& rTreeView = m_xFunctions->get_widget();
    Size aSize(rTreeView.get_approximate_digit_width() * 40, rTreeView.get_height_rows(8));
    m_xFunctions->set_size_request(aSize.Width(), aSize.Height());
    m_xDescriptionField->set_size_request(aSize.Width(), m_xDescriptionField->get_height_rows(3));

    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, SvxConfigPage, ImplUpdateDataHdl));
    m_aUpdateDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    m_xSearchEdit->connect_changed(LINK(this, SvxConfigPage, SearchUpdateHdl));
    m_xSearchEdit->connect_focus_out(LINK(this, SvxConfigPage, FocusOut_Impl));

    rTreeView.connect_row_activated(LINK(this, SvxConfigPage, FunctionDoubleClickHdl));
    rTreeView.connect_selection_changed(LINK(this, SvxConfigPage, SelectFunctionHdl));
}

IMPL_LINK_NOARG(SvxConfigPage, SelectElementHdl, weld::ComboBox&, void)
{
    SelectElement();
}

SvxConfigPage::~SvxConfigPage()
{
    int cnt = m_xSaveInListBox->get_count();
    for(int i=0; i < cnt; ++i)
    {
        SaveInData *pData = weld::fromId<SaveInData*>(m_xSaveInListBox->get_id(i));
        delete pData;
    }
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
            ::comphelper::getProcessComponentContext(), uno::UNO_SET_THROW );

        m_xFrame = GetFrame();
        m_aModuleId = GetFrameWithDefaultAndIdentify( m_xFrame );

        // replace %MODULENAME in the label with the correct module name
        uno::Reference< css::frame::XModuleManager2 > xModuleManager(
            css::frame::ModuleManager::create( xContext ));
        OUString aModuleName = SvxConfigPageHelper::GetUIModuleName( m_aModuleId, xModuleManager );

        uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier >
            xModuleCfgSupplier( css::ui::theModuleUIConfigurationManagerSupplier::get(xContext) );

        // Set up data for module specific menus
        SaveInData* pModuleData = nullptr;

        try
        {
            xCfgMgr =
                xModuleCfgSupplier->getUIConfigurationManager( m_aModuleId );

            pModuleData = CreateSaveInData( xCfgMgr,
                                            uno::Reference< css::ui::XUIConfigurationManager >(),
                                            m_aModuleId,
                                            false );
        }
        catch ( container::NoSuchElementException& )
        {
        }

        if ( pModuleData != nullptr )
        {
            OUString sId(weld::toId(pModuleData));
            m_xSaveInListBox->append(sId, utl::ConfigManager::getProductName() + " " + aModuleName);
        }

        // try to retrieve the document based ui configuration manager
        OUString aTitle;
        uno::Reference< frame::XController > xController =
            m_xFrame->getController();
        if ( CanConfig( m_aModuleId ) && xController.is() )
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
            pDocData = CreateSaveInData( xDocCfgMgr, xCfgMgr, m_aModuleId, true );

            if ( !pDocData->IsReadOnly() )
            {
                OUString sId(weld::toId(pDocData));
                m_xSaveInListBox->append(sId, aTitle);
                nPos = m_xSaveInListBox->get_count() - 1;
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

        if ( CanConfig( m_aModuleId ) )
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

            for (uno::Reference<frame::XFrame> const& xf : aFrameList)
            {
                if ( xf.is() && xf != m_xFrame )
                {
                    OUString aCheckId;
                    try{
                        aCheckId = xModuleManager->identify( xf );
                    } catch(const uno::Exception&)
                        { aCheckId.clear(); }

                    if ( m_aModuleId == aCheckId )
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
                            SaveInData* pData = CreateSaveInData( xDocCfgMgr, xCfgMgr, m_aModuleId, true );

                            if ( pData && !pData->IsReadOnly() )
                            {
                                OUString sId(weld::toId(pData));
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
        const uno::Reference< uno::XComponentContext >& xContext(
            ::comphelper::getProcessComponentContext() );

        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(
            xContext );

        if ( !_inout_rxFrame.is() )
            _inout_rxFrame = xDesktop->getActiveFrame();

        if ( !_inout_rxFrame.is() )
        {
            _inout_rxFrame = xDesktop->getCurrentFrame();
        }

        if ( !_inout_rxFrame.is())
        {
            if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
                _inout_rxFrame = pViewFrame->GetFrame().GetFrameInterface();
        }

        if ( !_inout_rxFrame.is() )
        {
            SAL_WARN( "cui.customize", "SvxConfigPage::GetFrameWithDefaultAndIdentify(): no frame found!" );
            return sModuleID;
        }

        sModuleID = vcl::CommandInfoProvider::GetModuleIdentifier(_inout_rxFrame);
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

    SfxGroupInfo_Impl *pData = weld::fromId<SfxGroupInfo_Impl*>(m_xFunctions->get_selected_id());
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

OUString SvxConfigPage::GetSelectedDisplayName() const
{
    return m_xFunctions->get_selected_text();
}

bool SvxConfigPage::FillItemSet( SfxItemSet* )
{
    bool result = false;

    for (int i = 0, nCount = m_xSaveInListBox->get_count(); i < nCount; ++i)
    {
        OUString sId = m_xSaveInListBox->get_id(i);
        if (sId != notebookbarTabScope)
        {
            SaveInData* pData = weld::fromId<SaveInData*>(sId);
            result = pData->Apply();
        }
    }
    return result;
}

IMPL_LINK_NOARG(SvxConfigPage, SelectSaveInLocation, weld::ComboBox&, void)
{
    OUString sId = m_xSaveInListBox->get_active_id();
    if (sId != notebookbarTabScope)
        pCurrentSaveInData = weld::fromId<SaveInData*>(sId);
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
            OUString sId(weld::toId(entryData));
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
    std::u16string_view rBaseTitle, SvxConfigEntry const * pParentData )
{
    for (auto const& entryData : *pParentData->GetEntries())
    {
        if (entryData->IsPopup())
        {
            OUString subMenuTitle = OUString::Concat(rBaseTitle) + aMenuSeparatorStr + SvxConfigPageHelper::stripHotKey(entryData->GetName());

            OUString sId(weld::toId(entryData));
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

SvxConfigEntry *SvxConfigPage::CreateCommandFromSelection(const OUString &aURL)
{
    OUString aDisplayName;

    if ( aURL.isEmpty() ) {
        return nullptr;
    }

    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(aURL, m_aModuleId);

    if ( typeid(*pCurrentSaveInData) == typeid(ContextMenuSaveInData) )
        aDisplayName = vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties);
    else if ( typeid(*pCurrentSaveInData) == typeid(MenuSaveInData) )
        aDisplayName = vcl::CommandInfoProvider::GetMenuLabelForCommand(aProperties);
    else
        aDisplayName = vcl::CommandInfoProvider::GetLabelForCommand(aProperties);

    SvxConfigEntry* toret =
        new SvxConfigEntry( aDisplayName, aURL, false, /*bParentData*/false );

    toret->SetUserDefined();

    if ( aDisplayName.isEmpty() )
        toret->SetName( GetSelectedDisplayName() );

    return toret;
}

bool SvxConfigPage::IsCommandInMenuList(const SvxConfigEntry *pEntryData,
                                        const SvxEntries *pEntries)
{
    bool toret = false;

    if ( pEntries != nullptr
      && pEntryData != nullptr )
    {
        for (auto const& entry : *pEntries)
        {
                if ( entry->GetCommand() == pEntryData->GetCommand() )
                {
                    toret = true;
                    break;
                }
        }
    }

    return toret;
}

int SvxConfigPage::AddFunction(int nTarget, bool bAllowDuplicates)
{
    int toret = -1;
    OUString aURL = GetScriptURL();
    SvxConfigEntry* pParent = GetTopLevelSelection();

    if ( aURL.isEmpty() || pParent == nullptr )
    {
        return -1;
    }


    SvxConfigEntry * pNewEntryData = CreateCommandFromSelection( aURL );

    // check that this function is not already in the menu
    if ( !bAllowDuplicates
      && IsCommandInMenuList( pNewEntryData, pParent->GetEntries() )
    )
    {
        delete pNewEntryData;
    } else {
        toret = AppendEntry( pNewEntryData, nTarget );
    }

    UpdateButtonStates();
    return toret;
}

int SvxConfigPage::AppendEntry(
    SvxConfigEntry* pNewEntryData,
    int nTarget)
{
    SvxConfigEntry* pTopLevelSelection = GetTopLevelSelection();

    if (pTopLevelSelection == nullptr)
        return -1;

    // Grab the entries list for the currently selected menu
    SvxEntries* pEntries = pTopLevelSelection->GetEntries();

    int nNewEntry = -1;
    int nCurEntry =
        nTarget != -1 ? nTarget : m_xContentsListBox->get_selected_index();

    OUString sId(weld::toId(pNewEntryData));

    if (nCurEntry == -1 || nCurEntry == m_xContentsListBox->n_children() - 1)
    {
        pEntries->push_back( pNewEntryData );
        m_xContentsListBox->insert(-1, sId);
        nNewEntry = m_xContentsListBox->n_children() - 1;
    }
    else
    {
        SvxConfigEntry* pEntryData =
            weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nCurEntry));

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

namespace
{
    template<typename itertype> void TmplInsertEntryIntoUI(SvxConfigEntry* pNewEntryData, weld::TreeView& rTreeView, itertype& rIter, SaveInData* pSaveInData,
                                                           VirtualDevice& rDropDown, bool bMenu)
    {
        OUString sId(weld::toId(pNewEntryData));

        rTreeView.set_id(rIter, sId);

        if (pNewEntryData->IsSeparator())
        {
            rTreeView.set_text(rIter, "----------------------------------", 0);
        }
        else
        {
            auto xImage = pSaveInData->GetImage(pNewEntryData->GetCommand());
            if (xImage.is())
                rTreeView.set_image(rIter, xImage, -1);
            OUString aName = SvxConfigPageHelper::stripHotKey( pNewEntryData->GetName() );
            rTreeView.set_text(rIter, aName, 0);
        }

        if (bMenu)  // menus
        {
            if (pNewEntryData->IsPopup() || pNewEntryData->GetStyle() & css::ui::ItemStyle::DROP_DOWN)
                rTreeView.set_image(rIter, rDropDown, 1);
            else
                rTreeView.set_image(rIter, css::uno::Reference<css::graphic::XGraphic>(), 1);
        }
    }
}

void SvxConfigPage::InsertEntryIntoUI(SvxConfigEntry* pNewEntryData, weld::TreeView& rTreeView, int nPos, bool bMenu)
{
    TmplInsertEntryIntoUI<int>(pNewEntryData, rTreeView, nPos, GetSaveInData(),
                               m_xContentsListBox->get_dropdown_image(), bMenu);
}

void SvxConfigPage::InsertEntryIntoUI(SvxConfigEntry* pNewEntryData, weld::TreeView& rTreeView, weld::TreeIter& rIter, bool bMenu)
{
    TmplInsertEntryIntoUI<weld::TreeIter>(pNewEntryData, rTreeView, rIter, GetSaveInData(),
                                          m_xContentsListBox->get_dropdown_image(), bMenu);
}

IMPL_LINK(SvxConfigPage, MoveHdl, weld::Button&, rButton, void)
{
    MoveEntry(&rButton == m_xMoveUpButton.get());
}

IMPL_LINK_NOARG(SvxConfigPage, FunctionDoubleClickHdl, weld::TreeView&, bool)
{
    if (m_xAddCommandButton->get_sensitive())
        m_xAddCommandButton->clicked();
    return true;
}

IMPL_LINK_NOARG(SvxConfigPage, SelectFunctionHdl, weld::TreeView&, void)
{
    // GetScriptURL() returns a non-empty string if a
    // valid command is selected on the left box
    OUString aSelectCommand = GetScriptURL();
    bool bIsValidCommand = !aSelectCommand.isEmpty();

    // Enable/disable Add and Remove buttons depending on current selection
    if (bIsValidCommand)
    {
        m_xAddCommandButton->set_sensitive(true);
        m_xRemoveCommandButton->set_sensitive(true);

        if (SfxHelp::IsHelpInstalled())
        {
            m_xDescriptionField->set_text(m_xFunctions->GetCommandHelpText());
        }
        else
        {
            SfxGroupInfo_Impl *pData = weld::fromId<SfxGroupInfo_Impl*>(m_xFunctions->get_selected_id());
            if (pData)
            {
                bool bIsExperimental
                    = vcl::CommandInfoProvider::IsExperimental(pData->sCommand, m_aModuleId);

                OUString aExperimental = "\n" + CuiResId(RID_CUISTR_COMMANDEXPERIMENTAL);
                OUString aLabel = CuiResId(RID_CUISTR_COMMANDLABEL) + ": " + pData->sLabel + "\n";
                OUString aName = CuiResId(RID_CUISTR_COMMANDNAME) + ": " + pData->sCommand + "\n";
                OUString aTip = CuiResId(RID_CUISTR_COMMANDTIP) + ": " + pData->sTooltip;
                if (bIsExperimental)
                    m_xDescriptionField->set_text(aLabel + aName + aTip + aExperimental);
                else
                    m_xDescriptionField->set_text(aLabel + aName + aTip);
            }
        }
    }
    else
    {

        m_xAddCommandButton->set_sensitive(false);
        m_xRemoveCommandButton->set_sensitive(false);

        m_xDescriptionField->set_text(u""_ustr);
    }

    UpdateButtonStates();
}

IMPL_LINK_NOARG(SvxConfigPage, ImplUpdateDataHdl, Timer*, void)
{
    OUString aSearchTerm(m_xSearchEdit->get_text());
    m_xCommandCategoryListBox->categorySelected(m_xFunctions.get(), aSearchTerm, GetSaveInData());
    SelectFunctionHdl(m_xFunctions->get_widget());
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
        weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nSourceEntry));

    SvxConfigEntry* pTargetData =
        weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nTargetEntry));

    if ( pSourceData == nullptr || pTargetData == nullptr )
        return false;

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

SvxMainMenuOrganizerDialog::SvxMainMenuOrganizerDialog(
    weld::Window* pParent, SvxEntries* entries,
    SvxConfigEntry const * selection, bool bCreateMenu )
    : GenericDialogController(pParent, u"cui/ui/movemenu.ui"_ustr, u"MoveMenuDialog"_ustr)
    , m_xMenuBox(m_xBuilder->weld_widget(u"namebox"_ustr))
    , m_xMenuNameEdit(m_xBuilder->weld_entry(u"menuname"_ustr))
    , m_xMenuListBox(m_xBuilder->weld_tree_view(u"menulist"_ustr))
    , m_xMoveUpButton(m_xBuilder->weld_button(u"up"_ustr))
    , m_xMoveDownButton(m_xBuilder->weld_button(u"down"_ustr))
{
    m_xMenuListBox->set_size_request(-1, m_xMenuListBox->get_height_rows(12));

    // Copy the entries list passed in
    if ( entries != nullptr )
    {
        mpEntries.reset( new SvxEntries );
        for (auto const& entry : *entries)
        {
            m_xMenuListBox->append(weld::toId(entry),
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
        OUString prefix = CuiResId( RID_CUISTR_NEW_MENU );

        OUString newname = SvxConfigPageHelper::generateCustomName( prefix, entries );
        OUString newurl = SvxConfigPageHelper::generateCustomMenuURL( mpEntries.get() );

        SvxConfigEntry* pNewEntryData =
            new SvxConfigEntry( newname, newurl, true, /*bParentData*/false );
        pNewEntryData->SetName( newname );
        pNewEntryData->SetUserDefined();
        pNewEntryData->SetMain();

        m_sNewMenuEntryId = weld::toId(pNewEntryData);
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
        m_xDialog->set_title(CuiResId(RID_CUISTR_MOVE_MENU));
    }

    m_xMenuListBox->connect_selection_changed(LINK(this, SvxMainMenuOrganizerDialog, SelectHdl));

    m_xMoveUpButton->connect_clicked(LINK( this, SvxMainMenuOrganizerDialog, MoveHdl));
    m_xMoveDownButton->connect_clicked(LINK( this, SvxMainMenuOrganizerDialog, MoveHdl));

    UpdateButtonStates();
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

    SvxConfigEntry* pNewEntryData = weld::fromId<SvxConfigEntry*>(m_sNewMenuEntryId);
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

    std::swap(mpEntries->at(nSourceEntry), mpEntries->at(nTargetEntry));

    UpdateButtonStates();
}

SvxConfigEntry* SvxMainMenuOrganizerDialog::GetSelectedEntry()
{
    const int nSelected(m_xMenuListBox->get_selected_index());
    if (nSelected == -1)
        return nullptr;
    return weld::fromId<SvxConfigEntry*>(m_xMenuListBox->get_id(nSelected));
}

SvxConfigEntry::SvxConfigEntry( OUString aDisplayName,
                                OUString aCommandURL, bool bPopup, bool bParentData )
    : nId( 1 )
    , aLabel(std::move(aDisplayName))
    , aCommand(std::move(aCommandURL))
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

bool SvxConfigEntry::IsMovable() const
{
    return !IsPopup() || IsMain();
}

bool SvxConfigEntry::IsDeletable() const
{
    return !IsMain() || IsUserDefined();
}

bool SvxConfigEntry::IsRenamable() const
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
    const uno::Reference<uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();
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
                for (beans::PropertyValue const& prop : aProps)
                {
                    if ( prop.Name == ITEM_DESCRIPTOR_STYLE )
                    {
                        prop.Value >>= result;
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
        uno::Any a = xPropSet->getPropertyValue( u"LayoutManager"_ustr );
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

        window = VCLUnoHelper::GetWindow( xWindow );
    }

    if ( window == nullptr || window->GetType() != WindowType::TOOLBOX )
        return;

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

void ToolbarSaveInData::SetSystemStyle(
    const OUString& rResourceURL,
    sal_Int32 nStyle )
{
    if ( !(rResourceURL.startsWith( "private" ) &&
         m_xPersistentWindowState.is() &&
         m_xPersistentWindowState->hasByName( rResourceURL )) )
        return;

    try
    {
        uno::Sequence< beans::PropertyValue > aProps;

        uno::Any a( m_xPersistentWindowState->getByName( rResourceURL ) );

        if ( a >>= aProps )
        {
            for ( beans::PropertyValue& prop : asNonConstRange(aProps) )
            {
                if ( prop.Name == ITEM_DESCRIPTOR_STYLE )
                {
                    prop.Value <<= nStyle;
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
        TOOLS_WARN_EXCEPTION("cui.customize", "Exception setting toolbar style");
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
                for (beans::PropertyValue const& prop : aProps)
                {
                    if ( prop.Name == ITEM_DESCRIPTOR_UINAME )
                    {
                        prop.Value >>= result;
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
                for (beans::PropertyValue const& prop : aPropSeq)
                {
                    if ( prop.Name == ITEM_DESCRIPTOR_LABEL )
                    {
                        prop.Value >>= result;
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

        pRootEntry.reset( new SvxConfigEntry( u"MainToolbars"_ustr, OUString(), true, /*bParentData*/false) );

        const uno::Sequence< uno::Sequence < beans::PropertyValue > > info =
            GetConfigManager()->getUIElementsInfo(
                css::ui::UIElementType::TOOLBAR );

        for ( uno::Sequence<beans::PropertyValue> const & props : info )
        {
            OUString url;
            OUString systemname;
            OUString uiname;

            for ( const beans::PropertyValue& prop : props )
            {
                if ( prop.Name == ITEM_DESCRIPTOR_RESOURCEURL )
                {
                    prop.Value >>= url;
                    systemname = url.copy( url.lastIndexOf( '/' ) + 1 );
                }
                else if ( prop.Name == ITEM_DESCRIPTOR_UINAME )
                {
                    prop.Value >>= uiname;
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

                if ( systemname.startsWith( CUSTOM_TOOLBAR_STR ) )
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
            const uno::Sequence< uno::Sequence < beans::PropertyValue > > info_ =
                xParentCfgMgr->getUIElementsInfo(
                    css::ui::UIElementType::TOOLBAR );

            for ( uno::Sequence<beans::PropertyValue> const & props : info_ )
            {
                OUString url;
                OUString systemname;
                OUString uiname;

                for ( const beans::PropertyValue& prop : props )
                {
                    if ( prop.Name == ITEM_DESCRIPTOR_RESOURCEURL )
                    {
                        prop.Value >>= url;
                        systemname = url.copy( url.lastIndexOf( '/' ) + 1 );
                    }
                    else if ( prop.Name == ITEM_DESCRIPTOR_UINAME )
                    {
                        prop.Value >>= uiname;
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
    const uno::Reference<uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();

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
            auto pPropValueSeq = aPropValueSeq.getArray();
            pPropValueSeq[nIndex].Name = m_aDescriptorContainer;
            pPropValueSeq[nIndex].Value <<= xSubMenuBar;
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
    uno::Reference< container::XIndexAccess > xSettings =
        GetConfigManager()->createSettings();

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
        TOOLS_WARN_EXCEPTION("cui.customize", "caught exception saving settings");
    }

    PersistChanges( GetConfigManager() );
}

void ToolbarSaveInData::CreateToolbar( SvxConfigEntry* pToolbar )
{
    // show the new toolbar in the UI also
    uno::Reference< container::XIndexAccess >
        xSettings = GetConfigManager()->createSettings();

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
        TOOLS_WARN_EXCEPTION("cui.customize", "caught exception saving settings");
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
        auto pURLSeq = aURLSeq.getArray();
        for (auto const& entry : *pToolbar->GetEntries())
        {
            pURLSeq[ 0 ] = entry->GetCommand();

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
                        for (beans::PropertyValue const& prop : aPropSeq)
                        {
                            if ( prop.Name == "Name" )
                            {
                                prop.Value >>= aLabel;
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
    : GenericDialogController(pWindow, u"cui/ui/newtoolbardialog.ui"_ustr, u"NewToolbarDialog"_ustr)
    , m_xEdtName(m_xBuilder->weld_entry(u"edit"_ustr))
    , m_xSaveInListBox(m_xBuilder->weld_combo_box(u"savein"_ustr))
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
    uno::Reference< css::ui::XImageManager > xImageManager,
    uno::Reference< css::ui::XImageManager > xParentImageManager)
    : GenericDialogController(pWindow, u"cui/ui/iconselectordialog.ui"_ustr, u"IconSelector"_ustr)
    , m_xImageManager(std::move(xImageManager))
    , m_xParentImageManager(std::move(xParentImageManager))
    , m_xTbSymbol(new ValueSet(m_xBuilder->weld_scrolled_window(u"symbolswin"_ustr, true)))
    , m_xTbSymbolWin(new weld::CustomWeld(*m_xBuilder, u"symbolsToolbar"_ustr, *m_xTbSymbol))
    , m_xFtNote(m_xBuilder->weld_label(u"noteLabel"_ustr))
    , m_xBtnImport(m_xBuilder->weld_button(u"importButton"_ustr))
    , m_xBtnDelete(m_xBuilder->weld_button(u"deleteButton"_ustr))
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

    m_xTbSymbol->SetStyle(m_xTbSymbol->GetStyle() | WB_ITEMBORDER | WB_VSCROLL);
    m_xTbSymbol->SetColCount(11);
    m_xTbSymbol->SetLineCount(5);
    m_xTbSymbol->SetItemWidth(m_nExpectedSize);
    m_xTbSymbol->SetItemHeight(m_nExpectedSize);
    m_xTbSymbol->SetExtraSpacing(6);
    m_xTbSymbol->SetMargin(4);
    Size aSize(m_xTbSymbol->CalcWindowSizePixel(Size(m_nExpectedSize, m_nExpectedSize), 11, 5));
    m_xTbSymbol->set_size_request(aSize.Width(), aSize.Height());

    const uno::Reference< uno::XComponentContext >& xComponentContext =
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

    uno::Sequence< uno::Any > aArgs{ uno::Any(aDirectory),
                                     uno::Any(css::embed::ElementModes::READWRITE) };

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
    if ( m_xImportedImageManager.is() )
    {
        const uno::Sequence< OUString > names = m_xImportedImageManager->getAllImageNames( SvxConfigPageHelper::GetImageType() );
        for (auto const & name : names )
            aImageInfo1.emplace( name, false );
    }

    uno::Sequence< OUString > name( 1 );
    auto pname = name.getArray();
    for (auto const& elem : aImageInfo1)
    {
        pname[ 0 ] = elem.first;
        uno::Sequence< uno::Reference< graphic::XGraphic> > graphics = m_xImportedImageManager->getImages( SvxConfigPageHelper::GetImageType(), name );
        if ( graphics.hasElements() )
        {
            m_aGraphics.push_back(graphics[0]);
            Image img(graphics[0]);
            m_xTbSymbol->InsertItem(m_aGraphics.size(), img, elem.first);
        }
    }

    ImageInfo                 aImageInfo;

    if ( m_xParentImageManager.is() )
    {
        const uno::Sequence< OUString > names = m_xParentImageManager->getAllImageNames( SvxConfigPageHelper::GetImageType() );
        for ( auto const & i : names )
            aImageInfo.emplace( i, false );
    }

    const uno::Sequence< OUString > names = m_xImageManager->getAllImageNames( SvxConfigPageHelper::GetImageType() );
    for ( auto const & i : names )
    {
        ImageInfo::iterator pIter = aImageInfo.find( i );
        if ( pIter != aImageInfo.end() )
            pIter->second = true;
        else
            aImageInfo.emplace( i, true );
    }

    // large growth factor, expecting many entries
    for (auto const& elem : aImageInfo)
    {
        pname[ 0 ] = elem.first;

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

        if ( graphics.hasElements() )
        {
            Image img(graphics[0]);
            if (!img.GetBitmap().IsEmpty())
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

IMPL_LINK_NOARG(SvxIconSelectorDialog, SelectHdl, ValueSet*, void)
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
    aImportDialog.SetContext(sfx2::FileDialogHelper::IconImport);

    // disable the link checkbox in the dialog
    uno::Reference< css::ui::dialogs::XFilePickerControlAccess >
        xController( aImportDialog.GetFilePicker(), uno::UNO_QUERY);
    if ( xController.is() )
    {
        xController->enableControl(
            css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK,
            false);
    }

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilter = rFilter.GetImportFormatNumberForShortName(u"png");
    aImportDialog.SetCurrentFilter(rFilter.GetImportFormatName(nFilter));

    if ( ERRCODE_NONE == aImportDialog.Execute() )
    {
        uno::Sequence< OUString > paths = aImportDialog.GetSelectedFiles();
        ImportGraphics ( paths );
    }
}

IMPL_LINK_NOARG(SvxIconSelectorDialog, DeleteHdl, weld::Button&, void)
{
    OUString message = CuiResId( RID_CUISTR_DELETE_ICON_CONFIRM );

    std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(m_xDialog.get(),
                                               VclMessageType::Warning, VclButtonsType::OkCancel,
                                               message));
    if (xWarn->run() != RET_OK)
        return;

    sal_uInt16 nId = m_xTbSymbol->GetSelectedItemId();

    uno::Sequence<OUString> URLs { m_xTbSymbol->GetItemText(nId) };
    m_xTbSymbol->RemoveItem(nId);
    m_xImportedImageManager->removeImages( SvxConfigPageHelper::GetImageType(), URLs );
    if ( m_xImportedImageManager->isModified() )
    {
        m_xImportedImageManager->store();
    }
}

bool SvxIconSelectorDialog::ReplaceGraphicItem(
    const OUString& aURL )
{
    uno::Reference< graphic::XGraphic > xGraphic;
    uno::Sequence< beans::PropertyValue > aMediaProps{ comphelper::makePropertyValue(u"URL"_ustr, aURL) };

    css::awt::Size aSize;
    bool bOK = false;
    try
    {
        xGraphic = m_xGraphProvider->queryGraphic( aMediaProps );

        uno::Reference< beans::XPropertySet > props =
            m_xGraphProvider->queryGraphicDescriptor( aMediaProps );
        uno::Any a = props->getPropertyValue( u"SizePixel"_ustr );
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
        assert(nId > 0 && "otherwise totally broken");

        if ( m_xTbSymbol->GetItemText( nId ) == aURL )
        {
            try
            {
                // replace/insert image with provided URL
                size_t nPos = nId - 1;
                assert(nPos == m_xTbSymbol->GetItemPos(nId));
                m_xTbSymbol->RemoveItem(nId);

                Image aImage( xGraphic );
                if ( bOK && ((aSize.Width != m_nExpectedSize) || (aSize.Height != m_nExpectedSize)) )
                {
                    BitmapEx aBitmap(aImage.GetBitmap());
                    BitmapEx aBitmapex = BitmapEx::AutoScaleBitmap(aBitmap, m_nExpectedSize);
                    aImage = Image( aBitmapex);
                }
                m_xTbSymbol->InsertItem(nId, aImage, aURL, nPos); //modify

                m_aGraphics[nPos] = Graphic(aImage.GetBitmap()).GetXGraphic();

                m_xImportedImageManager->replaceImages( SvxConfigPageHelper::GetImageType(), { aURL }, { xGraphic } );
                m_xImportedImageManager->store();

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
    OUString ReplaceIconName(std::u16string_view rMessage)
    {
        OUString name;
        OUString message = CuiResId( RID_CUISTR_REPLACE_ICON_WARNING );
        OUString placeholder(u"%ICONNAME"_ustr );
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
        SvxIconReplacementDialog(weld::Window *pParent, std::u16string_view rMessage, bool bYestoAll)
            : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Warning, VclButtonsType::NONE, ReplaceIconName(rMessage)))
        {
            m_xQueryBox->set_title(CuiResId(RID_CUISTR_REPLACE_ICON_CONFIRM));
            m_xQueryBox->add_button(GetStandardText(StandardButtonType::Yes), 2);
            if (bYestoAll)
                m_xQueryBox->add_button(CuiResId(RID_CUISTR_YESTOALL), 5);
            m_xQueryBox->add_button(GetStandardText(StandardButtonType::No), 4);
            m_xQueryBox->add_button(GetStandardText(StandardButtonType::Cancel), 6);
            m_xQueryBox->set_default_response(2);
        }
        short run() { return m_xQueryBox->run(); }
    };
}

void SvxIconSelectorDialog::ImportGraphics(
    const uno::Sequence< OUString >& rPaths )
{
    std::vector<OUString> rejected;

    bool replaceAll = false;
    for (const auto& aPath : rPaths)
    {
        bool result = true;
        if ( m_xImportedImageManager->hasImage( SvxConfigPageHelper::GetImageType(), aPath ) )
        {
            bool replace = replaceAll;
            if (!replace)
            {
                sal_Int32 aIndex = aPath.lastIndexOf('/');
                OUString aIconName = aPath.copy(aIndex + 1);
                SvxIconReplacementDialog aDlg(m_xDialog.get(), aIconName, rPaths.getLength() > 1);
                sal_uInt16 ret = aDlg.run();
                if (ret == 2)
                {
                    replace = true;
                }
                else if (ret == 5)
                {
                    replace = true;
                    replaceAll = true;
                }
            }
            if (replace)
            {
                ReplaceGraphicItem(aPath);
            }
        }
        else
        {
            result = ImportGraphic(aPath);
        }
        if (!result)
        {
            rejected.push_back(aPath);
        }
    }

    if (rejected.empty())
        return;

    OUStringBuffer message;
    for (const auto& rejected_item : rejected)
    {
        message.append(rejected_item + "\n");
    }

    SvxIconChangeDialog aDialog(m_xDialog.get(), message.makeStringAndClear());
    aDialog.run();
}

bool SvxIconSelectorDialog::ImportGraphic( const OUString& aURL )
{
    bool result = false;

    uno::Sequence< beans::PropertyValue > aMediaProps{ comphelper::makePropertyValue(u"URL"_ustr, aURL) };

    try
    {
        uno::Reference< beans::XPropertySet > props =
            m_xGraphProvider->queryGraphicDescriptor( aMediaProps );

        uno::Any a = props->getPropertyValue(u"SizePixel"_ustr);

        uno::Reference< graphic::XGraphic > xGraphic = m_xGraphProvider->queryGraphic( aMediaProps );
        if ( xGraphic.is() )
        {
            bool bOK = true;
            css::awt::Size aSize;

            a >>= aSize;
            if ( 0 == aSize.Width || 0 == aSize.Height )
                bOK = false;

            Image aImage( xGraphic );

            if ( bOK && ((aSize.Width != m_nExpectedSize) || (aSize.Height != m_nExpectedSize)) )
            {
                BitmapEx aBitmap(aImage.GetBitmap());
                BitmapEx aBitmapex = BitmapEx::AutoScaleBitmap(aBitmap, m_nExpectedSize);
                aImage = Image( aBitmapex);
            }
            if ( bOK && !!aImage )
            {
                m_aGraphics.push_back(Graphic(aImage.GetBitmap()).GetXGraphic());
                m_xTbSymbol->InsertItem(m_aGraphics.size(), aImage, aURL);

                uno::Sequence<OUString> aImportURL { aURL };
                uno::Sequence< uno::Reference<graphic::XGraphic > > aImportGraph{ xGraphic };
                m_xImportedImageManager->insertImages( SvxConfigPageHelper::GetImageType(), aImportURL, aImportGraph );
                if ( m_xImportedImageManager->isModified() )
                {
                    m_xImportedImageManager->store();
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
        TOOLS_WARN_EXCEPTION("cui.customize", "Caught exception importing XGraphic");
    }
    return result;
}

/*******************************************************************************
*
* The SvxIconChangeDialog class added for issue83555
*
*******************************************************************************/
SvxIconChangeDialog::SvxIconChangeDialog(weld::Window *pWindow, const OUString& rMessage)
    : MessageDialogController(pWindow, u"cui/ui/iconchangedialog.ui"_ustr, u"IconChange"_ustr, u"grid"_ustr)
    , m_xLineEditDescription(m_xBuilder->weld_text_view(u"addrTextview"_ustr))
{
    m_xLineEditDescription->set_size_request(m_xLineEditDescription->get_approximate_digit_width() * 48,
                                             m_xLineEditDescription->get_text_height() * 8);
    m_xLineEditDescription->set_text(rMessage);
}

SvxConfigPageFunctionDropTarget::SvxConfigPageFunctionDropTarget(SvxConfigPage&rPage, weld::TreeView& rTreeView)
    : weld::ReorderingDropTarget(rTreeView)
    , m_rPage(rPage)
{
}

sal_Int8 SvxConfigPageFunctionDropTarget::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    sal_Int8 nRet = weld::ReorderingDropTarget::ExecuteDrop(rEvt);
    m_rPage.ListModified();
    return nRet;;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
