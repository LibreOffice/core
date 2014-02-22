/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <hintids.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <sfx2/sfx.hrc>
#include <svtools/htmlout.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>
#include <svl/whiter.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtpdsc.hxx>
#include <fmtflcnt.hxx>
#include <fmtinfmt.hxx>
#include <txatbase.hxx>
#include <frmatr.hxx>
#include <charfmt.hxx>
#include <fmtfld.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <swtable.hxx>
#include "fldbas.hxx"
#include <breakit.hxx>
#include <htmlnum.hxx>
#include <wrthtml.hxx>
#include <htmlfly.hxx>
#include <numrule.hxx>
#include <rtl/strbuf.hxx>

using namespace ::com::sun::star;

HTMLOutEvent aAnchorEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonclick,      OOO_STRING_SVTOOLS_HTML_O_onclick,      SFX_EVENT_MOUSECLICK_OBJECT },
    { OOO_STRING_SVTOOLS_HTML_O_SDonmouseover,  OOO_STRING_SVTOOLS_HTML_O_onmouseover,  SFX_EVENT_MOUSEOVER_OBJECT  },
    { OOO_STRING_SVTOOLS_HTML_O_SDonmouseout,       OOO_STRING_SVTOOLS_HTML_O_onmouseout,       SFX_EVENT_MOUSEOUT_OBJECT   },
    { 0,                        0,                      0                           }
};

static Writer& OutHTML_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt );

sal_uInt16 SwHTMLWriter::GetDefListLvl( const OUString& rNm, sal_uInt16 nPoolId )
{
    if( nPoolId == RES_POOLCOLL_HTML_DD )
    {
        return 1 | HTML_DLCOLL_DD;
    }
    else if( nPoolId == RES_POOLCOLL_HTML_DT )
    {
        return 1 | HTML_DLCOLL_DT;
    }

    OUString sDTDD( OOO_STRING_SVTOOLS_HTML_dt );
    sDTDD += " ";
    if( rNm.startsWith(sDTDD) )
        
        return (sal_uInt16)rNm.copy( sDTDD.getLength() ).toInt32() | HTML_DLCOLL_DT;

    sDTDD = OUString( OOO_STRING_SVTOOLS_HTML_dd ) + " ";
    if( rNm.startsWith(sDTDD) )
        
        return (sal_uInt16)rNm.copy( sDTDD.getLength() ).toInt32() | HTML_DLCOLL_DD;

    return 0;
}

void SwHTMLWriter::OutAndSetDefList( sal_uInt16 nNewLvl )
{
    
    if( nDefListLvl < nNewLvl )
    {
        
        
           
        
        

        ChangeParaToken( 0 );

        
        for( sal_uInt16 i=nDefListLvl; i<nNewLvl; i++ )
        {
            if( bLFPossible )
                OutNewLine();
            HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_deflist, sal_True );
            IncIndentLevel();
            bLFPossible = sal_True;
        }
    }
    else if( nDefListLvl > nNewLvl )
    {
        for( sal_uInt16 i=nNewLvl ; i < nDefListLvl; i++ )
        {
            DecIndentLevel();
            if( bLFPossible )
                OutNewLine();
            HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_deflist, sal_False );
            bLFPossible = sal_True;
        }
    }

    nDefListLvl = nNewLvl;
}


void SwHTMLWriter::ChangeParaToken( sal_uInt16 nNew )
{
    if( nNew != nLastParaToken && HTML_PREFORMTXT_ON == nLastParaToken )
    {
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_preformtxt, sal_False );
        bLFPossible = sal_True;
    }
    nLastParaToken = nNew;
}

sal_uInt16 SwHTMLWriter::GetCSS1ScriptForScriptType( sal_uInt16 nScriptType )
{
    sal_uInt16 nRet = CSS1_OUTMODE_ANY_SCRIPT;

    switch( nScriptType )
    {
    case i18n::ScriptType::LATIN:
        nRet = CSS1_OUTMODE_WESTERN;
        break;
    case i18n::ScriptType::ASIAN:
        nRet = CSS1_OUTMODE_CJK;
        break;
    case i18n::ScriptType::COMPLEX:
        nRet = CSS1_OUTMODE_CTL;
        break;
    }

    return nRet;
}


/*
 * Formate wie folgt ausgeben:
 * - fuer Formate, fuer die es entsprechende HTML-Tags gibt wird das
 *   Tag ausgegeben
 * - fuer alle anderen wird ein Absatz-Tag <P> ausgegeben und bUserFmt
 *   gesetzt
 * - Wenn eine Absatz-Ausrichtung am uebergebenen Item-Set des Nodes
 *   oder im Item-Set des Format gesetzt ist, wird ein ALIGN=xxx ausgegeben,
 *   sofern HTML es zulaesst
 * - in jedem Fall wird harte Attributierung als STYLE-Option geschrieben.
 *   Wenn bUserFmt nicht gesetzt ist, wird nur der uebergebene Item-Set
 *   betrachtet. Sonst werden auch Attribute des Formats ausgegeben.
 */

struct SwHTMLTxtCollOutputInfo
{
    OString aToken;        
    SfxItemSet *pItemSet;       

    sal_Bool bInNumBulList;         
    sal_Bool bParaPossible;         
    sal_Bool bOutPara;              
    sal_Bool bOutDiv;               

    SwHTMLTxtCollOutputInfo() :
        pItemSet( 0 ),
        bInNumBulList( sal_False ),
        bParaPossible( sal_False ),
        bOutPara( sal_False ),
        bOutDiv( sal_False )
    {}

    ~SwHTMLTxtCollOutputInfo();

    sal_Bool HasParaToken() const { return aToken.getLength()==1 && aToken[0]=='P'; }
    sal_Bool ShouldOutputToken() const { return bOutPara || !HasParaToken(); }
};

SwHTMLTxtCollOutputInfo::~SwHTMLTxtCollOutputInfo()
{
    delete pItemSet;
}

SwHTMLFmtInfo::SwHTMLFmtInfo( const SwFmt *pF, SwDoc *pDoc, SwDoc *pTemplate,
                              sal_Bool bOutStyles,
                              LanguageType eDfltLang,
                              sal_uInt16 nCSS1Script, sal_Bool bHardDrop ) :
    pFmt( pF ), pRefFmt(0), pItemSet( 0 ), bScriptDependent( false )
{
    sal_uInt16 nRefPoolId = 0;
    
    sal_uInt16 nDeep = SwHTMLWriter::GetCSS1Selector( pFmt, aToken, aClass,
                                                  nRefPoolId );
    OSL_ENSURE( nDeep ? !aToken.isEmpty() : aToken.isEmpty(),
            "Hier stimmt doch was mit dem Token nicht!" );
    OSL_ENSURE( nDeep ? nRefPoolId != 0 : nRefPoolId == 0,
            "Hier stimmt doch was mit der Vergleichs-Vorlage nicht!" );

    sal_Bool bTxtColl = pFmt->Which() == RES_TXTFMTCOLL ||
                    pFmt->Which() == RES_CONDTXTFMTCOLL;

    const SwFmt *pReferenceFmt = 0; 
    sal_Bool bSetDefaults = sal_True, bClearSame = sal_True;
    if( nDeep != 0 )
    {
        
        
        if( !bOutStyles )
        {
            
            
            switch( nDeep )
            {
            case CSS1_FMT_ISTAG:
            case CSS1_FMT_CMPREF:
                
                
                pReferenceFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId,
                                                        pTemplate );
                break;

            default:
                
                
                if( pTemplate )
                    pReferenceFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId,
                                                            pTemplate );
                else
                    pReferenceFmt = SwHTMLWriter::GetParentFmt( *pFmt, nDeep );
                break;
            }
        }
    }
    else if( bTxtColl )
    {
        
        
        
        
        if( !bOutStyles && pTemplate )
            pReferenceFmt = pTemplate->GetTxtCollFromPool( RES_POOLCOLL_TEXT, false );
        else
            pReferenceFmt = pDoc->GetTxtCollFromPool( RES_POOLCOLL_TEXT, false );
    }

    if( pReferenceFmt || nDeep==0 )
    {
        pItemSet = new SfxItemSet( *pFmt->GetAttrSet().GetPool(),
                                       pFmt->GetAttrSet().GetRanges() );
        
        
        

        pItemSet->Set( pFmt->GetAttrSet(), sal_True );

        if( pReferenceFmt )
            SwHTMLWriter::SubtractItemSet( *pItemSet, pReferenceFmt->GetAttrSet(),
                                           bSetDefaults, bClearSame );

        
        
        if( !pItemSet->Count() )
        {
            delete pItemSet;
            pItemSet = 0;
        }
    }

    if( bTxtColl )
    {
        if( bOutStyles )
        {
            
            
            static const sal_uInt16 aWhichIds[3][4] =
            {
                { RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                    RES_CHRATR_POSTURE, RES_CHRATR_WEIGHT },
                { RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE,
                    RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT },
                { RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE,
                    RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT }
            };

            sal_uInt16 nRef = 0;
            sal_uInt16 aSets[2] = {0,0};
            switch( nCSS1Script )
            {
            case CSS1_OUTMODE_WESTERN:
                nRef = 0;
                aSets[0] = 1;
                aSets[1] = 2;
                break;
            case CSS1_OUTMODE_CJK:
                nRef = 1;
                aSets[0] = 0;
                aSets[1] = 2;
                break;
            case CSS1_OUTMODE_CTL:
                nRef = 2;
                aSets[0] = 0;
                aSets[1] = 1;
                break;
            }
            for( sal_uInt16 i=0; i<4; i++ )
            {
                const SfxPoolItem& rRef = pFmt->GetFmtAttr( aWhichIds[nRef][i] );
                for( sal_uInt16 j=0; j<2; j++ )
                {
                    const SfxPoolItem& rSet = pFmt->GetFmtAttr( aWhichIds[aSets[j]][i] );
                    if( rSet != rRef )
                    {
                        if( !pItemSet )
                            pItemSet = new SfxItemSet( *pFmt->GetAttrSet().GetPool(),
                                                       pFmt->GetAttrSet().GetRanges() );
                        pItemSet->Put( rSet );
                    }
                }
            }
        }

        
        if( bOutStyles && bHardDrop && nDeep != 0 )
        {
            const SfxPoolItem *pItem;
            if( SFX_ITEM_SET==pFmt->GetAttrSet().GetItemState(
                                    RES_PARATR_DROP, true, &pItem ) )
            {
                sal_Bool bPut = sal_True;
                if( pTemplate )
                {
                    pReferenceFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId, pTemplate );
                    const SfxPoolItem *pRefItem;
                    sal_Bool bRefItemSet =
                        SFX_ITEM_SET==pReferenceFmt->GetAttrSet().GetItemState(
                                        RES_PARATR_DROP, true, &pRefItem );
                    bPut = !bRefItemSet || *pItem!=*pRefItem;
                }
                if( bPut )
                {
                    if( !pItemSet )
                        pItemSet = new SfxItemSet( *pFmt->GetAttrSet().GetPool(),
                                                   pFmt->GetAttrSet().GetRanges() );
                    pItemSet->Put( *pItem );
                }
            }
        }


        
        
        const SvxLRSpaceItem &rLRSpace =
            (pReferenceFmt ? pReferenceFmt : pFmt)->GetLRSpace();
        nLeftMargin = rLRSpace.GetTxtLeft();
        nRightMargin = rLRSpace.GetRight();
        nFirstLineIndent = rLRSpace.GetTxtFirstLineOfst();

        const SvxULSpaceItem &rULSpace =
            (pReferenceFmt ? pReferenceFmt : pFmt)->GetULSpace();
        nTopMargin = rULSpace.GetUpper();
        nBottomMargin = rULSpace.GetLower();

        
        sal_uInt16 nWhichId =
            SwHTMLWriter::GetLangWhichIdFromScript( nCSS1Script );
        const SvxLanguageItem& rLang =
            (const SvxLanguageItem&)pFmt->GetFmtAttr( nWhichId );
        LanguageType eLang = rLang.GetLanguage();
        if( eLang != eDfltLang )
        {
            if( !pItemSet )
                pItemSet = new SfxItemSet( *pFmt->GetAttrSet().GetPool(),
                                           pFmt->GetAttrSet().GetRanges() );
            pItemSet->Put( rLang );
        }

        static const sal_uInt16 aWhichIds[3] =
            { RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
                RES_CHRATR_CTL_LANGUAGE };
        for( sal_uInt16 i=0; i<3; i++ )
        {
            if( aWhichIds[i] != nWhichId )
            {
                const SvxLanguageItem& rTmpLang =
                    (const SvxLanguageItem&)pFmt->GetFmtAttr(aWhichIds[i]);
                if( rTmpLang.GetLanguage() != eLang )
                {
                    if( !pItemSet )
                        pItemSet = new SfxItemSet( *pFmt->GetAttrSet().GetPool(),
                                                   pFmt->GetAttrSet().GetRanges() );
                    pItemSet->Put( rTmpLang );
                }
            }
        }
    }
}

SwHTMLFmtInfo::~SwHTMLFmtInfo()
{
    delete pItemSet;
}

void OutHTML_SwFmt( Writer& rWrt, const SwFmt& rFmt,
                    const SfxItemSet *pNodeItemSet,
                    SwHTMLTxtCollOutputInfo& rInfo )
{
    OSL_ENSURE( RES_CONDTXTFMTCOLL==rFmt.Which() || RES_TXTFMTCOLL==rFmt.Which(),
            "keine Absatz-Vorlage" );

    SwHTMLWriter & rHWrt = (SwHTMLWriter&)rWrt;

    
    sal_uInt16 nNewDefListLvl = 0;
    sal_uInt16 nNumStart = USHRT_MAX;
    sal_Bool bForceDL = sal_False;
    sal_Bool bDT = sal_False;
    rInfo.bInNumBulList = sal_False;    
    sal_Bool bNumbered = sal_False;         
    sal_Bool bPara = sal_False;             
    rInfo.bParaPossible = sal_False;    
    sal_Bool bNoEndTag = sal_False;         

    rHWrt.bNoAlign = sal_False;         
    sal_Bool bNoStyle = sal_False;          
    sal_uInt8 nBulletGrfLvl = 255;      

    
    const SwTxtNode* pTxtNd = rWrt.pCurPam->GetNode()->GetTxtNode();

    SwHTMLNumRuleInfo aNumInfo;
    if( rHWrt.GetNextNumInfo() )
    {
        aNumInfo = *rHWrt.GetNextNumInfo();
        rHWrt.ClearNextNumInfo();
    }
    else
    {
        aNumInfo.Set( *pTxtNd );
    }

    if( aNumInfo.GetNumRule() )
    {
        rInfo.bInNumBulList = sal_True;
        nNewDefListLvl = 0;

        
        bNumbered = aNumInfo.IsNumbered();
        sal_uInt8 nLvl = aNumInfo.GetLevel();

        OSL_ENSURE( pTxtNd->GetActualListLevel() == nLvl,
                "Gemerkter Num-Level ist falsch" );
        OSL_ENSURE( bNumbered == static_cast< sal_Bool >(pTxtNd->IsCountedInList()),
                "Gemerkter Numerierungs-Zustand ist falsch" );

        if( bNumbered )
        {
            nBulletGrfLvl = nLvl; 
            
            
            
            
            
            if ( pTxtNd->IsListRestart() )
            {
                nNumStart = static_cast< sal_uInt16 >(pTxtNd->GetActualListStartValue());
            }
            OSL_ENSURE( rHWrt.nLastParaToken == 0,
                "<PRE> wurde nicht vor <LI> beendet." );
        }
    }

    
    SwHTMLFmtInfo aFmtInfo( &rFmt );
    SwHTMLFmtInfo *pFmtInfo;
    SwHTMLFmtInfos::iterator it = rHWrt.aTxtCollInfos.find( aFmtInfo );
    if( it != rHWrt.aTxtCollInfos.end() )
    {
        pFmtInfo = &*it;
    }
    else
    {
        pFmtInfo = new SwHTMLFmtInfo( &rFmt, rWrt.pDoc, rHWrt.pTemplate,
                                      rHWrt.bCfgOutStyles, rHWrt.eLang,
                                      rHWrt.nCSS1Script,
                                      false );
        rHWrt.aTxtCollInfos.insert( pFmtInfo );
        if( rHWrt.aScriptParaStyles.count( rFmt.GetName() ) )
            ((SwHTMLFmtInfo *)pFmtInfo)->bScriptDependent = true;
    }

    
    sal_uInt16 nToken = 0;          
    sal_Bool bOutNewLine = sal_False;   
    if( !pFmtInfo->aToken.isEmpty() )
    {
        
        
        rInfo.aToken = pFmtInfo->aToken;

        
        switch( rInfo.aToken[0] )
        {
        case 'A': OSL_ENSURE( rInfo.aToken == OOO_STRING_SVTOOLS_HTML_address,
                            "Doch kein ADDRESS?" );
                    rInfo.bParaPossible = sal_True;
                    rHWrt.bNoAlign = sal_True;
                    break;

        case 'B': OSL_ENSURE( rInfo.aToken == OOO_STRING_SVTOOLS_HTML_blockquote,
                            "Doch kein BLOCKQUOTE?" );
                    rInfo.bParaPossible = sal_True;
                    rHWrt.bNoAlign = sal_True;
                    break;

        case 'P':   if( rInfo.aToken.getLength() == 1 )
                    {
                        bPara = sal_True;
                    }
                    else
                    {
                        OSL_ENSURE( rInfo.aToken == OOO_STRING_SVTOOLS_HTML_preformtxt,
                                "Doch kein PRE?" );
                        if( HTML_PREFORMTXT_ON == rHWrt.nLastParaToken )
                        {
                            bOutNewLine = sal_True;
                        }
                        else
                        {
                            nToken = HTML_PREFORMTXT_ON;
                            rHWrt.bNoAlign = sal_True;
                            bNoEndTag = sal_True;
                        }
                    }
                    break;

        case 'D': OSL_ENSURE( rInfo.aToken == OOO_STRING_SVTOOLS_HTML_dt ||
                            rInfo.aToken == OOO_STRING_SVTOOLS_HTML_dd,
                            "Doch kein DD/DT?" );
                    bDT = rInfo.aToken == OOO_STRING_SVTOOLS_HTML_dt;
                    rInfo.bParaPossible = !bDT;
                    rHWrt.bNoAlign = sal_True;
                    bForceDL = sal_True;
                    break;
        }
    }
    else
    {
        
        

        rInfo.aToken = OOO_STRING_SVTOOLS_HTML_parabreak;
        bPara = sal_True;
    }

    
    if( pFmtInfo->pItemSet )
    {
        OSL_ENSURE( !rInfo.pItemSet, "Wo kommt der Item-Set her?" );
        rInfo.pItemSet = new SfxItemSet( *pFmtInfo->pItemSet );
    }

    
    if( pNodeItemSet )
    {
        if( rInfo.pItemSet )
            rInfo.pItemSet->Put( *pNodeItemSet );
        else
            rInfo.pItemSet = new SfxItemSet( *pNodeItemSet );
    }

    
    const SvxULSpaceItem& rULSpace =
        pNodeItemSet ? ((const SvxULSpaceItem &)pNodeItemSet->Get(RES_UL_SPACE))
                     : rFmt.GetULSpace();


    if( (rHWrt.bOutHeader &&
         rWrt.pCurPam->GetPoint()->nNode.GetIndex() ==
            rWrt.pCurPam->GetMark()->nNode.GetIndex()) ||
         rHWrt.bOutFooter )
    {
        if( rHWrt.bCfgOutStyles )
        {
            SvxULSpaceItem aULSpaceItem( rULSpace );
            if( rHWrt.bOutHeader )
                aULSpaceItem.SetLower( rHWrt.nHeaderFooterSpace );
            else
                aULSpaceItem.SetUpper( rHWrt.nHeaderFooterSpace );

            if( !rInfo.pItemSet )
                rInfo.pItemSet = new SfxItemSet(
                                    *rFmt.GetAttrSet().GetPool(),
                                    RES_UL_SPACE, RES_UL_SPACE );
            rInfo.pItemSet->Put( aULSpaceItem );
        }
        rHWrt.bOutHeader = sal_False;
        rHWrt.bOutFooter = sal_False;
    }

    if( bOutNewLine )
    {
        
        
        rInfo.aToken = OString();   
        rWrt.Strm().WriteCharPtr( SAL_NEWLINE_STRING );

        return;
    }


    
    const SfxPoolItem* pAdjItem = 0;
    const SfxPoolItem* pItem;

    if( rInfo.pItemSet &&
        SFX_ITEM_SET == rInfo.pItemSet->GetItemState( RES_PARATR_ADJUST,
                                                      false, &pItem ) )
    {
        pAdjItem = pItem;
    }

    
    
    sal_Bool bUseParSpace = !rHWrt.bOutTable ||
                        (rWrt.pCurPam->GetPoint()->nNode.GetIndex() !=
                         rWrt.pCurPam->GetMark()->nNode.GetIndex());
    
    
    const SvxLRSpaceItem& rLRSpace =
        pNodeItemSet ? ((const SvxLRSpaceItem &)pNodeItemSet->Get(RES_LR_SPACE))
                     : rFmt.GetLRSpace();
    if( (!rHWrt.bCfgOutStyles || bForceDL) && !rInfo.bInNumBulList )
    {
        sal_Int32 nLeftMargin;
        if( bForceDL )
            nLeftMargin = rLRSpace.GetTxtLeft();
        else
            nLeftMargin = rLRSpace.GetTxtLeft() > pFmtInfo->nLeftMargin
                ? rLRSpace.GetTxtLeft() - pFmtInfo->nLeftMargin
                : 0;

        if( nLeftMargin > 0 && rHWrt.nDefListMargin > 0 )
        {
            nNewDefListLvl = static_cast< sal_uInt16 >((nLeftMargin + (rHWrt.nDefListMargin/2)) /
                                                    rHWrt.nDefListMargin);
            if( nNewDefListLvl == 0 && bForceDL && !bDT )
                nNewDefListLvl = 1;
        }
        else
        {
            
            
            
            nNewDefListLvl = (bForceDL&& !bDT) ? 1 : 0;
        }

        sal_Bool bIsNextTxtNode =
            rWrt.pDoc->GetNodes()[rWrt.pCurPam->GetPoint()->nNode.GetIndex()+1]
                     ->IsTxtNode();

        if( bForceDL && bDT )
        {
            
            
            nNewDefListLvl++;
        }
        else if( !nNewDefListLvl && !rHWrt.bCfgOutStyles && bPara &&
                 rULSpace.GetLower()==0 &&
                 ((bUseParSpace && bIsNextTxtNode) || rHWrt.nDefListLvl==1) &&
                 (!pAdjItem || SVX_ADJUST_LEFT==
                    ((const SvxAdjustItem *)pAdjItem)->GetAdjust()) )
        {
            
            nNewDefListLvl = 1;
            bDT = sal_True;
            rInfo.bParaPossible = sal_False;
            rHWrt.bNoAlign = sal_True;
        }
    }

    if( nNewDefListLvl != rHWrt.nDefListLvl )
        rHWrt.OutAndSetDefList( nNewDefListLvl );

    
    if( rInfo.bInNumBulList )
    {
        OSL_ENSURE( !rHWrt.nDefListLvl, "DL in OL geht nicht!" );
        OutHTML_NumBulListStart( rHWrt, aNumInfo );

        if( bNumbered )
        {
            if( !rHWrt.aBulletGrfs[nBulletGrfLvl].isEmpty()  )
                bNumbered = sal_False;
            else
                nBulletGrfLvl = 255;
        }
    }

    
    
    rHWrt.nDfltLeftMargin = pFmtInfo->nLeftMargin;
    rHWrt.nDfltRightMargin = pFmtInfo->nRightMargin;
    rHWrt.nDfltFirstLineIndent = pFmtInfo->nFirstLineIndent;

    if( rInfo.bInNumBulList )
    {
        if( !rHWrt.IsHTMLMode( HTMLMODE_LSPACE_IN_NUMBUL ) )
            rHWrt.nDfltLeftMargin = rLRSpace.GetTxtLeft();

        
        rHWrt.nFirstLineIndent = rLRSpace.GetTxtFirstLineOfst();
    }

    if( rInfo.bInNumBulList && bNumbered && bPara && !rHWrt.bCfgOutStyles )
    {
        
        rHWrt.nDfltTopMargin = 0;
        rHWrt.nDfltBottomMargin = 0;
    }
    else if( rHWrt.nDefListLvl && bPara )
    {
        
        rHWrt.nDfltTopMargin = 0;
        rHWrt.nDfltBottomMargin = 0;
    }
    else
    {
        rHWrt.nDfltTopMargin = pFmtInfo->nTopMargin;
        
        
        
        
        
        if( rHWrt.bCfgNetscape4 && !bUseParSpace )
            rHWrt.nDfltBottomMargin = rULSpace.GetLower();
        else
            rHWrt.nDfltBottomMargin = pFmtInfo->nBottomMargin;
    }

    if( rHWrt.nDefListLvl )
    {
        rHWrt.nLeftMargin =
            (rHWrt.nDefListLvl-1) * rHWrt.nDefListMargin;
    }

    if( rHWrt.bLFPossible )
        rHWrt.OutNewLine(); 
    rInfo.bOutPara = sal_False;

    
    rHWrt.ChangeParaToken( nToken );

    sal_Bool bHasParSpace = bUseParSpace && rULSpace.GetLower() > 0;

    
    if( rInfo.bInNumBulList && bNumbered )
    {
        OStringBuffer sOut;
        sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_li);
        if( USHRT_MAX != nNumStart )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_value).
                 append("=\"").append(static_cast<sal_Int32>(nNumStart)).append("\"");
        }
        sOut.append('>');
        rWrt.Strm().WriteCharPtr( sOut.getStr() );
    }

    if( rHWrt.nDefListLvl > 0 && !bForceDL )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), bDT ? OOO_STRING_SVTOOLS_HTML_dt : OOO_STRING_SVTOOLS_HTML_dd );
    }

    if( pAdjItem &&
        rHWrt.IsHTMLMode( HTMLMODE_NO_CONTROL_CENTERING ) &&
        rHWrt.HasControls() )
    {
        
        
        
        OStringBuffer sOut;
        sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_division);
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

        rHWrt.bTxtAttr = sal_False;
        rHWrt.bOutOpts = sal_True;
        OutHTML_SvxAdjust( rWrt, *pAdjItem );
        rWrt.Strm().WriteChar( '>' );
        pAdjItem = 0;
        rHWrt.bNoAlign = sal_False;
        rInfo.bOutDiv = sal_True;
        rHWrt.IncIndentLevel();
        rHWrt.bLFPossible = sal_True;
            rHWrt.OutNewLine();
    }

    
    
    
    
    OString aToken = rInfo.aToken;
    if( !rHWrt.bCfgOutStyles && rInfo.bParaPossible && !bPara &&
        (bHasParSpace || pAdjItem) )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rInfo.aToken.getStr() );
        aToken = OOO_STRING_SVTOOLS_HTML_parabreak;
        bPara = sal_True;
        rHWrt.bNoAlign = sal_False;
        bNoStyle = sal_False;
    }

    LanguageType eLang = rInfo.pItemSet
        ? ((const SvxLanguageItem&)rInfo.pItemSet->Get(SwHTMLWriter::GetLangWhichIdFromScript(rHWrt.nCSS1Script))).GetLanguage()
           : rHWrt.eLang;

    if( rInfo.pItemSet )
    {
        static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_LANGUAGE };

        for( sal_uInt16 i=0; i<3; i++ )
        {
            
            const SfxPoolItem *pTmpItem;
            if( SFX_ITEM_SET == rInfo.pItemSet->GetItemState( aWhichIds[i],
                        true, &pTmpItem ) &&
                ((const SvxLanguageItem *)pTmpItem)->GetLanguage() == eLang )
                rInfo.pItemSet->ClearItem( aWhichIds[i] );
        }
    }

    
    sal_uInt16 nDir = rHWrt.GetHTMLDirection(
            (pNodeItemSet ? static_cast < const SvxFrameDirectionItem& >(
                                    pNodeItemSet->Get( RES_FRAMEDIR ) )
                          : rFmt.GetFrmDir() ).GetValue() );

    
    
    
    
    
    
    
    
    
    
    if( !bPara ||
        (!rInfo.bInNumBulList && !rHWrt.nDefListLvl) ||
        (rInfo.bInNumBulList && !bNumbered) ||
        (!rHWrt.bCfgOutStyles &&
         (bHasParSpace || pAdjItem ||
          (eLang != LANGUAGE_DONTKNOW && eLang != rHWrt.eLang))) ||
        nDir != rHWrt.nDirection ||
        rHWrt.bCfgOutStyles )
    {
        
        rHWrt.bTxtAttr = sal_False;
        rHWrt.bOutOpts = sal_True;

        OStringBuffer sOut;
        sOut.append('<').append(aToken);

        if( eLang != LANGUAGE_DONTKNOW && eLang != rHWrt.eLang )
        {
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            rHWrt.OutLanguage( eLang );
        }

        if( nDir != rHWrt.nDirection )
        {
            if( !sOut.isEmpty() )
                rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            rHWrt.OutDirection( nDir );
        }

        if( rHWrt.bCfgOutStyles &&
            (!pFmtInfo->aClass.isEmpty() || pFmtInfo->bScriptDependent) )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_class)
                .append("=\"");
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            OUString aClass( pFmtInfo->aClass );
            if( pFmtInfo->bScriptDependent )
            {
                if( !aClass.isEmpty() )
                   aClass += "-";
                switch( rHWrt.nCSS1Script )
                {
                case CSS1_OUTMODE_WESTERN:
                    aClass += "western";
                    break;
                case CSS1_OUTMODE_CJK:
                    aClass += "cjk";
                    break;
                case CSS1_OUTMODE_CTL:
                    aClass += "ctl";
                    break;
                }
            }
            HTMLOutFuncs::Out_String( rWrt.Strm(), aClass,
                                      rHWrt.eDestEnc, &rHWrt.aNonConvertableCharacters );
            sOut.append('\"');
        }
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

        
        if( !rHWrt.bNoAlign && pAdjItem )
            OutHTML_SvxAdjust( rWrt, *pAdjItem );

        
        if( rHWrt.bCfgOutStyles && rInfo.pItemSet && !bNoStyle)
        {
            OutCSS1_ParaTagStyleOpt( rWrt, *rInfo.pItemSet );
        }

        rWrt.Strm().WriteChar( '>' );

        
        rInfo.bOutPara =
            bPara &&
            ( rHWrt.bCfgOutStyles ||
                (!rHWrt.bCfgOutStyles && bHasParSpace) );

        
        if( bNoEndTag )
            rInfo.aToken = OString();
    }

    if( nBulletGrfLvl != 255 )
    {
        OSL_ENSURE( aNumInfo.GetNumRule(), "Wo ist die Numerierung geblieben???" );
        OSL_ENSURE( nBulletGrfLvl < MAXLEVEL, "So viele Ebenen gibt's nicht" );
        const SwNumFmt& rNumFmt = aNumInfo.GetNumRule()->Get(nBulletGrfLvl);
        OutHTML_BulletImage( rWrt, OOO_STRING_SVTOOLS_HTML_image, rNumFmt.GetBrush() );
    }

    rHWrt.GetNumInfo() = aNumInfo;

    
    rHWrt.nDfltLeftMargin = 0;
    rHWrt.nDfltRightMargin = 0;
    rHWrt.nDfltFirstLineIndent = 0;
    rHWrt.nDfltTopMargin = 0;
    rHWrt.nDfltBottomMargin = 0;
    rHWrt.nLeftMargin = 0;
    rHWrt.nFirstLineIndent = 0;
}

void OutHTML_SwFmtOff( Writer& rWrt, const SwHTMLTxtCollOutputInfo& rInfo )
{
    SwHTMLWriter & rHWrt = (SwHTMLWriter&)rWrt;

    
    if( rInfo.aToken.isEmpty() )
    {
        rHWrt.FillNextNumInfo();
        const SwHTMLNumRuleInfo& rNextInfo = *rHWrt.GetNextNumInfo();
        
        if( rInfo.bInNumBulList )
        {

            const SwHTMLNumRuleInfo& rNRInfo = rHWrt.GetNumInfo();
            if( rNextInfo.GetNumRule() != rNRInfo.GetNumRule() ||
                rNextInfo.GetDepth() != rNRInfo.GetDepth() ||
                rNextInfo.IsNumbered() || rNextInfo.IsRestart() )
                rHWrt.ChangeParaToken( 0 );
            OutHTML_NumBulListEnd( rHWrt, rNextInfo );
        }
        else if( rNextInfo.GetNumRule() != 0 )
            rHWrt.ChangeParaToken( 0 );

        return;
    }

    if( rInfo.ShouldOutputToken() )
    {
        if( rHWrt.bLFPossible )
            rHWrt.OutNewLine( sal_True );

        
        
        
        
        if( rInfo.bParaPossible && rInfo.bOutPara )
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_parabreak, sal_False );

        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rInfo.aToken.getStr(),
                                    sal_False );
        rHWrt.bLFPossible =
            rInfo.aToken != OOO_STRING_SVTOOLS_HTML_dt &&
            rInfo.aToken != OOO_STRING_SVTOOLS_HTML_dd &&
            rInfo.aToken != OOO_STRING_SVTOOLS_HTML_li;
    }
    if( rInfo.bOutDiv )
    {
        rHWrt.DecIndentLevel();
        if( rHWrt.bLFPossible )
            rHWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_division, sal_False );
        rHWrt.bLFPossible = sal_True;
    }

    
    if( rInfo.bInNumBulList )
    {
        rHWrt.FillNextNumInfo();
        OutHTML_NumBulListEnd( rHWrt, *rHWrt.GetNextNumInfo() );
    }
}


class HTMLSttEndPos
{
    sal_Int32 nStart;
    sal_Int32 nEnd;
    SfxPoolItem* pItem;

public:

    HTMLSttEndPos( const SfxPoolItem& rItem, sal_Int32 nStt, sal_Int32 nE );
    ~HTMLSttEndPos();

    const SfxPoolItem *GetItem() const { return pItem; }

    void SetStart( sal_Int32 nStt ) { nStart = nStt; }
    sal_Int32 GetStart() const { return nStart; }

    sal_Int32 GetEnd() const { return nEnd; }
    void SetEnd( sal_Int32 nE ) { nEnd = nE; }
};

HTMLSttEndPos::HTMLSttEndPos( const SfxPoolItem& rItem, sal_Int32 nStt,
                                                        sal_Int32 nE ) :
    nStart( nStt ),
    nEnd( nE ),
    pItem( rItem.Clone() )
{}

HTMLSttEndPos::~HTMLSttEndPos()
{
    delete pItem;
}

typedef std::vector<HTMLSttEndPos *> _HTMLEndLst;

enum HTMLOnOffState { HTML_NOT_SUPPORTED,   
                      HTML_REAL_VALUE,      
                      HTML_ON_VALUE,        
                      HTML_OFF_VALUE,       
                      HTML_CHRFMT_VALUE,    
                      HTML_COLOR_VALUE,     
                      HTML_STYLE_VALUE,     
                      HTML_DROPCAP_VALUE,   
                      HTML_AUTOFMT_VALUE }; 


class HTMLEndPosLst
{
    _HTMLEndLst aStartLst;  
    _HTMLEndLst aEndLst;    
    std::deque<sal_Int32> aScriptChgLst;    
                                    
                                    
    
    
    ::std::vector<sal_uInt16> aScriptLst;

    SwDoc *pDoc;            
    SwDoc* pTemplate;       
    const Color* pDfltColor;
    std::set<OUString>& rScriptTxtStyles;    //

    sal_uLong nHTMLMode;
    sal_Bool bOutStyles : 1;    


    
    sal_uInt16 _FindStartPos( const HTMLSttEndPos *pPos ) const;
    sal_uInt16 _FindEndPos( const HTMLSttEndPos *pPos ) const;

    
    
    void _InsertItem( HTMLSttEndPos *pPos, sal_uInt16 nEndPos );
    void _RemoveItem( sal_uInt16 nEndPos );

    
    HTMLOnOffState GetHTMLItemState( const SfxPoolItem& rItem );

    
    sal_Bool ExistsOnTagItem( sal_uInt16 nWhich, sal_Int32 nPos );

    
    
    sal_Bool ExistsOffTagItem( sal_uInt16 nWhich, sal_Int32 nStartPos,
                                          sal_Int32 nEndPos );


    
    void FixSplittedItem( HTMLSttEndPos *pPos, sal_Int32 nNewEnd,
                            sal_uInt16 nStartPos );

    
    void InsertItem( const SfxPoolItem& rItem, sal_Int32 nStart,
                                               sal_Int32 nEnd );

    
    void SplitItem( const SfxPoolItem& rItem, sal_Int32 nStart,
                                              sal_Int32 nEnd );

    
    void InsertNoScript( const SfxPoolItem& rItem, sal_Int32 nStart,
                          sal_Int32 nEnd, SwHTMLFmtInfos& rFmtInfos,
                         sal_Bool bParaAttrs=sal_False );

    const SwHTMLFmtInfo *GetFmtInfo( const SwFmt& rFmt,
                                     SwHTMLFmtInfos& rFmtInfos );

public:

    HTMLEndPosLst( SwDoc *pDoc, SwDoc* pTemplate, const Color* pDfltColor,
                   sal_Bool bOutStyles, sal_uLong nHTMLMode,
                   const OUString& rText, std::set<OUString>& rStyles );
    ~HTMLEndPosLst();

    
    void Insert( const SfxPoolItem& rItem, sal_Int32 nStart,  sal_Int32 nEnd,
                 SwHTMLFmtInfos& rFmtInfos, sal_Bool bParaAttrs=sal_False );
    void Insert( const SfxItemSet& rItemSet, sal_Int32 nStart, sal_Int32 nEnd,
                 SwHTMLFmtInfos& rFmtInfos, sal_Bool bDeep,
                 sal_Bool bParaAttrs=sal_False );
    void Insert( const SwDrawFrmFmt& rFmt, sal_Int32 nPos,
                 SwHTMLFmtInfos& rFmtInfos );

    sal_uInt16 GetScriptAtPos( sal_Int32 nPos,
                               sal_uInt16 nWeak=CSS1_OUTMODE_ANY_SCRIPT );

    void OutStartAttrs( SwHTMLWriter& rHWrt, sal_Int32 nPos,
                        HTMLOutContext *pContext = 0 );
    void OutEndAttrs( SwHTMLWriter& rHWrt, sal_Int32 nPos,
                      HTMLOutContext *pContext = 0 );

    sal_uInt16 Count() const { return aEndLst.size(); }

    sal_Bool IsHTMLMode( sal_uLong nMode ) const { return (nHTMLMode & nMode) != 0; }
};


sal_uInt16 HTMLEndPosLst::_FindStartPos( const HTMLSttEndPos *pPos ) const
{
    sal_uInt16 i;
    for( i = 0; i < aStartLst.size() && aStartLst[i] != pPos;  i++ )
        ;

    OSL_ENSURE(i != aStartLst.size(), "Item not found in Start List!" );

    return i==aStartLst.size() ? USHRT_MAX : i;
}

sal_uInt16 HTMLEndPosLst::_FindEndPos( const HTMLSttEndPos *pPos ) const
{
    sal_uInt16 i;

    for( i = 0; i < aEndLst.size() && aEndLst[i] != pPos;  i++ )
        ;

    OSL_ENSURE(i != aEndLst.size(), "Item not found in End List!" );

    return i==aEndLst.size() ? USHRT_MAX : i;
}


void HTMLEndPosLst::_InsertItem( HTMLSttEndPos *pPos, sal_uInt16 nEndPos )
{
    
    
    sal_Int32 nStart = pPos->GetStart();
    sal_uInt16 i;

    for( i = 0; i < aStartLst.size() &&
                     aStartLst[i]->GetStart() <= nStart; i++ )
        ;
    aStartLst.insert( aStartLst.begin() + i, pPos );

    
    aEndLst.insert( aEndLst.begin() + nEndPos, pPos );
}

void HTMLEndPosLst::_RemoveItem( sal_uInt16 nEndPos )
{
    HTMLSttEndPos *pPos = aEndLst[nEndPos];

    
    sal_uInt16 nStartPos = _FindStartPos( pPos );
    if( nStartPos != USHRT_MAX )
        aStartLst.erase( aStartLst.begin() + nStartPos );

    aEndLst.erase( aEndLst.begin() + nEndPos );

    delete pPos;
}

HTMLOnOffState HTMLEndPosLst::GetHTMLItemState( const SfxPoolItem& rItem )
{
    HTMLOnOffState eState = HTML_NOT_SUPPORTED;
    switch( rItem.Which() )
    {
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CTL_POSTURE:
        switch( ((const SvxPostureItem&)rItem).GetPosture() )
        {
        case ITALIC_NORMAL:
            eState = HTML_ON_VALUE;
            break;
        case ITALIC_NONE:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_CROSSEDOUT:
        switch( ((const SvxCrossedOutItem&)rItem).GetStrikeout() )
        {
        case STRIKEOUT_SINGLE:
        case STRIKEOUT_DOUBLE:
            eState = HTML_ON_VALUE;
            break;
        case STRIKEOUT_NONE:
            eState = HTML_OFF_VALUE;
            break;
        default:
            ;
        }
        break;

    case RES_CHRATR_ESCAPEMENT:
        switch( (const SvxEscapement)
                        ((const SvxEscapementItem&)rItem).GetEnumValue() )
        {
        case SVX_ESCAPEMENT_SUPERSCRIPT:
        case SVX_ESCAPEMENT_SUBSCRIPT:
            eState = HTML_ON_VALUE;
            break;
        case SVX_ESCAPEMENT_OFF:
            eState = HTML_OFF_VALUE;
            break;
        default:
            ;
        }
        break;

    case RES_CHRATR_UNDERLINE:
        switch( ((const SvxUnderlineItem&)rItem).GetLineStyle() )
        {
        case UNDERLINE_SINGLE:
            eState = HTML_ON_VALUE;
            break;
        case UNDERLINE_NONE:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_OVERLINE:
    case RES_CHRATR_HIDDEN:
        if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;

    case RES_CHRATR_WEIGHT:
    case RES_CHRATR_CJK_WEIGHT:
    case RES_CHRATR_CTL_WEIGHT:
        switch( ((const SvxWeightItem&)rItem).GetWeight() )
        {
        case WEIGHT_BOLD:
            eState = HTML_ON_VALUE;
            break;
        case WEIGHT_NORMAL:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_BLINK:
        eState = ((const SvxBlinkItem&)rItem).GetValue() ? HTML_ON_VALUE
                                                         : HTML_OFF_VALUE;
        break;

    case RES_CHRATR_COLOR:
        eState = HTML_COLOR_VALUE;
        break;

    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_TXTATR_INETFMT:
        eState = HTML_REAL_VALUE;
        break;

    case RES_TXTATR_CHARFMT:
        eState = HTML_CHRFMT_VALUE;
        break;

    case RES_TXTATR_AUTOFMT:
        eState = HTML_AUTOFMT_VALUE;
        break;

    case RES_CHRATR_CASEMAP:
        eState = HTML_STYLE_VALUE;
        break;

    case RES_CHRATR_KERNING:
        if( IsHTMLMode(HTMLMODE_FULL_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;

    case RES_CHRATR_BACKGROUND:
        if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;

    case RES_PARATR_DROP:
        eState = HTML_DROPCAP_VALUE;
        break;

    case RES_CHRATR_BOX:
        if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;
    }

    return eState;
}

sal_Bool HTMLEndPosLst::ExistsOnTagItem( sal_uInt16 nWhich, sal_Int32 nPos )
{
    for( sal_uInt16 i=0; i<aStartLst.size(); i++ )
    {
        HTMLSttEndPos *pTest = aStartLst[i];

        if( pTest->GetStart() > nPos )
        {
            
            break;
        }
        else if( pTest->GetEnd() > nPos )
        {
            
            
            const SfxPoolItem *pItem = pTest->GetItem();
            if( pItem->Which() == nWhich &&
                HTML_ON_VALUE == GetHTMLItemState(*pItem) )
            {
                
                return sal_True;
            }
        }
    }

    return sal_False;
}

sal_Bool HTMLEndPosLst::ExistsOffTagItem( sal_uInt16 nWhich, sal_Int32 nStartPos,
                                      sal_Int32 nEndPos )
{
    if( nWhich != RES_CHRATR_CROSSEDOUT &&
        nWhich != RES_CHRATR_UNDERLINE &&
        nWhich != RES_CHRATR_BLINK )
    {
        return sal_False;
    }

    for( sal_uInt16 i=0; i<aStartLst.size(); i++ )
    {
        HTMLSttEndPos *pTest = aStartLst[i];

        if( pTest->GetStart() > nStartPos )
        {
            
            break;
        }
        else if( pTest->GetStart()==nStartPos &&
                 pTest->GetEnd()==nEndPos )
        {
            
            
            const SfxPoolItem *pItem = pTest->GetItem();
            sal_uInt16 nTstWhich = pItem->Which() ;
            if( (nTstWhich == RES_CHRATR_CROSSEDOUT ||
                 nTstWhich == RES_CHRATR_UNDERLINE ||
                 nTstWhich == RES_CHRATR_BLINK) &&
                HTML_OFF_VALUE == GetHTMLItemState(*pItem) )
            {
                
                
                return sal_True;
            }
        }
    }

    return sal_False;
}

void HTMLEndPosLst::FixSplittedItem( HTMLSttEndPos *pPos, sal_Int32 nNewEnd,
                                        sal_uInt16 nStartPos )
{
    
    pPos->SetEnd( nNewEnd );

    
    sal_uInt16 nEndPos = _FindEndPos( pPos );
    if( nEndPos != USHRT_MAX )
        aEndLst.erase( aEndLst.begin() + nEndPos );

    
    
    for( nEndPos=0; nEndPos < aEndLst.size() &&
                    aEndLst[nEndPos]->GetEnd() <= nNewEnd; nEndPos++ )
        ;
    aEndLst.insert( aEndLst.begin() + nEndPos, pPos );

    
    for( sal_uInt16 i=nStartPos+1; i<aStartLst.size(); i++ )
    {
        HTMLSttEndPos *pTest = aStartLst[i];
        sal_Int32 nTestEnd = pTest->GetEnd();
        if( pTest->GetStart() >= nNewEnd )
        {
            
            
            break;
        }
        else if( nTestEnd > nNewEnd )
        {
            
            

            
            pTest->SetEnd( nNewEnd );

            
            sal_uInt16 nEPos = _FindEndPos( pTest );
            if( nEPos != USHRT_MAX )
                aEndLst.erase( aEndLst.begin() + nEPos );

            
            
            aEndLst.insert( aEndLst.begin() + nEndPos, pTest );

            
            InsertItem( *pTest->GetItem(), nNewEnd, nTestEnd );
        }
    }
}


void HTMLEndPosLst::InsertItem( const SfxPoolItem& rItem, sal_Int32 nStart,
                                                          sal_Int32 nEnd )
{
    sal_uInt16 i;
    for( i = 0; i < aEndLst.size(); i++ )
    {
        HTMLSttEndPos *pTest = aEndLst[i];
        sal_Int32 nTestEnd = pTest->GetEnd();
        if( nTestEnd <= nStart )
        {
            
            continue;
        }
        else if( nTestEnd < nEnd )
        {
            
            
            _InsertItem( new HTMLSttEndPos( rItem, nStart, nTestEnd ), i );
            nStart = nTestEnd;
        }
        else
        {
            
            
            break;
        }
    }

    
    _InsertItem( new HTMLSttEndPos( rItem, nStart, nEnd ), i );
}



void HTMLEndPosLst::SplitItem( const SfxPoolItem& rItem, sal_Int32 nStart,
                                                           sal_Int32 nEnd )
{
    sal_uInt16 nWhich = rItem.Which();

    
    

    for( sal_uInt16 i=0; i<aStartLst.size(); i++ )
    {
        HTMLSttEndPos *pTest = aStartLst[i];
        sal_Int32 nTestStart = pTest->GetStart();
        sal_Int32 nTestEnd = pTest->GetEnd();

        if( nTestStart >= nEnd )
        {
            
            break;
        }
        else if( nTestEnd > nStart )
        {
            
            const SfxPoolItem *pItem = pTest->GetItem();

            
            
            if( pItem->Which() == nWhich &&
                HTML_ON_VALUE == GetHTMLItemState( *pItem ) )
            {
                bool bDelete = true;

                if( nTestStart < nStart )
                {
                    
                    
                    FixSplittedItem( pTest, nStart, i );
                    bDelete = false;
                }
                else
                {
                    
                    
                    
                    aStartLst.erase( aStartLst.begin() + i );
                    i--;

                    sal_uInt16 nEndPos = _FindEndPos( pTest );
                    if( nEndPos != USHRT_MAX )
                        aEndLst.erase( aEndLst.begin() + nEndPos );
                }

                
                if( nTestEnd > nEnd )
                {
                    InsertItem( *pTest->GetItem(), nEnd, nTestEnd );
                }

                if( bDelete )
                    delete pTest;
            }
        }
    }
}

const SwHTMLFmtInfo *HTMLEndPosLst::GetFmtInfo( const SwFmt& rFmt,
                                                SwHTMLFmtInfos& rFmtInfos )
{
    SwHTMLFmtInfo *pFmtInfo;
    const SwHTMLFmtInfo aFmtInfo( &rFmt );
    SwHTMLFmtInfos::iterator it = rFmtInfos.find( aFmtInfo );
    if( it != rFmtInfos.end() )
    {
        pFmtInfo = &*it;
    }
    else
    {
        pFmtInfo = new SwHTMLFmtInfo( &rFmt, pDoc, pTemplate,
                                      bOutStyles );
        rFmtInfos.insert( pFmtInfo );
        if ( rScriptTxtStyles.count( rFmt.GetName() ) )
            ((SwHTMLFmtInfo *)pFmtInfo)->bScriptDependent = true;
    }

    return pFmtInfo;
}

HTMLEndPosLst::HTMLEndPosLst( SwDoc *pD, SwDoc* pTempl,
                              const Color* pDfltCol, sal_Bool bStyles,
                              sal_uLong nMode, const OUString& rText,
                              std::set<OUString>& rStyles ):
    pDoc( pD ),
    pTemplate( pTempl ),
    pDfltColor( pDfltCol ),
    rScriptTxtStyles( rStyles ),
    nHTMLMode( nMode ),
    bOutStyles( bStyles )
{
    sal_Int32 nEndPos = rText.getLength();
    sal_Int32 nPos = 0;
    while( nPos < nEndPos )
    {
        sal_uInt16 nScript = g_pBreakIt->GetBreakIter()->getScriptType( rText, nPos );
        nPos = g_pBreakIt->GetBreakIter()->endOfScript( rText, nPos, nScript );
        aScriptChgLst.push_back( nPos );
        aScriptLst.push_back( nScript );
    }
}

HTMLEndPosLst::~HTMLEndPosLst()
{
    OSL_ENSURE(aStartLst.empty(), "Start List not empty in destructor");
    OSL_ENSURE(aEndLst.empty(), "End List not empty in destructor");
}



void HTMLEndPosLst::InsertNoScript( const SfxPoolItem& rItem,
                            sal_Int32 nStart, sal_Int32 nEnd,
                            SwHTMLFmtInfos& rFmtInfos, sal_Bool bParaAttrs )
{
    
    if( nStart != nEnd )
    {
        sal_Bool bSet = sal_False, bSplit = sal_False;
        switch( GetHTMLItemState(rItem) )
        {
        case HTML_ON_VALUE:
            
            
            if( !ExistsOnTagItem( rItem.Which(), nStart ) )
                bSet = sal_True;
            break;

        case HTML_OFF_VALUE:
            
            
            
            
            if( ExistsOnTagItem( rItem.Which(), nStart ) )
                bSplit = sal_True;
            bSet = bOutStyles && !bParaAttrs &&
                   !ExistsOffTagItem( rItem.Which(), nStart, nEnd );
            break;

        case HTML_REAL_VALUE:
            
            bSet = sal_True;
            break;

        case HTML_STYLE_VALUE:
            
            
            
            
            
            bSet = bOutStyles &&
                   (!bParaAttrs
                  || rItem.Which()==RES_CHRATR_BACKGROUND
                  || rItem.Which()==RES_CHRATR_BOX
                  || rItem.Which()==RES_CHRATR_OVERLINE);
            break;

        case HTML_CHRFMT_VALUE:
            {
                OSL_ENSURE( RES_TXTATR_CHARFMT == rItem.Which(),
                        "Doch keine Zeichen-Vorlage" );
                const SwFmtCharFmt& rChrFmt = (const SwFmtCharFmt&)rItem;
                const SwCharFmt* pFmt = rChrFmt.GetCharFmt();

                const SwHTMLFmtInfo *pFmtInfo = GetFmtInfo( *pFmt, rFmtInfos );
                if( !pFmtInfo->aToken.isEmpty() )
                {
                    
                    
                    InsertItem( rItem, nStart, nEnd );
                }
                if( pFmtInfo->pItemSet )
                {
                    Insert( *pFmtInfo->pItemSet, nStart, nEnd,
                            rFmtInfos, sal_True, bParaAttrs );
                }
            }
            break;

        case HTML_AUTOFMT_VALUE:
            {
                const SwFmtAutoFmt& rAutoFmt = (const SwFmtAutoFmt&)rItem;
                const boost::shared_ptr<SfxItemSet> pSet = rAutoFmt.GetStyleHandle();
                if( pSet.get() )
                    Insert( *pSet.get(), nStart, nEnd, rFmtInfos, sal_True, bParaAttrs );
            }
            break;

        case HTML_COLOR_VALUE:
            
            
            {
                OSL_ENSURE( RES_CHRATR_COLOR == rItem.Which(),
                        "Doch keine Vordergrund-Farbe" );
                Color aColor( ((const SvxColorItem&)rItem).GetValue() );
                if( COL_AUTO == aColor.GetColor() )
                    aColor.SetColor( COL_BLACK );
                bSet = !bParaAttrs || !pDfltColor ||
                       !pDfltColor->IsRGBEqual( aColor );
            }
            break;

        case HTML_DROPCAP_VALUE:
            {
                OSL_ENSURE( RES_PARATR_DROP == rItem.Which(),
                        "Doch kein Drop-Cap" );
                const SwFmtDrop& rDrop = (const SwFmtDrop&)rItem;
                nEnd = nStart + rDrop.GetChars();
                if( !bOutStyles )
                {
                    
                    const SwCharFmt *pCharFmt = rDrop.GetCharFmt();
                    if( pCharFmt )
                    {
                        Insert( pCharFmt->GetAttrSet(), nStart, nEnd,
                                rFmtInfos, sal_True, bParaAttrs );
                    }
                }
                else
                {
                    bSet = sal_True;
                }
            }
            break;
        default:
            ;
        }

        if( bSet )
            InsertItem( rItem, nStart, nEnd );
        if( bSplit )
            SplitItem( rItem, nStart, nEnd );
    }
}

void HTMLEndPosLst::Insert( const SfxPoolItem& rItem,
                            sal_Int32 nStart, sal_Int32 nEnd,
                            SwHTMLFmtInfos& rFmtInfos, sal_Bool bParaAttrs )
{
    sal_Bool bDependsOnScript = sal_False, bDependsOnAnyScript = sal_False;
    sal_uInt16 nScript = i18n::ScriptType::LATIN;
    switch( rItem.Which() )
    {
    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
        bDependsOnScript = sal_True;
        nScript = i18n::ScriptType::LATIN;
        break;

    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
        bDependsOnScript = sal_True;
        nScript = i18n::ScriptType::ASIAN;
        break;

    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        bDependsOnScript = sal_True;
        nScript = i18n::ScriptType::COMPLEX;
        break;
    case RES_TXTATR_CHARFMT:
        {
            const SwFmtCharFmt& rChrFmt = (const SwFmtCharFmt&)rItem;
            const SwCharFmt* pFmt = rChrFmt.GetCharFmt();
            const SwHTMLFmtInfo *pFmtInfo = GetFmtInfo( *pFmt, rFmtInfos );
            if( pFmtInfo->bScriptDependent )
            {
                bDependsOnScript = sal_True;
                bDependsOnAnyScript = sal_True;
            }
        }
        break;
    case RES_TXTATR_INETFMT:
        {
            if( GetFmtInfo( *pDoc->GetCharFmtFromPool(
                     RES_POOLCHR_INET_NORMAL), rFmtInfos )->bScriptDependent ||
                GetFmtInfo( *pDoc->GetCharFmtFromPool(
                     RES_POOLCHR_INET_VISIT), rFmtInfos )->bScriptDependent )
            {
                bDependsOnScript = sal_True;
                bDependsOnAnyScript = sal_True;
            }
        }
        break;
    }

    if( bDependsOnScript )
    {
        sal_Int32 nPos = nStart;
        for( size_t i=0; i < aScriptChgLst.size(); i++ )
        {
            sal_Int32 nChgPos = aScriptChgLst[i];
            if( nPos >= nChgPos )
            {
                
                
                continue;
            }
            if( nEnd <= nChgPos )
            {
                
                
                
                if( bDependsOnAnyScript || nScript == aScriptLst[i] )
                    InsertNoScript( rItem, nPos, nEnd, rFmtInfos,
                                    bParaAttrs );
                break;
            }

            
            
            
            if( bDependsOnAnyScript || nScript == aScriptLst[i] )
                InsertNoScript( rItem, nPos, nChgPos, rFmtInfos, bParaAttrs );
            nPos = nChgPos;
        }
    }
    else
    {
        InsertNoScript( rItem, nStart, nEnd, rFmtInfos, bParaAttrs );
    }
}

void HTMLEndPosLst::Insert( const SfxItemSet& rItemSet,
                            sal_Int32 nStart, sal_Int32 nEnd,
                            SwHTMLFmtInfos& rFmtInfos,
                            sal_Bool bDeep, sal_Bool bParaAttrs )
{
    SfxWhichIter aIter( rItemSet );

    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( nWhich, bDeep, &pItem ) )
        {
            Insert( *pItem, nStart, nEnd, rFmtInfos, bParaAttrs );
        }

        nWhich = aIter.NextWhich();
    }
}

void HTMLEndPosLst::Insert( const SwDrawFrmFmt& rFmt, sal_Int32 nPos,
                            SwHTMLFmtInfos& rFmtInfos )
{
    
    
    const SdrObject* pTextObj =
        (const SdrObject *)SwHTMLWriter::GetMarqueeTextObj( rFmt );

    if( pTextObj )
    {
        
        
        
        const SfxItemSet& rFmtItemSet = rFmt.GetAttrSet();
        SfxItemSet aItemSet( *rFmtItemSet.GetPool(), RES_CHRATR_BEGIN,
                                                     RES_CHRATR_END );
        SwHTMLWriter::GetEEAttrsFromDrwObj( aItemSet, pTextObj, sal_True );
        sal_Bool bOutStylesOld = bOutStyles;
        bOutStyles = sal_False;
        Insert( aItemSet, nPos, nPos+1, rFmtInfos, sal_False, sal_False );
        bOutStyles = bOutStylesOld;
    }
}

sal_uInt16 HTMLEndPosLst::GetScriptAtPos( sal_Int32 nPos ,
                                             sal_uInt16 nWeak )
{
    sal_uInt16 nRet = CSS1_OUTMODE_ANY_SCRIPT;

    size_t nScriptChgs = aScriptChgLst.size();
    size_t i=0;
    while( i < nScriptChgs && nPos >= aScriptChgLst[i] )
        i++;
    OSL_ENSURE( i < nScriptChgs, "script list is to short" );
    if( i < nScriptChgs )
    {
        if( i18n::ScriptType::WEAK == aScriptLst[i] )
            nRet = nWeak;
        else
            nRet = SwHTMLWriter::GetCSS1ScriptForScriptType( aScriptLst[i] );
    }

    return nRet;
}

void HTMLEndPosLst::OutStartAttrs( SwHTMLWriter& rHWrt, sal_Int32 nPos,
                                      HTMLOutContext *pContext  )
{
    rHWrt.bTagOn = sal_True;

    
    
    sal_uInt16 nCharBoxIndex = 0;
    while( nCharBoxIndex < aStartLst.size() &&
           aStartLst[nCharBoxIndex]->GetItem()->Which() != RES_CHRATR_BOX )
    {
        ++nCharBoxIndex;
    }

    
    for( sal_uInt16 i=0; i< aStartLst.size(); i++ )
    {
        HTMLSttEndPos *pPos = 0;
        if( nCharBoxIndex < aStartLst.size() )
        {
            if( i == 0 )
                pPos = aStartLst[nCharBoxIndex];
            else if( i == nCharBoxIndex )
                pPos = aStartLst[0];
            else
                pPos = aStartLst[i];
        }
        else
            pPos = aStartLst[i];

        sal_Int32 nStart = pPos->GetStart();
        if( nStart > nPos )
        {
            
            break;
        }
        else if( nStart == nPos )
        {
            
            sal_uInt16 nCSS1Script = rHWrt.nCSS1Script;
            sal_uInt16 nWhich = pPos->GetItem()->Which();
            if( RES_TXTATR_CHARFMT == nWhich ||
                RES_TXTATR_INETFMT == nWhich ||
                 RES_PARATR_DROP == nWhich )
            {
                rHWrt.nCSS1Script = GetScriptAtPos( nPos, nCSS1Script );
            }
            if( pContext )
            {
                HTMLOutFuncs::FlushToAscii( rHWrt.Strm(), *pContext );
                pContext = 0; 
            }
            Out( aHTMLAttrFnTab, *pPos->GetItem(), rHWrt );
            rHWrt.nCSS1Script = nCSS1Script;
        }
    }
}

void HTMLEndPosLst::OutEndAttrs( SwHTMLWriter& rHWrt, sal_Int32 nPos,
                                     HTMLOutContext *pContext )
{
    rHWrt.bTagOn = sal_False;

    
    sal_uInt16 i=0;
    while( i < aEndLst.size() )
    {
        HTMLSttEndPos *pPos = aEndLst[i];
        sal_Int32 nEnd = pPos->GetEnd();

        if( SAL_MAX_INT32 == nPos || nEnd == nPos )
        {
            if( pContext )
            {
                HTMLOutFuncs::FlushToAscii( rHWrt.Strm(), *pContext );
                pContext = 0; 
            }
            
            bool bSkipOut = false;
            if( pPos->GetItem()->Which() == RES_CHRATR_BOX )
            {
                for(sal_uInt16 nIndex = _FindStartPos(pPos) + 1; nIndex < aStartLst.size(); ++nIndex )
                {
                    HTMLSttEndPos *pEndPos = aStartLst[nIndex];
                    if( pEndPos->GetItem()->Which() == RES_CHRATR_BOX &&
                        *static_cast<const SvxBoxItem*>(pEndPos->GetItem()) ==
                        *static_cast<const SvxBoxItem*>(pPos->GetItem()) )
                    {
                        pEndPos->SetStart(pPos->GetStart());
                        bSkipOut = true;
                        break;
                    }
                }
            }
            if( !bSkipOut )
            {
                Out( aHTMLAttrFnTab, *pPos->GetItem(), rHWrt );
            }
            _RemoveItem( i );
        }
        else if( nEnd > nPos )
        {
            
            break;
        }
        else
        {
            
            
            OSL_ENSURE( nEnd >= nPos,
                    "Das Attribut sollte schon laengst beendet sein" );
            i++;
        }
    }
}


/* Ausgabe der Nodes */
Writer& OutHTML_SwTxtNode( Writer& rWrt, const SwCntntNode& rNode )
{
    SwTxtNode * pNd = &((SwTxtNode&)rNode);
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const OUString& rStr = pNd->GetTxt();
    sal_Int32 nEnde = rStr.getLength();

    
    
    sal_uInt16 nPoolId = pNd->GetAnyFmtColl().GetPoolFmtId();

    if( !nEnde && (RES_POOLCOLL_HTML_HR==nPoolId ||
                   pNd->GetAnyFmtColl().GetName() == OOO_STRING_SVTOOLS_HTML_horzrule) )
    {
        
        
        
        
        rHTMLWrt.ChangeParaToken( 0 );

        
        rHTMLWrt.OutFlyFrm( rNode.GetIndex(), 0, HTML_POS_ANY );

        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine(); 

        rHTMLWrt.bLFPossible = sal_True;

        OStringBuffer sOut;
        sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_horzrule);

        const SfxItemSet* pItemSet = pNd->GetpSwAttrSet();
        if( !pItemSet )
        {
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() ).WriteChar( '>' );
            return rHTMLWrt;
        }
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pItemSet->GetItemState( RES_LR_SPACE, false, &pItem ))
        {
            sal_Int32 nLeft = ((SvxLRSpaceItem*)pItem)->GetLeft();
            sal_Int32 nRight = ((SvxLRSpaceItem*)pItem)->GetRight();
            if( nLeft || nRight )
            {
                const SwFrmFmt& rPgFmt =
                    rHTMLWrt.pDoc->GetPageDescFromPool
                    ( RES_POOLPAGE_HTML, false )->GetMaster();
                const SwFmtFrmSize& rSz   = rPgFmt.GetFrmSize();
                const SvxLRSpaceItem& rLR = rPgFmt.GetLRSpace();
                const SwFmtCol& rCol = rPgFmt.GetCol();

                long nPageWidth = rSz.GetWidth() - rLR.GetLeft() - rLR.GetRight();

                if( 1 < rCol.GetNumCols() )
                    nPageWidth /= rCol.GetNumCols();

                const SwTableNode* pTblNd = pNd->FindTableNode();
                if( pTblNd )
                {
                    const SwTableBox* pBox = pTblNd->GetTable().GetTblBox(
                                    pNd->StartOfSectionIndex() );
                    if( pBox )
                        nPageWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
                }

                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width)
                    .append("=\"");
                rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
                rWrt.OutULong( rHTMLWrt.ToPixel(nPageWidth-nLeft-nRight,false) ).WriteCharPtr( "\"" );

                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align)
                    .append("=\"");
                if( !nLeft )
                    sOut.append(OOO_STRING_SVTOOLS_HTML_AL_left);
                else if( !nRight )
                    sOut.append(OOO_STRING_SVTOOLS_HTML_AL_right);
                else
                    sOut.append(OOO_STRING_SVTOOLS_HTML_AL_center);
                sOut.append("\"");
            }
        }
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        if( SFX_ITEM_SET == pItemSet->GetItemState( RES_BOX, false, &pItem ))
        {
            const SvxBoxItem* pBoxItem = (const SvxBoxItem*)pItem;
            const editeng::SvxBorderLine* pBorderLine = pBoxItem->GetBottom();
            if( pBorderLine )
            {
                sal_uInt16 nWidth = pBorderLine->GetScaledWidth();
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_size)
                    .append("=\"");
                rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
                rWrt.OutULong( rHTMLWrt.ToPixel(nWidth,false) ).WriteCharPtr( "\"" );

                const Color& rBorderColor = pBorderLine->GetColor();
                if( !rBorderColor.IsRGBEqual( Color(COL_GRAY) ) )
                {
                    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_color)
                        .append("=");
                    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
                    HTMLOutFuncs::Out_Color( rWrt.Strm(), rBorderColor,
                                             rHTMLWrt.eDestEnc );
                }

                if( !pBorderLine->GetInWidth() )
                {
                    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_noshade);
                    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
                }
            }
        }
        rWrt.Strm().WriteChar( '>' );
        return rHTMLWrt;
    }

    
    
    
    
    if( !nEnde && (nPoolId == RES_POOLCOLL_STANDARD ||
                   nPoolId == RES_POOLCOLL_TABLE ||
                   nPoolId == RES_POOLCOLL_TABLE_HDLN) )
    {
        
        const SfxPoolItem* pItem;
        const SfxItemSet *pItemSet = pNd->GetpSwAttrSet();
        if( pItemSet && pItemSet->Count() &&
            SFX_ITEM_SET == pItemSet->GetItemState( RES_CHRATR_FONTSIZE, false, &pItem ) &&
            40 == ((const SvxFontHeightItem *)pItem)->GetHeight() )
        {
            
            sal_uLong nNdPos = rWrt.pCurPam->GetPoint()->nNode.GetIndex();
            const SwNode *pNextNd = rWrt.pDoc->GetNodes()[nNdPos+1];
            const SwNode *pPrevNd = rWrt.pDoc->GetNodes()[nNdPos-1];
            sal_Bool bStdColl = nPoolId == RES_POOLCOLL_STANDARD;
            if( ( bStdColl && (pNextNd->IsTableNode() ||
                               pNextNd->IsSectionNode()) ) ||
                ( !bStdColl && pNextNd->IsEndNode() &&
                               pPrevNd->IsStartNode() &&
                               SwTableBoxStartNode==
                                pPrevNd->GetStartNode()->GetStartNodeType() ) )
            {
                
                rHTMLWrt.OutBookmarks();
                rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken;

                
                rHTMLWrt.OutFlyFrm( rNode.GetIndex(), 0, HTML_POS_ANY );
                rHTMLWrt.bLFPossible = sal_False;

                return rWrt;
            }
        }
    }

    
    sal_Bool bPageBreakBehind = sal_False;
    if( rHTMLWrt.bCfgFormFeed &&
        !(rHTMLWrt.bOutTable || rHTMLWrt.bOutFlyFrame) &&
        rHTMLWrt.pStartNdIdx->GetIndex() !=
        rHTMLWrt.pCurPam->GetPoint()->nNode.GetIndex() )
    {
        sal_Bool bPageBreakBefore = sal_False;
        const SfxPoolItem* pItem;
        const SfxItemSet* pItemSet = pNd->GetpSwAttrSet();

        if( pItemSet )
        {
            if( SFX_ITEM_SET ==
                pItemSet->GetItemState( RES_PAGEDESC, true, &pItem ) &&
                ((SwFmtPageDesc *)pItem)->GetPageDesc() )
                bPageBreakBefore = sal_True;
            else if( SFX_ITEM_SET ==
                     pItemSet->GetItemState( RES_BREAK, true, &pItem ) )
            {
                switch( ((SvxFmtBreakItem *)pItem)->GetBreak() )
                {
                case SVX_BREAK_PAGE_BEFORE:
                    bPageBreakBefore = sal_True;
                    break;
                case SVX_BREAK_PAGE_AFTER:
                    bPageBreakBehind = sal_True;
                    break;
                case SVX_BREAK_PAGE_BOTH:
                    bPageBreakBefore = sal_True;
                    bPageBreakBehind = sal_True;
                    break;
                default:
                    ;
                }
            }
        }

        if( bPageBreakBefore )
            rWrt.Strm().WriteChar( '\f' );
    }

    
    rHTMLWrt.OutForm();

    
    sal_Bool bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                         0, HTML_POS_PREFIX );
    
    
    if( bFlysLeft )
        bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                        0, HTML_POS_BEFORE );

    if( rHTMLWrt.pCurPam->GetPoint()->nNode == rHTMLWrt.pCurPam->GetMark()->nNode )
        nEnde = rHTMLWrt.pCurPam->GetMark()->nContent.GetIndex();

    
    rHTMLWrt.bTagOn = sal_True;

    
    const SwFmt& rFmt = pNd->GetAnyFmtColl();
    SwHTMLTxtCollOutputInfo aFmtInfo;
    sal_Bool bOldLFPossible = rHTMLWrt.bLFPossible;
    OutHTML_SwFmt( rWrt, rFmt, pNd->GetpSwAttrSet(), aFmtInfo );

    
    
    rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken;
    if( !bOldLFPossible && rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();


    
    rHTMLWrt.bOutOpts = sal_False;
    rHTMLWrt.OutBookmarks();

    
    
    if( rHTMLWrt.bLFPossible &&
        rHTMLWrt.GetLineLen() >= rHTMLWrt.nWhishLineLen )
    {
        rHTMLWrt.OutNewLine();
    }
    rHTMLWrt.bLFPossible = sal_False;

    
    sal_Int32 nOffset = 0;
    OUString aOutlineTxt;
    OUString aFullText;

    
    
    if ( pNd->IsOutline() &&
         pNd->GetNumRule() == pNd->GetDoc()->GetOutlineNumRule() )
    {
        aOutlineTxt = pNd->GetNumString();
        nOffset = nOffset + aOutlineTxt.getLength();
        aFullText = aOutlineTxt;
    }
    OUString aFootEndNoteSym;
    if( rHTMLWrt.pFmtFtn )
    {
        aFootEndNoteSym = rHTMLWrt.GetFootEndNoteSym( *rHTMLWrt.pFmtFtn );
        nOffset = nOffset + aFootEndNoteSym.getLength();
        aFullText += aFootEndNoteSym;
    }

    
    aFullText += rStr;
    HTMLEndPosLst aEndPosLst( rWrt.pDoc, rHTMLWrt.pTemplate,
                              rHTMLWrt.pDfltColor, rHTMLWrt.bCfgOutStyles,
                              rHTMLWrt.GetHTMLMode(), aFullText,
                                 rHTMLWrt.aScriptTextStyles );
    if( aFmtInfo.pItemSet )
    {
        aEndPosLst.Insert( *aFmtInfo.pItemSet, 0, nEnde + nOffset,
                           rHTMLWrt.aChrFmtInfos, sal_False, sal_True );
    }


    if( !aOutlineTxt.isEmpty() || rHTMLWrt.pFmtFtn )
    {
        
        
        aEndPosLst.OutStartAttrs( rHTMLWrt, 0 );

        
        
        

        if( !aOutlineTxt.isEmpty() )
            HTMLOutFuncs::Out_String( rWrt.Strm(), aOutlineTxt,
                                         rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters);

        if( rHTMLWrt.pFmtFtn )
        {
            rHTMLWrt.OutFootEndNoteSym( *rHTMLWrt.pFmtFtn, aFootEndNoteSym,
                                        aEndPosLst.GetScriptAtPos( aOutlineTxt.getLength(), rHTMLWrt.nCSS1Script ) );
            rHTMLWrt.pFmtFtn = 0;
        }
    }

    
    
    rHTMLWrt.bTxtAttr = sal_True;


    sal_uInt16 nAttrPos = 0;
    sal_Int32 nStrPos = rHTMLWrt.pCurPam->GetPoint()->nContent.GetIndex();
    const SwTxtAttr * pHt = 0;
    sal_uInt16 nCntAttr = pNd->HasHints() ? pNd->GetSwpHints().Count() : 0;
    if( nCntAttr && nStrPos > *( pHt = pNd->GetSwpHints()[ 0 ] )->GetStart() )
    {
        
        do {
            aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset );

            nAttrPos++;
            if( pHt->Which() == RES_TXTATR_FIELD
                || pHt->Which() == RES_TXTATR_ANNOTATION )
                continue;

            if ( pHt->End() && !pHt->HasDummyChar() )
            {
                const sal_Int32 nHtEnd = *pHt->End(),
                       nHtStt = *pHt->GetStart();
                if( !rHTMLWrt.bWriteAll && nHtEnd <= nStrPos )
                    continue;

                
                if( nHtEnd == nHtStt )
                    continue;

                
                if( rHTMLWrt.bWriteAll )
                    aEndPosLst.Insert( pHt->GetAttr(), nHtStt + nOffset,
                                       nHtEnd + nOffset,
                                       rHTMLWrt.aChrFmtInfos );
                else
                {
                    sal_Int32 nTmpStt = nHtStt < nStrPos ? nStrPos : nHtStt;
                    sal_Int32 nTmpEnd = nHtEnd < nEnde ? nHtEnd : nEnde;
                    aEndPosLst.Insert( pHt->GetAttr(), nTmpStt + nOffset,
                                       nTmpEnd + nOffset,
                                       rHTMLWrt.aChrFmtInfos );
                }
                continue;
                
            }

        } while( nAttrPos < nCntAttr && nStrPos >
            *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );

        
        aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset );
        aEndPosLst.OutStartAttrs( rHTMLWrt, nStrPos + nOffset );
    }

    sal_Bool bWriteBreak = (HTML_PREFORMTXT_ON != rHTMLWrt.nLastParaToken);
    if( bWriteBreak && pNd->GetNumRule()  )
        bWriteBreak = sal_False;

    {
        HTMLOutContext aContext( rHTMLWrt.eDestEnc );

        sal_Int32 nPreSplitPos = 0;
        for( ; nStrPos < nEnde; nStrPos++ )
        {
            
            if( bFlysLeft )
            {
                aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset, &aContext );
                bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                                nStrPos, HTML_POS_INSIDE,
                                                &aContext );
            }

            sal_Bool bOutChar = sal_True;
            const SwTxtAttr * pTxtHt = 0;
            if( nAttrPos < nCntAttr && *pHt->GetStart() == nStrPos
                && nStrPos != nEnde )
            {
                do {
                    if ( pHt->End() && !pHt->HasDummyChar() )
                    {
                        if( *pHt->End() != nStrPos )
                        {
                            
                            
                            
                            aEndPosLst.Insert( pHt->GetAttr(), nStrPos + nOffset,
                                               *pHt->End() + nOffset,
                                               rHTMLWrt.aChrFmtInfos );
                        }
                    }
                    else
                    {
                        
                        OSL_ENSURE( !pTxtHt, "Wieso gibt es da schon ein Attribut ohne Ende?" );
                        if( rHTMLWrt.nTxtAttrsToIgnore>0 )
                        {
                            rHTMLWrt.nTxtAttrsToIgnore--;
                        }
                        else
                        {
                            pTxtHt = pHt;
                            sal_uInt16 nFldWhich;
                            if( RES_TXTATR_FIELD != pHt->Which()
                                || ( RES_POSTITFLD != (nFldWhich = ((const SwFmtFld&)pHt->GetAttr()).GetField()->Which())
                                     && RES_SCRIPTFLD != nFldWhich ) )
                            {
                                bWriteBreak = sal_False;
                            }
                        }
                        bOutChar = sal_False;       
                    }
                } while( ++nAttrPos < nCntAttr && nStrPos ==
                    *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );
            }

            
            if( pTxtHt && RES_TXTATR_FLYCNT == pTxtHt->Which() )
            {
                const SwFrmFmt* pFrmFmt =
                    ((const SwFmtFlyCnt &)pTxtHt->GetAttr()).GetFrmFmt();

                if( RES_DRAWFRMFMT == pFrmFmt->Which() )
                    aEndPosLst.Insert( *((const SwDrawFrmFmt *)pFrmFmt),
                                        nStrPos + nOffset,
                                        rHTMLWrt.aChrFmtInfos );
            }

            aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset, &aContext );
            aEndPosLst.OutStartAttrs( rHTMLWrt, nStrPos + nOffset, &aContext );

            if( pTxtHt )
            {
                rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken && nStrPos > 0 &&
                                       rStr[nStrPos-1] == ' ';
                sal_uInt16 nCSS1Script = rHTMLWrt.nCSS1Script;
                rHTMLWrt.nCSS1Script = aEndPosLst.GetScriptAtPos(
                                                nStrPos + nOffset, nCSS1Script );
                HTMLOutFuncs::FlushToAscii( rWrt.Strm(), aContext );
                Out( aHTMLAttrFnTab, pTxtHt->GetAttr(), rHTMLWrt );
                rHTMLWrt.nCSS1Script = nCSS1Script;
                rHTMLWrt.bLFPossible = sal_False;
            }

            if( bOutChar )
            {
                
                sal_uInt64 c = rStr[nStrPos];
                if( nStrPos < nEnde - 1 )
                {
                    const sal_Unicode d = rStr[nStrPos + 1];
                    if( (c >= 0xd800 && c <= 0xdbff) && (d >= 0xdc00 && d <= 0xdfff) )
                    {
                        sal_uInt64 templow = d&0x03ff;
                        sal_uInt64 temphi = ((c&0x03ff) + 0x0040)<<10;
                        c = temphi|templow;
                        nStrPos++;
                    }
                }

                
                
                if( ' '==c && !rHTMLWrt.nLastParaToken  )
                {
                    sal_Int32 nLineLen;
                    if( rHTMLWrt.nLastParaToken )
                        nLineLen = nStrPos - nPreSplitPos;
                    else
                        nLineLen = rHTMLWrt.GetLineLen();

                    sal_Int32 nWordLen = rStr.indexOf( ' ', nStrPos+1 );
                    if( nWordLen == -1 )
                        nWordLen = nEnde;
                    nWordLen -= nStrPos;

                    if( nLineLen >= rHTMLWrt.nWhishLineLen ||
                        (nLineLen+nWordLen) >= rHTMLWrt.nWhishLineLen )
                    {
                        HTMLOutFuncs::FlushToAscii( rWrt.Strm(), aContext );
                        rHTMLWrt.OutNewLine();
                        bOutChar = sal_False;
                        if( rHTMLWrt.nLastParaToken )
                            nPreSplitPos = nStrPos+1;
                    }
                }

                if( bOutChar )
                {
                    if( 0x0a == c )
                    {
                        HTMLOutFuncs::FlushToAscii( rWrt.Strm(), aContext );
                        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_linebreak );
                    }
                    
                    else if( c > 0xffff)
                    {
                        OString sOut("&#");
                        sOut += OString::number( (sal_uInt64)c );
                        sOut += ";";
                        rWrt.Strm().WriteCharPtr( sOut.getStr() );
                    }
                    else
                        HTMLOutFuncs::Out_Char( rWrt.Strm(), (sal_Unicode)c, aContext, &rHTMLWrt.aNonConvertableCharacters );

                    
                    
                    
                    bWriteBreak = (0x0a == c) &&
                                  (HTML_PREFORMTXT_ON != rHTMLWrt.nLastParaToken);
                }
            }
        }
        HTMLOutFuncs::FlushToAscii( rWrt.Strm(), aContext );
    }

    aEndPosLst.OutEndAttrs( rHTMLWrt, SAL_MAX_INT32 );

    
    if( bFlysLeft )
        bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                       nEnde, HTML_POS_INSIDE );
    OSL_ENSURE( !bFlysLeft, "Es wurden nicht alle Rahmen gespeichert!" );

    rHTMLWrt.bTxtAttr = sal_False;

    if( bWriteBreak )
    {
        sal_Bool bEndOfCell = rHTMLWrt.bOutTable &&
                         rWrt.pCurPam->GetPoint()->nNode.GetIndex() ==
                         rWrt.pCurPam->GetMark()->nNode.GetIndex();

        if( bEndOfCell && !nEnde &&
            rHTMLWrt.IsHTMLMode(HTMLMODE_NBSP_IN_TABLES) )
        {
            
            
            
            rWrt.Strm().WriteChar( '&' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_S_nbsp ).WriteChar( ';' );
        }
        else
        {
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_linebreak );
            const SvxULSpaceItem& rULSpace =
                (const SvxULSpaceItem &)pNd->GetSwAttrSet().Get(RES_UL_SPACE);
            if( rULSpace.GetLower() > 0 && !bEndOfCell &&
                !rHTMLWrt.IsHTMLMode(HTMLMODE_NO_BR_AT_PAREND) )
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_linebreak );
            rHTMLWrt.bLFPossible = sal_True;
        }
    }

    if( rHTMLWrt.bClearLeft || rHTMLWrt.bClearRight )
    {
        const sal_Char *pStr;
        if( rHTMLWrt.bClearLeft )
        {
            if( rHTMLWrt.bClearRight )
                pStr = OOO_STRING_SVTOOLS_HTML_AL_all;
            else
                pStr = OOO_STRING_SVTOOLS_HTML_AL_left;
        }
        else
            pStr = OOO_STRING_SVTOOLS_HTML_AL_right;

        OStringBuffer sOut(OOO_STRING_SVTOOLS_HTML_linebreak);
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_clear).append("=").append(pStr).append("\"");
        HTMLOutFuncs::Out_AsciiTag( rHTMLWrt.Strm(), sOut.getStr() );

        rHTMLWrt.bClearLeft = sal_False;
        rHTMLWrt.bClearRight = sal_False;

        rHTMLWrt.bLFPossible = sal_True;
    }

    
    
    if( !rHTMLWrt.bLFPossible && !rHTMLWrt.nLastParaToken &&
        nEnde > 0 && ' ' == rStr[nEnde-1] )
        rHTMLWrt.bLFPossible = sal_True;

    rHTMLWrt.bTagOn = sal_False;
    OutHTML_SwFmtOff( rWrt, aFmtInfo );

    
    rHTMLWrt.OutForm( sal_False );

    if( bPageBreakBehind )
        rWrt.Strm().WriteChar( '\f' );

    return rHTMLWrt;
}


sal_uInt32 SwHTMLWriter::ToPixel( sal_uInt32 nVal, const bool bVert ) const
{
    if( Application::GetDefaultDevice() && nVal )
    {
        Size aSz( bVert ? 0 : nVal, bVert ? nVal : 0 );
        aSz = Application::GetDefaultDevice()->LogicToPixel(aSz, MapMode( MAP_TWIP ));
        nVal = bVert ? aSz.Height() : aSz.Width();
        if( !nVal )     
            nVal = 1;
    }
    return nVal;
}


static Writer& OutHTML_CSS1Attr( Writer& rWrt, const SfxPoolItem& rHt )
{
    
    

    if( ((SwHTMLWriter&)rWrt).bCfgOutStyles && ((SwHTMLWriter&)rWrt).bTxtAttr )
        OutCSS1_HintSpanTag( rWrt, rHt );

    return rWrt;
}


/* File CHRATR.HXX: */

static Writer& OutHTML_SvxColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( !rHTMLWrt.bTxtAttr && rHTMLWrt.bCfgOutStyles && rHTMLWrt.bCfgPreferStyles )
    {
        
        
        return rWrt;
    }

    if( rHTMLWrt.bTagOn )
    {
        Color aColor( ((const SvxColorItem&)rHt).GetValue() );
        if( COL_AUTO == aColor.GetColor() )
            aColor.SetColor( COL_BLACK );

        OStringBuffer sOut;
        sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_font).append(' ')
            .append(OOO_STRING_SVTOOLS_HTML_O_color).append("=");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_Color( rWrt.Strm(), aColor, rHTMLWrt.eDestEnc ).WriteChar( '>' );
    }
    else
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_font, sal_False );

    return rWrt;
}


static Writer& OutHTML_SwPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontItalic nPosture = ((const SvxPostureItem&)rHt).GetPosture();
    if( ITALIC_NORMAL == nPosture )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_italic, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

static Writer& OutHTML_SvxFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        OUString aNames;
        SwHTMLWriter::PrepareFontList( ((const SvxFontItem&)rHt), aNames, 0,
                           rHTMLWrt.IsHTMLMode(HTMLMODE_FONT_GENERIC) );
        OStringBuffer sOut;
        sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_font).append(' ')
            .append(OOO_STRING_SVTOOLS_HTML_O_face).append("=\"");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( rWrt.Strm(), aNames, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters )
           .WriteCharPtr( "\">" );
    }
    else
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_font , sal_False );

    return rWrt;
}

static Writer& OutHTML_SvxFontHeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        OStringBuffer sOut;
        sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_font);

        sal_uInt32 nHeight = ((const SvxFontHeightItem&)rHt).GetHeight();
        sal_uInt16 nSize = rHTMLWrt.GetHTMLFontSize( nHeight );
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_size).append("=\"").
             append(static_cast<sal_Int32>(nSize)).append("\"");
        rWrt.Strm().WriteCharPtr( sOut.getStr() );

        if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
        {
            
            OutCSS1_HintStyleOpt( rWrt, rHt );
        }
        rWrt.Strm().WriteChar( '>' );
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_font, sal_False );
    }

    return rWrt;
}

static Writer& OutHTML_SvxLanguage( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    LanguageType eLang = ((const SvxLanguageItem &)rHt).GetLanguage();
    if( LANGUAGE_DONTKNOW == eLang )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        OStringBuffer sOut;
        sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_span);
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        rHTMLWrt.OutLanguage( ((const SvxLanguageItem &)rHt).GetLanguage() );
        rWrt.Strm().WriteChar( '>' );
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_span, sal_False );
    }

    return rWrt;
}
static Writer& OutHTML_SwWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontWeight nBold = ((const SvxWeightItem&)rHt).GetWeight();
    if( WEIGHT_BOLD == nBold )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_bold, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}


static Writer& OutHTML_SwCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    
    const FontStrikeout nStrike = ((const SvxCrossedOutItem&)rHt).GetStrikeout();
    if( STRIKEOUT_NONE != nStrike )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_strike, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}


static Writer& OutHTML_SvxEscapement( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SvxEscapement eEscape =
        (const SvxEscapement)((const SvxEscapementItem&)rHt).GetEnumValue();
    const sal_Char *pStr = 0;
    switch( eEscape )
    {
    case SVX_ESCAPEMENT_SUPERSCRIPT: pStr = OOO_STRING_SVTOOLS_HTML_superscript; break;
    case SVX_ESCAPEMENT_SUBSCRIPT: pStr = OOO_STRING_SVTOOLS_HTML_subscript; break;
    default:
        ;
    }

    if( pStr )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), pStr, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}



static Writer& OutHTML_SwUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontUnderline eUnder = ((const SvxUnderlineItem&)rHt).GetLineStyle();
    if( UNDERLINE_NONE != eUnder )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_underline, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}


static Writer& OutHTML_SwFlyCnt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;
    SwFmtFlyCnt& rFlyCnt = (SwFmtFlyCnt&)rHt;

    const SwFrmFmt& rFmt = *rFlyCnt.GetFrmFmt();
    const SdrObject *pSdrObj = 0;

    SwHTMLFrmType eType =
        (SwHTMLFrmType)rHTMLWrt.GuessFrmType( rFmt, pSdrObj );
    sal_uInt8 nMode = aHTMLOutFrmAsCharTable[eType][rHTMLWrt.nExportMode];
    rHTMLWrt.OutFrmFmt( nMode, rFmt, pSdrObj );
    return rWrt;
}




static Writer& OutHTML_SwBlink( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( ((const SvxBlinkItem&)rHt).GetValue() )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_blink, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

Writer& OutHTML_INetFmt( Writer& rWrt, const SwFmtINetFmt& rINetFmt, sal_Bool bOn )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    OUString aURL( rINetFmt.GetValue() );
    const SvxMacroTableDtor *pMacTable = rINetFmt.GetMacroTbl();
    sal_Bool bEvents = pMacTable != 0 && !pMacTable->empty();

    
    if( aURL.isEmpty() && !bEvents && rINetFmt.GetName().isEmpty() )
        return rWrt;

    
    if( !bOn )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_anchor, sal_False );
        return rWrt;
    }

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_anchor);

    bool bScriptDependent = false;
    {
        const SwCharFmt* pFmt = rWrt.pDoc->GetCharFmtFromPool(
                 RES_POOLCHR_INET_NORMAL );
        SwHTMLFmtInfo aFmtInfo( pFmt );
        SwHTMLFmtInfos::const_iterator it = rHTMLWrt.aChrFmtInfos.find( aFmtInfo );
        if( it != rHTMLWrt.aChrFmtInfos.end() )
        {
            bScriptDependent = it->bScriptDependent;
        }
    }
    if( !bScriptDependent )
    {
        const SwCharFmt* pFmt = rWrt.pDoc->GetCharFmtFromPool(
                 RES_POOLCHR_INET_VISIT );
        SwHTMLFmtInfo aFmtInfo( pFmt );
        SwHTMLFmtInfos::const_iterator it = rHTMLWrt.aChrFmtInfos.find( aFmtInfo );
        if( it != rHTMLWrt.aChrFmtInfos.end() )
        {
            bScriptDependent = it->bScriptDependent;
        }
    }

    if( bScriptDependent )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_class).append("=\"");
        switch( rHTMLWrt.nCSS1Script )
        {
        case CSS1_OUTMODE_WESTERN:
            sOut.append("western");
            break;
        case CSS1_OUTMODE_CJK:
            sOut.append("cjk");
            break;
        case CSS1_OUTMODE_CTL:
            sOut.append("ctl");
            break;
        }
        sOut.append('\"');
    }

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    OUString sRel;

    if( !aURL.isEmpty() || bEvents )
    {
        OUString sTmp( aURL.toAsciiUpperCase() );
        sal_Int32 nPos = sTmp.indexOf( "\" REL=" );
        if( nPos >= 0 )
        {
            sRel = aURL.copy( nPos+1 );
            aURL = aURL.copy( 0, nPos);
        }
        aURL = comphelper::string::strip(aURL, ' ');

        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_href).append("=\"");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        rHTMLWrt.OutHyperlinkHRefValue( aURL );
        sOut.append('\"');
    }

    if( !rINetFmt.GetName().isEmpty() )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_name).append("=\"");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( rWrt.Strm(), rINetFmt.GetName(),
                                  rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        sOut.append('\"');
    }

    const OUString& rTarget = rINetFmt.GetTargetFrame();
    if( !rTarget.isEmpty() )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_target).append("=\"");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( rWrt.Strm(), rTarget, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        sOut.append('\"');
    }

    if( !sRel.isEmpty() )
        sOut.append(OUStringToOString(sRel, RTL_TEXTENCODING_ASCII_US));

    if( !sOut.isEmpty() )
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    if( bEvents )
        HTMLOutFuncs::Out_Events( rWrt.Strm(), *pMacTable, aAnchorEventTable,
                                  rHTMLWrt.bCfgStarBasic, rHTMLWrt.eDestEnc,
                                     &rHTMLWrt.aNonConvertableCharacters    );
    rWrt.Strm().WriteCharPtr( ">" );

    return rWrt;
}

static Writer& OutHTML_SwFmtINetFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SwFmtINetFmt& rINetFmt = (const SwFmtINetFmt&)rHt;

    if( rHTMLWrt.bTagOn )
    {
        
        if( rHTMLWrt.aINetFmts.size() )
        {
            SwFmtINetFmt *pINetFmt =
                rHTMLWrt.aINetFmts.back();
            OutHTML_INetFmt( rWrt, *pINetFmt, sal_False );
        }

        
        OutHTML_INetFmt( rWrt, rINetFmt, sal_True );

        
        SwFmtINetFmt *pINetFmt = new SwFmtINetFmt( rINetFmt );
        rHTMLWrt.aINetFmts.push_back( pINetFmt );
    }
    else
    {
        
        OutHTML_INetFmt( rWrt, rINetFmt, sal_False );

        OSL_ENSURE( rHTMLWrt.aINetFmts.size(), "da fehlt doch ein URL-Attribut" );
        if( rHTMLWrt.aINetFmts.size() )
        {
            
            SwFmtINetFmt *pINetFmt = rHTMLWrt.aINetFmts.back();
            rHTMLWrt.aINetFmts.pop_back();
            delete pINetFmt;
        }

        if( !rHTMLWrt.aINetFmts.empty() )
        {
            
            
            SwFmtINetFmt *pINetFmt = rHTMLWrt.aINetFmts.back();
            OutHTML_INetFmt( rWrt, *pINetFmt, sal_True );
        }
    }

    return rWrt;
}

static Writer& OutHTML_SwTxtCharFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SwFmtCharFmt& rChrFmt = (const SwFmtCharFmt&)rHt;
    const SwCharFmt* pFmt = rChrFmt.GetCharFmt();

    if( !pFmt )
    {
        return rWrt;
    }

    SwHTMLFmtInfo aFmtInfo( pFmt );
    SwHTMLFmtInfos::const_iterator it = rHTMLWrt.aChrFmtInfos.find( aFmtInfo );
    if( it == rHTMLWrt.aChrFmtInfos.end())
        return rWrt;

    const SwHTMLFmtInfo *pFmtInfo = &*it;
    OSL_ENSURE( pFmtInfo, "Wieso gint es keine Infos ueber die Zeichenvorlage?" );

    if( rHTMLWrt.bTagOn )
    {
        OStringBuffer sOut;
        sOut.append('<');
        if( !pFmtInfo->aToken.isEmpty() )
            sOut.append(pFmtInfo->aToken);
        else
            sOut.append(OOO_STRING_SVTOOLS_HTML_span);
        if( rHTMLWrt.bCfgOutStyles &&
            (!pFmtInfo->aClass.isEmpty() || pFmtInfo->bScriptDependent) )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_class)
                .append("=\"");
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            OUString aClass( pFmtInfo->aClass );
            if( pFmtInfo->bScriptDependent )
            {
                if( !aClass.isEmpty() )
                   aClass += "-";
                switch( rHTMLWrt.nCSS1Script )
                {
                case CSS1_OUTMODE_WESTERN:
                    aClass += "western";
                    break;
                case CSS1_OUTMODE_CJK:
                    aClass += "cjk";
                    break;
                case CSS1_OUTMODE_CTL:
                    aClass += "ctl";
                    break;
                }
            }
            HTMLOutFuncs::Out_String( rWrt.Strm(), aClass,
                                          rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
            sOut.append('\"');
        }
        sOut.append('>');
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                !pFmtInfo->aToken.isEmpty() ? pFmtInfo->aToken.getStr()
                                       : OOO_STRING_SVTOOLS_HTML_span,
                sal_False );
    }

    return rWrt;
}

static Writer& OutHTML_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( !rHTMLWrt.bOutOpts || !rHTMLWrt.bTagOn )
        return  rWrt;

    SvxAdjustItem& rAdjust = (SvxAdjustItem&)rHt;
    const sal_Char* pStr = 0;
    switch( rAdjust.GetAdjust() )
    {
    case SVX_ADJUST_CENTER: pStr = OOO_STRING_SVTOOLS_HTML_AL_center; break;
    case SVX_ADJUST_LEFT: pStr = OOO_STRING_SVTOOLS_HTML_AL_left; break;
    case SVX_ADJUST_RIGHT: pStr = OOO_STRING_SVTOOLS_HTML_AL_right; break;
    case SVX_ADJUST_BLOCK: pStr = OOO_STRING_SVTOOLS_HTML_AL_justify; break;
    default:
        ;
    }
    if( pStr )
    {
        OStringBuffer sOut;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append("=\"")
            .append(pStr).append("\"");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    }

    return rWrt;
}

/*
 * lege hier die Tabellen fuer die HTML-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb der HTML-DLL
 * bekannt sein muessen.
 */


SwAttrFnTab aHTMLAttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutHTML_CSS1Attr,
/* RES_CHRATR_CHARSETCOLOR  */      0,
/* RES_CHRATR_COLOR */              OutHTML_SvxColor,
/* RES_CHRATR_CONTOUR   */          0,
/* RES_CHRATR_CROSSEDOUT    */      OutHTML_SwCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      OutHTML_SvxEscapement,
/* RES_CHRATR_FONT  */              OutHTML_SvxFont,
/* RES_CHRATR_FONTSIZE  */          OutHTML_SvxFontHeight,
/* RES_CHRATR_KERNING   */          OutHTML_CSS1Attr,
/* RES_CHRATR_LANGUAGE  */          OutHTML_SvxLanguage,
/* RES_CHRATR_POSTURE   */          OutHTML_SwPosture,
/* RES_CHRATR_PROPORTIONALFONTSIZE*/0,
/* RES_CHRATR_SHADOWED  */          0,
/* RES_CHRATR_UNDERLINE */          OutHTML_SwUnderline,
/* RES_CHRATR_WEIGHT    */          OutHTML_SwWeight,
/* RES_CHRATR_WORDLINEMODE  */      0,
/* RES_CHRATR_AUTOKERN  */          0,
/* RES_CHRATR_BLINK */              OutHTML_SwBlink,
/* RES_CHRATR_NOHYPHEN  */          0, 
/* RES_CHRATR_NOLINEBREAK */        0, 
/* RES_CHRATR_BACKGROUND */         OutHTML_CSS1Attr, 
/* RES_CHRATR_CJK_FONT */           OutHTML_SvxFont,
/* RES_CHRATR_CJK_FONTSIZE */       OutHTML_SvxFontHeight,
/* RES_CHRATR_CJK_LANGUAGE */       OutHTML_SvxLanguage,
/* RES_CHRATR_CJK_POSTURE */        OutHTML_SwPosture,
/* RES_CHRATR_CJK_WEIGHT */         OutHTML_SwWeight,
/* RES_CHRATR_CTL_FONT */           OutHTML_SvxFont,
/* RES_CHRATR_CTL_FONTSIZE */       OutHTML_SvxFontHeight,
/* RES_CHRATR_CTL_LANGUAGE */       OutHTML_SvxLanguage,
/* RES_CHRATR_CTL_POSTURE */        OutHTML_SwPosture,
/* RES_CHRATR_CTL_WEIGHT */         OutHTML_SwWeight,
/* RES_CHRATR_ROTATE */             0,
/* RES_CHRATR_EMPHASIS_MARK */      0,
/* RES_CHRATR_TWO_LINES */          0,
/* RES_CHRATR_SCALEW */             0,
/* RES_CHRATR_RELIEF */             0,
/* RES_CHRATR_HIDDEN */             OutHTML_CSS1Attr,
/* RES_CHRATR_OVERLINE */           OutHTML_CSS1Attr,
/* RES_CHRATR_RSID */               0,
/* RES_CHRATR_BOX */                OutHTML_CSS1Attr,
/* RES_CHRATR_SHADOW */             0,
/* RES_CHRATR_HIGHLGHT */           0,
/* RES_CHRATR_DUMMY2 */             0,
/* RES_CHRATR_DUMMY3 */             0,

/* RES_TXTATR_REFMARK */            0,
/* RES_TXTATR_TOXMARK */            0,
/* RES_TXTATR_META */               0,
/* RES_TXTATR_METAFIELD */          0,
/* RES_TXTATR_AUTOFMT */            0,
/* RES_TXTATR_INETFMT */            OutHTML_SwFmtINetFmt,
/* RES_TXTATR_CHARFMT */            OutHTML_SwTxtCharFmt,
/* RES_TXTATR_CJK_RUBY */           0,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_INPUTFIELD */         OutHTML_SwFmtFld,

/* RES_TXTATR_FIELD */              OutHTML_SwFmtFld,
/* RES_TXTATR_FLYCNT */             OutHTML_SwFlyCnt,
/* RES_TXTATR_FTN */                OutHTML_SwFmtFtn,
/* RES_TXTATR_ANNOTATION */         OutHTML_SwFmtFld,
/* RES_TXTATR_DUMMY3 */             0,
/* RES_TXTATR_DUMMY1 */             0, 
/* RES_TXTATR_DUMMY2 */             0, 

/* RES_PARATR_LINESPACING   */      0,
/* RES_PARATR_ADJUST    */          OutHTML_SvxAdjust,
/* RES_PARATR_SPLIT */              0,
/* RES_PARATR_WIDOWS    */          0,
/* RES_PARATR_ORPHANS   */          0,
/* RES_PARATR_TABSTOP   */          0,
/* RES_PARATR_HYPHENZONE*/          0,
/* RES_PARATR_DROP */               OutHTML_CSS1Attr,
/* RES_PARATR_REGISTER */           0, 
/* RES_PARATR_NUMRULE */            0, 
/* RES_PARATR_SCRIPTSPACE */        0, 
/* RES_PARATR_HANGINGPUNCTUATION */ 0, 
/* RES_PARATR_FORBIDDEN_RULES */    0, 
/* RES_PARATR_VERTALIGN */          0, 
/* RES_PARATR_SNAPTOGRID*/          0, 
/* RES_PARATR_CONNECT_TO_BORDER */  0, 
/* RES_PARATR_OUTLINELEVEL */       0,
/* RES_PARATR_RSID */               0,
/* RES_PARATR_GRABBAG */            0,

/* RES_PARATR_LIST_ID */            0, 
/* RES_PARATR_LIST_LEVEL */         0, 
/* RES_PARATR_LIST_ISRESTART */     0, 
/* RES_PARATR_LIST_RESTARTVALUE */  0, 
/* RES_PARATR_LIST_ISCOUNTED */     0, 

/* RES_FILL_ORDER   */              0,
/* RES_FRM_SIZE */                  0,
/* RES_PAPER_BIN    */              0,
/* RES_LR_SPACE */                  0,
/* RES_UL_SPACE */                  0,
/* RES_PAGEDESC */                  0,
/* RES_BREAK */                     0,
/* RES_CNTNT */                     0,
/* RES_HEADER */                    0,
/* RES_FOOTER */                    0,
/* RES_PRINT */                     0,
/* RES_OPAQUE */                    0,
/* RES_PROTECT */                   0,
/* RES_SURROUND */                  0,
/* RES_VERT_ORIENT */               0,
/* RES_HORI_ORIENT */               0,
/* RES_ANCHOR */                    0,
/* RES_BACKGROUND */                0,
/* RES_BOX  */                      0,
/* RES_SHADOW */                    0,
/* RES_FRMMACRO */                  0,
/* RES_COL */                       0,
/* RES_KEEP */                      0,
/* RES_URL */                       0,
/* RES_EDIT_IN_READONLY */          0,
/* RES_LAYOUT_SPLIT */              0,
/* RES_CHAIN */                     0,
/* RES_TEXTGRID */                  0,
/* RES_LINENUMBER */                0,
/* RES_FTN_AT_TXTEND */             0,
/* RES_END_AT_TXTEND */             0,
/* RES_COLUMNBALANCE */             0,
/* RES_FRAMEDIR */                  0,
/* RES_HEADER_FOOTER_EAT_SPACING */ 0,
/* RES_ROW_SPLIT */                 0,
/* RES_FOLLOW_TEXT_FLOW */          0,
/* RES_COLLAPSING_BORDERS */        0,
/* RES_WRAP_INFLUENCE_ON_OBJPOS */  0,
/* RES_AUTO_STYLE */                0,
/* RES_FRMATR_STYLE_NAME */         0,
/* RES_FRMATR_CONDITIONAL_STYLE_NAME */ 0,
/* RES_FILL_STYLE */                0,
/* RES_FILL_GRADIENT */             0,
/* RES_FRMATR_GRABBAG */            0,
/* RES_TEXT_VERT_ADJUST */          0,

/* RES_GRFATR_MIRRORGRF */          0,
/* RES_GRFATR_CROPGRF   */          0,
/* RES_GRFATR_ROTATION */           0,
/* RES_GRFATR_LUMINANCE */          0,
/* RES_GRFATR_CONTRAST */           0,
/* RES_GRFATR_CHANNELR */           0,
/* RES_GRFATR_CHANNELG */           0,
/* RES_GRFATR_CHANNELB */           0,
/* RES_GRFATR_GAMMA */              0,
/* RES_GRFATR_INVERT */             0,
/* RES_GRFATR_TRANSPARENCY */       0,
/* RES_GRFATR_DRWAMODE */           0,
/* RES_GRFATR_DUMMY1 */             0,
/* RES_GRFATR_DUMMY2 */             0,
/* RES_GRFATR_DUMMY3 */             0,
/* RES_GRFATR_DUMMY4 */             0,
/* RES_GRFATR_DUMMY5 */             0,

/* RES_BOXATR_FORMAT */             0,
/* RES_BOXATR_FORMULA */            0,
/* RES_BOXATR_VALUE */              0
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
