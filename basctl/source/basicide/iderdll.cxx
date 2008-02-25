/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iderdll.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 15:56:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"


#include <ide_pch.hxx>


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


#include <svtools/solar.hrc>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>
#include <svx/svxids.hrc>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <basobj.hxx>
#include <bastypes.hxx>
#include <basdoc.hxx>
#include <basicmod.hxx>
#include <propbrw.hxx>


#define ITEMID_SEARCH   0
#include <svx/srchitem.hxx>

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


static BasicIDEDLL* pBasicIDEDLL = 0;

BasicIDEDLL* BasicIDEDLL::GetDLL()
{
    return pBasicIDEDLL;
}

IDEResId::IDEResId( USHORT nId ):
    ResId( nId, *(*(BasicIDEModule**)GetAppData(SHL_IDE))->GetResMgr() )
{
}

BasicIDEDLL::BasicIDEDLL()
{
    pBasicIDEDLL = this;
    pShell = 0;
    pExtraData = 0;

    GetExtraData(); // damit GlobalErrorHdl gesetzt wird.
}

BasicIDEDLL::~BasicIDEDLL()
{
    delete pExtraData;
    *(BasicIDEDLL**)GetAppData(SHL_IDE) = NULL;
}

void BasicIDEDLL::Init()
{
    if ( pBasicIDEDLL )
        return;

    SfxObjectFactory* pFact = &BasicDocShell::Factory();
    (void)pFact;

    ByteString aResMgrName( "basctl" );
    ResMgr* pMgr = ResMgr::CreateResMgr(
        aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

    BASIC_MOD() = new BasicIDEModule( pMgr, &BasicDocShell::Factory() );

    new BasicIDEDLL;
    SfxModule* pMod = BASIC_MOD();

    SfxObjectFactory& rFactory = BasicDocShell::Factory();
    rFactory.SetDocumentServiceName( String::CreateFromAscii( "com.sun.star.script.BasicIDE" ) );

    BasicDocShell::RegisterInterface( pMod );
    BasicIDEShell::RegisterFactory( SVX_INTERFACE_BASIDE_VIEWSH );
    BasicIDEShell::RegisterInterface( pMod );

    PropBrwMgr::RegisterChildWindow();
}

/*************************************************************************
|*
|* Deinitialisierung
|*
\************************************************************************/
void BasicIDEDLL::Exit()
{
    // the BasicIDEModule must be destroyed
    BasicIDEModule** ppShlPtr = (BasicIDEModule**) GetAppData(SHL_IDE);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
    DELETEZ( pBasicIDEDLL );
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
    //delete pAccelerator;
}

void BasicIDEData::InitAccelerator()
{/*
    if ( !pAccelerator )
    {
        pAccelerator = new Accelerator;
        pAccelerator->InsertItem( 1, KeyCode( KEY_F5 ) );
        pAccelerator->InsertItem( 2, KeyCode( KEY_F5, KEY_SHIFT ) );
        pAccelerator->InsertItem( 4, KeyCode( KEY_F7 ) );
        pAccelerator->InsertItem( 5, KeyCode( KEY_F8 ) );
        pAccelerator->InsertItem( 6, KeyCode( KEY_F8, KEY_SHIFT ) );
        pAccelerator->InsertItem( 7, KeyCode( KEY_F9 ) );
        pAccelerator->InsertItem( 8, KeyCode( KEY_F9, KEY_SHIFT ) );
    }
 */
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

IMPL_LINK( BasicIDEData, GlobalBasicBreakHdl, StarBASIC *, pBasic )
{
    long nRet = 0;
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if ( pIDEShell )
    {
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        if ( pBasMgr )
        {
            // Hier lande ich zweimal, wenn Step into protected Basic
            // => schlecht, wenn Passwortabfrage 2x, ausserdem sieht man in
            // dem PasswordDlg nicht, fuer welche Lib...
            // => An dieser Stelle keine Passwort-Abfrage starten
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            OSL_ENSURE( aDocument.isValid(), "BasicIDEData::GlobalBasicBreakHdl: no document for the basic manager!" );
            if ( aDocument.isValid() )
            {
                ::rtl::OUString aOULibName( pBasic->GetName() );
                Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                    {
                           // Ein Step-Out muesste mich aus den geschuetzten Bereich befoerdern...
                        nRet = SbDEBUG_STEPOUT;
                    }
                    else
                    {
                          nRet = pIDEShell->CallBasicBreakHdl( pBasic );
                    }
                }
            }
        }
    }

    return nRet;
}

IMPL_LINK( BasicIDEData, ExecuteMacroEvent, void *, pData )
{
    if ( pData )
    {
        SFX_APP()->EnterBasicCall();
        SbMethod* pMethod = (SbMethod*)pData;

        // Ist es eine StarScript-Methode? Am Parent erkennen
        DBG_ASSERT( pMethod->GetParent()->GetFlags() & SBX_EXTSEARCH, "Kein EXTSEARCH!" );
        BasicIDE::RunMethod( pMethod );
        pMethod->ReleaseRef();  // muss vorher inkrementiert worden sein!
        SFX_APP()->LeaveBasicCall();
    }
    return 0;
}

