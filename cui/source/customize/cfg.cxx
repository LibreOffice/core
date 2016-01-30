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

#include "sal/config.h"

#include <cassert>
#include <stdlib.h>
#include <time.h>

#include <vcl/help.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/decoview.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <sfx2/app.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/stritem.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/svlbitm.hxx>
#include "svtools/treelistentry.hxx"
#include "svtools/viewdataentry.hxx"
#include <tools/diagnose_ex.h>
#include <toolkit/helper/vclunohelper.hxx>

#include <algorithm>
#include <cuires.hrc>
#include "cfg.hrc"
#include "helpid.hrc"

#include "acccfg.hxx"
#include "cfg.hxx"
#include "eventdlg.hxx"
#include <dialmgr.hxx>

#include <comphelper/documentinfo.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/FileSystemStorageFactory.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
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
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/util/thePathSettings.hpp>

#include "dlgname.hxx"

#define PRTSTR(x) OUStringToOString(x, RTL_TEXTENCODING_ASCII_US).pData->buffer

#define ENTRY_HEIGHT 16

static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_CONTAINER[]   = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";
static const char ITEM_DESCRIPTOR_ISVISIBLE[]   = "IsVisible";
static const char ITEM_DESCRIPTOR_RESOURCEURL[] = "ResourceURL";
static const char ITEM_DESCRIPTOR_UINAME[]      = "UIName";

static const char ITEM_MENUBAR_URL[] = "private:resource/menubar/menubar";
static const char ITEM_TOOLBAR_URL[] = "private:resource/toolbar/";

static const char CUSTOM_TOOLBAR_STR[] = "custom_toolbar_";
static const char CUSTOM_MENU_STR[] = "vnd.openoffice.org:CustomMenu";

static const char aSeparatorStr[] = "----------------------------------";
static const char aMenuSeparatorStr[] = " | ";

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

    OSL_TRACE("printPropertySet: %d properties", aPropDetails.getLength());

    for ( sal_Int32 i = 0; i < aPropDetails.getLength(); ++i )
    {
        OUString tmp;
        sal_Int32 ival;

        uno::Any a = xPropSet->getPropertyValue( aPropDetails[i].Name );

        if ( a >>= tmp )
        {
            OSL_TRACE("%s: Got property: %s = %s",
                PRTSTR(prefix), PRTSTR(aPropDetails[i].Name), PRTSTR(tmp));
        }
        else if ( ( a >>= ival ) )
        {
            OSL_TRACE("%s: Got property: %s = %d",
                PRTSTR(prefix), PRTSTR(aPropDetails[i].Name), ival);
        }
        else
        {
            OSL_TRACE("%s: Got property: %s of type %s",
                PRTSTR(prefix), PRTSTR(aPropDetails[i].Name), PRTSTR(a.getValueTypeName()));
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

        OSL_TRACE("%s: Got property: %s = %s",
            PRTSTR(prefix), PRTSTR(aProp[i].Name), PRTSTR(tmp));
    }
}

void printEntries(SvxEntries* entries)
{
    SvxEntries::const_iterator iter = entries->begin();

    for ( ; iter != entries->end(); ++iter )
    {
        SvxConfigEntry* entry = *iter;

        OSL_TRACE("printEntries: %s", PRTSTR(entry->GetName()));
    }
}

#endif

OUString
stripHotKey( const OUString& str )
{
    sal_Int32 index = str.indexOf( '~' );
    if ( index == -1 )
    {
        return str;
    }
    else
    {
        return str.replaceAt( index, 1, OUString() );
    }
}

OUString replaceSaveInName(
    const OUString& rMessage,
    const OUString& rSaveInName )
{
    OUString name;
    OUString placeholder("%SAVE IN SELECTION%" );

    sal_Int32 pos = rMessage.indexOf( placeholder );

    if ( pos != -1 )
    {
        name = rMessage.replaceAt(
            pos, placeholder.getLength(), rSaveInName );
    }

    return name;
}

OUString
replaceSixteen( const OUString& str, sal_Int32 nReplacement )
{
    OUString result( str );
    OUString sixteen = OUString::number( 16 );
    OUString expected = OUString::number( nReplacement );

    sal_Int32 len = sixteen.getLength();
    sal_Int32 index = result.indexOf( sixteen );

    while ( index != -1 )
    {
        result = result.replaceAt( index, len, expected );
        index = result.indexOf( sixteen, index );
    }

    return result;
}

OUString
generateCustomName(
    const OUString& prefix,
    SvxEntries* entries,
    sal_Int32 suffix = 1 )
{
    // find and replace the %n placeholder in the prefix string
    OUString name;
    OUString placeholder("%n" );

    sal_Int32 pos = prefix.indexOf( placeholder );

    if ( pos != -1 )
    {
        name = prefix.replaceAt(
            pos, placeholder.getLength(), OUString::number( suffix ) );
    }
    else
    {
        // no placeholder found so just append the suffix
        name = prefix + OUString::number( suffix );
    }

    if (!entries)
        return name;

    // now check is there is an already existing entry with this name
    SvxEntries::const_iterator iter = entries->begin();

    while ( iter != entries->end() )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( name.equals( pEntry->GetName() ) )
        {
            break;
        }
        ++iter;
    }

    if ( iter != entries->end() )
    {
        // name already exists so try the next number up
        return generateCustomName( prefix, entries, ++suffix );
    }

    return name;
}

sal_uInt32 generateRandomValue()
{
    return comphelper::rng::uniform_uint_distribution(0, std::numeric_limits<unsigned int>::max());
}

OUString
generateCustomURL(
    SvxEntries* entries )
{
    OUString url = ITEM_TOOLBAR_URL;
    url += CUSTOM_TOOLBAR_STR;

    // use a random number to minimize possible clash with existing custom toolbars
    url += OUString::number( generateRandomValue(), 16 );

    // now check is there is an already existing entry with this url
    SvxEntries::const_iterator iter = entries->begin();

    while ( iter != entries->end() )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( url.equals( pEntry->GetCommand() ) )
        {
            break;
        }
        ++iter;
    }

    if ( iter != entries->end() )
    {
        // url already exists so try the next number up
        return generateCustomURL( entries );
    }

    return url;
}

OUString
generateCustomMenuURL(
    SvxEntries* entries,
    sal_Int32 suffix = 1 )
{
    OUString url(CUSTOM_MENU_STR );
    url += OUString::number( suffix );

    if (!entries)
        return url;

    // now check is there is an already existing entry with this url
    SvxEntries::const_iterator iter = entries->begin();

    while ( iter != entries->end() )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( url.equals( pEntry->GetCommand() ) )
        {
            break;
        }
        ++iter;
    }

    if ( iter != entries->end() )
    {
        // url already exists so try the next number up
        return generateCustomMenuURL( entries, ++suffix );
    }

    return url;
}

static sal_Int16 theImageType =
    css::ui::ImageType::COLOR_NORMAL |
    css::ui::ImageType::SIZE_DEFAULT;

void InitImageType()
{
    theImageType =
        css::ui::ImageType::COLOR_NORMAL |
        css::ui::ImageType::SIZE_DEFAULT;

    if ( SvtMiscOptions().AreCurrentSymbolsLarge() )
    {
        theImageType |= css::ui::ImageType::SIZE_LARGE;
    }
}

sal_Int16 GetImageType()
{
    return theImageType;
}

void RemoveEntry( SvxEntries* pEntries, SvxConfigEntry* pChildEntry )
{
    SvxEntries::iterator iter = pEntries->begin();

    while ( iter != pEntries->end() )
    {
        if ( pChildEntry == *iter )
        {
            pEntries->erase( iter );
            break;
        }
        ++iter;
    }
}

bool
SvxConfigPage::CanConfig( const OUString& aModuleId )
{
    OSL_TRACE("SupportsDocumentConfig: %s", PRTSTR(aModuleId));

    if  ( aModuleId == "com.sun.star.script.BasicIDE" || aModuleId == "com.sun.star.frame.Bibliography" )
    {
        return false;
    }
    return true;
}

OUString GetModuleName( const OUString& aModuleId )
{
    if ( aModuleId == "com.sun.star.text.TextDocument" ||
         aModuleId == "com.sun.star.text.GlobalDocument" )
        return OUString("Writer");
    else if ( aModuleId == "com.sun.star.text.WebDocument" )
        return OUString("Writer/Web");
    else if ( aModuleId == "com.sun.star.drawing.DrawingDocument" )
        return OUString("Draw");
    else if ( aModuleId == "com.sun.star.presentation.PresentationDocument" )
        return OUString("Impress");
    else if ( aModuleId == "com.sun.star.sheet.SpreadsheetDocument" )
        return OUString("Calc");
    else if ( aModuleId == "com.sun.star.script.BasicIDE" )
        return OUString("Basic");
    else if ( aModuleId == "com.sun.star.formula.FormulaProperties" )
        return OUString("Math");
    else if ( aModuleId == "com.sun.star.sdb.RelationDesign" )
        return OUString("Relation Design");
    else if ( aModuleId == "com.sun.star.sdb.QueryDesign" )
        return OUString("Query Design");
    else if ( aModuleId == "com.sun.star.sdb.TableDesign" )
        return OUString("Table Design");
    else if ( aModuleId == "com.sun.star.sdb.DataSourceBrowser" )
        return OUString("Data Source Browser" );
    else if ( aModuleId == "com.sun.star.sdb.DatabaseDocument" )
        return OUString("Database" );

    return OUString();
}

OUString GetUIModuleName( const OUString& aModuleId, const uno::Reference< css::frame::XModuleManager2 >& rModuleManager )
{
    assert(rModuleManager.is());

    OUString aModuleUIName;

    try
    {
        uno::Any a = rModuleManager->getByName( aModuleId );
        uno::Sequence< beans::PropertyValue > aSeq;

        if ( a >>= aSeq )
        {
            for ( sal_Int32 i = 0; i < aSeq.getLength(); ++i )
            {
                if ( aSeq[i].Name == "ooSetupFactoryUIName" )
                {
                    aSeq[i].Value >>= aModuleUIName;
                    break;
                }
            }
        }
    }
    catch ( uno::RuntimeException& )
    {
        throw;
    }
    catch ( uno::Exception& )
    {
    }

    if ( aModuleUIName.isEmpty() )
        aModuleUIName = GetModuleName( aModuleId );

    return aModuleUIName;
}

bool GetMenuItemData(
    const uno::Reference< container::XIndexAccess >& rItemContainer,
    sal_Int32 nIndex,
    OUString& rCommandURL,
    OUString& rLabel,
    sal_uInt16& rType,
    uno::Reference< container::XIndexAccess >& rSubMenu )
{
    try
    {
        uno::Sequence< beans::PropertyValue > aProp;
        if ( rItemContainer->getByIndex( nIndex ) >>= aProp )
        {
            for ( sal_Int32 i = 0; i < aProp.getLength(); ++i )
            {
                if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                {
                    aProp[i].Value >>= rCommandURL;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_CONTAINER )
                {
                    aProp[i].Value >>= rSubMenu;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_LABEL )
                {
                    aProp[i].Value >>= rLabel;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_TYPE )
                {
                    aProp[i].Value >>= rType;
                }
            }

            return true;
        }
    }
    catch ( css::lang::IndexOutOfBoundsException& )
    {
    }

    return false;
}

bool GetToolbarItemData(
    const uno::Reference< container::XIndexAccess >& rItemContainer,
    sal_Int32 nIndex,
    OUString& rCommandURL,
    OUString& rLabel,
    sal_uInt16& rType,
    bool& rIsVisible,
    sal_Int32& rStyle,
    uno::Reference< container::XIndexAccess >& rSubMenu )
{
    try
    {
        uno::Sequence< beans::PropertyValue > aProp;
        if ( rItemContainer->getByIndex( nIndex ) >>= aProp )
        {
            for ( sal_Int32 i = 0; i < aProp.getLength(); ++i )
            {
                if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                {
                    aProp[i].Value >>= rCommandURL;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_STYLE )
                {
                    aProp[i].Value >>= rStyle;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_CONTAINER )
                {
                    aProp[i].Value >>= rSubMenu;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_LABEL )
                {
                    aProp[i].Value >>= rLabel;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_TYPE )
                {
                    aProp[i].Value >>= rType;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_ISVISIBLE )
                {
                    aProp[i].Value >>= rIsVisible;
                }
            }

            return true;
        }
    }
    catch ( css::lang::IndexOutOfBoundsException& )
    {
    }

    return false;
}

uno::Sequence< beans::PropertyValue >
ConvertSvxConfigEntry(
    const uno::Reference< container::XNameAccess >& xCommandToLabelMap,
    const SvxConfigEntry* pEntry )
{
    static const OUString aDescriptorCommandURL (
        ITEM_DESCRIPTOR_COMMANDURL  );

    static const OUString aDescriptorType(
            ITEM_DESCRIPTOR_TYPE  );

    static const OUString aDescriptorLabel(
            ITEM_DESCRIPTOR_LABEL  );

    uno::Sequence< beans::PropertyValue > aPropSeq( 3 );

    aPropSeq[0].Name = aDescriptorCommandURL;
    aPropSeq[0].Value <<= OUString( pEntry->GetCommand() );

    aPropSeq[1].Name = aDescriptorType;
    aPropSeq[1].Value <<= css::ui::ItemType::DEFAULT;

    // If the name has not been changed and the name is the same as
    // in the default command to label map then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    aPropSeq[2].Name = aDescriptorLabel;
    if ( !pEntry->HasChangedName() && !pEntry->GetCommand().isEmpty() )
    {
        bool isDefaultName = false;
        try
        {
            uno::Any a( xCommandToLabelMap->getByName( pEntry->GetCommand() ) );
            uno::Sequence< beans::PropertyValue > tmpPropSeq;
            if ( a >>= tmpPropSeq )
            {
                for ( sal_Int32 i = 0; i < tmpPropSeq.getLength(); ++i )
                {
                    if ( tmpPropSeq[i].Name.equals( aDescriptorLabel ) )
                    {
                        OUString tmpLabel;
                        tmpPropSeq[i].Value >>= tmpLabel;

                        if ( tmpLabel.equals( pEntry->GetName() ) )
                        {
                            isDefaultName = true;
                        }

                        break;
                    }
                }
            }
        }
        catch ( container::NoSuchElementException& )
        {
            // isDefaultName is left as FALSE
        }

        if ( isDefaultName )
        {
            aPropSeq[2].Value <<= OUString();
        }
        else
        {
            aPropSeq[2].Value <<= OUString( pEntry->GetName() );
        }
    }
    else
    {
        aPropSeq[2].Value <<= OUString( pEntry->GetName() );
    }

    return aPropSeq;
}

uno::Sequence< beans::PropertyValue >
ConvertToolbarEntry(
    const uno::Reference< container::XNameAccess >& xCommandToLabelMap,
    const SvxConfigEntry* pEntry )
{
    static const OUString aDescriptorCommandURL (
        ITEM_DESCRIPTOR_COMMANDURL  );

    static const OUString aDescriptorType(
            ITEM_DESCRIPTOR_TYPE  );

    static const OUString aDescriptorLabel(
            ITEM_DESCRIPTOR_LABEL  );

    static const OUString aIsVisible(
            ITEM_DESCRIPTOR_ISVISIBLE  );

    uno::Sequence< beans::PropertyValue > aPropSeq( 4 );

    aPropSeq[0].Name = aDescriptorCommandURL;
    aPropSeq[0].Value <<= OUString( pEntry->GetCommand() );

    aPropSeq[1].Name = aDescriptorType;
    aPropSeq[1].Value <<= css::ui::ItemType::DEFAULT;

    // If the name has not been changed and the name is the same as
    // in the default command to label map then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    aPropSeq[2].Name = aDescriptorLabel;
    if ( !pEntry->HasChangedName() && !pEntry->GetCommand().isEmpty() )
    {
        bool isDefaultName = false;
        try
        {
            uno::Any a( xCommandToLabelMap->getByName( pEntry->GetCommand() ) );
            uno::Sequence< beans::PropertyValue > tmpPropSeq;
            if ( a >>= tmpPropSeq )
            {
                for ( sal_Int32 i = 0; i < tmpPropSeq.getLength(); ++i )
                {
                    if ( tmpPropSeq[i].Name.equals( aDescriptorLabel ) )
                    {
                        OUString tmpLabel;
                        tmpPropSeq[i].Value >>= tmpLabel;

                        if ( tmpLabel.equals( pEntry->GetName() ) )
                        {
                            isDefaultName = true;
                        }

                        break;
                    }
                }
            }
        }
        catch ( container::NoSuchElementException& )
        {
            // isDefaultName is left as FALSE
        }

        if ( isDefaultName )
        {
            aPropSeq[2].Value <<= OUString();
        }
        else
        {
            aPropSeq[2].Value <<= OUString( pEntry->GetName() );
        }
    }
    else
    {
        aPropSeq[2].Value <<= OUString( pEntry->GetName() );
    }

    aPropSeq[3].Name = aIsVisible;
    aPropSeq[3].Value <<= pEntry->IsVisible();

    return aPropSeq;
}

VclPtr<SfxTabPage> CreateSvxMenuConfigPage( vcl::Window *pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxMenuConfigPage>::Create( pParent, *rSet );
}

VclPtr<SfxTabPage> CreateSvxContextMenuConfigPage( vcl::Window *pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxMenuConfigPage>::Create( pParent, *rSet, false );
}

VclPtr<SfxTabPage> CreateKeyboardConfigPage( vcl::Window *pParent, const SfxItemSet* rSet )
{
       return VclPtr<SfxAcceleratorConfigPage>::Create( pParent, *rSet );
}

VclPtr<SfxTabPage> CreateSvxToolbarConfigPage( vcl::Window *pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxToolbarConfigPage>::Create( pParent, *rSet );
}

VclPtr<SfxTabPage> CreateSvxEventConfigPage( vcl::Window *pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxEventConfigPage>::Create( pParent, *rSet, SvxEventConfigPage::EarlyInit() );
}

namespace {

bool showKeyConfigTabPage( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    if (!xFrame.is())
    {
        return false;
    }
    OUString sModuleId(
        css::frame::ModuleManager::create(
            comphelper::getProcessComponentContext())
        ->identify(xFrame));
    return !sModuleId.isEmpty()
        && sModuleId != "com.sun.star.frame.StartModule";
}

bool EntrySort( SvxConfigEntry* a, SvxConfigEntry* b )
{
    return a->GetName().compareTo( b->GetName() ) < 0;
}

bool SvxConfigEntryModified( SvxConfigEntry* pEntry )
{
    SvxEntries* pEntries = pEntry->GetEntries();
    if ( !pEntries )
        return false;

    for ( const auto& entry : *pEntries )
    {
        if ( entry->IsModified() || SvxConfigEntryModified( entry ) )
            return true;
    }
    return false;
}

}

/******************************************************************************
 *
 * SvxConfigDialog is the configuration dialog which is brought up from the
 * Tools menu. It includes tabs for customizing menus, toolbars, events and
 * key bindings.
 *
 *****************************************************************************/
SvxConfigDialog::SvxConfigDialog(vcl::Window * pParent, const SfxItemSet* pInSet)
    : SfxTabDialog(pParent, "CustomizeDialog",
        "cui/ui/customizedialog.ui", pInSet)
    , m_nMenusPageId(0)
    , m_nContextMenusPageId(0)
    , m_nKeyboardPageId(0)
    , m_nToolbarsPageId(0)
    , m_nEventsPageId(0)
{
    InitImageType();

    m_nMenusPageId = AddTabPage("menus", CreateSvxMenuConfigPage, nullptr);
    if ( getenv("LO_USE_NEWCONTEXTMENU") )
        m_nContextMenusPageId = AddTabPage("contextmenus", CreateSvxContextMenuConfigPage, nullptr);
    else
        RemoveTabPage("contextmenus");
    m_nKeyboardPageId = AddTabPage("keyboard", CreateKeyboardConfigPage, nullptr);
    m_nToolbarsPageId = AddTabPage("toolbars", CreateSvxToolbarConfigPage, nullptr);
    m_nEventsPageId = AddTabPage("events", CreateSvxEventConfigPage, nullptr);

    const SfxPoolItem* pItem =
        pInSet->GetItem( pInSet->GetPool()->GetWhich( SID_CONFIG ) );

    if ( pItem )
    {
        OUString text = static_cast<const SfxStringItem*>(pItem)->GetValue();

        if (text.startsWith( ITEM_TOOLBAR_URL ) )
        {
            SetCurPageId(m_nToolbarsPageId);
        }
    }
}

void SvxConfigDialog::SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    m_xFrame = xFrame;

    if (!showKeyConfigTabPage( xFrame ))
        RemoveTabPage(m_nKeyboardPageId);
}

void SvxConfigDialog::PageCreated( sal_uInt16 nId, SfxTabPage& rPage )
{
    if (nId == m_nMenusPageId || nId == m_nKeyboardPageId ||
        nId == m_nToolbarsPageId || nId == m_nContextMenusPageId)
    {
        rPage.SetFrame(m_xFrame);
    }
    else if (nId == m_nEventsPageId)
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

uno::Reference< graphic::XGraphic > GetGraphic(
    const uno::Reference< css::ui::XImageManager >& xImageManager,
    const OUString& rCommandURL )
{
    uno::Reference< graphic::XGraphic > result;

    if ( xImageManager.is() )
    {
        // TODO handle large graphics
        uno::Sequence< uno::Reference< graphic::XGraphic > > aGraphicSeq;

        uno::Sequence<OUString> aImageCmdSeq { rCommandURL };

        try
        {
            aGraphicSeq =
                xImageManager->getImages( GetImageType(), aImageCmdSeq );

            if ( aGraphicSeq.getLength() > 0 )
            {
                result =  aGraphicSeq[0];
            }
        }
        catch ( uno::Exception& )
        {
            // will return empty XGraphic
        }
    }

    return result;
}

Image SaveInData::GetImage( const OUString& rCommandURL )
{
    Image aImage;

    uno::Reference< graphic::XGraphic > xGraphic =
        GetGraphic( m_xImgMgr, rCommandURL );

    if ( xGraphic.is() )
    {
        aImage = Image( xGraphic );
    }
    else if ( xDefaultImgMgr != nullptr && (*xDefaultImgMgr).is() )
    {
        xGraphic = GetGraphic( (*xDefaultImgMgr), rCommandURL );

        if ( xGraphic.is() )
        {
            aImage = Image( xGraphic );
        }
    }

    return aImage;
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
            ITEM_DESCRIPTOR_CONTAINER  ),
        pRootEntry( nullptr )
{
    try
    {
        OUString url( ITEM_MENUBAR_URL  );
        m_xMenuSettings = GetConfigManager()->getSettings( url, sal_False );
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
    if ( pRootEntry != nullptr )
    {
        delete pRootEntry;
    }
}

SvxEntries*
MenuSaveInData::GetEntries()
{
    if ( pRootEntry == nullptr )
    {
        pRootEntry = new SvxConfigEntry(
            OUString("MainMenus"),
            OUString(), true);

        if ( m_xMenuSettings.is() )
        {
            LoadSubMenus( m_xMenuSettings, OUString(), pRootEntry );
        }
        else if ( GetDefaultData() != nullptr )
        {
            // If the doc has no config settings use module config settings
            LoadSubMenus( GetDefaultData()->m_xMenuSettings, OUString(), pRootEntry );
        }
    }

    return pRootEntry->GetEntries();
}

void
MenuSaveInData::SetEntries( SvxEntries* pNewEntries )
{
    // delete old menu hierarchy first
    delete pRootEntry->GetEntries();

    // now set new menu hierarchy
    pRootEntry->SetEntries( pNewEntries );
}

bool SaveInData::LoadSubMenus(
    const uno::Reference< container::XIndexAccess >& xMenuSettings,
    const OUString& rBaseTitle,
    SvxConfigEntry* pParentData )
{
    SvxEntries* pEntries = pParentData->GetEntries();

    // Don't access non existing menu configuration!
    if ( !xMenuSettings.is() )
        return true;

    for ( sal_Int32 nIndex = 0; nIndex < xMenuSettings->getCount(); ++nIndex )
    {
        uno::Reference< container::XIndexAccess >   xSubMenu;
        OUString                aCommandURL;
        OUString                aLabel;

        sal_uInt16 nType( css::ui::ItemType::DEFAULT );

        bool bItem = GetMenuItemData( xMenuSettings, nIndex,
            aCommandURL, aLabel, nType, xSubMenu );

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

                // If custom label not set retrieve it from the command
                // to info service
                if ( aLabel.isEmpty() )
                {
                    uno::Sequence< beans::PropertyValue > aPropSeq;
                    if ( a >>= aPropSeq )
                    {
                        for ( sal_Int32 i = 0; i < aPropSeq.getLength(); ++i )
                        {
                            if ( aPropSeq[i].Name == ITEM_DESCRIPTOR_LABEL )
                            {
                                aPropSeq[i].Value >>= aLabel;
                                break;
                            }
                        }
                    }
                }

                if ( xSubMenu.is() )
                {
                    // popup menu
                    SvxConfigEntry* pEntry = new SvxConfigEntry(
                        aLabel, aCommandURL, true );

                    pEntry->SetUserDefined( bIsUserDefined );

                    pEntries->push_back( pEntry );

                    OUString subMenuTitle( rBaseTitle );

                    if ( !subMenuTitle.isEmpty() )
                    {
                        subMenuTitle += aMenuSeparatorStr;
                    }
                    else
                    {
                        pEntry->SetMain();
                    }

                    subMenuTitle += stripHotKey( aLabel );

                    LoadSubMenus( xSubMenu, subMenuTitle, pEntry );
                }
                else
                {
                    SvxConfigEntry* pEntry = new SvxConfigEntry(
                        aLabel, aCommandURL, false );
                    pEntry->SetUserDefined( bIsUserDefined );
                    pEntries->push_back( pEntry );
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
    return true;
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

        Apply( pRootEntry, xIndexContainer, xFactory );

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
        catch ( container::NoSuchElementException& )
        {
            OSL_TRACE("caught container::NoSuchElementException saving settings");
        }
        catch ( css::io::IOException& )
        {
            OSL_TRACE("caught IOException saving settings");
        }
        catch ( css::uno::Exception& )
        {
            OSL_TRACE("caught some other exception saving settings");
        }

        SetModified( false );

        result = PersistChanges( GetConfigManager() );
    }

    return result;
}

void MenuSaveInData::Apply(
    SvxConfigEntry* pRootEntry_,
    uno::Reference< container::XIndexContainer >& rMenuBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory,
    SvTreeListEntry *pParentEntry )
{
    (void)pRootEntry_;
    (void)pParentEntry;

    SvxEntries::const_iterator iter = GetEntries()->begin();
    SvxEntries::const_iterator end = GetEntries()->end();

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();

    for ( ; iter != end; ++iter )
    {
        SvxConfigEntry* pEntryData = *iter;

        uno::Sequence< beans::PropertyValue > aPropValueSeq =
            ConvertSvxConfigEntry( m_xCommandToLabelMap, pEntryData );

        uno::Reference< container::XIndexContainer > xSubMenuBar(
            rFactory->createInstanceWithContext( xContext ),
            uno::UNO_QUERY );

        sal_Int32 nIndex = aPropValueSeq.getLength();
        aPropValueSeq.realloc( nIndex + 1 );
        aPropValueSeq[nIndex].Name = m_aDescriptorContainer;
        aPropValueSeq[nIndex].Value <<= xSubMenuBar;
        rMenuBar->insertByIndex(
            rMenuBar->getCount(), uno::makeAny( aPropValueSeq ));
        ApplyMenu( xSubMenuBar, rFactory, pEntryData );
    }
}

void SaveInData::ApplyMenu(
    uno::Reference< container::XIndexContainer >& rMenuBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory,
    SvxConfigEntry* pMenuData )
{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();

    SvxEntries::const_iterator iter = pMenuData->GetEntries()->begin();
    SvxEntries::const_iterator end = pMenuData->GetEntries()->end();

    for ( ; iter != end; ++iter )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( pEntry->IsPopup() )
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                ConvertSvxConfigEntry( m_xCommandToLabelMap, pEntry );

            uno::Reference< container::XIndexContainer > xSubMenuBar(
                rFactory->createInstanceWithContext( xContext ),
                    uno::UNO_QUERY );

            sal_Int32 nIndex = aPropValueSeq.getLength();
            aPropValueSeq.realloc( nIndex + 1 );
            aPropValueSeq[nIndex].Name = ITEM_DESCRIPTOR_CONTAINER;
            aPropValueSeq[nIndex].Value <<= xSubMenuBar;

            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::makeAny( aPropValueSeq ));

            ApplyMenu( xSubMenuBar, rFactory, pEntry );
            pEntry->SetModified( false );
        }
        else if ( pEntry->IsSeparator() )
        {
            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::makeAny( m_aSeparatorSeq ));
        }
        else
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                ConvertSvxConfigEntry( m_xCommandToLabelMap, pEntry );
            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::makeAny( aPropValueSeq ));
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

    delete pRootEntry;
    pRootEntry = nullptr;

    try
    {
        m_xMenuSettings = GetConfigManager()->getSettings(
            m_aMenuResourceURL, sal_False );
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
        typedef std::unordered_map< OUString, bool, OUStringHash, std::equal_to< OUString > > MenuInfo;
        MenuInfo aMenuInfo;

        m_pRootEntry.reset( new SvxConfigEntry( "ContextMenus", OUString(), true ) );
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
                    aElementProp.Value >>= aUrl;
            }

            css::uno::Reference< css::container::XIndexAccess > xPopupMenu;
            try
            {
                xPopupMenu = GetConfigManager()->getSettings( aUrl, sal_False );
            }
            catch ( const css::uno::Exception& )
            {}

            if ( xPopupMenu.is() )
            {
                OUString aMenuName = aUrl.copy( aUrl.lastIndexOf( '/' ) + 1 );
                OUString aUIMenuName = GetUIName( aUrl );
                if ( aUIMenuName.isEmpty() )
                    aUIMenuName = aMenuName;

                // insert into std::unordered_map to filter duplicates from the parent
                aMenuInfo.insert( MenuInfo::value_type( aMenuName, true ) );

                SvxConfigEntry* pEntry = new SvxConfigEntry( aUIMenuName, aUrl, true );
                pEntry->SetMain();
                m_pRootEntry->GetEntries()->push_back( pEntry );
                LoadSubMenus( xPopupMenu, aUIMenuName, pEntry );
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
            OUString aUrl, aMenuName;
            for ( const auto& aElementProp : aElement )
            {
                if ( aElementProp.Name == ITEM_DESCRIPTOR_RESOURCEURL )
                {
                    aElementProp.Value >>= aUrl;
                    aMenuName = aUrl.copy( aUrl.lastIndexOf( '/' ) + 1 );
                }
            }

            css::uno::Reference< css::container::XIndexAccess > xPopupMenu;
            try
            {
                if ( aMenuInfo.find( aMenuName ) == aMenuInfo.end() )
                    xPopupMenu = xParentCfgMgr->getSettings( aUrl, sal_False );
            }
            catch ( const css::uno::Exception& )
            {}

            if ( xPopupMenu.is() )
            {
                OUString aUIMenuName = GetUIName( aUrl );
                if ( aUIMenuName.isEmpty() )
                    aUIMenuName = aMenuName;

                SvxConfigEntry* pEntry = new SvxConfigEntry( aUIMenuName, aUrl, true, true );
                pEntry->SetMain();
                m_pRootEntry->GetEntries()->push_back( pEntry );
                LoadSubMenus( xPopupMenu, aUIMenuName, pEntry );
            }
        }
        std::sort( m_pRootEntry->GetEntries()->begin(), m_pRootEntry->GetEntries()->end(), EntrySort );
    }
    return m_pRootEntry->GetEntries();
}

void ContextMenuSaveInData::SetEntries( SvxEntries* pNewEntries )
{
    delete m_pRootEntry->GetEntries();
    m_pRootEntry->SetEntries( pNewEntries );
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
    return m_pRootEntry && m_pRootEntry->GetEntries()->size();
}

bool ContextMenuSaveInData::Apply()
{
    if ( !IsModified() )
        return false;

    SvxEntries* pEntries = GetEntries();
    for ( const auto& pEntry : *pEntries )
    {
        if ( pEntry->IsModified() || SvxConfigEntryModified( pEntry ) )
        {
            css::uno::Reference< css::container::XIndexContainer > xIndexContainer( GetConfigManager()->createSettings(), css::uno::UNO_QUERY );
            css::uno::Reference< css::lang::XSingleComponentFactory > xFactory( xIndexContainer, css::uno::UNO_QUERY );
            ApplyMenu( xIndexContainer, xFactory, pEntry );

            OUString aUrl = pEntry->GetCommand();
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
        {}
    }
    PersistChanges( GetConfigManager() );
    m_pRootEntry.reset();
}

class PopupPainter : public SvLBoxString
{
public:
    PopupPainter( SvTreeListEntry* pEntry, const OUString& rStr )
        : SvLBoxString( pEntry, 0, rStr )
    { }

    virtual ~PopupPainter() { }

    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override
    {
        SvLBoxString::Paint(rPos, rOutDev, rRenderContext, pView, rEntry);

        rRenderContext.Push(PushFlags::FILLCOLOR);

        SvTreeListBox* pTreeBox = static_cast< SvTreeListBox* >(&rOutDev);
        long nX = pTreeBox->GetSizePixel().Width();

        ScrollBar* pVScroll = pTreeBox->GetVScroll();
        if (pVScroll->IsVisible())
        {
            nX -= pVScroll->GetSizePixel().Width();
        }

        const SvViewDataItem* pItem = rOutDev.GetViewDataItem( &rEntry, this );
        nX -= pItem->maSize.Height();

        long nSize = pItem->maSize.Height() / 2;
        long nHalfSize = nSize / 2;
        long nY = rPos.Y() + nHalfSize;

        if (rRenderContext.GetFillColor() == COL_WHITE)
        {
            rRenderContext.SetFillColor(Color(COL_BLACK));
        }
        else
        {
            rRenderContext.SetFillColor(Color(COL_WHITE));
        }

        long n = 0;
        while (n <= nHalfSize)
        {
            rRenderContext.DrawRect(Rectangle(nX + n, nY + n, nX + n, nY + nSize - n));
            ++n;
        }

        rRenderContext.Pop();
    }
};

/******************************************************************************
 *
 * SvxMenuEntriesListBox is the listbox in which the menu items for a
 * particular menu are shown. We have a custom listbox because we need
 * to add drag'n'drop support from the Macro Selector and within the
 * listbox
 *
 *****************************************************************************/
SvxMenuEntriesListBox::SvxMenuEntriesListBox(vcl::Window* pParent, SvxConfigPage* pPg)
    : SvTreeListBox(pParent, WB_TABSTOP|WB_CLIPCHILDREN|WB_HIDESELECTION|WB_BORDER)
    , pPage(pPg)
    , m_bIsInternalDrag( false )
{
    SetSpaceBetweenEntries( 3 );
    SetEntryHeight( ENTRY_HEIGHT );

    SetHighlightRange();
    SetSelectionMode(SINGLE_SELECTION);

    SetDragDropMode( DragDropMode::CTRL_MOVE  |
                     DragDropMode::APP_COPY   |
                     DragDropMode::ENABLE_TOP |
                     DragDropMode::APP_DROP);
}

SvxMenuEntriesListBox::~SvxMenuEntriesListBox()
{
    disposeOnce();
}

void SvxMenuEntriesListBox::dispose()
{
    pPage.clear();
    SvTreeListBox::dispose();
}

// drag and drop support
DragDropMode SvxMenuEntriesListBox::NotifyStartDrag(
    TransferDataContainer& aTransferDataContainer, SvTreeListEntry* pEntry )
{
    (void)aTransferDataContainer;
    (void)pEntry;

    m_bIsInternalDrag = true;
    return GetDragDropMode();
}

void SvxMenuEntriesListBox::DragFinished( sal_Int8 nDropAction )
{
    (void)nDropAction;
    m_bIsInternalDrag = false;
}

sal_Int8 SvxMenuEntriesListBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    if ( m_bIsInternalDrag )
    {
        // internal copy isn't allowed!
        if ( rEvt.mnAction == DND_ACTION_COPY )
            return DND_ACTION_NONE;
        else
            return SvTreeListBox::AcceptDrop( rEvt );
    }

    // Always do COPY instead of MOVE if D&D comes from outside!
    AcceptDropEvent aNewAcceptDropEvent( rEvt );
    aNewAcceptDropEvent.mnAction = DND_ACTION_COPY;
    return SvTreeListBox::AcceptDrop( aNewAcceptDropEvent );
}

bool SvxMenuEntriesListBox::NotifyAcceptDrop( SvTreeListEntry* )
{
    return true;
}

TriState SvxMenuEntriesListBox::NotifyMoving(
    SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
    SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos)
{
    // only try to do a move if we are dragging within the list box
    if ( m_bIsInternalDrag )
    {
        if ( pPage->MoveEntryData( pSource, pTarget ) )
        {
            SvTreeListBox::NotifyMoving(
                pTarget, pSource, rpNewParent, rNewChildPos );
            return TRISTATE_TRUE;
        }
        else
        {
            return TRISTATE_FALSE;
        }
    }
    else
    {
        return NotifyCopying( pTarget, pSource, rpNewParent, rNewChildPos );
    }
}

TriState SvxMenuEntriesListBox::NotifyCopying(
    SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
    SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos)
{
    (void)pSource;
    (void)rpNewParent;
    (void)rNewChildPos;

    if ( !m_bIsInternalDrag )
    {
        // if the target is NULL then add function to the start of the list
        pPage->AddFunction( pTarget, pTarget == nullptr );

        // AddFunction already adds the listbox entry so return TRISTATE_FALSE
        // to stop another listbox entry being added
        return TRISTATE_FALSE;
    }

    // Copying is only allowed from external controls, not within the listbox
    return TRISTATE_FALSE;
}

void SvxMenuEntriesListBox::KeyInput( const KeyEvent& rKeyEvent )
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
        // pass on to superclass
        SvTreeListBox::KeyInput( rKeyEvent );
    }
}

/******************************************************************************
 *
 * SvxConfigPage is the abstract base class on which the Menu and Toolbar
 * configuration tabpages are based. It includes methods which are common to
 * both tabpages to add, delete, move and rename items etc.
 *
 *****************************************************************************/
SvxConfigPage::SvxConfigPage(vcl::Window *pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "MenuAssignPage", "cui/ui/menuassignpage.ui", &rSet)
    , bInitialised(false)
    , pCurrentSaveInData(nullptr)
    , m_pContentsListBox(nullptr)
    , m_pSelectorDlg(nullptr)
{
    get(m_pTopLevel, "toplevel");
    get(m_pTopLevelLabel, "toplevelft");
    get(m_pTopLevelListBox, "toplevellist");
    get(m_pNewTopLevelButton, "toplevelbutton");
    get(m_pModifyTopLevelButton, "menuedit");
    get(m_pContents, "contents");
    get(m_pContentsLabel, "contentslabel");
    get(m_pAddCommandsButton, "add");
    get(m_pModifyCommandButton, "modify");
    get(m_pMoveUpButton, "up");
    get(m_pMoveDownButton, "down");
    get(m_pSaveInListBox, "savein");
    get(m_pDescriptionField, "desc");
    m_pDescriptionField->set_height_request(m_pDescriptionField->GetTextHeight()*4);
    get(m_pEntries, "entries");
    Size aSize(LogicToPixel(Size(108, 115), MAP_APPFONT));
    m_pEntries->set_height_request(aSize.Height());
    m_pEntries->set_width_request(aSize.Width());

    m_pDescriptionField->SetControlBackground( GetSettings().GetStyleSettings().GetDialogColor() );
    m_pDescriptionField->EnableCursor( false );
}

SvxConfigPage::~SvxConfigPage()
{
    disposeOnce();
}

void SvxConfigPage::dispose()
{
    m_pTopLevel.clear();
    m_pTopLevelLabel.clear();
    m_pTopLevelListBox.clear();
    m_pNewTopLevelButton.clear();
    m_pModifyTopLevelButton.clear();
    m_pContents.clear();
    m_pContentsLabel.clear();
    m_pEntries.clear();
    m_pAddCommandsButton.clear();
    m_pModifyCommandButton.clear();
    m_pMoveUpButton.clear();
    m_pMoveDownButton.clear();
    m_pSaveInListBox.clear();
    m_pDescriptionField.clear();

    m_pSelectorDlg.disposeAndClear();
    m_pContentsListBox.disposeAndClear();
    SfxTabPage::dispose();
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
        OUString aModuleName = GetUIModuleName( aModuleId, xModuleManager );

        OUString title = m_pTopLevel->get_label();
        OUString aSearchString("%MODULENAME" );
        sal_Int32 index = title.indexOf( aSearchString );

        if ( index != -1 )
        {
            title = title.replaceAt(
                index, aSearchString.getLength(), aModuleName );
            m_pTopLevel->set_label(title);
        }

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
            nPos = m_pSaveInListBox->InsertEntry(
                utl::ConfigManager::getProductName() + " " + aModuleName );
            m_pSaveInListBox->SetEntryData( nPos, pModuleData );
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
                nPos = m_pSaveInListBox->InsertEntry( aTitle );
                m_pSaveInListBox->SetEntryData( nPos, pDocData );
            }
        }

        // if an item to select has been passed in (eg. the ResourceURL for a
        // toolbar) then try to select the SaveInData entry that has that item
        bool bURLToSelectFound = false;
        if ( !m_aURLToSelect.isEmpty() )
        {
            if ( pDocData && pDocData->HasURL( m_aURLToSelect ) )
            {
                m_pSaveInListBox->SelectEntryPos( nPos );
                pCurrentSaveInData = pDocData;
                bURLToSelectFound = true;
            }
            else if ( pModuleData && pModuleData->HasURL( m_aURLToSelect ) )
            {
                m_pSaveInListBox->SelectEntryPos( 0 );
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
                m_pSaveInListBox->SelectEntryPos( nPos );
                pCurrentSaveInData = pDocData;
            }
            else
            {
                m_pSaveInListBox->SelectEntryPos( 0 );
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
                DBG_UNHANDLED_EXCEPTION();
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

                    if ( aModuleId.equals( aCheckId ) )
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
                                nPos = m_pSaveInListBox->InsertEntry( aTitle2 );
                                m_pSaveInListBox->SetEntryData( nPos, pData );
                            }
                        }
                    }
                }
            }
        }

        m_pSaveInListBox->SetSelectHdl(
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
        DBG_UNHANDLED_EXCEPTION();
    }

    return sModuleID;
}

bool SvxConfigPage::FillItemSet( SfxItemSet* )
{
    bool result = false;

    for ( sal_Int32 i = 0 ; i < m_pSaveInListBox->GetEntryCount(); ++i )
    {
        SaveInData* pData =
            static_cast<SaveInData*>(m_pSaveInListBox->GetEntryData( i ));

        result = pData->Apply();
    }
    return result;
}

IMPL_LINK_NOARG_TYPED( SvxConfigPage, SelectSaveInLocation, ListBox&, void )
{
    pCurrentSaveInData = static_cast<SaveInData*>(m_pSaveInListBox->GetEntryData(
            m_pSaveInListBox->GetSelectEntryPos()));

    Init();
}

void SvxConfigPage::ReloadTopLevelListBox( SvxConfigEntry* pToSelect )
{
    sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectEntryPos();
    m_pTopLevelListBox->Clear();

    if ( GetSaveInData() && GetSaveInData()->GetEntries() )
    {
        SvxEntries::const_iterator iter = GetSaveInData()->GetEntries()->begin();
        SvxEntries::const_iterator end = GetSaveInData()->GetEntries()->end();

        for ( ; iter != end; ++iter )
        {
            SvxConfigEntry* pEntryData = *iter;
            const sal_Int32 nPos = m_pTopLevelListBox->InsertEntry( stripHotKey( pEntryData->GetName() ) );
            m_pTopLevelListBox->SetEntryData( nPos, pEntryData );

            if ( pEntryData == pToSelect )
                nSelectionPos = nPos;

            AddSubMenusToUI( stripHotKey( pEntryData->GetName() ), pEntryData );
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

    nSelectionPos = nSelectionPos < m_pTopLevelListBox->GetEntryCount() ?
        nSelectionPos : m_pTopLevelListBox->GetEntryCount() - 1;

    m_pTopLevelListBox->SelectEntryPos( nSelectionPos );
    m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );
}

void SvxConfigPage::AddSubMenusToUI(
    const OUString& rBaseTitle, SvxConfigEntry* pParentData )
{
    SvxEntries::const_iterator iter = pParentData->GetEntries()->begin();
    SvxEntries::const_iterator end = pParentData->GetEntries()->end();

    for ( ; iter != end; ++iter )
    {
        SvxConfigEntry* pEntryData = *iter;

        if ( pEntryData->IsPopup() )
        {
            OUString subMenuTitle( rBaseTitle );
            subMenuTitle += aMenuSeparatorStr;
            subMenuTitle += stripHotKey( pEntryData->GetName() );

            const sal_Int32 nPos = m_pTopLevelListBox->InsertEntry( subMenuTitle );
            m_pTopLevelListBox->SetEntryData( nPos, pEntryData );

            AddSubMenusToUI( subMenuTitle, pEntryData );
        }
    }
}

SvxEntries* SvxConfigPage::FindParentForChild(
    SvxEntries* pRootEntries, SvxConfigEntry* pChildData )
{
    SvxEntries::const_iterator iter = pRootEntries->begin();
    SvxEntries::const_iterator end = pRootEntries->end();

    for ( ; iter != end; ++iter )
    {
        SvxConfigEntry* pEntryData = *iter;

        if ( pEntryData == pChildData )
        {
            return pRootEntries;
        }
        else if ( pEntryData->IsPopup() )
        {
            SvxEntries* result =
                FindParentForChild( pEntryData->GetEntries(), pChildData );

            if ( result != nullptr )
            {
                return result;
            }
        }
    }
    return nullptr;
}

SvTreeListEntry* SvxConfigPage::AddFunction(
    SvTreeListEntry* pTarget, bool bFront, bool bAllowDuplicates )
{
    OUString aDisplayName = m_pSelectorDlg->GetSelectedDisplayName();
    OUString aURL = m_pSelectorDlg->GetScriptURL();

    if ( aURL.isEmpty() )
    {
        return nullptr;
    }

    SvxConfigEntry* pNewEntryData =
        new SvxConfigEntry( aDisplayName, aURL, false );
    pNewEntryData->SetUserDefined();

    // check that this function is not already in the menu
    SvxConfigEntry* pParent = GetTopLevelSelection();

    if ( !bAllowDuplicates )
    {
        for (SvxEntries::const_iterator iter(pParent->GetEntries()->begin()), end(pParent->GetEntries()->end());
             iter != end ; ++iter)
        {
            SvxConfigEntry *pCurEntry = *iter;

            if ( pCurEntry->GetCommand() == pNewEntryData->GetCommand() )
            {
                // asynchronous error message, because of MsgBoxes
                PostUserEvent(
                    LINK( this, SvxConfigPage, AsyncInfoMsg ), nullptr, true );
                delete pNewEntryData;
                return nullptr;
            }
        }
    }

    return InsertEntry( pNewEntryData, pTarget, bFront );
}

SvTreeListEntry* SvxConfigPage::InsertEntry(
    SvxConfigEntry* pNewEntryData,
    SvTreeListEntry* pTarget,
    bool bFront )
{
    // Grab the entries list for the currently selected menu
    SvxEntries* pEntries = GetTopLevelSelection()->GetEntries();

    SvTreeListEntry* pNewEntry = nullptr;
    SvTreeListEntry* pCurEntry =
        pTarget != nullptr ? pTarget : m_pContentsListBox->GetCurEntry();

    if ( bFront )
    {
        pEntries->insert( pEntries->begin(), pNewEntryData );
        pNewEntry = InsertEntryIntoUI( pNewEntryData, 0 );
    }
    else if ( pCurEntry == nullptr || pCurEntry == m_pContentsListBox->Last() )
    {
        pEntries->push_back( pNewEntryData );
        pNewEntry = InsertEntryIntoUI( pNewEntryData );
    }
    else
    {
        SvxConfigEntry* pEntryData =
            static_cast<SvxConfigEntry*>(pCurEntry->GetUserData());

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
            pNewEntry = InsertEntryIntoUI( pNewEntryData, nPos );
        }
    }

    if ( pNewEntry != nullptr )
    {
        m_pContentsListBox->Select( pNewEntry );
        m_pContentsListBox->MakeVisible( pNewEntry );

        GetSaveInData()->SetModified();
        GetTopLevelSelection()->SetModified();
    }

    return pNewEntry;
}

SvTreeListEntry* SvxConfigPage::InsertEntryIntoUI(
    SvxConfigEntry* pNewEntryData, sal_uLong nPos )
{
    SvTreeListEntry* pNewEntry = nullptr;

    if (pNewEntryData->IsSeparator())
    {
        pNewEntry = m_pContentsListBox->InsertEntry(
            OUString(aSeparatorStr),
            nullptr, false, nPos, pNewEntryData);
    }
    else
    {
        OUString aName = stripHotKey( pNewEntryData->GetName() );

        Image aImage = GetSaveInData()->GetImage(
            pNewEntryData->GetCommand());

        if ( !!aImage )
        {
            pNewEntry = m_pContentsListBox->InsertEntry(
                aName, aImage, aImage, nullptr, false, nPos, pNewEntryData );
        }
        else
        {
            pNewEntry = m_pContentsListBox->InsertEntry(
                aName, nullptr, false, nPos, pNewEntryData );
        }

        if ( pNewEntryData->IsPopup() ||
             pNewEntryData->GetStyle() & css::ui::ItemStyle::DROP_DOWN )
        {
            // add new popup painter, it gets destructed by the entry
            pNewEntry->ReplaceItem(
                std::unique_ptr<PopupPainter>(new PopupPainter(pNewEntry, aName)),
                pNewEntry->ItemCount() - 1 );
        }
    }

    return pNewEntry;
}

IMPL_LINK_NOARG_TYPED( SvxConfigPage, AsyncInfoMsg, void*, void )
{
    // Asynchronous msg because of D&D
    ScopedVclPtr<MessageDialog>::Create( this,
        CUI_RES( RID_SVXSTR_MNUCFG_ALREADY_INCLUDED ),
        VCL_MESSAGE_INFO )->Execute();
}

IMPL_LINK_TYPED( SvxConfigPage, MoveHdl, Button *, pButton, void )
{
    MoveEntry(pButton == m_pMoveUpButton);
}

void SvxConfigPage::MoveEntry( bool bMoveUp )
{
    SvTreeListEntry *pSourceEntry = m_pContentsListBox->FirstSelected();
    SvTreeListEntry *pTargetEntry = nullptr;
    SvTreeListEntry *pToSelect = nullptr;

    if ( !pSourceEntry )
    {
        return;
    }

    if ( bMoveUp )
    {
        // Move Up is just a Move Down with the source and target reversed
        pTargetEntry = pSourceEntry;
        pSourceEntry = SvTreeListBox::PrevSibling( pTargetEntry );
        pToSelect = pTargetEntry;
    }
    else
    {
        pTargetEntry = SvTreeListBox::NextSibling( pSourceEntry );
        pToSelect = pSourceEntry;
    }

    if ( MoveEntryData( pSourceEntry, pTargetEntry ) )
    {
        m_pContentsListBox->GetModel()->Move( pSourceEntry, pTargetEntry );
        m_pContentsListBox->Select( pToSelect );
        m_pContentsListBox->MakeVisible( pToSelect );

        UpdateButtonStates();
    }
}

bool SvxConfigPage::MoveEntryData(
    SvTreeListEntry* pSourceEntry, SvTreeListEntry* pTargetEntry )
{
    //#i53677#
    if (nullptr == pSourceEntry || nullptr == pTargetEntry)
    {
        return false;
    }

    // Grab the entries list for the currently selected menu
    SvxEntries* pEntries = GetTopLevelSelection()->GetEntries();

    SvxConfigEntry* pSourceData =
        static_cast<SvxConfigEntry*>(pSourceEntry->GetUserData());

    SvxConfigEntry* pTargetData =
        static_cast<SvxConfigEntry*>(pTargetEntry->GetUserData());

    if ( pSourceData != nullptr && pTargetData != nullptr )
    {
        // remove the source entry from our list
        RemoveEntry( pEntries, pSourceData );

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

SvxMenuConfigPage::SvxMenuConfigPage(vcl::Window *pParent, const SfxItemSet& rSet, bool bIsMenuBar)
    : SvxConfigPage(pParent, rSet)
    , m_bIsMenuBar( bIsMenuBar )
{
    m_pContentsListBox = VclPtr<SvxMenuEntriesListBox>::Create(m_pEntries, this);
    m_pContentsListBox->set_grid_left_attach(0);
    m_pContentsListBox->set_grid_top_attach(0);
    m_pContentsListBox->set_hexpand(true);
    m_pContentsListBox->set_vexpand(true);
    m_pContentsListBox->Show();

    m_pTopLevelListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenu ) );

    m_pContentsListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenuEntry ) );

    m_pMoveUpButton->SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );
    m_pMoveDownButton->SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );

    m_pNewTopLevelButton->SetClickHdl  (
        LINK( this, SvxMenuConfigPage, NewMenuHdl ) );

    m_pAddCommandsButton->SetClickHdl  (
        LINK( this, SvxMenuConfigPage, AddCommandsHdl ) );

    PopupMenu* pMenu = m_pModifyTopLevelButton->GetPopupMenu();
    pMenu->SetMenuFlags(
        pMenu->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    m_pModifyTopLevelButton->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, MenuSelectHdl ) );

    PopupMenu* pEntry = m_pModifyCommandButton->GetPopupMenu();
    pEntry->SetMenuFlags(
        pEntry->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    m_pModifyCommandButton->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, EntrySelectHdl ) );

    if ( !bIsMenuBar )
    {
        m_pTopLevel->set_label( CUI_RES( RID_SVXSTR_PRODUCTNAME_CONTEXTMENUS ) );
        m_pNewTopLevelButton->Hide();
        pMenu->HideItem( pMenu->GetItemId( "move" ) );
        pMenu->HideItem( pMenu->GetItemId( "menuitem3" ) );
    }
}

SvxMenuConfigPage::~SvxMenuConfigPage()
{
    disposeOnce();
}

// Populates the Menu combo box
void SvxMenuConfigPage::Init()
{
    // ensure that the UI is cleared before populating it
    m_pTopLevelListBox->Clear();
    m_pContentsListBox->Clear();

    ReloadTopLevelListBox();

    m_pTopLevelListBox->SelectEntryPos(0);
    m_pTopLevelListBox->GetSelectHdl().Call(*m_pTopLevelListBox);
}

void SvxMenuConfigPage::dispose()
{
    for ( sal_Int32 i = 0 ; i < m_pSaveInListBox->GetEntryCount(); ++i )
    {
        MenuSaveInData* pData =
            static_cast<MenuSaveInData*>(m_pSaveInListBox->GetEntryData( i ));

        delete pData;
    }
    m_pSaveInListBox->Clear();

    SvxConfigPage::dispose();
}

IMPL_LINK_NOARG_TYPED( SvxMenuConfigPage, SelectMenuEntry, SvTreeListBox *, void )
{
    UpdateButtonStates();
}

void SvxMenuConfigPage::UpdateButtonStates()
{
    PopupMenu* pPopup = m_pModifyCommandButton->GetPopupMenu();

    // Disable Up and Down buttons depending on current selection
    SvTreeListEntry* selection = m_pContentsListBox->GetCurEntry();

    if ( m_pContentsListBox->GetEntryCount() == 0 || selection == nullptr )
    {
        m_pMoveUpButton->Enable( false );
        m_pMoveDownButton->Enable( false );

        pPopup->EnableItem( "addseparator" );
        pPopup->EnableItem( "modrename", false );
        pPopup->EnableItem( "moddelete", false );

        m_pDescriptionField->SetText("");

        return;
    }

    SvTreeListEntry* first = m_pContentsListBox->First();
    SvTreeListEntry* last = m_pContentsListBox->Last();

    m_pMoveUpButton->Enable( selection != first );
    m_pMoveDownButton->Enable( selection != last );

    SvxConfigEntry* pEntryData =
        static_cast<SvxConfigEntry*>(selection->GetUserData());

    if ( pEntryData->IsSeparator() )
    {
        pPopup->EnableItem( "moddelete" );
        pPopup->EnableItem( "addseparator", false );
        pPopup->EnableItem( "modrename", false );

        m_pDescriptionField->SetText("");
    }
    else
    {
        pPopup->EnableItem( "addseparator" );
        pPopup->EnableItem( "moddelete" );
        pPopup->EnableItem( "modrename" );

        m_pDescriptionField->SetText(pEntryData->GetHelpText());
    }
}

void SvxMenuConfigPage::DeleteSelectedTopLevel()
{
    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    SvxEntries* pParentEntries =
        FindParentForChild( GetSaveInData()->GetEntries(), pMenuData );

    RemoveEntry( pParentEntries, pMenuData );
    delete pMenuData;

    ReloadTopLevelListBox();

    GetSaveInData()->SetModified( );
}

bool SvxMenuConfigPage::DeleteSelectedContent()
{
    SvTreeListEntry *pActEntry = m_pContentsListBox->FirstSelected();

    if ( pActEntry != nullptr )
    {
        // get currently selected menu entry
        SvxConfigEntry* pMenuEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        // get currently selected menu
        SvxConfigEntry* pMenu = GetTopLevelSelection();

        // remove menu entry from the list for this menu
        RemoveEntry( pMenu->GetEntries(), pMenuEntry );

        // remove menu entry from UI
        m_pContentsListBox->GetModel()->Remove( pActEntry );

        // if this is a submenu entry, redraw the menus list box
        if ( pMenuEntry->IsPopup() )
        {
            ReloadTopLevelListBox();
        }

        // delete data for menu entry
        delete pMenuEntry;

        GetSaveInData()->SetModified();
        pMenu->SetModified();

        return true;
    }
    return false;
}

short SvxMenuConfigPage::QueryReset()
{
    OUString msg = CUI_RES( RID_SVXSTR_CONFIRM_MENU_RESET );

    OUString saveInName = m_pSaveInListBox->GetEntry(
        m_pSaveInListBox->GetSelectEntryPos() );

    OUString label = replaceSaveInName( msg, saveInName );

    ScopedVclPtrInstance<QueryBox> qbox( this, WB_YES_NO, label );

    return qbox->Execute();
}

IMPL_LINK_NOARG_TYPED( SvxMenuConfigPage, SelectMenu, ListBox&, void )
{
    m_pContentsListBox->Clear();

    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    PopupMenu* pPopup = m_pModifyTopLevelButton->GetPopupMenu();
    if ( pMenuData )
    {
        pPopup->EnableItem( "delete", pMenuData->IsDeletable() );
        pPopup->EnableItem( "rename", pMenuData->IsRenamable() );
        pPopup->EnableItem( "move", pMenuData->IsMovable() );

        SvxEntries* pEntries = pMenuData->GetEntries();
        SvxEntries::const_iterator iter = pEntries->begin();

        for ( ; iter != pEntries->end(); ++iter )
        {
            SvxConfigEntry* pEntry = *iter;
            InsertEntryIntoUI( pEntry );
        }
    }

    UpdateButtonStates();
}

IMPL_LINK_TYPED( SvxMenuConfigPage, MenuSelectHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();

    if (sIdent == "delete")
    {
        DeleteSelectedTopLevel();
    }
    else if (sIdent == "rename")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        OUString aNewName( stripHotKey( pMenuData->GetName() ) );
        OUString aDesc = CUI_RESSTR( RID_SVXSTR_LABEL_NEW_NAME );

        VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
        pNameDialog->SetHelpId( HID_SVX_CONFIG_RENAME_MENU );
        pNameDialog->SetText( CUI_RESSTR( RID_SVXSTR_RENAME_MENU ) );

        if ( pNameDialog->Execute() == RET_OK ) {
            pNameDialog->GetName( aNewName );
            pMenuData->SetName( aNewName );

            ReloadTopLevelListBox();

            GetSaveInData()->SetModified();
        }
    }
    else if (sIdent == "move")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        VclPtr<SvxMainMenuOrganizerDialog> pDialog(
            VclPtr<SvxMainMenuOrganizerDialog>::Create( this,
                GetSaveInData()->GetEntries(), pMenuData ));

        if ( pDialog->Execute() == RET_OK )
        {
            GetSaveInData()->SetEntries( pDialog->GetEntries() );

            ReloadTopLevelListBox( pDialog->GetSelectedEntry() );

            GetSaveInData()->SetModified();
        }
    }
}

IMPL_LINK_TYPED( SvxMenuConfigPage, EntrySelectHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();
    if (sIdent == "addsubmenu")
    {
        OUString aNewName;
        OUString aDesc = CUI_RESSTR( RID_SVXSTR_SUBMENU_NAME );

        VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
        pNameDialog->SetHelpId( HID_SVX_CONFIG_NAME_SUBMENU );
        pNameDialog->SetText( CUI_RESSTR( RID_SVXSTR_ADD_SUBMENU ) );

        if ( pNameDialog->Execute() == RET_OK ) {
            pNameDialog->GetName(aNewName);

            SvxConfigEntry* pNewEntryData =
                new SvxConfigEntry( aNewName, aNewName, true );
            pNewEntryData->SetUserDefined();

            InsertEntry( pNewEntryData );

            ReloadTopLevelListBox();

            GetSaveInData()->SetModified();
        }
    }
    else if (sIdent == "addseparator")
    {
        SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
        pNewEntryData->SetUserDefined();
        InsertEntry( pNewEntryData );
    }
    else if (sIdent == "moddelete")
    {
        DeleteSelectedContent();
    }
    else if (sIdent == "modrename")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        OUString aNewName( stripHotKey( pEntry->GetName() ) );
        OUString aDesc = CUI_RESSTR( RID_SVXSTR_LABEL_NEW_NAME );

        VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
        pNameDialog->SetHelpId( HID_SVX_CONFIG_RENAME_MENU_ITEM );
        pNameDialog->SetText( CUI_RESSTR( RID_SVXSTR_RENAME_MENU ) );

        if ( pNameDialog->Execute() == RET_OK ) {
            pNameDialog->GetName(aNewName);

            pEntry->SetName( aNewName );
            m_pContentsListBox->SetEntryText( pActEntry, aNewName );

            GetSaveInData()->SetModified();
        }
    }
    else
    {
        return;
    }

    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK_NOARG_TYPED( SvxMenuConfigPage, AddFunctionHdl, SvxScriptSelectorDialog&, void )
{
    AddFunction();
}

IMPL_LINK_NOARG_TYPED( SvxMenuConfigPage, NewMenuHdl, Button *, void )
{
    VclPtrInstance<SvxMainMenuOrganizerDialog> pDialog(
        nullptr, GetSaveInData()->GetEntries(), nullptr, true );

    if ( pDialog->Execute() == RET_OK )
    {
        GetSaveInData()->SetEntries( pDialog->GetEntries() );
        ReloadTopLevelListBox( pDialog->GetSelectedEntry() );
        GetSaveInData()->SetModified();
    }
}

IMPL_LINK_NOARG_TYPED( SvxMenuConfigPage, AddCommandsHdl, Button *, void )
{
    if ( m_pSelectorDlg == nullptr )
    {
        // Create Script Selector which also shows builtin commands
        m_pSelectorDlg = VclPtr<SvxScriptSelectorDialog>::Create( this, true, m_xFrame );

        m_pSelectorDlg->SetAddHdl(
            LINK( this, SvxMenuConfigPage, AddFunctionHdl ) );

        m_pSelectorDlg->SetDialogDescription( CUI_RES( RID_SVXSTR_MENU_ADDCOMMANDS_DESCRIPTION ) );
    }

    // Position the Script Selector over the Add button so it is
    // beside the menu contents list and does not obscure it
    m_pSelectorDlg->SetPosPixel( m_pAddCommandsButton->GetPosPixel() );

    m_pSelectorDlg->SetImageProvider( GetSaveInData() );

    m_pSelectorDlg->Show();
}

SaveInData* SvxMenuConfigPage::CreateSaveInData(
    const uno::Reference< css::ui::XUIConfigurationManager >& xCfgMgr,
    const uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool bDocConfig )
{
    if ( !m_bIsMenuBar )
        return static_cast< SaveInData* >( new ContextMenuSaveInData( xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig ) );

    return static_cast< SaveInData* >( new MenuSaveInData( xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig ) );
}

SvxMainMenuOrganizerDialog::SvxMainMenuOrganizerDialog(
    vcl::Window* pParent, SvxEntries* entries,
    SvxConfigEntry* selection, bool bCreateMenu )
    : ModalDialog(pParent, "MoveMenuDialog", "cui/ui/movemenu.ui")
    , mpEntries(nullptr)
    , bModified(false)
{
    get(m_pMenuBox, "namebox");
    get(m_pMenuNameEdit, "menuname");
    get(m_pMoveUpButton, "up");
    get(m_pMoveDownButton, "down");
    get(m_pMenuListBox, "menulist");
    m_pMenuListBox->set_height_request(m_pMenuListBox->GetTextHeight() * 12);

    // Copy the entries list passed in
    if ( entries != nullptr )
    {
        mpEntries = new SvxEntries();
        SvxEntries::const_iterator iter = entries->begin();

        while ( iter != entries->end() )
        {
            SvxConfigEntry* pEntry = *iter;
            SvTreeListEntry* pLBEntry =
                m_pMenuListBox->InsertEntry( stripHotKey( pEntry->GetName() ) );
            pLBEntry->SetUserData( pEntry );
            mpEntries->push_back( pEntry );

            if ( pEntry == selection )
            {
                m_pMenuListBox->Select( pLBEntry );
            }
            ++iter;
        }
    }

    if ( bCreateMenu )
    {
        // Generate custom name for new menu
        OUString prefix = CUI_RES( RID_SVXSTR_NEW_MENU );

        OUString newname = generateCustomName( prefix, entries );
        OUString newurl = generateCustomMenuURL( mpEntries );

        SvxConfigEntry* pNewEntryData =
            new SvxConfigEntry( newname, newurl, true );
        pNewEntryData->SetUserDefined();
        pNewEntryData->SetMain();

        pNewMenuEntry =
            m_pMenuListBox->InsertEntry( stripHotKey( pNewEntryData->GetName() ) );
        m_pMenuListBox->Select( pNewMenuEntry );

        pNewMenuEntry->SetUserData( pNewEntryData );

        if (mpEntries)
            mpEntries->push_back(pNewEntryData);

        m_pMenuNameEdit->SetText( newname );
        m_pMenuNameEdit->SetModifyHdl(
            LINK( this, SvxMainMenuOrganizerDialog, ModifyHdl ) );
    }
    else
    {
        pNewMenuEntry = nullptr;

        // hide name label and textfield
        m_pMenuBox->Hide();
        // change the title
        SetText( CUI_RES( RID_SVXSTR_MOVE_MENU ) );
    }

    m_pMenuListBox->SetSelectHdl(
        LINK( this, SvxMainMenuOrganizerDialog, SelectHdl ) );

    m_pMoveUpButton->SetClickHdl (
        LINK( this, SvxMainMenuOrganizerDialog, MoveHdl) );
    m_pMoveDownButton->SetClickHdl (
        LINK( this, SvxMainMenuOrganizerDialog, MoveHdl) );
}

SvxMainMenuOrganizerDialog::~SvxMainMenuOrganizerDialog()
{
    disposeOnce();
}

void SvxMainMenuOrganizerDialog::dispose()
{
    m_pMenuBox.clear();
    m_pMenuNameEdit.clear();
    m_pMenuListBox.clear();
    m_pMoveUpButton.clear();
    m_pMoveDownButton.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SvxMainMenuOrganizerDialog, ModifyHdl, Edit&, void)
{
    // if the Edit control is empty do not change the name
    if (m_pMenuNameEdit->GetText().isEmpty())
    {
        return;
    }

    SvxConfigEntry* pNewEntryData =
        static_cast<SvxConfigEntry*>(pNewMenuEntry->GetUserData());

    pNewEntryData->SetName(m_pMenuNameEdit->GetText());

    m_pMenuListBox->SetEntryText( pNewMenuEntry, pNewEntryData->GetName() );
}

IMPL_LINK_NOARG_TYPED( SvxMainMenuOrganizerDialog, SelectHdl, SvTreeListBox*, void )
{
    UpdateButtonStates();
}

void SvxMainMenuOrganizerDialog::UpdateButtonStates()
{
    // Disable Up and Down buttons depending on current selection
    SvTreeListEntry* selection = m_pMenuListBox->GetCurEntry();
    SvTreeListEntry* first = m_pMenuListBox->First();
    SvTreeListEntry* last = m_pMenuListBox->Last();

    m_pMoveUpButton->Enable( selection != first );
    m_pMoveDownButton->Enable( selection != last );
}

IMPL_LINK_TYPED( SvxMainMenuOrganizerDialog, MoveHdl, Button *, pButton, void )
{
    SvTreeListEntry *pSourceEntry = m_pMenuListBox->FirstSelected();
    SvTreeListEntry *pTargetEntry = nullptr;

    if ( !pSourceEntry )
    {
        return;
    }

    if (pButton == m_pMoveDownButton)
    {
        pTargetEntry = SvTreeListBox::NextSibling( pSourceEntry );
    }
    else if (pButton == m_pMoveUpButton)
    {
        // Move Up is just a Move Down with the source and target reversed
        pTargetEntry = pSourceEntry;
        pSourceEntry = SvTreeListBox::PrevSibling( pTargetEntry );
    }

    if ( pSourceEntry != nullptr && pTargetEntry != nullptr )
    {
        SvxConfigEntry* pSourceData =
            static_cast<SvxConfigEntry*>(pSourceEntry->GetUserData());
        SvxConfigEntry* pTargetData =
            static_cast<SvxConfigEntry*>(pTargetEntry->GetUserData());

        SvxEntries::iterator iter1 = GetEntries()->begin();
        SvxEntries::iterator iter2 = GetEntries()->begin();
        SvxEntries::const_iterator end = GetEntries()->end();

        // Advance the iterators to the positions of the source and target
        while (*iter1 != pSourceData && ++iter1 != end) ;
        while (*iter2 != pTargetData && ++iter2 != end) ;

        // Now swap the entries in the menu list and in the UI
        if ( iter1 != end && iter2 != end )
        {
            std::swap( *iter1, *iter2 );
            m_pMenuListBox->GetModel()->Move( pSourceEntry, pTargetEntry );
            m_pMenuListBox->MakeVisible( pSourceEntry );

            bModified = true;
        }
    }

    if ( bModified )
    {
        UpdateButtonStates();
    }
}


SvxConfigEntry* SvxMainMenuOrganizerDialog::GetSelectedEntry()
{
    return static_cast<SvxConfigEntry*>(m_pMenuListBox->FirstSelected()->GetUserData());
}

const OUString&
SvxConfigEntry::GetHelpText()
{
    if ( aHelpText.isEmpty() )
    {
        if ( !aCommand.isEmpty() )
        {
            aHelpText = Application::GetHelp()->GetHelpText( aCommand, nullptr );
        }
    }

    return aHelpText;
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
    , mpEntries( nullptr )
{
    if (bPopUp)
    {
        mpEntries = new SvxEntries();
    }
}

SvxConfigEntry::~SvxConfigEntry()
{
    if ( mpEntries != nullptr )
    {
        SvxEntries::const_iterator iter = mpEntries->begin();

        for ( ; iter != mpEntries->end(); ++iter )
        {
            delete *iter;
        }
        delete mpEntries;
    }
}

bool SvxConfigEntry::IsMovable()
{
    if ( IsPopup() && !IsMain() )
    {
        return false;
    }
    return true;
}

bool SvxConfigEntry::IsDeletable()
{
    if ( IsMain() && !IsUserDefined() )
    {
        return false;
    }
    return true;
}

bool SvxConfigEntry::IsRenamable()
{
    if ( IsMain() && !IsUserDefined() )
    {
        return false;
    }
    return true;
}

SvxToolbarConfigPage::SvxToolbarConfigPage(vcl::Window *pParent, const SfxItemSet& rSet)
    : SvxConfigPage(pParent, rSet)
{
    SetHelpId( HID_SVX_CONFIG_TOOLBAR );

    m_pContentsListBox = VclPtr<SvxToolbarEntriesListBox>::Create(m_pEntries, this);
    m_pContentsListBox->set_grid_left_attach(0);
    m_pContentsListBox->set_grid_top_attach(0);
    m_pContentsListBox->set_hexpand(true);
    m_pContentsListBox->set_vexpand(true);
    m_pContentsListBox->Show();

    m_pContentsListBox->SetHelpId( HID_SVX_CONFIG_TOOLBAR_CONTENTS );
    m_pNewTopLevelButton->SetHelpId( HID_SVX_NEW_TOOLBAR );
    m_pModifyTopLevelButton->SetHelpId( HID_SVX_MODIFY_TOOLBAR );
    m_pAddCommandsButton->SetHelpId( HID_SVX_NEW_TOOLBAR_ITEM );
    m_pModifyCommandButton->SetHelpId( HID_SVX_MODIFY_TOOLBAR_ITEM );
    m_pSaveInListBox->SetHelpId( HID_SVX_SAVE_IN );
    m_pMoveUpButton->SetHelpId( HID_SVX_UP_TOOLBAR_ITEM );
    m_pMoveDownButton->SetHelpId( HID_SVX_DOWN_TOOLBAR_ITEM );

    m_pTopLevel->set_label(CUI_RES(RID_SVXSTR_PRODUCTNAME_TOOLBARS));

    m_pTopLevelLabel->SetText( CUI_RES( RID_SVXSTR_TOOLBAR ) );
    m_pModifyTopLevelButton->SetText( CUI_RES( RID_SVXSTR_TOOLBAR ) );
    m_pContents->set_label(CUI_RES(RID_SVXSTR_TOOLBAR_CONTENT));
    m_pContentsLabel->SetText( CUI_RES( RID_SVXSTR_COMMANDS ) );

    m_pTopLevelListBox->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectToolbar ) );
    m_pContentsListBox->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectToolbarEntry ) );

    m_pNewTopLevelButton->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, NewToolbarHdl ) );

    m_pAddCommandsButton->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, AddCommandsHdl ) );

    m_pMoveUpButton->SetClickHdl ( LINK( this, SvxToolbarConfigPage, MoveHdl) );
    m_pMoveDownButton->SetClickHdl ( LINK( this, SvxToolbarConfigPage, MoveHdl) );
    // Always enable Up and Down buttons
    // added for issue i53677 by shizhoubo
    m_pMoveDownButton->Enable();
    m_pMoveUpButton->Enable();

    PopupMenu* pMenu = new PopupMenu( CUI_RES( MODIFY_TOOLBAR ) );
    pMenu->SetMenuFlags(
        pMenu->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    m_pModifyTopLevelButton->SetPopupMenu( pMenu );
    m_pModifyTopLevelButton->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, ToolbarSelectHdl ) );

    PopupMenu* pEntry = new PopupMenu(
        CUI_RES( MODIFY_TOOLBAR_CONTENT ) );
    pEntry->SetMenuFlags(
        pEntry->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    m_pModifyCommandButton->SetPopupMenu( pEntry );
    m_pModifyCommandButton->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, EntrySelectHdl ) );

    // default toolbar to select is standardbar unless a different one
    // has been passed in
    m_aURLToSelect = ITEM_TOOLBAR_URL;
    m_aURLToSelect += "standardbar";

    const SfxPoolItem* pItem =
        rSet.GetItem( rSet.GetPool()->GetWhich( SID_CONFIG ) );

    if ( pItem )
    {
        OUString text = static_cast<const SfxStringItem*>(pItem)->GetValue();
        if (text.startsWith( ITEM_TOOLBAR_URL ))
        {
            m_aURLToSelect = text.copy( 0 );
        }
    }
}

SvxToolbarConfigPage::~SvxToolbarConfigPage()
{
    disposeOnce();
}

void SvxToolbarConfigPage::dispose()
{
    for ( sal_Int32 i = 0 ; i < m_pSaveInListBox->GetEntryCount(); ++i )
    {
        ToolbarSaveInData* pData =
            static_cast<ToolbarSaveInData*>(m_pSaveInListBox->GetEntryData( i ));

        delete pData;
    }
    m_pSaveInListBox->Clear();

    SvxConfigPage::dispose();
}

void SvxToolbarConfigPage::DeleteSelectedTopLevel()
{
    const sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectEntryPos();
    ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>( GetSaveInData() );
    pSaveInData->RemoveToolbar( GetTopLevelSelection() );

    if ( m_pTopLevelListBox->GetEntryCount() > 1 )
    {
        // select next entry after the one being deleted
        // selection position is indexed from 0 so need to
        // subtract one from the entry count
        if ( nSelectionPos != m_pTopLevelListBox->GetEntryCount() - 1 )
        {
            m_pTopLevelListBox->SelectEntryPos( nSelectionPos + 1 );
        }
        else
        {
            m_pTopLevelListBox->SelectEntryPos( nSelectionPos - 1 );
        }
        m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );

        // and now remove the entry
        m_pTopLevelListBox->RemoveEntry( nSelectionPos );
    }
    else
    {
        ReloadTopLevelListBox();
    }
}

bool SvxToolbarConfigPage::DeleteSelectedContent()
{
    SvTreeListEntry *pActEntry = m_pContentsListBox->FirstSelected();

    if ( pActEntry != nullptr )
    {
        // get currently selected entry
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        SvxConfigEntry* pToolbar = GetTopLevelSelection();

        // remove entry from the list for this toolbar
        RemoveEntry( pToolbar->GetEntries(), pEntry );

        // remove toolbar entry from UI
        m_pContentsListBox->GetModel()->Remove( pActEntry );

        // delete data for toolbar entry
        delete pEntry;

        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar( pToolbar );
        UpdateButtonStates();

        // if this is the last entry in the toolbar and it is a user
        // defined toolbar pop up a dialog asking the user if they
        // want to delete the toolbar
        if ( m_pContentsListBox->GetEntryCount() == 0 &&
             GetTopLevelSelection()->IsDeletable() )
        {
            ScopedVclPtrInstance<MessageDialog> qbox(this,
                CUI_RES(RID_SXVSTR_CONFIRM_DELETE_TOOLBAR), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

            if ( qbox->Execute() == RET_YES )
            {
                DeleteSelectedTopLevel();
            }
        }

        return true;
    }

    return false;
}

IMPL_LINK_TYPED( SvxToolbarConfigPage, MoveHdl, Button *, pButton, void )
{
    MoveEntry(pButton == m_pMoveUpButton);
}

void SvxToolbarConfigPage::MoveEntry( bool bMoveUp )
{
    SvxConfigPage::MoveEntry( bMoveUp );

    // Apply change to currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if ( pToolbar )
        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar( pToolbar );
    else
    {
        SAL_WARN( "cui.customize", "SvxToolbarConfigPage::MoveEntry(): no entry" );
        UpdateButtonStates();
    }
}

IMPL_LINK_TYPED( SvxToolbarConfigPage, ToolbarSelectHdl, MenuButton *, pButton, void )
{
    sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectEntryPos();

    SvxConfigEntry* pToolbar =
        static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData( nSelectionPos ));

    ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>( GetSaveInData() );

    switch( pButton->GetCurItemId() )
    {
        case ID_DELETE:
        {
            DeleteSelectedTopLevel();
            UpdateButtonStates();
            break;
        }
        case ID_RENAME:
        {
            OUString aNewName( stripHotKey( pToolbar->GetName() ) );
            OUString aDesc = CUI_RESSTR( RID_SVXSTR_LABEL_NEW_NAME );

            VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
            pNameDialog->SetHelpId( HID_SVX_CONFIG_RENAME_TOOLBAR );
            pNameDialog->SetText( CUI_RESSTR( RID_SVXSTR_RENAME_TOOLBAR ) );

            if ( pNameDialog->Execute() == RET_OK )
            {
                pNameDialog->GetName(aNewName);

                pToolbar->SetName( aNewName );
                pSaveInData->ApplyToolbar( pToolbar );

                // have to use remove and insert to change the name
                m_pTopLevelListBox->RemoveEntry( nSelectionPos );
                nSelectionPos =
                    m_pTopLevelListBox->InsertEntry( aNewName, nSelectionPos );
                m_pTopLevelListBox->SetEntryData( nSelectionPos, pToolbar );
                m_pTopLevelListBox->SelectEntryPos( nSelectionPos );
            }
            break;
        }
        case ID_DEFAULT_STYLE:
        {
            ScopedVclPtrInstance<MessageDialog> qbox(this,
                CUI_RES(RID_SVXSTR_CONFIRM_RESTORE_DEFAULT), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

            if ( qbox->Execute() == RET_YES )
            {
                ToolbarSaveInData* pSaveInData_ =
                    static_cast<ToolbarSaveInData*>(GetSaveInData());

                pSaveInData_->RestoreToolbar( pToolbar );

                m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );
            }

            break;
        }
        case ID_ICONS_ONLY:
        {
            pToolbar->SetStyle( 0 );
            pSaveInData->SetSystemStyle( m_xFrame, pToolbar->GetCommand(), 0 );

            m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );

            break;
        }
        case ID_TEXT_ONLY:
        {
            pToolbar->SetStyle( 1 );
            pSaveInData->SetSystemStyle( m_xFrame, pToolbar->GetCommand(), 1 );

            m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );

            break;
        }
        case ID_ICONS_AND_TEXT:
        {
            pToolbar->SetStyle( 2 );
            pSaveInData->SetSystemStyle( m_xFrame, pToolbar->GetCommand(), 2 );

            m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );

            break;
        }
    }
}

IMPL_LINK_TYPED( SvxToolbarConfigPage, EntrySelectHdl, MenuButton *, pButton, void )
{
    bool bNeedsApply = false;

    // get currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    switch( pButton->GetCurItemId() )
    {
        case ID_RENAME:
        {
            SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

            OUString aNewName( stripHotKey( pEntry->GetName() ) );
            OUString aDesc = CUI_RESSTR( RID_SVXSTR_LABEL_NEW_NAME );

            VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
            pNameDialog->SetHelpId( HID_SVX_CONFIG_RENAME_TOOLBAR_ITEM );
            pNameDialog->SetText( CUI_RESSTR( RID_SVXSTR_RENAME_TOOLBAR ) );

            if ( pNameDialog->Execute() == RET_OK ) {
                pNameDialog->GetName(aNewName);

                if( aNewName == "" ) //tdf#80758 - Accelerator character ("~") is passed as
                    pEntry->SetName( "~" ); // the button name in case of empty values.
                else
                    pEntry->SetName( aNewName );

                m_pContentsListBox->SetEntryText( pActEntry, aNewName );
                bNeedsApply = true;
            }
            break;
        }
        case ID_DEFAULT_COMMAND:
        {
            SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

            sal_uInt16 nSelectionPos = 0;

            // find position of entry within the list
            for ( sal_uLong i = 0; i < m_pContentsListBox->GetEntryCount(); ++i )
            {
                if ( m_pContentsListBox->GetEntry( nullptr, i ) == pActEntry )
                {
                    nSelectionPos = i;
                    break;
                }
            }

            ToolbarSaveInData* pSaveInData =
                static_cast<ToolbarSaveInData*>( GetSaveInData() );

            OUString aSystemName =
                pSaveInData->GetSystemUIName( pEntry->GetCommand() );

            if ( !pEntry->GetName().equals( aSystemName ) )
            {
                pEntry->SetName( aSystemName );
                m_pContentsListBox->SetEntryText(
                    pActEntry, stripHotKey( aSystemName ) );
                bNeedsApply = true;
            }

            uno::Sequence<OUString> aURLSeq { pEntry->GetCommand() };

            try
            {
                GetSaveInData()->GetImageManager()->removeImages(
                    GetImageType(), aURLSeq );

                // reset backup in entry
                pEntry->SetBackupGraphic(
                    uno::Reference< graphic::XGraphic >() );

                GetSaveInData()->PersistChanges(
                    GetSaveInData()->GetImageManager() );

                m_pContentsListBox->GetModel()->Remove( pActEntry );

                SvTreeListEntry* pNewLBEntry =
                    InsertEntryIntoUI( pEntry, nSelectionPos );

                m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                    pEntry->IsVisible() ?
                        SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );

                m_pContentsListBox->Select( pNewLBEntry );
                m_pContentsListBox->MakeVisible( pNewLBEntry );

                bNeedsApply = true;
            }
               catch ( uno::Exception& )
               {
                OSL_TRACE("Error restoring image");
               }
            break;
        }
        case ID_BEGIN_GROUP:
        {
            SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
            pNewEntryData->SetUserDefined();

            SvTreeListEntry* pNewLBEntry = InsertEntry( pNewEntryData );

            m_pContentsListBox->SetCheckButtonInvisible( pNewLBEntry );
            m_pContentsListBox->SetCheckButtonState(
                pNewLBEntry, SV_BUTTON_TRISTATE );

            bNeedsApply = true;
            break;
        }
        case ID_DELETE:
        {
            DeleteSelectedContent();
            break;
        }
        case ID_ICON_ONLY:
        {
            break;
        }
        case ID_TEXT_ONLY:
        {
            break;
        }
        case ID_ICON_AND_TEXT:
        {
            break;
        }
        case ID_CHANGE_SYMBOL:
        {
            SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

            sal_uInt16 nSelectionPos = 0;

            // find position of entry within the list
            for ( sal_uLong i = 0; i < m_pContentsListBox->GetEntryCount(); ++i )
            {
                if ( m_pContentsListBox->GetEntry( nullptr, i ) == pActEntry )
                {
                    nSelectionPos = i;
                    break;
                }
            }

            VclPtr<SvxIconSelectorDialog> pIconDialog(
                VclPtr<SvxIconSelectorDialog>::Create( nullptr,
                    GetSaveInData()->GetImageManager(),
                    GetSaveInData()->GetParentImageManager() ));

            if ( pIconDialog->Execute() == RET_OK )
            {
                uno::Reference< graphic::XGraphic > newgraphic =
                    pIconDialog->GetSelectedIcon();

                if ( newgraphic.is() )
                {
                    uno::Sequence< uno::Reference< graphic::XGraphic > >
                        aGraphicSeq( 1 );

                    uno::Sequence<OUString> aURLSeq { pEntry->GetCommand() };

                    if ( !pEntry->GetBackupGraphic().is() )
                    {
                        uno::Reference< graphic::XGraphic > backup;
                        backup = GetGraphic(
                            GetSaveInData()->GetImageManager(), aURLSeq[ 0 ] );

                        if ( backup.is() )
                        {
                            pEntry->SetBackupGraphic( backup );
                        }
                    }

                    aGraphicSeq[ 0 ] = newgraphic;
                    try
                    {
                        GetSaveInData()->GetImageManager()->replaceImages(
                            GetImageType(), aURLSeq, aGraphicSeq );

                        Image aImage( newgraphic );

                        m_pContentsListBox->GetModel()->Remove( pActEntry );
                        SvTreeListEntry* pNewLBEntry =
                            InsertEntryIntoUI( pEntry, nSelectionPos );

                        m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                            pEntry->IsVisible() ?
                                SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );

                        m_pContentsListBox->Select( pNewLBEntry );
                        m_pContentsListBox->MakeVisible( pNewLBEntry );

                        GetSaveInData()->PersistChanges(
                            GetSaveInData()->GetImageManager() );
                    }
                    catch ( uno::Exception& )
                    {
                        OSL_TRACE("Error replacing image");
                    }
                }
            }
            break;
        }
        case ID_RESET_SYMBOL:
        {
            SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

            sal_uInt16 nSelectionPos = 0;

            // find position of entry within the list
            for ( sal_uLong i = 0; i < m_pContentsListBox->GetEntryCount(); ++i )
            {
                if ( m_pContentsListBox->GetEntry( nullptr, i ) == pActEntry )
                {
                    nSelectionPos = i;
                    break;
                }
            }

            uno::Reference< graphic::XGraphic > backup =
                pEntry->GetBackupGraphic();

            uno::Sequence< uno::Reference< graphic::XGraphic > >
                aGraphicSeq( 1 );
            aGraphicSeq[ 0 ] = backup;

            uno::Sequence<OUString> aURLSeq { pEntry->GetCommand() };

            try
            {
                GetSaveInData()->GetImageManager()->replaceImages(
                    GetImageType(), aURLSeq, aGraphicSeq );

                Image aImage( backup );
                m_pContentsListBox->GetModel()->Remove( pActEntry );

                SvTreeListEntry* pNewLBEntry =
                    InsertEntryIntoUI( pEntry, nSelectionPos );

                m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                    pEntry->IsVisible() ?
                        SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );

                m_pContentsListBox->Select( pNewLBEntry );
                m_pContentsListBox->MakeVisible( pNewLBEntry );

                // reset backup in entry
                pEntry->SetBackupGraphic(
                    uno::Reference< graphic::XGraphic >() );

                GetSaveInData()->PersistChanges(
                    GetSaveInData()->GetImageManager() );
            }
            catch ( uno::Exception& )
            {
                OSL_TRACE("Error resetting image");
            }
            break;
        }
    }

    if ( bNeedsApply )
    {
        static_cast<ToolbarSaveInData*>( GetSaveInData())->ApplyToolbar( pToolbar );
        UpdateButtonStates();
    }
}

void SvxToolbarConfigPage::Init()
{
    // ensure that the UI is cleared before populating it
    m_pTopLevelListBox->Clear();
    m_pContentsListBox->Clear();

    ReloadTopLevelListBox();

    sal_Int32 nPos = 0;
    if ( !m_aURLToSelect.isEmpty() )
    {
        for ( sal_Int32 i = 0 ; i < m_pTopLevelListBox->GetEntryCount(); ++i )
        {
            SvxConfigEntry* pData =
                static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData( i ));

            if ( pData->GetCommand().equals( m_aURLToSelect ) )
            {
                nPos = i;
                break;
            }
        }

        // in future select the default toolbar: Standard
        m_aURLToSelect = ITEM_TOOLBAR_URL;
        m_aURLToSelect += "standardbar";
    }

    m_pTopLevelListBox->SelectEntryPos(nPos);
    m_pTopLevelListBox->GetSelectHdl().Call(*m_pTopLevelListBox);
}

SaveInData* SvxToolbarConfigPage::CreateSaveInData(
    const uno::Reference< css::ui::XUIConfigurationManager >& xCfgMgr,
    const uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool bDocConfig )
{
    return static_cast< SaveInData* >(
        new ToolbarSaveInData( xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig ));
}

ToolbarSaveInData::ToolbarSaveInData(
    const uno::Reference < css::ui::XUIConfigurationManager >& xCfgMgr,
    const uno::Reference < css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool docConfig ) :

    SaveInData              ( xCfgMgr, xParentCfgMgr, aModuleId, docConfig ),
    pRootEntry              ( nullptr ),
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
    delete pRootEntry;
}

void ToolbarSaveInData::SetSystemStyle(
    uno::Reference< frame::XFrame > xFrame,
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

        window = VCLUnoHelper::GetWindow( xWindow );
    }

    if ( window != nullptr && window->GetType() == WINDOW_TOOLBOX )
    {
        ToolBox* toolbox = static_cast<ToolBox*>(window);

        if ( nStyle == 0 )
        {
            toolbox->SetButtonType();
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
                        aProps[ i ].Value = uno::makeAny( nStyle );
                        break;
                    }
                }
            }

            uno::Reference< container::XNameReplace >
                xNameReplace( m_xPersistentWindowState, uno::UNO_QUERY );

            xNameReplace->replaceByName( rResourceURL, uno::makeAny( aProps ) );
        }
        catch ( uno::Exception& )
        {
            // do nothing, a default value is returned
            OSL_TRACE("Exception setting toolbar style");
        }
    }
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
    typedef std::unordered_map<OUString, bool,
                               OUStringHash, std::equal_to< OUString > > ToolbarInfo;

    ToolbarInfo aToolbarInfo;

    if ( pRootEntry == nullptr )
    {

        pRootEntry = new SvxConfigEntry(
            OUString("MainToolbars"),
            OUString(), true);

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
                    GetConfigManager()->getSettings( url, sal_False );

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
                    uiname, url, true );

                pEntry->SetMain();
                pEntry->SetStyle( GetSystemStyle( url ) );


                // insert into std::unordered_map to filter duplicates from the parent
                aToolbarInfo.insert( ToolbarInfo::value_type( systemname, true ));

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
                    aToolbarInfo.insert( ToolbarInfo::value_type( systemname, true ));

                    try
                    {
                        uno::Reference< container::XIndexAccess > xToolbarSettings =
                            xParentCfgMgr->getSettings( url, sal_False );

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

        std::sort( GetEntries()->begin(), GetEntries()->end(), EntrySort );
    }

    return pRootEntry->GetEntries();
}

void
ToolbarSaveInData::SetEntries( SvxEntries* pNewEntries )
{
    // delete old menu hierarchy first
    delete pRootEntry->GetEntries();

    // now set new menu hierarchy
    pRootEntry->SetEntries( pNewEntries );
}

bool
ToolbarSaveInData::HasURL( const OUString& rURL )
{
    SvxEntries::const_iterator iter = GetEntries()->begin();
    SvxEntries::const_iterator end = GetEntries()->end();

    while ( iter != end )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( pEntry->GetCommand().equals( rURL ) )
        {
            if ( pEntry->IsParentData() )
                return false;
            else
                return true;
        }

        ++iter;
    }
    return false;
}

bool ToolbarSaveInData::HasSettings()
{
    // return true if there is at least one toolbar entry
    if ( GetEntries()->size() > 0 )
    {
        return true;
    }
    return false;
}

void ToolbarSaveInData::Reset()
{
    SvxEntries::const_iterator toolbars = GetEntries()->begin();
    SvxEntries::const_iterator end = GetEntries()->end();

    // reset each toolbar by calling removeSettings for its toolbar URL
    for ( ; toolbars != end; ++toolbars )
    {
        SvxConfigEntry* pToolbar = *toolbars;

        try
        {
            OUString url = pToolbar->GetCommand();
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
    delete pRootEntry;
    pRootEntry = nullptr;

    // reset all icons to default
    try
    {
        GetImageManager()->reset();
        PersistChanges( GetImageManager() );
    }
    catch ( uno::Exception& )
    {
        OSL_TRACE("Error resetting all icons when resetting toolbars");
    }
}

bool ToolbarSaveInData::Apply()
{
    // toolbar changes are instantly applied
    return false;
}

void ToolbarSaveInData::ApplyToolbar(
    uno::Reference< container::XIndexContainer >& rToolbarBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory,
    SvxConfigEntry* pToolbarData )
{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();

    SvxEntries::const_iterator iter = pToolbarData->GetEntries()->begin();
    SvxEntries::const_iterator end = pToolbarData->GetEntries()->end();

    for ( ; iter != end; ++iter )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( pEntry->IsPopup() )
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                ConvertToolbarEntry( m_xCommandToLabelMap, pEntry );

            uno::Reference< container::XIndexContainer > xSubMenuBar(
                rFactory->createInstanceWithContext( xContext ),
                    uno::UNO_QUERY );

            sal_Int32 nIndex = aPropValueSeq.getLength();
            aPropValueSeq.realloc( nIndex + 1 );
            aPropValueSeq[nIndex].Name = m_aDescriptorContainer;
            aPropValueSeq[nIndex].Value <<= xSubMenuBar;
            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::makeAny( aPropValueSeq ));

            ApplyToolbar( xSubMenuBar, rFactory, pEntry );
        }
        else if ( pEntry->IsSeparator() )
        {
            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::makeAny( m_aSeparatorSeq ));
        }
        else
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                ConvertToolbarEntry( m_xCommandToLabelMap, pEntry );

            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::makeAny( aPropValueSeq ));
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
            uno::makeAny( OUString( pToolbar->GetName() ) ) );
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
    catch ( container::NoSuchElementException& )
    {
        OSL_TRACE("caught container::NoSuchElementException saving settings");
    }
    catch ( css::io::IOException& )
    {
        OSL_TRACE("caught IOException saving settings");
    }
    catch ( css::uno::Exception& )
    {
        OSL_TRACE("caught some other exception saving settings");
    }

    PersistChanges( GetConfigManager() );
}

void ToolbarSaveInData::CreateToolbar( SvxConfigEntry* pToolbar )
{
    // show the new toolbar in the UI also
    uno::Reference< container::XIndexAccess >
        xSettings( GetConfigManager()->createSettings(), uno::UNO_QUERY );

    uno::Reference< container::XIndexContainer >
        xIndexContainer ( xSettings, uno::UNO_QUERY );

    uno::Reference< beans::XPropertySet >
        xPropertySet( xSettings, uno::UNO_QUERY );

    xPropertySet->setPropertyValue(
            ITEM_DESCRIPTOR_UINAME,
            uno::makeAny( pToolbar->GetName() ) );

    try
    {
        GetConfigManager()->insertSettings( pToolbar->GetCommand(), xSettings );
    }
    catch ( container::ElementExistException& )
    {
        OSL_TRACE("caught ElementExistsException saving settings");
    }
    catch ( css::lang::IllegalArgumentException& )
    {
        OSL_TRACE("caught IOException saving settings");
    }
    catch ( css::lang::IllegalAccessException& )
    {
        OSL_TRACE("caught IOException saving settings");
    }
    catch ( css::uno::Exception& )
    {
        OSL_TRACE("caught some other exception saving settings");
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
        RemoveEntry( GetEntries(), pToolbar );
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
            xToolbarSettings = GetParentConfigManager()->getSettings( url, sal_False );
            pToolbar->SetParentData();
        }
        else
            xToolbarSettings = GetConfigManager()->getSettings( url, sal_False );

        LoadToolbar( xToolbarSettings, pToolbar );

        // After reloading, ensure that the icon is reset of each entry
        // in the toolbar
        SvxEntries::const_iterator iter = pToolbar->GetEntries()->begin();
        uno::Sequence< OUString > aURLSeq( 1 );
        for ( ; iter != pToolbar->GetEntries()->end(); ++iter )
        {
            SvxConfigEntry* pEntry = *iter;
            aURLSeq[ 0 ] = pEntry->GetCommand();

            try
            {
                GetImageManager()->removeImages( GetImageType(), aURLSeq );
            }
               catch ( uno::Exception& )
               {
                OSL_TRACE("Error restoring icon when resetting toolbar");
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

bool ToolbarSaveInData::LoadToolbar(
    const uno::Reference< container::XIndexAccess >& xToolbarSettings,
    SvxConfigEntry* pParentData )
{
    SvxEntries*         pEntries            = pParentData->GetEntries();

    for ( sal_Int32 nIndex = 0; nIndex < xToolbarSettings->getCount(); ++nIndex )
    {
        uno::Reference< container::XIndexAccess >   xSubMenu;
        OUString                aCommandURL;
        OUString                aLabel;
        bool                bIsVisible;
        sal_Int32               nStyle;

        sal_uInt16 nType( css::ui::ItemType::DEFAULT );

        bool bItem = GetToolbarItemData( xToolbarSettings, nIndex, aCommandURL,
            aLabel, nType, bIsVisible, nStyle, xSubMenu );

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

                // If custom label not set retrieve it from the command
                // to info service
                if ( aLabel.equals( OUString() ) )
                {
                    uno::Sequence< beans::PropertyValue > aPropSeq;
                    if ( a >>= aPropSeq )
                    {
                        for ( sal_Int32 i = 0; i < aPropSeq.getLength(); ++i )
                        {
                            if ( aPropSeq[i].Name == ITEM_DESCRIPTOR_LABEL )
                            {
                                aPropSeq[i].Value >>= aLabel;
                                break;
                            }
                        }
                    }
                }

                if ( xSubMenu.is() )
                {
                    SvxConfigEntry* pEntry = new SvxConfigEntry(
                        aLabel, aCommandURL, true );

                    pEntry->SetUserDefined( bIsUserDefined );
                    pEntry->SetVisible( bIsVisible );

                    pEntries->push_back( pEntry );

                    LoadToolbar( xSubMenu, pEntry );
                }
                else
                {
                    SvxConfigEntry* pEntry = new SvxConfigEntry(
                        aLabel, aCommandURL, false );
                    pEntry->SetUserDefined( bIsUserDefined );
                    pEntry->SetVisible( bIsVisible );
                    pEntry->SetStyle( nStyle );
                    pEntries->push_back( pEntry );
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

    return true;
}

IMPL_LINK_NOARG_TYPED( SvxToolbarConfigPage, SelectToolbarEntry, SvTreeListBox *, void )
{
    UpdateButtonStates();
}

void SvxToolbarConfigPage::UpdateButtonStates()
{
    PopupMenu* pPopup = m_pModifyCommandButton->GetPopupMenu();
    pPopup->EnableItem( ID_RENAME, false );
    pPopup->EnableItem( ID_DELETE, false );
    pPopup->EnableItem( ID_BEGIN_GROUP, false );
    pPopup->EnableItem( ID_DEFAULT_COMMAND, false );
    pPopup->EnableItem( ID_ICON_ONLY, false );
    pPopup->EnableItem( ID_ICON_AND_TEXT, false );
    pPopup->EnableItem( ID_TEXT_ONLY, false );
    pPopup->EnableItem( ID_CHANGE_SYMBOL, false );
    pPopup->EnableItem( ID_RESET_SYMBOL, false );

    m_pDescriptionField->SetText("");

    SvTreeListEntry* selection = m_pContentsListBox->GetCurEntry();
    if ( m_pContentsListBox->GetEntryCount() == 0 || selection == nullptr )
    {
        return;
    }

    SvxConfigEntry* pEntryData = static_cast<SvxConfigEntry*>(selection->GetUserData());
    if ( pEntryData->IsSeparator() )
        pPopup->EnableItem( ID_DELETE );
    else
    {
        pPopup->EnableItem( ID_BEGIN_GROUP );
        pPopup->EnableItem( ID_DELETE );
        pPopup->EnableItem( ID_RENAME );
        pPopup->EnableItem( ID_ICON_ONLY );
        pPopup->EnableItem( ID_ICON_AND_TEXT );
        pPopup->EnableItem( ID_TEXT_ONLY );
        pPopup->EnableItem( ID_CHANGE_SYMBOL );

        if ( !pEntryData->IsUserDefined() )
            pPopup->EnableItem( ID_DEFAULT_COMMAND );

        if ( pEntryData->IsIconModified() )
            pPopup->EnableItem( ID_RESET_SYMBOL );

        m_pDescriptionField->SetText(pEntryData->GetHelpText());
    }
}

short SvxToolbarConfigPage::QueryReset()
{
    OUString msg = CUI_RES( RID_SVXSTR_CONFIRM_TOOLBAR_RESET );

    OUString saveInName = m_pSaveInListBox->GetEntry(
        m_pSaveInListBox->GetSelectEntryPos() );

    OUString label = replaceSaveInName( msg, saveInName );

    ScopedVclPtrInstance< QueryBox > qbox( this, WB_YES_NO, label );

    return qbox->Execute();
}

IMPL_LINK_NOARG_TYPED( SvxToolbarConfigPage, SelectToolbar, ListBox&, void )
{
    m_pContentsListBox->Clear();

    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if ( pToolbar == nullptr )
    {
        m_pModifyTopLevelButton->Enable( false );
        m_pModifyCommandButton->Enable( false );
        m_pAddCommandsButton->Enable( false );

        return;
    }

    m_pModifyTopLevelButton->Enable();
    m_pModifyCommandButton->Enable();
    m_pAddCommandsButton->Enable();

    PopupMenu* pPopup = m_pModifyTopLevelButton->GetPopupMenu();

    pPopup->EnableItem( ID_DELETE, pToolbar->IsDeletable() );
    pPopup->EnableItem( ID_RENAME, pToolbar->IsRenamable() );
    pPopup->EnableItem( ID_DEFAULT_STYLE, !pToolbar->IsRenamable() );

    switch( pToolbar->GetStyle() )
    {
        case 0:
        {
            pPopup->CheckItem( ID_ICONS_ONLY );
            break;
        }
        case 1:
        {
            pPopup->CheckItem( ID_TEXT_ONLY );
            break;
        }
        case 2:
        {
            pPopup->CheckItem( ID_ICONS_AND_TEXT );
            break;
        }
    }

    SvxEntries* pEntries = pToolbar->GetEntries();
    SvxEntries::const_iterator iter = pEntries->begin();

    for ( ; iter != pEntries->end(); ++iter )
    {
        SvxConfigEntry* pEntry = *iter;

        SvTreeListEntry* pNewLBEntry = InsertEntryIntoUI( pEntry );

        if(pEntry->IsSeparator())
            m_pContentsListBox->SetCheckButtonInvisible( pNewLBEntry );

        if (pEntry->IsBinding())
        {
            m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                pEntry->IsVisible() ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
        }
        else
        {
            m_pContentsListBox->SetCheckButtonState(
                pNewLBEntry, SV_BUTTON_TRISTATE );
        }
    }

    UpdateButtonStates();
}

IMPL_LINK_NOARG_TYPED( SvxToolbarConfigPage, NewToolbarHdl, Button *, void )
{
    OUString prefix = CUI_RES( RID_SVXSTR_NEW_TOOLBAR );

    OUString aNewName =
        generateCustomName( prefix, GetSaveInData()->GetEntries() );

    OUString aNewURL =
        generateCustomURL( GetSaveInData()->GetEntries() );

    VclPtrInstance< SvxNewToolbarDialog > pNameDialog( nullptr, aNewName );

    for ( sal_Int32 i = 0 ; i < m_pSaveInListBox->GetEntryCount(); ++i )
    {
        SaveInData* pData =
            static_cast<SaveInData*>(m_pSaveInListBox->GetEntryData( i ));

        const sal_Int32 nInsertPos = pNameDialog->m_pSaveInListBox->InsertEntry(
            m_pSaveInListBox->GetEntry( i ) );

        pNameDialog->m_pSaveInListBox->SetEntryData( nInsertPos, pData );
    }

    pNameDialog->m_pSaveInListBox->SelectEntryPos(
        m_pSaveInListBox->GetSelectEntryPos() );

    if ( pNameDialog->Execute() == RET_OK )
    {
        aNewName = pNameDialog->GetName();

        sal_Int32 nInsertPos = pNameDialog->m_pSaveInListBox->GetSelectEntryPos();

        ToolbarSaveInData* pData = static_cast<ToolbarSaveInData*>(
            pNameDialog->m_pSaveInListBox->GetEntryData( nInsertPos ));

        if ( GetSaveInData() != pData )
        {
            m_pSaveInListBox->SelectEntryPos( nInsertPos );
            m_pSaveInListBox->GetSelectHdl().Call(*m_pSaveInListBox);
        }

        SvxConfigEntry* pToolbar =
            new SvxConfigEntry( aNewName, aNewURL, true );

        pToolbar->SetUserDefined();
        pToolbar->SetMain();

        pData->CreateToolbar( pToolbar );

        nInsertPos = m_pTopLevelListBox->InsertEntry( pToolbar->GetName() );
        m_pTopLevelListBox->SetEntryData( nInsertPos, pToolbar );
        m_pTopLevelListBox->SelectEntryPos( nInsertPos );
        m_pTopLevelListBox->GetSelectHdl().Call(*m_pTopLevelListBox);

        pData->SetModified();
    }
}

IMPL_LINK_NOARG_TYPED( SvxToolbarConfigPage, AddCommandsHdl, Button *, void )
{
    if ( m_pSelectorDlg == nullptr )
    {
        // Create Script Selector which shows slot commands
        m_pSelectorDlg = VclPtr<SvxScriptSelectorDialog>::Create( this, true, m_xFrame );

        // Position the Script Selector over the Add button so it is
        // beside the menu contents list and does not obscure it
        m_pSelectorDlg->SetPosPixel( m_pAddCommandsButton->GetPosPixel() );

        m_pSelectorDlg->SetAddHdl(
            LINK( this, SvxToolbarConfigPage, AddFunctionHdl ) );
    }

    m_pSelectorDlg->SetImageProvider( GetSaveInData() );

    m_pSelectorDlg->Show();
}

IMPL_LINK_NOARG_TYPED( SvxToolbarConfigPage, AddFunctionHdl, SvxScriptSelectorDialog&, void )
{
    AddFunction();
}

SvTreeListEntry* SvxToolbarConfigPage::AddFunction(
    SvTreeListEntry* pTarget, bool bFront, bool bAllowDuplicates )
{
    SvTreeListEntry* pNewLBEntry =
        SvxConfigPage::AddFunction( pTarget, bFront, bAllowDuplicates );

    SvxConfigEntry* pEntry = static_cast<SvxConfigEntry*>(pNewLBEntry->GetUserData());

    if ( pEntry->IsBinding() )
    {
        pEntry->SetVisible( true );
        m_pContentsListBox->SetCheckButtonState(
            pNewLBEntry, SV_BUTTON_CHECKED );
    }
    else
    {
        m_pContentsListBox->SetCheckButtonState(
            pNewLBEntry, SV_BUTTON_TRISTATE );
    }

    // get currently selected toolbar and apply change
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    if ( pToolbar != nullptr )
    {
        static_cast<ToolbarSaveInData*>( GetSaveInData() )->ApplyToolbar( pToolbar );
    }

    return pNewLBEntry;
}

SvxToolbarEntriesListBox::SvxToolbarEntriesListBox(vcl::Window* pParent, SvxToolbarConfigPage* pPg)
    : SvxMenuEntriesListBox(pParent, pPg)
    , pPage(pPg)
{
    m_pButtonData = new SvLBoxButtonData( this );
    BuildCheckBoxButtonImages( m_pButtonData );
    EnableCheckButton( m_pButtonData );
}

SvxToolbarEntriesListBox::~SvxToolbarEntriesListBox()
{
    disposeOnce();
}

void SvxToolbarEntriesListBox::dispose()
{
    delete m_pButtonData;
    m_pButtonData = nullptr;

    pPage.clear();
    SvxMenuEntriesListBox::dispose();
}

void SvxToolbarEntriesListBox::BuildCheckBoxButtonImages( SvLBoxButtonData* pData )
{
    // Build checkbox images according to the current application
    // settings. This is necessary to be able to have correct colors
    // in all color modes, like high contrast.
    const AllSettings& rSettings = Application::GetSettings();

    ScopedVclPtrInstance< VirtualDevice > pVDev;
    Size            aSize( 26, 20 );

    pVDev->SetOutputSizePixel( aSize );

    Image aImage = GetSizedImage( *pVDev.get(), aSize,
        CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Default ));

    // Fill button data struct with new images
    pData->SetImage(SvBmp::UNCHECKED,     aImage);
    pData->SetImage(SvBmp::CHECKED,       GetSizedImage( *pVDev.get(), aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Checked )) );
    pData->SetImage(SvBmp::HICHECKED,     GetSizedImage( *pVDev.get(), aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed )) );
    pData->SetImage(SvBmp::HIUNCHECKED,   GetSizedImage( *pVDev.get(), aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed)) );
    pData->SetImage(SvBmp::TRISTATE,      GetSizedImage( *pVDev.get(), aSize, Image() ) ); // Use tristate bitmaps to have no checkbox for separator entries
    pData->SetImage(SvBmp::HITRISTATE,    GetSizedImage( *pVDev.get(), aSize, Image() ) );

    // Get image size
    m_aCheckBoxImageSizePixel = aImage.GetSizePixel();
}

Image SvxToolbarEntriesListBox::GetSizedImage(
    VirtualDevice& rVDev, const Size& aNewSize, const Image& aImage )
{
    // Create new checkbox images for treelistbox. They must have a
    // decent width to have a clear column for the visibility checkbox.

    // Standard transparent color is light magenta as is won't be
    // used for other things
    Color   aFillColor( COL_LIGHTMAGENTA );

    // Position image at the center of (width-2),(height) rectangle.
    // We need 2 pixels to have a bigger border to the next button image
    sal_uInt16  nPosX = std::max( (sal_uInt16) (((( aNewSize.Width() - 2 ) - aImage.GetSizePixel().Width() ) / 2 ) - 1), (sal_uInt16) 0 );
    sal_uInt16  nPosY = std::max( (sal_uInt16) (((( aNewSize.Height() - 2 ) - aImage.GetSizePixel().Height() ) / 2 ) + 1), (sal_uInt16) 0 );
    Point   aPos( nPosX > 0 ? nPosX : 0, nPosY > 0 ? nPosY : 0 );
    rVDev.SetFillColor( aFillColor );
    rVDev.SetLineColor( aFillColor );
    rVDev.DrawRect( Rectangle( Point(), aNewSize ));
    rVDev.DrawImage( aPos, aImage );

    // Draw separator line 2 pixels left from the right border
    Color aLineColor = GetDisplayBackground().GetColor().IsDark() ? Color( COL_WHITE ) : Color( COL_BLACK );
    rVDev.SetLineColor( aLineColor );
    rVDev.DrawLine( Point( aNewSize.Width()-3, 0 ), Point( aNewSize.Width()-3, aNewSize.Height()-1 ));

    // Create new image that uses the fillcolor as transparent
    return Image( rVDev.GetBitmap( Point(), aNewSize ), aFillColor );
}

void SvxToolbarEntriesListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvTreeListBox::DataChanged( rDCEvt );

    if (( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) &&
        ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ))
    {
        BuildCheckBoxButtonImages( m_pButtonData );
        Invalidate();
    }
}


void SvxToolbarEntriesListBox::ChangeVisibility( SvTreeListEntry* pEntry )
{
    if ( pEntry != nullptr )
    {
        SvxConfigEntry* pEntryData =
            static_cast<SvxConfigEntry*>(pEntry->GetUserData());

        if ( pEntryData->IsBinding() )
        {
            pEntryData->SetVisible( !pEntryData->IsVisible() );

            SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();

            ToolbarSaveInData* pToolbarSaveInData = static_cast<ToolbarSaveInData*>(
                pPage->GetSaveInData() );

               pToolbarSaveInData->ApplyToolbar( pToolbar );

            SetCheckButtonState( pEntry, pEntryData->IsVisible() ?
                SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
        }
    }
}

void SvxToolbarEntriesListBox::CheckButtonHdl()
{
    ChangeVisibility( GetHdlEntry() );
}

void SvxToolbarEntriesListBox::KeyInput( const KeyEvent& rKeyEvent )
{
    // space key will change visibility of toolbar items
    if ( rKeyEvent.GetKeyCode() == KEY_SPACE )
    {
        ChangeVisibility( GetCurEntry() );
    }
    else
    {
        // pass on to superclass
        SvxMenuEntriesListBox::KeyInput( rKeyEvent );
    }
}

TriState SvxToolbarEntriesListBox::NotifyMoving(
    SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
    SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos)
{
    TriState result = SvxMenuEntriesListBox::NotifyMoving(
        pTarget, pSource, rpNewParent, rNewChildPos );

    if ( result )
    {
        // Instant Apply changes to UI
        SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();
        if ( pToolbar != nullptr )
        {
            ToolbarSaveInData* pSaveInData =
                static_cast<ToolbarSaveInData*>( pPage->GetSaveInData() );
            pSaveInData->ApplyToolbar( pToolbar );
        }
    }

    return result;
}

TriState SvxToolbarEntriesListBox::NotifyCopying(
    SvTreeListEntry*  pTarget,
    SvTreeListEntry*  pSource,
    SvTreeListEntry*& rpNewParent,
    sal_uLong&      rNewChildPos)
{
    (void)pSource;
    (void)rpNewParent;
    (void)rNewChildPos;

    if ( !m_bIsInternalDrag )
    {
        // if the target is NULL then add function to the start of the list
        static_cast<SvxToolbarConfigPage*>(pPage.get())->AddFunction( pTarget, pTarget == nullptr );

        // Instant Apply changes to UI
        SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();
        if ( pToolbar != nullptr )
        {
            ToolbarSaveInData* pSaveInData =
                static_cast<ToolbarSaveInData*>( pPage->GetSaveInData() );
            pSaveInData->ApplyToolbar( pToolbar );
        }

        // AddFunction already adds the listbox entry so return TRISTATE_FALSE
        // to stop another listbox entry being added
        return TRISTATE_FALSE;
    }

    // Copying is only allowed from external controls, not within the listbox
    return TRISTATE_FALSE;
}

SvxNewToolbarDialog::SvxNewToolbarDialog(vcl::Window* pWindow, const OUString& rName)
    : ModalDialog(pWindow, "NewToolbarDialog", "cui/ui/newtoolbardialog.ui")
{
    get(m_pEdtName, "edit");
    get(m_pBtnOK, "ok");
    get(m_pSaveInListBox, "savein");
    m_pEdtName->SetText( rName );
    m_pEdtName->SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
}

SvxNewToolbarDialog::~SvxNewToolbarDialog()
{
    disposeOnce();
}

void SvxNewToolbarDialog::dispose()
{
    m_pEdtName.clear();
    m_pBtnOK.clear();
    m_pSaveInListBox.clear();
    ModalDialog::dispose();
}


/*******************************************************************************
*
* The SvxIconSelectorDialog class
*
*******************************************************************************/
SvxIconSelectorDialog::SvxIconSelectorDialog( vcl::Window *pWindow,
    const uno::Reference< css::ui::XImageManager >& rXImageManager,
    const uno::Reference< css::ui::XImageManager >& rXParentImageManager )
    :
    ModalDialog          ( pWindow, "IconSelector", "cui/ui/iconselectordialog.ui" ),
    m_nNextId            ( 0 ),
    m_xImageManager      ( rXImageManager ),
    m_xParentImageManager( rXParentImageManager )
{
    get(pTbSymbol, "symbolsToolbar");
    get(pFtNote, "noteLabel");
    get(pBtnImport, "importButton");
    get(pBtnDelete, "deleteButton");

    aTbSize = pTbSymbol->LogicToPixel(Size(160, 80), MapMode(MAP_APPFONT));
    pTbSymbol->set_width_request(aTbSize.Width());
    pTbSymbol->set_height_request(aTbSize.Height());
    pTbSymbol->SetStyle(pTbSymbol->GetStyle() | WB_SCROLL | WB_LINESPACING);

    typedef std::unordered_map< OUString, bool,
                                OUStringHash, std::equal_to< OUString > > ImageInfo;

    pTbSymbol->SetPageScroll( true );

    bool bLargeIcons = GetImageType() & css::ui::ImageType::SIZE_LARGE;
    m_nExpectedSize = bLargeIcons ? 26 : 16;

    if ( m_nExpectedSize != 16 )
    {
        pFtNote->SetText( replaceSixteen( pFtNote->GetText(), m_nExpectedSize ) );
    }

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
        pBtnImport->Enable( false );
    }

    aDirectory += "soffice.cfg/import";

    uno::Reference< lang::XSingleServiceFactory > xStorageFactory(
          css::embed::FileSystemStorageFactory::create( xComponentContext ) );

    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[ 0 ] <<= aDirectory;
    aArgs[ 1 ] <<= css::embed::ElementModes::READWRITE;

    uno::Reference< css::embed::XStorage > xStorage(
        xStorageFactory->createInstanceWithArguments( aArgs ), uno::UNO_QUERY );

    uno::Sequence< uno::Any > aProp( 2 );
    beans::PropertyValue aPropValue;

    aPropValue.Name = "UserConfigStorage";
    aPropValue.Value <<= xStorage;
    aProp[ 0 ] <<= aPropValue;

    aPropValue.Name = "OpenMode";
    aPropValue.Value <<= css::embed::ElementModes::READWRITE;
    aProp[ 1 ] <<= aPropValue;

    m_xImportedImageManager = css::ui::ImageManager::create( xComponentContext );
    m_xImportedImageManager->initialize(aProp);

    ImageInfo mImageInfo;
    uno::Sequence< OUString > names;
    if ( m_xImportedImageManager.is() )
    {
        names = m_xImportedImageManager->getAllImageNames( GetImageType() );
        for ( sal_Int32 n = 0; n < names.getLength(); ++n )
            mImageInfo.insert( ImageInfo::value_type( names[n], false ));
    }
    sal_uInt16 nId = 1;
    ImageInfo::const_iterator pConstIter = mImageInfo.begin();
    uno::Sequence< OUString > name( 1 );
    while ( pConstIter != mImageInfo.end() )
    {
        name[ 0 ] = pConstIter->first;
        uno::Sequence< uno::Reference< graphic::XGraphic> > graphics = m_xImportedImageManager->getImages( GetImageType(), name );
        if ( graphics.getLength() > 0 )
        {
            Image img = Image( graphics[ 0 ] );
            pTbSymbol->InsertItem( nId, img, pConstIter->first );

            graphics[ 0 ]->acquire();

            pTbSymbol->SetItemData(
                nId, static_cast< void * > ( graphics[ 0 ].get() ) );

            ++nId;
        }
        ++pConstIter;
    }

    ImageInfo                 aImageInfo;

    if ( m_xParentImageManager.is() )
    {
        names = m_xParentImageManager->getAllImageNames( GetImageType() );
        for ( sal_Int32 n = 0; n < names.getLength(); ++n )
            aImageInfo.insert( ImageInfo::value_type( names[n], false ));
    }

    names = m_xImageManager->getAllImageNames( GetImageType() );
    for ( sal_Int32 n = 0; n < names.getLength(); ++n )
    {
        ImageInfo::iterator pIter = aImageInfo.find( names[n] );
        if ( pIter != aImageInfo.end() )
            pIter->second = true;
        else
            aImageInfo.insert( ImageInfo::value_type( names[n], true ));
    }

    // large growth factor, expecting many entries
    pConstIter = aImageInfo.begin();
    while ( pConstIter != aImageInfo.end() )
    {
        name[ 0 ] = pConstIter->first;

        uno::Sequence< uno::Reference< graphic::XGraphic> > graphics;
        try
        {
            if ( pConstIter->second )
                graphics = m_xImageManager->getImages( GetImageType(), name );
            else
                graphics = m_xParentImageManager->getImages( GetImageType(), name );
        }
        catch ( uno::Exception& )
        {
            // can't get sequence for this name so it will not be
            // added to the list
        }

        if ( graphics.getLength() > 0 )
        {
            Image img = Image( graphics[ 0 ] );
            if ( !img.GetBitmapEx().IsEmpty() )
            {
                pTbSymbol->InsertItem( nId, img, pConstIter->first );

                uno::Reference< graphic::XGraphic > xGraphic = graphics[ 0 ];

                if ( xGraphic.is() )
                    xGraphic->acquire();

                pTbSymbol->SetItemData(
                    nId, static_cast< void * > ( xGraphic.get() ) );

                ++nId;
            }
        }

        ++pConstIter;
    }

    pBtnDelete->Enable( false );
    pTbSymbol->SetSelectHdl( LINK(this, SvxIconSelectorDialog, SelectHdl) );
    pBtnImport->SetClickHdl( LINK(this, SvxIconSelectorDialog, ImportHdl) );
    pBtnDelete->SetClickHdl( LINK(this, SvxIconSelectorDialog, DeleteHdl) );

    m_nNextId = pTbSymbol->GetItemCount()+1;
}

SvxIconSelectorDialog::~SvxIconSelectorDialog()
{
    disposeOnce();
}

void SvxIconSelectorDialog::dispose()
{
    if (pTbSymbol)
    {
        sal_uInt16 nCount = pTbSymbol->GetItemCount();

        for (sal_uInt16 n = 0; n < nCount; ++n )
        {
            sal_uInt16 nId = pTbSymbol->GetItemId(n);

            uno::XInterface* xi = static_cast< uno::XInterface* >(
                pTbSymbol->GetItemData( nId ) );

            if ( xi != nullptr )
            xi->release();
        }
    }

    pTbSymbol.clear();
    pFtNote.clear();
    pBtnImport.clear();
    pBtnDelete.clear();
    ModalDialog::dispose();
}

uno::Reference< graphic::XGraphic> SvxIconSelectorDialog::GetSelectedIcon()
{
    uno::Reference< graphic::XGraphic > result;

    sal_uInt16 nId;
    for ( sal_uInt16 n = 0; n < pTbSymbol->GetItemCount(); ++n )
    {
        nId = pTbSymbol->GetItemId( n );
        if ( pTbSymbol->IsItemChecked( nId ) )
        {
            result.set( static_cast< graphic::XGraphic* >( pTbSymbol->GetItemData( nId ) ) );
        }
    }

    return result;
}

IMPL_LINK_TYPED( SvxIconSelectorDialog, SelectHdl, ToolBox *, pToolBox, void )
{
    (void)pToolBox;

    sal_uInt16 nCount = pTbSymbol->GetItemCount();

    for (sal_uInt16 n = 0; n < nCount; ++n )
    {
        sal_uInt16 nId = pTbSymbol->GetItemId( n );

        if ( pTbSymbol->IsItemChecked( nId ) )
        {
            pTbSymbol->CheckItem( nId, false );
        }
    }

    sal_uInt16 nId = pTbSymbol->GetCurItemId();
    pTbSymbol->CheckItem( nId );

    OUString aSelImageText = pTbSymbol->GetItemText( nId );
    if ( m_xImportedImageManager->hasImage( GetImageType(), aSelImageText ) )
    {
        pBtnDelete->Enable();
    }
    else
    {
        pBtnDelete->Enable( false );
    }
}

IMPL_LINK_NOARG_TYPED( SvxIconSelectorDialog, ImportHdl, Button *, void)
{
    sfx2::FileDialogHelper aImportDialog(
        css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
        SFXWB_GRAPHIC | SFXWB_MULTISELECTION );

    // disable the link checkbox in the dialog
    uno::Reference< css::ui::dialogs::XFilePickerControlAccess >
        xController( aImportDialog.GetFilePicker(), uno::UNO_QUERY);
    if ( xController.is() )
    {
        xController->enableControl(
            css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK,
            sal_False);
    }

    aImportDialog.SetCurrentFilter(
        "PNG - Portable Network Graphic");

    if ( ERRCODE_NONE == aImportDialog.Execute() )
    {
        uno::Sequence< OUString > paths = aImportDialog.GetMPath();
        ImportGraphics ( paths );
    }
}

IMPL_LINK_NOARG_TYPED( SvxIconSelectorDialog, DeleteHdl, Button *, void )
{
    OUString message = CUI_RES( RID_SVXSTR_DELETE_ICON_CONFIRM );
    if ( ScopedVclPtr<WarningBox>::Create( this, WinBits(WB_OK_CANCEL), message )->Execute() == RET_OK )
    {
        sal_uInt16 nCount = pTbSymbol->GetItemCount();

        for (sal_uInt16 n = 0; n < nCount; ++n )
        {
            sal_uInt16 nId = pTbSymbol->GetItemId( n );

            if ( pTbSymbol->IsItemChecked( nId ) )
            {
                OUString aSelImageText = pTbSymbol->GetItemText( nId );
                uno::Sequence< OUString > URLs { aSelImageText };
                pTbSymbol->RemoveItem( pTbSymbol->GetItemPos( nId ) );
                m_xImportedImageManager->removeImages( GetImageType(), URLs );
                uno::Reference< css::ui::XUIConfigurationPersistence >
                    xConfigPersistence( m_xImportedImageManager, uno::UNO_QUERY );
                if ( xConfigPersistence.is() && xConfigPersistence->isModified() )
                {
                    xConfigPersistence->store();
                }
                break;
            }
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
    sal_uInt16 nCount = pTbSymbol->GetItemCount();
    for (sal_uInt16 n = 0; n < nCount; ++n )
    {
        sal_uInt16 nId = pTbSymbol->GetItemId( n );

        if ( OUString( pTbSymbol->GetItemText( nId ) ) == aURL )
        {
            try
            {
                // replace/insert image with provided URL
                pTbSymbol->RemoveItem( pTbSymbol->GetItemPos( nId ) );
                aMediaProps[0].Value <<= aURL;

                Image aImage( xGraphic );
                if ( bOK && ((aSize.Width != m_nExpectedSize) || (aSize.Height != m_nExpectedSize)) )
                {
                    BitmapEx aBitmap = aImage.GetBitmapEx();
                    BitmapEx aBitmapex = BitmapEx::AutoScaleBitmap(aBitmap, m_nExpectedSize);
                    aImage = Image( aBitmapex);
                }
                pTbSymbol->InsertItem( nId,aImage, aURL, ToolBoxItemBits::NONE, 0 ); //modify

                xGraphic = aImage.GetXGraphic();

                URLs[0] = aURL;
                aImportGraph[ 0 ] = xGraphic;
                m_xImportedImageManager->replaceImages( GetImageType(), URLs, aImportGraph );
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

void SvxIconSelectorDialog::ImportGraphics(
    const uno::Sequence< OUString >& rPaths )
{
    uno::Sequence< OUString > rejected( rPaths.getLength() );
    sal_Int32 rejectedCount = 0;

    sal_uInt16 ret = 0;
    sal_Int32 aIndex;
    OUString aIconName;
    uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
    aMediaProps[0].Name = "URL";
    uno::Reference< css::ui::XUIConfigurationPersistence >
        xConfigPer( m_xImportedImageManager, uno::UNO_QUERY );

    if ( rPaths.getLength() == 1 )
    {
        if ( m_xImportedImageManager->hasImage( GetImageType(), rPaths[0] ) )
        {
            aIndex = rPaths[0].lastIndexOf( '/' );
            aIconName = rPaths[0].copy( aIndex+1 );
            ret = ScopedVclPtr<SvxIconReplacementDialog>::Create( this, aIconName )->ShowDialog();
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
            if ( m_xImportedImageManager->hasImage( GetImageType(), aPath ) )
            {
                aIndex = rPaths[i].lastIndexOf( '/' );
                aIconName = rPaths[i].copy( aIndex+1 );
                ret = ScopedVclPtr<SvxIconReplacementDialog>::Create( this, aIconName, true )->ShowDialog();
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
        OUString message;
        OUString newLine("\n");
        OUString fPath;
        if (rejectedCount > 1)
              fPath = rPaths[0].copy(8) + "/";
        for ( sal_Int32 i = 0; i < rejectedCount; ++i )
        {
            message += fPath + rejected[i];
            message += newLine;
        }

        ScopedVclPtrInstance< SvxIconChangeDialog > aDialog(this, message);
        aDialog->Execute();
    }
}

bool SvxIconSelectorDialog::ImportGraphic( const OUString& aURL )
{
    bool result = false;

    sal_uInt16 nId = m_nNextId;
    ++m_nNextId;

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
                    pTbSymbol->InsertItem( nId, aImage, aURL, ToolBoxItemBits::NONE, 0 );

                    xGraphic = aImage.GetXGraphic();
                    xGraphic->acquire();

                    pTbSymbol->SetItemData(
                        nId, static_cast< void * > ( xGraphic.get() ) );
                    uno::Sequence<OUString> aImportURL { aURL };
                    uno::Sequence< uno::Reference<graphic::XGraphic > > aImportGraph( 1 );
                    aImportGraph[ 0 ] = xGraphic;
                    m_xImportedImageManager->insertImages( GetImageType(), aImportURL, aImportGraph );
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
                    OSL_TRACE("could not create Image from XGraphic");
                }
            }
            else
            {
                OSL_TRACE("could not get query XGraphic");
            }
    }
    catch( uno::Exception& e )
    {
        OSL_TRACE("Caught exception importing XGraphic: %s", PRTSTR(e.Message));
    }
    return result;
}

/*******************************************************************************
*
* The SvxIconReplacementDialog class
*
*******************************************************************************/
SvxIconReplacementDialog::SvxIconReplacementDialog(
    vcl::Window *pWindow, const OUString& aMessage, bool /*bYestoAll*/ )
    :
MessBox( pWindow, WB_DEF_YES, CUI_RES( RID_SVXSTR_REPLACE_ICON_CONFIRM ),  CUI_RES( RID_SVXSTR_REPLACE_ICON_WARNING ) )

{
    SetImage( WarningBox::GetStandardImage() );
    SetMessText( ReplaceIconName( aMessage ) );
    RemoveButton( 1 );
    AddButton( StandardButtonType::Yes, 2);
    AddButton( CUI_RES( RID_SVXSTR_YESTOALL ), 5);
    AddButton( StandardButtonType::No, 3);
    AddButton( StandardButtonType::Cancel, 4);
}

SvxIconReplacementDialog::SvxIconReplacementDialog(
    vcl::Window *pWindow, const OUString& aMessage )
    : MessBox( pWindow, WB_YES_NO_CANCEL, CUI_RES( RID_SVXSTR_REPLACE_ICON_CONFIRM ),  CUI_RES( RID_SVXSTR_REPLACE_ICON_WARNING ) )
{
    SetImage( WarningBox::GetStandardImage() );
    SetMessText( ReplaceIconName( aMessage ));
}

OUString SvxIconReplacementDialog::ReplaceIconName( const OUString& rMessage )
{
    OUString name;
    OUString message = CUI_RES( RID_SVXSTR_REPLACE_ICON_WARNING );
    OUString placeholder("%ICONNAME" );
    sal_Int32 pos = message.indexOf( placeholder );
    if ( pos != -1 )
    {
        name = message.replaceAt(
            pos, placeholder.getLength(), rMessage );
    }
    return name;
}

sal_uInt16 SvxIconReplacementDialog::ShowDialog()
{
    this->Execute();
    return ( this->GetCurButtonId() );
}
/*******************************************************************************
*
* The SvxIconChangeDialog class added for issue83555
*
*******************************************************************************/
SvxIconChangeDialog::SvxIconChangeDialog(
    vcl::Window *pWindow, const OUString& aMessage)
    :ModalDialog(pWindow, "IconChange", "cui/ui/iconchangedialog.ui")
{
    get(pFImageInfo, "infoImage");
    get(pLineEditDescription, "addrTextview");

    Size aSize(LogicToPixel(Size(140, 83), MapMode(MAP_APPFONT)));
    pLineEditDescription->set_width_request(aSize.Width());
    pLineEditDescription->set_height_request(aSize.Height());

    pFImageInfo->SetImage(InfoBox::GetStandardImage());
    pLineEditDescription->SetControlBackground( GetSettings().GetStyleSettings().GetDialogColor() );
    pLineEditDescription->SetText(aMessage);
}

SvxIconChangeDialog::~SvxIconChangeDialog()
{
    disposeOnce();
}

void SvxIconChangeDialog::dispose()
{
    pFImageInfo.clear();
    pLineEditDescription.clear();
    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
