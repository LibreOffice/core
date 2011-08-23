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

#include <hintids.hxx>

#if !(defined _SVSTDARR_STRINGS_DECL && defined _SVSTDARR_BYTESTRINGS_DECL && \
      defined _SVSTDARR_USHORTS_DECL && defined _SVSTDARR_XUB_STRLEN_DECL && \
      defined _SVSTDARR_BOOLS_DECL)
#define _SVSTDARR_STRINGS
#define _SVSTDARR_BYTESTRINGS
#define _SVSTDARR_USHORTS
#define _SVSTDARR_XUB_STRLEN
#define _SVSTDARR_BOOLS
#endif

#include <bf_svtools/imap.hxx>
#include <bf_svtools/urihelper.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/cscoitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/tstpitem.hxx>

#include <horiornt.hxx>

#include <doc.hxx>

#include <errhdl.hxx>

#include <pam.hxx>
#include <fmtanchr.hxx>
#include <txtftn.hxx>
#include <fmturl.hxx>
#include <fchrfmt.hxx>
#include <fmtftn.hxx>
#include <fmtflcnt.hxx>
#include <fmtfld.hxx>
#include <fmtinfmt.hxx>
#include <txtflcnt.hxx>
#include <charatr.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <paratr.hxx>
#include <poolfmt.hxx>
#include <sw3io.hxx>
#include <sw3imp.hxx>
#include <ndtxt.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <crypter.hxx>
#include <wrong.hxx>
#include <tox.hxx>
#include <fmthbsh.hxx>
// OD 27.06.2003 #108784#

// Export
#include <fldbas.hxx>
#include <frmatr.hxx>

#include <swerror.h>
#include <SwStyleNameMapper.hxx>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

#define URL_DECODE 	\
    , INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS

/*N*/ SV_IMPL_PTRARR_SORT(Sw3SortFmts,SwFmtPtr)

////////////////////////////////////////////////////////////////////////////

// Vorbereitung des Exports eines Text-Nodes in das Sw31 File-Format
// (dazu muessen SwFmtInetFmt-Hints in Felder umgewandelt werden)
// Idee:
// Die Start- und End-Positionen aller Hints werden in zwei USHORT-Arrays
// gespeichert, die SfxPoolItems der Hints in einem dritten. Diese
// "Hint"-Arrays werden entsprechend aufbereitet und dann statt der
// Original-Hints ausgegeben.
//
// Wie wird aufbereitet?
// Die "Hints" werden zunaechst in die Arrays kopiert. dabei werden
// - Schachtelungen von SwFmtINetFmt-Hints aufgeloest.
// - leere SwFmtINetFmt-Hints "entfernt"
// - Hints innerhalb von SwFmtINetFmt-Hints "entfernt"
//
// Danach werden die Texte der SwFmtINetFmt-Hints extrahiert und der
// Text des Nodes sowie die Htnt-Psotionen an die Verwendung von Feldern
// angepasst.

SV_DECL_PTRARR(SfxPoolItems,SfxPoolItem * ,16,16)
struct Sw3ExportTxtAttrs
{
    SvXub_StrLens 	aItemStarts;	// Start-Pos der Hints
    SvXub_StrLens	aItemEnds;		// End-Pos der Hints
    SfxPoolItems 	aItems;			// Items der Hints
    SvByteStrings 	aINetFmtTexts;	// Texte der SwFmtINetFmt-Hints

    ByteString		aText;			// Node-Text

    USHORT			nDrawFrmFmts; 	// Anzahl zeichengeb. Zeichen-Objekte

    Sw3ExportTxtAttrs() : nDrawFrmFmts( 0 ) {}
};

class SwInsHardBlankSoftHyph
{
    SvXub_StrLens 	aItemStarts;	// Start-Pos der Hints
    SfxPoolItems 	aItems;			// Items der Hints
public:
    SwInsHardBlankSoftHyph() {}
    ~SwInsHardBlankSoftHyph();
    void AddItem( xub_StrLen nPos, sal_Unicode c );
    void ChangePos( xub_StrLen nHtEnd, xub_StrLen nOffs );
    void OutAttr( Sw3IoImp& rIo, xub_StrLen nStt, xub_StrLen nEnd );
};


////////////////////////////////////////////////////////////////////////////

// Ausgabe von FlyFrames, die an einem Node kleben

/*N*/ void Sw3IoImp::OutNodeFlyFrames( ULONG nNodeId )
/*N*/ {
/*N*/ 	// FlyFrames duerfen Tabellen enthalten, koennen also Tabelle in Tabelle
/*N*/ 	// simulieren
/*N*/ 	SwTable* pSave = pCurTbl; pCurTbl = NULL;
/*N*/ 	SwFmt* pFly;
/*N*/ 	while( ( pFly = FindFlyFrm( nNodeId ) ) != NULL )
/*N*/ 	{
/*N*/ 		if( !pFly->IsDefault() )
/*N*/ 		{
/*N*/ 			BYTE cType = SWG_FLYFMT;
/*N*/             // OD 27.06.2003 #108784# - do *not* export drawing objects in header/footer
/*N*/             bool bExport = true;
/*N*/             if( RES_DRAWFRMFMT == pFly->Which() )
/*N*/             {
/*N*/ 				cType = SWG_SDRFMT;
/*N*/                 SwFrmFmt* pDrawFrmFmt = static_cast<SwFrmFmt*>(pFly);
/*N*/                 const SwFmtAnchor& rFmtAnchor = pDrawFrmFmt->GetAnchor();
/*N*/                 if ( rFmtAnchor.GetAnchorId() != FLY_PAGE &&
/*N*/                      pDrawFrmFmt->GetDoc()->IsInHeaderFooter( rFmtAnchor.GetCntntAnchor()->nNode ) )
/*N*/                 {
/*N*/                     bExport = false;
/*N*/                 }
/*N*/             }
/*N*/             if ( bExport )
/*N*/             {
/*N*/                 OutFormat( cType, *pFly );
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ 	pCurTbl = pSave;
/*N*/ }

// zeichengebundene Zeichen-Objekte absatzgebunden exportieren
void Sw3IoImp::ExportNodeDrawFrmFmts( const SwTxtNode& rNd, xub_StrLen nStart,
                                     xub_StrLen nEnd, USHORT nCount )
{
   ASSERT( pExportInfo, "Wo sind die Export-Informationen???" );
   if( !pExportInfo || !nCount )
       return;

   pExportInfo->bDrwFrmFmt31 = TRUE;

   USHORT nCntAttr = rNd.HasHints() ? rNd.GetSwpHints().Count() : 0;
   USHORT nExported = 0;
   for( USHORT n = 0; n < nCntAttr && nExported < nCount; n++ )
   {
       const SwTxtAttr* pHt = rNd.GetSwpHints()[ n ];
       BOOL   bHtEnd   = BOOL( pHt->GetEnd() != NULL );
       xub_StrLen nHtStart = *pHt->GetStart();

       if( !bHtEnd && nHtStart >= nStart && nHtStart < nEnd &&
           RES_TXTATR_FLYCNT==pHt->GetAttr().Which() )
       {
           const SwFmtFlyCnt& rFlyCnt = (const SwFmtFlyCnt&)pHt->GetAttr();
           const SwFmt *pFmt = rFlyCnt.GetFrmFmt();
           if( RES_DRAWFRMFMT == pFmt->Which() )
           {
               OutFormat( SWG_SDRFMT, *pFmt );
               nExported++;
           }
       }
   }

   pExportInfo->bDrwFrmFmt31 = FALSE;
}

/*N*/ sal_Char Sw3IoImp::ConvStarSymbolCharToStarBats( sal_Unicode c )
/*N*/ {
/*N*/ 	if( !hBatsFontConv )
/*N*/ 	{
/*N*/ 		hBatsFontConv = CreateFontToSubsFontConverter( sStarSymbol,
/*N*/ 				 FONTTOSUBSFONT_EXPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
/*N*/ 		ASSERT( hBatsFontConv, "Got no symbol font converter" );
/*N*/ 	}
/*N*/ 	if( hBatsFontConv )
/*N*/ 	{
/*N*/ 		c = ConvertFontToSubsFontChar( hBatsFontConv, c );
/*N*/ 	}
/*N*/ 
/*N*/ 	return (sal_Char)c;
/*N*/ }

/*N*/ sal_Unicode Sw3IoImp::ConvStarBatsCharToStarSymbol( sal_Char c )
/*N*/ {
/*N*/ 	sal_Unicode cNew = (sal_Unicode)(sal_uChar)c;
/*N*/ 	if( !hBatsFontConv )
/*N*/ 	{
/*N*/ 		hBatsFontConv = CreateFontToSubsFontConverter( sStarBats,
/*N*/ 				 FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
/*N*/ 		ASSERT( hBatsFontConv, "Got no symbol font converter" );
/*N*/ 	}
/*N*/ 	if( hBatsFontConv )
/*N*/ 	{
/*N*/ 		cNew = ConvertFontToSubsFontChar( hBatsFontConv, (sal_Unicode)(sal_uChar)c + 0xf000 );
/*N*/ 	}
/*N*/ 
/*N*/ 	return cNew;
/*N*/ }

sal_Unicode Sw3IoImp::ConvStarMathCharToStarSymbol( sal_Char c )
{
    sal_Unicode cNew = c;
    if( !hMathFontConv )
    {
        hMathFontConv = CreateFontToSubsFontConverter( sStarMath,
                 FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
        ASSERT( hMathFontConv, "Got no symbol font converter" );
    }
    if( hMathFontConv )
    {
        cNew = ConvertFontToSubsFontChar( hMathFontConv, (sal_Unicode)(sal_uChar)c + 0xf000 );
    }

    return cNew;
}

/*N*/ sal_Bool lcl_sw3io_isStarSymbolFontItem( const SvxFontItem& rFontItem )
/*N*/ {
/*N*/ 	return ( rFontItem.GetFamilyName().EqualsAscii( "StarSymbol", 0, sizeof("StarSymbol")-1 ) ||
/*N*/ 			 rFontItem.GetFamilyName().EqualsAscii( "OpenSymbol", 0, sizeof("OpenSymbol")-1 ) );
/*N*/ }

// Hilfsroutine fuer ConvertText: Suche nach dem naechsten Hint,
// der eine Konversion verbietet. Zur Zeit sind dies Hints, die entweder
// direkt oder indirekt auf einen Font mit CHARSET_SYMBOL hinweisen.
/*N*/ const SvxFontItem *lcl_sw3io_getNextFontHint( const SwpHints* pHints, USHORT& rHint,
/*N*/ 								xub_StrLen& rStart, xub_StrLen& rEnd,
/*N*/ 								sal_Bool& rIsMathOrBatsFontItem,
/*N*/ 								Sw3Fmts *pConvToSymbolFmts,
/*N*/ 							    const SvxFontItem& rStarBatsItem,
/*N*/ 							    const SvxFontItem& rStarMathItem )
/*N*/ {
/*N*/ 	rStart = rEnd = (xub_StrLen)-1;
/*N*/ 	rIsMathOrBatsFontItem = sal_False;
/*N*/ 
/*N*/ 	if( !pHints )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	const SvxFontItem *pFontItem = 0;
/*N*/ 	while( rHint < pHints->Count() )
/*N*/ 	{
/*N*/ 		const SwTxtAttr* pHnt = (*pHints) [rHint++];
/*N*/ 		if( pHnt->Which() == RES_CHRATR_FONT )
/*N*/ 		{
/*N*/ 			rStart = *pHnt->GetStart();
/*N*/ 			rEnd   = *pHnt->GetEnd();
/*N*/ 			pFontItem = &pHnt->GetFont();
/*N*/ 			rIsMathOrBatsFontItem =
/*N*/ 				RTL_TEXTENCODING_SYMBOL == pFontItem->GetCharSet() &&
/*N*/ 				( pFontItem->GetFamilyName().EqualsIgnoreCaseAscii( "StarBats", 0, sizeof("StarBats")-1 ) ||
/*N*/ 				  pFontItem->GetFamilyName().EqualsIgnoreCaseAscii( "StarMath", 0, sizeof("StarMath")-1 ) );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		// Gibt es einen CharFormat-Hint mit einem Symbol-Font?
/*N*/ 		else if( pHnt->Which() == RES_TXTATR_CHARFMT )
/*N*/ 		{
/*N*/ 			SwCharFmt* pFmt = pHnt->GetCharFmt().GetCharFmt();
/*N*/ 			if( pFmt->GetAttrSet().GetItemState( RES_CHRATR_FONT, FALSE )
/*N*/ 				== SFX_ITEM_SET )
/*N*/ 			{
/*N*/ 				rStart = *pHnt->GetStart();
/*N*/ 				rEnd   = *pHnt->GetEnd();
/*N*/ 				pFontItem = &pFmt->GetFont();
/*N*/ 				if( pConvToSymbolFmts &&
/*N*/ 					lcl_sw3io_isStarSymbolFontItem( *pFontItem ) )
/*N*/ 				{
/*?*/ 					BYTE nFlags = pConvToSymbolFmts->GetFlags( pFmt );
/*?*/ 					if( (SW3IO_CONV_FROM_BATS & nFlags) != 0 )
/*?*/ 						pFontItem = &rStarBatsItem;
/*?*/ 					else if( (SW3IO_CONV_FROM_MATH & nFlags) != 0 )
/*?*/ 						pFontItem = &rStarMathItem;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return pFontItem;
/*N*/ }

// Text Node konvertieren
// Wird aufgerufen, wenn sich die Systeme unterscheiden. Der Text wird
// vom einen in den anderen Zeichensatz konvertiert. Nicht konvertierbare
// Zeichen werden farblich unterlegt; Hints mit CHARSET_SYMBOL-Zeichensaetzen
// werden uebersprungen

/*N*/ const SwTxtAttr* lcl_sw3io_hasTxtAttr( const SwpHints *pHints, xub_StrLen nIdx )
/*N*/ {
/*N*/ 	const SwTxtAttr* pRet = 0;
/*N*/ 	if( pHints )
/*N*/ 	{
/*N*/ 		USHORT nHints = pHints->Count();
/*N*/ 		for( USHORT i = 0; i < nHints; i++ )
/*N*/ 		{
/*N*/ 			const SwTxtAttr *pPos = (*pHints)[i];
/*N*/ 			const xub_StrLen nStart = *pPos->GetStart();
/*N*/ 			if( nIdx == nStart && !pPos->GetEnd() )
/*N*/ 			{
/*N*/ 				pRet = pPos;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			if( nStart > nIdx )
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRet;
/*N*/ }

/*N*/ sal_Bool Sw3IoImp::ConvertText( ByteString& rText8, String& rText,
/*N*/ 							xub_StrLen nStart, xub_StrLen nEnd,
/*N*/ 							xub_StrLen nOffset, const SwTxtNode& rNd,
/*N*/ 							rtl_TextEncoding eEnc,
/*N*/ 						   	const SvxFontItem& rFontItem,
/*N*/ 							SwInsHardBlankSoftHyph* pHBSH, BOOL bTo8 )
/*N*/ {
/*N*/ 	sal_Bool bRet = sal_False;
/*N*/ 	const SwpHints *pHints = rNd.GetpSwpHints();
/*N*/ 	if( bTo8 )
/*N*/ 	{
/*N*/ 		sal_Bool bToBats = lcl_sw3io_isStarSymbolFontItem( rFontItem );
/*N*/ 		bRet = bToBats;
/*N*/ 		if( bToBats || RTL_TEXTENCODING_SYMBOL == rFontItem.GetCharSet() )
/*N*/ 		{
/*N*/ 			for( xub_StrLen nPos = nStart; nPos < nEnd; nPos++ )
/*N*/ 			{
/*N*/ 				sal_Unicode c = rText.GetChar( nPos );
/*N*/ 				switch ( c )
/*N*/ 				{
/*N*/ 				case CHAR_HARDBLANK:
/*N*/ 				case CHAR_HARDHYPHEN:
/*?*/ 				case CHAR_SOFTHYPHEN:
/*?*/ 					if( pHBSH )
/*?*/ 					{
/*?*/ 						pHBSH->AddItem( nPos, c );
/*?*/ 						c = '\xff';
/*?*/ 					}
/*?*/ 					break;
/*N*/ 
/*N*/ 				case CH_TXTATR_BREAKWORD:
/*N*/ 				case CH_TXTATR_INWORD:
/*N*/ 					if( lcl_sw3io_hasTxtAttr( pHints, nPos+nOffset ) )
/*N*/ 						c = '\xff';
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				if( bToBats )
/*N*/ 					rText8 += ConvStarSymbolCharToStarBats( c );
/*N*/ 				else
/*N*/ 					rText8 += (sal_Char)c;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			xub_StrLen nCopy = nStart;
/*N*/ 			for( xub_StrLen nPos = nStart; nPos < nEnd; nPos++ )
/*N*/ 			{
/*N*/ 				sal_Unicode c = rText.GetChar( nPos );
/*N*/ 				BOOL bToFF = FALSE;
/*N*/ 				switch ( c )
/*N*/ 				{
/*N*/ 				case CHAR_HARDBLANK:
/*N*/ 				case CHAR_HARDHYPHEN:
/*N*/ 				case CHAR_SOFTHYPHEN:
/*N*/ 					if( pHBSH )
/*N*/ 					{
/*N*/ 						pHBSH->AddItem( nPos, c );
/*N*/ 						bToFF = TRUE;
/*N*/ 					}
/*N*/ 					break;
/*N*/ 
/*N*/ 				case CH_TXTATR_BREAKWORD:
/*N*/ 				case CH_TXTATR_INWORD:
/*N*/ 					bToFF = 0 != lcl_sw3io_hasTxtAttr( pHints, nPos+nOffset );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				if( bToFF )
/*N*/ 				{
/*N*/ 					if( nCopy < nPos )
/*N*/ 						rText8 += ByteString( rText.Copy(nCopy,nPos-nCopy),
/*N*/ 										  	eEnc );
/*N*/ 					rText8 += '\xff';
/*N*/ 					nCopy = nPos + 1;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if( nCopy < nEnd )
/*N*/ 				rText8 += ByteString( rText.Copy(nCopy,nEnd-nCopy), eEnc );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const SwTxtAttr* pTAttr;
/*N*/ 		if( RTL_TEXTENCODING_SYMBOL == rFontItem.GetCharSet() )
/*N*/ 		{
/*N*/ 			sal_Bool bBatsToSymbol =
/*N*/ 				rFontItem.GetFamilyName().EqualsIgnoreCaseAscii( sStarBats );
/*N*/ 			sal_Bool bMathToSymbol =
/*N*/ 				rFontItem.GetFamilyName().EqualsIgnoreCaseAscii( sStarMath );
/*N*/ 			bRet = bBatsToSymbol || bMathToSymbol;
/*N*/ 			for( xub_StrLen nPos = nStart; nPos < nEnd; nPos++ )
/*N*/ 			{
/*N*/ 				sal_Char c = rText8.GetChar( nPos );
/*N*/ 				if( '\xff' == c && 0 != (pTAttr =
/*N*/ 						lcl_sw3io_hasTxtAttr( pHints, nPos+nOffset )) )
/*N*/ 					rText += GetCharOfTxtAttr( *pTAttr );
/*N*/ 				else if( bBatsToSymbol )
/*N*/ 					rText += ConvStarBatsCharToStarSymbol( c );
/*N*/ 				else if( bMathToSymbol )
                        rText += ConvStarMathCharToStarSymbol( c );
/*N*/ 				else
/*N*/ 					rText += ByteString::ConvertToUnicode( c,
/*N*/ 										RTL_TEXTENCODING_SYMBOL );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			xub_StrLen nCopy = nStart;
/*N*/ 			for( xub_StrLen nPos = nStart; nPos < nEnd; nPos++ )
/*N*/ 			{
/*N*/ 				sal_Char c = rText8.GetChar( nPos );
/*N*/ 				sal_Unicode cNew;
/*N*/ 				if( '\xff' == c )
/*N*/ 				{
/*N*/ 					if( 0 != ( pTAttr = lcl_sw3io_hasTxtAttr(
/*N*/ 												pHints, nPos+nOffset )) ||
/*N*/ 						CHAR_HARDBLANK ==
/*N*/ 							( cNew = rNd.GetTxt().GetChar( nPos+nOffset )) ||
/*N*/ 					  	CHAR_HARDHYPHEN == cNew || CHAR_SOFTHYPHEN == cNew )
/*N*/ 					{
/*N*/ 						if( nCopy < nPos )
/*N*/ 							rText += String( rText8.Copy(nCopy,nPos-nCopy), eEnc );
/*N*/ 						if( pTAttr )
/*N*/ 							rText += GetCharOfTxtAttr( *pTAttr );
/*N*/ 						else
/*N*/ 							rText += cNew;
/*N*/ 						nCopy = nPos + 1;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if( CHAR_SOFTHYPHEN ==rNd.GetTxt().GetChar( nPos+nOffset ) )
/*N*/ 				{
/*N*/ 					// The original charcter has been converted into a soft 
/*N*/ 					// hyphen, but there was no text attribute at this position.
/*N*/ 					// We then have to replace the soft hyphen with a hard one.
/*N*/ 					// The check is based on the source char set and not on
/*N*/ 					// the actual one. The assumption is here that there is
/*N*/ 					// no difference in the position of the soft hyphen. 
/*N*/ 					// However, to not accidentially do a wrong conversion
/*N*/ 					// we check this again. The only mistake we might make
/*N*/ 					// if the assumption is wrong is to not convert a soft
/*N*/ 					// hyphen.
/*N*/ 					if( eEnc == eSrcSet ||
/*N*/ 						CHAR_SOFTHYPHEN == ByteString::ConvertToUnicode( c, eEnc ) )
/*N*/ 					{
/*N*/ 						if( nCopy < nPos )
/*N*/ 							rText += String( rText8.Copy(nCopy,nPos-nCopy), eEnc );
/*N*/ 						rText += '-';
/*N*/ 						nCopy = nPos + 1;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if( nCopy < nEnd )
/*N*/ 				rText += String( rText8.Copy( nCopy, nEnd-nCopy ), eEnc );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

typedef const SvxFontItem *SvxFontItemPtr;
SV_DECL_PTRARR( SvxFontItems, SvxFontItemPtr, 5, 5 )//STRIP008 ;

typedef SwTxtAttr *SwTxtAttrPtr;
SV_DECL_PTRARR( SwTxtAttrs, SwTxtAttrPtr, 5, 5 )//STRIP008 ;

/*N*/ void Sw3IoImp::ConvertText( ByteString& rText8, String& rText,
/*N*/ 							xub_StrLen nOffset, SwTxtNode& rNd,
/*N*/ 							rtl_TextEncoding eEnc, const SvxFontItem& rFontItem,
/*N*/ 							SwInsHardBlankSoftHyph* pHBSH, BOOL bTo8 )
/*N*/ 
/*N*/ {
/*N*/ 	SvxFontItems aFontItemStack;
/*N*/ 	SvXub_StrLens aEndPosStack;
/*N*/ 	SwTxtAttrs aDeleteFontTxtAttrs;
/*N*/ 	SvXub_StrLens aInsertSymbolFontStartPoss;
/*N*/ 	SvXub_StrLens aInsertSymbolFontEndPoss;
/*N*/ 
/*N*/ 	SvxFontItem aStarBatsItem( FAMILY_DONTKNOW, sStarBats, aEmptyStr,
/*N*/ 							   PITCH_DONTKNOW, RTL_TEXTENCODING_SYMBOL );
/*N*/ 	SvxFontItem aStarMathItem( FAMILY_DONTKNOW, sStarMath, aEmptyStr,
/*N*/ 							   PITCH_DONTKNOW, RTL_TEXTENCODING_SYMBOL );
/*N*/ 
/*N*/ 	SwpHints *pHints = rNd.GetpSwpHints();
/*N*/ 	// find next
/*N*/ 	xub_StrLen nFntStart = (xub_StrLen)-1, nFntEnd = 0;
/*N*/ 	USHORT nHint = 0;
/*N*/ 	const SvxFontItem *pFontItem = &rFontItem;
/*N*/ 	sal_Bool bIsBatsOrMathFontItem;
/*N*/ 	const SvxFontItem *pNewFontItem =
/*N*/ 			lcl_sw3io_getNextFontHint( pHints, nHint, nFntStart,
/*N*/ 									   nFntEnd, bIsBatsOrMathFontItem,
/*N*/ 									   pConvToSymbolFmts, aStarBatsItem,
/*N*/ 									   aStarMathItem );
/*N*/ 	if( !bTo8 && pNewFontItem && bIsBatsOrMathFontItem )
/*N*/ 		aDeleteFontTxtAttrs.Insert( pHints->GetHt( nHint-1), aDeleteFontTxtAttrs.Count() );
/*N*/ 	xub_StrLen nLen = nOffset + (bTo8 ? rText.Len() : rText8.Len() );
/*N*/ 	xub_StrLen nCopy = nOffset;
        xub_StrLen nPos=0;
/*N*/ 	for( nPos = 0; nPos < nLen; nPos++ )
/*N*/ 	{
/*N*/ 		if( aEndPosStack.Count() &&
/*N*/ 			nPos == aEndPosStack[aEndPosStack.Count()-1] )
/*N*/ 		{
/*N*/ 			if( nPos > nCopy )
/*N*/ 			{
/*N*/ 				sal_Bool bSymConv = ConvertText( rText8, rText, nCopy, nPos,
/*N*/ 												 nOffset, rNd, eEnc,
/*N*/ 												 *pFontItem, pHBSH, bTo8 );
/*N*/ 				if( bSymConv && !bTo8 )
/*N*/ 				{
/*N*/ 					aInsertSymbolFontStartPoss.Insert( nCopy,
/*N*/ 							aInsertSymbolFontStartPoss.Count() );
/*N*/ 					aInsertSymbolFontEndPoss.Insert( nPos,
/*N*/ 							aInsertSymbolFontEndPoss.Count() );
/*N*/ 				}
/*N*/ 				nCopy = nPos;
/*N*/ 			}
/*N*/ 			pFontItem = aFontItemStack[ aFontItemStack.Count()-1 ];
/*N*/ 			aFontItemStack.Remove( aFontItemStack.Count()-1 );
/*N*/ 			aEndPosStack.Remove( aEndPosStack.Count()-1 );
/*N*/ 		}
/*N*/ 		while( (xub_StrLen)-1 != nFntStart && nPos == nFntStart )
/*N*/ 		{
/*N*/ 			if( nPos > nCopy )
/*N*/ 			{
/*N*/ 				sal_Bool bSymConv = ConvertText( rText8, rText, nCopy, nPos,
/*N*/ 												 nOffset, rNd,eEnc, *pFontItem,
/*N*/ 												 pHBSH, bTo8 );
/*N*/ 				if( bSymConv && !bTo8 )
/*N*/ 				{
/*?*/ 					aInsertSymbolFontStartPoss.Insert( nCopy,
/*?*/ 							aInsertSymbolFontStartPoss.Count() );
/*?*/ 					aInsertSymbolFontEndPoss.Insert( nPos,
/*?*/ 							aInsertSymbolFontEndPoss.Count() );
/*N*/ 				}
/*N*/ 				nCopy = nPos;
/*N*/ 			}
/*N*/ 			aEndPosStack.Insert( nFntEnd, aEndPosStack.Count() );
/*N*/ 			aFontItemStack.Insert( pFontItem, aFontItemStack.Count() );
/*N*/ 			pFontItem = pNewFontItem;
/*N*/ 			pNewFontItem = lcl_sw3io_getNextFontHint( pHints, nHint, nFntStart,
/*N*/ 									 nFntEnd, bIsBatsOrMathFontItem,
/*N*/ 									 pConvToSymbolFmts, aStarBatsItem,
/*N*/ 									 aStarMathItem);
/*N*/ 			if( !bTo8 && pNewFontItem && bIsBatsOrMathFontItem )
/*N*/ 				aDeleteFontTxtAttrs.Insert( pHints->GetHt( nHint-1 ), aDeleteFontTxtAttrs.Count() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( nLen > nCopy )
/*N*/ 	{
/*N*/ 		sal_Bool bSymConv = ConvertText( rText8, rText, nCopy, nLen, nOffset,
/*N*/ 							   rNd, eEnc, *pFontItem, pHBSH, bTo8 );
/*N*/ 		if( bSymConv && !bTo8 )
/*N*/ 		{
/*N*/ 			aInsertSymbolFontStartPoss.Insert( nCopy,
/*N*/ 					aInsertSymbolFontStartPoss.Count() );
/*N*/ 			aInsertSymbolFontEndPoss.Insert( nPos,
/*N*/ 					aInsertSymbolFontEndPoss.Count() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	while( aDeleteFontTxtAttrs.Count() )
/*N*/ 	{
/*N*/ 		SwTxtAttr *pAttr = aDeleteFontTxtAttrs[0];
/*N*/ 		aDeleteFontTxtAttrs.Remove( 0 );
/*N*/ 		rNd.Delete( pAttr );
/*N*/ 	}
/*N*/ 	if( aInsertSymbolFontStartPoss.Count() )
/*N*/ 	{
/*N*/ 		const Font& rSymbolFont = SwNumRule::GetDefBulletFont();
/*N*/ 		SvxFontItem aFontItem( rSymbolFont.GetFamily(), rSymbolFont.GetName(),
/*N*/ 						  	   rSymbolFont.GetStyleName(),
/*N*/ 							   rSymbolFont.GetPitch(),
/*N*/ 							   rSymbolFont.GetCharSet() );
/*N*/ 		for( USHORT i=0; i < aInsertSymbolFontStartPoss.Count(); i++ )
/*N*/ 		{
/*N*/ 			rNd.Insert( aFontItem, aInsertSymbolFontStartPoss[i],
/*N*/ 						aInsertSymbolFontEndPoss[i] );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::ConvertText( SwTxtNode& rNd, const ByteString& rText8,
/*N*/ 							xub_StrLen nOffset,
/*N*/ 							SvUShorts *pEncs, SvXub_StrLens *pPoss )
/*N*/ {
/*N*/ 	if( !rText8.Len() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	const SvxFontItem& rFont = rNd.GetSwAttrSet().GetFont();
/*N*/ 	BOOL bNdSym = rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL;
/*N*/ 
/*N*/ 	String& rNdText = (String &)rNd.GetTxt();
/*N*/ 		String aText;
/*N*/ 		ByteString aText8( rText8 );
/*N*/ 		SvxFontItem aFontItem( rFont );
/*N*/ 		if( pConvToSymbolFmts &&
/*N*/ 			lcl_sw3io_isStarSymbolFontItem( aFontItem ) )
/*N*/ 		{
/*?*/ 			BYTE nFlags = pConvToSymbolFmts->GetFlags( rNd.GetFmtColl() );
/*?*/ 			if( (SW3IO_CONV_FROM_BATS & nFlags) != 0 )
/*?*/ 			{
/*?*/ 				aFontItem.GetFamilyName() = sStarBats;
/*?*/ 				aFontItem.GetCharSet() = RTL_TEXTENCODING_SYMBOL;
/*?*/ 			}
/*?*/ 			else if( (SW3IO_CONV_FROM_MATH & nFlags) != 0 )
/*?*/ 			{
/*?*/ 				aFontItem.GetFamilyName() = sStarMath;
/*?*/ 				aFontItem.GetCharSet() = RTL_TEXTENCODING_SYMBOL;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		ConvertText( aText8, aText, nOffset, rNd,
/*N*/ 							   eSrcSet, aFontItem, 0, FALSE );
/*N*/ 		rNdText.Replace( nOffset, aText.Len(), aText );
/*N*/ 		if( bNdSym &&
/*N*/ 			SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState( RES_CHRATR_FONT,
/*N*/ 															 sal_False ) &&
/*N*/ 			( rFont.GetFamilyName().EqualsIgnoreCaseAscii( sStarBats ) ||
/*N*/ 			  rFont.GetFamilyName().EqualsIgnoreCaseAscii( sStarMath ) ) )
/*N*/ 		{
/*?*/ 			const Font& rSymbolFont = SwNumRule::GetDefBulletFont();
/*?*/ 			SvxFontItem aFontItem( rSymbolFont.GetFamily(),
/*?*/ 								   rSymbolFont.GetName(),
/*?*/ 								   rSymbolFont.GetStyleName(),
/*?*/ 								   rSymbolFont.GetPitch(),
/*?*/ 								   rSymbolFont.GetCharSet() );
/*?*/ 			((SwCntntNode&)rNd).SetAttr( aFontItem );
/*N*/ 		}
/*N*/ 
/*N*/ 	if( pEncs )
/*N*/ 	{
/*?*/ 		for( USHORT i=0; i < pEncs->Count(); i++ )
/*?*/ 		{
/*?*/ 			xub_StrLen nStart = (*pPoss)[2*i];
/*?*/ 			String aTmp( rText8.Copy( nStart - nOffset,
/*?*/ 									  (*pPoss)[2*i+1] - nStart ),
/*?*/ 						 (rtl_TextEncoding)(*pEncs)[i] );
/*?*/ 			rNdText.Replace( nStart, aTmp.Len(), aTmp );
/*?*/ 		}
/*N*/ 	}
/*N*/ }

// Text Node einlesen
// Falls kein Node angegeben ist, wird ein neuer Node an der angegebenen
// Position erzeugt.
// nInsFirstPara - beim Document einfuegen, muss der erste Absatz
// 					sonderbehandelt werden (Absatz Attribute!)

/*N*/ void Sw3IoImp::InTxtNode( SwTxtNode* pNd, SwNodeIndex& rPos, xub_StrLen nOffset,
/*N*/ 						  BYTE nInsFirstPara )
/*N*/ {
/*N*/ 	SwTxtNode *pOldNd = pNd && pNd->GetDepends() ? pNd : 0;
/*N*/ 
/*N*/ 	SvStringsDtor *pINetFldTexts = 0;	// Texte aus Internet-Feldern
/*N*/ 	SvXub_StrLens *pINetFldPoss  = 0;		// Positionen der Internet-Felder
/*N*/ 
/*N*/ 	SvXub_StrLens *pErasePoss  = 0;			// Positionen der Draw-Formate
/*N*/ 
/*N*/ 	SvUShorts     *pCharSetColorEncs = 0;
/*N*/ 	SvXub_StrLens *pCharSetColorPoss  = 0;	//
/*N*/ 
/*N*/ 	OpenRec( SWG_TEXTNODE );
/*N*/ 	BYTE   cNumLevel = NO_NUMBERING;
/*N*/ 	USHORT nColl = IDX_DFLT_VALUE, nCondColl = IDX_DFLT_VALUE;
/*N*/ 	SwWrongList *pWrong = 0;
/*N*/ 	// 0x0L: Laenge der Daten
/*N*/ 	// 0x10: Numerierung folgt
/*N*/ 	// 0x20: Wrong-Liste ist nicht dirty
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 	*pStrm >> nColl;
/*N*/ 	if( !IsVersion(SWG_LONGIDX) && (cFlags & 0x10) )
/*N*/ 	{
/*N*/ 		*pStrm >> cNumLevel;
/*N*/ 		// Im SW31-format wurde fuer nicht numerierte
/*N*/ 		// Absaetzte noch ein NO_NUMLEVEL mit rausgeschrieben.
/*N*/ 		if( NO_NUM == cNumLevel  &&
/*N*/ 			IsVersion( SWG_NONUMLEVEL, SWG_DESKTOP40 ) &&
/*N*/ 			pStrm->Tell() != nFlagRecEnd )		// wenn noch Daten da sind
/*?*/ 			*pStrm >> cNumLevel;				// NO_NUM -> NO_NUMLEVEL
/*N*/ 
/*N*/ 		// Wenn ein NO_NUM gelesen wurde muss es noch in ein NO_NUMLEVEL
/*N*/ 		// umgewandelt werden.
/*N*/ 		bConvertNoNum |= (NO_NUM == cNumLevel);
/*N*/ 	}
/*N*/ 
/*N*/ 	if( IsVersion( SWG_CONDCOLLS, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*N*/ 		// bedingte Vorlagen gibt es nicht im 31-Export-Format
/*N*/ 		*pStrm >> nCondColl;
/*N*/ 		if( IDX_DFLT_VALUE != nCondColl )
/*N*/ 		{
/*N*/ 			// es ist eine gesetzt, dieses ist die bedingte Vorlage
/*N*/ 			USHORT nTmp = nCondColl;
/*N*/ 			nCondColl = nColl;
/*N*/ 			nColl = nTmp;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseFlagRec();
/*N*/ 	SwTxtFmtColl* pColl = FindTxtColl( nColl );
/*N*/ 
/*N*/ 	// JP 07.08.00: set never the default text format collection on a node
/*N*/ 	if( pColl == pDoc->GetDfltTxtFmtColl() )
/*?*/ 		pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
/*N*/ 
/*N*/ 	// Der Text des Nodes darf nicht einfach so konvertiert werden!
/*N*/ 	ByteString aText8;
/*N*/ 	pStrm->ReadByteString( aText8 );
/*N*/ 	if( pCrypter )
            pCrypter->Decrypt( aText8 );
/*N*/ 	String aText( aText8, eSrcSet );
/*N*/ 	if( !pNd )
/*N*/ 	{
/*N*/ 		pNd = pDoc->GetNodes().MakeTxtNode( rPos, pColl );
/*N*/ 		rPos--;
/*N*/ 		(String&) pNd->GetTxt() = aText;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( !nInsFirstPara )
/*N*/ 			pNd->ChgFmtColl( pColl );
/*N*/ 		SwIndex aOff( pNd, nOffset );
/*N*/ 		pNd->Insert( aText, aOff );
/*N*/ 	}
/*N*/ 
/*N*/ 	// Der Offset kann wegen Einf. von nicht sichtbaren Redlines auch
/*N*/ 	// negativ werden. Das darf aber auch ausser weiteren Redlines nichts
/*N*/ 	// mehr kommen.
/*N*/ 	INT32 nOffsetL = nOffset;
/*N*/ 
/*N*/ 	BOOL bConverted = FALSE;
/*N*/ 
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		BYTE cType = Peek();
/*N*/ 		switch( cType )
/*N*/ 		{
/*N*/ 			case SWG_ATTRSET:
/*N*/ 				if( nInsFirstPara )
/*N*/ 				{
/*?*/ 					SwAttrSet aTmpSet( pDoc->GetAttrPool(),
/*?*/ 							RES_CHRATR_BEGIN, RES_CHRATR_END - 1 );
/*?*/ 					InAttrSet( aTmpSet );
/*?*/ 					if( aTmpSet.Count() )
/*?*/ 					{
/*?*/ 						ASSERT( nOffsetL>=0,
/*?*/ 								"Offset darf hier nicht negativ sein" );
/*?*/ 						if( 2 == nInsFirstPara )
/*?*/ 							pNd->SetAttr( aTmpSet, 0, aText.Len() );
/*?*/ 						else
/*?*/ 							pNd->SetAttr( aTmpSet, (xub_StrLen)nOffsetL,
/*?*/ 										  pNd->GetTxt().Len() );
/*?*/ 
/*?*/ 						if( pNd->GetpSwAttrSet() )
/*?*/ 							pNd->GetpSwAttrSet()->SetModifyAtAttr( pNd );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if( pOldNd )
/*N*/ 				{
/*?*/ 					SwAttrSet aTmpSet( pDoc->GetAttrPool(), aTxtNodeSetRange );
/*?*/ 					InAttrSet( aTmpSet );
/*?*/ 					if( aTmpSet.Count() )
/*?*/ 					{
/*?*/ 						pNd->SwCntntNode::SetAttr( aTmpSet );
/*?*/ 
/*?*/ 						if( pNd->GetpSwAttrSet() )
/*?*/ 							pNd->GetpSwAttrSet()->SetModifyAtAttr( pNd );
/*?*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( !pNd->GetpSwAttrSet() )
/*N*/ 						((SwCntntNode*) pNd)->NewAttrSet( pDoc->GetAttrPool() );
/*N*/ 					InAttrSet( *pNd->GetpSwAttrSet() );
/*N*/ 					pNd->GetpSwAttrSet()->SetModifyAtAttr( pNd );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case SWG_SDRFMT:
/*N*/ 				// Keine Draw-Formate in Kopf oder Fusszeilen einfuegen oder
/*N*/ 				// wenn kein Drawing-Layer da ist!
/*N*/ 				if( (nGblFlags & SW3F_NODRAWING) || bInsIntoHdrFtr )
/*N*/ 				{
/*?*/ 					SkipRec(); break;
/*N*/ 				} // sonst weiter:
/*N*/ 			case SWG_FLYFMT:
/*N*/ 			{
/*N*/ 				// Absatzgebundener oder Rahmengebundener FlyFrame
/*N*/ 				USHORT eSave_StartNodeType = eStartNodeType;
/*N*/ 				eStartNodeType = SwFlyStartNode;
/*N*/ 				SwFrmFmt* pFmt = (SwFrmFmt*) InFormat( cType, NULL );
/*N*/ 				eStartNodeType = eSave_StartNodeType;
/*N*/ 
/*N*/ 				if( !pFmt )
/*N*/ 					break;
/*N*/ 
/*N*/ 				// Anker darin versenken
/*N*/ 				SwFmtAnchor aAnchor( pFmt->GetAnchor() );
/*N*/ 				if( FLY_AT_CNTNT==aAnchor.GetAnchorId() ||
/*N*/ 					FLY_IN_CNTNT==aAnchor.GetAnchorId() )
/*N*/ 				{
/*N*/ 					// Absatzgebunende Rahmen: Die Abfrage auf FLY_IN_CNTNT
/*N*/ 					// ist drinne, weil der SW31-Export sowas dummerweise
/*N*/ 					// mal exportiert hat...
/*N*/ 					aAnchor.SetType( FLY_AT_CNTNT );
/*N*/ 					SwPosition aPos( rPos );
/*N*/ 					aAnchor.SetAnchor( &aPos );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Dies sollte bisher nur ein rahmengebundener Rahmen
/*N*/ 					// sein, koennte aber auch mal was anderes werden. Es
/*N*/ 					// bleibt dann auf jeden Fall auch di Cntnt-Position
/*N*/ 					// erhalten.
/*?*/ 					SwPosition aPos( rPos, SwIndex(pNd,aAnchor.GetPageNum()) );
/*?*/ 					aAnchor.SetAnchor( &aPos );
/*N*/ 				}
/*N*/ 				aAnchor.SetPageNum( 0 );
/*N*/ 				pFmt->SetAttr( aAnchor );
/*N*/ 				// Layout-Frames im Insert Mode fuer absatzgebundene
/*N*/ 				// Flys erzeugen
/*N*/ 				if( bInsert && !nRes ) pFmt->MakeFrms();
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case SWG_ATTRIBUTE:
/*N*/ 				ASSERT( nOffsetL>=0, "Offset darf hier nicht negativ sein" );
/*N*/ 				InTxtAttr( *pNd, aText8, (xub_StrLen)nOffsetL, &pINetFldTexts,
/*N*/ 						   &pINetFldPoss, &pErasePoss,
/*N*/ 						   &pCharSetColorEncs, &pCharSetColorPoss );
/*N*/ 				break;
/*N*/ 			case SWG_NUMRULE:
/*N*/ 				// NumRules gibt es an dieser Stelle nur im 3.1 und 4.0
/*N*/ 				// Fileformat. Seit dem 5.0-Filformat werden sie in einem
/*N*/ 				// eigenen Stream bzw. am Dok-Anfang gespeichert.
/*N*/ 				OpenNumRange40( rPos );
/*N*/ 				break;
/*N*/ 			case SWG_NODENUM:
/*N*/ 				// Den NodeNum-Record gibt es seit der 5.0
/*N*/ 				{
/*N*/ 					SwNodeNum aNodeNum;
/*N*/ 					InNodeNum( aNodeNum );
/*N*/ 					pNd->UpdateNum( aNodeNum );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case SWG_MARK:
/*N*/ 				ASSERT( nOffsetL>=0, "Offset darf hier nicht negativ sein" );
/*N*/ 				InNodeMark( rPos, (xub_StrLen)nOffsetL );
/*N*/ 				break;
/*N*/ 
/*N*/ 			case SWG_NODEREDLINE:
/*N*/ 				// nOffsetL ist Referenz-Parameter.
/*N*/ 				// nOffsetL kann jetzt negativ werden!
/*N*/ 				// The text has to be converted before any redlines are
/*N*/ 				// inserted. Otherwise, the content positions will not match
/*N*/ 				// the indices within the 8-Bit-Text.
/*N*/ /*?*/ 				if( !bConverted )	// SW50.SDW
/*N*/ /*?*/ 				{
/*N*/ /*?*/ 					ConvertText( *pNd, aText8, (xub_StrLen)nOffsetL,
/*N*/ /*?*/ 								 pCharSetColorEncs, pCharSetColorPoss );
/*N*/ /*?*/ 					bConverted = TRUE;
/*N*/ /*?*/ 				}
/*N*/ /*?*/ 				InNodeRedline( rPos, nOffsetL );
/*?*/ 				break;
/*N*/ 
/*N*/ 			case SWG_WRONGLIST:
/*N*/ 			{
/*N*/ 				if( IsVersion( SWG_DESKTOP40 ) )
/*N*/ 				{
/*N*/ 					OpenRec( SWG_WRONGLIST );
/*N*/ 					pWrong = new SwWrongList;
/*N*/ 					UINT16 nBeginInv, nEndInv, nCount;
/*N*/ 					OpenFlagRec();
/*N*/ 					*pStrm >> nBeginInv >> nEndInv;
/*N*/ 					CloseFlagRec();
/*N*/ 					pWrong->SetInvalid( (xub_StrLen)nBeginInv,
/*N*/ 										(xub_StrLen)nEndInv );
/*N*/ 					*pStrm >> nCount;
/*N*/ 					for( USHORT i=0; Good() && i<nCount; i++ )
/*N*/ 					{
/*N*/ 						UINT32 nWrong;
/*N*/ 						*pStrm >> nWrong;
/*N*/ 						xub_StrLen nPos = (xub_StrLen)nWrong;
/*N*/ 						xub_StrLen nLen = (xub_StrLen)(0xFFFF & (nWrong >> 16));
/*N*/ 						pWrong->Insert( nPos, nLen, pWrong->Count() );
/*N*/ 					}
/*N*/ 					if( bSpellAllAgain )
/*N*/ 						pWrong->SetInvalid( 0, STRING_MAXLEN );
/*N*/ 					if( bSpellWrongAgain )
/*N*/ 						pWrong->InvalidateWrong();
/*N*/ 					CloseRec( SWG_WRONGLIST );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					SkipRec();
/*N*/ 			}
/*N*/ 				break;
/*N*/ 			default:
/*?*/ 				SkipRec();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	CloseRec( SWG_TEXTNODE );
/*N*/ 
/*N*/ 	// Eventuell den Text konvertieren
/*N*/ 	if( !bConverted )
/*N*/ 		ConvertText( *pNd, aText8, (xub_StrLen)nOffsetL,
/*N*/ 					 pCharSetColorEncs, pCharSetColorPoss );
/*N*/ 
/*N*/ 	// Numerierung uebernehmen
/*N*/ 	if( !IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		if( cNumLevel != NO_NUMBERING )
/*N*/ 		{
/*N*/ 			// MAXLEVEL war im 3.1/4.0-SW 5 und kann sich nichr mehr aendern,
/*N*/ 			// deshalb baruchen wir es nicht zu beachten.
/*N*/ #if 0
/*N*/ 			if( cNumLevel != NO_NUM && GetRealLevel(cNumLevel) >= MAXLEVEL )
/*N*/ 			{
/*N*/ 				// die Numerierungs-Ebene ist zu hoch => die hoecht moegliche
/*N*/ 				// setzen
/*N*/ 				BYTE cTmp = MAXLEVEL-1;
/*N*/ 				if( cNumLevel & NO_NUMLEVEL )
/*N*/ 					cTmp |= NO_NUMLEVEL;
/*N*/ 				cNumLevel = cTmp;
/*N*/ 			}
/*N*/ #endif
/*N*/ 			pNd->UpdateNum( SwNodeNum( cNumLevel ) );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			CloseNumRange40( rPos );
/*N*/ 	}
/*N*/ 
/*N*/ 	const SwNodeNum *pNdNum = pNd->GetNum();
/*N*/ 	const SwAttrSet *pAttrSet = pNd->GetpSwAttrSet();
/*N*/ 	if( pNdNum && IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		if( pAttrSet )
/*N*/ 		{
/*N*/ 			// Wenn der Absatz numeriert ist, muss die zugehoerige Numerierung
/*N*/ 			// noch als benutzt markiert werden bzw. eine automatische
/*N*/ 			// Numerierung beim Einfuegen umbenannt werden. Da automatische
/*N*/ 			// Numerierungen nicht in Vorlagen vorkommen koennen, gehen
/*N*/ 			// wir hier ueber das Attribut im Node-AttrSet und damit direkt
/*N*/ 			// an unser Namens-Array.
/*N*/ 			const SfxPoolItem *pItem;
/*N*/ 			if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_PARATR_NUMRULE,
/*N*/ 														FALSE, &pItem )  )
/*N*/ 			{
/*N*/ 				const String& rName = ((const SwNumRuleItem*)pItem)->GetValue();
/*N*/ 				if( rName.Len() )
/*N*/ 				{
/*N*/ 					Sw3NumRuleInfo aTmp( rName );
/*N*/ 					USHORT nPos;
/*N*/ 					if( aNumRuleInfos.Seek_Entry( &aTmp, &nPos ) )
/*N*/ 					{
/*N*/ 						Sw3NumRuleInfo *pInfo = aNumRuleInfos[nPos];
/*N*/ 						if( !bNormal || bInsert )
/*N*/ 						{
/*N*/ 							// Beim Einfuegen oder Laden von Seitenvorlagen
/*N*/ 							// muss das Item evtl. noch an den geaenderten Namen
/*N*/ 							// der Seiten-Vorlage angepasst werden.
/*?*/ 							pInfo->SetUsed();
/*?*/ 							if( rName != pInfo->GetNewName() )
/*?*/ 							{
/*?*/ 								((SwCntntNode *)pNd)
/*?*/ 									->SetAttr( SwNumRuleItem(pInfo->GetNewName()) );
/*?*/ 							}
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							// Die entsprechende NumRule wird benutzt und braucht
/*N*/ 							// nicht mehr geloescht zu werden. Also raus aus
/*N*/ 							// dem Array damit.
/*N*/ 							aNumRuleInfos.Remove( nPos, 1 );
/*N*/ 							delete pInfo;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*?*/ 					SwNodeNum aNodeNum( NO_NUMBERING );
/*?*/ 					pNd->UpdateNum( aNodeNum );
/*?*/ 					pNdNum = 0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( bPageDescs && !bNumRules )
/*N*/ 		{
/*N*/ 			// Wenn Seiten-Vorlagen aber keine Numerierungs-Vorlagen
/*N*/ 			// geladen werden, dann muessen wir sicherstellen, dass
/*N*/ 			// die Numerierungs-Vorlage auch existiert.
/*?*/ 			const SfxPoolItem* pItem =
/*?*/ 				pNd->GetNoCondAttr( RES_PARATR_NUMRULE, TRUE );
/*?*/ 			if( pItem && ((SwNumRuleItem*)pItem)->GetValue().Len() &&
/*?*/ 				!pDoc->FindNumRulePtr( ((SwNumRuleItem*)pItem)->GetValue() ) )
/*?*/ 			{
/*?*/ 				const String& rName = ((SwNumRuleItem*)pItem)->GetValue();
/*?*/ 				USHORT nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rName, GET_POOLID_NUMRULE );
/*?*/ 				if( nPoolId != USHRT_MAX )
/*?*/ 					pDoc->GetNumRuleFromPool( nPoolId );
/*?*/ 				else
/*?*/ 					pDoc->MakeNumRule( rName );
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef NUM_RELSPACE
/*N*/ 	// Wenn der Absatz ein LRSpace-Item enthaelt und in der Kapitel-Numerierung
/*N*/ 	// ist muss das LRSpace-Item noch angepasst werden. Relative Werte
/*N*/ 	// koennen dabei nicht vorkommen.
/*N*/ 	const SwNumRule *pOutline = pDoc->GetOutlineNumRule();
/*N*/ 	const SfxPoolItem *pItem;
/*N*/ 	if( pAttrSet && (!pNdNum || NO_NUMBERING != pNdNum->GetLevel()) &&
/*N*/ 		NO_NUMBERING != pColl->GetOutlineLevel() &&
/*N*/ 		pOutline && nVersion != SWG_NUMRELSPACE )
/*N*/ 	{
/*N*/ 		const SwNumFmt& rNumFmt = pOutline->Get(
/*N*/ 				GetRealLevel(((const SwTxtFmtColl*)pColl)->GetOutlineLevel()) );
/*N*/ 		USHORT nNumLSpace = rNumFmt.GetAbsLSpace();
/*N*/ 
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_LR_SPACE, FALSE,
/*N*/ 													&pItem ) )
/*N*/ 		{
/*N*/ 			const SvxLRSpaceItem *pParaLRSpace =
/*N*/ 				(const SvxLRSpaceItem *)pItem;
/*N*/ 
/*N*/ 			USHORT nWishLSpace = (USHORT)pParaLRSpace->GetTxtLeft();
/*N*/ 			USHORT nNewLSpace =
/*N*/ 						nWishLSpace > nNumLSpace ? nWishLSpace-nNumLSpace : 0U;
/*N*/ 
/*N*/ 			const SvxLRSpaceItem& rCollLRSpace = pColl->GetLRSpace();
/*N*/ 			if( nNewLSpace == rCollLRSpace.GetTxtLeft() &&
/*N*/ 				pParaLRSpace->GetRight() == rCollLRSpace.GetRight() &&
/*N*/ 				pParaLRSpace->GetTxtFirstLineOfst() ==
/*N*/ 											rCollLRSpace.GetTxtFirstLineOfst() )
/*N*/ 			{
/*N*/ 				pNd->ResetAttr( RES_LR_SPACE );
/*N*/ 			}
/*N*/ 			else if( nNewLSpace != pParaLRSpace->GetTxtLeft() )
/*N*/ 			{
/*?*/ 				SvxLRSpaceItem aLRSpace( *pParaLRSpace );
/*?*/ 				short nFirst = aLRSpace.GetTxtFirstLineOfst();
/*?*/ 				if( nFirst < 0 && (USHORT)-nFirst > nNewLSpace )
/*?*/ 					aLRSpace.SetTxtFirstLineOfst( -(short)nNewLSpace );
/*?*/ 				aLRSpace.SetTxtLeft( nNewLSpace );
/*?*/ 				((SwCntntNode *)pNd)->SetAttr( aLRSpace );
/*N*/ 			}
/*N*/ 
/*N*/ 			if( !IsVersion(SWG_NUMRELSPACE) && nWishLSpace != nNewLSpace )
/*N*/ 				lcl_sw3io__ConvertNumTabStop( *pNd, (long)nWishLSpace -
/*N*/ 													(long)nNewLSpace, FALSE	);
/*N*/ 		}
/*N*/ 		else if( nNumLSpace > 0 && !IsVersion(SWG_NUMRELSPACE) )
/*N*/ 		{
/*N*/ 			lcl_sw3io__ConvertNumTabStop( *pNd, nNumLSpace, FALSE	);
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 
/*N*/ 	if( pINetFldTexts )
/*N*/ 	{
/*N*/ 		ASSERT( pINetFldPoss, "INet-Feld-Texte ohne Positionen???" );
/*N*/ 
/*N*/ 		// Es mussen noch Texte von Internet-Feldern eingefuegt werden
/*N*/ 
/*N*/ 		INT32 nOffset2 = 0;	// Verschiebung durch die Felder selbst
/*N*/ 
/*N*/ 		for( USHORT i=0; i<pINetFldTexts->Count(); i++ )
/*N*/ 		{
/*N*/ 			const String &rStr = *(*pINetFldTexts)[i];
/*N*/ 
/*N*/ 			// den Text hinter dem 0xff vom Feld einfuegen
/*N*/ 			xub_StrLen nPos = xub_StrLen( nOffset2 + nOffset +
/*N*/ 										  (*pINetFldPoss)[i] + 1 );
/*N*/ 			SwIndex aOff( pNd, nPos );
/*N*/ 
/*N*/ 			if( rStr.Len() )
/*N*/ 			{
/*N*/ 				pNd->Insert( rStr, aOff );
/*N*/ 			}
/*N*/ 
/*N*/ 			// und das 0xff loeschen
/*N*/ 			aOff.Assign( pNd, nPos-1 );
/*N*/ 			pNd->Erase( aOff, 1 );
/*N*/ 
/*N*/ 			// und den Offset korrigieren
/*N*/ 			nOffset2 += rStr.Len();
/*N*/ 			nOffset2--;
/*N*/ 		}
/*N*/ 
/*N*/ 		// die Wrong-Liste ist jetzt ungueltig
/*N*/ 		delete pWrong;
/*N*/ 		pWrong = 0;
/*N*/ 		cFlags &= 0xdf;
/*N*/ 
/*N*/ 		// und die Array loeschen
/*N*/ 		delete pINetFldTexts;
/*N*/ 		delete pINetFldPoss;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pErasePoss )
/*N*/ 	{
/*N*/ 		// Es mussen noch 0xff-Zeichen aus dem Node geloescht werden
/*?*/ 		USHORT i = pErasePoss->Count();
/*?*/ 		while( i )
/*?*/ 		{
/*?*/ 			xub_StrLen nPos = (*pErasePoss)[--i];
/*?*/ 
/*?*/ 			ASSERT( CH_TXTATR_BREAKWORD == pNd->GetTxt().GetChar( nPos ) ||
/*?*/ 					CH_TXTATR_INWORD == pNd->GetTxt().GetChar( nPos ),
/*?*/ 					"Es sollten nur 0xff geloescht werden" );
/*?*/ 
/*?*/ 			SwIndex aOff( pNd, nPos );
/*?*/ 			pNd->Erase( aOff, 1 );
/*?*/ 		}
/*?*/ 
/*?*/ 
/*?*/ 		// die Wrong-Liste ist jetzt ungueltig
/*?*/ 		delete pWrong;
/*?*/ 		pWrong = 0;
/*?*/ 		cFlags &= 0xdf;
/*?*/ 
/*?*/ 		delete pErasePoss;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Wrong-Liste uebernehmen
/*N*/ 	// ACHTUNG: dirty-bit wird invers gespeichert weil in alten Doks 0 steht
/*N*/ 	BOOL bWrongDirty = ( ( cFlags & 0x20 ) == 0 ) ||
/*N*/ 						nVersion < SWG_DESKTOP40 ||
/*N*/ 						bSpellAllAgain || bSpellWrongAgain;
/*N*/ 	pNd->SetWrongDirty( bWrongDirty );
/*N*/ 	pNd->SetWrong( pWrong );
/*N*/ 
/*N*/ 	// Condition-Collections setzen:
/*N*/ 	if( IDX_DFLT_VALUE != nCondColl )
/*N*/ 	{
/*?*/ 		if( bInsert )
/*?*/ 		{
/*?*/ 			// dann muss die richtige Collection neu bestimmt werden!
/*?*/ 			pNd->ChkCondColl();
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			SwTxtFmtColl* pCColl = FindTxtColl( nCondColl );
/*?*/ 			pNd->SetCondFmtColl( pCColl );
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef NUM_RELSPACE
/*N*/ 	if( pNdNum && NO_NUMBERING != pNdNum->GetLevel() &&
/*N*/ 		IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		// In Dokumenten, in denen der Einzug einer Numerierung noch
/*N*/ 		// absolut war, muss der Absatz-Einzug noch angepasst werden.
/*N*/ 		// Weil man dazu die bedingte Vorlage braucht, darf das erst
/*N*/ 		// hier geschehen.
/*N*/ 		const SwNumRule *pNumRule = pNd->GetNumRule();
/*N*/ 		if( pNumRule )
/*N*/ 			lcl_sw3io__ConvertNumLRSpace( *pNd, *pNumRule,
/*N*/ 										  pNdNum->GetLevel(),
/*N*/ 										  !IsVersion(SWG_NUMRELSPACE) );
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	rPos++;
/*N*/ }

// Zaehlen der Worte eines Nodes
//!! Wird auch vom SW2-Reader benutzt!!

/*N*/ void sw3io_countwords( const String& rDelimWrd, const String& rStr,
/*N*/ 						ULONG &rWords, ULONG &rChars )
/*N*/ {
/*N*/ 	FASTBOOL bInWord = FALSE;
/*N*/ 	USHORT nSpChars = 0;
/*N*/ 
/*N*/ 	for( xub_StrLen nPos = 0; nPos < rStr.Len(); nPos++ )
/*N*/ 	{
/*N*/ 		sal_Unicode c = rStr.GetChar( nPos );
/*N*/ 		switch( c )
/*N*/ 		{
/*N*/ 			case CH_TXTATR_BREAKWORD:
/*N*/ 			case CH_TXTATR_INWORD:
/*N*/ 				++nSpChars;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case 0x0A:
/*N*/ 				++nSpChars;
/*N*/ 				if ( bInWord )
/*N*/ 				{
/*N*/ 					rWords++;
/*N*/ 					bInWord = FALSE;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 			default:
/*N*/ 				if( rDelimWrd.Search( c ) == STRING_NOTFOUND  )
/*N*/ 					bInWord = TRUE;
/*N*/ 				else if ( bInWord )
/*N*/ 				{
/*N*/ 					rWords++;
/*N*/ 					bInWord = FALSE;
/*N*/ 				}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bInWord )
/*N*/ 		rWords++;
/*N*/ 	rChars += rStr.Len() - nSpChars;
/*N*/ }

/*N*/ SwInsHardBlankSoftHyph::~SwInsHardBlankSoftHyph()
/*N*/ {
/*N*/ 	for( USHORT n = 0, nCnt = aItems.Count(); n < nCnt; ++n )
/*N*/ 		delete aItems[ n ];
/*N*/ }
/*N*/ void SwInsHardBlankSoftHyph::AddItem( xub_StrLen nPos, sal_Unicode c )
/*N*/ {
/*N*/ 	SfxPoolItem* pItem = 0;
/*N*/ 	switch ( c )
/*N*/ 	{
/*N*/ 	case CHAR_HARDBLANK:	pItem = new SwFmtHardBlank( ' ', FALSE ); break;
/*?*/ 	case CHAR_HARDHYPHEN:	pItem = new SwFmtHardBlank( '-', FALSE ); break;
/*N*/ 	case CHAR_SOFTHYPHEN:	pItem = new SwFmtSoftHyph; break;
/*N*/ 	}
/*N*/ 	if( pItem )
/*N*/ 	{
/*N*/ 		USHORT nInsPos = aItemStarts.Count();
/*N*/ 		aItemStarts.Insert( nPos, nInsPos );
/*N*/ 		aItems.C40_INSERT( SfxPoolItem, pItem, nInsPos );
/*N*/ 	}
/*N*/ }
/*N*/ void SwInsHardBlankSoftHyph::ChangePos( xub_StrLen nHtEnd, xub_StrLen nOffs )
/*N*/ {
/*N*/ 	for( USHORT n = 0, nCnt = aItemStarts.Count(); n < nCnt; ++n )
/*N*/ 	{
/*?*/ 		xub_StrLen& rStt = aItemStarts[ n ];
/*?*/ 		if( rStt >= nHtEnd )
/*?*/ 			rStt -= nOffs;
/*N*/ 	}
/*N*/ }
/*N*/ void SwInsHardBlankSoftHyph::OutAttr( Sw3IoImp& rIo, xub_StrLen nStt,
/*N*/ 										xub_StrLen nEnd )
/*N*/ {
/*N*/ 	for( USHORT n = 0, nCnt = aItemStarts.Count(); n < nCnt; ++n )
/*N*/ 	{
/*N*/ 		xub_StrLen nHtStt = aItemStarts[ n ];
/*N*/ 		if( nHtStt >= nStt && nHtStt < nEnd )
/*N*/ 		{
/*N*/ 			nHtStt -= nStt;
/*N*/ 			const SfxPoolItem* pAttr = aItems[ n ];
/*N*/ 			rIo.OutAttr( *pAttr, nHtStt, nHtStt );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ Sw3ExportTxtAttrs *Sw3IoImp::ExportTxtNode( const SwTxtNode& rNd,
/*N*/ 											xub_StrLen nStart, xub_StrLen nEnd,
/*N*/ 											rtl_TextEncoding eEnc,
/*N*/ 											SwInsHardBlankSoftHyph& rHBSH )
/*N*/ {
/*N*/ 	USHORT nCntAttr = rNd.HasHints() ? rNd.GetSwpHints().Count() : 0;
/*N*/ 	if( !nCntAttr )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	// Erstmal nachschauen, ob es ueberhaupt Hints gibt, die einen Export
/*N*/ 	// erfordern. Der zusaetzliche Schleifendurchlauf sollte sich in den
/*N*/ 	// meisten Faellen lohnen, weil keine Arrays gewartet werden muessen.
/*N*/ 	USHORT n;
/*N*/ 	BOOL bExport = FALSE;
/*N*/ 	for( n = 0; !bExport && n < nCntAttr; n++ )
/*N*/ 	{
/*N*/ 		const SwTxtAttr* pHt = rNd.GetSwpHints()[n];
/*N*/ 		BOOL   bHtEnd   = BOOL( pHt->GetEnd() != NULL );
/*N*/ 		xub_StrLen nHtStart = *pHt->GetStart();
/*N*/ 		xub_StrLen nHtEnd   = *pHt->GetAnyEnd();
/*N*/ 
/*N*/ 		if( (bHtEnd && nHtEnd > nStart && nHtStart < nEnd) ||
/*N*/ 			(!bHtEnd && nHtStart >= nStart && nHtStart < nEnd ) )
/*N*/ 		{
/*N*/ 			switch( pHt->GetAttr().Which() )
/*N*/ 			{
/*N*/ 			case RES_TXTATR_INETFMT:
/*N*/ 				// SwFmtINetFmt-Attribute werden als Felder exportiert
/*N*/ 				bExport = TRUE;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case RES_TXTATR_FIELD:
/*N*/ 				{
/*N*/ 					// alle neuen Felder (ab Script-Feld) werden ignoriert
/*N*/ 					const SwFmtFld& rFmtFld =
/*N*/ 						(const SwFmtFld&)pHt->GetAttr();
/*N*/ 					bExport = rFmtFld.GetFld()->Which() >= RES_SCRIPTFLD;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 			case RES_TXTATR_FLYCNT:
/*N*/ 				{
/*N*/ 					// zeichengebunde Draw-Formate werden absatz-gebunden
/*N*/ 					const SwFmtFlyCnt& rFlyCnt =
/*N*/ 						(const SwFmtFlyCnt&)pHt->GetAttr();
/*N*/ 					bExport = RES_DRAWFRMFMT == rFlyCnt.GetFrmFmt()->Which();
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !bExport )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	Sw3ExportTxtAttrs *pInfo = new Sw3ExportTxtAttrs;
/*N*/ 	xub_StrLen nINetFmtStart = 0, nINetFmtEnd = 0;
/*N*/ 	for( n = 0; n < nCntAttr; n++ )
/*N*/ 	{
/*N*/ 		const SwTxtAttr* pHt = rNd.GetSwpHints()[n];
/*N*/ 		BOOL   bHtEnd   = BOOL( pHt->GetEnd() != NULL );
/*N*/ 		xub_StrLen nHtStart = *pHt->GetStart();
/*N*/ 		xub_StrLen nHtEnd   = *pHt->GetAnyEnd();
/*N*/ 
/*N*/ 		if( (bHtEnd && nHtEnd > nStart && nHtStart < nEnd) ||
/*N*/ 			(!bHtEnd && nHtStart >= nStart && nHtStart < nEnd) )
/*N*/ 		{
/*N*/ 			// Der Hint liegt zumindest teilweise im Text
/*N*/ 			const SfxPoolItem& rAttr = pHt->GetAttr();
/*N*/ 			BOOL bInsert = FALSE, bSplit = FALSE;
/*N*/ 
/*N*/ 			USHORT nWhich = rAttr.Which();
/*N*/ 			switch( nWhich )
/*N*/ 			{
/*N*/ 			case RES_TXTATR_INETFMT:
/*N*/ 				// leere SwFmtINetFmt-Hint koennen und muessen ignoriert
/*N*/ 				// werden
/*N*/ 				if( nHtStart != nHtEnd )
/*N*/ 				{
/*N*/ 					if( nHtStart >= nINetFmtEnd )
/*N*/ 					{
/*N*/ 						// das SwFmtINetFmt ist auf dem Top-Level
/*N*/ 						nINetFmtStart = nHtStart;
/*N*/ 						nINetFmtEnd	= nHtEnd;
/*N*/ 						bInsert = TRUE;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						// die SwFmtINetFmt-Attribute sind geschachtelt,
/*N*/ 						// das auessere muss gesplittet werden
/*N*/ 						ASSERT( nHtEnd <= nINetFmtEnd,
/*N*/ 								"Seit wann koennen sich gleiche Attribute ueberlappen?" );
/*N*/ 						bSplit = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 			default:
/*N*/ 				// laut AMA werden alle anderen Attribute an den Grenzen
/*N*/ 				// von SwFmtINetFmt-Attributen aufgespannt. Wenn das mal
/*N*/ 				// doch nicht der Fall ist, gibt's einen
/*N*/ 				ASSERT( nHtStart >= nINetFmtEnd || nHtEnd <= nINetFmtEnd,
/*N*/ 						"Ein Attribut ueberlappt sich mit einen SwFmtINetFmt" );
/*N*/ 
/*N*/ 				// Attribute im inneren eines SwFmtINetFmt werden ignoriert,
/*N*/ 				// es sein denn, die spannen genau den Bereich des
/*N*/ 				// SwFmtINetFmts auf
/*N*/ 				if( nHtStart >= nINetFmtEnd ||
/*N*/ 					(nHtStart == nINetFmtStart && nHtEnd == nINetFmtEnd) )
/*N*/ 				{
/*N*/ 					// das Attribut beginnt erst hinter einen SwFmtINetFmt
/*N*/ 					// oder spannt exakt den gleichen Bereich auf
/*N*/ 					bInsert = TRUE;
/*N*/ 				}
/*N*/ 				else if( RES_TXTATR_NOEND_BEGIN <= nWhich &&
/*N*/ 						 RES_TXTATR_NOEND_END > nWhich )
/*N*/ 				{
/*N*/ 					// ein Hint ohne Ende muss auch immer eingefuegt werden
/*N*/ 					bSplit = TRUE;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 
/*N*/ 			if( bInsert )
/*N*/ 			{
/*N*/ 				// das Item als letztes an seiner Start-Position beginnen,
/*N*/ 				// aber in jedem Fall vor einem SwFmtINetFmt
/*N*/ 				USHORT nAPos = pInfo->aItemStarts.Count();
/*N*/ 				while( nAPos > 0 &&
/*N*/ 					   ( (pInfo->aItemStarts[nAPos-1] > nHtStart) ||
/*N*/ 						 (pInfo->aItemStarts[nAPos-1] == nHtStart &&
/*N*/ 						  RES_TXTATR_INETFMT==pInfo->aItems[nAPos-1]->Which()) ) )
/*N*/ 					nAPos--;
/*N*/ 
/*N*/ 				pInfo->aItemStarts.Insert( nHtStart, nAPos );
/*N*/ 				pInfo->aItemEnds.Insert( nHtEnd, nAPos );
/*N*/ 				const SfxPoolItem *pItem = &rAttr;
/*N*/ 				pInfo->aItems.C40_INSERT( SfxPoolItem, pItem, nAPos );
/*N*/ 			}
/*N*/ 			else if( bSplit )
/*N*/ 			{
/*N*/ 				// Ein anderes SwFmtINetFmt splitten. Dazu muss zunaechst
/*N*/ 				// das umgebende gesucht werden.
/*?*/ 				USHORT nAPos = pInfo->aItemStarts.Count();
/*?*/ 				while( nAPos > 0 && pInfo->aItemStarts[nAPos-1] > nHtStart )
/*?*/ 					nAPos--;
/*?*/ 
/*?*/ 				// es muss ein Attribut geben, das zuvor geoffent wurde
/*?*/ 				ASSERT( nAPos, "kein Attribut gefunden" );
/*?*/ 				if( !nAPos )
/*?*/ 					continue;
/*?*/ 				nAPos--;
/*?*/ 
/*?*/ 				// Das unmittlabr vor der aktuellen Position geoffente
/*?*/ 				// Attribut kann auch ein Text-Attribut ohen Ende sein
/*?*/ 				const SfxPoolItem *pLastItem = pInfo->aItems[nAPos];
/*?*/ 				if( RES_TXTATR_NOEND_BEGIN <= pLastItem->Which() &&
/*?*/ 					RES_TXTATR_NOEND_END > pLastItem->Which())
/*?*/ 				{
/*?*/ 					ASSERT( bHtEnd, "zwei Hints ohne Ende an gleicher Pos.?" );
/*?*/ 					// es muss dann aber an der aktuellen Position beginnen
/*?*/ 					ASSERT( pInfo->aItemStarts[nAPos]==nHtStart,
/*?*/ 							"Text-Attribut ohne Ende an falscher Position" );
/*?*/ 					if( pInfo->aItemStarts[nAPos]==nHtStart )
/*?*/ 						continue;
/*?*/ 
/*?*/ 					// das Feld ein Zeichen spater beginnen
/*?*/ 					nHtStart++;
/*?*/ 					if( nHtStart-nHtEnd == 0 )
/*?*/ 						continue;
/*?*/ 
/*?*/ 					nAPos++;
/*?*/ 					ASSERT( nAPos<pInfo->aItems.Count(),
/*?*/ 							"Wo ist das SwFmtINetFmt geblieben?" );
/*?*/ 					pLastItem = pInfo->aItems[nAPos];
/*?*/ 				}
/*?*/ 
/*?*/ 				// muss ein SwFmtINetFmt-Attribut sein!
/*?*/ 				xub_StrLen nLastEnd = pInfo->aItemEnds[nAPos];
/*?*/ 				ASSERT( RES_TXTATR_INETFMT==pLastItem->Which(),
/*?*/ 						"das umgebende Item muesste ein SwFmtINetFmt sein!!" );
/*?*/ 				if( !RES_TXTATR_INETFMT==pLastItem->Which() )
/*?*/ 					continue;
/*?*/ 
/*?*/ 				// das bisherige Attribut vorzeitig beenden, wenn es dann
/*?*/ 				// nicht leer ist und sonst loeschen.
/*?*/ 				if( pInfo->aItemStarts[nAPos] < nHtStart )
/*?*/ 				{
/*?*/ 					pInfo->aItemEnds[nAPos] = nHtStart;
/*?*/ 					nAPos++;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					pInfo->aItemStarts.Remove( nAPos, 1 );
/*?*/ 					pInfo->aItemEnds.Remove( nAPos, 1 );
/*?*/ 					pInfo->aItems.Remove( nAPos, 1 );
/*?*/ 				}
/*?*/ 
/*?*/ 				// jetzt fuegen wir das neue Attribut ein
/*?*/ 				pInfo->aItemStarts.Insert( nHtStart, nAPos );
/*?*/ 				pInfo->aItemEnds.Insert( nHtEnd, nAPos );
/*?*/ 				const SfxPoolItem *pItem = &rAttr;
/*?*/ 				pInfo->aItems.C40_INSERT( SfxPoolItem, pItem, nAPos );
/*?*/ 				nAPos++;
/*?*/ 
/*?*/ 				// und noch den Rest von dem alten Attribut, wenn er nicht
/*?*/ 				// leer ist
/*?*/ 				if( !bHtEnd )
/*?*/ 					nHtEnd++;
/*?*/ 				if( nHtEnd < nLastEnd )
/*?*/ 				{
/*?*/ 					pInfo->aItemStarts.Insert( nHtEnd, nAPos );
/*?*/ 					pInfo->aItemEnds.Insert( nLastEnd, nAPos );
/*?*/ 					pInfo->aItems.C40_INSERT( SfxPoolItem, pLastItem, nAPos );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// jetzt muessen noch der Text und die Positionen der Hints angepasst
/*N*/ 	// werden
/*N*/ 	String aText( rNd.GetTxt() );
/*N*/ 	const SvxFontItem& rFont = rNd.GetSwAttrSet().GetFont();
/*N*/ 	ConvertText( pInfo->aText, aText, 0, (SwTxtNode& )rNd, eEnc,
/*N*/ 						   rFont,
/*N*/ 						   &rHBSH, TRUE );
/*N*/ 	for( USHORT nAPos=0; nAPos<pInfo->aItems.Count(); nAPos++ )
/*N*/ 	{
/*N*/ 		const SfxPoolItem *pAttr = pInfo->aItems[nAPos];
/*N*/ 		xub_StrLen nHtStart = pInfo->aItemStarts[nAPos];
/*N*/ 		xub_StrLen nHtEnd = pInfo->aItemEnds[nAPos];
/*N*/ 		xub_StrLen nOffs = 0;
/*N*/ 		BOOL bRemoveTxtAttr = FALSE;
/*N*/ 		switch( pAttr->Which() )
/*N*/ 		{
/*N*/ 		case RES_TXTATR_INETFMT:
/*N*/ 			// wieviel muss gelosecht werden ?
/*N*/ 			nOffs = (nHtEnd - nHtStart) - 1;
/*N*/ 
/*N*/ 			// den Text ses SwFmtINetFmt merken
/*N*/ 			pInfo->aINetFmtTexts.Insert(
/*N*/ 				new ByteString( pInfo->aText.Copy( nHtStart, nOffs+1 ) ),
/*N*/ 				pInfo->aINetFmtTexts.Count() );
/*N*/ 
/*N*/ 			// und noch den auszugebenden Text anpassen
/*N*/ 			pInfo->aText.SetChar( nHtStart, '\xff' );
/*N*/ 			nHtStart++;
/*N*/ 			break;
/*N*/ 
/*?*/ 		case RES_TXTATR_FIELD:
/*?*/ 			{
/*?*/ 				const SwFmtFld *pFmtFld =
/*?*/ 					(const SwFmtFld *)pAttr;
/*?*/ 				if( pFmtFld->GetFld()->Which() >= RES_SCRIPTFLD )
/*?*/ 					bRemoveTxtAttr = TRUE;
/*?*/ 			}
/*?*/ 			break;
/*?*/ 
/*?*/ 		case RES_TXTATR_FLYCNT:
/*?*/ 			{
/*?*/ 				const SwFmtFlyCnt *pFlyCnt = (const SwFmtFlyCnt *)pAttr;
/*?*/ 				if( RES_DRAWFRMFMT == pFlyCnt->GetFrmFmt()->Which() )
/*?*/ 				{
/*?*/ 					bRemoveTxtAttr = TRUE;
/*?*/ 					pInfo->nDrawFrmFmts++;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( bRemoveTxtAttr )
/*N*/ 		{
/*?*/ 			nOffs = 1;
/*?*/ 			nHtEnd += 1;
/*?*/ 			pInfo->aItemStarts.Remove( nAPos, 1 );
/*?*/ 			pInfo->aItemEnds.Remove( nAPos, 1 );
/*?*/ 			pInfo->aItems.Remove( nAPos, 1 );
/*?*/ 			nAPos--;	// nicht schoen, aber wirksam
/*N*/ 		}
/*N*/ 
/*N*/ 		// die Start- und Ende-Positionen korregieren
/*N*/ 		if( nOffs )
/*N*/ 		{
/*N*/ 			for( USHORT i=0; i<pInfo->aItems.Count(); i++ )
/*N*/ 			{
/*N*/ 				if( pInfo->aItemStarts[i] >= nHtEnd )
/*N*/ 					pInfo->aItemStarts[i] -= nOffs;
/*N*/ 				if( pInfo->aItemEnds[i] >= nHtEnd )
/*N*/ 					pInfo->aItemEnds[i] -= nOffs;
/*N*/ 			}
/*N*/ 
/*N*/ 			rHBSH.ChangePos( nHtEnd, nOffs );
/*N*/ 			pInfo->aText.Erase( nHtStart, nOffs );
/*N*/ 
/*N*/ 			if( nEnd != STRING_LEN )
/*N*/ 				nEnd -= nOffs;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nEnd == STRING_LEN || nEnd < nStart )
/*N*/ 		nEnd = pInfo->aText.Len();
/*N*/ 	else if ( nEnd != pInfo->aText.Len() )
/*?*/ 		pInfo->aText.Erase( nEnd );
/*N*/ 	if( nStart )
/*?*/ 		pInfo->aText.Erase( 0, nStart );
/*N*/ 
/*N*/ 	return pInfo;
/*N*/ }


/*N*/ void lcl_sw3io__ConvertNumTabStop( const SwCntntNode& rCNd, long nOffset,
/*N*/ 								   SfxItemSet& rItemSet )
/*N*/ {
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	if( SFX_ITEM_SET == rCNd.GetSwAttrSet().GetItemState(
/*N*/ 										RES_PARATR_TABSTOP, TRUE, &pItem ))
/*N*/ 	{
/*N*/ 		SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
/*N*/ 		lcl_sw3io__ConvertNumTabStop( aTStop, nOffset );
/*N*/ 		rItemSet.Put( aTStop );
/*N*/ 	}
/*N*/ }

// Text Node:
// BYTE			Flags
//				0x10 - mit Numerierung
// 				0x20 - Wrong-Liste ist nicht dirty
// UINT16		String-Index der Absatzvorlage
// BYTE			Numerierungs-Level (opt.)
// String		Text
// SWG_ATTRSET	eigene Attribute (opt.)
// SWG_FLYFMT	FlyFrame (opt).
// SWG_SDRFMT	Drawing-Objekt (opt).
// SWG_TEXTATTR harte Attribute (opt, mehrfach).
// SWG_WRONGLIST Liste falscher Worte (opt)

/*N*/ void Sw3IoImp::OutTxtNode
/*N*/ ( SwCntntNode & rNode, xub_StrLen nStart, xub_StrLen nEnd, ULONG nPosIdx )
/*N*/ {
/*N*/ 	SwTxtNode *pNd  = &((SwTxtNode&) rNode );
/*N*/ 	const SwFmtColl* pColl = &pNd->GetAnyFmtColl();
/*N*/ 	ASSERT( pColl != pDoc->GetDfltTxtFmtColl(),
/*N*/ 			"the default text format collection isn't allowed on a node" );
/*N*/ 	BOOL bNewNumRule = FALSE;
/*N*/ 	// 0x0L: Laenge der Daten
/*N*/ 	// 0x10: Numerierung folgt (nur 3.1/4.0)
/*N*/ 	// 0x20: Wrong-Liste ist nicht dirty
/*N*/ 	BYTE   cFlags 	 = IsSw31Export() ? 0x02 : 0x04; // CollIdx & CondCollIdx
/*N*/ 	USHORT nColl	 = aStringPool.Add( pColl->GetName(),
/*N*/ 										pColl->GetPoolFmtId() );
/*N*/ 	BYTE   cNumLevel = NO_NUMBERING;
/*N*/ 
/*N*/ 	// Numerierungsregel fuer 3.1/4.0-Export ermitteln
/*N*/ 	const SwNodeNum *pNdNum = pNd->GetNum();
/*N*/ #ifdef DBG_UTIL
/*N*/ 	{
/*N*/ 		const SwNumRule* pNumRule = pNd->GetNumRule();
/*N*/ 		ASSERT( pNumRule ? pNdNum!=0 : TRUE,
/*N*/ 				"Node hat NumRule aber kein NodeNum" );
/*N*/ 		ASSERT( pNdNum ? pNumRule!=0 : TRUE,
/*N*/ 				"Node hat NodeNum aber keine NumRule" );
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	SfxItemSet *pExportAttrSet = 0;
/*N*/ 	const SfxItemSet *pAttrSet = rNode.GetpSwAttrSet();
/*N*/ 
/*N*/ 	if( pNdNum && pNdNum->GetLevel() != NO_NUMBERING )
/*N*/ 	{
/*N*/ 		const SwNumRule* pNumRule = pNd->GetNumRule();
/*N*/ 		if( pNumRule )
/*N*/ 		{
/*N*/ 			cNumLevel = pNdNum->GetLevel();
/*N*/ 			BYTE cRealLevel = GetRealLevel( cNumLevel );
/*N*/ 			if( IsSw31Or40Export() )
/*N*/ 			{
/*N*/ 				if( cRealLevel >= OLD_MAXLEVEL )
/*N*/ 				{
/*N*/ 					// Die Numerierungs-Ebene ist zu hoch => die
/*N*/ 					// hoechst moegliche setzen
/*N*/ 					BYTE cTmp = OLD_MAXLEVEL-1;
/*N*/ 					if( cNumLevel & NO_NUMLEVEL )
/*N*/ 						cTmp |= NO_NUMLEVEL;
/*N*/ 					cNumLevel = cTmp;
/*N*/ 				}
/*N*/ 				if( IsSw31Export() )
/*N*/ 					cFlags += NO_NUMLEVEL & cNumLevel ? 0x12 : 0x11;
/*N*/ 				else
/*N*/ 					cFlags += 0x11;
/*N*/ 				if( pNumRule != pCurNumRule )
/*N*/ 				{
/*N*/ 					// Dann vor dem betroffenen Node ausgeben
/*N*/ 					pCurNumRule = (SwNumRule *)pNumRule;
/*N*/ 
/*N*/ 					//JP 06.10.95: falls SH mal wieder mit der Numerierung
/*N*/ 					//durcheinander kommt
/*N*/ 					//	bNewNumRule = TRUE;
/*N*/ 					bNewNumRule = 0 != pCurNumRule;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ #ifdef NUM_RELSPACE
/*N*/ 			// Den Erstzeilen-Einzug immer aus der NumRule uebernehmen
/*N*/ 			// und als linken Einzug die Summe aus Absatz- und
/*N*/ 			// NumRule-Einzug schreiben.
/*N*/ 			const SwNumFmt& rNumFmt = pNumRule->Get( cRealLevel );
/*N*/ 			const SvxLRSpaceItem& rLRSpace =
/*N*/ 					(const SvxLRSpaceItem&)rNode.GetAttr(RES_LR_SPACE);
/*N*/ 
/*N*/ 			USHORT nLeft = rNumFmt.GetAbsLSpace();
/*N*/ 			USHORT nOldLeft = (USHORT)rLRSpace.GetTxtLeft();
/*N*/ 			if( !pNumRule->IsAbsSpaces() )
/*N*/ 				nLeft += (USHORT)rLRSpace.GetTxtLeft();
/*N*/ 			short nFirstLineOffset =
/*N*/ 				(cNumLevel & NO_NUMLEVEL)==0 ? rNumFmt.GetFirstLineOffset() : 0;
/*N*/ 			if( nLeft != rLRSpace.GetTxtLeft() ||
/*N*/ 				nFirstLineOffset != rLRSpace.GetTxtFirstLineOfst() )
/*N*/ 			{
/*N*/ 				if( pAttrSet )
/*N*/ 					pExportAttrSet = new SfxItemSet( *pAttrSet );
/*N*/ 				else
/*?*/ 					pExportAttrSet = new SfxItemSet( pDoc->GetAttrPool(),
/*?*/ 											 pColl->GetAttrSet().GetRanges() );
/*N*/ 				pAttrSet = pExportAttrSet;
/*N*/ 				SvxLRSpaceItem aLRSpace( rLRSpace );
/*N*/ 				aLRSpace.SetTxtFirstLineOfst( nFirstLineOffset );
/*N*/ 				aLRSpace.SetTxtLeft( nLeft );
/*N*/ 
/*N*/ 				pExportAttrSet->Put( aLRSpace );
/*N*/ 
/*N*/ 				if( IsSw31Or40Export() && nOldLeft != nLeft )
/*N*/ 					lcl_sw3io__ConvertNumTabStop( rNode,
/*N*/ 												  (long)nOldLeft - (long)nLeft,
/*N*/ 												  *pExportAttrSet );
/*N*/ 			}
/*N*/ #endif
/*N*/ 		}
/*N*/ 		else if( IsSw31Or40Export() )
/*N*/ 			pCurNumRule = NULL;
/*N*/ 
/*N*/ 	}
/*N*/ 	else if( IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		pCurNumRule = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Wrong-List-Dirty-Flag (wird wegen alter doks invers gespeichert)
/*N*/ 	if( !IsSw31Export() && !rNode.IsWrongDirty() )
/*N*/ 		cFlags += 0x20;
/*N*/ 
/*N*/ 	OpenRec( SWG_TEXTNODE );
/*N*/ 	*pStrm << cFlags << nColl;
/*N*/ 	if( IsSw31Or40Export() && (cFlags & 0x10) )
/*N*/ 	{
/*N*/ 		// Frueher wurde hier fuer NO_NUMLEVEL noch ein NO_NUM
/*N*/ 		// rausgeschrieben. Ist jetzt nicht mehr noetig.
/*N*/ 		//if( NO_NUMLEVEL & cNumLevel )
/*N*/ 		//	*pStrm << (BYTE)NO_NUM << cNumLevel;
/*N*/ 		//else
/*N*/ 		if( IsSw31Export() && (NO_NUMLEVEL & cNumLevel) )
/*?*/ 			*pStrm << (BYTE)NO_NUM << cNumLevel;
/*N*/ 		else
/*N*/ 			*pStrm << cNumLevel;
/*N*/ 	}
/*N*/ 
/*N*/ 	// bedingte Vorlagen nicht beim SW31-Export rausschreiben
/*N*/ 	if( !IsSw31Export() )
/*N*/ 	{
/*N*/ 		USHORT nCondColl = IDX_DFLT_VALUE;
/*N*/ 		if( pNd->GetCondFmtColl() )
/*N*/ 		{
/*N*/ 			// dann die bedingte Vorlage schreiben!!
/*?*/ 			pColl = pNd->GetFmtColl();
/*?*/ 			nCondColl =
/*?*/ 				aStringPool.Add( pColl->GetName(), pColl->GetPoolFmtId() );
/*N*/ 		}
/*N*/ 		*pStrm << nCondColl;
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef NUM_RELSPACE
/*N*/ 	// Wenn der Absatz ein LRSpace-Item enthaelt und in der Kapitel-Numerierung
/*N*/ 	// ist muss das LRSpace-Item noch angepasst werden. Relative Werte
/*N*/ 	// koennen dabei nicht vorkommen. Der Ertzeilen-Einzug geht verloren.
/*N*/ 	const SwNumRule *pOutline = pDoc->GetOutlineNumRule();
/*N*/ 	const SfxPoolItem *pItem;
/*N*/ 	if( pAttrSet && (!pNdNum || NO_NUMBERING == pNdNum->GetLevel()) &&
/*N*/ 		NO_NUMBERING != ((const SwTxtFmtColl *)pColl)->GetOutlineLevel() &&
/*N*/ 		SFX_ITEM_SET == pAttrSet->GetItemState( RES_LR_SPACE, FALSE, &pItem ) &&
/*N*/ 		pOutline )
/*N*/ 	{
/*?*/ 		const SvxLRSpaceItem *pParaLRSpace = (const SvxLRSpaceItem *)pItem;
/*?*/ 
/*?*/ 		const SwNumFmt& rNumFmt = pOutline->Get(
/*?*/ 			GetRealLevel(((const SwTxtFmtColl *)pColl)->GetOutlineLevel()) );
/*?*/ 		USHORT nLSpace = (USHORT)pParaLRSpace->GetTxtLeft();
/*?*/ 		USHORT nOldLSpace = nLSpace;
/*?*/ 		if( pOutline->IsAbsSpaces() )
/*?*/ 			nLSpace = rNumFmt.GetAbsLSpace();
/*?*/ 		else
/*?*/ 			nLSpace += rNumFmt.GetAbsLSpace();
/*?*/ 
/*?*/ 		if( nLSpace  != pParaLRSpace->GetTxtLeft() ||
/*?*/ 			rNumFmt.GetFirstLineOffset()!=pParaLRSpace->GetTxtFirstLineOfst() )
/*?*/ 		{
/*?*/ 			if( !pExportAttrSet )
/*?*/ 			{
/*?*/ 				pExportAttrSet = new SfxItemSet( *pAttrSet );
/*?*/ 				pAttrSet = pExportAttrSet;
/*?*/ 			}
/*?*/ 
/*?*/ 			SvxLRSpaceItem aLRSpace( *pParaLRSpace );
/*?*/ 			aLRSpace.SetTxtFirstLineOfst( rNumFmt.GetFirstLineOffset());
/*?*/ 			aLRSpace.SetTxtLeft( nLSpace );
/*?*/ 			pExportAttrSet->Put( aLRSpace );
/*?*/ 
/*?*/ 			if( IsSw31Or40Export() && nOldLSpace != nLSpace )
/*?*/ 				lcl_sw3io__ConvertNumTabStop( rNode,
/*?*/ 											  (long)nOldLSpace - (long)nLSpace,
/*?*/ 											  *pExportAttrSet );
/*?*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	if( (nEnd == STRING_LEN ? pNd->GetTxt().Len() : nEnd) > STRING_MAXLEN52 )
/*N*/ 		nEnd = STRING_MAXLEN52;
/*N*/ 
/*N*/ 	SwInsHardBlankSoftHyph aHBSH;
/*N*/ 
/*N*/ 	String aText;
/*N*/ 	Sw3ExportTxtAttrs *pExpInfo = IsSw31Export()
/*N*/ 			? ExportTxtNode( *pNd, nStart, nEnd, eSrcSet, aHBSH )
/*N*/ 			: 0;
/*N*/ 
/*N*/ 	ByteString aText8;
/*N*/ 	if( !pExpInfo )
/*N*/ 	{
/*N*/ 		aText = pNd->GetTxt();
/*N*/ 		if( nEnd == STRING_LEN || nEnd < nStart )
/*N*/ 			nEnd = aText.Len();
/*N*/ 		else if ( nEnd != aText.Len() )
/*?*/ 			aText.Erase( nEnd );
/*N*/ 		if( nStart )
/*?*/ 			aText.Erase( 0, nStart );
/*N*/ 		const SvxFontItem& rFont = pNd->GetSwAttrSet().GetFont();
/*N*/ 		ConvertText( aText8, aText, nStart, *pNd, eSrcSet,
/*N*/ 							   rFont, &aHBSH, TRUE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aText8 = pExpInfo->aText;
/*N*/ 
/*N*/ 	if( aText.Len() )
/*N*/ 		sw3io_countwords( aDefWordDelim, aText, aStat.nWord, aStat.nChar );
/*N*/ 
/*N*/ 	if( aText8.Len() && pCrypter )
            pCrypter->Encrypt( aText8 );
/*N*/ 	pStrm->WriteByteString( aText8 );
/*N*/ 	aStat.nPara++;
/*N*/ 
/*N*/ 	if( pAttrSet )
/*N*/ 		OutAttrSet( *pAttrSet );
/*N*/ 	delete pExportAttrSet;
/*N*/ 	pAttrSet = pExportAttrSet = 0;
/*N*/ 	OutNodeMarks( nPosIdx );
/*N*/ 
/*N*/ 	// die absatzgebunden Rahmen schreiben und beim SW31-Export ggf. auch
/*N*/ 	// noch die zeichengebunden Zeichen-Objekte als abstzgebundene Objekte
/*N*/ 	OutNodeFlyFrames( nPosIdx );
/*N*/ 	if( pExpInfo && pExpInfo->nDrawFrmFmts )
            ExportNodeDrawFrmFmts( *pNd, nStart, nEnd, pExpInfo->nDrawFrmFmts );
/*N*/ 
/*N*/ 	// Beim SW31-Export evtl. die "umgebauten" Hints ausgeben, sonst die
/*N*/ 	// Original-Hints
/*N*/ 	if( pExpInfo )
/*N*/ 		ExportTxtAttrs( pExpInfo, nStart, nEnd );
/*N*/ 	else if( ((SwTxtNode&)rNode).HasHints() )
/*N*/ 		OutTxtAttrs( *pNd, nStart, nEnd );
/*N*/ 	aHBSH.OutAttr( *this, nStart, nEnd );
/*N*/ 
/*N*/ 	// Evtl. noch die neue Numerierungsregel (3.1/4.0) oder die
/*N*/ 	// SwNodeNum-Struktor (5.0 ff) ausgeben
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		if( bNewNumRule )
/*N*/ 			OutNumRule( SWG_NUMRULE, *pCurNumRule );
/*N*/ 	}
/*N*/ 	else if( pNdNum )
/*N*/ 	{
/*N*/ 		OutNodeNum( *pNdNum );
/*N*/ 	}
/*N*/ 
/*N*/ 	// Eventuell noch die Wrong-Liste
/*N*/ 	const SwWrongList* pWrong = pNd->GetWrong();
/*N*/ 	if( !IsSw31Export() && pWrong )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_WRONGLIST );
/*N*/ 
/*N*/ 		// der Header
/*N*/ 		cFlags = 0x04;	// 4 Bytes Daten
/*N*/ 		xub_StrLen nBegin = pWrong->GetBeginInv();
/*N*/ 		if( nBegin > STRING_MAXLEN52 )
/*N*/ 			nBegin = STRING_MAXLEN52;
/*N*/ 
/*N*/ 		xub_StrLen nEnd = pWrong->GetEndInv();
/*N*/ 		if( nEnd > STRING_MAXLEN52 )
/*N*/ 			nEnd = STRING_MAXLEN52;
/*N*/ 
/*N*/ 		*pStrm << cFlags
/*N*/ 			   << (UINT16)nBegin
/*N*/ 			   << (UINT16)nEnd;
/*N*/ 
/*N*/ 		// nun die eigentliche Liste
/*N*/ 		OpenValuePos16( 0 );
/*N*/ 		USHORT nCount = pWrong->Count();
/*N*/ 		for( USHORT i=0; i<nCount; i++ )
/*N*/ 		{
/*N*/ 			xub_StrLen nIdx = pWrong->Pos( i );
/*N*/ 			if( nIdx < STRING_MAXLEN52 )
/*N*/ 			{
/*N*/ 				xub_StrLen nLen = pWrong->Len( i );
/*N*/ 				if( nIdx + nLen > STRING_MAXLEN52 )
/*N*/ 					nLen = STRING_MAXLEN52 - nIdx;
/*N*/ 				 UINT32 n = nIdx + (nLen << 16);
/*N*/ 				*pStrm << n;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		CloseValuePos16( nCount );
/*N*/ 		CloseRec( SWG_WRONGLIST );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		// Redline-Markierungen rausschreiben. Muss wegen geloeschter
/*N*/ 		// Redlines immer als letztes passieren, weil beim Einfuegen
/*N*/ 		// eines solechen Doks Attribute schon gesetzt sein muessen!
/*N*/ 		OutNodeRedlines( nPosIdx );
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_TEXTNODE );
/*N*/ }

/*N*/ void Sw3IoImp::OutEmptyTxtNode( ULONG nNodeIdx, BOOL bNodeMarks )
/*N*/ {
/*N*/ 	// 0x0L: length of data
/*N*/ 	// 0x20: wrong list is valid
/*N*/ 	BYTE   cFlags = 0x24; // CollIdx & CondCollIdx
/*N*/ 	USHORT nColl = aStringPool.Add( *SwStyleNameMapper::GetTextUINameArray()
/*N*/ 						[ RES_POOLCOLL_STANDARD - RES_POOLCOLL_TEXT_BEGIN ],
/*N*/ 						RES_POOLCOLL_STANDARD );
/*N*/ 
/*N*/ 	OpenRec( SWG_TEXTNODE );
/*N*/ 	*pStrm << cFlags << nColl << IDX_DFLT_VALUE;
/*N*/ 	OutString( *pStrm, aEmptyStr );
/*N*/ 
/*N*/ 	if( bNodeMarks )
/*?*/ 		OutNodeMarks( nNodeIdx );
/*N*/ 
/*N*/ 	aStat.nPara++;
/*N*/ 	CloseRec( SWG_TEXTNODE );
/*N*/ }

// nOffset ist ungleich Null, wenn innerhalb eines Nodes eingefuegt werden
// soll. Dann ist nOffset die Start-Position des Textes.

/*N*/ void Sw3IoImp::InTxtAttr( SwTxtNode& rNd, const ByteString& rText8,
/*N*/ 						  xub_StrLen nOffset,
/*N*/ 						  SvStringsDtor **pINetFldTexts,
/*N*/ 						  SvXub_StrLens **pINetFldPoss,
/*N*/ 						  SvXub_StrLens **pErasePoss,
/*N*/ 						  SvUShorts		**pCharSetColorEncs,
/*N*/ 						  SvXub_StrLens **pCharSetColorPoss )
/*N*/ {
/*N*/ 	// Dieser Record kann auch leer sein
/*N*/ 	// (bei teilweisem Speichern eines Nodes, z.B.)
/*N*/ 	xub_StrLen nLen = rNd.GetTxt().Len();
/*N*/ 	if( nLen ) nLen --;
/*N*/ 	xub_StrLen nStart, nEnd;
/*N*/ 	SfxPoolItem* pItem = InAttr( nStart, nEnd, &rNd );
/*N*/ 	if( !pItem )
/*N*/ 	{
/*N*/ 		if( bDrawFmtSkipped )
/*N*/ 		{
/*N*/ 			ASSERT( bInsIntoHdrFtr,
/*?*/ 					"Draw-Formate durften nur in Kopf-/Fusszeilen geloecht werden" );
/*?*/ 			ASSERT( CH_TXTATR_BREAKWORD == rNd.GetTxt().GetChar(nStart) ||
/*?*/ 					CH_TXTATR_INWORD == rNd.GetTxt().GetChar(nStart),
/*?*/ 					"Wo ist das 0xff des Draw-Formats?" );
/*?*/ 
/*?*/ 			if( !(*pErasePoss) )
/*?*/ 				*pErasePoss = new SvXub_StrLens;
/*?*/ 			(*pErasePoss)->Insert( nStart, (*pErasePoss)->Count() );
/*?*/ 
/*?*/ 			bDrawFmtSkipped = FALSE;
/*?*/ 			return;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( !pFmtINetFmt )
/*N*/ 			return;
/*N*/ 
/*N*/ 		// Es wurde ein INetFlield gelesen und in einen INet-Attribut
/*N*/ 		// umgewandelt
/*N*/ 
/*N*/ 		// Text und Position merken
/*N*/ 		if( !(*pINetFldTexts) )
/*N*/ 			*pINetFldTexts = new SvStringsDtor;
/*N*/ 		(*pINetFldTexts)->Insert( new String( aINetFldText ),
/*N*/ 										(*pINetFldTexts)->Count() );
/*N*/ 		if( !(*pINetFldPoss) )
/*N*/ 			*pINetFldPoss = new SvXub_StrLens;
/*N*/ 		(*pINetFldPoss)->Insert( nStart, (*pINetFldPoss)->Count() );
/*N*/ 
/*N*/ 		if( aINetFldText.Len() )
/*N*/ 		{
/*N*/ 			// ggf. das Attribut ueber dem =xff aufspannen
/*N*/ 			// das Item wird ann unten geloescht
/*N*/ 			pItem = pFmtINetFmt;
/*N*/ 			nEnd++;
/*N*/ 			aINetFldText.Erase();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// leere Felder nicht einfuegen
/*N*/ 			delete pFmtINetFmt;
/*N*/ 		}
/*N*/ 
/*N*/ 		pFmtINetFmt = 0;
/*N*/ 
/*N*/ 		if( !pItem )
/*N*/ 			return;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nEnd < nStart ) nEnd = nLen;
/*N*/ 	nStart += nOffset;
/*N*/ 	nEnd   += nOffset;
/*N*/ 	USHORT nWhich = pItem->Which();
/*N*/ 	if( nWhich == RES_TXTATR_FTN )
/*N*/ 	{
/*N*/ 		//JP 02.12.96:
/*N*/ 		// Das Fussnoten-Attribut liest seine Section "auf der Wiese" ein
/*N*/ 		// und erzeugt auch ihr TextAttribut (weil an dem noch weitere
/*N*/ 		// Informationen gesetzt werden muessen - Referenznummer)
/*N*/ 		SwTxtFtn& rFtn = *((SwFmtFtn*)pItem)->GetTxtFtn();
/*N*/ 		*rFtn.GetStart() = nStart;
/*N*/ 		rNd.Insert( &rFtn, SETATTR_NOTXTATRCHR );
/*N*/ 		return ;
/*N*/ 	}
/*N*/ 	else if( RES_CHRATR_CHARSETCOLOR == nWhich )
/*N*/ 	{
/*?*/ 		if( !(*pCharSetColorEncs) )
/*?*/ 			*pCharSetColorEncs = new SvUShorts;
/*?*/ 		(*pCharSetColorEncs)->Insert(
/*?*/ 			((const SvxCharSetColorItem *)pItem)->GetCharSet(),
/*?*/ 			(*pCharSetColorEncs)->Count() );
/*?*/ 
/*?*/ 		if( !(*pCharSetColorPoss) )
/*?*/ 			*pCharSetColorPoss = new SvXub_StrLens;
/*?*/ 		(*pCharSetColorPoss)->Insert( nStart, (*pCharSetColorPoss)->Count() );
/*?*/ 		(*pCharSetColorPoss)->Insert( nEnd, (*pCharSetColorPoss)->Count() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Bug 31560: mehrere TOX-Marks ohne Ende an der gleichen Position!
/*N*/ 		if( nStart == nEnd && (( RES_TXTATR_TOXMARK == nWhich &&
/*N*/ 			  ((SwTOXMark*)pItem)->IsAlternativeText() ) ||
/*N*/ 			( RES_TXTATR_NOEND_BEGIN <= nWhich && nWhich < RES_TXTATR_NOEND_END )))
/*N*/ 		{
/*N*/ 			// teste doch mal ob das Zeichen am der Position steht und on
/*N*/ 			// an der Position nicht schon ein Attribut ohne Ende gesetzt ist!
/*N*/ 			if( '\xff' != rText8.GetChar(nStart-nOffset) )
/*N*/ 			{
/*N*/ 				nWhich = 0;
/*N*/ 				ASSERT( !this, "TextAttribut ohne Ende ohne 0xFF" );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				sal_Unicode cReplace = 0;
/*N*/ 				switch( nWhich )
/*N*/ 				{
/*N*/ 				case RES_TXTATR_TOXMARK:
/*N*/ 					{
/*N*/ 						// pruefe mal auf doppelte:
/*N*/ 						SwTxtAttr* pAttr = rNd.GetTxtAttr( nStart, nWhich );
/*N*/ 						if( pAttr )
/*N*/ 						{
/*N*/ 							nWhich = 0;
/*N*/ 							ASSERT( !this, "TOXMark ohne Ende doppelt" );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case RES_TXTATR_SOFTHYPH:
/*N*/ 					// set the unicode character into the node text
/*N*/ 					cReplace = CHAR_SOFTHYPHEN;
/*N*/ 					break;
/*N*/ 
/*N*/ 				case RES_TXTATR_HARDBLANK:
/*N*/ 					cReplace = ((SwFmtHardBlank*)pItem)->GetChar();
/*N*/ 					if( ' ' == cReplace )
/*N*/ 						cReplace = CHAR_HARDBLANK;
/*N*/ 					else if( '-' == cReplace )
/*N*/ 						cReplace = CHAR_HARDHYPHEN;
/*N*/ 					else
/*N*/ 						cReplace = 0;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				if( cReplace )
/*N*/ 				{
/*N*/ 					rNd.Replace( SwIndex( &rNd, nStart ), cReplace );
/*N*/ 			 		nWhich = 0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( nWhich )
/*N*/ 		{
/*N*/ 			SwTxtAttr* pAttr = rNd.Insert( *pItem, nStart, nEnd, SETATTR_NOTXTATRCHR );
/*N*/ 			// Sonderbehandlung fuer einige Text-Attribute:
/*N*/ 			if( pAttr && RES_TXTATR_FLYCNT == nWhich )
/*N*/ 				// ein zeichengebundener FlyFrm muss noch verankert werden
/*N*/ 				((SwTxtFlyCnt*) pAttr)->SetAnchor( &rNd );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	delete pItem;
/*N*/ }

// Schreiben aller harten Attributierungen

/*N*/ void Sw3IoImp::OutTxtAttrs( const SwTxtNode& rNd, xub_StrLen nStart,
/*N*/ 						    xub_StrLen nEnd )
/*N*/ {
/*N*/ 	USHORT nCntAttr = rNd.HasHints() ? rNd.GetSwpHints().Count() : 0;
/*N*/ 	if( nCntAttr )
/*N*/ 	{
/*N*/ 		for( USHORT n = 0; n < nCntAttr; n++ )
/*N*/ 		{
/*N*/ 			const SwTxtAttr* pHt = rNd.GetSwpHints()[ n ];
/*N*/ 			BOOL   bHtEnd   = BOOL( pHt->GetEnd() != NULL );
/*N*/ 			xub_StrLen nHtStart = *pHt->GetStart();
/*N*/ 			xub_StrLen nHtEnd   = *pHt->GetAnyEnd();
/*N*/ 
/*N*/ 			// MIB 11.11.96: Der Bereich des Hints muss sich nur irgendwie
/*N*/ 			// mit dem auszugenden Bereich ueberschneiden
/*N*/ 			if( (bHtEnd && nHtEnd > nStart && nHtStart < nEnd) ||
/*N*/ 				(!bHtEnd && nHtStart >= nStart && nHtStart < nEnd ) )
/*N*/ 			{
/*N*/ 				// Der Hint liegt zumindest teilweise im Text, also
/*N*/ 				// Start und Ende korrigieren und Hint ausgeben
/*N*/ 				nHtStart = ( nHtStart < nStart ) ? 0 : ( nHtStart - nStart );
/*N*/ 				nHtEnd   = ( nHtEnd > nEnd ? nEnd : nHtEnd ) - nStart;
/*N*/ 				const SfxPoolItem& rAttr = pHt->GetAttr();
/*N*/ 				OutAttr( rAttr, nHtStart, nHtEnd );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::ExportTxtAttrs( const Sw3ExportTxtAttrs* pInfo,
/*N*/ 							   xub_StrLen nStart, xub_StrLen nEnd )
/*N*/ {
/*N*/ 	USHORT nINetFmtCnt = 0;
/*N*/ 	for( USHORT n = 0; n < pInfo->aItems.Count(); n++ )
/*N*/ 	{
/*N*/ 		xub_StrLen nHtStart = pInfo->aItemStarts[n];
/*N*/ 		xub_StrLen nHtEnd   = pInfo->aItemEnds[n];;
/*N*/ 
/*N*/ 		// Der Hint liegt zumindest teilweise im Text, also
/*N*/ 		// Start und Ende korrigieren und Hint ausgeben
/*N*/ 		nHtStart = ( nHtStart < nStart ) ? 0 : ( nHtStart - nStart );
/*N*/ 		nHtEnd   = ( nHtEnd > nEnd ? nEnd : nHtEnd ) - nStart;
/*N*/ 		const SfxPoolItem* pAttr = pInfo->aItems[n];
/*N*/ 		if( RES_TXTATR_INETFMT==pAttr->Which() )
/*N*/ 		{
/*N*/ 			// ein SwFmtINetFmtNet muss bei SW31-Export zum Feld werden
/*N*/ 
/*N*/ 			// Start OutAttr()
/*N*/ 			OpenRec( SWG_ATTRIBUTE );
/*N*/ 			BYTE cFlags = 0x04;			// Which + Version
/*N*/ 			cFlags += 0x12;				// Begin
/*N*/ 			USHORT nWhich = RES_TXTATR_FIELD -	RES_TXTATR_NOEND_BEGIN + 0x3000;
/*N*/ 			*pStrm  << (BYTE) cFlags
/*N*/ 					<< (UINT16) nWhich
/*N*/ 					<< (UINT16) 0		   		// rAttr.GetVersion();
/*N*/ 					<< (UINT16)nHtStart;
/*N*/ 
/*N*/ 			// Start SwFmtFld::Store()
/*N*/ 
/*N*/ 			// Start OutField()
/*N*/ 			*pStrm << (INT16) ( RES_INTERNETFLD - RES_FIELDS_BEGIN )
/*N*/ 				   << (INT16) 0;
/*N*/ 
/*N*/ 			// Start lcl_sw3io_OutINetField()
/*N*/ 			OutString( *pStrm, ::binfilter::StaticBaseUrl::AbsToRel(
/*N*/ 						((const SwFmtINetFmt *)pAttr)->GetValue() URL_DECODE ) );
/*N*/ 			pStrm->WriteByteString( *pInfo->aINetFmtTexts[nINetFmtCnt] );
/*N*/ 			// Ende lcl_sw3io_OutINetField()
/*N*/ 
/*N*/ 			// Ende OutField()
/*N*/ 
/*N*/ 			// Ende SwFmtFld::Store()
/*N*/ 
/*N*/ 			CloseRec( SWG_ATTRIBUTE );
/*N*/ 			// Ende OutAttr()
/*N*/ 
/*N*/ 			nINetFmtCnt++;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			OutAttr( *pAttr, nHtStart, nHtEnd );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


// Die Formate von Grafik- und OLE-Nodes muessen nicht registriert
// werden; die Layout-Frames erhalten ja eine Node-Referenz.

//#define SWG_GRAPHIC_EXT 'X' jetzt SWG_IMAGEMAP

/*N*/ void Sw3IoImp::InGrfNode( SwNodeIndex& rPos )
/*N*/ {
/*N*/ 	Graphic aGrf;
/*N*/ 	Graphic* pGrf = &aGrf;
/*N*/ 	String aGrfName, aFltName, aStrmName, aURL, aTarget, aAltText;
/*N*/ 	ImageMap *pImgMap = 0;
/*N*/ 	PolyPolygon *pContour = 0;
/*N*/ 	OpenRec( SWG_GRFNODE );
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 	CloseFlagRec();
/*N*/ 	BOOL bLink = BOOL( ( cFlags & 0x10 ) == 0 );
/*N*/ 	BOOL bEmptyGrf = BOOL( cFlags & 0x20 );
/*N*/ 	BOOL bIsServerMap = BOOL( (cFlags & 0x40) != 0 );
/*N*/ 
/*N*/ 	InString( *pStrm, aGrfName );
/*N*/ 	InString( *pStrm, aFltName );
/*N*/ 
/*N*/ 	if( IsVersion( SWG_DESKTOP40 ) )
/*N*/ 		InString( *pStrm, aAltText );
/*N*/ 
/*N*/ 	aStrmName = aGrfName;
/*N*/ 	SwAttrSet aSet( pDoc->GetAttrPool(), aNoTxtNodeSetRange );
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		BYTE cType = Peek();
/*N*/ 		switch( cType )
/*N*/ 		{
/*N*/ 			case SWG_ATTRSET:
/*N*/ 				InAttrSet( aSet );
/*N*/ 				break;
/*N*/ 			case SWG_IMAGEMAP:
/*N*/ 				{
/*N*/ 					BOOL bDummy; // IsURL-Map-Flag wird im Node selbst gesp.
/*N*/ 					pImgMap = InImageMap( aURL, aTarget, bDummy );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 			case SWG_CONTOUR:
                    pContour = InContour();
/*N*/ 				break;
/*N*/ 
/*N*/ 			default:
/*?*/ 				SkipRec();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	CloseRec( SWG_GRFNODE );
/*N*/ 	if( CheckPersist() )
/*N*/ 	{
/*N*/ 		// Muss die Grafik noch geladen werden?
/*N*/ 		if( !bEmptyGrf )
/*N*/ 		{
/*N*/ 			if( bLink )
/*N*/ 			{
/*N*/ 				pGrf = 0;
/*N*/               if( aGrfName.Len() )
/*N*/ 			aGrfName = ::binfilter::StaticBaseUrl::RelToAbs( aGrfName );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SvStorageRef pPicStg = pRoot->OpenStorage( N_PICTURES,
/*N*/ 							STREAM_READ | STREAM_SHARE_DENYWRITE, 0 );
/*N*/ 				SvStorageStreamRef pPicStrm;
/*N*/ 				if( pPicStg.Is() )
/*N*/ 					pPicStrm = pPicStg->OpenStream
/*N*/ 						( aGrfName, STREAM_READ | STREAM_SHARE_DENYWRITE );
/*N*/ 				aGrfName.Erase();	// Ist ja gar kein Grafikname!
/*N*/ 				ASSERT( pPicStrm.Is() && pPicStrm->GetError() == SVSTREAM_OK, "Grafik nicht gefunden" );
/*N*/ 				if( pPicStrm.Is() && pPicStrm->GetError() == SVSTREAM_OK )
/*N*/ 				{
/*N*/ 					// Wenn kein DocFileName gesetzt ist, wird eine TmpFile
/*N*/ 					// erzeugt, was wir im Moment nicht wollen!
/*N*/ 					pPicStrm->SetVersion( pRoot->GetVersion() );
/*N*/ 					String aDummy( String::CreateFromAscii("file:///Dummy") );
/*N*/ 					aGrf.SetDocFileName( aDummy, 0L );
/*N*/ 					BOOL bSwapOut = BOOL( !bInsert );
/*N*/ 					// Beim SW3.1-Import verliern wir unserden Dok-Storage,
/*N*/ 					// also darf die Grafik nicht rausgeswappt werden.
/*N*/ 					// MIB 3.9.98: Ausserdem muessen Grafiken immer beim
/*N*/ 					// LoadStyles reingeswappt werden, weil dann der Storage
/*N*/ 					// aus dem geladen wird nicht unbedingt der Doc-Storage
/*N*/ 					// sein muss. Das ist zum Beispiel beim Aktualisieren von
/*N*/ 					// Vorlagen aus einer Dokument-Vorlage der Fall (#55896#)
/*N*/ 					// Aufgrund eines Hackks im Organizer stimmt dort der
/*N*/ 					// Dok-Storage uebrigens ...
/*N*/ 					if( bBlock || bOrganizer || nVersion < SWG_MAJORVERSION )
/*N*/ 						bSwapOut = FALSE;
/*N*/ 					aGrf.ReadEmbedded( *pPicStrm, bSwapOut );
/*N*/ 					aDummy.Erase();
/*N*/ 					aGrf.SetDocFileName( aDummy, 0L );
/*N*/ 					if( pPicStrm->GetError() != SVSTREAM_OK )
/*N*/ 						Error( ERR_SWG_READ_ERROR );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					Warning( WARN_SWG_POOR_LOAD );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( !nRes )
/*N*/ 		{
/*N*/ 			if( !IsVersion( SWG_URLANDMAP, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 			{
/*N*/ 				// bei importierten Dateien muss ggf noch die URL in das
/*N*/ 				// Format gestopft werden
/*N*/ 				SwFmtURL aFmtURL;
/*N*/ 				aFmtURL.SetURL( aURL, bIsServerMap );
/*N*/ 				aFmtURL.SetTargetFrameName( aTarget );
/*N*/ 				if( pImgMap )
/*?*/ 					aFmtURL.SetMap( pImgMap );
/*N*/ 				aSet.Put( aFmtURL );
/*N*/ 			}
/*N*/ 			SwGrfNode* pNd = pDoc->GetNodes().MakeGrfNode( rPos,
/*N*/ 									  aGrfName, aFltName, pGrf,
/*N*/ 									  (SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl(),
/*N*/ 									  &aSet, bLink );
/*N*/ 			if( !bLink && !bBlock && !bInsert && !bOrganizer )
/*N*/ 				pNd->SetStreamName( aStrmName );
/*N*/ 			pNd->SetAlternateText( aAltText );
/*N*/ 			pNd->SetContour( pContour );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	delete pImgMap;
/*N*/ 	delete pContour;
/*N*/ }

// Ausgabe eines Grafik-Nodes

/*N*/ void Sw3IoImp::OutGrfNode( const SwNoTxtNode& rNode )
/*N*/ {
/*N*/ 	if( CheckPersist() )
/*N*/ 	{
/*N*/ 		SwGrfNode& rGrf = (SwGrfNode&) rNode;
/*N*/ 
/*N*/ 		String aName, sFilterNm;
/*N*/ 		BYTE cFlags = 0x00;
/*N*/ 		if( !rGrf.IsGrfLink() )		// gelinkte Graphic
/*N*/ 		{
/*N*/ 			cFlags = 0x10;
/*N*/ 			if( GRAPHIC_NONE == rGrf.GetGrf().GetType() )
/*N*/ 				cFlags |= 0x20;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// Falls die Grafik bereits im Storage ist, ist der Stream-Name
/*N*/ 				// gesetzt. Dann brauchen wir sie nicht mehr zu speichern.
/*N*/ 				// oder es ist ein SaveAs, dann auf jedenfall kopieren
/*N*/ 				if( !rGrf.StoreGraphics( pRoot ) )
/*N*/ 				{
/*N*/ 					Warning( WARN_SWG_POOR_LOAD );
/*N*/ 					cFlags |= 0x20;		// dann als leere Grf kennzeichnen!
/*N*/ //					Error( ERR_SWG_WRITE_ERROR );
/*N*/ 				}
/*N*/ 				// Den Namen merken
/*N*/ 				else
/*N*/ 					aName = rGrf.GetStreamName();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nFileFlags |= SWGF_HAS_GRFLNK;
/*N*/ 			rGrf.GetFileFilterNms( &aName, &sFilterNm );
/*N*/ 			aName = ::binfilter::StaticBaseUrl::AbsToRel( aName );
/*N*/ 		}
/*N*/ 
/*N*/ 		// Beim 31-Export muss die URL noch am Node gespeichert werden
/*N*/ 		const SfxPoolItem *pURLItem = 0;
/*N*/ 		if( IsSw31Export() &&
/*N*/ 			SFX_ITEM_SET == rNode.GetFlyFmt()->GetAttrSet().
/*N*/ 									GetItemState( RES_URL, FALSE, &pURLItem ) )
/*N*/ 		{
/*N*/ 			if ( ((SwFmtURL*)pURLItem)->IsServerMap() )
/*N*/ 				cFlags |= 0x40;
/*N*/ 		}
/*N*/ 
/*N*/ 		OpenRec( SWG_GRFNODE );
/*N*/ 		*pStrm << cFlags;
/*N*/ 		OutString( *pStrm, aName );
/*N*/ 		OutString( *pStrm, sFilterNm );
/*N*/ 		if( !IsSw31Export() )
/*N*/ 			OutString( *pStrm, rGrf.GetAlternateText() );
/*N*/ 		if( rNode.GetpSwAttrSet() )
/*N*/ 			OutAttrSet( *rNode.GetpSwAttrSet() );
/*N*/ 
/*N*/ 		if( pURLItem )
/*N*/ 		{
/*N*/ 			const String& rURL = ((SwFmtURL*)pURLItem)->GetURL();
/*N*/ 			const String& rTarget = ((SwFmtURL*)pURLItem)->GetTargetFrameName();
/*N*/ 			const ImageMap *pIMap = ((SwFmtURL*)pURLItem)->GetMap();
/*N*/ 			if( rURL.Len() || rTarget.Len() || pIMap || (cFlags & 0x40) )
/*N*/ 				OutImageMap( rURL, rTarget, pIMap, (cFlags & 0x40) );
/*N*/ 		}
/*N*/ 
/*N*/ 		// wegen der while( BytesLeft() )-Schleife beim Einlesen brauchen
/*N*/ 		// wir hier einen eigenen Record, der aber auch fuer andere Sachen
/*N*/ 		// verwendet werden kann und sollte
/*N*/ 		if( !IsSw31Export() && rNode.HasContour() )
                OutContour( *rNode.HasContour() );
/*N*/ 
/*N*/ 		CloseRec( SWG_GRFNODE );
/*N*/ 		aStat.nGrf++;
/*N*/ 	}
/*N*/ }

// Einlesen eines OLE-Nodes

/*N*/ void Sw3IoImp::InOLENode( SwNodeIndex& rPos )
/*N*/ {
/*N*/ 	SwOLENode* pOLENd = 0;
/*N*/ 	SwNoTxtNode* pNoTxtNd = 0;
/*N*/ 	String aObjName, aAltText;
/*N*/ 
/*N*/ 	OpenRec( SWG_OLENODE );
/*N*/ 	InString( *pStrm, aObjName );
/*N*/ 
/*N*/ 	if( IsVersion( SWG_DESKTOP40 ) )
/*N*/ 		InString( *pStrm, aAltText );
/*N*/ 
/*N*/ 	// change the StarImageObj to a graphic
/*N*/ 	SvPersistRef xSrcDoc( new SvPersist() );
/*N*/ 	SvInfoObjectRef xObjInfo;
/*N*/ 	if( xSrcDoc->DoOwnerLoad( pRoot ) && xSrcDoc->GetObjectList() )
/*N*/ 	{
/*N*/ 		// Suche die richtige Info
/*N*/ 		xObjInfo = xSrcDoc->Find( aObjName );
/*N*/ 		ASSERT( xObjInfo.Is(), "Keine Objektinfo zum Einfuegen gefunden" );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( xObjInfo.Is() )
/*N*/ 	{
/*N*/ 		SvStorageRef xSimStg( pRoot->OpenStorage(
/*N*/ 											xObjInfo->GetStorageName() ) );
/*N*/ 		String aStmName;
/*N*/ 		if( xSimStg.Is() && (
/*N*/ 			xSimStg->IsStream( aStmName = String(
/*N*/ 					RTL_CONSTASCII_USTRINGPARAM( "StarImageDocument" )) ) ||
/*N*/ 			xSimStg->IsStream( aStmName = String(
/*N*/ 					RTL_CONSTASCII_USTRINGPARAM( "StarImageDocument 4.0" ))) ))
/*N*/ 		{
/*N*/ 			SvStorageStreamRef xSimStm( xSimStg->OpenStream( aStmName ) );
/*N*/ 
/*N*/ 			if( xSimStm.Is() && !xSimStm->GetError() )
/*N*/ 			{
/*N*/ 				Graphic aGraphic;
/*N*/ 				xSimStm->SetBufferSize( 32768 );
/*N*/ 				xSimStm->SetKey( xSimStg->GetKey() );
/*N*/ 				*xSimStm >> aGraphic;
/*N*/ 				xSimStm->SetBufferSize( 0 );
/*N*/ 
/*N*/ 				pNoTxtNd = pDoc->GetNodes().MakeGrfNode( rPos, aEmptyStr,
/*N*/ 						aEmptyStr, &aGraphic,
/*N*/ 						(SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !pNoTxtNd && CheckPersist() )
/*N*/ 	{
/*N*/ 		// Im Insert Mode muss das OLE-Objekt in den Ziel-Storage kopiert werden
/*N*/ 		if( bInsert && xObjInfo.Is() )
/*N*/ 		{
/*?*/ 			SvPersistRef rpDstDoc( pDoc->GetPersist() );
/*?*/ 
/*?*/ 			String aStgName( xObjInfo->GetStorageName() );
/*?*/ 			SvStorageRef rpDst( pDoc->GetPersist()->GetStorage() );
/*?*/ 			// Sind Objektname und Storagename eindeutig?
/*?*/ 			if( rpDstDoc->GetObjectList() )
/*?*/ 				for( ULONG i = 0; i < rpDstDoc->GetObjectList()->Count(); i++ )
/*?*/ 				{
/*?*/ 					SvInfoObject* pTst = rpDstDoc->GetObjectList()->GetObject(i);
/*?*/ 					// TODO: unicode: is this correct?
/*?*/ 					if( aObjName.EqualsIgnoreCaseAscii( pTst->GetObjName() ))
/*?*/ 						aObjName = Sw3Io::UniqueName( rpDst, "Obj" );
/*?*/ 					// TODO: unicode: is this correct?
/*?*/ 					if( aStgName.EqualsIgnoreCaseAscii( pTst->GetStorageName() ) )
/*?*/ 						aStgName = Sw3Io::UniqueName( rpDst, "Obj" );
/*?*/ 				}
/*?*/ 			if( !rpDstDoc->Copy( aObjName, aStgName, &xObjInfo, xSrcDoc ) )
/*?*/ 			{
/*?*/ 				Error( ERR_SWG_READ_ERROR );
/*?*/ 				return;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		pNoTxtNd = pOLENd = pDoc->GetNodes().MakeOLENode( rPos, aObjName,
/*N*/ 						(SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl() );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pNoTxtNd )
/*N*/ 	{
/*N*/ 		pNoTxtNd->SetAlternateText( aAltText );
/*N*/ 
/*N*/ 		while( BytesLeft() )
/*N*/ 		{
/*N*/ 			BYTE cType = Peek();
/*N*/ 			switch( cType )
/*N*/ 			{
/*?*/ 			case SWG_ATTRSET:
/*?*/ 				if( !pNoTxtNd->GetpSwAttrSet() )
/*?*/ 					((SwCntntNode*) pNoTxtNd)->NewAttrSet( pDoc->GetAttrPool() );
/*?*/ 				InAttrSet( *pNoTxtNd->GetpSwAttrSet() );
/*?*/ 				pNoTxtNd->GetpSwAttrSet()->SetModifyAtAttr( pNoTxtNd );
/*?*/ 				break;
/*N*/ 
/*N*/ 			case SW_OLE_CHARTNAME:
/*N*/ 				if( pOLENd )
/*N*/ 				{
/*N*/ 					String sStr;
/*N*/ 					OpenRec( SW_OLE_CHARTNAME );
/*N*/ 					InString( *pStrm, sStr );
/*N*/ 					CloseRec( SW_OLE_CHARTNAME );
/*N*/ 					pOLENd->SetChartTblName( sStr );
/*N*/ 				}
/*N*/ 				else
/*?*/ 					SkipRec();
/*N*/ 				break;
/*N*/ 
/*?*/ 			case SWG_IMAGEMAP:
/*?*/ 				{
/*?*/ 					String aURL, aTarget;
/*?*/ 					BOOL bIsServerMap = FALSE;
/*?*/ 					ImageMap *pImgMap = InImageMap( aURL, aTarget, bIsServerMap);
/*?*/ 					SwFmtURL aFmtURL;
/*?*/ 					aFmtURL.SetURL( aURL, bIsServerMap );
/*?*/ 					aFmtURL.SetTargetFrameName( aTarget );
/*?*/ 					if ( pImgMap )
/*?*/ 					{
/*?*/ 						aFmtURL.SetMap( pImgMap );
/*?*/ 						delete pImgMap;
/*?*/ 					}
/*?*/ 					pNoTxtNd->SetAttr( aFmtURL );
/*?*/ 				}
/*?*/ 				break;
/*?*/ 
/*?*/ 			case SWG_CONTOUR:
/*?*/ 				if( pOLENd )
/*?*/ 				{
                        PolyPolygon *pContour = InContour();
 /*?*/                  pOLENd->SetContour( pContour );
 /*?*/                  delete pContour;
/*?*/ 				}
/*?*/ 				else
/*?*/ 					SkipRec();
/*?*/ 				break;
/*N*/ 
/*N*/ 			default:
/*N*/ 				SkipRec();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		// falls ein 3.0-Dokument gelesen wird: Node merken
/*N*/ 		if( pOLENd )
/*N*/ 		{
/*N*/ 			if( bNormal && !bInsert && !bBlock && nVersion<=SWG_SHORTFIELDS )
/*N*/ 			{
/*?*/ 				if( !p30OLENodes )
/*?*/ 					p30OLENodes = new SwOLENodes;
/*?*/ 
/*?*/ 				p30OLENodes->Insert( pOLENd, p30OLENodes->Count() );
/*N*/ 			}
/*N*/ 
/*N*/ 			if( bInsert )
/*?*/ 				pOLENd->SetOLESizeInvalid( TRUE );	//wg. Druckerwechsel
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( !xObjInfo.Is() )
/*?*/ 		Warning( WARN_SWG_POOR_LOAD );
/*N*/ 		
/*N*/ 	CloseRec( SWG_OLENODE );
/*N*/ }

// Ausgabe eines OLE-Nodes

/*N*/ void Sw3IoImp::OutOLENode( const SwNoTxtNode& rNd )
/*N*/ {
/*N*/ 	if( CheckPersist() )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_OLENODE );
/*N*/ 		SwOLENode& rNode = (SwOLENode&) rNd;
/*N*/ 		SwOLEObj& rObj = rNode.GetOLEObj();
/*N*/ 
/*N*/ 		String aName( rObj.GetName() );
/*N*/ 		OutString( *pStrm, aName );
/*N*/ 		if( !IsSw31Export() )
/*?*/ 			OutString( *pStrm, rNode.GetAlternateText() );
/*N*/ 		if( rNode.GetpSwAttrSet() )
/*N*/ 			OutAttrSet( *rNode.GetpSwAttrSet() );
/*N*/ 
/*N*/ 		if( rNode.GetChartTblName().Len() )
/*N*/ 		{
/*N*/ 			OpenRec( SW_OLE_CHARTNAME );
/*N*/ 			OutString( *pStrm, rNode.GetChartTblName() );
/*N*/ 			CloseRec( SW_OLE_CHARTNAME );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( !IsSw31Export() && rNode.HasContour() )
                OutContour( *rNode.HasContour() );
/*N*/ 
/*N*/ 		CloseRec( SWG_OLENODE );
/*N*/ 		aStat.nOLE++;
/*N*/ 	}
/*N*/ }


// Einlesen eines Text-Wiederholungs-Nodes

/*N*/ void Sw3IoImp::InRepTxtNode( SwNodeIndex& rPos )
/*N*/ {
/*N*/ 	UINT32 nRepetitions;
/*N*/ 
/*N*/ 	OpenRec( SWG_REPTEXTNODE );
/*N*/ 	*pStrm >> nRepetitions;
/*N*/ 
/*N*/ 	rPos--;
/*N*/ 	SwTxtNode *pNode = pDoc->GetNodes()[rPos]->GetTxtNode();
/*N*/ 	rPos++;
/*N*/ 
/*N*/ 	for( ULONG i=0; i<nRepetitions; i++ )
/*N*/ 		pNode->MakeCopy( pDoc, rPos );
/*N*/ 
/*N*/ 	CloseRec( SWG_REPTEXTNODE );
/*N*/ }
/*N*/ 
/*N*/ 
/*N*/ // Ausgabe eines Text-Wiederholungs-Nodes
/*N*/ 
/*N*/ void Sw3IoImp::OutRepTxtNode( ULONG nRepetitions )
/*N*/ {
/*N*/ 	OpenRec( SWG_REPTEXTNODE );
/*N*/ 	*pStrm << (UINT32)nRepetitions;
/*N*/ 	CloseRec( SWG_REPTEXTNODE );
/*N*/ }

// Der Image-Map-Record war frueher ein SWG_GRAPHIC_EXT-Record.
// Deshalb enthaelt er immer der URL fuer eine Server-seitige
// Image-Map und kein ismap-Flag! Aus dem gleichen Grund wird die
// URL fuer eine Client-seitige Image-Map ueber ein Falg gesteuert.
// damit alte Writer-Version keine Warnung ausgeben, wenn der String
// leer ist.

/*N*/ ImageMap *Sw3IoImp::InImageMap( String& rURL, String& rTarget, BOOL& rIsMap )
/*N*/ {
/*N*/ 	OpenRec( SWG_IMAGEMAP );
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	rIsMap = BOOL( (cFlags & 0x10) != 0 );
/*N*/ 
/*N*/ 	InString( *pStrm, rURL );
/*N*/ 	if( rURL.Len() )
/*N*/ 		rURL = ::binfilter::StaticBaseUrl::SmartRelToAbs( rURL );
/*N*/ 
/*N*/ 	// bis hier hatten wir frueher einen SWG_GRAPHIC_EXT-Record!
/*N*/ 	if( IsVersion( SWG_TARGETFRAME, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*N*/ 		String sDummy;
/*N*/ 		InString( *pStrm, rTarget );
/*N*/ 		InString( *pStrm, sDummy );
/*N*/ 	}
/*N*/ 
/*N*/ 	ImageMap *pIMap = 0;
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 	{
/*?*/ 		pIMap = new ImageMap;
/*?*/ 		pIMap->Read(
                *pStrm,
                ::binfilter::StaticBaseUrl::GetBaseURL(INetURLObject::NO_DECODE));
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_IMAGEMAP );
/*N*/ 
/*N*/ 	return pIMap;	// muss ggf. vom Aufrufer geloescht werden!
/*N*/ }

/*N*/ void lcl_sw3io__ConvertMarkToOutline( String& rURL )
/*N*/ {
/*N*/ 	if( rURL.Len() && '#' == rURL.GetChar( 0 ) )
/*N*/ 	{
/*N*/ 		String sCmp, sMark( INetURLObject::decode( rURL, INET_HEX_ESCAPE,
/*N*/ 										INetURLObject::DECODE_WITH_CHARSET,
/*N*/ 										RTL_TEXTENCODING_UTF8 ));
/*N*/ 		xub_StrLen nPos = sMark.SearchBackward( cMarkSeperator );
/*N*/ 		if( STRING_NOTFOUND != nPos &&
/*N*/ 			( sCmp = sMark.Copy( nPos + 1 ) ).EraseAllChars().Len() &&
/*N*/ 			COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToOutline ) )
/*N*/ 		{
/*N*/ 			rURL = '#';
/*N*/ 			rURL += String(INetURLObject::createFragment( sMark.Copy( 1, nPos-1 ) ));
/*N*/ 			rURL += cMarkSeperator;
/*N*/ 			rURL.AppendAscii( pMarkToOutline );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::OutImageMap( const String& rURL, const String& rTarget,
/*N*/ 							const ImageMap *pIMap, BOOL bIsServerMap )
/*N*/ {
/*N*/ 	// Dieser Record ist fuer den 31-Export ein SWG_GRAPHIC_EXT und
/*N*/ 	// enthaelt dann nur eine URL
/*N*/ 	OpenRec( SWG_IMAGEMAP );
/*N*/ 	BYTE cFlags = 0x00;
/*N*/ 	if( !IsSw31Export() && bIsServerMap )
/*N*/ 		cFlags += 0x10;	// es ist eine Image-Map
/*N*/ 	if( !IsSw31Export() && pIMap )
/*N*/ 		cFlags += 0x20;	// es folgt eine Image-Map
/*N*/ 
/*N*/ 	*pStrm << cFlags;
/*N*/ 
/*N*/ 	// Unabhaengigkeit von der AbsToRel-Schnittstelle sicherstellen!
/*N*/ 	String aURL( rURL );
/*N*/ 	if( aURL.Len() )
/*N*/ 	{
/*N*/ 		lcl_sw3io__ConvertMarkToOutline( aURL );
/*N*/ 		aURL = ::binfilter::StaticBaseUrl::AbsToRel( aURL URL_DECODE);
/*N*/ 	}
/*N*/ 	OutString( *pStrm, aURL );
/*N*/ 
/*N*/ 	// bis hier hatten wir frueher einen SWG_GRAPHIC_EXT-Record!
/*N*/ 
/*N*/ 	if( !IsSw31Export() )
/*N*/ 	{
/*N*/ 		OutString( *pStrm, rTarget );
/*N*/ 		OutString( *pStrm, aEmptyStr );
/*N*/ 
/*N*/ 		if( pIMap )
/*?*/ 		  pIMap->Write(
                *pStrm,
                ::binfilter::StaticBaseUrl::GetBaseURL(INetURLObject::NO_DECODE));
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_IMAGEMAP );
/*N*/ }

 PolyPolygon *Sw3IoImp::InContour()
 {
    PolyPolygon *pContour = 0;
 
    OpenRec( SWG_CONTOUR );
    BYTE cFlags = OpenFlagRec();
    CloseFlagRec();
 
    if( (cFlags & 0x10) != 0 )
    {
        pContour = new PolyPolygon;
        *pStrm >> *pContour;
    }
 
    CloseRec( SWG_CONTOUR );
 
    return pContour;
 }

void Sw3IoImp::OutContour( const PolyPolygon& rPoly )
{
   OpenRec( SWG_CONTOUR );

   BYTE cFlags = 0x10; // es folgt ein Contour Poly-Polygon
   *pStrm << cFlags;

   // das Contour-PolyPolygon rausschreiben
   *pStrm << rPoly;

   CloseRec( SWG_CONTOUR );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
