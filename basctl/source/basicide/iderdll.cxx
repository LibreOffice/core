/*************************************************************************
 *
 *  $RCSfile: iderdll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <ide_pch.hxx>

#pragma hdrstop

#include <svheader.hxx>

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFXGENLINK_HXX //autogen
#include <sfx2/genlink.hxx>
#endif

#pragma hdrstop

#include <svtools/solar.hrc>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>
#include <svx/svxids.hrc>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <basobj.hxx>
#include <bastypes.hxx>

#define ITEMID_SEARCH   0
#include <svx/srchitem.hxx>

IDEResId::IDEResId( USHORT nId ):
    ResId( nId, (*(BasicIDEDLL**)GetAppData(SHL_IDE))->GetResMgr() )
{
}


BasicIDEDLL::BasicIDEDLL()
{
    *(BasicIDEDLL**)GetAppData(SHL_IDE) = this;

    ByteString aResMgrName( "basctl" );
    aResMgrName += ByteString::CreateFromInt32( SOLARUPD );
    pResMgr = ResMgr::CreateResMgr(
        aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );
    pShell = 0;
    pExtraData = 0;
    GetExtraData(); // damit GlobalErrorHdl gesetzt wird.
}

BasicIDEDLL::~BasicIDEDLL()
{
    delete pResMgr;
    delete pExtraData;
    *(BasicIDEDLL**)GetAppData(SHL_IDE) = NULL;
}

BasicIDEData* BasicIDEDLL::GetExtraData()
{
    if ( !pExtraData )
        pExtraData = new BasicIDEData;
     return pExtraData;
}

BasicIDEData::BasicIDEData() : aObjCatPos( INVPOSITION, INVPOSITION )
{
    nBasicDialogCount = 0;
    bChoosingMacro = FALSE;
    bShellInCriticalSection = FALSE;
    pSearchItem = new SvxSearchItem( SID_SEARCH_ITEM );
    StarBASIC::SetGlobalErrorHdl( LINK( this, BasicIDEData, GlobalBasicErrorHdl ) );
    StarBASIC::SetGlobalBreakHdl( LINK( this, BasicIDEData, GlobalBasicBreakHdl ) );

    pAccelerator = 0;
}

BasicIDEData::~BasicIDEData()
{
    // ErrorHdl zuruecksetzen ist zwar sauberer, aber diese Instanz wird
    // sowieso sehr spaet, nach dem letzten Basic, zerstoert.
    // Durch den Aufruf werden dann aber wieder AppDaten erzeugt und nicht
    // mehr zerstoert => MLK's beim Purify
//  StarBASIC::SetGlobalErrorHdl( Link() );
//  StarBASIC::SetGlobalBreakHdl( Link() );
//  StarBASIC::setGlobalStarScriptListener( XEngineListenerRef() );

    delete pSearchItem;
    delete pAccelerator;
}

void BasicIDEData::InitAccelerator()
{
    if ( !pAccelerator )
    {
        pAccelerator = new Accelerator;
        pAccelerator->InsertItem( 1, KeyCode( KEY_F5 ) );
        pAccelerator->InsertItem( 2, KeyCode( KEY_F5, KEY_SHIFT ) );
        pAccelerator->InsertItem( 3, KeyCode( KEY_F5, KEY_MOD2 ) );
        pAccelerator->InsertItem( 4, KeyCode( KEY_F7 ) );
        pAccelerator->InsertItem( 5, KeyCode( KEY_F8 ) );
        pAccelerator->InsertItem( 6, KeyCode( KEY_F8, KEY_SHIFT ) );
        pAccelerator->InsertItem( 7, KeyCode( KEY_F9 ) );
    }
}

SvxSearchItem& BasicIDEData::GetSearchItem() const
{
    return *pSearchItem;
}

void BasicIDEData::SetSearchItem( const SvxSearchItem& rItem )
{
    delete pSearchItem;
    pSearchItem = (SvxSearchItem*)rItem.Clone();
}

IMPL_LINK( BasicIDEData, GlobalBasicErrorHdl, StarBASIC *, pBasic )
{
    BasicIDE::BasicStopped();

    // Waerend der Macroauswahl keine Fehler ausgeben:
    if ( bChoosingMacro )
        return 1;
    if ( bShellInCriticalSection )
        return 2;

    long nRet = 0;
    BasicIDEShell* pShell = 0;
    if ( SFX_APP()->HasFeature( SFX_FEATURE_BASIC_IDE ) )
    {
        BasicManager* pBasicManager = BasicIDE::FindBasicManager( pBasic );
        if ( pBasicManager )
        {
            USHORT nLib = pBasicManager->GetLibId( pBasic );
            if ( !pBasicManager->HasPassword( nLib ) ||
                    pBasicManager->IsPasswordVerified( nLib ) )
            {
                pShell = IDE_DLL()->GetShell();
                if ( !pShell )
                {
                    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
                    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
                    SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
                    if( pDispatcher )
                    {
                        pDispatcher->Execute( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON );
                    }
                    pShell = IDE_DLL()->GetShell();
                }
            }
        }
    }

    if ( pShell )
        nRet = pShell->CallBasicErrorHdl( pBasic );
    else
        BasicIDE::HandleBasicError();

    return nRet;
}


IMPL_LINK( BasicIDEData, GlobalBasicBreakHdl, StarBASIC *, pBasic )
{
    BasicIDEShell* pShell = IDE_DLL()->GetShell();
    if ( pShell )
    {
        BasicManager* pBasicManager = BasicIDE::FindBasicManager( pBasic );
        if ( pBasicManager )
        {
            USHORT nLib = pBasicManager->GetLibId( pBasic );
            // Hier lande ich zweimal, wenn Step into protected Basic
            // => schlecht, wenn Passwortabfrage 2x, ausserdem sieht man in
            // dem PasswordDlg nicht, fuer welche Lib...
            // => An dieser Stelle keine Passwort-Abfrage starten
            if ( !pBasicManager->HasPassword( nLib ) ||
                    pBasicManager->IsPasswordVerified( nLib ) /* ||
                    QueryPassword( pBasicManager, nLib ) */ )
            {
                return pShell->CallBasicBreakHdl( pBasic );
            }
            else
            {
                // Ein Step-Out muesste mich aus den geschuetzten Bereich befoerdern...
                return SbDEBUG_STEPOUT;
            }
        }
    }

    return 0;
}

IMPL_LINK( BasicIDEData, ExecuteMacroEvent, void *, pData )
{
    if ( pData )
    {
        SFX_APP()->EnterBasicCall();
        SbMethod* pMethod = (SbMethod*)pData;

        // Ist es eine StarScript-Methode? Am Parent erkennen
        SbModule* pModule = pMethod->GetModule();
        DBG_ASSERT( pMethod->GetParent()->GetFlags() & SBX_EXTSEARCH, "Kein EXTSEARCH!" );
        BasicIDE::RunMethod( pMethod );
        pMethod->ReleaseRef();  // muss vorher inkrementiert worden sein!
        SFX_APP()->LeaveBasicCall();
    }
    return 0;
}

