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

#include <hintids.hxx>
#include <tools/stream.hxx>
#include <svtools/rtftoken.h>
#include <svtools/rtfkeywd.hxx>
#include <svl/intitem.hxx>
#include <svtools/rtfout.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/numitem.hxx>
#include <shellio.hxx>
#include <fltini.hxx>
#include <swtypes.hxx>
#include <swparrtf.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <charfmt.hxx>
#include <charatr.hxx>
#include <paratr.hxx>
#include <cmdid.h>
#include <numrule.hxx>

#define RTF_NUMRULE_NAME "RTF_Num"

static void lcl_ExpandNumFmts( SwNumRule& rRule )
{
    // dann noch das NumFormat in alle Ebenen setzen
    for( sal_uInt8 n = 1; n < MAXLEVEL; ++n )
        if( !rRule.GetNumFmt( n ) )
        {
            SwNumFmt aNumFmt( rRule.Get( 0 ));
            aNumFmt.SetIndentAt( aNumFmt.GetIndentAt() * ( n + 1 ) );
            rRule.Set( n, aNumFmt );
        }
}

SfxItemSet& GetNumChrFmt( SwDoc& rDoc, SwNumRule& rRule, sal_uInt8 nNumLvl )
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

void SwRTFParser::ReadListLevel( SwNumRule& rRule, sal_uInt8 nNumLvl )
{
    int nToken;
    int nNumOpenBrakets = 1;        // die erste wurde schon vorher erkannt !!
    int nLvlTxtLevel = 0, nLvlNumberLevel = 0;
    String sLvlText, sLvlNumber;
    SwNumFmt* pCurNumFmt;
    SvxNumberFormat::LabelFollowedBy eFollowedBy = SvxNumberFormat::NOTHING;

    if( MAXLEVEL >= nNumLvl )
    {
        pCurNumFmt = (SwNumFmt*)rRule.GetNumFmt( nNumLvl );
        pCurNumFmt->SetIndentAt( 0 );
        pCurNumFmt->SetFirstLineIndent( 0 );
    }
    else
        pCurNumFmt = 0;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':
            if( nNumOpenBrakets )
            {
                if( nLvlTxtLevel == nNumOpenBrakets )
                {
                    if( DelCharAtEnd( sLvlText, ';' ).Len() &&
                        sLvlText.Len() && sLvlText.Len() ==
                        (sal_uInt16)(sLvlText.GetChar( 0 )) + 1 )
                        sLvlText.Erase( 0, 1 );
                    nLvlTxtLevel = 0;
                }
                if( nLvlNumberLevel == nNumOpenBrakets )
                {
                    DelCharAtEnd( sLvlNumber, ';' );
                    nLvlNumberLevel = 0;
                }
            }
            --nNumOpenBrakets;
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
                ++nNumOpenBrakets;
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
                pCurNumFmt->SetStart( sal_uInt16( nTokenValue ));
            break;

        case RTF_LEVELTEXT:
            nLvlTxtLevel = nNumOpenBrakets;
            break;

        case RTF_LEVELNUMBERS:
            nLvlNumberLevel = nNumOpenBrakets;
            break;


        case RTF_TEXTTOKEN:
            if( nLvlTxtLevel == nNumOpenBrakets )
                sLvlText += aToken;
            else if( nLvlNumberLevel == nNumOpenBrakets )
                sLvlNumber += aToken;
            break;

        case RTF_LEVELFOLLOW:
          /* removed; waiting for swnum02 to be integrated! */
            switch (nTokenValue)
            {
            case 0:
                eFollowedBy = SvxNumberFormat::LISTTAB;
                break;
            case 1:
                eFollowedBy = SvxNumberFormat::SPACE;
                break;
            }
            break;

        case RTF_LEVELOLD:
        case RTF_LEVELPREV:
        case RTF_LEVELPREVSPACE:
        case RTF_LEVELINDENT:
        case RTF_LEVELSPACE:
        case RTF_LEVELLEGAL:
        case RTF_LEVELNORESTART:
            break;

        default:
            if( pCurNumFmt && (
                RTF_CHRFMT == (nToken & ~(0xff | RTF_SWGDEFS) ) ||
                RTF_PARFMT == (nToken & ~(0xff | RTF_SWGDEFS) ) ))
            {
                SfxItemSet aSet( pDoc->GetAttrPool(), aTxtNodeSetRange );
                // put the current CharFmtAttr into the set
                SfxItemSet& rCFmtSet = GetNumChrFmt( *pDoc, rRule, nNumLvl );
                aSet.Put( rCFmtSet );
                // and put the current "LRSpace" into the set
                {
                    SvxLRSpaceItem aLR( RES_LR_SPACE );
                    aLR.SetTxtLeft( pCurNumFmt->GetIndentAt() );
                    aLR.SetTxtFirstLineOfst(pCurNumFmt->GetFirstLineIndent());
                    aSet.Put( aLR );
                }

                ReadAttr( nToken, &aSet );

                //#i24880# Word appears to ignore char background for numbering
                aSet.ClearItem(RES_CHRATR_BACKGROUND);

                // put all CharFmt Items into the charfmt
                rCFmtSet.Put( aSet );

                // test for LRSpace Item. If exist then set all relevant
                // values on the numrule format
                const SfxPoolItem* pItem;
                if( SFX_ITEM_SET == aSet.GetItemState( RES_LR_SPACE,
                        sal_False, &pItem ))
                {
                    const SvxLRSpaceItem& rLR = *(SvxLRSpaceItem*)pItem;
                    pCurNumFmt->SetListtabPos( rLR.GetTxtLeft( ) );
                    pCurNumFmt->SetIndentAt( rLR.GetTxtLeft() );
                    pCurNumFmt->SetFirstLineIndent( rLR.GetTxtFirstLineOfst());
                }

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
                sLvlText.Copy( 0, sal_uInt16( sLvlNumber.GetChar( 0 ))-1 ));
            pCurNumFmt->SetSuffix( sLvlText.Copy(
                    sal_uInt16( sLvlNumber.GetChar( sLvlNumber.Len()-1 )) ));
            // wieviele Levels stehen im String?
            pCurNumFmt->SetIncludeUpperLevels( (sal_uInt8)sLvlNumber.Len() );
        }
        else
        {
            pCurNumFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);
            pCurNumFmt->SetSuffix( sLvlText );
        }

        /* removed; waiting for swnum02 to be integrated!*/
        pCurNumFmt->SetLabelFollowedBy( eFollowedBy );
        if (eFollowedBy == SvxNumberFormat::LISTTAB && !pCurNumFmt->IsItemize())
        {
            pCurNumFmt->SetIndentAt(0);
            pCurNumFmt->SetFirstLineIndent(0);
        }
    }

    SkipToken( -1 );
}

void SwRTFParser::ReadListTable()
{
    int nToken;
    int nNumOpenBrakets = 1;        // die erste wurde schon vorher erkannt !!
    bNewNumList = sal_True;

    sal_uInt8 nNumLvl = 0;
    SwNumRule* pCurRule = 0;
    SwListEntry aEntry;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':       if( --nNumOpenBrakets && IsParserWorking() )
                        {
                            // Style konnte vollstaendig gelesen werden,
                            // also ist das noch ein stabiler Status
                            SaveState( RTF_LISTTABLE );
                            if( 1 == nNumOpenBrakets )
                            {
                                if( aEntry.nListId )
                                    aListArr.push_back( aEntry );
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
                ++nNumOpenBrakets;
            }
            break;

        case RTF_LIST:
            {
                if( pCurRule && pCurRule->IsContinusNum() )
                    lcl_ExpandNumFmts( *pCurRule );

                String sTmp(rtl::OUString(RTF_NUMRULE_NAME " 1"));
                aEntry.nListDocPos = pDoc->MakeNumRule( sTmp, 0, sal_False, SvxNumberFormat::LABEL_ALIGNMENT );
                pCurRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
                // #i91400#
                pCurRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, sal_False ),
                                   *pDoc );

                pCurRule->SetAutoRule( sal_False );
                nNumLvl = (sal_uInt8)-1;
            }
            break;

        case RTF_LISTID:            aEntry.nListId = nTokenValue;       break;
        case RTF_LISTTEMPLATEID:    aEntry.nListTemplateId = nTokenValue; break;

        case RTF_LISTRESTARTHDN:
            break;
        case RTF_LISTNAME:
            if (nNextCh=='}') break;  // empty listnames
            if( RTF_TEXTTOKEN == GetNextToken() )
            {
                String sTmp( DelCharAtEnd( aToken, ';' ));
                if( sTmp.Len() && !pDoc->FindNumRulePtr( sTmp ))
                {
                    // #i91400#
                    pCurRule->SetName( sTmp, *pDoc );
                }
            }
            SkipGroup();
            break;

        case RTF_LISTSIMPLE:
            pCurRule->SetContinusNum( sal_True );
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

static sal_Bool lcl_IsEqual( SwNumRule* pOrigRule, SwNumRule* pRule )
{
    sal_Bool bRet = 0;
    if( pOrigRule && pRule )
    {
        bRet =  pOrigRule->GetRuleType() == pRule->GetRuleType() &&
                pOrigRule->IsContinusNum() == pRule->IsContinusNum() &&
                pOrigRule->IsAbsSpaces() == pRule->IsAbsSpaces();
        if( bRet )
            for( sal_uInt8 n = 0; bRet && n < MAXLEVEL; ++n )
            {
                const SwNumFmt* pOFmt = pOrigRule->GetNumFmt( n ),
                              * pFmt = pRule->GetNumFmt( n );
                if( pFmt && pOFmt )
                {
                    SwCharFmt* pOCFmt = pOFmt->GetCharFmt(),
                             * pCFmt = pFmt->GetCharFmt();
                    if( pOCFmt && pCFmt )
                    {
                        bRet = 0 != (pCFmt->GetAttrSet() == pOCFmt->GetAttrSet());
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
    int nNumOpenBrakets = 1;        // die erste wurde schon vorher erkannt !!
    SwListEntry aEntry;
    SwNumRule* pRule = 0, *pOrigRule = 0;
    sal_uInt8 nNumLvl = 0;
    sal_Bool bOverrideFormat = sal_False, bOverrideStart = sal_False;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':
            if( --nNumOpenBrakets && IsParserWorking() )
            {
                // Style konnte vollstaendig gelesen werden,
                // also ist das noch ein stabiler Status
                SaveState( RTF_LISTOVERRIDETABLE );

                if( 1 == nNumOpenBrakets )
                {
                    bOverrideFormat = sal_False, bOverrideStart = sal_False;
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
                    {
                        int nMatch=-1;
                        for( size_t n = aListArr.size(); n; )
                        {
                            if( aListArr[ --n ].nListId == aEntry.nListId)
                            {
                                nMatch=n;
                                break;
                            }
                        }
                        if(nMatch>=0)
                        {
                            sal_uInt16 nMatch2 = static_cast< sal_uInt16 >(nMatch);
                            if (!aListArr[nMatch2].nListNo )
                            {
                                aListArr[nMatch2].nListNo = aEntry.nListNo;
                            }
                            else
                            {
                                aEntry.nListDocPos=aListArr[nMatch2].nListDocPos;
                                aEntry.nListTemplateId=aListArr[nMatch2].nListTemplateId;
                                aListArr.push_back( aEntry );
                            }
                            if(pOrigRule)
                                aListArr[nMatch2].nListDocPos = aEntry.nListDocPos;
                        }
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
                ++nNumOpenBrakets;
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
                    for( size_t n = 0; n < aListArr.size(); ++n )
                        if( aListArr[ n ].nListId == aEntry.nListId )
                        {
                            pRule = pDoc->GetNumRuleTbl()[
                                                aListArr[ n ].nListDocPos ];
                            pOrigRule = pRule;

                            String sTmp(rtl::OUString(RTF_NUMRULE_NAME " 1"));
                            aEntry.nListDocPos = pDoc->MakeNumRule( sTmp, pRule );
                            pRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
                            // #i91400#
                            pRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, sal_False ),
                                            *pDoc );

                            pRule->SetAutoRule( sal_False );
                            nNumLvl = (sal_uInt8)-1;
                            aListArr.push_back( aEntry );
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
            bOverrideStart = sal_True;
            break;

        case RTF_LISTOVERRIDEFORMAT:
            bOverrideFormat = sal_True;
            break;

        case RTF_LFOLEVEL:
            // was fehlt noch?
            break;
        }
    }

    // search the outline numrule and set it into the doc
    if( !GetStyleTbl().empty() )
    {
        if( !bStyleTabValid )
            MakeStyleTab();

        const SfxPoolItem* pItem( 0 );
        std::map<sal_Int32,SwTxtFmtColl*>::const_iterator iterColl;
        sal_uInt16 nRulePos( USHRT_MAX );
        const SwNumRule *pNumRule = 0;
        for (SvxRTFStyleTbl::iterator it = GetStyleTbl().begin();
            it != GetStyleTbl().end(); ++it)
        {
            SvxRTFStyleType* pStyle = it->second;
            if ( MAXLEVEL > pStyle->nOutlineNo )
            {
                iterColl = aTxtCollTbl.find( it->first );
                if ( iterColl != aTxtCollTbl.end() )
                {
                    const SfxItemState eItemState =
                        iterColl->second->GetItemState( RES_PARATR_NUMRULE, sal_False, &pItem );
                    if ( eItemState == SFX_ITEM_SET )
                    {
                        nRulePos = pDoc->FindNumRule( ((SwNumRuleItem*)pItem)->GetValue() );
                        if ( nRulePos != USHRT_MAX )
                        {
                            pNumRule = pDoc->GetNumRuleTbl()[ nRulePos ];
                            if ( pNumRule->IsAutoRule() &&
                                 pNumRule != pDoc->GetOutlineNumRule() )
                            {
                                pDoc->SetOutlineNumRule( *pNumRule );
                                pDoc->DelNumRule( pNumRule->GetName() );
                                // now pNumRule pointer is invalid !!!

                                // now decrement all position in the listtable, which will
                                // behind the doc-rule position
                                for( size_t n = aListArr.size(); n; )
                                {
                                    SwListEntry& rEntry = aListArr[ --n ];
                                    if( rEntry.nListDocPos == nRulePos )
                                        aListArr.erase( aListArr.begin()+n );
                                    else if( rEntry.nListDocPos > nRulePos )
                                        --rEntry.nListDocPos;
                                }
                                break;
                            }
                        }
                    }
                }
            }

            pStyle->aAttrSet.ClearItem( FN_PARAM_NUM_LEVEL );
        }
    }

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}

SwNumRule* SwRTFParser::GetNumRuleOfListNo( long nListNo, sal_Bool bRemoveFromList )
{
    SwNumRule* pRet = 0;
    SwListEntry* pEntry;
    for( size_t n = aListArr.size(); n; )
        if( ( pEntry = &aListArr[ --n ])->nListNo == nListNo )
        {
            if( bRemoveFromList )
                aListArr.erase( aListArr.begin()+n );
            else
            {
                pEntry->bRuleUsed = sal_True;
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
        for ( sal_uInt8 nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
        {
            SwNumFmt& rNFmt = (SwNumFmt&)pRule->Get( nLvl );
            SwCharFmt* pCFmt = rNFmt.GetCharFmt();
            if( pCFmt )
            {
                rNFmt.ForgetCharFmt();
                if( !pCFmt->GetDepends() )
                    pDoc->DelCharFmt( pCFmt );
            }
        }
        pDoc->DelNumRule( pRule->GetName() );
    }
#if OSL_DEBUG_LEVEL > 0
    else
    {
        OSL_ENSURE( pRule, "NumRulePointer 0 kann nicht geloescht werden" );
    }
#endif
}

void SwRTFParser::RemoveUnusedNumRules()
{
    SwListEntry* pEntry;
    std::set<SwNumRule*> aDelArr;
    size_t n;
    for( n = aListArr.size(); n; )
    {
        if( !( pEntry = &aListArr[ --n ])->bRuleUsed )
        {
            // really *NOT* used by anyone else?
            sal_Bool unused=sal_True;
            for(size_t j = 0;  j < aListArr.size();  ++j)
            {
                if (aListArr[n].nListNo==aListArr[j].nListNo)
                    unused&=!aListArr[j].bRuleUsed;
            }
            if (unused)
            {
                SwNumRule* p = pDoc->GetNumRuleTbl()[pEntry->nListDocPos];
                // dont delete named char formats
                if( aDelArr.find( p ) == aDelArr.end() &&
                    p->GetName().EqualsAscii( RTF_NUMRULE_NAME, 0,
                                    sizeof( RTF_NUMRULE_NAME )) )
                    aDelArr.insert( p );
            }
        }
    }

    while( !aDelArr.empty() )
    {
        RemoveUnusedNumRule( *aDelArr.begin() );
        aDelArr.erase( aDelArr.begin() );
    }
}

const Font* SwRTFParser::FindFontOfItem( const SvxFontItem& rItem ) const
{
    SvxRTFFontTbl& rFntTbl = ((SwRTFParser*)this)->GetFontTbl();
    for (SvxRTFFontTbl::iterator it = rFntTbl.begin(); it != rFntTbl.end(); ++it)
    {
        const Font* pFnt = it->second;
        if( pFnt->GetFamily() == rItem.GetFamily() &&
            pFnt->GetName() == rItem.GetFamilyName() &&
            pFnt->GetStyleName() == rItem.GetStyleName() &&
            pFnt->GetPitch() == rItem.GetPitch() &&
            pFnt->GetCharSet() == rItem.GetCharSet() )
            return pFnt;
    }
    return 0;
}


SwNumRule *SwRTFParser::ReadNumSecLevel( int nToken )
{
    // lese die \pnseclvl - Gruppe
    // nTokenValue gibt schon den richtigen Level vor 1 - 9!
    sal_uInt8 nLevel = 0;
    long nListNo = 0;
    sal_Bool bContinus = sal_True;

    if( RTF_PNSECLVL == nToken )
    {
        // suche die Rule - steht unter Nummer 3
        nListNo = 3;
        bContinus = sal_False;
        nLevel = MAXLEVEL <= (unsigned long) nTokenValue ? MAXLEVEL - 1
            : (!nTokenValue ? 0 : sal_uInt8( nTokenValue - 1 ));
    }
    else
    {
        switch( nToken = GetNextToken() )
        {
        case RTF_PNLVL:         nListNo = 3;
                                bContinus = sal_False;
                                nLevel = MAXLEVEL <= (unsigned long) nTokenValue
                                                    ? MAXLEVEL - 1
                                    : (!nTokenValue ? 0 : sal_uInt8( nTokenValue-1 ));
                                break;

        case RTF_PNLVLBODY:
            nListNo = 2;
            break;
        case RTF_PNLVLBLT:
            nListNo = 1;
            break;
        case RTF_PNLVLCONT:
            SkipGroup();
            return 0;
        default:
            SkipGroup();
            return 0;
        }
    }

    // suche die Rule - steht unter Nummer 3
    sal_uInt16 nNewFlag = static_cast< sal_uInt16 >(1 << nListNo);
    SwNumRule* pCurRule = GetNumRuleOfListNo( nListNo,
                                        0 != ( nNewNumSectDef & nNewFlag ) );
    if( !pCurRule )
    {
        // dann muessen wir die mal anlegen
        nNewNumSectDef &= ~nNewFlag;
        String sTmp(rtl::OUString(RTF_NUMRULE_NAME " 1"));
        SwListEntry aEntry( nListNo, 0, pDoc->MakeNumRule( sTmp ));
        aEntry.nListNo = nListNo;
        aListArr.push_back( aEntry );
        pCurRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
        // #i91400#
        pCurRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, sal_False ), *pDoc );

        pCurRule->SetAutoRule( sal_False );
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
        SetNewGroup( sal_False );
        GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, nLevel ));
        GetAttrSet().Put( SwNumRuleItem( pCurRule->GetName() ));
        SetNewGroup( sal_True );
    }

    FontUnderline eUnderline;
    int nNumOpenBrakets = 1;        // die erste wurde schon vorher erkannt !!
    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':
            if( --nNumOpenBrakets && IsParserWorking() )
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
                ++nNumOpenBrakets;
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
                const Font& rSVFont = GetFont( sal_uInt16(nTokenValue) );
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                            SvxFontItem( rSVFont.GetFamily(),
                                rSVFont.GetName(), rSVFont.GetStyleName(),
                                rSVFont.GetPitch(), rSVFont.GetCharSet(),
                                            RES_CHRATR_FONT ));
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
                            SvxFontHeightItem( (const sal_uInt16)nTokenValue, 100, RES_CHRATR_FONTSIZE ));
            }
            break;

        case RTF_PNB:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxWeightItem(
                                nTokenValue ? WEIGHT_BOLD : WEIGHT_NORMAL, RES_CHRATR_WEIGHT ));
            }
            break;

        case RTF_PNI:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxPostureItem(
                            nTokenValue ? ITALIC_NORMAL : ITALIC_NONE, RES_CHRATR_POSTURE ));
            }
            break;

        case RTF_PNCAPS:
        case RTF_PNSCAPS:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxCaseMapItem(
                                nTokenValue ? SVX_CASEMAP_KAPITAELCHEN
                                            : SVX_CASEMAP_NOT_MAPPED, RES_CHRATR_CASEMAP ));
            }
            break;
        case RTF_PNSTRIKE:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxCrossedOutItem(
                        nTokenValue ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ));
            }
            break;

        case RTF_PNCF:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxColorItem(
                            GetColor( sal_uInt16(nTokenValue) ), RES_CHRATR_COLOR ));
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
                                    SvxWordLineModeItem( sal_True, RES_CHRATR_WORDLINEMODE ));
            }
            eUnderline = UNDERLINE_SINGLE;
            goto NUMATTR_SETUNDERLINE;

NUMATTR_SETUNDERLINE:
            {
                GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                        SvxUnderlineItem( eUnderline, RES_CHRATR_UNDERLINE ));
            }
            break;

        case RTF_PNINDENT:
            if( 0 > short( nTokenValue ) )
                nTokenValue = - (short)nTokenValue;
            pCurNumFmt->SetFirstLineIndent( - nTokenValue );
            pCurNumFmt->SetIndentAt( (nLevel + 1 ) * nTokenValue );
            break;
        case RTF_PNSP:
            pCurNumFmt->SetCharTextDistance( sal_uInt16( nTokenValue ));
            break;

        case RTF_PNPREV:
            if( nLevel )
            {
                sal_uInt8 nPrev = 2, nLast = nLevel;
                while( nLast && 1 < pCurRule->Get( --nLast ).GetIncludeUpperLevels() )
                    ++nPrev;
                pCurNumFmt->SetIncludeUpperLevels( nPrev );
            }
            break;

        case RTF_PNQC:  pCurNumFmt->SetNumAdjust( SVX_ADJUST_CENTER );  break;
        case RTF_PNQL:  pCurNumFmt->SetNumAdjust( SVX_ADJUST_LEFT );        break;
        case RTF_PNQR:  pCurNumFmt->SetNumAdjust( SVX_ADJUST_RIGHT );       break;

        case RTF_PNSTART:
            pCurNumFmt->SetStart( sal_uInt16( nTokenValue ));
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
        1 == pCurNumFmt->GetPrefix().getLength() &&
        '.' == pCurNumFmt->GetPrefix()[0] &&
        SVX_NUM_CHAR_SPECIAL != pCurNumFmt->GetNumberingType() )
        pCurNumFmt->SetPrefix( aEmptyStr );

    // falls das ein nicht numerierter Absatz mit ein Prefix-Text mit
    // einem Zeichen ist, dann setze den als Bulletzeichen
    if( pCurNumFmt->GetCharFmt() && SVX_NUM_NUMBER_NONE == pCurNumFmt->GetNumberingType() &&
        3 == nListNo && 1 == pCurNumFmt->GetPrefix().getLength() )
    {
        SwCharFmt* pChFmt = pCurNumFmt->GetCharFmt();
        pCurNumFmt->SetNumberingType(SVX_NUM_CHAR_SPECIAL);
        pCurNumFmt->SetBulletFont( FindFontOfItem( pChFmt->GetFont() ) );

        pCurNumFmt->SetBulletChar( pCurNumFmt->GetPrefix()[0] );
        pCurNumFmt->SetPrefix( aEmptyStr );

        // den Font oder sogar das gesamte CharFormat loeschen?
        if( SFX_ITEM_SET == pChFmt->GetItemState( RES_CHRATR_FONT, sal_False ))
        {
            if( 1 == pChFmt->GetAttrSet().Count() )
            {
                pCurNumFmt->SetCharFmt( 0 );
                pDoc->DelCharFmt( pChFmt );
            }
            else
                pChFmt->ResetFmtAttr( RES_CHRATR_FONT );
        }
    }

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
    return pCurRule;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
