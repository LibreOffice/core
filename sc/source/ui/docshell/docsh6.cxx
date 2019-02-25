/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <scitems.hxx>

#include <svx/pageitem.hxx>
#include <sfx2/linkmgr.hxx>

#include <docsh.hxx>

#include <stlpool.hxx>
#include <global.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <tablink.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scmod.hxx>
#include <compiler.hxx>
#include <interpre.hxx>
#include <formulaopt.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <utility>

namespace {

struct ScStylePair
{
    SfxStyleSheetBase *pSource;
    SfxStyleSheetBase *pDest;
};

}

//  Ole

void ScDocShell::SetVisArea( const tools::Rectangle & rVisArea )
{
    //  with the SnapVisArea call in SetVisAreaOrSize, it's safe to always
    //  use both the size and position of the VisArea
    SetVisAreaOrSize( rVisArea );
}

static void lcl_SetTopRight( tools::Rectangle& rRect, const Point& rPos )
{
    Size aSize = rRect.GetSize();
    rRect.SetRight( rPos.X() );
    rRect.SetLeft( rPos.X() - aSize.Width() + 1 );
    rRect.SetTop( rPos.Y() );
    rRect.SetBottom( rPos.Y() + aSize.Height() - 1 );
}

void ScDocShell::SetVisAreaOrSize( const tools::Rectangle& rVisArea )
{
    bool bNegativePage = m_aDocument.IsNegativePage( m_aDocument.GetVisibleTab() );

    tools::Rectangle aArea = rVisArea;
    // when loading, don't check for negative values, because the sheet orientation
    // might be set later
    if ( !m_aDocument.IsImportingXML() )
    {
        if ( ( bNegativePage ? (aArea.Right() > 0) : (aArea.Left() < 0) ) || aArea.Top() < 0 )
        {
            //  VisArea start position can't be negative.
            //  Move the VisArea, otherwise only the upper left position would
            //  be changed in SnapVisArea, and the size would be wrong.

            Point aNewPos( 0, std::max( aArea.Top(), long(0) ) );
            if ( bNegativePage )
            {
                aNewPos.setX( std::min( aArea.Right(), long(0) ) );
                lcl_SetTopRight( aArea, aNewPos );
            }
            else
            {
                aNewPos.setX( std::max( aArea.Left(), long(0) ) );
                aArea.SetPos( aNewPos );
            }
        }
    }

    //      adjust position here!

    //  when loading an ole object, the VisArea is set from the document's
    //  view settings and must be used as-is (document content may not be complete yet).
    if ( !m_aDocument.IsImportingXML() )
        SnapVisArea( aArea );

    //TODO/LATER: it's unclear which IPEnv is used here
    /*
    SvInPlaceEnvironment* pEnv = GetIPEnv();
    if (pEnv)
    {
        vcl::Window* pWin = pEnv->GetEditWin();
        pEnv->MakeScale( aArea.GetSize(), MapUnit::Map100thMM,
                            pWin->LogicToPixel( aArea.GetSize() ) );
    } */

    //TODO/LATER: formerly in SvInplaceObject
    SfxObjectShell::SetVisArea( aArea );

    if (m_bIsInplace)                     // adjust zoom in the InPlace View
    {
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if (pViewSh)
        {
            if (pViewSh->GetViewData().GetDocShell() == this)
                pViewSh->UpdateOleZoom();
        }
    }

    if (m_aDocument.IsEmbedded())
    {
        ScRange aOld;
        m_aDocument.GetEmbedded( aOld);
        m_aDocument.SetEmbedded( m_aDocument.GetVisibleTab(), aArea );
        ScRange aNew;
        m_aDocument.GetEmbedded( aNew);
        if (aOld != aNew)
            PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB,PaintPartFlags::Grid);

        //TODO/LATER: currently not implemented
        //ViewChanged( ASPECT_CONTENT );          // show in the container as well
    }
}

bool ScDocShell::IsOle()
{
    return (GetCreateMode() == SfxObjectCreateMode::EMBEDDED);
}

void ScDocShell::UpdateOle( const ScViewData* pViewData, bool bSnapSize )
{
    //  if it isn't Ole at all, one can be spared the calculations
    //  (VisArea will then be reset at the save)

    if (GetCreateMode() == SfxObjectCreateMode::STANDARD)
        return;

    OSL_ENSURE(pViewData,"pViewData==0 at ScDocShell::UpdateOle");

    tools::Rectangle aOldArea = SfxObjectShell::GetVisArea();
    tools::Rectangle aNewArea = aOldArea;

    bool bEmbedded = m_aDocument.IsEmbedded();
    if (bEmbedded)
        aNewArea = m_aDocument.GetEmbeddedRect();
    else
    {
        SCTAB nTab = pViewData->GetTabNo();
        if ( nTab != m_aDocument.GetVisibleTab() )
            m_aDocument.SetVisibleTab( nTab );

        bool bNegativePage = m_aDocument.IsNegativePage( nTab );
        SCCOL nX = pViewData->GetPosX(SC_SPLIT_LEFT);
        if ( nX != m_aDocument.GetPosLeft() )
            m_aDocument.SetPosLeft( nX );
        SCROW nY = pViewData->GetPosY(SC_SPLIT_BOTTOM);
        if ( nY != m_aDocument.GetPosTop() )
            m_aDocument.SetPosTop( nY );
        tools::Rectangle aMMRect = m_aDocument.GetMMRect( nX,nY, nX,nY, nTab );
        if (bNegativePage)
            lcl_SetTopRight( aNewArea, aMMRect.TopRight() );
        else
            aNewArea.SetPos( aMMRect.TopLeft() );
        if (bSnapSize)
            SnapVisArea(aNewArea);            // uses the new VisibleTab
    }

    if (aNewArea != aOldArea)
        SetVisAreaOrSize( aNewArea ); // the start must also be adjusted here
}

//  Style stuff for Organizer, etc.

SfxStyleSheetBasePool* ScDocShell::GetStyleSheetPool()
{
    return static_cast<SfxStyleSheetBasePool*>(m_aDocument.GetStyleSheetPool());
}

//  After loading styles from another document (LoadStyles, Insert), the SetItems
//  (ATTR_PAGE_HEADERSET, ATTR_PAGE_FOOTERSET) must be converted to the correct pool
//  before the source pool is deleted.

static void lcl_AdjustPool( SfxStyleSheetBasePool* pStylePool )
{
    pStylePool->SetSearchMask(SfxStyleFamily::Page);
    SfxStyleSheetBase *pStyle = pStylePool->First();
    while ( pStyle )
    {
        SfxItemSet& rStyleSet = pStyle->GetItemSet();

        const SfxPoolItem* pItem;
        if (rStyleSet.GetItemState(ATTR_PAGE_HEADERSET,false,&pItem) == SfxItemState::SET)
        {
            const SfxItemSet& rSrcSet = static_cast<const SvxSetItem*>(pItem)->GetItemSet();
            auto pDestSet = std::make_unique<SfxItemSet>(*rStyleSet.GetPool(),rSrcSet.GetRanges());
            pDestSet->Put(rSrcSet);
            rStyleSet.Put(SvxSetItem(ATTR_PAGE_HEADERSET,std::move(pDestSet)));
        }
        if (rStyleSet.GetItemState(ATTR_PAGE_FOOTERSET,false,&pItem) == SfxItemState::SET)
        {
            const SfxItemSet& rSrcSet = static_cast<const SvxSetItem*>(pItem)->GetItemSet();
            auto pDestSet = std::make_unique<SfxItemSet>(*rStyleSet.GetPool(),rSrcSet.GetRanges());
            pDestSet->Put(rSrcSet);
            rStyleSet.Put(SvxSetItem(ATTR_PAGE_FOOTERSET,std::move(pDestSet)));
        }

        pStyle = pStylePool->Next();
    }
}

void ScDocShell::LoadStyles( SfxObjectShell &rSource )
{
    m_aDocument.StylesToNames();

    SfxObjectShell::LoadStyles(rSource);
    lcl_AdjustPool( GetStyleSheetPool() );      // adjust SetItems

    m_aDocument.UpdStlShtPtrsFrmNms();

    UpdateAllRowHeights();

        //  Paint

    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Grid | PaintPartFlags::Left );
}

void ScDocShell::LoadStylesArgs( ScDocShell& rSource, bool bReplace, bool bCellStyles, bool bPageStyles )
{
    //  similar to LoadStyles, but with selectable behavior for XStyleLoader::loadStylesFromURL call

    if ( !bCellStyles && !bPageStyles )     // nothing to do
        return;

    ScStyleSheetPool* pSourcePool = rSource.GetDocument().GetStyleSheetPool();
    ScStyleSheetPool* pDestPool = m_aDocument.GetStyleSheetPool();

    SfxStyleFamily eFamily = bCellStyles ?
            ( bPageStyles ? SfxStyleFamily::All : SfxStyleFamily::Para ) :
            SfxStyleFamily::Page;
    SfxStyleSheetIterator aIter( pSourcePool, eFamily );
    sal_uInt16 nSourceCount = aIter.Count();
    if ( nSourceCount == 0 )
        return;                             // no source styles

    std::unique_ptr<ScStylePair[]> pStyles(new ScStylePair[ nSourceCount ]);
    sal_uInt16 nFound = 0;

    //  first create all new styles

    SfxStyleSheetBase* pSourceStyle = aIter.First();
    while (pSourceStyle)
    {
        OUString aName = pSourceStyle->GetName();
        SfxStyleSheetBase* pDestStyle = pDestPool->Find( pSourceStyle->GetName(), pSourceStyle->GetFamily() );
        if ( pDestStyle )
        {
            // touch existing styles only if replace flag is set
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

    //  then copy contents (after inserting all styles, for parent etc.)

    for ( sal_uInt16 i = 0; i < nFound; ++i )
    {
        pStyles[i].pDest->GetItemSet().PutExtended(
            pStyles[i].pSource->GetItemSet(), SfxItemState::DONTCARE, SfxItemState::DEFAULT);
        if(pStyles[i].pSource->HasParentSupport())
            pStyles[i].pDest->SetParent(pStyles[i].pSource->GetParent());
        // follow is never used
    }

    lcl_AdjustPool( GetStyleSheetPool() );      // adjust SetItems
    UpdateAllRowHeights();
    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Grid | PaintPartFlags::Left );      // Paint
}

void ScDocShell::ReconnectDdeLink(SfxObjectShell& rServer)
{
    ::sfx2::LinkManager* pLinkManager = m_aDocument.GetLinkManager();
    if (!pLinkManager)
        return;

    pLinkManager->ReconnectDdeLink(rServer);
}

void ScDocShell::UpdateLinks()
{
    typedef std::unordered_set<OUString> StrSetType;

    sfx2::LinkManager* pLinkManager = m_aDocument.GetLinkManager();
    StrSetType aNames;

    // out with the no longer used links

    size_t nCount = pLinkManager->GetLinks().size();
    for (size_t k=nCount; k>0; )
    {
        --k;
        ::sfx2::SvBaseLink* pBase = pLinkManager->GetLinks()[k].get();
        if (ScTableLink* pTabLink = dynamic_cast<ScTableLink*>(pBase))
        {
            if (pTabLink->IsUsed())
                aNames.insert(pTabLink->GetFileName());
            else        // no longer used -> delete
            {
                pTabLink->SetAddUndo(true);
                pLinkManager->Remove(k);
            }
        }
    }

    // enter new links

    SCTAB nTabCount = m_aDocument.GetTableCount();
    for (SCTAB i = 0; i < nTabCount; ++i)
    {
        if (!m_aDocument.IsLinked(i))
            continue;

        OUString aDocName = m_aDocument.GetLinkDoc(i);
        OUString aFltName = m_aDocument.GetLinkFlt(i);
        OUString aOptions = m_aDocument.GetLinkOpt(i);
        sal_uLong nRefresh  = m_aDocument.GetLinkRefreshDelay(i);
        bool bThere = false;
        for (SCTAB j = 0; j < i && !bThere; ++j)                // several times in the document?
        {
            if (m_aDocument.IsLinked(j)
                    && m_aDocument.GetLinkDoc(j) == aDocName
                    && m_aDocument.GetLinkFlt(j) == aFltName
                    && m_aDocument.GetLinkOpt(j) == aOptions)
                    // Ignore refresh delay in compare, it should be the
                    // same for identical links and we don't want dupes
                    // if it ain't.
                bThere = true;
        }

        if (!bThere)                                        // already entered as filter?
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

void ScDocShell::ReloadTabLinks()
{
    sfx2::LinkManager* pLinkManager = m_aDocument.GetLinkManager();

    bool bAny = false;
    size_t nCount = pLinkManager->GetLinks().size();
    for (size_t i=0; i<nCount; i++ )
    {
        ::sfx2::SvBaseLink* pBase = pLinkManager->GetLinks()[i].get();
        if (ScTableLink* pTabLink = dynamic_cast<ScTableLink*>(pBase))
        {
//          pTabLink->SetAddUndo(sal_False);        //! merge Undos

            // Painting only after Update() makes no sense:
            // ScTableLink::Refresh() will post a Paint only is bDoPaint is true
            // pTabLink->SetPaint(false);          //  Paint only once at the end
            pTabLink->Update();
            //pTabLink->SetPaint(true);
//          pTabLink->SetAddUndo(sal_True);
            bAny = true;
        }
    }

    if ( bAny )
    {
        //  Paint only once
        PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
                                    PaintPartFlags::Grid | PaintPartFlags::Top | PaintPartFlags::Left );

        SetDocumentModified();
    }
}

void ScDocShell::SetFormulaOptions( const ScFormulaOptions& rOpt, bool bForLoading )
{
    m_aDocument.SetGrammar( rOpt.GetFormulaSyntax() );

    // This is nasty because it resets module globals from within a docshell!
    // For actual damage caused see fdo#82183 where an unconditional
    // ScGlobal::ResetFunctionList() (without checking GetUseEnglishFuncName())
    // lead to a crash because the function list was still used by the Formula
    // Wizard when loading the second document.
    // Do the stupid stuff only when we're not called while loading a document.

    /* TODO: bForLoading is a workaround, rather get rid of setting any
     * globals from per document instances like ScDocShell. */

    /* XXX  this is utter crap, we rely on the options being set here at least
     * once, for the very first document, empty or loaded. */
    static bool bInitOnce = true;

    if (!bForLoading || bInitOnce)
    {
        bool bForceInit = bInitOnce;
        bInitOnce = false;
        if (bForceInit || rOpt.GetUseEnglishFuncName() != SC_MOD()->GetFormulaOptions().GetUseEnglishFuncName())
        {
            // This needs to be called first since it may re-initialize the entire
            // opcode map.
            if (rOpt.GetUseEnglishFuncName())
            {
                // switch native symbols to English.
                ScCompiler aComp(nullptr, ScAddress());
                ScCompiler::OpCodeMapPtr xMap = aComp.GetOpCodeMap(css::sheet::FormulaLanguage::ENGLISH);
                ScCompiler::SetNativeSymbols(xMap);
            }
            else
                // re-initialize native symbols with localized function names.
                ScCompiler::ResetNativeSymbols();

            // Force re-population of function names for the function wizard, function tip etc.
            ScGlobal::ResetFunctionList();
        }

        // Update the separators.
        ScCompiler::UpdateSeparatorsNative(
                rOpt.GetFormulaSepArg(), rOpt.GetFormulaSepArrayCol(), rOpt.GetFormulaSepArrayRow());

        // Global interpreter settings.
        ScInterpreter::SetGlobalConfig(rOpt.GetCalcConfig());
    }

    // Per document interpreter settings.
    m_aDocument.SetCalcConfig( rOpt.GetCalcConfig() );
}

void ScDocShell::CheckConfigOptions()
{
    if (IsConfigOptionsChecked())
        // no need to check repeatedly.
        return;

    OUString aDecSep = ScGlobal::GetpLocaleData()->getNumDecimalSep();
    OUString aDecSepAlt = ScGlobal::GetpLocaleData()->getNumDecimalSepAlt();

    ScModule* pScMod = SC_MOD();
    const ScFormulaOptions& rOpt=pScMod->GetFormulaOptions();
    const OUString& aSepArg = rOpt.GetFormulaSepArg();
    const OUString& aSepArrRow = rOpt.GetFormulaSepArrayRow();
    const OUString& aSepArrCol = rOpt.GetFormulaSepArrayCol();

    if (aDecSep == aSepArg || aDecSep == aSepArrRow || aDecSep == aSepArrCol ||
            aDecSepAlt == aSepArg || aDecSepAlt == aSepArrRow || aDecSepAlt == aSepArrCol)
    {
        // One of arg separators conflicts with the current decimal
        // separator.  Reset them to default.
        ScFormulaOptions aNew = rOpt;
        aNew.ResetFormulaSeparators();
        SetFormulaOptions(aNew);
        pScMod->SetFormulaOptions(aNew);

        // Launch a nice warning dialog to let the users know of this change.
        ScTabViewShell* pViewShell = GetBestViewShell();
        if (pViewShell)
        {
            vcl::Window* pParent = pViewShell->GetFrameWin();
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pParent ? pParent->GetFrameWeld() : nullptr,
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          ScResId(STR_OPTIONS_WARN_SEPARATORS)));
            xInfoBox->run();
        }

        // For now, this is the only option setting that could launch info
        // dialog.  But in the future we may want to implement a nicer
        // dialog to display a list of warnings in case we have several
        // pieces of information to display.
    }

    SetConfigOptionsChecked(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
