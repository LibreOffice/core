/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Kohei Yoshida <kyoshida@novell.com> (Novell, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

sal_Bool ScUndoAllRangeNames::CanRepeat(SfxRepeatTarget& /*rTarget*/) const
{
    return sal_False;
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

sal_Bool ScUndoAddRangeData::CanRepeat(SfxRepeatTarget& /*rTarget*/) const
{
    return sal_False;
}

OUString ScUndoAddRangeData::GetComment() const
{
    return ScGlobal::GetRscString(STR_UNDO_RANGENAMES);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
