/*************************************************************************
 *
 *  $RCSfile: rtfnum.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:25 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif
#ifndef _RTFKEYWD_HXX //autogen
#include <svtools/rtfkeywd.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _RTFOUT_HXX
#include <svtools/rtfout.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX //autogen
#include <svx/wrlmitem.hxx>
#endif

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWPARRTF_HXX
#include <swparrtf.hxx>
#endif
#ifndef _WRTRTF_HXX
#include <wrtrtf.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif

#define RTF_NUMRULE_NAME "RTF_Num"

SV_IMPL_VARARR( SwListArr, SwListEntry )


void lcl_ExpandNumFmts( SwNumRule& rRule )
{
    // dann noch das NumFormat in alle Ebenen setzen
    for( BYTE n = 1; n < MAXLEVEL; ++n )
        if( !rRule.GetNumFmt( n ) )
        {
            SwNumFmt aNumFmt( rRule.Get( 0 ));
            aNumFmt.SetAbsLSpace( aNumFmt.GetAbsLSpace() * ( n + 1 ) );
            rRule.Set( n, aNumFmt );
        }
}

SfxItemSet& GetNumChrFmt( SwDoc& rDoc, SwNumRule& rRule, BYTE nNumLvl )
{
    SwCharFmt* pChrFmt = rRule.Get( nNumLvl ).GetCharFmt();
    if( !pChrFmt )
    {
        String sNm( rRule.GetName() );
        ( sNm += ' ' ) += String::CreateFromInt32( nNumLvl + 1 );
        pChrFmt = rDoc.MakeCharFmt( sNm, rDoc.GetDfltCharFmt() );
        if( !rRule.GetNumFmt( nNumLvl ))
            rRule.Set( nNumLvl, rRule.Get( nNumLvl ) );
        ((SwNumFmt*)rRule.GetNumFmt( nNumLvl ))->SetCharFmt( pChrFmt );
    }
    return (SfxItemSet&)pChrFmt->GetAttrSet();
}

void SwRTFParser::ReadListLevel( SwNumRule& rRule, BYTE nNumLvl )
{
    int nToken;
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!
    int nLvlTxtLevel = 0, nLvlNumberLevel = 0;
    String sLvlText, sLvlNumber;
    SwNumFmt* pCurNumFmt;
    if( MAXLEVEL >= nNumLvl )
    {
        pCurNumFmt = (SwNumFmt*)rRule.GetNumFmt( nNumLvl );
        pCurNumFmt->SetAbsLSpace( 0 );
        pCurNumFmt->SetFirstLineOffset( 0 );
    }
    else
        pCurNumFmt = 0;

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':
            if( nOpenBrakets )
            {
                if( nLvlTxtLevel == nOpenBrakets )
                {
                    if( DelCharAtEnd( sLvlText, ';' ).Len() &&
                        sLvlText.Len() && sLvlText.Len() ==
                        (USHORT)(sLvlText.GetChar( 0 )) + 1 )
                        sLvlText.Erase( 0, 1 );
                    nLvlTxtLevel = 0;
                }
                if( nLvlNumberLevel == nOpenBrakets )
                {
                    DelCharAtEnd( sLvlNumber, ';' );
                    nLvlNumberLevel = 0;
                }
            }
            --nOpenBrakets;
            break;

        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                // Unknown und alle bekannten nicht ausgewerteten Gruppen
                // sofort ueberspringen
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ))
//                      RTF_PANOSE != nToken && RTF_FALT != nToken &&
//                      RTF_FALT != nToken && RTF_FNAME != nToken &&
//                      RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nOpenBrakets;
            }
            break;

        case RTF_LEVELNFC:
            {
                sal_Int16 eType = SVX_NUM_ARABIC;
                switch( nTokenValue )
                {
                case 1:     eType = SVX_NUM_ROMAN_UPPER;            break;
                case 2:     eType = SVX_NUM_ROMAN_LOWER;            break;
                case 3:     eType = SVX_NUM_CHARS_UPPER_LETTER_N;   break;
                case 4:     eType = SVX_NUM_CHARS_LOWER_LETTER_N;   break;
                case 255:
                case 23:    eType = SVX_NUM_CHAR_SPECIAL;           break;
                }
                if( pCurNumFmt )
                    pCurNumFmt->SetNumberingType(eType);
            }
            break;

        case RTF_LEVELJC:
            {
                SvxAdjust eAdj = SVX_ADJUST_LEFT;
                switch( nTokenValue )
                {
                case 1:     eAdj = SVX_ADJUST_CENTER;   break;
                case 2:     eAdj = SVX_ADJUST_RIGHT;    break;
                }
                if( pCurNumFmt )
                    pCurNumFmt->SetNumAdjust( eAdj );
            }
            break;

        case RTF_LEVELSTARTAT:
            if( pCurNumFmt && -1 != nTokenValue )
                pCurNumFmt->SetStart( USHORT( nTokenValue ));
            break;

        case RTF_LEVELTEXT:
            nLvlTxtLevel = nOpenBrakets;
            break;

        case RTF_LEVELNUMBERS:
            nLvlNumberLevel = nOpenBrakets;
            break;

        case RTF_LI:
            if( pCurNumFmt && -1 != nTokenValue )
            {
                if( 0 > nTokenValue )
                    nTokenValue =- nTokenValue;
                pCurNumFmt->SetAbsLSpace( USHORT( nTokenValue ));
            }
            break;

        case RTF_FI:
            if( pCurNumFmt && -1 != nTokenValue )
            {
                if( 0 < short( nTokenValue ))
                    nTokenValue = - short( nTokenValue );
                pCurNumFmt->SetFirstLineOffset( short( nTokenValue ));
            }
            break;

        case RTF_TEXTTOKEN:
            if( nLvlTxtLevel == nOpenBrakets )
                sLvlText += aToken;
            else if( nLvlNumberLevel == nOpenBrakets )
                sLvlNumber += aToken;
            break;

        case RTF_LEVELOLD:
        case RTF_LEVELPREV:
        case RTF_LEVELPREVSPACE:
        case RTF_LEVELINDENT:
        case RTF_LEVELSPACE:
        case RTF_LEVELFOLLOW:
        case RTF_LEVELLEGAL:
        case RTF_LEVELNORESTART:
            break;

        default:
            if( pCurNumFmt &&
                RTF_CHRFMT == (nToken & ~(0xff | RTF_SWGDEFS) ))
            {
                ReadAttr( nToken, &GetNumChrFmt( *pDoc, rRule, nNumLvl ) );

                // dann aus der Vorlage den Font holen
                if( SVX_NUM_CHAR_SPECIAL == pCurNumFmt->GetNumberingType() )
                    pCurNumFmt->SetBulletFont( FindFontOfItem(
                                pCurNumFmt->GetCharFmt()->GetFont() ) );
            }
            break;
        }
    }

    if( IsParserWorking() && pCurNumFmt )
    {
        // dann erzeuge mal die Pre/Postfix-Strings
        if( sLvlText.Len() &&
            SVX_NUM_CHAR_SPECIAL == pCurNumFmt->GetNumberingType() )
        {
            pCurNumFmt->SetBulletChar( sLvlText.GetChar( 0 ) );
            // dann aus der Vorlage den Font holen
            if( pCurNumFmt->GetCharFmt() )
                pCurNumFmt->SetBulletFont( FindFontOfItem(
                        pCurNumFmt->GetCharFmt()->GetFont() ) );
        }
        else if( sLvlNumber.Len() && sLvlText.Len() )
        {
            // in sLvlText steht der Text, in sLvlNumber die Position
            // der Ebenen in sLvlText
            pCurNumFmt->SetPrefix(
                sLvlText.Copy( 0, USHORT( sLvlNumber.GetChar( 0 ))-1 ));
            pCurNumFmt->SetSuffix( sLvlText.Copy(
                    USHORT( sLvlNumber.GetChar( sLvlNumber.Len()-1 )) ));
            // wieviele Levels stehen im String?
            pCurNumFmt->SetIncludeUpperLevels( (BYTE)sLvlNumber.Len() );
        }
        else if( !sLvlText.Len() )
        {
            pCurNumFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);
            pCurNumFmt->SetSuffix( aEmptyStr );
        }
    }

    SkipToken( -1 );
}

void SwRTFParser::ReadListTable()
{
    int nToken;
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!
    bNewNumList = TRUE;

    USHORT nRuleCount = 0;
    BYTE nNumLvl = 0;
    SwNumRule* pCurRule = 0;
    SwListEntry aEntry;

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':       if( --nOpenBrakets && IsParserWorking() )
                        {
                            // Style konnte vollstaendig gelesen werden,
                            // also ist das noch ein stabiler Status
                            SaveState( RTF_LISTTABLE );
                            if( 1 == nOpenBrakets )
                            {
                                if( aEntry.nListId )
                                    aListArr.Insert( aEntry, aListArr.Count() );
                                aEntry.Clear();
                            }
                        }
                        break;

        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                // Unknown und alle bekannten nicht ausgewerteten Gruppen
                // sofort ueberspringen
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ))
//                      RTF_PANOSE != nToken && RTF_FALT != nToken &&
//                      RTF_FALT != nToken && RTF_FNAME != nToken &&
//                      RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nOpenBrakets;
            }
            break;

        case RTF_LIST:
            {
                if( pCurRule && pCurRule->IsContinusNum() )
                    lcl_ExpandNumFmts( *pCurRule );

                String sTmp( String::CreateFromAscii(
                    RTL_CONSTASCII_STRINGPARAM( RTF_NUMRULE_NAME " 1" )));
                aEntry.nListDocPos = pDoc->MakeNumRule( sTmp );
                pCurRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
                pCurRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, FALSE ) );
                pCurRule->SetAutoRule( FALSE );
                nNumLvl = (BYTE)-1;
            }
            break;

        case RTF_LISTID:            aEntry.nListId = nTokenValue;       break;
        case RTF_LISTTEMPLATEID:    aEntry.nListTemplateId = nTokenValue; break;

        case RTF_LISTRESTARTHDN:
            break;
        case RTF_LISTNAME:
            if( RTF_TEXTTOKEN == GetNextToken() )
            {
                String sTmp( DelCharAtEnd( aToken, ';' ));
                if( sTmp.Len() && !pDoc->FindNumRulePtr( sTmp ))
                    pCurRule->SetName( sTmp );
            }
            SkipGroup();
            break;

        case RTF_LISTSIMPLE:
            pCurRule->SetContinusNum( TRUE );
            break;

        case RTF_LISTLEVEL:
            {
                if( ++nNumLvl < MAXLEVEL )
                    pCurRule->Set( nNumLvl, pCurRule->Get( nNumLvl ));
                ReadListLevel( *pCurRule, nNumLvl );
            }
            break;
        }
    }

    if( pCurRule && pCurRule->IsContinusNum() )
        lcl_ExpandNumFmts( *pCurRule );

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}

BOOL lcl_IsEqual( SwNumRule* pOrigRule, SwNumRule* pRule )
{
    BOOL bRet = 0;
    if( pOrigRule && pRule )
    {
        bRet =  pOrigRule->GetRuleType() == pRule->GetRuleType() &&
                pOrigRule->IsContinusNum() == pRule->IsContinusNum() &&
                pOrigRule->IsAbsSpaces() == pRule->IsAbsSpaces();
        if( bRet )
            for( BYTE n = 0; bRet && n < MAXLEVEL; ++n )
            {
                const SwNumFmt* pOFmt = pOrigRule->GetNumFmt( n ),
                              * pFmt = pRule->GetNumFmt( n );
                if( pFmt && pOFmt )
                {
                    SwCharFmt* pOCFmt = pOFmt->GetCharFmt(),
                             * pCFmt = pFmt->GetCharFmt();
                    if( pOCFmt && pCFmt )
                    {
                        bRet = pCFmt->GetAttrSet() == pOCFmt->GetAttrSet();
                    }
                    else
                        bRet = !pCFmt && !pOCFmt;
                    if( bRet )
                    {
                        ((SwNumFmt*)pOFmt)->SetCharFmt( 0 );
                        ((SwNumFmt*)pFmt)->SetCharFmt( 0 );
                        bRet = *pOFmt == *pFmt;
                        ((SwNumFmt*)pOFmt)->SetCharFmt( pOCFmt );
                        ((SwNumFmt*)pFmt)->SetCharFmt( pCFmt );
                    }
                }
                else
                    bRet = !pFmt && !pOFmt;
            }
    }
    return bRet;
}

void SwRTFParser::ReadListOverrideTable()
{
    int nToken;
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!
    SwListEntry aEntry;
    SwNumRule* pRule = 0, *pOrigRule = 0;
    BYTE nNumLvl = 0;
    BOOL bOverrideFormat = FALSE, bOverrideStart = FALSE;

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':
            if( --nOpenBrakets && IsParserWorking() )
            {
                // Style konnte vollstaendig gelesen werden,
                // also ist das noch ein stabiler Status
                SaveState( RTF_LISTOVERRIDETABLE );

                if( 1 == nOpenBrakets )
                {
                    bOverrideFormat = FALSE, bOverrideStart = FALSE;
                    if( pRule )
                    {
                        if( lcl_IsEqual( pOrigRule, pRule ))
                        {
                            // no changes on the rule -> use the original rule
                            aEntry.nListDocPos = pDoc->FindNumRule(
                                                    pOrigRule->GetName() );
                            // delete the temp Rule
                            RemoveUnusedNumRule( pRule );
                        }
                        else if( pRule->IsContinusNum() )
                            lcl_ExpandNumFmts( *pRule );
                    }

                    if( aEntry.nListId && aEntry.nListNo )
                        for( USHORT n = aListArr.Count(); n; )
                            if( aListArr[ --n ].nListId == aEntry.nListId &&
                                !aListArr[ n ].nListNo )
                            {
                                aListArr[ n ].nListNo = aEntry.nListNo;
                                if( pOrigRule )
                                    aListArr[ n ].nListDocPos = aEntry.nListDocPos;
                                break;
                            }
                    aEntry.Clear();
                    pOrigRule = 0;
                    pRule = 0;
                }
            }
            break;

        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                // Unknown und alle bekannten nicht ausgewerteten Gruppen
                // sofort ueberspringen
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ))
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nOpenBrakets;
            }
            break;

        case RTF_LISTOVERRIDE:      aEntry.Clear();                 break;
        case RTF_LISTID:            aEntry.nListId = nTokenValue;   break;
        case RTF_LS:                aEntry.nListNo = nTokenValue;   break;
        case RTF_LISTOVERRIDECOUNT:
            if( nTokenValue )
            {
                pRule = 0;
                // dann erzeugen wir mal schnell eine Kopie von der NumRule,
                // denn diese wird jetzt mal kurz veraendert.
                if( aEntry.nListId )
                    for( USHORT n = 0; n < aListArr.Count(); ++n )
                        if( aListArr[ n ].nListId == aEntry.nListId )
                        {
                            pRule = pDoc->GetNumRuleTbl()[
                                                aListArr[ n ].nListDocPos ];
                            pOrigRule = pRule;

                            String sTmp( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( RTF_NUMRULE_NAME " 1" )));
                            aEntry.nListDocPos = pDoc->MakeNumRule( sTmp, pRule );
                            pRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
                            pRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, FALSE ) );
                            pRule->SetAutoRule( FALSE );
                            nNumLvl = (BYTE)-1;
                            aListArr.Insert( aEntry, aListArr.Count() );
                            break;
                        }

            }
            break;

        case RTF_LISTLEVEL:
            if( pRule && bOverrideFormat )
            {
                if( ++nNumLvl < MAXLEVEL )
                    pRule->Set( nNumLvl, pRule->Get( nNumLvl ));
                ReadListLevel( *pRule, nNumLvl );
            }
            break;

        case RTF_LEVELSTARTAT:
            if( pRule && bOverrideStart )
            {
            }
            break;

        case RTF_LISTOVERRIDESTART:
            bOverrideStart = TRUE;
            break;

        case RTF_LISTOVERRIDEFORMAT:
            bOverrideFormat = TRUE;
            break;

        case RTF_LFOLEVEL:
            // was fehlt noch?
            break;
        }
    }

    // search the outline numrule and set it into the doc
    if( GetStyleTbl().Count() )
    {
        if( !bStyleTabValid )
            MakeStyleTab();

        const SfxPoolItem* pItem;
        const SwTxtFmtColl* pColl;
        SvxRTFStyleType* pStyle = GetStyleTbl().First();
        do {
            if( MAXLEVEL > pStyle->nOutlineNo &&
                0 != ( pColl = aTxtCollTbl.Get( (USHORT)GetStyleTbl().
                                                        GetCurKey() )) &&
                SFX_ITEM_SET == pColl->GetItemState( RES_PARATR_NUMRULE,
                FALSE, &pItem ))
            {
                USHORT nRulePos = pDoc->FindNumRule(
                                ((SwNumRuleItem*)pItem)->GetValue() );
                if( USHRT_MAX != nRulePos )
                {
                    const SwNumRule *pRule = pDoc->GetNumRuleTbl()[ nRulePos ];
                    pDoc->SetOutlineNumRule( *pRule );
                    pDoc->DelNumRule( pRule->GetName() );
                    // now pRule pointer is invalid !!!

                    // now decrement all position in the listtable, which will
                    // behind the doc-rule position
                    for( USHORT n = aListArr.Count(); n; )
                    {
                        SwListEntry& rEntry = aListArr[ --n ];
                        if( rEntry.nListDocPos == nRulePos )
                            aListArr.Remove( n );
                        else if( rEntry.nListDocPos > nRulePos )
                            --rEntry.nListDocPos;
                    }
                    break;
                }
            }

            pStyle->aAttrSet.ClearItem( FN_PARAM_NUM_LEVEL );

        } while( 0 != (pStyle = GetStyleTbl().Next()) );
    }

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}

SwNumRule* SwRTFParser::GetNumRuleOfListNo( long nListNo, BOOL bRemoveFromList )
{
    SwNumRule* pRet = 0;
    SwListEntry* pEntry;
    for( USHORT n = aListArr.Count(); n; )
        if( ( pEntry = &aListArr[ --n ])->nListNo == nListNo )
        {
            if( bRemoveFromList )
                aListArr.Remove( n );
            else
            {
                pEntry->bRuleUsed = TRUE;
                pRet = pDoc->GetNumRuleTbl()[ pEntry->nListDocPos ];
            }
            break;
        }
    return pRet;
}

void SwRTFParser::RemoveUnusedNumRule( SwNumRule* pRule )
{
    if( pRule )
    {
        for ( BYTE nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
        {
            SwNumFmt& rNFmt = (SwNumFmt&)pRule->Get( nLvl );
            SwCharFmt* pCFmt = rNFmt.GetCharFmt();
            if( pCFmt )
            {
                pCFmt->Remove( &rNFmt );
                if( !pCFmt->GetDepends() )
                    pDoc->DelCharFmt( pCFmt );
            }
        }
        pDoc->DelNumRule( pRule->GetName() );
    }
#ifndef PRODUCT
    else
    {
        ASSERT( pRule, "NumRulePointer 0 kann nicht geloescht werden" );
    }
#endif
}

void SwRTFParser::RemoveUnusedNumRules()
{
    SwListEntry* pEntry;
    SvPtrarr aDelArr;
    USHORT n;
    for( n = aListArr.Count(); n; )
        if( !( pEntry = &aListArr[ --n ])->bRuleUsed )
        {
            void * p = pDoc->GetNumRuleTbl()[pEntry->nListDocPos];
            // dont delete named char formats
            if( USHRT_MAX == aDelArr.GetPos( p ) &&
                ((SwNumRule*)p)->GetName().EqualsAscii( RTF_NUMRULE_NAME, 0,
                                sizeof( RTF_NUMRULE_NAME )) )
                aDelArr.Insert( p, aDelArr.Count() );
        }

    for( n = aDelArr.Count(); n; )
    {
        SwNumRule* pDel = (SwNumRule*)aDelArr[ --n ];
        RemoveUnusedNumRule( pDel );
    }
}

const Font* SwRTFParser::FindFontOfItem( const SvxFontItem& rItem ) const
{
    SvxRTFFontTbl& rFntTbl = ((SwRTFParser*)this)->GetFontTbl();
    const Font* pFnt = rFntTbl.First();
    while( pFnt )
    {
        if( pFnt->GetFamily() == rItem.GetFamily() &&
            pFnt->GetName() == rItem.GetFamilyName() &&
            pFnt->GetStyleName() == rItem.GetStyleName() &&
            pFnt->GetPitch() == rItem.GetPitch() &&
            pFnt->GetCharSet() == rItem.GetCharSet() )
            return pFnt;

        pFnt = rFntTbl.Next();
    }
    return 0;
}


void SwRTFParser::ReadNumSecLevel( int nToken )
{
    // lese die \pnseclvl - Gruppe
    // nTokenValue gibt schon den richtigen Level vor 1 - 9!
    BYTE nLevel = 0;
    long nListNo = 0;
    BOOL bContinus = TRUE;

    if( RTF_PNSECLVL == nToken )
    {
        // suche die Rule - steht unter Nummer 3
        nListNo = 3;
        bContinus = FALSE;
        nLevel = MAXLEVEL <= nTokenValue ? MAXLEVEL - 1
                                         : BYTE( nTokenValue - 1 );
    }
    else
    {
        switch( nToken = GetNextToken() )
        {
        case RTF_PNLVL:         nListNo = 3;
                                bContinus = FALSE;
                                nLevel = MAXLEVEL <= nTokenValue
                                                    ? MAXLEVEL - 1
                                                    : BYTE( nTokenValue-1 );
                                break;

        case RTF_PNLVLBODY:     nListNo = 2;    break;
        case RTF_PNLVLBLT:      nListNo = 1;    break;
        case RTF_PNLVLCONT:     nListNo = 4;    break;

        default:
            SkipGroup();
            return ;
        }
    }

    // suche die Rule - steht unter Nummer 3
    USHORT nNewFlag = 1 << nListNo;
    SwNumRule* pCurRule = GetNumRuleOfListNo( nListNo,
                                        0 != ( nNewNumSectDef & nNewFlag ) );
    if( !pCurRule )
    {
        // dann muessen wir die mal anlegen
        nNewNumSectDef &= ~nNewFlag;
        String sTmp( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( RTF_NUMRULE_NAME " 1" )));
        SwListEntry aEntry( nListNo, 0, pDoc->MakeNumRule( sTmp ));
        aEntry.nListNo = nListNo;
        aListArr.Insert( aEntry, aListArr.Count() );
        pCurRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
        pCurRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, FALSE ));
        pCurRule->SetAutoRule( FALSE );
        pCurRule->SetContinusNum( bContinus );
    }

    if( !pCurRule->GetNumFmt( nLevel ))
        pCurRule->Set( nLevel, pCurRule->Get( nLevel ));
    SwNumFmt* pCurNumFmt = (SwNumFmt*)pCurRule->GetNumFmt( nLevel );
    if( RTF_PNLVLBLT == nToken )
        pCurNumFmt->SetNumberingType(SVX_NUM_CHAR_SPECIAL);
    pCurNumFmt->SetSuffix( aEmptyStr );
    pCurNumFmt->SetPrefix( aEmptyStr );
    pCurNumFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);

    if( bStyleTabValid && RTF_PNSECLVL != nToken )
    {
        // dann den akt. Lvl und Rule am Absatz setzen.
        // Dieses muss aber in den vorherigen "Kontext", sprich in den vor
        // der Klammer offenen Attrset. Darum das SetNewGroup davor und dahinter
        SetNewGroup( FALSE );
        GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, nLevel ));
        GetAttrSet().Put( SwNumRuleItem( pCurRule->GetName() ));
        SetNewGroup( TRUE );
    }

    FontUnderline eUnderline;
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!
    while( nOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':
            if( --nOpenBrakets && IsParserWorking() )
            {
                // Style konnte vollstaendig gelesen werden,
                // also ist das noch ein stabiler Status
                SaveState( RTF_PNSECLVL );
            }
            break;

        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                // Unknown und alle bekannten nicht ausgewerteten Gruppen
                // sofort ueberspringen
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ))
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nOpenBrakets;
            }
            break;

        case RTF_PNCARD:
        case RTF_PNORD:
        case RTF_PNORDT:
        case RTF_PNDEC:     pCurNumFmt->SetNumberingType(SVX_NUM_ARABIC);               break;
        case RTF_PNUCLTR:   pCurNumFmt->SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER_N); break;
        case RTF_PNUCRM:    pCurNumFmt->SetNumberingType(SVX_NUM_ROMAN_UPPER);          break;
        case RTF_PNLCLTR:   pCurNumFmt->SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER_N); break;
        case RTF_PNLCRM:    pCurNumFmt->SetNumberingType(SVX_NUM_ROMAN_LOWER);          break;

        case RTF_PNF:
            {
                const Font& rSVFont = GetFont( USHORT(nTokenValue) );
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                            SvxFontItem( rSVFont.GetFamily(),
                                rSVFont.GetName(), rSVFont.GetStyleName(),
                                rSVFont.GetPitch(), rSVFont.GetCharSet() ));
                if( SVX_NUM_CHAR_SPECIAL == pCurNumFmt->GetNumberingType() )
                    pCurNumFmt->SetBulletFont( &rSVFont );
            }
            break;
        case RTF_PNFS:
            {
                if( -1 == nTokenValue )
                    nTokenValue = 240;
                else
                    nTokenValue *= 10;
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                            SvxFontHeightItem( (const USHORT)nTokenValue ));
            }
            break;

        case RTF_PNB:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxWeightItem(
                                nTokenValue ? WEIGHT_BOLD : WEIGHT_NORMAL ));
            }
            break;

        case RTF_PNI:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxPostureItem(
                            nTokenValue ? ITALIC_NORMAL : ITALIC_NONE ));
            }
            break;

        case RTF_PNCAPS:
        case RTF_PNSCAPS:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxCaseMapItem(
                                nTokenValue ? SVX_CASEMAP_KAPITAELCHEN
                                            : SVX_CASEMAP_NOT_MAPPED ));
            }
            break;
        case RTF_PNSTRIKE:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxCrossedOutItem(
                        nTokenValue ? STRIKEOUT_SINGLE : STRIKEOUT_NONE ));
            }
            break;

        case RTF_PNCF:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxColorItem(
                            GetColor( USHORT(nTokenValue) ) ));
            }
            break;


        case RTF_PNUL:
            eUnderline = nTokenValue ? UNDERLINE_SINGLE : UNDERLINE_NONE;
            goto NUMATTR_SETUNDERLINE;
        case RTF_PNULD:
            eUnderline = UNDERLINE_DOTTED;
            goto NUMATTR_SETUNDERLINE;
        case RTF_PNULDB:
            eUnderline = UNDERLINE_DOUBLE;
            goto NUMATTR_SETUNDERLINE;
        case RTF_PNULNONE:
            eUnderline = UNDERLINE_NONE;
            goto NUMATTR_SETUNDERLINE;
        case RTF_PNULW:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                                    SvxWordLineModeItem( TRUE ));
            }
            eUnderline = UNDERLINE_SINGLE;
            goto NUMATTR_SETUNDERLINE;

NUMATTR_SETUNDERLINE:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                        SvxUnderlineItem( eUnderline ));
            }
            break;

        case RTF_PNINDENT:
            if( 0 > short( nTokenValue ) )
                nTokenValue = - (short)nTokenValue;
            pCurNumFmt->SetFirstLineOffset( - short( nTokenValue ));
            pCurNumFmt->SetAbsLSpace( (nLevel + 1 ) * USHORT( nTokenValue ));
            break;
        case RTF_PNSP:
            pCurNumFmt->SetCharTextDistance( USHORT( nTokenValue ));
            break;

        case RTF_PNPREV:
            if( nLevel )
            {
                BYTE nPrev = 2, nLast = nLevel;
                while( nLast && 1 < pCurRule->Get( --nLast ).GetIncludeUpperLevels() )
                    ++nPrev;
                pCurNumFmt->SetIncludeUpperLevels( nPrev );
            }
            break;

        case RTF_PNQC:  pCurNumFmt->SetNumAdjust( SVX_ADJUST_CENTER );  break;
        case RTF_PNQL:  pCurNumFmt->SetNumAdjust( SVX_ADJUST_LEFT );        break;
        case RTF_PNQR:  pCurNumFmt->SetNumAdjust( SVX_ADJUST_RIGHT );       break;

        case RTF_PNSTART:
            pCurNumFmt->SetStart( USHORT( nTokenValue ));
            break;

        case RTF_PNNUMONCE:
        case RTF_PNACROSS:
        case RTF_PNHANG:
        case RTF_PNRESTART:     break;

        case RTF_PNTXTA:
            {
                String sTmp;
                GetTextToEndGroup( sTmp );
                if( SVX_NUM_CHAR_SPECIAL == pCurNumFmt->GetNumberingType() )
                {
                    pCurNumFmt->SetBulletChar( sTmp.GetChar( 0 ) );
                    if( pCurNumFmt->GetCharFmt() )
                        pCurNumFmt->SetBulletFont( FindFontOfItem(
                                    pCurNumFmt->GetCharFmt()->GetFont() ) );
                    sTmp.Erase();
                }
                pCurNumFmt->SetSuffix( sTmp );
            }
            break;

        case RTF_PNTXTB:
            {
                String sTmp;
                pCurNumFmt->SetPrefix( GetTextToEndGroup( sTmp ) );
            }
            break;
        }
    }

    // falls vollstaendige Numerierung an ist und das Zeichen davor ein
    // Punkt ist, dann will RTF den Punkt als Trenner zwischen den Ebenen
    // haben - das haben wir aber schon als default
    if( 1 < pCurNumFmt->GetIncludeUpperLevels() &&
        1 == pCurNumFmt->GetPrefix().Len() &&
        '.' == pCurNumFmt->GetPrefix().GetChar( 0 ) &&
        SVX_NUM_CHAR_SPECIAL != pCurNumFmt->GetNumberingType() )
        pCurNumFmt->SetPrefix( aEmptyStr );

    // falls das ein nicht numerierter Absatz mit ein Prefix-Text mit
    // einem Zeichen ist, dann setze den als Bulletzeichen
    if( pCurNumFmt->GetCharFmt() && SVX_NUM_NUMBER_NONE == pCurNumFmt->GetNumberingType() &&
        3 == nListNo && 1 == pCurNumFmt->GetPrefix().Len() )
    {
        SwCharFmt* pChFmt = pCurNumFmt->GetCharFmt();
        pCurNumFmt->SetNumberingType(SVX_NUM_CHAR_SPECIAL);
        pCurNumFmt->SetBulletFont( FindFontOfItem( pChFmt->GetFont() ) );

        pCurNumFmt->SetBulletChar( pCurNumFmt->GetPrefix().GetChar( 0 ) );
        pCurNumFmt->SetPrefix( aEmptyStr );

        // den Font oder sogar das gesamte CharFormat loeschen?
        if( SFX_ITEM_SET == pChFmt->GetItemState( RES_CHRATR_FONT, FALSE ))
        {
            if( 1 == pChFmt->GetAttrSet().Count() )
            {
                pCurNumFmt->SetCharFmt( 0 );
                pDoc->DelCharFmt( pChFmt );
            }
            else
                pChFmt->ResetAttr( RES_CHRATR_FONT );
        }
    }

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}


/*  */

// dann noch die Ausgabe-Funktionen (nur fuer WinWord 97)

void SwRTFWriter::OutRTFListTab()
{
    ByteString sOverrideList;
    const SwNumRuleTbl& rListTbl = pDoc->GetNumRuleTbl();
    USHORT nId = 1, nTmplId = 1, n;

    // prepare the NodeNum to generate the NumString
    SwNodeNum aNdNum( 0 );
    for( n = 0; n < MAXLEVEL; ++n )
        aNdNum.GetLevelVal()[ n ] = n;
    BYTE aNumLvlPos[ MAXLEVEL ];

    for( n = rListTbl.Count()+1; n; )
    {
        const SwNumRule* pRule;
        --n;
        if( n == rListTbl.Count() )
            pRule = pDoc->GetOutlineNumRule();
        else
        {
            pRule = rListTbl[ n ];
            if( !pDoc->IsUsed( *pRule ))
                continue;
        }

        BYTE nEnd = MAXLEVEL;
        while( nEnd-- && !pRule->GetNumFmt( nEnd ))
            ;
        ++nEnd;

        for( BYTE nLvl = 0; nLvl < nEnd; ++nLvl )
            if( SVX_NUM_NUMBER_NONE != pRule->Get( nLvl ).GetNumberingType() )
                break;

        if( nLvl == nEnd )      // alle Level no number -> dont write it
            continue;

        // mit der ersten Rule wird auch die ListTable geschrieben
        if( !sOverrideList.Len() )
            OutComment( *this, sRTF_LISTTABLE );

        Strm() << '{' << sRTF_LIST << sRTF_LISTTEMPLATEID;
        OutULong( nTmplId );
        ByteString sTmp;

        if( pRule->IsContinusNum() )
            Strm() << sRTF_LISTSIMPLE;

        for( nLvl = 0; nLvl < nEnd; ++nLvl )
        {
            const SwNumFmt& rFmt = pRule->Get( nLvl );
            Strm() << sNewLine;
            if( nLvl > 8 )          // RTF-kennt nur 9 Ebenen
                OutComment( *this, sRTF_SOUTLVL );

            Strm() << '{' << sRTF_LISTLEVEL << sRTF_LEVELNFC;

            USHORT nVal = 0;
            switch( rFmt.GetNumberingType() )
            {
            case SVX_NUM_ROMAN_UPPER:           nVal = 1;   break;
            case SVX_NUM_ROMAN_LOWER:           nVal = 2;   break;
            case SVX_NUM_CHARS_UPPER_LETTER:
            case SVX_NUM_CHARS_UPPER_LETTER_N:  nVal = 3;   break;
            case SVX_NUM_CHARS_LOWER_LETTER:
            case SVX_NUM_CHARS_LOWER_LETTER_N:  nVal = 4;   break;

            case SVX_NUM_BITMAP:
            case SVX_NUM_CHAR_SPECIAL:          nVal = 23;  break;
            }
            OutLong( nVal ) << sRTF_LEVELJC;

            switch( rFmt.GetNumAdjust() )
            {
            case SVX_ADJUST_CENTER:     nVal = 1;   break;
            case SVX_ADJUST_RIGHT:      nVal = 2;   break;
            default:                    nVal = 0;   break;
            }
            OutLong( nVal ) << sRTF_LEVELSTARTAT;
            OutLong( rFmt.GetStart() )
                << sRTF_LEVELFOLLOW << "2{" << sRTF_LEVELTEXT << ' ';

            BOOL bWriteBulletFont = FALSE;
            memset( aNumLvlPos, 0, MAXLEVEL );
            if( SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType() ||
                SVX_NUM_BITMAP == rFmt.GetNumberingType() )
            {
                Strm() << "\\'01\\'"; OutHex( rFmt.GetBulletChar() );
                bWriteBulletFont = TRUE;
            }
            else if( SVX_NUM_NUMBER_NONE != rFmt.GetNumberingType() )
            {
                memset( aNumLvlPos, 0, MAXLEVEL );
                BYTE* pLvlPos = aNumLvlPos;
                aNdNum.SetLevel( nLvl );
                String sNumStr( pRule->MakeNumString( aNdNum, FALSE, TRUE ));

                // now search the nums in the string
                for( BYTE i = 0; i <= nLvl; ++i )
                {
                    String sSrch( String::CreateFromInt32( i ));
                    xub_StrLen nFnd = sNumStr.Search( sSrch );
                    if( STRING_NOTFOUND != nFnd )
                    {
                        *pLvlPos = (BYTE)(nFnd + rFmt.GetPrefix().Len() + 1 );
                        ++pLvlPos;
                        sNumStr.SetChar( nFnd, (sal_Unicode)i );
                    }
                }

                Strm() << "\\'";
                OutHex( sNumStr.Len() + rFmt.GetPrefix().Len() +
                                        rFmt.GetSuffix().Len() );

                if( rFmt.GetPrefix().Len() )
                    RTFOutFuncs::Out_String( Strm(), rFmt.GetPrefix(),
                                            DEF_ENCODING, bWriteHelpFmt );


                for( xub_StrLen x = 0; x < sNumStr.Len(); ++x )
                    if( sNumStr.GetChar( x ) < 0x20 ||
                        sNumStr.GetChar( x ) > 0xFF )
                    {
                        Strm() << "\\'"; OutHex( sNumStr.GetChar( x ) );
                    }
                    else
                        Strm() << (sal_Char)sNumStr.GetChar( x );

                if( rFmt.GetSuffix().Len() )
                    RTFOutFuncs::Out_String( Strm(), rFmt.GetSuffix(),
                                            DEF_ENCODING, bWriteHelpFmt );
            }

            // write the levelnumbers
            Strm() << ";}{" << sRTF_LEVELNUMBERS;
            for( BYTE i = 0; i <= nLvl && aNumLvlPos[ i ]; ++i )
            {
                Strm() << "\\'"; OutHex( aNumLvlPos[ i ] );
            }
            Strm() << ";}";

            if( rFmt.GetCharFmt() )
                Out_SfxItemSet( aRTFAttrFnTab, *this,
                                rFmt.GetCharFmt()->GetAttrSet(), TRUE );

            if( bWriteBulletFont )
            {
                Strm() << sRTF_F;
                const Font* pFont = rFmt.GetBulletFont();
                if( !pFont )
                    pFont = &SwNumRule::GetDefBulletFont();
                OutULong( GetId( *pFont ));
            }

            Strm() << sRTF_FI;
            OutLong( rFmt.GetFirstLineOffset() ) << sRTF_LI;
            OutLong( rFmt.GetAbsLSpace() );

            Strm() << '}';

            if( nLvl > 8 )      // RTF-kennt nur 9 Ebenen
                Strm() << '}';
        }

        if( !pRule->IsAutoRule() )
        {
            Strm() << '{' << sRTF_LISTNAME << ' ';
            RTFOutFuncs::Out_String( Strm(), pRule->GetName(), DEF_ENCODING,
                                    bWriteHelpFmt ) << ";}" ;
        }
        Strm() << sRTF_LISTID;
        OutULong( nId ) << '}' << sNewLine;

        sTmp = '{';
        ((((((( sTmp += sRTF_LISTOVERRIDE ) +=
                    sRTF_LISTID ) += ByteString::CreateFromInt32( nId )) +=
                    sRTF_LISTOVERRIDECOUNT ) += '0' ) +=
                    sRTF_LS ) += ByteString::CreateFromInt32( n )) += '}';
        sOverrideList += sTmp;

        ++nId, ++nTmplId;
    }

    if( sOverrideList.Len() )
        Strm() << "}{" << sRTF_LISTOVERRIDETABLE
               << sOverrideList.GetBuffer() << '}' << sNewLine;
}

USHORT SwRTFWriter::GetId( const SwNumRuleItem& rItem ) const
{
    return pDoc->FindNumRule( rItem.GetValue() );
}

void SwRTFWriter::OutListNum( const SwTxtNode& rNd )
{
    const SwNumRule* pRule = rNd.GetNumRule();
    const SwNodeNum* pNdNum = 0;
    // or is an outlinerule valid?
    if( pRule )
        pNdNum = rNd.GetNum();
    else if( MAXLEVEL > rNd.GetTxtColl()->GetOutlineLevel() &&
            0 != ( pNdNum = rNd.GetOutlineNum() ) )
        pRule = pDoc->GetOutlineNumRule();

    if( pRule && pNdNum && MAXLEVEL > pNdNum->GetLevel() )
    {
        bOutFmtAttr = FALSE;
        bOutListNumTxt = TRUE;

        BOOL bValidNum = MAXLEVEL > pNdNum->GetLevel();
        BYTE nLvl = GetRealLevel( pNdNum->GetLevel() );
        const SwNumFmt* pFmt = pRule->GetNumFmt( nLvl );
        if( !pFmt )
            pFmt = &pRule->Get( nLvl );

        const SfxItemSet& rNdSet = rNd.GetSwAttrSet();

        SfxItemSet aSet( *rNdSet.GetPool(), rNdSet.GetRanges() );
        aSet.SetParent( &rNdSet );
        SvxLRSpaceItem aLR( (SvxLRSpaceItem&)rNdSet.Get( RES_LR_SPACE ) );
        aLR.SetTxtLeft( aLR.GetTxtLeft() + pFmt->GetAbsLSpace() );

        if( bValidNum )
        {
            aLR.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
            Strm() << '{' << sRTF_LISTTEXT << sRTF_PARD << sRTF_PLAIN << ' ';
        }
        aSet.Put( aLR );
        Out_SfxItemSet( aRTFAttrFnTab, *this, aSet, TRUE );

        if( pFmt->GetCharFmt() )
            Out_SfxItemSet( aRTFAttrFnTab, *this,
                        pFmt->GetCharFmt()->GetAttrSet(), TRUE );

        if( bValidNum )
        {
            String sTxt;
            if( SVX_NUM_CHAR_SPECIAL == pFmt->GetNumberingType() || SVX_NUM_BITMAP == pFmt->GetNumberingType() )
                sTxt = pFmt->GetBulletChar();
            else
                sTxt = pRule->MakeNumString( *pNdNum );

            if( bOutFmtAttr )
            {
                Strm() << ' ';
                bOutFmtAttr = FALSE;
            }

            RTFOutFuncs::Out_String( Strm(), sTxt, DEF_ENCODING, bWriteHelpFmt);

            if( OUTLINE_RULE != pRule->GetRuleType() )
            {
                Strm() /*<< sRTF_TAB*/ << '}' << sRTF_ILVL;
                if( nLvl > 8 )          // RTF-kennt nur 9 Ebenen
                {
                    OutULong( 8 );
                    OutComment( *this, sRTF_SOUTLVL );
                    OutULong( nLvl ) << '}';
                }
                else
                    OutULong( nLvl );
                Strm()  << ' ';
            }
            else
                Strm() /*<< sRTF_TAB*/ << '}';
        }
        bOutListNumTxt = FALSE;
    }
}


