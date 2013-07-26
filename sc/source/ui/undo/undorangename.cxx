/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "undorangename.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "docfunc.hxx"
#include "sc.hrc"

#include "sfx2/app.hxx"

#include <memory>

using ::std::auto_ptr;

ScUndoAllRangeNames::ScUndoAllRangeNames(
    ScDocShell* pDocSh,
    const std::map<OUString, ScRangeName*>& rOldNames,
    const boost::ptr_map<OUString, ScRangeName>& rNewNames) :
    ScSimpleUndo(pDocSh)
{
    std::map<OUString, ScRangeName*>::const_iterator itr, itrEnd;
    for (itr = rOldNames.begin(), itrEnd = rOldNames.end(); itr != itrEnd; ++itr)
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        auto_ptr<ScRangeName> p(new ScRangeName(*itr->second));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        maOldNames.insert(itr->first, p);
    }

    boost::ptr_map<OUString, ScRangeName>::const_iterator it, itEnd;
    for (it = rNewNames.begin(), itEnd = rNewNames.end(); it != itEnd; ++it)
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        auto_ptr<ScRangeName> p(new ScRangeName(*it->second));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        maNewNames.insert(it->first, p);
    }
}

ScUndoAllRangeNames::~ScUndoAllRangeNames()
{
}

void ScUndoAllRangeNames::Undo()
{
    DoChange(maOldNames);
}

void ScUndoAllRangeNames::Redo()
{
    DoChange(maNewNames);
}

void ScUndoAllRangeNames::Repeat(SfxRepeatTarget& /*rTarget*/)
{
}

bool ScUndoAllRangeNames::CanRepeat(SfxRepeatTarget& /*rTarget*/) const
{
    return false;
}

OUString ScUndoAllRangeNames::GetComment() const
{
    return ScGlobal::GetRscString(STR_UNDO_RANGENAMES);
}

void ScUndoAllRangeNames::DoChange(const boost::ptr_map<OUString, ScRangeName>& rNames)
{
    ScDocument& rDoc = *pDocShell->GetDocument();

    rDoc.CompileNameFormula(true);

    rDoc.SetAllRangeNames(rNames);

    rDoc.CompileNameFormula(true);

    SFX_APP()->Broadcast(SfxSimpleHint(SC_HINT_AREAS_CHANGED));
}

ScUndoAddRangeData::ScUndoAddRangeData(ScDocShell* pDocSh, ScRangeData* pRangeData, SCTAB nTab) :
    ScSimpleUndo(pDocSh),
    mpRangeData(new ScRangeData(*pRangeData)),
    mnTab(nTab)
{

}

ScUndoAddRangeData::~ScUndoAddRangeData()
{
    delete mpRangeData;
}

void ScUndoAddRangeData::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScRangeName* pRangeName = NULL;
    if (mnTab == -1)
    {
        pRangeName = pDoc->GetRangeName();
    }
    else
    {
        pRangeName = pDoc->GetRangeName( mnTab );
    }
    pRangeName->erase(*mpRangeData);
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );

}

void ScUndoAddRangeData::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScRangeName* pRangeName = NULL;
    if (mnTab == -1)
    {
        pRangeName = pDoc->GetRangeName();
    }
    else
    {
        pRangeName = pDoc->GetRangeName( mnTab );
    }
    pRangeName->insert(new ScRangeData(*mpRangeData));
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
}

void ScUndoAddRangeData::Repeat(SfxRepeatTarget& /*rTarget*/)
{
}

bool ScUndoAddRangeData::CanRepeat(SfxRepeatTarget& /*rTarget*/) const
{
    return false;
}

OUString ScUndoAddRangeData::GetComment() const
{
    return ScGlobal::GetRscString(STR_UNDO_RANGENAMES);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
