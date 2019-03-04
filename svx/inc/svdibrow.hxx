/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_INC_SVDIBROW_HXX
#define INCLUDED_SVX_INC_SVDIBROW_HXX

#include <svtools/brwbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/idle.hxx>
#include <memory>

class SfxItemSet;
class ImpItemListRow;
class BrowserMouseEvent;

class SdrItemBrowserControl final : public BrowseBox
{
friend class ImpItemEdit;
    std::vector<std::unique_ptr<ImpItemListRow>> aList;
    long nCurrentPaintRow;
    VclPtr<Edit> pEditControl;
    OUString aWNameMemorized;
    Link<SdrItemBrowserControl&,void> aEntryChangedHdl;
    Link<SdrItemBrowserControl&,void> aSetDirtyHdl;
    std::unique_ptr<ImpItemListRow> pCurrentChangeEntry;
    long   nLastWhichOfs;
    sal_uInt16 nLastWhich;
    bool bWhichesButNames;
    bool bDontHideIneffectiveItems;
    bool bDontSortItems;

    void ImpCtor();
    void ImpSetEntry(const ImpItemListRow& rEntry, std::size_t nEntryNum);
    void ImpSaveWhich();
    void ImpRestoreWhich();
    std::size_t GetCurrentPos() const;
    bool BeginChangeEntry(std::size_t nPos);

    virtual long GetRowCount() const override;
    virtual bool SeekRow(long nRow) override;
    virtual void PaintField(vcl::RenderContext& rDev, const tools::Rectangle& rRect, sal_uInt16 nColumnId) const override;
    virtual void DoubleClick(const BrowserMouseEvent&) override;
    virtual void KeyInput(const KeyEvent& rEvt) override;
    virtual void Select() override;
    void SetDirty(); // is called for example in mode switches
    virtual tools::Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) override;
    virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) override;

public:
    SdrItemBrowserControl(vcl::Window* pParent);
    virtual ~SdrItemBrowserControl() override;
    virtual void dispose() override;
    void Clear();
    void SetAttributes(const SfxItemSet* pAttr, const SfxItemSet* p2ndSet);
    sal_uInt16 GetCurrentWhich() const;
    void EndChangeEntry();
    void BreakChangeEntry();

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long _nRow, sal_uInt16 _nColId) const override;

    const ImpItemListRow* GetCurrentChangeEntry() const { return pCurrentChangeEntry.get(); }
    OUString GetNewEntryValue() const                 { return pEditControl->GetText(); }
    void SetEntryChangedHdl(const Link<SdrItemBrowserControl&,void>& rLink)    { aEntryChangedHdl=rLink; }
    void SetSetDirtyHdl(const Link<SdrItemBrowserControl&,void>& rLink)        { aSetDirtyHdl=rLink; }
};

class SdrView;

class SdrItemBrowser: public FloatingWindow {
    VclPtr<SdrItemBrowserControl> aBrowse;
    Idle aIdle;
    SdrView* pView;
    bool bDirty;
    static vcl::Window* ImpGetViewWin(SdrView const & rView);
    DECL_LINK(IdleHdl, Timer *, void);
    DECL_LINK(ChangedHdl, SdrItemBrowserControl&, void);
    DECL_LINK(SetDirtyHdl, SdrItemBrowserControl&, void);
public:
    SdrItemBrowser(SdrView& rView);
    virtual ~SdrItemBrowser() override;
    void ForceParent();
    void SetDirty();
    void Undirty();
    virtual void dispose() override;
    virtual void Resize() override;
    virtual void GetFocus() override;
    void SetAttributes(const SfxItemSet* pAttr, const SfxItemSet* p2ndSet) { aBrowse->SetAttributes(pAttr,p2ndSet); }
    SdrItemBrowserControl *GetBrowserControl() { return aBrowse.get(); }
};

#endif // INCLUDED_SVX_INC_SVDIBROW_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
