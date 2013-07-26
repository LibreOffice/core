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

// STATIC DATA -----------------------------------------------------------

//----------------------------------------------------------------------

//
//  Ole
//

void ScDocShell::SetVisArea( const Rectangle & rVisArea )
{
    //  with the SnapVisArea call in SetVisAreaOrSize, it's safe to always
    //  use both the size and position of the VisArea
    SetVisAreaOrSize( rVisArea, sal_True );
}

static void lcl_SetTopRight( Rectangle& rRect, const Point& rPos )
{
    Size aSize = rRect.GetSize();
    rRect.Right() = rPos.X();
    rRect.Left() = rPos.X() - aSize.Width() + 1;
    rRect.Top() = rPos.Y();
    rRect.Bottom() = rPos.Y() + aSize.Height() - 1;
}

void ScDocShell::SetVisAreaOrSize( const Rectangle& rVisArea, sal_Bool bModifyStart )
{
    sal_Bool bNegativePage = aDocument.IsNegativePage( aDocument.GetVisibleTab() );

    Rectangle aArea = rVisArea;
    if (bModifyStart)
    {
        // when loading, don't check for negative values, because the sheet orientation
        // might be set later
        if ( !aDocument.IsImportingXML() )
        {
            if ( ( bNegativePage ? (aArea.Right() > 0) : (aArea.Left() < 0) ) || aArea.Top() < 0 )
            {
                //  VisArea start position can't be negative.
                //  Move the VisArea, otherwise only the upper left position would
                //  be changed in SnapVisArea, and the size would be wrong.

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

    //      hier Position anpassen!

    //  when loading an ole object, the VisArea is set from the document's
    //  view settings and must be used as-is (document content may not be complete yet).
    if ( !aDocument.IsImportingXML() )
        SnapVisArea( aArea );

    //TODO/LATER: it's unclear which IPEnv is used here
    /*
    SvInPlaceEnvironment* pEnv = GetIPEnv();
    if (pEnv)
    {
        Window* pWin = pEnv->GetEditWin();
        pEnv->MakeScale( aArea.GetSize(), MAP_100TH_MM,
                            pWin->LogicToPixel( aArea.GetSize() ) );
    } */

    //TODO/LATER: formerly in SvInplaceObject
    SfxObjectShell::SetVisArea( aArea );

    if (bIsInplace)                     // Zoom in der InPlace View einstellen
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

        //TODO/LATER: currently not implemented
        //ViewChanged( ASPECT_CONTENT );          // auch im Container anzeigen
    }
}

sal_Bool ScDocShell::IsOle()
{
    return (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
}

void ScDocShell::UpdateOle( const ScViewData* pViewData, sal_Bool bSnapSize )
{
    //  wenn's gar nicht Ole ist, kann man sich die Berechnungen sparen
    //  (VisArea wird dann beim Save wieder zurueckgesetzt)

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
            SnapVisArea(aNewArea);            // uses the new VisibleTab
    }

    if (aNewArea != aOldArea)
        SetVisAreaOrSize( aNewArea, true ); // hier muss auch der Start angepasst werden
}

//
//  Style-Krempel fuer Organizer etc.
//

SfxStyleSheetBasePool* ScDocShell::GetStyleSheetPool()
{
    return (SfxStyleSheetBasePool*)aDocument.GetStyleSheetPool();
}


//  nach dem Laden von Vorlagen aus einem anderen Dokment (LoadStyles, Insert)
//  muessen die SetItems (ATTR_PAGE_HEADERSET, ATTR_PAGE_FOOTERSET) auf den richtigen
//  Pool umgesetzt werden, bevor der Quell-Pool geloescht wird.

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
    lcl_AdjustPool( GetStyleSheetPool() );      // SetItems anpassen

    aDocument.UpdStlShtPtrsFrmNms();

    UpdateAllRowHeights();

        //  Paint

    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID | PAINT_LEFT );
}

void ScDocShell::LoadStylesArgs( ScDocShell& rSource, bool bReplace, bool bCellStyles, bool bPageStyles )
{
    //  similar to LoadStyles, but with selectable behavior for XStyleLoader::loadStylesFromURL call

    if ( !bCellStyles && !bPageStyles )     // nothing to do
        return;

    ScStyleSheetPool* pSourcePool = rSource.GetDocument()->GetStyleSheetPool();
    ScStyleSheetPool* pDestPool = aDocument.GetStyleSheetPool();

    SfxStyleFamily eFamily = bCellStyles ?
            ( bPageStyles ? SFX_STYLE_FAMILY_ALL : SFX_STYLE_FAMILY_PARA ) :
            SFX_STYLE_FAMILY_PAGE;
    SfxStyleSheetIterator aIter( pSourcePool, eFamily );
    sal_uInt16 nSourceCount = aIter.Count();
    if ( nSourceCount == 0 )
        return;                             // no source styles

    ScStylePair* pStyles = new ScStylePair[ nSourceCount ];
    sal_uInt16 nFound = 0;

    //  first create all new styles

    SfxStyleSheetBase* pSourceStyle = aIter.First();
    while (pSourceStyle)
    {
        String aName = pSourceStyle->GetName();
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
            pStyles[i].pSource->GetItemSet(), SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);
        if(pStyles[i].pSource->HasParentSupport())
            pStyles[i].pDest->SetParent(pStyles[i].pSource->GetParent());
        // follow is never used
    }

    lcl_AdjustPool( GetStyleSheetPool() );      // adjust SetItems
    UpdateAllRowHeights();
    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID | PAINT_LEFT );      // Paint

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

    // nicht mehr benutzte Links raus

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
            else        // nicht mehr benutzt -> loeschen
            {
                pTabLink->SetAddUndo(sal_True);
                pLinkManager->Remove(k);
            }
        }
    }

    // neue Links eintragen

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
        for (SCTAB j = 0; j < i && !bThere; ++j)                // im Dokument mehrfach?
        {
            if (aDocument.IsLinked(j)
                    && aDocument.GetLinkDoc(j) == aDocName
                    && aDocument.GetLinkFlt(j) == aFltName
                    && aDocument.GetLinkOpt(j) == aOptions)
                    // Ignore refresh delay in compare, it should be the
                    // same for identical links and we don't want dupes
                    // if it ain't.
                bThere = true;
        }

        if (!bThere)                                        // schon als Filter eingetragen?
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

sal_Bool ScDocShell::ReloadTabLinks()
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
//			pTabLink->SetAddUndo(sal_False);		//! Undo's zusammenfassen

			// Painting only after Update() makes no sense:
			// ScTableLink::Refresh() will post a Paint only is bDoPaint is true
            // pTabLink->SetPaint(false);          //  Paint nur einmal am Ende
            pTabLink->Update();
            //pTabLink->SetPaint(true);
//			pTabLink->SetAddUndo(sal_True);
            bAny = true;
        }
    }

    if ( bAny )
    {
        //  Paint nur einmal
        PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
                                    PAINT_GRID | PAINT_TOP | PAINT_LEFT );

        SetDocumentModified();
    }

    return sal_True;        //! Fehler erkennen
}

void ScDocShell::SetFormulaOptions(const ScFormulaOptions& rOpt )
{
    aDocument.SetGrammar( rOpt.GetFormulaSyntax() );

    // This needs to be called first since it may re-initialize the entire
    // opcode map.
    if (rOpt.GetUseEnglishFuncName())
    {
        // switch native symbols to English.
        ScCompiler aComp(NULL, ScAddress());
        ScCompiler::OpCodeMapPtr xMap = aComp.GetOpCodeMap(::com::sun::star::sheet::FormulaLanguage::ENGLISH);
        ScCompiler::SetNativeSymbols(xMap);
    }
    else
        // re-initialize native symbols with localized function names.
        ScCompiler::ResetNativeSymbols();

    // Force re-population of function names for the function wizard, function tip etc.
    ScGlobal::ResetFunctionList();

    // Update the separators.
    ScCompiler::UpdateSeparatorsNative(
        rOpt.GetFormulaSepArg(), rOpt.GetFormulaSepArrayCol(), rOpt.GetFormulaSepArrayRow());

    // Global interpreter settings.
    ScInterpreter::SetGlobalConfig(rOpt.GetCalcConfig());
}

void ScDocShell::CheckConfigOptions()
{
    if (IsConfigOptionsChecked())
        // no need to check repeatedly.
        return;

    OUString aDecSep = ScGlobal::GetpLocaleData()->getNumDecimalSep();

    ScModule* pScMod = SC_MOD();
    const ScFormulaOptions& rOpt=pScMod->GetFormulaOptions();
    OUString aSepArg = rOpt.GetFormulaSepArg();
    OUString aSepArrRow = rOpt.GetFormulaSepArrayRow();
    OUString aSepArrCol = rOpt.GetFormulaSepArrayCol();

    if (aDecSep == aSepArg || aDecSep == aSepArrRow || aDecSep == aSepArrCol)
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
            Window* pParent = pViewShell->GetFrameWin();
            InfoBox aBox(pParent, ScGlobal::GetRscString(STR_OPTIONS_WARN_SEPARATORS));
            aBox.Execute();
        }

        // For now, this is the only option setting that could launch info
        // dialog.  But in the future we may want to implement a nicer
        // dialog to display a list of warnings in case we have several
        // pieces of information to display.
    }

    SetConfigOptionsChecked(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
