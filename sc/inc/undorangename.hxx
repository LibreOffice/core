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
 *       Kohei Yoshida <kyoshida@novell.com> (Novell, Inc)
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

#ifndef __SC_UNDORANGENAME_HXX__
#define __SC_UNDORANGENAME_HXX__

#include "undobase.hxx"
#include "rangenam.hxx"
#include <boost/ptr_container/ptr_map.hpp>

class ScDocShell;

/**
 * Undo object for named ranges, both in global and sheet-local scopes.
 */
class ScUndoAllRangeNames : public ScSimpleUndo
{
public:
    ScUndoAllRangeNames(ScDocShell* pDocSh,
                        const std::map<OUString, ScRangeName*>& rOldNames,
                        const boost::ptr_map<OUString, ScRangeName>& rNewNames);

    virtual ~ScUndoAllRangeNames();

    virtual void Undo();
    virtual void Redo();
    virtual void Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool CanRepeat(SfxRepeatTarget& rTarget) const;
    virtual OUString GetComment() const;

private:
    void DoChange(const boost::ptr_map<OUString, ScRangeName>& rNames);

private:
    boost::ptr_map<OUString, ScRangeName> maOldNames;
    boost::ptr_map<OUString, ScRangeName> maNewNames;
};

class ScUndoAddRangeData : public ScSimpleUndo
{
public:
    // nTab = -1 for global range names
    ScUndoAddRangeData(ScDocShell* pDocSh, ScRangeData* pRangeData, SCTAB nTab);

    virtual ~ScUndoAddRangeData();

    virtual void Undo();
    virtual void Redo();
    virtual void Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool CanRepeat(SfxRepeatTarget& rTarget) const;
    virtual OUString GetComment() const;

private:
    ScRangeData* mpRangeData;
    SCTAB mnTab;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
