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

#ifndef _SVDIBROW_HXX
#define _SVDIBROW_HXX

#include <svtools/brwbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/floatwin.hxx>

class SfxItemSet;
class ImpItemListRow;
class BrowserMouseEvent;

class _SdrItemBrowserControl: public BrowseBox
{
friend class ImpItemEdit;
    std::vector<ImpItemListRow*> aList;
    long nAktPaintRow;
    Edit* pEditControl;
    OUString aWNamMerk;
    Link aEntryChangedHdl;
    Link aSetDirtyHdl;
    ImpItemListRow* pAktChangeEntry;
    long   nLastWhichOfs;
    sal_uInt16 nLastWhich;
    sal_uInt16 nLastWhichOben;
    sal_uInt16 nLastWhichUnten;
    bool bWhichesButNames;
    bool bDontHideIneffectiveItems;
    bool bDontSortItems;
    bool bShowWhichIds;
    bool bShowRealValues;

private:
    void ImpCtor();
    void ImpSetEntry(const ImpItemListRow& rEntry, sal_uIntPtr nEntryNum);
    ImpItemListRow* ImpGetEntry(sal_uIntPtr nPos) const { return aList[nPos]; }
    void ImpSaveWhich();
    void ImpRestoreWhich();

protected:
    virtual long GetRowCount() const;
    virtual sal_Bool SeekRow(long nRow);
    virtual void PaintField(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId) const;
    virtual void DoubleClick(const BrowserMouseEvent&);
    virtual void KeyInput(const KeyEvent& rEvt);
    virtual void Select();
    virtual void SetDirty(); // is called for example in mode switches
    virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
    virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);
public:
    _SdrItemBrowserControl(Window* pParent, WinBits nBits=WB_3DLOOK|WB_BORDER|WB_TABSTOP);
    virtual ~_SdrItemBrowserControl();
    void Clear();
    void SetAttributes(const SfxItemSet* pAttr, const SfxItemSet* p2ndSet=NULL);
    sal_uIntPtr GetCurrentPos() const;
    sal_uInt16 GetCurrentWhich() const;
    virtual bool BegChangeEntry(sal_uIntPtr nPos);
    virtual bool EndChangeEntry();
    virtual void     BrkChangeEntry();

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long _nRow, sal_uInt16 _nColId) const;

    const ImpItemListRow* GetAktChangeEntry() const { return pAktChangeEntry; }
    OUString GetNewEntryValue() const                 { return pEditControl->GetText(); }
    void SetEntryChangedHdl(const Link& rLink)      { aEntryChangedHdl=rLink; }
    const Link& GetEntryChangedHdl() const          { return aEntryChangedHdl; }
    void SetSetDirtyHdl(const Link& rLink)          { aSetDirtyHdl=rLink; }
    const Link& GetSetDirtyHdl() const              { return aSetDirtyHdl; }
};

#define WB_STDSIZEABLEDOCKWIN  (WB_STDDOCKWIN|WB_3DLOOK|WB_CLOSEABLE|WB_SIZEMOVE)
#define WB_STDSIZEABLEFLOATWIN (WB_STDFLOATWIN|WB_3DLOOK|WB_CLOSEABLE|WB_SIZEMOVE)

class _SdrItemBrowserWindow: public FloatingWindow {
    _SdrItemBrowserControl aBrowse;
public:
    _SdrItemBrowserWindow(Window* pParent, WinBits nBits=WB_STDSIZEABLEDOCKWIN);
    virtual ~_SdrItemBrowserWindow();
    virtual void Resize();
    virtual void GetFocus();
    void Clear()                                            { aBrowse.Clear(); }
    void SetAttributes(const SfxItemSet* pAttr, const SfxItemSet* p2ndSet=NULL) { aBrowse.SetAttributes(pAttr,p2ndSet); }
    void SetFloatingMode(bool /*bOn*/) {}
    const _SdrItemBrowserControl& GetBrowserControl() const { return aBrowse; }
    _SdrItemBrowserControl& GetBrowserControl()             { return aBrowse; }
};

class SdrView;

class SdrItemBrowser: public _SdrItemBrowserWindow {
    Timer aIdleTimer;
    SdrView* pView;
    bool bDirty;
private:
    static Window* ImpGetViewWin(SdrView& rView);
    DECL_LINK(IdleHdl, void *);
    DECL_LINK(ChangedHdl,_SdrItemBrowserControl*);
    DECL_LINK(SetDirtyHdl, void *);
public:
    SdrItemBrowser(SdrView& rView);
    void ForceParent();
    void SetView(SdrView& rView) { pView=&rView; ForceParent(); SetDirty(); }
    void SetDirty();
    void Undirty();
    void ForceUndirty() { if (bDirty) Undirty(); }
};

#endif //_SVDIBROW_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
