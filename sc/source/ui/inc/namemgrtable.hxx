/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *  Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>
#include <vcl/ctrl.hxx>

#include "scresid.hxx"
#include "address.hxx"

#include <vector>
#include <boost/ptr_container/ptr_map.hpp>

class ScRangeName;
class ScRangeData;

struct ScRangeNameLine
{
    rtl::OUString aName;
    rtl::OUString aExpression;
    rtl::OUString aScope;
};

/** Implements the Control behind the table
 *  It controls the size of the table
 */
class ScRangeManagerCtrl : public Control
{
public:
    ScRangeManagerCtrl(Window* pParent, const ScResId& rResId):
        Control( pParent, rResId) {}
};

//Implements the table for the manage names dialog
//TODO: cache the lines for performance improvements
//otherwise handling of a large set of range names might get extremely slow
//Need some sort of a filter to handle several range names
class SC_DLLPUBLIC ScRangeManagerTable : public SvTabListBox
{
private:
    HeaderBar maHeaderBar;
    rtl::OUString maGlobalString;

    // should be const because we should not modify it here
    const boost::ptr_map<rtl::OUString, ScRangeName>& mrRangeMap;
    // for performance, save which entries already have the formula entry
    // otherwise opening the dialog with a lot of range names is extremelly slow because
    // we would calculate all formula strings during opening
    std::map<SvTreeListEntry*, bool> maCalculatedFormulaEntries;
    const ScAddress maPos;

    void GetLine(ScRangeNameLine& aLine, SvTreeListEntry* pEntry);
    void Init();
    void CheckForFormulaString();
    const ScRangeData* findRangeData(const ScRangeNameLine& rLine);

public:
    ScRangeManagerTable( Window* pParent, boost::ptr_map<rtl::OUString, ScRangeName>& aTabRangeNames, const ScAddress& rPos );
    virtual ~ScRangeManagerTable();

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
