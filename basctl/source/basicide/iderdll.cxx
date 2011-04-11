/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"


#include <ide_pch.hxx>


#include <svheader.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/genlink.hxx>


#include <svl/solar.hrc>
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
#include <svl/srchitem.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


static BasicIDEDLL* pBasicIDEDLL = 0;

BasicIDEDLL* BasicIDEDLL::GetDLL()
{
    return pBasicIDEDLL;
}

IDEResId::IDEResId( sal_uInt16 nId ):
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
    bChoosingMacro = sal_False;
    bShellInCriticalSection = sal_False;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */