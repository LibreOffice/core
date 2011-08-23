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

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/uno/Reference.h>

#include <tools/urlobj.hxx>
#include <bf_svtools/rectitem.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/eitem.hxx>

#include <bf_svtools/stritem.hxx>


#include <bf_svtools/pathoptions.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "docinf.hxx"
#include "appuno.hxx"
#include "objsh.hxx"
#include "app.hxx"
#include "appdata.hxx"
#include "appimp.hxx"
#include "dlgcont.hxx"

#include "bf_basic/basmgr.hxx"

#include "scriptcont.hxx"

#define ITEMID_SEARCH SID_SEARCH_ITEM


#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#include "bf_so3/staticbaseurl.hxx"

namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;

//--------------------------------------------------------------------

namespace
{
    static BasicManager*& lcl_getAppBasicManager()
    {
        static BasicManager* s_pAppBasicManager = NULL;
        return s_pAppBasicManager;
    }
}

//========================================================================

StarBASIC* SfxApplication::GetBasic_Impl() const
{
    BasicManager* pBasMgr = lcl_getAppBasicManager();
    return pBasMgr ? pBasMgr->GetLib(0) : NULL;
}

//=========================================================================

BasicManager* SfxApplication::GetBasicManager()
{
    if ( pAppData_Impl->nBasicCallLevel == 0 )
        // sicherheitshalber
        EnterBasicCall();

    BasicManager*& pBasMgr = lcl_getAppBasicManager();
    if ( !pBasMgr )
    {
        // Directory bestimmen
        SvtPathOptions aPathCFG;
        String aAppBasicDir( aPathCFG.GetBasicPath() );
        if ( !aAppBasicDir.Len() )
            aPathCFG.SetBasicPath( String::CreateFromAscii("$(prog)") );

        // #58293# soffice.new nur im ::com::sun::star::sdbcx::User-Dir suchen => erstes Verzeichnis
        String aAppFirstBasicDir = aAppBasicDir.GetToken(1);

        // Basic erzeugen und laden
        // MT: #47347# AppBasicDir ist jetzt ein PATH!
        INetURLObject aAppBasic( SvtPathOptions().SubstituteVariable( String::CreateFromAscii("$(progurl)") ) );
        aAppBasic.insertName( Application::GetAppName() );

        pBasMgr = new BasicManager( new StarBASIC, &aAppBasicDir );

        // Als Destination das erste Dir im Pfad:
        String aFileName( aAppBasic.getName() );
        aAppBasic = INetURLObject( aAppBasicDir.GetToken(1) );
        DBG_ASSERT( aAppBasic.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
        aAppBasic.insertName( aFileName );
        pBasMgr->SetStorageName( aAppBasic.PathToFileName() );

        // globale Variablen
        StarBASIC *pBas = pBasMgr->GetLib(0);
        sal_Bool bBasicWasModified = pBas->IsModified();

        // Basic container
        SfxScriptLibraryContainer* pBasicCont = new SfxScriptLibraryContainer
            ( DEFINE_CONST_UNICODE( "StarBasic" ), pBasMgr );
        pBasicCont->acquire();	// Hold via UNO
        Reference< XLibraryContainer > xBasicCont = static_cast< XLibraryContainer* >( pBasicCont );
//        pAppData_Impl->pBasicLibContainer = pBasicCont;
        pBasicCont->setBasicManager( pBasMgr );

        // Dialog container
        SfxDialogLibraryContainer* pDialogCont = new SfxDialogLibraryContainer( NULL );
        pDialogCont->acquire();	// Hold via UNO
        Reference< XLibraryContainer > xDialogCont = static_cast< XLibraryContainer* >( pDialogCont );
//        pAppData_Impl->pDialogLibContainer = pDialogCont;

        LibraryContainerInfo* pInfo = new LibraryContainerInfo
            ( xBasicCont, xDialogCont, static_cast< OldBasicPassword* >( pBasicCont ) );
        pBasMgr->SetLibraryContainerInfo( pInfo );

        // Konstanten
//ASDBG		RegisterBasicConstants( "so", aConstants, sizeof(aConstants)/sizeof(SfxConstant) );

        // Durch MakeVariable wird das Basic modifiziert.
        if ( !bBasicWasModified )
            pBas->SetModified( sal_False );
    }

    return pBasMgr;
}

//--------------------------------------------------------------------

/*N*/ StarBASIC* SfxApplication::GetBasic()
/*N*/ {
/*N*/ 	return GetBasicManager()->GetLib(0);
/*N*/ }

//--------------------------------------------------------------------


//--------------------------------------------------------------------

/*N*/ void SfxApplication::EnterBasicCall()
/*N*/ {
/*N*/ 	if ( 1 == ++pAppData_Impl->nBasicCallLevel )
/*N*/ 	{
/*N*/ 		DBG_TRACE( "SfxShellObject: BASIC-on-demand" );
/*N*/ 
/*N*/ 		// das kann l"anger dauern, da Progress nicht geht, wenigstens Sanduhr
/*N*/ //(mba)/task        SfxWaitCursor aWait;
/*N*/ 
/*N*/ 		// zuerst das BASIC laden
/*N*/ 		GetBasic();
/*
        // als erstes SfxShellObject das SbxObject der SfxApplication erzeugen
        SbxObject *pSbx = GetSbxObject();
        DBG_ASSERT( pSbx, "SfxShellObject: can't create SbxObject for SfxApplication" );

        // die SbxObjects aller Module erzeugen
        SfxModuleArr_Impl& rArr = GetModules_Impl();
        for ( sal_uInt16 n = 0; n < rArr.Count(); ++n )
        {
            SfxModule *pMod = rArr.GetObject(n);
            if ( pMod->IsLoaded() )
            {
                pSbx = pMod->GetSbxObject();
                DBG_ASSERT( pSbx, "SfxModule: can't create SbxObject" );
            }
        }

        // die SbxObjects aller Tasks erzeugen
        for ( SfxTask *pTask = SfxTask::GetFirst(); pTask; pTask = SfxTask::GetNext( *pTask ) )
            pTask->GetSbxObject();

        // die SbxObjects aller SfxObjectShells erzeugen (ggf. Frame-los!)
        for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst( NULL, sal_False );
              pObjSh;
              pObjSh = SfxObjectShell::GetNext(*pObjSh, NULL, sal_False) )
        {
            // kein IP-Object oder wenn doch dann initialisiert?
            SvStorageRef aStorage;
            if ( !pObjSh->IsHandsOff() )
                aStorage = pObjSh->GetStorage();
            if ( !pObjSh->GetInPlaceObject() || aStorage.Is() )
            {
                DBG( DbgOutf( "SfxShellObject: BASIC-on-demand for %s",
                              pObjSh->SfxShell::GetName().GetBuffer() ) );
                pSbx = pObjSh->GetSbxObject();
                DBG_ASSERT( pSbx, "SfxShellObject: can't create SbxObject" );
            }
        }

        // die SbxObjects der SfxShells auf den Stacks der Frames erzeugen
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(0,0,sal_False);
              pFrame;
              pFrame = SfxViewFrame::GetNext(*pFrame,0,0,sal_False) )
        {
            // den Dispatcher des Frames rausholen
            SfxDispatcher *pDispat = pFrame->GetDispatcher();
            pDispat->Flush();

            // "uber alle SfxShells auf dem Stack des Dispatchers iterieren
            // Frame selbst wird ausgespart, da er indirekt angezogen wird,
            // sofern er ein Dokument enth"alt.
            for ( sal_uInt16 nStackIdx = pDispat->GetShellLevel(*pFrame);
                  0 != nStackIdx;
                  --nStackIdx )
            {
                DBG( DbgOutf( "SfxShellObject: BASIC-on-demand for level %u", nStackIdx-1 ); )
                pSbx = pDispat->GetShell(nStackIdx - 1)->GetSbxObject();
                DBG_ASSERT( pSbx, "SfxShellObject: can't create SbxObject" );
            }

            if ( !pFrame->GetObjectShell() )
            {
                DBG( DbgOutf( "SfxShellObject: BASIC-on-demand for empty frame" ); )
                pSbx = pFrame->GetSbxObject();
                DBG_ASSERT( pSbx, "SfxShellObject: can't create SbxObject" );
            }
        }
*/
        // Factories anmelden
//        SbxBase::AddFactory( new SfxSbxObjectFactory_Impl );
/*N*/ 	}
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxApplication::LeaveBasicCall()
/*N*/ {
/*N*/ 	--pAppData_Impl->nBasicCallLevel;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
