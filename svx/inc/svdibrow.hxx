/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    XubString aWNamMerk;
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
#if _SOLAR__PRIVATE
    void ImpCtor();
    void ImpSetEntry(const ImpItemListRow& rEntry, sal_uIntPtr nEntryNum);
    ImpItemListRow* ImpGetEntry(sal_uIntPtr nPos) const { return aList[nPos]; }
    void ImpSaveWhich();
    void ImpRestoreWhich();
#endif // __PRIVATE
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
    virtual String  GetCellText(long _nRow, sal_uInt16 _nColId) const;

    const ImpItemListRow* GetAktChangeEntry() const { return pAktChangeEntry; }
    XubString GetNewEntryValue() const                 { return pEditControl->GetText(); }
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
