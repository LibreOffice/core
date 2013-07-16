/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/simptabl.hxx>
#include <vcl/ctrl.hxx>

#include "scresid.hxx"
#include "address.hxx"

#include <vector>
#include <boost/ptr_container/ptr_map.hpp>

class ScRangeName;
class ScRangeData;

struct ScRangeNameLine
{
    OUString aName;
    OUString aExpression;
    OUString aScope;
};

//Implements the table for the manage names dialog
//TODO: cache the lines for performance improvements
//otherwise handling of a large set of range names might get extremely slow
//Need some sort of a filter to handle several range names
class SC_DLLPUBLIC ScRangeManagerTable : public SvSimpleTable
{
private:
    OUString maGlobalString;

    // should be const because we should not modify it here
    const boost::ptr_map<OUString, ScRangeName>& mrRangeMap;
    // for performance, save which entries already have the formula entry
    // otherwise opening the dialog with a lot of range names is extremelly slow because
    // we would calculate all formula strings during opening
    std::map<SvTreeListEntry*, bool> maCalculatedFormulaEntries;
    const ScAddress maPos;

    void GetLine(ScRangeNameLine& aLine, SvTreeListEntry* pEntry);
    void Init();
    void CheckForFormulaString();
    const ScRangeData* findRangeData(const ScRangeNameLine& rLine);

    void setColWidths();

public:
    ScRangeManagerTable( SvSimpleTableContainer& rParent, boost::ptr_map<OUString, ScRangeName>& aTabRangeNames, const ScAddress& rPos );
    virtual ~ScRangeManagerTable();

    virtual void Resize();

    void addEntry( const ScRangeNameLine& rLine, bool bSetCurEntry = true );
    void DeleteSelectedEntries();
    void SetEntry( const ScRangeNameLine& rLine );

    void GetCurrentLine(ScRangeNameLine& rLine);
    bool IsMultiSelection();
    std::vector<ScRangeNameLine> GetSelectedEntries();

    DECL_LINK( ScrollHdl, void*);
    DECL_LINK( HeaderEndDragHdl, void*);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
