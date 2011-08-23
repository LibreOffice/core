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

#ifdef _MSC_VER
#pragma hdrstop
#endif

//------------------------------------------------------------------------

#include <bf_sfx2/app.hxx>
#include <bf_sfx2/objsh.hxx>
#include <bf_sfx2/sfxsids.hrc>

#define SC_PROGRESS_CXX
#include "progress.hxx"
#include "globstr.hrc"
namespace binfilter {



static ScProgress theDummyInterpretProgress;
SfxProgress*	ScProgress::pGlobalProgress = NULL;
ULONG			ScProgress::nGlobalRange = 0;
ULONG			ScProgress::nGlobalPercent = 0;
BOOL			ScProgress::bGlobalNoUserBreak = TRUE;
ScProgress*		ScProgress::pInterpretProgress = &theDummyInterpretProgress;
ScProgress*		ScProgress::pOldInterpretProgress = NULL;
ULONG			ScProgress::nInterpretProgress = 0;
BOOL			ScProgress::bAllowInterpretProgress = TRUE;
ScDocument*		ScProgress::pInterpretDoc;
BOOL			ScProgress::bIdleWasDisabled = FALSE;



/*N*/ ScProgress::ScProgress( SfxObjectShell* pObjSh, const String& rText,
/*N*/ 						ULONG nRange, BOOL bAllDocs, BOOL bWait )
/*N*/ {
/*N*/ 
/*N*/ 	if ( pGlobalProgress || SfxProgress::GetActiveProgress( NULL ) )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if ( lcl_IsHiddenDocument(pObjSh) )
/*N*/ 	}
/*N*/ 	else if ( SFX_APP()->IsDowning() )
/*N*/ 	{
/*N*/ 		//	kommt vor z.B. beim Speichern des Clipboard-Inhalts als OLE beim Beenden
/*N*/ 		//	Dann wuerde ein SfxProgress wild im Speicher rummuellen
/*N*/ 		//!	Soll das so sein ???
/*N*/ 
/*N*/ 		pProgress = NULL;
/*N*/ 	}
/*N*/ 	else if ( pObjSh && ( pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
/*N*/ 						  pObjSh->GetProgress() ) )
/*N*/ 	{
/*N*/ 		//	#62808# no own progress for embedded objects,
/*N*/ 		//	#73633# no second progress if the document already has one
/*N*/ 
/*N*/ 		pProgress = NULL;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pProgress = new SfxProgress( pObjSh, rText, nRange, bAllDocs, bWait );
/*N*/ 		pGlobalProgress = pProgress;
/*N*/ 		nGlobalRange = nRange;
/*N*/ 		nGlobalPercent = 0;
/*N*/ 		bGlobalNoUserBreak = TRUE;
/*N*/ 	}
/*N*/ }


/*N*/ ScProgress::ScProgress() :
/*N*/ 		pProgress( NULL )
/*N*/ {	// DummyInterpret
/*N*/ }


/*N*/ ScProgress::~ScProgress()
/*N*/ {
/*N*/ 	if ( pProgress )
/*N*/ 	{
/*N*/ 		delete pProgress;
/*N*/ 		pGlobalProgress = NULL;
/*N*/ 		nGlobalRange = 0;
/*N*/ 		nGlobalPercent = 0;
/*N*/ 		bGlobalNoUserBreak = TRUE;
/*N*/ 	}
/*N*/ }


// static



// static



// static




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
