/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_NAMEMGRTABLE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_NAMEMGRTABLE_HXX

#include <svtools/simptabl.hxx>
#include <vcl/ctrl.hxx>

#include "scresid.hxx"
#include "address.hxx"

#include <memory>
#include <vector>
#include <map>

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
public:
    class InitListener
    {
    public:
        virtual ~InitListener();
        virtual void tableInitialized() = 0;
    };

private:
    OUString maGlobalString;

    // should be const because we should not modify it here
    const std::map<OUString, std::unique_ptr<ScRangeName>>& m_RangeMap;
    // for performance, save which entries already have the formula entry
    // otherwise opening the dialog with a lot of range names is extremelly slow because
    // we would calculate all formula strings during opening
    std::map<SvTreeListEntry*, bool> maCalculatedFormulaEntries;
    const ScAddress maPos;

    InitListener* mpInitListener;

    static void GetLine(ScRangeNameLine& aLine, SvTreeListEntry* pEntry);
    void Init();
    void CheckForFormulaString();
    const ScRangeData* findRangeData(const ScRangeNameLine& rLine);

    void setColWidths();

public:
    ScRangeManagerTable(SvSimpleTableContainer& rParent,
        std::map<OUString, std::unique_ptr<ScRangeName>>& rTabRangeNames,
        const ScAddress& rPos);
    virtual ~ScRangeManagerTable();
    virtual void dispose() override;

    virtual void Resize() override;
    virtual void StateChanged( StateChangedType nStateChange ) override;

    void setInitListener( InitListener* pListener );

    void addEntry( const ScRangeNameLine& rLine, bool bSetCurEntry = true );
    void DeleteSelectedEntries();
    void SetEntry( const ScRangeNameLine& rLine );

    void GetCurrentLine(ScRangeNameLine& rLine);
    bool IsMultiSelection();
    std::vector<ScRangeNameLine> GetSelectedEntries();

    DECL_LINK_TYPED( ScrollHdl, SvTreeListBox*, void);
    DECL_LINK_TYPED( HeaderEndDragHdl, HeaderBar*, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
