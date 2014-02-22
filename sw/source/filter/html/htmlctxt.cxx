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

#include "hintids.hxx"
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svtools/htmltokn.h>
#include <editeng/boxitem.hxx>

#include "doc.hxx"
#include "pam.hxx"
#include "ndtxt.hxx"
#include "shellio.hxx"
#include "paratr.hxx"
#include "htmlnum.hxx"
#include "css1kywd.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"

using namespace ::com::sun::star;




class _HTMLAttrContext_SaveDoc
{
    SwHTMLNumRuleInfo aNumRuleInfo; 
    SwPosition  *pPos;              
                                    
    _HTMLAttrTable *pAttrTab;       
                                    
                                    

    sal_uInt16 nContextStMin;           
                                    
                                    
    sal_uInt16 nContextStAttrMin;       
                                    
                                    

    sal_Bool bStripTrailingPara : 1;    
    sal_Bool bKeepNumRules : 1;         
    sal_Bool bFixHeaderDist : 1;
    sal_Bool bFixFooterDist : 1;

public:

    _HTMLAttrContext_SaveDoc() :
        pPos( 0 ), pAttrTab( 0 ),
        nContextStMin( USHRT_MAX ), nContextStAttrMin( USHRT_MAX ),
        bStripTrailingPara( sal_False ), bKeepNumRules( sal_False ),
        bFixHeaderDist( sal_False ), bFixFooterDist( sal_False )
    {}

    ~_HTMLAttrContext_SaveDoc() { delete pPos; delete pAttrTab; }

    
    void SetPos( const SwPosition& rPos ) { pPos = new SwPosition(rPos); }
    const SwPosition *GetPos() const { return pPos; }

    
    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; }

    _HTMLAttrTable *GetAttrTab( sal_Bool bCreate= sal_False );

    void SetContextStMin( sal_uInt16 nMin ) { nContextStMin = nMin; }
    sal_uInt16 GetContextStMin() const { return nContextStMin; }

    void SetContextStAttrMin( sal_uInt16 nMin ) { nContextStAttrMin = nMin; }
    sal_uInt16 GetContextStAttrMin() const { return nContextStAttrMin; }

    void SetStripTrailingPara( sal_Bool bSet ) { bStripTrailingPara = bSet; }
    sal_Bool GetStripTrailingPara() const { return bStripTrailingPara; }

    void SetKeepNumRules( sal_Bool bSet ) { bKeepNumRules = bSet; }
    sal_Bool GetKeepNumRules() const { return bKeepNumRules; }

    void SetFixHeaderDist( sal_Bool bSet ) { bFixHeaderDist = bSet; }
    sal_Bool GetFixHeaderDist() const { return bFixHeaderDist; }

    void SetFixFooterDist( sal_Bool bSet ) { bFixFooterDist = bSet; }
    sal_Bool GetFixFooterDist() const { return bFixFooterDist; }
};

_HTMLAttrTable *_HTMLAttrContext_SaveDoc::GetAttrTab( sal_Bool bCreate )
{
    if( !pAttrTab && bCreate )
    {
        pAttrTab = new _HTMLAttrTable;
        memset( pAttrTab, 0, sizeof( _HTMLAttrTable ));
    }
    return pAttrTab;
}


_HTMLAttrContext_SaveDoc *_HTMLAttrContext::GetSaveDocContext( sal_Bool bCreate )
{
    if( !pSaveDocContext && bCreate )
        pSaveDocContext = new _HTMLAttrContext_SaveDoc;

    return pSaveDocContext;
}

void _HTMLAttrContext::ClearSaveDocContext()
{
    delete pSaveDocContext;
    pSaveDocContext = 0;
}


void SwHTMLParser::SplitAttrTab( const SwPosition& rNewPos )
{
    
    
    OSL_ENSURE(aParaAttrs.empty(),
        "Danger: there are non-final paragraph attributes");
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    const SwNodeIndex* pOldEndPara = &pPam->GetPoint()->nNode;
    sal_Int32 nOldEndCnt = pPam->GetPoint()->nContent.GetIndex();

    const SwNodeIndex& rNewSttPara = rNewPos.nNode;
    sal_Int32 nNewSttCnt = rNewPos.nContent.GetIndex();

    sal_Bool bMoveBack = sal_False;

    
    
    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
            nCnt--; ++pTbl )
    {
        _HTMLAttr *pAttr = *pTbl;
        while( pAttr )
        {
            _HTMLAttr *pNext = pAttr->GetNext();
            _HTMLAttr *pPrev = pAttr->GetPrev();

            sal_uInt16 nWhich = pAttr->pItem->Which();
            if( !nOldEndCnt && RES_PARATR_BEGIN <= nWhich &&
                pAttr->GetSttParaIdx() < pOldEndPara->GetIndex() )
            {
                
                
                if( !bMoveBack )
                {
                    bMoveBack = pPam->Move( fnMoveBackward );
                    nOldEndCnt = pPam->GetPoint()->nContent.GetIndex();
                }
            }
            else if( bMoveBack )
            {
                pPam->Move( fnMoveForward );
                nOldEndCnt = pPam->GetPoint()->nContent.GetIndex();
            }

            if( (RES_PARATR_BEGIN <= nWhich && bMoveBack) ||
                pAttr->GetSttParaIdx() < pOldEndPara->GetIndex() ||
                (pAttr->GetSttPara() == *pOldEndPara &&
                 pAttr->GetSttCnt() != nOldEndCnt) )
            {
                
                
                
                
                
                _HTMLAttr *pSetAttr = pAttr->Clone( *pOldEndPara, nOldEndCnt );

                if( pNext )
                    pNext->InsertPrev( pSetAttr );
                else
                {
                    if (pSetAttr->bInsAtStart)
                        aSetAttrTab.push_front( pSetAttr );
                    else
                        aSetAttrTab.push_back( pSetAttr );
                }
            }
            else if( pPrev )
            {
                
                
                
                if( pNext )
                    pNext->InsertPrev( pPrev );
                else
                {
                    if (pPrev->bInsAtStart)
                        aSetAttrTab.push_front( pPrev );
                    else
                        aSetAttrTab.push_back( pPrev );
                }
            }

            
            pAttr->nSttPara = rNewSttPara;
            pAttr->nEndPara = rNewSttPara;
            pAttr->nSttCntnt = nNewSttCnt;
            pAttr->nEndCntnt = nNewSttCnt;
            pAttr->pPrev = 0;

            pAttr = pNext;
        }
    }

    if( bMoveBack )
        pPam->Move( fnMoveForward );

}

void SwHTMLParser::SaveDocContext( _HTMLAttrContext *pCntxt,
                                   sal_uInt16 nFlags,
                                   const SwPosition *pNewPos )
{
    _HTMLAttrContext_SaveDoc *pSave = pCntxt->GetSaveDocContext( sal_True );
    pSave->SetStripTrailingPara( (HTML_CNTXT_STRIP_PARA & nFlags) != 0 );
    pSave->SetKeepNumRules( (HTML_CNTXT_KEEP_NUMRULE & nFlags) != 0 );
    pSave->SetFixHeaderDist( (HTML_CNTXT_HEADER_DIST & nFlags) != 0 );
    pSave->SetFixFooterDist( (HTML_CNTXT_FOOTER_DIST & nFlags) != 0 );

    if( pNewPos )
    {
        
        
        if( !pSave->GetKeepNumRules() )
        {
            
            
            
            pSave->SetNumInfo( GetNumInfo() );
            GetNumInfo().Clear();
        }

        if( (HTML_CNTXT_KEEP_ATTRS & nFlags) != 0 )
        {
            
            SplitAttrTab( *pNewPos );
        }
        else
        {
            _HTMLAttrTable *pSaveAttrTab = pSave->GetAttrTab( sal_True );
            SaveAttrTab( *pSaveAttrTab );
        }


        pSave->SetPos( *pPam->GetPoint() );
        *pPam->GetPoint() = *pNewPos;
    }

    
    
    if( (HTML_CNTXT_PROTECT_STACK & nFlags) != 0  )
    {
        pSave->SetContextStMin( nContextStMin );
        nContextStMin = aContexts.size();

        if( (HTML_CNTXT_KEEP_ATTRS & nFlags) == 0 )
        {
            pSave->SetContextStAttrMin( nContextStAttrMin );
            nContextStAttrMin = aContexts.size();
        }
    }
}

void SwHTMLParser::RestoreDocContext( _HTMLAttrContext *pCntxt )
{
    _HTMLAttrContext_SaveDoc *pSave = pCntxt->GetSaveDocContext();
    if( !pSave )
        return;

    if( pSave->GetStripTrailingPara() )
        StripTrailingPara();

    if( pSave->GetPos() )
    {
        if( pSave->GetFixHeaderDist() || pSave->GetFixFooterDist() )
            FixHeaderFooterDistance( pSave->GetFixHeaderDist(),
                                     pSave->GetPos() );

        _HTMLAttrTable *pSaveAttrTab = pSave->GetAttrTab();
        if( !pSaveAttrTab )
        {
            
            
            SplitAttrTab( *pSave->GetPos() );
        }
        else
        {
            RestoreAttrTab( *pSaveAttrTab );
        }

        *pPam->GetPoint() = *pSave->GetPos();

        
        SetAttr();
    }

    if( USHRT_MAX != pSave->GetContextStMin() )
    {
        nContextStMin = pSave->GetContextStMin();
        if( USHRT_MAX != pSave->GetContextStAttrMin() )
            nContextStAttrMin = pSave->GetContextStAttrMin();
    }

    if( !pSave->GetKeepNumRules() )
    {
        
        GetNumInfo().Set( pSave->GetNumInfo() );
    }

    pCntxt->ClearSaveDocContext();
}


void SwHTMLParser::EndContext( _HTMLAttrContext *pContext )
{
    if( pContext->GetPopStack() )
    {
        
        
        while( aContexts.size() > nContextStMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            OSL_ENSURE( pCntxt != pContext,
                    "Kontext noch im Stack" );
            if( pCntxt == pContext )
                break;

            EndContext( pCntxt );
            delete pCntxt;
        }
    }

    
    if( pContext->HasAttrs() )
        EndContextAttrs( pContext );

    
    
    
    if( pContext->GetSpansSection() )
        EndSection();

    
    if( pContext->HasSaveDocContext() )
        RestoreDocContext( pContext );

    
    if( AM_NONE != pContext->GetAppendMode() &&
        pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( pContext->GetAppendMode() );

    
    if( pContext->IsFinishPREListingXMP() )
        FinishPREListingXMP();

    if( pContext->IsRestartPRE() )
        StartPRE();

    if( pContext->IsRestartXMP() )
        StartXMP();

    if( pContext->IsRestartListing() )
        StartListing();
}

void SwHTMLParser::ClearContext( _HTMLAttrContext *pContext )
{
    _HTMLAttrs &rAttrs = pContext->GetAttrs();
    for( sal_uInt16 i=0; i<rAttrs.size(); i++ )
    {
        
        
        
        
        
        DeleteAttr( rAttrs[i] );
    }

    OSL_ENSURE( !pContext->GetSpansSection(),
            "Bereich kann nicht mehr verlassen werden" );

    OSL_ENSURE( !pContext->HasSaveDocContext(),
            "Rahmen kann nicht mehr verlassen werden" );

    
    if( pContext->IsFinishPREListingXMP() )
        FinishPREListingXMP();

    if( pContext->IsRestartPRE() )
        StartPRE();

    if( pContext->IsRestartXMP() )
        StartXMP();

    if( pContext->IsRestartListing() )
        StartListing();
}


sal_Bool SwHTMLParser::DoPositioning( SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo &rPropInfo,
                                  _HTMLAttrContext *pContext )
{
    sal_Bool bRet = sal_False;

    
    
    
    
    
    if( SwCSS1Parser::MayBePositioned( rPropInfo ) )
    {
        SfxItemSet aFrmItemSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
        if( !IsNewDoc() )
            Reader::ResetFrmFmtAttrs(aFrmItemSet );

        
        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE, rItemSet, rPropInfo,
                                aFrmItemSet );

        
        SetVarSize( rItemSet, rPropInfo, aFrmItemSet );

        
        SetSpace( Size(0,0), rItemSet, rPropInfo, aFrmItemSet );

        
        SetFrmFmtAttrs( rItemSet, rPropInfo,
                        HTML_FF_BOX|HTML_FF_PADDING|HTML_FF_BACKGROUND|HTML_FF_DIRECTION,
                        aFrmItemSet );

        InsertFlyFrame( aFrmItemSet, pContext, rPropInfo.aId,
                        CONTEXT_FLAGS_ABSPOS );
        pContext->SetPopStack( sal_True );
        rPropInfo.aId = "";
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool SwHTMLParser::CreateContainer( const OUString& rClass,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo &rPropInfo,
                                    _HTMLAttrContext *pContext )
{
    sal_Bool bRet = sal_False;
    if( rClass.equalsIgnoreAsciiCase( "sd-abs-pos" ) &&
        pCSS1Parser->MayBePositioned( rPropInfo ) )
    {
        
        SfxItemSet *pFrmItemSet = pContext->GetFrmItemSet( pDoc );
        if( !IsNewDoc() )
            Reader::ResetFrmFmtAttrs( *pFrmItemSet );

        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE,
                                rItemSet, rPropInfo, *pFrmItemSet );
        Size aDummy(0,0);
        SetFixSize( aDummy, aDummy, sal_False, sal_False, rItemSet, rPropInfo,
                    *pFrmItemSet );
        SetSpace( aDummy, rItemSet, rPropInfo, *pFrmItemSet );
        SetFrmFmtAttrs( rItemSet, rPropInfo, HTML_FF_BOX|HTML_FF_BACKGROUND|HTML_FF_DIRECTION,
                        *pFrmItemSet );

        bRet = sal_True;
    }

    return bRet;
}


void SwHTMLParser::InsertAttrs( SfxItemSet &rItemSet,
                                SvxCSS1PropertyInfo &rPropInfo,
                                _HTMLAttrContext *pContext,
                                sal_Bool bCharLvl )
{
    
    
    if( bCharLvl && !pPam->GetPoint()->nContent.GetIndex() &&
        SVX_ADJUST_LEFT == rPropInfo.eFloat )
    {
        SwFmtDrop aDrop;
        aDrop.GetChars() = 1;

        pCSS1Parser->FillDropCap( aDrop, rItemSet );

        
        
        if( aDrop.GetLines() > 1 )
        {
            NewAttr( &aAttrTab.pDropCap, aDrop );

            _HTMLAttrs &rAttrs = pContext->GetAttrs();
            rAttrs.push_back( aAttrTab.pDropCap );

            return;
        }
    }


    if( !bCharLvl )
        pCSS1Parser->SetFmtBreak( rItemSet, rPropInfo );


    OSL_ENSURE(aContexts.size() <= nContextStAttrMin ||
            aContexts.back() != pContext,
            "SwHTMLParser::InsertAttrs: Context already on the Stack");

    SfxItemIter aIter( rItemSet );

    const SfxPoolItem *pItem = aIter.FirstItem();
    while( pItem )
    {
        _HTMLAttr **ppAttr = 0;

        switch( pItem->Which() )
        {
        case RES_LR_SPACE:
            {
                
                
                

                const SvxLRSpaceItem *pLRItem =
                    (const SvxLRSpaceItem *)pItem;

                
                
                sal_uInt16 nOldLeft = 0, nOldRight = 0;
                short nOldIndent = 0;
                sal_Bool bIgnoreTop = aContexts.size() > nContextStMin &&
                                  aContexts.back() == pContext;
                GetMarginsFromContext( nOldLeft, nOldRight, nOldIndent,
                                       bIgnoreTop  );


                
                sal_uInt16 nLeft = nOldLeft, nRight = nOldRight;
                short nIndent = nOldIndent;
                pContext->GetMargins( nLeft, nRight, nIndent );

                
                
                
                
                
                
                if( rPropInfo.bLeftMargin )
                {
                    OSL_ENSURE( rPropInfo.nLeftMargin < 0 ||
                            rPropInfo.nLeftMargin == pLRItem->GetTxtLeft(),
                            "linker Abstand stimmt nicht mit Item ueberein" );
                    if( rPropInfo.nLeftMargin < 0 &&
                        -rPropInfo.nLeftMargin > nOldLeft )
                        nLeft = 0;
                    else
                        nLeft = nOldLeft + static_cast< sal_uInt16 >(rPropInfo.nLeftMargin);
                }
                if( rPropInfo.bRightMargin )
                {
                    OSL_ENSURE( rPropInfo.nRightMargin < 0 ||
                            rPropInfo.nRightMargin == pLRItem->GetRight(),
                            "rechter Abstand stimmt nicht mit Item ueberein" );
                    if( rPropInfo.nRightMargin < 0 &&
                        -rPropInfo.nRightMargin > nOldRight )
                        nRight = 0;
                    else
                        nRight = nOldRight + static_cast< sal_uInt16 >(rPropInfo.nRightMargin);
                }
                if( rPropInfo.bTextIndent )
                    nIndent = pLRItem->GetTxtFirstLineOfst();

                
                pContext->SetMargins( nLeft, nRight, nIndent );

                
                SvxLRSpaceItem aLRItem( *pLRItem );
                aLRItem.SetTxtFirstLineOfst( nIndent );
                aLRItem.SetTxtLeft( nLeft );
                aLRItem.SetRight( nRight );
                NewAttr( &aAttrTab.pLRSpace, aLRItem );
                EndAttr( aAttrTab.pLRSpace, 0, sal_False );
            }
            break;

        case RES_UL_SPACE:
            if( !rPropInfo.bTopMargin || !rPropInfo.bBottomMargin )
            {
                sal_uInt16 nUpper = 0, nLower = 0;
                GetULSpaceFromContext( nUpper, nLower );
                SvxULSpaceItem aULSpace( *((const SvxULSpaceItem *)pItem) );
                if( !rPropInfo.bTopMargin )
                    aULSpace.SetUpper( nUpper );
                if( !rPropInfo.bBottomMargin )
                    aULSpace.SetLower( nLower );

                NewAttr( &aAttrTab.pULSpace, aULSpace );

                
                _HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( aAttrTab.pULSpace );

                pContext->SetULSpace( aULSpace.GetUpper(), aULSpace.GetLower() );
            }
            else
            {
                ppAttr = &aAttrTab.pULSpace;
            }
            break;
        case RES_CHRATR_FONTSIZE:
            
            if( ((const SvxFontHeightItem *)pItem)->GetProp() == 100 )
                ppAttr = &aAttrTab.pFontHeight;
            break;
        case RES_CHRATR_CJK_FONTSIZE:
            
            if( ((const SvxFontHeightItem *)pItem)->GetProp() == 100 )
                ppAttr = &aAttrTab.pFontHeightCJK;
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            
            if( ((const SvxFontHeightItem *)pItem)->GetProp() == 100 )
                ppAttr = &aAttrTab.pFontHeightCTL;
            break;

        case RES_BACKGROUND:
            if( bCharLvl )
            {
                
                SvxBrushItem aBrushItem( *(const SvxBrushItem *)pItem );
                aBrushItem.SetWhich( RES_CHRATR_BACKGROUND );

                
                NewAttr( &aAttrTab.pCharBrush, aBrushItem );

                
                _HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( aAttrTab.pCharBrush );
            }
            else if( pContext->GetToken() != HTML_TABLEHEADER_ON &&
                     pContext->GetToken() != HTML_TABLEDATA_ON )
            {
                ppAttr = &aAttrTab.pBrush;
            }
            break;

        case RES_BOX:
            if( bCharLvl )
            {
                SvxBoxItem aBoxItem( *(const SvxBoxItem *)pItem );
                aBoxItem.SetWhich( RES_CHRATR_BOX );

                NewAttr( &aAttrTab.pCharBox, aBoxItem );

                _HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( aAttrTab.pCharBox );
            }
            else
            {
                ppAttr = &aAttrTab.pBox;
            }
            break;

        default:
            
            ppAttr = GetAttrTabEntry( pItem->Which() );
            break;
        }

        if( ppAttr )
        {
            
            NewAttr( ppAttr, *pItem );

            
            _HTMLAttrs &rAttrs = pContext->GetAttrs();
            rAttrs.push_back( *ppAttr );
        }

        
        pItem = aIter.NextItem();
    }

    if( !rPropInfo.aId.isEmpty() )
        InsertBookmark( rPropInfo.aId );
}

void SwHTMLParser::InsertAttr( _HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                               _HTMLAttrContext *pCntxt )
{
    if( !ppAttr )
    {
        ppAttr = GetAttrTabEntry( rItem.Which() );
        if( !ppAttr )
            return;
    }

    
    NewAttr( ppAttr, rItem );

    
    _HTMLAttrs &rAttrs = pCntxt->GetAttrs();
    rAttrs.push_back( *ppAttr );
}

void SwHTMLParser::SplitPREListingXMP( _HTMLAttrContext *pCntxt )
{
    
    pCntxt->SetFinishPREListingXMP( sal_True );

    
    if( IsReadPRE() )
        pCntxt->SetRestartPRE( sal_True );
    if( IsReadXMP() )
        pCntxt->SetRestartXMP( sal_True );
    if( IsReadListing() )
        pCntxt->SetRestartListing( sal_True );

    
    FinishPREListingXMP();
}

SfxItemSet *_HTMLAttrContext::GetFrmItemSet( SwDoc *pCreateDoc )
{
    if( !pFrmItemSet && pCreateDoc )
        pFrmItemSet = new SfxItemSet( pCreateDoc->GetAttrPool(),
                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    return pFrmItemSet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
