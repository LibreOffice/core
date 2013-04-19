/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
