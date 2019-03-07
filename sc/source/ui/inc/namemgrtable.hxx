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
#include <vcl/weld.hxx>

#include <address.hxx>

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
    OUString const maGlobalString;

    // should be const because we should not modify it here
    const std::map<OUString, std::unique_ptr<ScRangeName>>& m_RangeMap;
    // for performance, save which entries already have the formula entry
    // otherwise opening the dialog with a lot of range names is extremely slow because
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
        const std::map<OUString, std::unique_ptr<ScRangeName>>& rTabRangeNames,
        const ScAddress& rPos);
    virtual ~ScRangeManagerTable() override;
    virtual void dispose() override;

    virtual void Resize() override;
    virtual void StateChanged( StateChangedType nStateChange ) override;

    void setInitListener( InitListener* pListener );

    void addEntry( const ScRangeNameLine& rLine, bool bSetCurEntry );
    void DeleteSelectedEntries();
    void SetEntry( const ScRangeNameLine& rLine );

    void GetCurrentLine(ScRangeNameLine& rLine);
    bool IsMultiSelection();
    std::vector<ScRangeNameLine> GetSelectedEntries();

    DECL_LINK( ScrollHdl, SvTreeListBox*, void);
    DECL_LINK( HeaderEndDragHdl, HeaderBar*, void);
};

class SC_DLLPUBLIC RangeManagerTable
{
private:
    std::unique_ptr<weld::TreeView> m_xTreeView;

    OUString const maGlobalString;

    // should be const because we should not modify it here
    const std::map<OUString, std::unique_ptr<ScRangeName>>& m_RangeMap;
    // for performance, save which entries already have the formula entry
    // otherwise opening the dialog with a lot of range names is extremely slow because
    // we would calculate all formula strings during opening
    std::map<OUString, bool> maCalculatedFormulaEntries;
    const ScAddress maPos;

    int m_nId;

    void GetLine(ScRangeNameLine& aLine, weld::TreeIter& rEntry);
    void Init();
    void CheckForFormulaString();
    const ScRangeData* findRangeData(const ScRangeNameLine& rLine);

    DECL_LINK(SizeAllocHdl, const Size&, void);
    DECL_LINK(VisRowsScrolledHdl, weld::TreeView&, void);

public:
    RangeManagerTable(std::unique_ptr<weld::TreeView>,
        const std::map<OUString, std::unique_ptr<ScRangeName>>& rTabRangeNames,
        const ScAddress& rPos);

    int n_children() const { return m_xTreeView->n_children(); }

    void addEntry(const ScRangeNameLine& rLine);
    std::vector<ScRangeNameLine> GetSelectedEntries();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
