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

#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <tools/urlobj.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandinfoprovider.hxx>

using namespace css;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

#define MAX_MENU_ITEMS  99

namespace {

static const char CMD_CLEAR_LIST[]   = ".uno:ClearRecentFileList";
static const char CMD_OPEN_AS_TEMPLATE[] = ".uno:OpenTemplate";
static const char CMD_OPEN_REMOTE[]  = ".uno:OpenRemote";

class RecentFilesMenuController :  public svt::PopupMenuControllerBase
{
    using svt::PopupMenuControllerBase::disposing;

public:
    RecentFilesMenuController( const uno::Reference< uno::XComponentContext >& xContext,
                               const uno::Sequence< uno::Any >& args );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.framework.RecentFilesMenuController");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.frame.PopupMenuController"};
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

    std::vector< OUString >   m_aRecentFilesItems;
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
    for ( sal_Int32 i = 0; i < args.getLength(); ++i )
    {
        args[i] >>= aPropValue;
        if ( aPropValue.Name == "InToolbar" )
        {
            aPropValue.Value >>= m_bShowToolbarEntries;
            break;
        }
    }
}

// private function
void RecentFilesMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    VCLXPopupMenu* pPopupMenu    = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( rPopupMenu ));
    PopupMenu*     pVCLPopupMenu = nullptr;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    if ( pVCLPopupMenu )
    {
        Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );

        int nPickListMenuItems = std::min<sal_Int32>( aHistoryList.getLength(), MAX_MENU_ITEMS );
        m_aRecentFilesItems.clear();
        if (( nPickListMenuItems > 0 ) && !m_bDisabled )
        {
            for ( int i = 0; i < nPickListMenuItems; i++ )
            {
                Sequence< PropertyValue >& rPickListEntry = aHistoryList[i];
                OUString aURL;

                for ( int j = 0; j < rPickListEntry.getLength(); j++ )
                {
                    if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_URL )
                    {
                        rPickListEntry[j].Value >>= aURL;
                        break;
                    }
                }

                m_aRecentFilesItems.push_back( aURL );
            }
        }

        if ( !m_aRecentFilesItems.empty() )
        {
            const sal_uInt32 nCount = m_aRecentFilesItems.size();
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
                    aMenuShortCut.append( sal_Int32( i + 1 ) );
                    aMenuShortCut.append( ". " );
                }

                OUString aURLString = "vnd.sun.star.popup:RecentFileList?entry=" + OUString::number(i);

                // Abbreviate URL
                OUString   aMenuTitle;
                INetURLObject   aURL( m_aRecentFilesItems[i] );
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

                pVCLPopupMenu->InsertItem( sal_uInt16( i+1 ), aMenuShortCut.makeStringAndClear() );
                pVCLPopupMenu->SetTipHelpText( sal_uInt16( i+1 ), aTipHelpText );
                pVCLPopupMenu->SetItemCommand( sal_uInt16( i+1 ), aURLString );
            }

            pVCLPopupMenu->InsertSeparator();
            // Clear List menu entry
            pVCLPopupMenu->InsertItem( sal_uInt16( nCount + 1 ),
                                       FwkResId(STR_CLEAR_RECENT_FILES) );
            pVCLPopupMenu->SetItemCommand( sal_uInt16( nCount + 1 ),
                                           CMD_CLEAR_LIST );
            pVCLPopupMenu->SetHelpText( sal_uInt16( nCount + 1 ),
                                        FwkResId(STR_CLEAR_RECENT_FILES_HELP) );

            // Open remote menu entry
            if ( m_bShowToolbarEntries )
            {
                pVCLPopupMenu->InsertSeparator();
                pVCLPopupMenu->InsertItem( CMD_OPEN_AS_TEMPLATE, m_xFrame );
                pVCLPopupMenu->InsertItem( CMD_OPEN_REMOTE, m_xFrame );
            }
        }
        else
        {
            if ( m_bShowToolbarEntries )
            {
                pVCLPopupMenu->InsertItem( CMD_OPEN_AS_TEMPLATE, m_xFrame );
                pVCLPopupMenu->InsertItem( CMD_OPEN_REMOTE, m_xFrame );
            }
            else
            {
                // No recent documents => insert "no document" string
                pVCLPopupMenu->InsertItem( 1, FwkResId(STR_NODOCUMENT) );
                // Do not disable it, otherwise the Toolbar controller and MenuButton
                // will display SV_RESID_STRING_NOSELECTIONPOSSIBLE instead of STR_NODOCUMENT
                pVCLPopupMenu->SetItemBits( 1, pVCLPopupMenu->GetItemBits( 1 ) | MenuItemBits::NOSELECT );
            }
        }
    }
}

void RecentFilesMenuController::executeEntry( sal_Int32 nIndex )
{
    if (( nIndex >= 0 ) &&
        ( nIndex < sal::static_int_cast<sal_Int32>( m_aRecentFilesItems.size() )))
    {
        Sequence< PropertyValue > aArgsList(3);
        aArgsList[0].Name = "Referer";
        aArgsList[0].Value <<= OUString( "private:user" );

        // documents in the picklist will never be opened as templates
        aArgsList[1].Name = "AsTemplate";
        aArgsList[1].Value <<= false;

        // Type detection needs to know which app we are opening it from.
        aArgsList[2].Name = "DocumentService";
        aArgsList[2].Value <<= m_aModuleName;

        dispatchCommand( m_aRecentFilesItems[ nIndex ], aArgsList, "_default" );
    }
}

// XEventListener
void SAL_CALL RecentFilesMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL RecentFilesMenuController::statusChanged( const FeatureStateEvent& Event )
{
    osl::MutexGuard aLock( m_aMutex );
    m_bDisabled = !Event.IsEnabled;
}

void SAL_CALL RecentFilesMenuController::itemSelected( const css::awt::MenuEvent& rEvent )
{
    Reference< css::awt::XPopupMenu > xPopupMenu;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu = m_xPopupMenu;
    aLock.clear();

    if ( xPopupMenu.is() )
    {
        const OUString aCommand( xPopupMenu->getCommand( rEvent.MenuId ) );

        if ( aCommand == CMD_CLEAR_LIST )
        {
            SvtHistoryOptions().Clear( ePICKLIST );
            dispatchCommand(
                "vnd.org.libreoffice.recentdocs:ClearRecentFileList",
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
}

void SAL_CALL RecentFilesMenuController::itemActivated( const css::awt::MenuEvent& )
{
    osl::MutexGuard aLock( m_aMutex );
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
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( aURL.Complete.startsWith( m_aBaseURL ) )
        return Reference< XDispatch >( static_cast< OWeakObject* >( this ), UNO_QUERY );
    else
        return Reference< XDispatch >();
}

// XDispatch
void SAL_CALL RecentFilesMenuController::dispatch(
    const URL& aURL,
    const Sequence< PropertyValue >& /*seqProperties*/ )
{
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( aURL.Complete.startsWith( m_aBaseURL ) )
    {
        // Parse URL to retrieve entry argument and its value
        sal_Int32 nQueryPart = aURL.Complete.indexOf( '?', m_aBaseURL.getLength() );
        if ( nQueryPart > 0 )
        {
            const OUString aEntryArgStr( "entry=" );
            sal_Int32 nEntryArg = aURL.Complete.indexOf( aEntryArgStr, nQueryPart );
            sal_Int32 nEntryPos = nEntryArg + aEntryArgStr.getLength();
            if (( nEntryArg > 0 ) && ( nEntryPos < aURL.Complete.getLength() ))
            {
                sal_Int32 nAddArgs = aURL.Complete.indexOf( '&', nEntryPos );
                OUString aEntryArg;

                if ( nAddArgs < 0 )
                    aEntryArg = aURL.Complete.copy( nEntryPos );
                else
                    aEntryArg = aURL.Complete.copy( nEntryPos, nAddArgs-nEntryPos );

                sal_Int32 nEntry = aEntryArg.toInt32();
                executeEntry( nEntry );
            }
        }
    }
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
