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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
    const ScRangeName* pOldGlobal, const ScRangeName* pNewGlobal,
    const ScRangeName::TabNameCopyMap &rOldLocal, const ScRangeName::TabNameCopyMap &rNewLocal) :
    ScSimpleUndo(pDocSh)
{
    if (pOldGlobal)
        maOldGlobalNames = *pOldGlobal;
    if (pNewGlobal)
        maNewGlobalNames = *pNewGlobal;

    // Copy sheet-local names.
    ScRangeName::TabNameCopyMap::const_iterator itr, itrEnd;
    for (itr = rOldLocal.begin(), itrEnd = rOldLocal.end(); itr != itrEnd; ++itr)
    {
        auto_ptr<ScRangeName> p(new ScRangeName(*itr->second));
        maOldLocalNames.insert(itr->first, p);
    }

    for (itr = rNewLocal.begin(), itrEnd = rNewLocal.end(); itr != itrEnd; ++itr)
    {
        auto_ptr<ScRangeName> p(new ScRangeName(*itr->second));
        maNewLocalNames.insert(itr->first, p);
    }
}

ScUndoAllRangeNames::~ScUndoAllRangeNames()
{
}

void ScUndoAllRangeNames::Undo()
{
    ScDocument& rDoc = *pDocShell->GetDocument();

    rDoc.CompileNameFormula(true);

    // Global names.
    if (maOldGlobalNames.empty())
        rDoc.SetRangeName(NULL);
    else
        rDoc.SetRangeName(new ScRangeName(maOldGlobalNames));

    ScRangeName::TabNameCopyMap aCopy;
    ScRangeName::copyLocalNames(maOldLocalNames, aCopy);
    rDoc.SetAllTabRangeNames(aCopy);

    rDoc.CompileNameFormula(true);

    SFX_APP()->Broadcast(SfxSimpleHint(SC_HINT_AREAS_CHANGED));
}

void ScUndoAllRangeNames::Redo()
{
    ScRangeName::TabNameCopyMap aCopy;
    ScRangeName::copyLocalNames(maNewLocalNames, aCopy);
}

void ScUndoAllRangeNames::Repeat(SfxRepeatTarget& /*rTarget*/)
{
}

BOOL ScUndoAllRangeNames::CanRepeat(SfxRepeatTarget& /*rTarget*/) const
{
    return false;
}

String ScUndoAllRangeNames::GetComment() const
{
    return ScGlobal::GetRscString(STR_UNDO_RANGENAMES);
}

