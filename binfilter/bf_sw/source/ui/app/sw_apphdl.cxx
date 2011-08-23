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



#define _SVSTDARR_STRINGSDTOR

#ifndef _SFXEVENT_HXX //autogen
#include <bf_sfx2/event.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <bf_svtools/colorcfg.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <bf_svtools/whiter.hxx>
#endif
#ifndef _SFXISETHINT_HXX //autogen
#include <bf_svtools/isethint.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <bf_svtools/ctloptions.hxx>
#endif


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>		  	// Funktion-Ids
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _GLOBALS_H
#include <globals.h>		// globale Konstanten z.B.
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _USRPREF_HXX //autogen
#include <usrpref.hxx>
#endif
#ifndef _PRTOPT_HXX //autogen
#include <prtopt.hxx>
#endif
#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _FONTCFG_HXX //autogen
#include <fontcfg.hxx>
#endif
#ifndef _BARCFG_HXX
#include <barcfg.hxx>
#endif
#ifndef _UINUMS_HXX //autogen
#include <uinums.hxx>
#endif
#ifndef _DBCONFIG_HXX
#include <dbconfig.hxx>
#endif

#include <bf_sfx2/app.hxx>
#include <bf_svx/svxids.hrc>

// #107253#

using namespace ::com::sun::star;

#define C2S(cChar) String::CreateFromAscii(cChar)

#ifndef _CFGID_H
#include <cfgid.h>
#endif

#include <shells.hrc>
#include "bf_so3/staticbaseurl.hxx"

namespace binfilter {

/*M*/ void SwModule::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
/*M*/ {
/*M*/ 	if( rHint.ISA( SfxEventHint ) )
/*M*/ 	{
/*M*/ 		SfxEventHint& rEvHint = (SfxEventHint&) rHint;
/*M*/ 		SwDocShell* pDocSh = PTR_CAST( SwDocShell, rEvHint.GetObjShell() );
/*M*/ 		if( pDocSh )
/*M*/ 		{
/*M*/ 			SwWrtShell* pWrtSh = pDocSh ? pDocSh->GetWrtShell() : 0;
/*M*/ 			switch( rEvHint.GetEventId() )
/*M*/ 			{
/*          MA 07. Mar. 96: UpdateInputFlds() nur noch bei Dokument neu.
                                (Und bei Einfuegen Textbaust.)
                case SFX_EVENT_OPENDOC:
                // dann am aktuellen Dokument die Input-Fedler updaten
                if( pWrtSh )
                    pWrtSh->UpdateInputFlds();
                break;
*/
/*M*/ 			case SFX_EVENT_CREATEDOC:
/*M*/ 				// alle FIX-Date/Time Felder auf akt. setzen
/*M*/ 				if( pWrtSh )
/*M*/ 				{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	pWrtSh->SetFixFields();
/*M*/ 				}
/*M*/ 				break;
/*M*/ 			}
/*M*/ 		}
/*M*/ 	}
/*M*/ 	else if(rHint.ISA(SfxItemSetHint))
/*M*/ 	{
/*M*/ 		if(	SFX_ITEM_SET == ((SfxItemSetHint&)rHint).GetItemSet().GetItemState(SID_ATTR_PATHNAME))
/*M*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ::GetGlossaries()->UpdateGlosPath( sal_False );
/*M*/ 		}
/*M*/
/*M*/ 		if(	SFX_ITEM_SET == ((SfxItemSetHint&)rHint).GetItemSet().
/*M*/ 					GetItemState( SID_ATTR_ADDRESS, sal_False ))
/*M*/ 			bAuthorInitialised = FALSE;
/*M*/ 	}
/*M*/     else if(rHint.ISA(SfxSimpleHint))
/*M*/     {
/*M*/         ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
/*M*/         if(SFX_HINT_COLORS_CHANGED == nHintId ||
/*N*/            SFX_HINT_ACCESSIBILITY_CHANGED == nHintId )
/*M*/         {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/         }
/*N*/         else if( SFX_HINT_CTL_SETTINGS_CHANGED == nHintId )
/*N*/         {
/*N*/             const SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
/*N*/             while( pObjSh )
/*N*/             {
/*N*/                 if( pObjSh->IsA(TYPE(SwDocShell)) )
/*N*/                 {
/*N*/                     const SwDoc* pDoc = ((SwDocShell*)pObjSh)->GetDoc();
/*N*/                     ViewShell* pVSh = 0;
/*N*/                     pDoc->GetEditShell( &pVSh );
/*N*/                     if ( pVSh )
/*N*/                         pVSh->ChgNumberDigits();
/*N*/                 }
/*N*/                 pObjSh = SfxObjectShell::GetNext(*pObjSh);
/*N*/             }
/*N*/         }
/*M*/         else if(SFX_HINT_DEINITIALIZING == nHintId)
/*M*/         {
/*M*/             DELETEZ(pWebUsrPref);
/*M*/             DELETEZ(pUsrPref)   ;
/*M*/             DELETEZ(pModuleConfig);
/*M*/             DELETEZ(pPrtOpt)      ;
/*M*/             DELETEZ(pWebPrtOpt)   ;
/*M*/             DELETEZ(pChapterNumRules);
/*M*/             DELETEZ(pStdFontConfig)     ;
/*M*/             DELETEZ(pAuthorNames)       ;
/*M*/             DELETEZ(pDBConfig);
/*M*/             EndListening(*pColorConfig);
/*M*/             DELETEZ(pColorConfig);
/*N*/             EndListening(*pCTLOptions);
/*N*/             DELETEZ(pCTLOptions);
/*M*/         }
/*M*/ 	}
/*M*/ }

/* -----------------------------20.02.01 12:43--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwDBConfig*	SwModule::GetDBConfig()
/*N*/ {
/*N*/ 	if(!pDBConfig)
/*N*/ 		pDBConfig = new SwDBConfig;
/*N*/ 	return pDBConfig;
/*N*/ }
/* -----------------------------11.04.2002 15:27------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ ColorConfig& SwModule::GetColorConfig()
/*N*/ {
/*N*/     if(!pColorConfig)
/*N*/ 	{
/*N*/         pColorConfig = new ColorConfig;
/*N*/ 	    SwViewOption::ApplyColorConfigValues(*pColorConfig);
/*N*/         StartListening(*pColorConfig);
/*N*/     }
/*N*/     return *pColorConfig;
/*N*/ }

SvtCTLOptions& SwModule::GetCTLOptions()
{
    if(!pCTLOptions)
    {
        pCTLOptions = new SvtCTLOptions;
        StartListening(*pCTLOptions);
    }
    return *pCTLOptions;
}
/*-----------------30.01.97 08.30-------------------

--------------------------------------------------*/
/*N*/ const SwMasterUsrPref *SwModule::GetUsrPref(sal_Bool bWeb) const
/*N*/ {
/*N*/ 	SwModule* pNonConstModule = (SwModule*)this;
/*N*/ 	if(bWeb && !pWebUsrPref)
/*N*/ 	{
/*N*/ 		// im Load der SwMasterUsrPref wird der SpellChecker gebraucht, dort darf
/*N*/ 		// er aber nicht angelegt werden #58256#
/*N*/ 		pNonConstModule->pWebUsrPref = new SwMasterUsrPref(TRUE);
/*N*/ 	}
/*N*/ 	else if(!bWeb && !pUsrPref)
/*N*/ 	{
/*N*/ 		pNonConstModule->pUsrPref = new SwMasterUsrPref(FALSE);
/*N*/ 	}
/*N*/ 	return  bWeb ? pWebUsrPref : pUsrPref;
/*N*/ }


}
