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


#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE

#include <hintids.hxx>

#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <bf_svtools/lckbitem.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/macitem.hxx>
#include <bf_svtools/zforlist.hxx>
#include <bf_svtools/zformat.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_sfx2/docinf.hxx>
#include <bf_sfx2/docinf.hxx>
#include <bf_sfx2/request.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_sfx2/evntconf.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/docfilt.hxx>
#include <bf_svx/svxids.hrc>
#include <bf_svx/drawitem.hxx>
#include <bf_svx/langitem.hxx>

#include <bf_offmgr/app.hxx>

#include "bf_basic/sbstar.hxx"
#include "bf_basic/basmgr.hxx"
#include <bf_so3/svstor.hxx>
#include <comphelper/classids.hxx>

#include <swunodef.hxx>

#include <errhdl.hxx>

#include <fmtcol.hxx>
#include <swevent.hxx>

#include <docsh.hxx>	  	// Dokumenterzeugung

#include <horiornt.hxx>

#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <viewopt.hxx>
#include <globdoc.hxx>
#include <docstyle.hxx>
#include <doc.hxx>
#include <pagedesc.hxx>
#include <shellio.hxx>
#include <poolfmt.hxx>
#include <usrpref.hxx>
#include <wdocsh.hxx>
#include <unotxdoc.hxx>
#include <acmplwrd.hxx>
#include <swmodule.hxx>
#include <unoobj.hxx>
#include <swwait.hxx>

#include <cmdid.h>
#include <globals.h>
#include <helpid.h>
#include <app.hrc>
#include <poolfmt.hrc>
#include <globals.hrc>

#include <SwStyleNameMapper.hxx>

#include <sw3io.hxx>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;


/*--------------------------------------------------------------------
    Beschreibung: 	Benachrichtigung bei geaenderter DocInfo
 --------------------------------------------------------------------*/


/*N*/ void SwDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
/*N*/ {
/*N*/ 	if( !pDoc )
/*N*/ 	{
/*N*/ //MA: Kommt bei der OLE-Registration vor!
/*N*/ //		ASSERT( !this, "DocShell ist nicht richtig initialisiert!" );
/*N*/ 		return ;
/*N*/ 	}
/*N*/
/*N*/ 	USHORT nAction = 0;
/*N*/ 	if( rHint.ISA(SfxDocumentInfoHint) )
/*N*/ 		nAction = 1;
/*N*/ 	else if( rHint.ISA(SfxSimpleHint) )
/*N*/ 	{
/*N*/ 		// swithc for more actions
/*N*/ 		switch( ((SfxSimpleHint&) rHint).GetId() )
/*N*/ 		{
/*N*/ 		case SFX_HINT_TITLECHANGED:
/*N*/ 			if( GetMedium() )
/*N*/ 				nAction = 2;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if( nAction )
/*N*/ 	{
/*N*/ 		BOOL bUnlockView;
/*N*/ 		if( pWrtShell )
/*N*/ 		{
/*N*/ 			bUnlockView = !pWrtShell->IsViewLocked();
/*N*/ 			pWrtShell->LockView( TRUE );	//lock visible section
/*N*/ 			pWrtShell->StartAllAction();
/*N*/ 		}
/*N*/ 		switch( nAction )
/*N*/ 		{
/*N*/ 		case 1:
/*N*/ 			pDoc->DocInfoChgd( *((SfxDocumentInfoHint&)rHint).GetObject() );
/*N*/ 			break;
/*N*/
/*N*/ 		case 2:
/*N*/ 			pDoc->GetSysFldType( RES_FILENAMEFLD )->UpdateFlds();
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		if( pWrtShell )
/*N*/ 		{
/*N*/ 			pWrtShell->EndAllAction();
/*N*/ 			if( bUnlockView )
/*N*/ 				pWrtShell->LockView( FALSE );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwDoc::SetInfo( const SfxDocumentInfo& rInfo )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	if( pDocShell )
/*N*/ }

/*N*/ void SwDocShell::FillClass( SvGlobalName * pClassName,
/*N*/ 								   ULONG * pClipFormat,
/*N*/ 								   String * pAppName,
/*N*/ 								   String * pLongUserName,
/*N*/ 								   String * pUserName,
/*N*/ 								   long nVersion ) const
/*N*/ {
/*N*/ 	SfxInPlaceObject::FillClass(pClassName, pClipFormat, pAppName, pLongUserName,
/*N*/ 								pUserName, nVersion);
/*N*/
/*N*/ 	if (nVersion == SOFFICE_FILEFORMAT_31)
/*N*/ 	{
/*N*/ //        *pClassName		= SvGlobalName(0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61,
//									   0x04, 0x02, 0x1C, 0x00, 0x70,0x02);
/*N*/ 		*pClassName		= SvGlobalName( BF_SO3_SW_CLASSID_30 );

/*N*/ 		*pClipFormat	= SOT_FORMATSTR_ID_STARWRITER_30;
/*N*/ 		pAppName->AssignAscii( "Swriter 3.1" );
/*N*/ 		*pLongUserName	= SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE_31);
/*N*/ 	}
/*N*/ 	else if (nVersion == SOFFICE_FILEFORMAT_40)
/*N*/ 	{
//        *pClassName		= SvGlobalName(0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61,
//									   0x04, 0x02, 0x1C, 0x00, 0x70,0x02);
/*N*/ 		*pClassName		= SvGlobalName( BF_SO3_SW_CLASSID_40 );
/*N*/
/*N*/ 		*pClipFormat	= SOT_FORMATSTR_ID_STARWRITER_40;
/*N*/ 		pAppName->AssignAscii( "StarWriter 4.0" );
/*N*/ 		*pLongUserName	= SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE_40);
/*N*/ 	}
/*N*/ 	else if (nVersion == SOFFICE_FILEFORMAT_50)
/*N*/ 	{
/*N*/ 		*pClassName		= SvGlobalName( BF_SO3_SW_CLASSID_50 );
/*N*/ 		*pClipFormat	= SOT_FORMATSTR_ID_STARWRITER_50;
/*N*/ 		*pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE_50);
/*N*/ 	}
/*N*/ 	else if (nVersion == SOFFICE_FILEFORMAT_60)
/*N*/ 	{
/*N*/ 		*pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE);

            // for binfilter, we need the FormatIDs to be set. Not setting them
            // has always been an error (!)
             *pClassName		= SvGlobalName( BF_SO3_SW_CLASSID_60 );
             *pClipFormat	= SOT_FORMATSTR_ID_STARWRITER_60;

/*N*/ 	}
/*N*/ 	*pUserName = SW_RESSTR(STR_HUMAN_SWDOC_NAME);
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ void SwDocShell::SetModified( BOOL bSet )
/*N*/ {
/*N*/ 	SfxInPlaceObject::SetModified( bSet );
/*N*/ 	if( IsEnableSetModified() && !pDoc->IsInCallModified() )
/*N*/ 	{
/*N*/ 		EnableSetModified( FALSE );
/*N*/
/*N*/ 		if( bSet )
/*N*/ 		{
/*N*/ 			BOOL bOld = pDoc->IsModified();
/*N*/ 			pDoc->SetModified();
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pDoc->ResetModified();
/*N*/
/*N*/ 		EnableSetModified( TRUE );
/*N*/ 	}
/*N*/ 	Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
/*N*/ }

/*N*/ULONG SwDocShell::LoadStylesFromFile( const String& rURL,
/*N*/					SwgReaderOption& rOpt, BOOL bUnoCall )
/*N*/{
/*N*/	ULONG nErr = 0;DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	return nErr;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
