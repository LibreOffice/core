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

#include "scitems.hxx"

#include <svx/pageitem.hxx>
#include <sfx2/linkmgr.hxx>

#include "docsh.hxx"

#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "global.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "tablink.hxx"
#include "globstr.hrc"
#include "scmod.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "calcconfig.hxx"

#include <vcl/msgbox.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::util::XChangesBatch;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY_THROW;

namespace {

struct ScStylePair
{
    SfxStyleSheetBase *pSource;
    SfxStyleSheetBase *pDest;
};

}





//

//

void ScDocShell::SetVisArea( const Rectangle & rVisArea )
{
    
    
    SetVisAreaOrSize( rVisArea, true );
}

static void lcl_SetTopRight( Rectangle& rRect, const Point& rPos )
{
    Size aSize = rRect.GetSize();
    rRect.Right() = rPos.X();
    rRect.Left() = rPos.X() - aSize.Width() + 1;
    rRect.Top() = rPos.Y();
    rRect.Bottom() = rPos.Y() + aSize.Height() - 1;
}

void ScDocShell::SetVisAreaOrSize( const Rectangle& rVisArea, bool bModifyStart )
{
    sal_Bool bNegativePage = aDocument.IsNegativePage( aDocument.GetVisibleTab() );

    Rectangle aArea = rVisArea;
    if (bModifyStart)
    {
        
        
        if ( !aDocument.IsImportingXML() )
        {
            if ( ( bNegativePage ? (aArea.Right() > 0) : (aArea.Left() < 0) ) || aArea.Top() < 0 )
            {
                
                
                

                Point aNewPos( 0, std::max( aArea.Top(), (long) 0 ) );
                if ( bNegativePage )
                {
                    aNewPos.X() = std::min( aArea.Right(), (long) 0 );
                    lcl_SetTopRight( aArea, aNewPos );
                }
                else
                {
                    aNewPos.X() = std::max( aArea.Left(), (long) 0 );
                    aArea.SetPos( aNewPos );
                }
            }
        }
    }
    else
    {
        Rectangle aOldVisArea = SfxObjectShell::GetVisArea();
        if ( bNegativePage )
            lcl_SetTopRight( aArea, aOldVisArea.TopRight() );
        else
            aArea.SetPos( aOldVisArea.TopLeft() );
    }

    

    
    
    if ( !aDocument.IsImportingXML() )
        SnapVisArea( aArea );

    
    /*
    SvInPlaceEnvironment* pEnv = GetIPEnv();
    if (pEnv)
    {
        Window* pWin = pEnv->GetEditWin();
        pEnv->MakeScale( aArea.GetSize(), MAP_100TH_MM,
                            pWin->LogicToPixel( aArea.GetSize() ) );
    } */

    
    SfxObjectShell::SetVisArea( aArea );

    if (bIsInplace)                     
    {
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if (pViewSh)
        {
            if (pViewSh->GetViewData()->GetDocShell() == this)
                pViewSh->UpdateOleZoom();
        }
    }

    if (aDocument.IsEmbedded())
    {
        ScRange aOld;
        aDocument.GetEmbedded( aOld);
        aDocument.SetEmbedded( aDocument.GetVisibleTab(), aArea );
        ScRange aNew;
        aDocument.GetEmbedded( aNew);
        if (aOld != aNew)
            PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB,PAINT_GRID);

        
        
    }
}

bool ScDocShell::IsOle()
{
    return (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
}

void ScDocShell::UpdateOle( const ScViewData* pViewData, bool bSnapSize )
{
    
    

    if (GetCreateMode() == SFX_CREATE_MODE_STANDARD)
        return;

    OSL_ENSURE(pViewData,"pViewData==0 bei ScDocShell::UpdateOle");

    Rectangle aOldArea = SfxObjectShell::GetVisArea();
    Rectangle aNewArea = aOldArea;

    sal_Bool bEmbedded = aDocument.IsEmbedded();
    if (bEmbedded)
        aNewArea = aDocument.GetEmbeddedRect();
    else
    {
        SCTAB nTab = pViewData->GetTabNo();
        if ( nTab != aDocument.GetVisibleTab() )
            aDocument.SetVisibleTab( nTab );

        sal_Bool bNegativePage = aDocument.IsNegativePage( nTab );
        SCCOL nX = pViewData->GetPosX(SC_SPLIT_LEFT);
        SCROW nY = pViewData->GetPosY(SC_SPLIT_BOTTOM);
        Rectangle aMMRect = aDocument.GetMMRect( nX,nY, nX,nY, nTab );
        if (bNegativePage)
            lcl_SetTopRight( aNewArea, aMMRect.TopRight() );
        else
            aNewArea.SetPos( aMMRect.TopLeft() );
        if (bSnapSize)
            SnapVisArea(aNewArea);            
    }

    if (aNewArea != aOldArea)
        SetVisAreaOrSize( aNewArea, true ); 
}

//

//

SfxStyleSheetBasePool* ScDocShell::GetStyleSheetPool()
{
    return (SfxStyleSheetBasePool*)aDocument.GetStyleSheetPool();
}






static void lcl_AdjustPool( SfxStyleSheetBasePool* pStylePool )
{
    pStylePool->SetSearchMask(SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_ALL);
    SfxStyleSheetBase *pStyle = pStylePool->First();
    while ( pStyle )
    {
        SfxItemSet& rStyleSet = pStyle->GetItemSet();

        const SfxPoolItem* pItem;
        if (rStyleSet.GetItemState(ATTR_PAGE_HEADERSET,false,&pItem) == SFX_ITEM_SET)
        {
            SfxItemSet& rSrcSet = ((SvxSetItem*)pItem)->GetItemSet();
            SfxItemSet* pDestSet = new SfxItemSet(*rStyleSet.GetPool(),rSrcSet.GetRanges());
            pDestSet->Put(rSrcSet);
            rStyleSet.Put(SvxSetItem(ATTR_PAGE_HEADERSET,pDestSet));
        }
        if (rStyleSet.GetItemState(ATTR_PAGE_FOOTERSET,false,&pItem) == SFX_ITEM_SET)
        {
            SfxItemSet& rSrcSet = ((SvxSetItem*)pItem)->GetItemSet();
            SfxItemSet* pDestSet = new SfxItemSet(*rStyleSet.GetPool(),rSrcSet.GetRanges());
            pDestSet->Put(rSrcSet);
            rStyleSet.Put(SvxSetItem(ATTR_PAGE_FOOTERSET,pDestSet));
        }

        pStyle = pStylePool->Next();
    }
}

void ScDocShell::LoadStyles( SfxObjectShell &rSource )
{
    aDocument.StylesToNames();

    SfxObjectShell::LoadStyles(rSource);
    lcl_AdjustPool( GetStyleSheetPool() );      

    aDocument.UpdStlShtPtrsFrmNms();

    UpdateAllRowHeights();

        

    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID | PAINT_LEFT );
}

void ScDocShell::LoadStylesArgs( ScDocShell& rSource, bool bReplace, bool bCellStyles, bool bPageStyles )
{
    

    if ( !bCellStyles && !bPageStyles )     
        return;

    ScStyleSheetPool* pSourcePool = rSource.GetDocument()->GetStyleSheetPool();
    ScStyleSheetPool* pDestPool = aDocument.GetStyleSheetPool();

    SfxStyleFamily eFamily = bCellStyles ?
            ( bPageStyles ? SFX_STYLE_FAMILY_ALL : SFX_STYLE_FAMILY_PARA ) :
            SFX_STYLE_FAMILY_PAGE;
    SfxStyleSheetIterator aIter( pSourcePool, eFamily );
    sal_uInt16 nSourceCount = aIter.Count();
    if ( nSourceCount == 0 )
        return;                             

    ScStylePair* pStyles = new ScStylePair[ nSourceCount ];
    sal_uInt16 nFound = 0;

    

    SfxStyleSheetBase* pSourceStyle = aIter.First();
    while (pSourceStyle)
    {
        OUString aName = pSourceStyle->GetName();
        SfxStyleSheetBase* pDestStyle = pDestPool->Find( pSourceStyle->GetName(), pSourceStyle->GetFamily() );
        if ( pDestStyle )
        {
            
            if ( bReplace )
            {
                pStyles[nFound].pSource = pSourceStyle;
                pStyles[nFound].pDest = pDestStyle;
                ++nFound;
            }
        }
        else
        {
            pStyles[nFound].pSource = pSourceStyle;
            pStyles[nFound].pDest = &pDestPool->Make( aName, pSourceStyle->GetFamily(), pSourceStyle->GetMask() );
            ++nFound;
        }

        pSourceStyle = aIter.Next();
    }

    

    for ( sal_uInt16 i = 0; i < nFound; ++i )
    {
        pStyles[i].pDest->GetItemSet().PutExtended(
            pStyles[i].pSource->GetItemSet(), SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);
        if(pStyles[i].pSource->HasParentSupport())
            pStyles[i].pDest->SetParent(pStyles[i].pSource->GetParent());
        
    }

    lcl_AdjustPool( GetStyleSheetPool() );      
    UpdateAllRowHeights();
    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID | PAINT_LEFT );      

    delete[] pStyles;
}


void ScDocShell::ReconnectDdeLink(SfxObjectShell& rServer)
{
    ::sfx2::LinkManager* pLinkManager = aDocument.GetLinkManager();
    if (!pLinkManager)
        return;

    pLinkManager->ReconnectDdeLink(rServer);
}

void ScDocShell::UpdateLinks()
{
    typedef boost::unordered_set<OUString, OUStringHash> StrSetType;

    sfx2::LinkManager* pLinkManager = aDocument.GetLinkManager();
    StrSetType aNames;

    

    size_t nCount = pLinkManager->GetLinks().size();
    for (size_t k=nCount; k>0; )
    {
        --k;
        ::sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[k];
        if (pBase->ISA(ScTableLink))
        {
            ScTableLink* pTabLink = static_cast<ScTableLink*>(pBase);
            if (pTabLink->IsUsed())
                aNames.insert(pTabLink->GetFileName());
            else        
            {
                pTabLink->SetAddUndo(true);
                pLinkManager->Remove(k);
            }
        }
    }

    

    SCTAB nTabCount = aDocument.GetTableCount();
    for (SCTAB i = 0; i < nTabCount; ++i)
    {
        if (!aDocument.IsLinked(i))
            continue;

        OUString aDocName = aDocument.GetLinkDoc(i);
        OUString aFltName = aDocument.GetLinkFlt(i);
        OUString aOptions = aDocument.GetLinkOpt(i);
        sal_uLong nRefresh  = aDocument.GetLinkRefreshDelay(i);
        bool bThere = false;
        for (SCTAB j = 0; j < i && !bThere; ++j)                
        {
            if (aDocument.IsLinked(j)
                    && aDocument.GetLinkDoc(j) == aDocName
                    && aDocument.GetLinkFlt(j) == aFltName
                    && aDocument.GetLinkOpt(j) == aOptions)
                    
                    
                    
                bThere = true;
        }

        if (!bThere)                                        
        {
            if (!aNames.insert(aDocName).second)
                bThere = true;
        }

        if (!bThere)
        {
            ScTableLink* pLink = new ScTableLink( this, aDocName, aFltName, aOptions, nRefresh );
            pLink->SetInCreate(true);
            pLinkManager->InsertFileLink(*pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName);
            pLink->Update();
            pLink->SetInCreate(false);
        }
    }
}

bool ScDocShell::ReloadTabLinks()
{
    sfx2::LinkManager* pLinkManager = aDocument.GetLinkManager();

    bool bAny = false;
    size_t nCount = pLinkManager->GetLinks().size();
    for (size_t i=0; i<nCount; i++ )
    {
        ::sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
        if (pBase->ISA(ScTableLink))
        {
            ScTableLink* pTabLink = static_cast<ScTableLink*>(pBase);


			
			
            
            pTabLink->Update();
            

            bAny = true;
        }
    }

    if ( bAny )
    {
        
        PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
                                    PAINT_GRID | PAINT_TOP | PAINT_LEFT );

        SetDocumentModified();
    }

    return true;        
}

void ScDocShell::SetFormulaOptions(const ScFormulaOptions& rOpt )
{
    aDocument.SetGrammar( rOpt.GetFormulaSyntax() );

    
    
    if (rOpt.GetUseEnglishFuncName())
    {
        
        ScCompiler aComp(NULL, ScAddress());
        ScCompiler::OpCodeMapPtr xMap = aComp.GetOpCodeMap(::com::sun::star::sheet::FormulaLanguage::ENGLISH);
        ScCompiler::SetNativeSymbols(xMap);
    }
    else
        
        ScCompiler::ResetNativeSymbols();

    
    ScGlobal::ResetFunctionList();

    
    ScCompiler::UpdateSeparatorsNative(
        rOpt.GetFormulaSepArg(), rOpt.GetFormulaSepArrayCol(), rOpt.GetFormulaSepArrayRow());

    
    ScInterpreter::SetGlobalConfig(rOpt.GetCalcConfig());
}

void ScDocShell::CheckConfigOptions()
{
    if (IsConfigOptionsChecked())
        
        return;

    OUString aDecSep = ScGlobal::GetpLocaleData()->getNumDecimalSep();

    ScModule* pScMod = SC_MOD();
    const ScFormulaOptions& rOpt=pScMod->GetFormulaOptions();
    OUString aSepArg = rOpt.GetFormulaSepArg();
    OUString aSepArrRow = rOpt.GetFormulaSepArrayRow();
    OUString aSepArrCol = rOpt.GetFormulaSepArrayCol();

    if (aDecSep == aSepArg || aDecSep == aSepArrRow || aDecSep == aSepArrCol)
    {
        
        
        ScFormulaOptions aNew = rOpt;
        aNew.ResetFormulaSeparators();
        SetFormulaOptions(aNew);
        pScMod->SetFormulaOptions(aNew);

        
        ScTabViewShell* pViewShell = GetBestViewShell();
        if (pViewShell)
        {
            Window* pParent = pViewShell->GetFrameWin();
            InfoBox aBox(pParent, ScGlobal::GetRscString(STR_OPTIONS_WARN_SEPARATORS));
            aBox.Execute();
        }

        
        
        
        
    }

    SetConfigOptionsChecked(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
