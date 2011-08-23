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

#include "swrect.hxx"
#include <hintids.hxx>
#include <bf_svx/paperinf.hxx>

//Statt uiparam.hxx selbst definieren, das spart keys
#ifndef _SVX_DIALOGS_HRC
#include <bf_svx/dialogs.hrc>
#endif
#define ITEMID_FONTLIST			SID_ATTR_CHAR_FONTLIST

#ifndef _SOT_STORINFO_HXX
#include <sot/storinfo.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <bf_svtools/ctrltool.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <bf_svtools/lingucfg.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <bf_sfx2/docfile.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <bf_sfx2/printer.hxx>
#endif
#ifndef _SVX_ASIANCFG_HXX
#include <bf_svx/asiancfg.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <bf_sfx2/request.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <bf_svtools/intitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <bf_svx/adjitem.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <bf_sfx2/docfilt.hxx>
#endif
#ifndef _XTABLE_HXX //autogen
#include <bf_svx/xtable.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#define ITEMID_COLOR_TABLE SID_COLOR_TABLE
#include <bf_svx/drawitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <bf_svx/flstitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <bf_svx/tstpitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <bf_svx/colritem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <bf_svx/hyznitem.hxx>
#endif

#ifndef _OFF_APP_HXX //autogen
#include <bf_offmgr/app.hxx>
#endif

#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSTYLE_HXX
#include <docstyle.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>		// I/O, Hausformat
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _FONTCFG_HXX
#include <fontcfg.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _UNOTXDOC_HXX
#include <unotxdoc.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

// #107253#
#ifndef _SWLINGUCONFIG_HXX
#include <swlinguconfig.hxx>
#endif
namespace binfilter {


using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;
#define C2U(cChar) OUString::createFromAscii(cChar)
/*-----------------21.09.96 15.29-------------------

--------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


/*N*/ sal_Bool SwDocShell::InitNew( SvStorage * pStor )
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::InitNew" );
/*N*/
/*N*/ 	sal_Bool bRet = SfxInPlaceObject::InitNew( pStor );
/*N*/ 	ASSERT( GetMapUnit() == MAP_TWIP, "map unit is not twip!" );
/*N*/ 	sal_Bool bHTMLTemplSet = sal_False;
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		AddLink();		// pDoc / pIo ggf. anlegen
/*N*/
/*N*/         sal_Bool bWeb = ISA( SwWebDocShell );
/*N*/         if ( bWeb )
/*?*/ 			bHTMLTemplSet = SetHTMLTemplate( *GetDoc() );//Styles aus HTML.vor
/*N*/ 		else if( ISA( SwGlobalDocShell ) )
/*?*/ 			GetDoc()->SetGlobalDoc();		// Globaldokument
/*N*/
/*N*/
/*
        //JP 12.07.95: so einfach waere es fuer die neu Mimik
        pDoc->SetDefault( SvxTabStopItem( 1,
                    GetStar Writer App()->GetUsrPref()->GetDefTabDist(),
                    SVX_TAB_ADJUST_DEFAULT,
                    RES_PARATR_TABSTOP));
*/
/*N*/ 		if ( GetCreateMode() ==  SFX_CREATE_MODE_EMBEDDED )
/*N*/ 		{
                SvEmbeddedObject* pObj = this;
                const Size aSz( lA4Width - 2 * lMinBorder, 6 * MM50 );
                SwRect aVis( Point( DOCUMENTBORDER, DOCUMENTBORDER ), aSz );
                pObj->SetVisArea( aVis.SVRect() );
                pDoc->SetBrowseMode( TRUE );
/*N*/ 		}
        // set forbidden characters if necessary
/*N*/         SvxAsianConfig aAsian;
/*N*/         Sequence<Locale> aLocales =  aAsian.GetStartEndCharLocales();
/*N*/         if(aLocales.getLength())
/*N*/         {
/*?*/             DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const Locale* pLocales = aLocales.getConstArray();
/*N*/         }
/*N*/         pDoc->SetKernAsianPunctuation(!aAsian.IsKerningWesternTextOnly());
/*N*/         pDoc->SetCharCompressType((SwCharCompressType)aAsian.GetCharDistanceCompression());
/*N*/         pDoc->SetPrintData(*SW_MOD()->GetPrtOptions(bWeb));

/*N*/         SubInitNew();

        // fuer alle

/*N*/ 		SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();
/*N*/ 		SfxPrinter* pPrt = pDoc->GetPrt();

/*N*/ 		String sEntry;
/*N*/         USHORT aFontWhich[] =
/*N*/         {   RES_CHRATR_FONT,
/*N*/             RES_CHRATR_CJK_FONT,
/*N*/             RES_CHRATR_CTL_FONT
/*N*/         };
/*N*/         USHORT aFontIds[] =
/*N*/         {
/*N*/             FONT_STANDARD,
/*N*/             FONT_STANDARD_CJK,
/*N*/             FONT_STANDARD_CTL
/*N*/         };
/*M*/         USHORT nFontTypes[] =
/*M*/         {
/*M*/             DEFAULTFONT_LATIN_TEXT,
/*M*/             DEFAULTFONT_CJK_TEXT,
/*M*/             DEFAULTFONT_CTL_TEXT
/*M*/         };
/*M*/         USHORT aLangTypes[] =
/*N*/         {
/*N*/             RES_CHRATR_LANGUAGE,
/*N*/             RES_CHRATR_CJK_LANGUAGE,
/*N*/             RES_CHRATR_CTL_LANGUAGE
/*N*/         };
/*N*/
/*N*/         for(USHORT i = 0; i < 3; i++)
/*N*/         {
/*N*/             USHORT nFontWhich = aFontWhich[i];
/*N*/             USHORT nFontId = aFontIds[i];
/*N*/             SvxFontItem* pFontItem = 0;
/*N*/             if(!pStdFont->IsFontDefault(nFontId))
/*N*/             {
/*?*/                 sEntry = pStdFont->GetFontFor(nFontId);
/*?*/                 sal_Bool bDelete = sal_False;
/*?*/                 const SfxFont* pFnt = pPrt ? pPrt->GetFontByName(sEntry): 0;
/*?*/                 if(!pFnt)
/*?*/                 {
/*?*/                     pFnt = new SfxFont( FAMILY_DONTKNOW, sEntry, PITCH_DONTKNOW,
/*?*/                                         ::gsl_getSystemTextEncoding() );
/*?*/                     bDelete = sal_True;
/*?*/                 }
/*?*/                 pFontItem = new SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
/*?*/                                     aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet(), nFontWhich);
/*?*/                 if(bDelete)
/*?*/                 {
/*?*/                     delete (SfxFont*) pFnt;
/*?*/                     bDelete = sal_False;
/*?*/                 }
/*?*/             }
/*N*/             else
/*M*/             {
/*N*/                 const SvxLanguageItem& rLang = (const SvxLanguageItem&)pDoc->GetDefault( aLangTypes[i] );
/*N*/ 				// #107782# OJ use korean language if latin was used
/*N*/ 				LanguageType eLanguage = rLang.GetLanguage();
/*N*/ 				if ( i == 0 )
/*N*/ 				{
/*N*/ 					LanguageType eUiLanguage = Application::GetSettings().GetUILanguage();
/*N*/ 					switch( eUiLanguage )
/*N*/ 					{
/*N*/ 						case LANGUAGE_KOREAN:
/*N*/ 						case LANGUAGE_KOREAN_JOHAB:
/*N*/ 							eLanguage = eUiLanguage;
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 				Font aLangDefFont = OutputDevice::GetDefaultFont(
/*N*/                     nFontTypes[i],
/*N*/                     eLanguage,
/*N*/                     DEFAULTFONT_FLAGS_ONLYONE );
/*N*/                 pFontItem = new SvxFontItem(aLangDefFont.GetFamily(), aLangDefFont.GetName(),
/*N*/                                     aEmptyStr, aLangDefFont.GetPitch(), aLangDefFont.GetCharSet(), nFontWhich);
/*M*/             }
/*N*/             pDoc->SetDefault(*pFontItem);
/*N*/ 			if( !bHTMLTemplSet )
/*N*/ 			{
/*N*/ 				SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
/*N*/ 				pColl->ResetAttr(nFontWhich);
/*N*/ 			}
/*N*/             delete pFontItem;
/*N*/         }
/*M*/         USHORT aFontIdPoolId[] =
/*M*/         {
/*M*/             FONT_OUTLINE,       RES_POOLCOLL_HEADLINE_BASE,
/*M*/             FONT_LIST,          RES_POOLCOLL_NUMBUL_BASE,
/*M*/             FONT_CAPTION,       RES_POOLCOLL_LABEL,
/*M*/             FONT_INDEX,         RES_POOLCOLL_REGISTER_BASE,
/*M*/             FONT_OUTLINE_CJK,   RES_POOLCOLL_HEADLINE_BASE,
/*M*/             FONT_LIST_CJK,      RES_POOLCOLL_NUMBUL_BASE,
/*M*/             FONT_CAPTION_CJK,   RES_POOLCOLL_LABEL,
/*M*/             FONT_INDEX_CJK,     RES_POOLCOLL_REGISTER_BASE,
/*M*/             FONT_OUTLINE_CTL,   RES_POOLCOLL_HEADLINE_BASE,
/*M*/             FONT_LIST_CTL,      RES_POOLCOLL_NUMBUL_BASE,
/*M*/             FONT_CAPTION_CTL,   RES_POOLCOLL_LABEL,
/*M*/             FONT_INDEX_CTL,     RES_POOLCOLL_REGISTER_BASE
/*M*/         };
/*N*/
/*N*/         USHORT nFontWhich = RES_CHRATR_FONT;
/*M*/         for(USHORT nIdx = 0; nIdx < 24; nIdx += 2)
/*M*/         {
/*M*/             if(nIdx == 8)
/*M*/                 nFontWhich = RES_CHRATR_CJK_FONT;
/*M*/             else if(nIdx == 16)
/*M*/                 nFontWhich = RES_CHRATR_CTL_FONT;
/*M*/             if(!pStdFont->IsFontDefault(aFontIdPoolId[nIdx]))
/*M*/             {
/*M*/                 sEntry = pStdFont->GetFontFor(aFontIdPoolId[nIdx]);
/*M*/                 sal_Bool bDelete = sal_False;
/*M*/                 const SfxFont* pFnt = pPrt ? pPrt->GetFontByName(sEntry): 0;
/*M*/                 if(!pFnt)
/*M*/                 {
/*M*/                     pFnt = new SfxFont( FAMILY_DONTKNOW, sEntry, PITCH_DONTKNOW,
/*M*/                                         ::gsl_getSystemTextEncoding() );
/*M*/                     bDelete = sal_True;
/*M*/                 }
/*M*/                 SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
/*M*/ 				if( !bHTMLTemplSet ||
/*M*/ 					SFX_ITEM_SET != pColl->GetAttrSet().GetItemState(
/*M*/ 													nFontWhich, sal_False ) )
/*M*/ 				{
/*M*/ 					pColl->SetAttr(SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
/*M*/ 										aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet(), nFontWhich));
/*M*/ 				}
/*M*/                 if(bDelete)
/*M*/                 {
/*M*/                     delete (SfxFont*) pFnt;
/*M*/                 }
/*M*/             }
/*M*/         }
/*N*/     }
/*N*/
    /* #106748# If the default frame direction of a document is RTL
         the default adjusment is to the right. */
/*N*/      if( !bHTMLTemplSet &&
/*N*/ 		FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) )
/*N*/         pDoc->SetDefault( SvxAdjustItem(SVX_ADJUST_RIGHT) );
/*N*/
/*N*/ 	return bRet;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Ctor mit SfxCreateMode ?????
 --------------------------------------------------------------------*/


/*N*/ SwDocShell::SwDocShell(SfxObjectCreateMode eMode) :
/*N*/ 	pDoc(0),
/*N*/ 	pIo(0),
/*N*/ 	pBasePool(0),
/*N*/ 	pFontList(0),
/*N*/ 	SfxObjectShell ( eMode ),
/*N*/ 	pView( 0 ),
/*N*/     pWrtShell( 0 ),
/*N*/     nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG)
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
/*N*/     Init_Impl();
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Ctor / Dtor
 --------------------------------------------------------------------*/


/*?*/ SwDocShell::SwDocShell( SwDoc *pD, SfxObjectCreateMode eMode ):
/*?*/ 	pDoc(pD),
/*?*/ 	pIo(0),
/*?*/ 	pBasePool(0),
/*?*/ 	pFontList(0),
/*?*/ 	SfxObjectShell ( eMode ),
/*?*/ 	pView( 0 ),
/*?*/     pWrtShell( 0 ),
/*?*/     nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG)
/*?*/ {
/*?*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
/*?*/     Init_Impl();
/*?*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Dtor
 --------------------------------------------------------------------*/


/*N*/  SwDocShell::~SwDocShell()
/*N*/ {
/*N*/ 	RemoveLink();
/*N*/ 	delete pIo;
/*N*/ 	delete pFontList;
/*N*/
/*N*/ 	// wir als BroadCaster werden auch unser eigener Listener
/*N*/ 	// (fuer DocInfo/FileNamen/....)
/*N*/ 	EndListening( *this );
/*N*/ 	SvxColorTableItem* pColItem = (SvxColorTableItem*)GetItem(SID_COLOR_TABLE);
/*N*/ 	// wird nur die DocInfo fuer den Explorer gelesen, ist das Item nicht da
/*N*/ 	if(pColItem)
/*N*/ 	{
/*N*/ 		XColorTable* pTable = pColItem->GetColorTable();
/*N*/ 		// wurde eine neue Table angelegt, muss sie auch geloescht werden.
/*N*/ 		if((void*)pTable  != (void*)(OFF_APP())->GetStdColorTable())
/*?*/ 			delete pTable;
/*N*/ 	}
/*N*/ }
/* -----------------------------10.09.2001 15:59------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void  SwDocShell::Init_Impl()
/*N*/ {
/*N*/ 	SetShell(this);
/*N*/     SetPool(&SW_MOD()->GetPool());
/*N*/ 	SetBaseModel(new SwXTextDocument(this));
/*N*/ 	// wir als BroadCaster werden auch unser eigener Listener
    // (fuer DocInfo/FileNamen/....)
/*N*/ 	StartListening( *this );
    //position of the "Automatic" style filter for the stylist (app.src)
/*N*/     SetAutoStyleFilterIndex(3);

    // set map unit to twip
/*N*/ 	SetMapUnit( MAP_TWIP );
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: AddLink
 --------------------------------------------------------------------*/


/*N*/ void SwDocShell::AddLink()
/*N*/ {
/*N*/ 	if( !pDoc )
/*N*/ 	{
/*N*/ 		SwDocFac aFactory;
/*N*/ 		pDoc = aFactory.GetDoc();
/*N*/ 		pDoc->AddLink();
/*N*/ 		pDoc->SetHTMLMode( ISA(SwWebDocShell) );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		pDoc->AddLink();
/*N*/ 	pDoc->SetDocShell( this );		// am Doc den DocShell-Pointer setzen
/*N*/ 	uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
/*N*/ 	((SwXTextDocument*)xDoc.get())->Reactivate(this);
/*N*/
/*N*/ 	if( !pIo )
/*N*/ 		pIo = new Sw3Io( *pDoc );
/*N*/ 	else
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pIo->SetDoc( *pDoc );
/*N*/
/*N*/ 	SetPool(&pDoc->GetAttrPool());

    // am besten erst wenn eine sdbcx::View erzeugt wird !!!
/*N*/ 	pDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	neue FontList erzeugen Aenderung Drucker
 --------------------------------------------------------------------*/


/*N*/ void SwDocShell::UpdateFontList()
/*N*/ {
/*N*/ 	ASSERT(pDoc, "Kein Doc keine FontList");
/*N*/ 	if( pDoc )
/*N*/ 	{
/*N*/ 		SfxPrinter* pPrt = pDoc->GetPrt();
/*N*/ 		delete pFontList;
/*N*/
/*N*/ 		if( pPrt && pPrt->GetDevFontCount() && !pDoc->IsBrowseMode() )
/*N*/ 			pFontList = new FontList( pPrt );
/*N*/ 		else
/*N*/ 			pFontList = new FontList( Application::GetDefaultDevice() );
/*N*/
/*N*/ 		PutItem( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );
/*N*/ 	}
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: RemoveLink
 --------------------------------------------------------------------*/


/*N*/ void SwDocShell::RemoveLink()
/*N*/ {
/*N*/ 	// Uno-Object abklemmen
/*N*/ 	uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
/*N*/ 	((SwXTextDocument*)xDoc.get())->Invalidate();
/*N*/ 	aFinishedTimer.Stop();
/*N*/ 	if(pDoc)
/*N*/ 	{
/*N*/ 		DELETEZ(pBasePool);
/*N*/ 		sal_Int8 nRefCt = pDoc->RemoveLink();
/*N*/ 		pDoc->SetOle2Link(Link());
/*N*/ 		pDoc->SetDocShell( 0 );
/*N*/ 		if( !nRefCt )
/*?*/ 			delete pDoc;
/*N*/ 		pDoc = 0;       // wir haben das Doc nicht mehr !!
/*N*/ 	}
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Laden, Default-Format
 --------------------------------------------------------------------*/


/*N*/ sal_Bool  SwDocShell::Load(SvStorage* pStor)
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::Load" );
/*N*/ 	sal_Bool bRet = sal_False;
/*N*/ 	sal_Bool bXML = pStor->GetVersion() >= SOFFICE_FILEFORMAT_60;
/*N*/ 	if( SfxInPlaceObject::Load( pStor ))
/*N*/ 	{
/*N*/ 		RTL_LOGFILE_CONTEXT_TRACE( aLog, "after SfxInPlaceObject::Load" );
/*N*/ 		if( pDoc )              // fuer Letzte Version !!
/*?*/ 			RemoveLink();       // das existierende Loslassen
/*N*/
/*N*/ 		AddLink();      // Link setzen und Daten updaten !!
/*N*/
/*N*/ 		// Das Laden
/*N*/ 		// fuer MD
/*N*/ 		if( bXML )
/*N*/ 		{
/*?*/ 			ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*?*/ 			pBasePool = new SwDocStyleSheetPool( *pDoc,
/*?*/ 							SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*?*/             if(GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
/*?*/             {
/*?*/                 SfxMedium* pMedium = GetMedium();
/*?*/                 SFX_ITEMSET_ARG( pMedium->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
/*?*/                 nUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : ::com::sun::star::document::UpdateDocMode::NO_UPDATE;
/*?*/             }
/*?*/
/*N*/ 		}
/*N*/
/*N*/ 		sal_uInt32 nErr = ERR_SWG_READ_ERROR;
/*N*/ 		switch( GetCreateMode() )
/*N*/ 		{
//		case SFX_CREATE_MODE_INTERNAL:
//			nErr = 0;
//			break;

/*N*/ 		case SFX_CREATE_MODE_ORGANIZER:
/*?*/ 			if( bXML )
/*?*/ 			{
/*?*/ 				if( ReadXML )
/*?*/ 				{
/*?*/ 					ReadXML->SetOrganizerMode( TRUE );
/*?*/ 					SwReader aRdr( *pStor, aEmptyStr, pDoc );
/*?*/ 					nErr = aRdr.Read( *ReadXML );
/*?*/ 					ReadXML->SetOrganizerMode( FALSE );
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 nErr = pIo->LoadStyles( pStor );
/*?*/ 			break;

/*N*/ 		case SFX_CREATE_MODE_INTERNAL:
/*N*/ 		case SFX_CREATE_MODE_EMBEDDED:
/*N*/ 			if ( bXML )
/*N*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 			}
            // SfxProgress unterdruecken, wenn man Embedded ist
/*N*/ 			SW_MOD()->SetEmbeddedLoadSave( sal_True );
            // kein break;

/*N*/ 		case SFX_CREATE_MODE_STANDARD:
/*N*/ 		case SFX_CREATE_MODE_PREVIEW:
/*N*/ 			{
/*N*/ 				Reader *pReader = bXML ? ReadXML : ReadSw3;
/*N*/ 				if( pReader )
/*N*/ 				{
/*N*/ 					// die DocInfo vom Doc am DocShell-Medium setzen
/*N*/ 					RTL_LOGFILE_CONTEXT_TRACE( aLog, "before ReadDocInfo" );
/*N*/ 					if( !bXML )
/*N*/ 					{
/*N*/ 						SfxDocumentInfo aInfo;
/*N*/ 						aInfo.Load( pStor );
/*N*/ 						pDoc->DocInfoChgd( aInfo );
/*N*/ 					}
/*N*/ 					SwReader aRdr( *pStor, aEmptyStr, pDoc );
/*N*/ 					RTL_LOGFILE_CONTEXT_TRACE( aLog, "before Read" );
/*N*/ 					nErr = aRdr.Read( *pReader );
/*N*/ 					RTL_LOGFILE_CONTEXT_TRACE( aLog, "after Read" );

                    // If a XML document is loaded, the global doc/web doc
                    // flags have to be set, because they aren't loaded
                    // by this formats.
/*?*/ 					if( ISA( SwWebDocShell ) )
/*?*/ 					{
/*?*/ 						if( !pDoc->IsHTMLMode() )
/*?*/ 							pDoc->SetHTMLMode( TRUE );
/*?*/ 					}
/*N*/ 					if( ISA( SwGlobalDocShell ) )
/*N*/ 					{
/*N*/ 						if( !pDoc->IsGlobalDoc() )
/*?*/ 							pDoc->SetGlobalDoc( TRUE );
/*N*/ 					}
/*N*/ 				}
/*N*/ #ifdef DBG_UTIL
/*N*/ 				else
/*?*/ 					ASSERT( !this, "ohne Sw3Reader geht nichts" );
/*N*/ #endif
/*N*/ 			}
/*N*/ 			break;

/*N*/ #ifdef DBG_UTIL
/*N*/ 		default:
/*?*/ 			ASSERT( !this, "Load: new CreateMode?" );
/*N*/ #endif
/*N*/
/*N*/ 		}
/*N*/
/*N*/ 		if( !bXML )
/*N*/ 		{
/*N*/ 			ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*N*/ 			pBasePool = new SwDocStyleSheetPool( *pDoc,
/*N*/ 							SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*N*/ 		}
/*N*/ 		UpdateFontList();
/*N*/ 		InitDraw();
/*N*/
/*N*/ 		SetError( nErr );
/*N*/ 		bRet = !IsError( nErr );

        // StartFinishedLoading rufen.
/*N*/ 		if( bRet && !pDoc->IsInLoadAsynchron() &&
/*N*/ 			GetCreateMode() == SFX_CREATE_MODE_STANDARD )
/*N*/ 			StartLoadFinishedTimer();

        // SfxProgress unterdruecken, wenn man Embedded ist
/*N*/ 		SW_MOD()->SetEmbeddedLoadSave( sal_False );

/*N*/ 		if( pDoc->IsGlobalDoc() && !pDoc->IsGlblDocSaveLinks() )
/*N*/ 		{
/*N*/ 			// dann entferne alle nicht referenzierte OLE-Objecte
/*N*/ 			SvStorageInfoList aInfoList;
/*N*/ 			pStor->FillInfoList( &aInfoList );
/*N*/
/*N*/ 			// erstmal alle nicht "OLE-Objecte" aus der Liste entfernen
                sal_uInt32 n;
/*N*/ 			for( n = aInfoList.Count(); n; )
/*N*/ 			{
/*N*/ 				const String& rName = aInfoList.GetObject( --n ).GetName();
/*N*/ 				// in ndole.cxx wird dieser PreFix benutzt
/*N*/ 				if( 3 != rName.Match( String::CreateFromAscii("Obj") ))
/*N*/ 					aInfoList.Remove( n );
/*N*/ 			}
/*N*/
/*N*/ 			// dann alle referenzierten Object aus der Liste entfernen
/*N*/ 			SwClientIter aIter( *(SwModify*)pDoc->GetDfltGrfFmtColl() );
/*N*/ 			for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
/*N*/ 					pNd; pNd = (SwCntntNode*)aIter.Next() )
/*N*/ 			{
/*N*/ 				SwOLENode* pOLENd = pNd->GetOLENode();
/*N*/ 				if( pOLENd )
/*N*/ 				{
/*N*/ 					const String& rOLEName = pOLENd->GetOLEObj().GetName();
/*N*/ 					for( n = aInfoList.Count(); n; )
/*N*/ 					{
/*N*/ 						const String& rName = aInfoList.GetObject( --n ).GetName();
/*N*/ 						if( rOLEName == rName )
/*N*/ 						{
/*N*/ 							aInfoList.Remove( n );
/*N*/ 							break;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
            // und jetzt haben wir alle Objecte, die nicht mehr
            // referenziert werden
/*N*/ 			SvPersist* p = this;
/*N*/ 			for( n = aInfoList.Count(); n; )
/*N*/ 			{
/*?*/ 				const String& rName = aInfoList.GetObject( --n ).GetName();
/*?*/ 				SvInfoObjectRef aRef( p->Find( rName ) );
/*?*/ 				if( aRef.Is() )
/*?*/ 					p->Remove( &aRef );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


/*?*/ sal_Bool  SwDocShell::LoadFrom(SvStorage* pStor)
/*?*/ {
/*?*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::LoadFrom" );
/*?*/ 	sal_Bool bRet = sal_False;
/*?*/ 	if( pDoc )
/*?*/ 		RemoveLink();
/*?*/
/*?*/ 	AddLink();		// Link setzen und Daten updaten !!
/*?*/
/*?*/ 	do {		// middle check loop
/*?*/ 		sal_uInt32 nErr = ERR_SWG_READ_ERROR;
/*?*/ 		const String& rNm = pStor->GetName();
/*?*/ 		String aStreamName;
/*?*/ 		sal_Bool bXML = pStor->GetVersion() >= SOFFICE_FILEFORMAT_60;
/*?*/ 		if( bXML )
/*?*/ 			aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("styles.xml"));
/*?*/ 		else
/*?*/ 			aStreamName = SfxStyleSheetBasePool::GetStreamName();
/*?*/ 		if( pStor->IsStream( aStreamName ) )
/*?*/ 		{
/*?*/ 			// Das Laden
/*?*/ 			if( bXML )
/*?*/ 			{
/*?*/ 				ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*?*/ 				pBasePool = new SwDocStyleSheetPool( *pDoc,
/*?*/ 								SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*?*/ 				if( ReadXML )
/*?*/ 				{
/*?*/ 					ReadXML->SetOrganizerMode( TRUE );
/*?*/ 					SwReader aRdr( *pStor, aEmptyStr, pDoc );
/*?*/ 					nErr = aRdr.Read( *ReadXML );
/*?*/ 					ReadXML->SetOrganizerMode( FALSE );
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 nErr = pIo->LoadStyles( pStor );
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			// sollte es sich um eine 2. Vrolage handeln ??
/*?*/ 			if( SvStorage::IsStorageFile( rNm ) )
/*?*/ 				break;
/*?*/
/*?*/ 			const SfxFilter* pFltr = SwIoSystem::GetFileFilter( rNm, aEmptyStr );
/*?*/ 			if( !pFltr || !pFltr->GetUserData().EqualsAscii( FILTER_SWG ))
/*?*/ 				break;
/*?*/
/*?*/ 			SfxMedium aMed( rNm, STREAM_STD_READ, FALSE );
/*?*/ 			if( 0 == ( nErr = aMed.GetInStream()->GetError() ) )
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwWait aWait( *this, sal_True );
/*?*/ 			}
/*?*/ 		}
/*?*/
/*?*/ 		if( !bXML )
/*?*/ 		{
/*?*/ 			ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*?*/ 			pBasePool = new SwDocStyleSheetPool( *pDoc,
/*?*/ 								SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*?*/ 		}
/*?*/
/*?*/ 		SetError( nErr );
/*?*/ 		bRet = !IsError( nErr );
/*?*/
/*?*/ 	} while( sal_False );
/*?*/
/*?*/ 	SfxObjectShell::LoadFrom( pStor );
/*?*/ 	pDoc->ResetModified();
/*?*/ 	return bRet;
/*?*/ }


/*M*/ void SwDocShell::SubInitNew()
/*M*/ {
/*M*/ 	ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*M*/ 	pBasePool = new SwDocStyleSheetPool( *pDoc,
/*M*/ 					SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*M*/ 	UpdateFontList();
/*M*/ 	InitDraw();
/*M*/
/*M*/     pDoc->SetLinkUpdMode( GLOBALSETTING );
/*M*/ 	pDoc->SetFldUpdateFlags( AUTOUPD_GLOBALSETTING );
/*M*/
/*M*/ 	sal_Bool bWeb = ISA(SwWebDocShell);
/*M*/
/*M*/ 	sal_uInt16 nRange[] =	{
/*N*/         RES_PARATR_ADJUST, RES_PARATR_ADJUST,
/*M*/ 							RES_CHRATR_COLOR, RES_CHRATR_COLOR,
/*M*/ 							RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
/*M*/                             RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
/*M*/                             RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
/*M*/ 							0, 0, 0  };
/*M*/ 	if(!bWeb)
/*M*/ 	{
/*M*/         nRange[ (sizeof(nRange)/sizeof(nRange[0])) - 3 ] = RES_PARATR_TABSTOP;
/*M*/         nRange[ (sizeof(nRange)/sizeof(nRange[0])) - 2 ] = RES_PARATR_HYPHENZONE;
/*M*/ 	}
/*M*/ 	SfxItemSet aDfltSet( pDoc->GetAttrPool(), nRange );
/*M*/
/*M*/     //! get lingu options without loading lingu DLL
/*M*/     SvtLinguOptions aLinguOpt;
/*N*/
/*N*/ 	// #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
/*N*/     SwLinguConfig().GetOptions( aLinguOpt );
/*M*/
/*M*/     sal_Int16   nVal = aLinguOpt.nDefaultLanguage,
/*M*/                 eCJK = aLinguOpt.nDefaultLanguage_CJK,
/*M*/                 eCTL = aLinguOpt.nDefaultLanguage_CTL;
/*M*/ 	aDfltSet.Put( SvxLanguageItem( nVal, RES_CHRATR_LANGUAGE ) );
/*M*/     aDfltSet.Put( SvxLanguageItem( eCJK, RES_CHRATR_CJK_LANGUAGE ) );
/*M*/     aDfltSet.Put( SvxLanguageItem( eCTL, RES_CHRATR_CTL_LANGUAGE ) );
/*M*/
/*M*/     if(!bWeb)
/*M*/ 	{
/*M*/ 		SvxHyphenZoneItem aHyp( (SvxHyphenZoneItem&) pDoc->GetDefault(
/*M*/ 														RES_PARATR_HYPHENZONE) );
/*M*/         aHyp.GetMinLead()   = aLinguOpt.nHyphMinLeading;
/*M*/         aHyp.GetMinTrail()  = aLinguOpt.nHyphMinTrailing;
/*M*/
/*M*/ 		aDfltSet.Put( aHyp );
/*M*/
/*M*/ 		sal_uInt16 nNewPos = SW_MOD()->GetUsrPref(FALSE)->GetDefTab();
/*M*/ 		if( nNewPos )
/*M*/ 			aDfltSet.Put( SvxTabStopItem( 1, nNewPos,
/*M*/ 											SVX_TAB_ADJUST_DEFAULT ) );
/*M*/ 	}
/*M*/     aDfltSet.Put( SvxColorItem( Color( COL_AUTO ), RES_CHRATR_COLOR ) );
/*N*/
/*M*/ 	pDoc->SetDefault( aDfltSet );
/*M*/ 	pDoc->ResetModified();
/*M*/ }


}
