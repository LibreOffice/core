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

#include <uielement/recentfilesmenucontroller.hxx>
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>

#include <com/sun/star/util/XStringWidth.hpp>

#include <cppuhelper/implbase1.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

#define MAX_MENU_ITEMS  99

static const char SFX_REFERER_USER[] = "private:user";
static const char CMD_CLEAR_LIST[]   = ".uno:ClearRecentFileList";
static const char CMD_PREFIX[]       = "vnd.sun.star.popup:RecentFileList?entry=";
static const char MENU_SHORTCUT[]     = "~N. ";

namespace framework
{

class RecentFilesStringLength : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XStringWidth >
{
    public:
        RecentFilesStringLength() {}
        virtual ~RecentFilesStringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const OUString& aString )
            throw (::com::sun::star::uno::RuntimeException)
        {
            return aString.getLength();
        }
};

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   RecentFilesMenuController                   ,
                                            OWeakObject                                 ,
                                            SERVICENAME_POPUPMENUCONTROLLER             ,
                                            IMPLEMENTATIONNAME_RECENTFILESMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   RecentFilesMenuController, {} )

RecentFilesMenuController::RecentFilesMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_bDisabled( sal_False )
{
}

RecentFilesMenuController::~RecentFilesMenuController()
{
}

// private function
void RecentFilesMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu* pPopupMenu    = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*     pVCLPopupMenu = 0;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
        Reference< XStringWidth > xStringLength( new RecentFilesStringLength );

        int nPickListMenuItems = ( aHistoryList.getLength() > MAX_MENU_ITEMS ) ? MAX_MENU_ITEMS : aHistoryList.getLength();
        m_aRecentFilesItems.clear();
        if (( nPickListMenuItems > 0 ) && !m_bDisabled )
        {
            for ( int i = 0; i < nPickListMenuItems; i++ )
            {
                Sequence< PropertyValue >& rPickListEntry = aHistoryList[i];
                RecentFile aRecentFile;

                for ( int j = 0; j < rPickListEntry.getLength(); j++ )
                {
                    Any a = rPickListEntry[j].Value;

                    if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_URL )
                        a >>= aRecentFile.aURL;
                    else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_TITLE )
                        a >>= aRecentFile.aTitle;
                }

                m_aRecentFilesItems.push_back( aRecentFile );
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
                        aMenuShortCut.append( MENU_SHORTCUT );
                        aMenuShortCut[ 1 ] = sal_Unicode( i + '1' );
                    }
                }
                else
                {
                    aMenuShortCut.append( sal_Int32( i + 1 ) );
                    aMenuShortCut.append( ". " );
                }

                OUStringBuffer aStrBuffer;
                aStrBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( CMD_PREFIX ) );
                aStrBuffer.append( sal_Int32( i ) );
                OUString  aURLString( aStrBuffer.makeStringAndClear() );

                // Abbreviate URL
                OUString   aMenuTitle;
                INetURLObject   aURL( m_aRecentFilesItems[i].aURL );
                OUString aTipHelpText( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

                if ( aURL.GetProtocol() == INET_PROT_FILE )
                {
                    // Do handle file URL differently: don't show the protocol, just the file name
                    aMenuTitle = aURL.GetLastName(INetURLObject::DECODE_WITH_CHARSET, RTL_TEXTENCODING_UTF8);
                }
                else
                {
                    // In all other URLs show the protocol name before the file name
                    aMenuTitle   = aURL.GetSchemeName(aURL.GetProtocol()) + ": " + aURL.getName();
                }

                aMenuShortCut.append( aMenuTitle );

                pVCLPopupMenu->InsertItem( sal_uInt16( i+1 ), aMenuShortCut.makeStringAndClear() );
                pVCLPopupMenu->SetTipHelpText( sal_uInt16( i+1 ), aTipHelpText );
                pVCLPopupMenu->SetItemCommand( sal_uInt16( i+1 ), aURLString );
            }

            pVCLPopupMenu->InsertSeparator();
            // Clear List menu entry
            pVCLPopupMenu->InsertItem( sal_uInt16( nCount + 1 ),
                                       FWK_RESSTR(STR_CLEAR_RECENT_FILES) );
            pVCLPopupMenu->SetItemCommand( sal_uInt16( nCount + 1 ),
                                           OUString( RTL_CONSTASCII_USTRINGPARAM( CMD_CLEAR_LIST ) ) );
            pVCLPopupMenu->SetHelpText( sal_uInt16( nCount + 1 ),
                                        FWK_RESSTR(STR_CLEAR_RECENT_FILES_HELP) );
        }
        else
        {
            // No recent documents => insert "no document" string
            pVCLPopupMenu->InsertItem( 1, FWK_RESSTR(STR_NODOCUMENT) );
            // Do not disable it, otherwise the Toolbar controller and MenuButton
            // will display SV_RESID_STRING_NOSELECTIONPOSSIBLE instead of STR_NODOCUMENT
            pVCLPopupMenu->SetItemBits( 1, pVCLPopupMenu->GetItemBits( 1 ) | MIB_NOSELECT );
        }
    }
}

void RecentFilesMenuController::executeEntry( sal_Int32 nIndex )
{
    Reference< XDispatch >            xDispatch;
    Reference< XDispatchProvider >    xDispatchProvider;
    css::util::URL                    aTargetURL;
    Sequence< PropertyValue >         aArgsList;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xDispatchProvider = Reference< XDispatchProvider >( m_xFrame, UNO_QUERY );
    aLock.clear();

    if (( nIndex >= 0 ) &&
        ( nIndex < sal::static_int_cast<sal_Int32>( m_aRecentFilesItems.size() )))
    {
        const RecentFile& rRecentFile = m_aRecentFilesItems[ nIndex ];

        aTargetURL.Complete = rRecentFile.aURL;
        m_xURLTransformer->parseStrict( aTargetURL );

        sal_Int32 nSize = 2;
        aArgsList.realloc( nSize );
        aArgsList[0].Name = "Referer";
        aArgsList[0].Value = makeAny( OUString( SFX_REFERER_USER ) );

        // documents in the picklist will never be opened as templates
        aArgsList[1].Name = "AsTemplate";
        aArgsList[1].Value = makeAny( (sal_Bool) sal_False );

        if (!m_aModuleName.isEmpty())
        {
            // Type detection needs to know which app we are opening it from.
            aArgsList.realloc(++nSize);
            aArgsList[nSize-1].Name = "DocumentService";
            aArgsList[nSize-1].Value <<= m_aModuleName;
        }

        xDispatch = xDispatchProvider->queryDispatch( aTargetURL, "_default", 0 );
    }

    if ( xDispatch.is() )
    {
        // Call dispatch asychronously as we can be destroyed while dispatch is
        // executed. VCL is not able to survive this as it wants to call listeners
        // after select!!!
        LoadRecentFile* pLoadRecentFile = new LoadRecentFile;
        pLoadRecentFile->xDispatch  = xDispatch;
        pLoadRecentFile->aTargetURL = aTargetURL;
        pLoadRecentFile->aArgSeq    = aArgsList;

        Application::PostUserEvent( STATIC_LINK(0, RecentFilesMenuController, ExecuteHdl_Impl), pLoadRecentFile );
    }
}

// XEventListener
void SAL_CALL RecentFilesMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL RecentFilesMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );
    m_bDisabled = !Event.IsEnabled;
}

void SAL_CALL RecentFilesMenuController::itemSelected( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu > xPopupMenu;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu = m_xPopupMenu;
    aLock.clear();

    if ( xPopupMenu.is() )
    {
        const OUString aCommand( xPopupMenu->getCommand( rEvent.MenuId ) );
        OSL_TRACE( "RecentFilesMenuController::itemSelected() - Command : %s",
                   OUStringToOString( aCommand, RTL_TEXTENCODING_UTF8 ).getStr() );

        if ( aCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( CMD_CLEAR_LIST ) ) )
            SvtHistoryOptions().Clear( ePICKLIST );
        else
            executeEntry( rEvent.MenuId-1 );
    }
}

void SAL_CALL RecentFilesMenuController::itemActivated( const css::awt::MenuEvent& ) throw (RuntimeException)
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
throw( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( aURL.Complete.indexOf( m_aBaseURL ) == 0 )
        return Reference< XDispatch >( static_cast< OWeakObject* >( this ), UNO_QUERY );
    else
        return Reference< XDispatch >();
}

// XDispatch
void SAL_CALL RecentFilesMenuController::dispatch(
    const URL& aURL,
    const Sequence< PropertyValue >& /*seqProperties*/ )
throw( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( aURL.Complete.indexOf( m_aBaseURL ) == 0 )
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

IMPL_STATIC_LINK_NOINSTANCE( RecentFilesMenuController, ExecuteHdl_Impl, LoadRecentFile*, pLoadRecentFile )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pLoadRecentFile->xDispatch->dispatch( pLoadRecentFile->aTargetURL, pLoadRecentFile->aArgSeq );
    }
    catch ( const Exception& )
    {
    }

    delete pLoadRecentFile;
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
