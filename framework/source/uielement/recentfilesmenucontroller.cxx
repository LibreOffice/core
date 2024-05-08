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

#include <strings.hrc>
#include <classes/fwkresid.hxx>

#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <tools/urlobj.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/graph.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/string_view.hxx>

#include <officecfg/Office/Common.hxx>

using namespace css;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

#define MAX_MENU_ITEMS  99
#define MAX_MENU_ITEMS_PER_MODULE  5

namespace {

constexpr OUString CMD_CLEAR_LIST = u".uno:ClearRecentFileList"_ustr;
constexpr OUString CMD_OPEN_AS_TEMPLATE = u".uno:OpenTemplate"_ustr;
constexpr OUString CMD_OPEN_REMOTE = u".uno:OpenRemote"_ustr;

class RecentFilesMenuController :  public svt::PopupMenuControllerBase
{
    using svt::PopupMenuControllerBase::disposing;

public:
    RecentFilesMenuController( const uno::Reference< uno::XComponentContext >& xContext,
                               const uno::Sequence< uno::Any >& args );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.framework.RecentFilesMenuController"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {u"com.sun.star.frame.PopupMenuController"_ustr};
    }

    // XStatusListener
    virtual void SAL_CALL statusChanged( const frame::FeatureStateEvent& Event ) override;

    // XMenuListener
    virtual void SAL_CALL itemSelected( const awt::MenuEvent& rEvent ) override;
    virtual void SAL_CALL itemActivated( const awt::MenuEvent& rEvent ) override;

    // XDispatchProvider
    virtual uno::Reference< frame::XDispatch > SAL_CALL queryDispatch( const util::URL& aURL, const OUString& sTarget, sal_Int32 nFlags ) override;

    // XDispatch
    virtual void SAL_CALL dispatch( const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& seqProperties ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

private:
    virtual void impl_setPopupMenu() override;
    void fillPopupMenu( css::uno::Reference< css::awt::XPopupMenu > const & rPopupMenu );
    void executeEntry( sal_Int32 nIndex );

    std::vector<std::pair<OUString, bool>>   m_aRecentFilesItems;
    bool                      m_bDisabled : 1;
    bool                      m_bShowToolbarEntries;
};

RecentFilesMenuController::RecentFilesMenuController( const uno::Reference< uno::XComponentContext >& xContext,
                                                      const uno::Sequence< uno::Any >& args ) :
    svt::PopupMenuControllerBase( xContext ),
    m_bDisabled( false ),
    m_bShowToolbarEntries( false )
{
    css::beans::PropertyValue aPropValue;
    for ( uno::Any const & arg : args )
    {
        arg >>= aPropValue;
        if ( aPropValue.Name == "InToolbar" )
        {
            aPropValue.Value >>= m_bShowToolbarEntries;
            break;
        }
    }
}

void InsertItem(const css::uno::Reference<css::awt::XPopupMenu>& rPopupMenu,
                const OUString& rCommand,
                const css::uno::Reference<css::frame::XFrame>& rFrame)
{
    sal_uInt16 nItemId = rPopupMenu->getItemCount() + 1;

    if (rFrame.is())
    {
        OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(rFrame));
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(rCommand, aModuleName);
        OUString aLabel(vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties));
        OUString aTooltip(vcl::CommandInfoProvider::GetTooltipForCommand(rCommand, aProperties, rFrame));
        css::uno::Reference<css::graphic::XGraphic> xGraphic(vcl::CommandInfoProvider::GetXGraphicForCommand(rCommand, rFrame));

        rPopupMenu->insertItem(nItemId, aLabel, 0, -1);
        rPopupMenu->setItemImage(nItemId, xGraphic, false);
        rPopupMenu->setHelpText(nItemId, aTooltip);
    }
    else
        rPopupMenu->insertItem(nItemId, OUString(), 0, -1);

    rPopupMenu->setCommand(nItemId, rCommand);
}


// private function
void RecentFilesMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );

    std::vector< SvtHistoryOptions::HistoryItem > aHistoryList = SvtHistoryOptions::GetList( EHistoryType::PickList );

    int nPickListMenuItems = std::min<sal_Int32>( aHistoryList.size(), MAX_MENU_ITEMS );
    m_aRecentFilesItems.clear();

    // tdf#56696 - retrieve expert configuration option if the recent document
    // list should show only files that can be handled by the current module
    const bool bShowCurrentModuleOnly
        = officecfg::Office::Common::History::ShowCurrentModuleOnly::get();

    size_t nItemPosModule = 0;
    size_t nItemPosPinned = 0;
    if (( nPickListMenuItems > 0 ) && !m_bDisabled )
    {
        size_t nItemPos = 0;

        // tdf#155699 - create a lambda to insert a recent document item at a specified position
        auto insertHistoryItemAtPos =
            [&](const SvtHistoryOptions::HistoryItem& rPickListEntry, const size_t aInsertPosition)
        {
            m_aRecentFilesItems.insert(m_aRecentFilesItems.begin() + aInsertPosition,
                                       { rPickListEntry.sURL, rPickListEntry.isReadOnly });
            nItemPos++;
        };

        if (m_aModuleName != "com.sun.star.frame.StartModule")
        {
            ::comphelper::MimeConfigurationHelper aConfigHelper(
                comphelper::getProcessComponentContext());

            // Show the first MAX_MENU_ITEMS_PER_MODULE items of the current module
            // on top of the recent document list.
            for (int i = 0; i < nPickListMenuItems; i++)
            {
                const SvtHistoryOptions::HistoryItem& rPickListEntry = aHistoryList[i];

                // tdf#155699 - insert pinned document at the beginning of the list
                if (rPickListEntry.isPinned)
                {
                    insertHistoryItemAtPos(rPickListEntry, nItemPosPinned);
                    nItemPosPinned++;
                    nItemPosModule++;
                }
                // tdf#56696 - insert documents of the current module
                else if ((bShowCurrentModuleOnly
                          || (nItemPosModule - nItemPosPinned) < MAX_MENU_ITEMS_PER_MODULE)
                         && aConfigHelper.GetDocServiceNameFromFilter(rPickListEntry.sFilter)
                                == m_aModuleName)
                {
                    insertHistoryItemAtPos(rPickListEntry, nItemPosModule);
                    nItemPosModule++;
                }
                // Insert all other documents at the end of the list if the expert option is not set
                else if (!bShowCurrentModuleOnly)
                    insertHistoryItemAtPos(rPickListEntry, nItemPos);
            }
        }
        else
        {
            for (int i = 0; i < nPickListMenuItems; i++)
            {
                const SvtHistoryOptions::HistoryItem& rPickListEntry = aHistoryList[i];
                // tdf#155699 - insert pinned document at the beginning of the list
                insertHistoryItemAtPos(rPickListEntry,
                                       rPickListEntry.isPinned ? nItemPosModule++ : nItemPos);
            }
        }
    }

    if ( !m_aRecentFilesItems.empty() )
    {
        const sal_uInt32 nCount = m_aRecentFilesItems.size();
        StyleSettings aIconSettings;
        bool bIsIconsAllowed = aIconSettings.GetUseImagesInMenus();

        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {

            OUStringBuffer aMenuShortCut;
            if ( i <= 9 )
            {
                if ( i == 9 )
                    aMenuShortCut.append( "1~0. " );
                else
                {
                    aMenuShortCut.append( "~N. " );
                    aMenuShortCut[ 1 ] = sal_Unicode( i + '1' );
                }
            }
            else
            {
                aMenuShortCut.append( OUString::number(sal_Int32( i + 1 ) ) + ". " );
            }

            OUString aURLString = "vnd.sun.star.popup:RecentFileList?entry=" + OUString::number(i);

            // Abbreviate URL
            OUString   aMenuTitle;
            INetURLObject const aURL(m_aRecentFilesItems[i].first);
            OUString aTipHelpText( aURL.getFSysPath( FSysStyle::Detect ) );

            if ( aURL.GetProtocol() == INetProtocol::File )
            {
                // Do handle file URL differently: don't show the protocol, just the file name
                aMenuTitle = aURL.GetLastName(INetURLObject::DecodeMechanism::WithCharset);
            }
            else
            {
                // In all other URLs show the protocol name before the file name
                aMenuTitle   = INetURLObject::GetSchemeName(aURL.GetProtocol()) + ": " + aURL.getName();
            }

            aMenuShortCut.append( aMenuTitle );

            rPopupMenu->insertItem(sal_uInt16( i+1 ), aMenuShortCut.makeStringAndClear(), 0, -1);

            if ( bIsIconsAllowed ) {
                // tdf#146219: don't use SvFileInformationManager::GetImageId,
                // which needs to access the URL to detect if it's a directory
                BitmapEx aThumbnail(SvFileInformationManager::GetFileImageId(aURL));
                rPopupMenu->setItemImage(sal_uInt16(i + 1), Graphic(aThumbnail).GetXGraphic(), false);
            }

            rPopupMenu->setTipHelpText(sal_uInt16(i + 1), aTipHelpText);
            rPopupMenu->setCommand(sal_uInt16(i + 1), aURLString);

            // tdf#155699 - show a separator after the pinned recent document items
            if (nItemPosPinned > 0 && i == nItemPosPinned - 1)
                rPopupMenu->insertSeparator(-1);

            // Show a separator after the MAX_MENU_ITEMS_PER_MODULE recent document items
            if (nItemPosModule > 0 && i == nItemPosModule - 1)
                rPopupMenu->insertSeparator(-1);
        }

        rPopupMenu->insertSeparator(-1);
        // Clear List menu entry
        rPopupMenu->insertItem(sal_uInt16(nCount + 1), FwkResId(STR_CLEAR_RECENT_FILES), 0, -1);
        rPopupMenu->setCommand(sal_uInt16(nCount + 1), CMD_CLEAR_LIST);
        rPopupMenu->setHelpText(sal_uInt16(nCount + 1), FwkResId(STR_CLEAR_RECENT_FILES_HELP));

        // Open remote menu entry
        if ( m_bShowToolbarEntries )
        {
            rPopupMenu->insertSeparator(-1);
            InsertItem(rPopupMenu, CMD_OPEN_AS_TEMPLATE, m_xFrame);
            InsertItem(rPopupMenu, CMD_OPEN_REMOTE, m_xFrame);
        }
    }
    else
    {
        if ( m_bShowToolbarEntries )
        {
            InsertItem(rPopupMenu, CMD_OPEN_AS_TEMPLATE, m_xFrame);
            InsertItem(rPopupMenu, CMD_OPEN_REMOTE, m_xFrame);
        }
        else
        {
            // Add InsertSeparator(), otherwise it will display
            // the first item icon of recent files instead of displaying no icon.
            rPopupMenu->insertSeparator(-1);
            // No recent documents => insert "no documents" string
            // Do not disable it, otherwise the Toolbar controller and MenuButton
            // will display SV_RESID_STRING_NOSELECTIONPOSSIBLE instead of STR_NODOCUMENT
            rPopupMenu->insertItem(1, FwkResId(STR_NODOCUMENT), static_cast<sal_Int16>(MenuItemBits::NOSELECT), -1);
        }
    }
}

void RecentFilesMenuController::executeEntry( sal_Int32 nIndex )
{
    if (( nIndex < 0 ) ||
        ( nIndex >= sal::static_int_cast<sal_Int32>( m_aRecentFilesItems.size() )))
        return;

    Sequence< PropertyValue > aArgsList{
        comphelper::makePropertyValue(u"Referer"_ustr, u"private:user"_ustr),

        // documents in the picklist will never be opened as templates
        comphelper::makePropertyValue(u"AsTemplate"_ustr, false),

        // Type detection needs to know which app we are opening it from.
        comphelper::makePropertyValue(u"DocumentService"_ustr, m_aModuleName)
    };
    if (m_aRecentFilesItems[nIndex].second) // tdf#149170 only add if true
    {
        aArgsList.realloc(aArgsList.size()+1);
        aArgsList.getArray()[aArgsList.size()-1] = comphelper::makePropertyValue(u"ReadOnly"_ustr, true);
    }
    dispatchCommand(m_aRecentFilesItems[nIndex].first, aArgsList, u"_default"_ustr);
}

// XEventListener
void SAL_CALL RecentFilesMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL RecentFilesMenuController::statusChanged( const FeatureStateEvent& Event )
{
    std::unique_lock aLock( m_aMutex );
    m_bDisabled = !Event.IsEnabled;
}

void SAL_CALL RecentFilesMenuController::itemSelected( const css::awt::MenuEvent& rEvent )
{
    Reference< css::awt::XPopupMenu > xPopupMenu;

    {
        std::unique_lock aLock(m_aMutex);
        xPopupMenu = m_xPopupMenu;
    }

    if ( !xPopupMenu.is() )
        return;

    const OUString aCommand( xPopupMenu->getCommand( rEvent.MenuId ) );

    if ( aCommand == CMD_CLEAR_LIST )
    {
        SvtHistoryOptions::Clear( EHistoryType::PickList, false );
        dispatchCommand(
            u"vnd.org.libreoffice.recentdocs:ClearRecentFileList"_ustr,
            css::uno::Sequence< css::beans::PropertyValue >() );
    }
    else if ( aCommand == CMD_OPEN_REMOTE )
    {
        Sequence< PropertyValue > aArgsList( 0 );
        dispatchCommand( CMD_OPEN_REMOTE, aArgsList );
    }
    else if ( aCommand == CMD_OPEN_AS_TEMPLATE )
    {
        Sequence< PropertyValue > aArgsList( 0 );
        dispatchCommand( CMD_OPEN_AS_TEMPLATE, aArgsList );
    }
    else
        executeEntry( rEvent.MenuId-1 );
}

void SAL_CALL RecentFilesMenuController::itemActivated( const css::awt::MenuEvent& )
{
    std::unique_lock aLock( m_aMutex );
    impl_setPopupMenu();
}

// XPopupMenuController
void RecentFilesMenuController::impl_setPopupMenu()
{
    if ( m_xPopupMenu.is() )
        fillPopupMenu( m_xPopupMenu );
}

// XDispatchProvider
Reference< XDispatch > SAL_CALL RecentFilesMenuController::queryDispatch(
    const URL& aURL,
    const OUString& /*sTarget*/,
    sal_Int32 /*nFlags*/ )
{
    std::unique_lock aLock( m_aMutex );

    throwIfDisposed(aLock);

    if ( aURL.Complete.startsWith( m_aBaseURL ) )
        return Reference< XDispatch >( this );
    else
        return Reference< XDispatch >();
}

// XDispatch
void SAL_CALL RecentFilesMenuController::dispatch(
    const URL& aURL,
    const Sequence< PropertyValue >& /*seqProperties*/ )
{
    std::unique_lock aLock( m_aMutex );

    throwIfDisposed(aLock);

    if ( !aURL.Complete.startsWith( m_aBaseURL ) )
        return;

    // Parse URL to retrieve entry argument and its value
    sal_Int32 nQueryPart = aURL.Complete.indexOf( '?', m_aBaseURL.getLength() );
    if ( nQueryPart <= 0 )
        return;

    static constexpr OUString aEntryArgStr( u"entry="_ustr );
    sal_Int32 nEntryArg = aURL.Complete.indexOf( aEntryArgStr, nQueryPart );
    sal_Int32 nEntryPos = nEntryArg + aEntryArgStr.getLength();
    if (( nEntryArg <= 0 ) || ( nEntryPos >= aURL.Complete.getLength() ))
        return;

    sal_Int32 nAddArgs = aURL.Complete.indexOf( '&', nEntryPos );
    std::u16string_view aEntryArg;

    if ( nAddArgs < 0 )
        aEntryArg = aURL.Complete.subView( nEntryPos );
    else
        aEntryArg = aURL.Complete.subView( nEntryPos, nAddArgs-nEntryPos );

    sal_Int32 nEntry = o3tl::toInt32(aEntryArg);
    executeEntry( nEntry );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_RecentFilesMenuController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &args)
{
    return cppu::acquire(new RecentFilesMenuController(context, args));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
