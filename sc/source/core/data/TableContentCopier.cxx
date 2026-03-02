/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <TableContentCopier.hxx>
#include <document.hxx>
#include <table.hxx>
#include <rangenam.hxx>
#include <dbdata.hxx>
#include <markdata.hxx>
#include <dpobject.hxx>
#include <refupdatecontext.hxx>
#include <clipcontext.hxx>
#include <formulacell.hxx>

sc::TableContentCopier::TableContentCopier(ScDocument& rDoc, SCTAB nSourceTabNo, SCTAB nTargetTabNo)
    : mrDoc(rDoc)
    , mnSourceTabNo(nSourceTabNo)
    , mnTargetTabNo(nTargetTabNo)
    , mpSourceTab(mrDoc.FetchTable(nSourceTabNo))
    , mpTargetTab(mrDoc.FetchTable(nTargetTabNo))
{
}

void sc::TableContentCopier::performCopy(const ScMarkData* pOnlyMarked, ScCloneFlags nCloneFlags,
                                         SCTAB nPreviousSourceTabNo)
{
    if (nPreviousSourceTabNo < 0)
        nPreviousSourceTabNo = mnSourceTabNo;

    const bool bGlobalNamesToLocal = true;
    const ScRangeName* pNames = mrDoc.GetRangeName(mnSourceTabNo);
    if (pNames)
        pNames->CopyUsedNames(mnSourceTabNo, nPreviousSourceTabNo, mnTargetTabNo, mrDoc, mrDoc,
                              bGlobalNamesToLocal);
    mrDoc.GetRangeName()->CopyUsedNames(-1, nPreviousSourceTabNo, mnTargetTabNo, mrDoc, mrDoc,
                                        bGlobalNamesToLocal);

    sc::CopyToDocContext aCopyDocCxt(mrDoc);
    if (mrDoc.pDBCollection)
        mrDoc.pDBCollection->CopyToTable(mnSourceTabNo, mnTargetTabNo);
    mpSourceTab->CopyToTable(aCopyDocCxt, 0, 0, mrDoc.MaxCol(), mrDoc.MaxRow(),
                             InsertDeleteFlags::ALL, (pOnlyMarked != nullptr), mpTargetTab,
                             pOnlyMarked, false /*bAsLink*/, true /*bColRowFlags*/, nCloneFlags,
                             false /*bCopyCaptions*/);
    mpTargetTab->SetTabBgColor(mpSourceTab->GetTabBgColor());

    SCTAB nDz = mnTargetTabNo - mnSourceTabNo;
    sc::RefUpdateContext aRefContext(mrDoc);
    aRefContext.meMode = URM_COPY;
    aRefContext.maRange
        = ScRange(0, 0, mnTargetTabNo, mrDoc.MaxCol(), mrDoc.MaxRow(), mnTargetTabNo);
    aRefContext.mnTabDelta = nDz;
    mpTargetTab->UpdateReference(aRefContext);

    sc::SetFormulaDirtyContext aFormulaDirtyCxt;
    mrDoc.SetAllFormulasDirty(aFormulaDirtyCxt);

    if (mrDoc.mpDrawLayer) //  Skip cloning Note caption object
        // page is already created in ScTable ctor
        mrDoc.mpDrawLayer->ScCopyPage(static_cast<sal_uInt16>(mnSourceTabNo),
                                      static_cast<sal_uInt16>(mnTargetTabNo));

    if (mrDoc.pDPCollection)
        mrDoc.pDPCollection->CopyToTab(mnSourceTabNo, mnTargetTabNo);

    mpTargetTab->SetPageStyle(mpSourceTab->GetPageStyle());
    mpTargetTab->SetPendingRowHeights(mpSourceTab->IsPendingRowHeights());

    // Copy the custom print range if exists.
    mpTargetTab->CopyPrintRange(*mpSourceTab);

    // Copy the RTL settings
    mpTargetTab->SetLayoutRTL(mpSourceTab->IsLayoutRTL());
    mpTargetTab->SetLoadingRTL(mpSourceTab->IsLoadingRTL());

    // Finally copy the note captions, which need
    // 1. the updated source ScColumn::nTab members if the target tab is before the source tab
    // 2. row heights and column widths of the destination
    // 3. RTL settings of the destination
    mpSourceTab->CopyCaptionsToTable(0, 0, mrDoc.MaxCol(), mrDoc.MaxRow(), mpTargetTab,
                                     true /*bCloneCaption*/);
}

void sc::TableContentCopier::updateReferencesAfterTabInsertion(
    sc::RefUpdateInsertTabContext& rContext)
{
    mpTargetTab->UpdateInsertTabAbs(mnTargetTabNo);
    mpTargetTab->AdjustRelativeTabRefs(mnSourceTabNo, mnTargetTabNo, sc::TargetTabState::Inserted);
    mpSourceTab->UpdateInsertTab(rContext);
    mpSourceTab->UpdateCompile();
}

void sc::TableContentCopier::recompileTargetFormulas()
{
    //  maybe already compiled in Clone, but used names need recompilation
    mpTargetTab->UpdateCompile(true);
}

sc::TableContentCopier::~TableContentCopier() { recompileTargetFormulas(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
