/*************************************************************************
 *
 *  $RCSfile: svdibrow.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVDIBROW_HXX
#define _SVDIBROW_HXX

#ifndef _SVX_BRWBOX_HXX //autogen
#include <svtools/brwbox.hxx>
#endif

#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif

class SfxItemSet;
class ImpItemListRow;
class BrowserMouseEvent;

class _SdrItemBrowserControl: public BrowseBox
{
friend class ImpItemEdit;
    Container aList;
    long nAktPaintRow;
    Edit* pEditControl;
    XubString aWNamMerk;
    Link aEntryChangedHdl;
    Link aSetDirtyHdl;
    ImpItemListRow* pAktChangeEntry;
    long   nLastWhichOfs;
    USHORT nLastWhich;
    USHORT nLastWhichOben;
    USHORT nLastWhichUnten;
    FASTBOOL bWhichesButNames;
    FASTBOOL bDontHideIneffectiveItems;
    FASTBOOL bDontSortItems;
    FASTBOOL bShowWhichIds;
    FASTBOOL bShowRealValues;
private:
#if __PRIVATE
    void ImpCtor();
    void ImpSetEntry(const ImpItemListRow& rEntry, ULONG nEntryNum);
    ImpItemListRow* ImpGetEntry(ULONG nPos) const { return (ImpItemListRow*)aList.GetObject(nPos); }
    void ImpSaveWhich();
    void ImpRestoreWhich();
#endif // __PRIVATE
protected:
    virtual long GetRowCount() const;
    virtual BOOL SeekRow(long nRow);
    virtual void PaintField(OutputDevice& rDev, const Rectangle& rRect, USHORT nColumnId) const;
#if SUPD >= 345
    virtual void DoubleClick(const BrowserMouseEvent&);
#else
    virtual void DoubleClick();
#endif
    virtual void KeyInput(const KeyEvent& rEvt);
    virtual void Select();
    virtual void SetDirty(); // wird z.B. bei Modusumschaltungen gerufen
public:
    _SdrItemBrowserControl(Window* pParent, WinBits nBits=WB_3DLOOK|WB_BORDER|WB_TABSTOP);
    virtual ~_SdrItemBrowserControl();
    void Clear();
    void SetAttributes(const SfxItemSet* pAttr, const SfxItemSet* p2ndSet=NULL);
    ULONG GetCurrentPos() const;
    USHORT GetCurrentWhich() const;
    virtual FASTBOOL BegChangeEntry(ULONG nPos);
    virtual FASTBOOL EndChangeEntry();
    virtual void     BrkChangeEntry();
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
    void SetFloatingMode(FASTBOOL bOn) {}
    const _SdrItemBrowserControl& GetBrowserControl() const { return aBrowse; }
    _SdrItemBrowserControl& GetBrowserControl()             { return aBrowse; }
};

class SdrView;

class SdrItemBrowser: public _SdrItemBrowserWindow {
    Timer aIdleTimer;
    SdrView* pView;
    FASTBOOL bDirty;
private:
    static Window* ImpGetViewWin(SdrView& rView);
    DECL_LINK(IdleHdl,Timer*);
    DECL_LINK(ChangedHdl,_SdrItemBrowserControl*);
    DECL_LINK(SetDirtyHdl,_SdrItemBrowserControl*);
public:
    SdrItemBrowser(SdrView& rView);
    void ForceParent();
    void SetView(SdrView& rView) { pView=&rView; ForceParent(); SetDirty(); }
    void SetDirty();
    void Undirty();
    void ForceUndirty() { if (bDirty) Undirty(); }
};

#endif //_SVDIBROW_HXX


