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

#ifndef __SVTOOLS_TREELISTBOX_HXX__
#define __SVTOOLS_TREELISTBOX_HXX__

#include "svtools/svtdllapi.h"

#include <deque>

#include <vcl/ctrl.hxx>
#include <vcl/seleng.hxx>
#include <vcl/edit.hxx>
#include <vcl/timer.hxx>
#include <vcl/accel.hxx>
#include <vcl/mnemonicengine.hxx>
#include <vcl/quickselectionengine.hxx>
#include <vcl/image.hxx>
#include <tools/gen.hxx>
#include <tools/contnr.hxx>
#include <svtools/treelist.hxx>
#include <svtools/transfer.hxx>

class Application;
class SvTreeListBox;
class SvLBoxEntry;
class SvViewDataItem;
class SvViewDataEntry;
class SvInplaceEdit2;
class SvLBoxString;
class SvLBoxButton;
class SvLBoxContextBmp;
class SvLBoxBmp;
class SvImpLBox;
class SvLBoxButtonData;
struct SvLBoxDDInfo;

namespace utl {
    class AccessibleStateSetHelper;
}

enum SvLBoxButtonKind
{
    SvLBoxButtonKind_enabledCheckbox,
    SvLBoxButtonKind_disabledCheckbox,
    SvLBoxButtonKind_staticImage
};

enum SvButtonState { SV_BUTTON_UNCHECKED, SV_BUTTON_CHECKED, SV_BUTTON_TRISTATE };

// *********************************************************************
// *************************** Tabulatoren *****************************
// *********************************************************************

#define SV_LBOXTAB_DYNAMIC          0x0001 // Ausgabespalte des Items verschiebt
                                           // sich entsprechend Child-Tiefe
#define SV_LBOXTAB_ADJUST_RIGHT     0x0002 // rechter Rand des Items am Tabulator
#define SV_LBOXTAB_ADJUST_LEFT      0x0004 // linker Rand ...
#define SV_LBOXTAB_ADJUST_CENTER    0x0008 // Item am Tabulator zentrieren
#define SV_LBOXTAB_ADJUST_NUMERIC   0x0010 // Dezimalpunkt am Tabulator (Strings)

// wird nicht mehr unterstuetzt! Fokus richtet sich jetzt nach Selektion!
#define SV_LBOXTAB_SHOW_FOCUS       0x0020 // Fokus visualisieren

#define SV_LBOXTAB_SHOW_SELECTION   0x0040 // Selektionszustand visualisieren
                                           // Item muss umschliessendes Polygon
                                           // zurueckgeben koennen (D&D-Cursor)
#define SV_LBOXTAB_EDITABLE         0x0100 // Item an Tabulator editierbar
#define SV_LBOXTAB_PUSHABLE         0x0200 // Item verhaelt sich wie ein Button
#define SV_LBOXTAB_INV_ALWAYS       0x0400 // Hintergrund immer loeschen
#define SV_LBOXTAB_FORCE            0x0800 // Default-Berechnung des ersten Tabulators
                                           // (auf die sich Abo-Tabpage, Extras/Optionen/Anpassen,
                                           // etc. verlassen) ausschalten. Die Position des ersten
                                           // Tabs entspricht jetzt exakt den eingestellten Flags &
                                           // Spaltenbreiten.

#define SV_TAB_BORDER 8

#define SV_LISTBOX_ID_TREEBOX 1   // fuer SvLBox::IsA()
#define SV_ENTRYHEIGHTOFFS_PIXEL 2

#define TREEFLAG_CHKBTN         0x0001
#define TREEFLAG_USESEL         0x0002
#define TREEFLAG_MANINS         0x0004
#define TREEFLAG_RECALCTABS     0x0008

typedef sal_Int64   ExtendedWinBits;

// disable the behavior of automatically selecting a "CurEntry" upon painting the control
#define EWB_NO_AUTO_CURENTRY        0x00000001

#define SV_ITEM_ID_LBOXSTRING       1
#define SV_ITEM_ID_LBOXBMP          2
#define SV_ITEM_ID_LBOXBUTTON       3
#define SV_ITEM_ID_LBOXCONTEXTBMP   4

class SvLBoxTab
{
    long    nPos;
    void*   pUserData;
public:
            SvLBoxTab();
            SvLBoxTab( long nPos, sal_uInt16 nFlags=SV_LBOXTAB_ADJUST_LEFT );
            SvLBoxTab( const SvLBoxTab& );
            ~SvLBoxTab();

    sal_uInt16  nFlags;

    void    SetUserData( void* pPtr ) { pUserData = pPtr; }
    void*   GetUserData() const { return pUserData; }
    sal_Bool    IsDynamic() const { return (sal_Bool)((nFlags & SV_LBOXTAB_DYNAMIC)!=0); }
    void    SetPos( long nNewPos) { nPos = nNewPos; }
    long    GetPos() const { return nPos; }
    long    CalcOffset( long nItemLength, long nTabWidth );
    // long CalcOffset( const String&, const OutputDevice& );
    sal_Bool    IsEditable() const { return (sal_Bool)((nFlags & SV_LBOXTAB_EDITABLE)!=0); }
    sal_Bool    IsPushable() const { return (sal_Bool)((nFlags & SV_LBOXTAB_PUSHABLE)!=0); }
};

// *********************************************************************
// *********************** View-abhaengige Daten ***********************
// *********************************************************************

class SvViewDataItem
{
public:
    Size    aSize;
            SvViewDataItem();
            ~SvViewDataItem();
};

// View-abhaengige Daten fuer einen Entry werden in virtueller Fkt.
// SvTreeListBox::CreateViewData erzeugt. Die ViewDaten-Erzeugung von
// Items kann nicht veraendert werden (wg. Array)
class SvViewDataEntry : public SvViewData
{
public:
    SvViewDataItem* pItemData;  // ein Array von SvViewDataItems
    sal_uInt16          nItmCnt;    // Anzahl Items fuer delete-operator

                    SvViewDataEntry();
    virtual         ~SvViewDataEntry();
};

// *********************************************************************
// ****************************** Items ********************************
// *********************************************************************

class SVT_DLLPUBLIC SvLBoxItem
{
public:
                        SvLBoxItem( SvLBoxEntry*, sal_uInt16 nFlags );
                        SvLBoxItem();
    virtual             ~SvLBoxItem();
    virtual sal_uInt16      IsA() = 0;
    const Size&         GetSize( SvTreeListBox* pView, SvLBoxEntry* pEntry );
    const Size&         GetSize( SvViewDataEntry* pData, sal_uInt16 nItemPos )
                        {
                            SvViewDataItem* pIData=pData->pItemData+nItemPos;
                            return pIData->aSize;
                        }

    virtual void        Paint( const Point& rPos, SvTreeListBox& rOutDev,
                            sal_uInt16 nViewDataEntryFlags,
                            SvLBoxEntry* pEntry ) = 0;

    virtual void        InitViewData( SvTreeListBox* pView, SvLBoxEntry* pEntry,
                            // wenn != 0 muss dieser Pointer benutzt werden!
                            // wenn == 0 muss er ueber die View geholt werden
                            SvViewDataItem* pViewData = 0) = 0;
    virtual SvLBoxItem* Create() const = 0;
    // view-abhaengige Daten werden nicht geklont
    virtual void        Clone( SvLBoxItem* pSource ) = 0;
};

class SVT_DLLPUBLIC SvLBoxTreeList : public SvTreeList
{
public:
    SvLBoxEntry* First() const;
    SvLBoxEntry* Next( SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
    SvLBoxEntry* Prev( SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
    SvLBoxEntry* Last() const;
    SvLBoxEntry* Clone( SvListEntry* pEntry, sal_uLong& nCloneCount ) const;
    SvLBoxEntry* GetEntry( SvListEntry* pParent, sal_uLong nPos ) const;
    SvLBoxEntry* GetEntry( sal_uLong nRootPos ) const;
    SvLBoxEntry* GetParent( SvListEntry* pEntry ) const;
    SvLBoxEntry* FirstChild( SvLBoxEntry* pParent ) const;
    SvLBoxEntry* NextSibling( SvLBoxEntry* pEntry ) const;
    SvLBoxEntry* PrevSibling( SvLBoxEntry* pEntry ) const;
    SvLBoxEntry* LastSibling( SvLBoxEntry* pEntry ) const;
    SvLBoxEntry* GetEntryAtAbsPos( sal_uLong nAbsPos ) const;
};

// *********************************************************************
// ****************************** SvTreeListBox *******************************
// *********************************************************************

#define WB_FORCE_SELECTION          ((WinBits)0x8000)

#define DragDropMode sal_uInt16
#define SV_DRAGDROP_NONE            (DragDropMode)0x0000
#define SV_DRAGDROP_CTRL_MOVE       (DragDropMode)0x0001
#define SV_DRAGDROP_CTRL_COPY       (DragDropMode)0x0002
#define SV_DRAGDROP_APP_MOVE        (DragDropMode)0x0004
#define SV_DRAGDROP_APP_COPY        (DragDropMode)0x0008
#define SV_DRAGDROP_APP_DROP        (DragDropMode)0x0010
// Entries duerfen ueber den obersten Eintrag gedroppt werden.
// Das Drop-Target ist in diesem Fall 0
#define SV_DRAGDROP_ENABLE_TOP      (DragDropMode)0x0020

#define SVLISTBOX_ID_LBOX 0   // fuer SvTreeListBox::IsA()

#define SVLBOX_IN_EDT           0x0001
#define SVLBOX_EDT_ENABLED      0x0002
#define SVLBOX_IS_EXPANDING     0x0004
#define SVLBOX_IS_TRAVELSELECT  0x0008
#define SVLBOX_TARGEMPH_VIS     0x0010
#define SVLBOX_EDTEND_CALLED    0x0020

struct SvTreeListBoxImpl;

class SVT_DLLPUBLIC SvTreeListBox
                :public Control
                ,public SvListView
                ,public DropTargetHelper
                ,public DragSourceHelper
                ,public ::vcl::IMnemonicEntryList
                ,public ::vcl::ISearchableStringList
{
    friend class SvImpLBox;
    friend class TreeControlPeer;

    SvTreeListBoxImpl* mpImpl;
    SvImpLBox*      pImp;
    Link            aCheckButtonHdl;
    Link            aScrolledHdl;
    Link            aExpandedHdl;
    Link            aExpandingHdl;
    Link            aSelectHdl;
    Link            aDeselectHdl;

    Accelerator     aInpEditAcc;
    Image           aPrevInsertedExpBmp;
    Image           aPrevInsertedColBmp;
    Image           aCurInsertedExpBmp;
    Image           aCurInsertedColBmp;

    short           nContextBmpWidthMax;
    short           nEntryHeight;
    short           nEntryHeightOffs;
    short           nIndent;
    short           nFocusWidth;
    sal_uInt16      nFirstSelTab;
    sal_uInt16      nLastSelTab;
    sal_uInt16      aContextBmpMode;

    long mnCheckboxItemWidth;

    SvLBoxEntry*    pHdlEntry;
    SvLBoxItem*     pHdlItem;

    DragDropMode    nDragDropMode;
    DragDropMode    nOldDragMode;
    SelectionMode   eSelMode;
    sal_Int8        nDragOptions;

    SvLBoxEntry*        pEdEntry;
    SvLBoxItem*         pEdItem;

protected:
    Link            aDoubleClickHdl;
    SvLBoxEntry*    pTargetEntry;
    SvLBoxButtonData*   pCheckButtonData;
    std::vector<SvLBoxTab*> aTabs;
    sal_uInt16      nTreeFlags;
    sal_uInt16      nImpFlags;
    // Move/CopySelection: Position des aktuellen Eintrags in Selektionsliste
    sal_uInt16      nCurEntrySelPos;

private:
    void SetBaseModel(SvLBoxTreeList* pNewModel);

    DECL_DLLPRIVATE_LINK( CheckButtonClick, SvLBoxButtonData * );
    DECL_DLLPRIVATE_LINK( TextEditEndedHdl_Impl, void * );
    // Handler, der von TreeList zum Clonen eines Entries aufgerufen wird
    DECL_DLLPRIVATE_LINK( CloneHdl_Impl, SvListEntry* );

     // handler and methods for Drag - finished handler.
    // The with get GetDragFinishedHdl() get link can set on the
    // TransferDataContainer. This link is a callback for the DragFinished
    // call. AddBox method is called from the GetDragFinishedHdl() and the
    // remove is called in link callback and in the destructor. So it can't
    // called to a deleted object.
    SVT_DLLPRIVATE static void AddBoxToDDList_Impl( const SvTreeListBox& rB );
    SVT_DLLPRIVATE static void RemoveBoxFromDDList_Impl( const SvTreeListBox& rB );
    DECL_DLLPRIVATE_STATIC_LINK( SvTreeListBox, DragFinishHdl_Impl, sal_Int8* );

protected:

    sal_Bool            CheckDragAndDropMode( SvTreeListBox* pSource, sal_Int8 );
    void            ImplShowTargetEmphasis( SvLBoxEntry* pEntry, sal_Bool bShow);
    void            EnableSelectionAsDropTarget( sal_Bool bEnable = sal_True,
                                                 sal_Bool bWithChildren = sal_True );
    // standard impl gibt 0 zurueck; muss von abgeleiteten Klassen, die
    // D&D unterstuetzen, ueberladen werden
    using Window::GetDropTarget;
    virtual SvLBoxEntry* GetDropTarget( const Point& );

    // view-spezifische Daten in den Dragserver stellen
    // wird an der Source-View aufgerufen (im BeginDrag-Handler)
    virtual void WriteDragServerInfo( const Point&, SvLBoxDDInfo* );
    // wird an der Target-View aufgerufen (im Drop-Handler)
    virtual void ReadDragServerInfo( const Point&,SvLBoxDDInfo* );

    // invalidate children on enable/disable
    virtual void StateChanged( StateChangedType eType );

    virtual sal_uLong Insert( SvLBoxEntry* pEnt,SvLBoxEntry* pPar,sal_uLong nPos=LIST_APPEND);
    virtual sal_uLong Insert( SvLBoxEntry* pEntry,sal_uLong nRootPos = LIST_APPEND );

    // Inplace-Editing
    SvInplaceEdit2*  pEdCtrl;
    void            EditText( const String&, const Rectangle&,const Selection&);
    void            EditText( const String&, const Rectangle&,const Selection&, sal_Bool bMulti);
    void            EditTextMultiLine( const String&, const Rectangle&,const Selection&);
    void            CancelTextEditing();
    sal_Bool            EditingCanceled() const;
    bool            IsEmptyTextAllowed() const;

    // Rueckgabewert muss von SvViewDataEntry abgeleitet sein!
    virtual SvViewData* CreateViewData( SvListEntry* );
    // InitViewData wird direkt nach CreateViewData aufgerufen
    // In InitViewData ist der Entry noch nicht in die View eingefuegt!
    virtual void InitViewData( SvViewData*, SvListEntry* pEntry );
    // ruft fuer Items aller Entries InitViewData auf
    void            RecalcViewData();
    // Callback von RecalcViewData
    virtual void    ViewDataInitialized( SvLBoxEntry* );

     // handler and methods for Drag - finished handler. This link can be set
    // to the TransferDataContainer. The AddBox/RemoveBox methods must be
    // called before the StartDrag call.
    // The Remove will be called from the handler, which then called
    // DragFinish method. The Remove also called in the DTOR of the SvTreeListBox -
    // so it can't called to a deleted object.
    Link GetDragFinishedHdl() const;

    // for asynchronous D&D
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt, SvTreeListBox* pSourceView );

    void            OnCurrentEntryChanged();

    // IMnemonicEntryList
    virtual const void* FirstSearchEntry( String& _rEntryText ) const;
    virtual const void* NextSearchEntry( const void* _pCurrentSearchEntry, String& _rEntryText ) const;
    virtual void        SelectSearchEntry( const void* _pEntry );
    virtual void        ExecuteSearchEntry( const void* _pEntry ) const;

    // ISearchableStringList
    virtual ::vcl::StringEntryIdentifier    CurrentEntry( String& _out_entryText ) const;
    virtual ::vcl::StringEntryIdentifier    NextEntry( ::vcl::StringEntryIdentifier _currentEntry, String& _out_entryText ) const;
    virtual void                            SelectEntry( ::vcl::StringEntryIdentifier _entry );

public:

    SvTreeListBox( Window* pParent, WinBits nWinStyle=0 );
    SvTreeListBox( Window* pParent, const ResId& rResId );
    ~SvTreeListBox();

    SvLBoxTreeList* GetModel() const { return (SvLBoxTreeList*)pModel; }
    using SvListView::SetModel;
    void SetModel(SvLBoxTreeList* pNewModel);

    sal_uInt16 IsA();
    sal_uLong           GetEntryCount() const {return pModel->GetEntryCount();}
    SvLBoxEntry*    First() const { return (SvLBoxEntry*)(pModel->First()); }
    SvLBoxEntry*    Next( SvLBoxEntry* pEntry, sal_uInt16* pDepth=0 ) const { return (SvLBoxEntry*)(pModel->Next(pEntry,pDepth));}
    SvLBoxEntry*    Prev( SvLBoxEntry* pEntry, sal_uInt16* pDepth=0 ) const { return (SvLBoxEntry*)(pModel->Prev(pEntry,pDepth));}
    SvLBoxEntry*    Last() const { return (SvLBoxEntry*)(pModel->Last()); }

    SvLBoxEntry*    FirstChild(SvLBoxEntry* pParent ) const { return (SvLBoxEntry*)(pModel->FirstChild(pParent)); }
    SvLBoxEntry*    NextSibling(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->NextSibling( pEntry )); }
    SvLBoxEntry*    PrevSibling(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->PrevSibling( pEntry )); }

    SvLBoxEntry*    FirstSelected() const { return (SvLBoxEntry*)SvListView::FirstSelected(); }
    using SvListView::NextSelected;
    SvLBoxEntry*    NextSelected( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(SvListView::NextSelected(pEntry)); }
    using SvListView::PrevSelected;
    SvLBoxEntry*    PrevSelected( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(SvListView::PrevSelected(pEntry)); }
    SvLBoxEntry*    LastSelected() const { return (SvLBoxEntry*)(SvListView::LastSelected()); }

    sal_Bool            CopySelection( SvTreeListBox* pSource, SvLBoxEntry* pTarget );
    sal_Bool            MoveSelection( SvTreeListBox* pSource, SvLBoxEntry* pTarget );
    sal_Bool            MoveSelectionCopyFallbackPossible( SvTreeListBox* pSource, SvLBoxEntry* pTarget, sal_Bool bAllowCopyFallback );
    void            RemoveSelection();

    DragDropMode    GetDragDropMode() const { return nDragDropMode; }
    SelectionMode   GetSelectionMode() const { return eSelMode; }

    // pParent==0 -> Root-Ebene
    SvLBoxEntry*    GetEntry( SvLBoxEntry* pParent, sal_uLong nPos ) const { return (SvLBoxEntry*)(pModel->GetEntry(pParent,nPos)); }
    SvLBoxEntry*    GetEntry( sal_uLong nRootPos ) const { return (SvLBoxEntry*)(pModel->GetEntry(nRootPos)); }

    SvLBoxEntry*    GetEntryFromPath( const ::std::deque< sal_Int32 >& _rPath ) const;
    void            FillEntryPath( SvLBoxEntry* pEntry, ::std::deque< sal_Int32 >& _rPath ) const;

    using Window::GetParent;
    SvLBoxEntry*    GetParent( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->GetParent(pEntry)); }
    SvLBoxEntry*    GetRootLevelParent(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->GetRootLevelParent( pEntry ));}

    using Window::GetChildCount;
    sal_uLong           GetChildCount( SvLBoxEntry* pParent ) const { return pModel->GetChildCount(pParent); }
    sal_uLong           GetLevelChildCount( SvLBoxEntry* pParent ) const;

    SvViewDataEntry* GetViewDataEntry( SvListEntry* pEntry ) const { return (SvViewDataEntry*)SvListView::GetViewData(pEntry); }
    SvViewDataItem*  GetViewDataItem( SvListEntry*, SvLBoxItem* ) const;

    bool IsInplaceEditingEnabled() const { return ((nImpFlags & SVLBOX_EDT_ENABLED) != 0); }
    bool IsEditingActive() const { return ((nImpFlags & SVLBOX_IN_EDT) != 0); }
    void EndEditing( bool bCancel = false );
    void ForbidEmptyText();

    void            Clear();

    /** enables or disables mnemonic characters in the entry texts.

        If mnemonics are enabled, then entries are selected and made current when
        there mnemonic character is pressed. If there are multiple entries with the
        same mnemonic, the selection cycles between them.

        Entries with an collapsed ancestor are not included in the calculation of
        mnemonics. That is, if you press the accelerator key of an invisible
        entry, then this entry is *not* selected.

        Be aware that enabling mnemonics is the more expensive the more
        entries you have in your list.
    */
    void            EnableEntryMnemonics( bool _bEnable = true );
    bool            IsEntryMnemonicsEnabled() const;

    /** handles the given key event.

        At the moment, this merely checks for accelerator keys, if entry mnemonics
        are enabled.

        The method might come handy when you want to use keyboard acceleration
        while the control does not have the focus.

        When the key event describes the pressing of a shortcut for an entry,
        then SelectSearchEntry resp. ExecuteSearchEntry are called.

        @see IMnemonicEntryList
        @see MnemonicEngine

        @return
            <TRUE/> if the event has been consumed, <FALSE/> otherwise.
    */
    bool            HandleKeyInput( const KeyEvent& rKEvt );

    void            SetSelectHdl( const Link& rNewHdl ) {aSelectHdl=rNewHdl; }
    void            SetDeselectHdl( const Link& rNewHdl ) {aDeselectHdl=rNewHdl; }
    void            SetDoubleClickHdl(const Link& rNewHdl) {aDoubleClickHdl=rNewHdl;}
    const Link&     GetSelectHdl() const { return aSelectHdl; }
    const Link&     GetDeselectHdl() const { return aDeselectHdl; }
    const Link&     GetDoubleClickHdl() const { return aDoubleClickHdl; }
    void            SetExpandingHdl(const Link& rNewHdl){aExpandingHdl=rNewHdl;}
    void            SetExpandedHdl(const Link& rNewHdl){aExpandedHdl=rNewHdl;}
    const Link&     GetExpandingHdl() const { return aExpandingHdl; }

    virtual void    ExpandedHdl();
    virtual long    ExpandingHdl();
    virtual void    SelectHdl();
    virtual void    DeselectHdl();
    virtual sal_Bool    DoubleClickHdl();
    sal_Bool            IsTravelSelect() const { return (sal_Bool)((nImpFlags&SVLBOX_IS_TRAVELSELECT)!=0);}
    SvLBoxEntry*    GetHdlEntry() const { return pHdlEntry; }
    SvLBoxItem*     GetHdlItem() const;

    // wird aufgerufen, wenn ein Eintrag mit gesetztem
    // ENTRYFLAG_CHILDREN_ON_DEMAND expandiert wird.
    virtual void RequestingChildren( SvLBoxEntry* pParent );

    // Drag & Drop

    //JP 28.3.2001: new Drag & Drop API
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual DragDropMode    NotifyStartDrag( TransferDataContainer& rData,
                                         SvLBoxEntry* );
    virtual void        DragFinished( sal_Int8 nDropAction );
    virtual sal_Bool        NotifyAcceptDrop( SvLBoxEntry* );

    void            SetDragOptions( sal_Int8 nOptions ) { nDragOptions = nOptions; }
    sal_Int8        GetDragOptions() const { return nDragOptions; }

    SvTreeListBox*         GetSourceView() const;

    virtual void    NotifyRemoving( SvLBoxEntry* );
    virtual SvLBoxEntry* CloneEntry( SvLBoxEntry* pSource );
    virtual SvLBoxEntry* CreateEntry() const; // zum 'new'en von Entries

    // Rueckgabe: sal_True==Ok, sal_False==Abbrechen
    virtual sal_Bool    NotifyMoving(
        SvLBoxEntry*  pTarget,       // D&D-Drop-Position in this->GetModel()
        SvLBoxEntry*  pEntry,        // Zu verschiebender Entry aus
                                     // GetSourceListBox()->GetModel()
        SvLBoxEntry*& rpNewParent,   // Neuer Target-Parent
        sal_uLong&        rNewChildPos); // Position in Childlist des Target-Parents

    // Rueckgabe: sal_True==Ok, sal_False==Abbrechen
    virtual sal_Bool    NotifyCopying(
        SvLBoxEntry*  pTarget,       // D&D-Drop-Position in this->GetModel()
        SvLBoxEntry*  pEntry,        // Zu kopierender Entry aus
                                     // GetSourceListBox()->GetModel()
        SvLBoxEntry*& rpNewParent,   // Neuer Target-Parent
        sal_uLong&        rNewChildPos); // Position in Childlist des Target-Parents

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    /** Fills the StateSet with all states (except DEFUNC, done by the accessible object). */
    virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet ) const;

    /** Fills the StateSet of one entry. */
    virtual void FillAccessibleEntryStateSet( SvLBoxEntry* pEntry, ::utl::AccessibleStateSetHelper& rStateSet ) const;

    /** Calculate and returns the bounding rectangle of an entry.
        @param pEntry
            The entry.
        @return  The bounding rectangle of an entry. */
    virtual Rectangle   GetBoundingRect( SvLBoxEntry* pEntry );

    /** Enables, that one cell of a tablistbox entry can be focused */
    void EnableCellFocus();

protected:
    using SvListView::Expand;
    using SvListView::Collapse;
    using SvListView::Select;
    using SvListView::SelectAll;

    SVT_DLLPRIVATE short        GetHeightOffset( const Image& rBmp, Size& rLogicSize);
    SVT_DLLPRIVATE short        GetHeightOffset( const Font& rFont, Size& rLogicSize);

    SVT_DLLPRIVATE void         SetEntryHeight( SvLBoxEntry* pEntry );
    SVT_DLLPRIVATE void         AdjustEntryHeight( const Image& rBmp );
    SVT_DLLPRIVATE void         AdjustEntryHeight( const Font& rFont );

    SVT_DLLPRIVATE void         ImpEntryInserted( SvLBoxEntry* pEntry );
    SVT_DLLPRIVATE long         PaintEntry1( SvLBoxEntry*, long nLine,
                                sal_uInt16 nTabFlagMask=0xffff,
                                sal_Bool bHasClipRegion=sal_False );

    SVT_DLLPRIVATE void         InitTreeView();
    SVT_DLLPRIVATE SvLBoxItem*  GetItem_Impl( SvLBoxEntry*, long nX, SvLBoxTab** ppTab,
                        sal_uInt16 nEmptyWidth );
    SVT_DLLPRIVATE void         ImplInitStyle();

protected:

    void            EditItemText( SvLBoxEntry* pEntry, SvLBoxString* pItem,
                        const Selection& );
    void            EditedText( const XubString& );

    // berechnet abhaengig von TreeList-Style & Bitmap-Groessen
    // alle Tabulatoren neu; wird beim Einfuegen/Austauschen von
    // Bitmaps, beim Wechsel des Models usw. automatisch gerufen
    virtual void    SetTabs();
    void            SetTabs_Impl();
    void            AddTab( long nPos,sal_uInt16 nFlags=SV_LBOXTAB_ADJUST_LEFT,
                        void* pUserData = 0 );
    sal_uInt16      TabCount() const { return aTabs.size(); }
    SvLBoxTab*      GetFirstDynamicTab() const;
    SvLBoxTab*      GetFirstDynamicTab( sal_uInt16& rTabPos ) const;
    SvLBoxTab*      GetFirstTab( sal_uInt16 nFlagMask, sal_uInt16& rTabPos );
    SvLBoxTab*      GetLastTab( sal_uInt16 nFlagMask, sal_uInt16& rTabPos );
    SvLBoxTab*      GetTab( SvLBoxEntry*, SvLBoxItem* ) const;
    void            ClearTabList();

    virtual void InitEntry(SvLBoxEntry*,const XubString&,const Image&,const Image&,SvLBoxButtonKind);

    virtual void    NotifyBeginScroll();
    virtual void    NotifyEndScroll();
    // nLines == 0 => horizontales Scrolling
    virtual void    NotifyScrolling( long nLines );
    virtual void    NotifyScrolled();
    void            SetScrolledHdl( const Link& rLink ) { aScrolledHdl = rLink; }
    const Link&     GetScrolledHdl() const { return aScrolledHdl; }
    long            GetXOffset() const { return GetMapMode().GetOrigin().X(); }

    // wird aufgerufen, _bevor_ Bereiche im Control invalidiert werden,
    // kann zum Hiden von Elementen benutzt werden, die von aussen
    // in das Control hineingezeichnet werden
    virtual void    NotifyInvalidating();

    virtual sal_uLong   GetAscInsertionPos( SvLBoxEntry*, SvLBoxEntry* pParent );
    virtual sal_uLong   GetDescInsertionPos( SvLBoxEntry*, SvLBoxEntry* pParent );
    virtual void    Command( const CommandEvent& rCEvt );

    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    CursorMoved( SvLBoxEntry* pNewCursor );
    virtual void    PreparePaint( SvLBoxEntry* );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            InitSettings(sal_Bool bFont,sal_Bool bForeground,sal_Bool bBackground);
    sal_Bool            IsCellFocusEnabled() const;
    bool            SetCurrentTabPos( sal_uInt16 _nNewPos );
    sal_uInt16          GetCurrentTabPos() const;
    void            CallImplEventListeners(sal_uLong nEvent, void* pData);

    void            ImplEditEntry( SvLBoxEntry* pEntry );

    sal_Bool        AreChildrenTransient() const;
    void            SetChildrenNotTransient();

public:

    void                SetExtendedWinBits( ExtendedWinBits _nBits );

    void            DisconnectFromModel();

    void            EnableCheckButton( SvLBoxButtonData* );
    void            SetCheckButtonData( SvLBoxButtonData* );
    void            SetNodeBitmaps( const Image& rCollapsedNodeBmp, const Image& rExpandedNodeBmp );

    /** returns the default image which clients should use for expanded nodes, to have a consistent user
        interface experience in the whole product.
    */
    static const Image& GetDefaultExpandedNodeImage( );

    /** returns the default image which clients should use for expanded nodes, to have a consistent user
        interface experience in the whole product.
    */
    static const Image& GetDefaultCollapsedNodeImage( );

    /** sets default bitmaps for collapsed and expanded nodes.
    */
    inline  void    SetNodeDefaultImages( )
    {
        SetNodeBitmaps(
            GetDefaultCollapsedNodeImage( ),
            GetDefaultExpandedNodeImage( )
        );
    }

    virtual SvLBoxEntry*    InsertEntry( const XubString& rText, SvLBoxEntry* pParent = 0,
                                         sal_Bool bChildrenOnDemand = sal_False,
                                         sal_uLong nPos=LIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox );

    virtual SvLBoxEntry*    InsertEntry( const XubString& rText,
                                         const Image& rExpandedEntryBmp,
                                         const Image& rCollapsedEntryBmp,
                                         SvLBoxEntry* pParent = 0,
                                         sal_Bool bChildrenOnDemand = sal_False,
                                         sal_uLong nPos = LIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox );

    const Image&    GetDefaultExpandedEntryBmp( ) const;
    const Image&    GetDefaultCollapsedEntryBmp( ) const;

    void            SetDefaultExpandedEntryBmp( const Image& rBmp );
    void            SetDefaultCollapsedEntryBmp( const Image& rBmp );

    void            SetCheckButtonState( SvLBoxEntry*, SvButtonState );
    SvButtonState   GetCheckButtonState( SvLBoxEntry* ) const;

    sal_Bool            IsExpandBitmapOnCursor() const  { return (sal_Bool)(aContextBmpMode & SVLISTENTRYFLAG_FOCUSED)!=0; }

    void            SetEntryText(SvLBoxEntry*, const XubString& );
    void            SetExpandedEntryBmp( SvLBoxEntry* _pEntry, const Image& _rImage );
    void            SetCollapsedEntryBmp( SvLBoxEntry* _pEntry, const Image& _rImage );

    virtual String  GetEntryText( SvLBoxEntry* pEntry ) const;
    String          SearchEntryText( SvLBoxEntry* pEntry ) const;
    const Image&    GetExpandedEntryBmp(SvLBoxEntry* _pEntry ) const;
    const Image&    GetCollapsedEntryBmp(SvLBoxEntry* _pEntry ) const;

    void            SetCheckButtonHdl( const Link& rLink )  { aCheckButtonHdl=rLink; }
    Link            GetCheckButtonHdl() const { return aCheckButtonHdl; }
    virtual void    CheckButtonHdl();

    void            SetSublistOpenWithReturn( sal_Bool bMode = sal_True );      // open/close sublist with return/enter
    void            SetSublistOpenWithLeftRight( sal_Bool bMode = sal_True );   // open/close sublist with cursor left/right

    void            EnableInplaceEditing( bool bEnable );
    // Editiert das erste StringItem des Entries, 0==Cursor
    void            EditEntry( SvLBoxEntry* pEntry = NULL );
    virtual sal_Bool    EditingEntry( SvLBoxEntry* pEntry, Selection& );
    virtual sal_Bool    EditedEntry( SvLBoxEntry* pEntry, const rtl::OUString& rNewText );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    void            SetUpdateMode( sal_Bool );

    virtual void    ModelHasCleared();
    virtual void    ModelHasInserted( SvListEntry* pEntry );
    virtual void    ModelHasInsertedTree( SvListEntry* pEntry );
    virtual void    ModelIsMoving(SvListEntry* pSource,
                        SvListEntry* pTargetParent, sal_uLong nChildPos );
    virtual void    ModelHasMoved(SvListEntry* pSource );
    virtual void    ModelIsRemoving( SvListEntry* pEntry );
    virtual void    ModelHasRemoved( SvListEntry* pEntry );
    void ModelHasEntryInvalidated( SvListEntry* pEntry );

    void            ShowTargetEmphasis( SvLBoxEntry*, sal_Bool bShow );
    void            ScrollOutputArea( short nDeltaEntries );

    short           GetEntryHeight() const  { return nEntryHeight; }
    void            SetEntryHeight( short nHeight, sal_Bool bAlways = sal_False );
    Size            GetOutputSizePixel() const;
    short           GetIndent() const { return nIndent; }
    void            SetIndent( short nIndent );
    void            SetSpaceBetweenEntries( short nSpace );
    short           GetSpaceBetweenEntries() const {return nEntryHeightOffs;}
    Point           GetEntryPosition( SvLBoxEntry* ) const;
    void            ShowEntry( SvLBoxEntry* );  // !!!OBSOLETE, use MakeVisible
    void            MakeVisible( SvLBoxEntry* pEntry );
    void            MakeVisible( SvLBoxEntry* pEntry, sal_Bool bMoveToTop );

    void            SetCollapsedNodeBmp( const Image& );
    void            SetExpandedNodeBmp( const Image& );
    Image           GetExpandedNodeBmp( ) const;

    void            SetFont( const Font& rFont );

    using Window::SetCursor;
    void            SetCursor( SvLBoxEntry* pEntry, sal_Bool bForceNoSelect = sal_False );

    SvLBoxEntry*    GetEntry( const Point& rPos, sal_Bool bHit = sal_False ) const;

    void            PaintEntry( SvLBoxEntry* );
    long            PaintEntry( SvLBoxEntry*, long nLine,
                                sal_uInt16 nTabFlagMask=0xffff );
    virtual Rectangle GetFocusRect( SvLBoxEntry*, long nLine );
    // Beruecksichtigt Einrueckung
    virtual long    GetTabPos( SvLBoxEntry*, SvLBoxTab* );
    void            InvalidateEntry( SvLBoxEntry* );
    SvLBoxItem*     GetItem( SvLBoxEntry*, long nX, SvLBoxTab** ppTab);
    SvLBoxItem*     GetItem( SvLBoxEntry*, long nX );

    void            SetDragDropMode( DragDropMode );
    void            SetSelectionMode( SelectionMode );

    sal_Bool Expand( SvLBoxEntry* pParent );
    sal_Bool Collapse( SvLBoxEntry* pParent );
    sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );
    sal_uLong SelectChildren( SvLBoxEntry* pParent, sal_Bool bSelect );
    virtual void SelectAll( sal_Bool bSelect, sal_Bool bPaint = sal_True );

    void SetCurEntry( SvLBoxEntry* _pEntry );
    SvLBoxEntry* GetCurEntry() const;

    using Window::Invalidate;
    virtual void    Invalidate( sal_uInt16 nFlags = 0);
    virtual void    Invalidate( const Rectangle&, sal_uInt16 nFlags = 0 );

    void            SetHighlightRange(sal_uInt16 nFirstTab=0,sal_uInt16 nLastTab=0xffff);

    virtual Region  GetDragRegion() const;

    // Children des Parents werden Children des naechstoberen Parents
    void            RemoveParentKeepChildren( SvLBoxEntry* pParent );

    DECL_LINK( DefaultCompare, SvSortData* );
    virtual void    ModelNotification( sal_uInt16 nActionId, SvListEntry* pEntry1,
                        SvListEntry* pEntry2, sal_uLong nPos );

    void            EndSelection();
    void            RepaintScrollBars() const;
    ScrollBar*      GetVScroll();
    ScrollBar*      GetHScroll();
    void            EnableAsyncDrag( sal_Bool b );

    SvLBoxEntry*    GetFirstEntryInView() const;
    SvLBoxEntry*    GetNextEntryInView(SvLBoxEntry*) const;
    SvLBoxEntry*    GetLastEntryInView() const;
    void            ScrollToAbsPos( long nPos );

    void            ShowFocusRect( const SvLBoxEntry* pEntry );
    void            InitStartEntry();

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry );

    void            EnableContextMenuHandling( void );
    void            EnableContextMenuHandling( sal_Bool bEnable );
    sal_Bool            IsContextMenuHandlingEnabled( void ) const;

    void            EnableList( bool _bEnable );
};

#define SV_LBOX_DD_FORMAT "SV_LBOX_DD_FORMAT"
struct SvLBoxDDInfo
{
    Application*    pApp;
    SvTreeListBox*         pSource;
    SvLBoxEntry*    pDDStartEntry;
    // relative Position im Eintrag bei Drag-Beginn (IconView)
    long            nMouseRelX,nMouseRelY;
    sal_uLong           nRes1,nRes2,nRes3,nRes4;
};

class SvInplaceEdit2
{
    Link        aCallBackHdl;
    Accelerator aAccReturn;
    Accelerator aAccEscape;
    Timer       aTimer;
    Edit*       pEdit;
    sal_Bool        bCanceled;
    sal_Bool        bAlreadyInCallBack;

    void        CallCallBackHdl_Impl();
    DECL_LINK( Timeout_Impl, void * );
    DECL_LINK( ReturnHdl_Impl, void * );
    DECL_LINK( EscapeHdl_Impl, void * );

public:
                SvInplaceEdit2( Window* pParent, const Point& rPos, const Size& rSize,
                   const String& rData, const Link& rNotifyEditEnd,
                   const Selection&, sal_Bool bMultiLine = sal_False );
               ~SvInplaceEdit2();
    sal_Bool        KeyInput( const KeyEvent& rKEvt );
    void        LoseFocus();
    sal_Bool        EditingCanceled() const { return bCanceled; }
    String      GetText() const;
    String      GetSavedValue() const;
    void        StopEditing( sal_Bool bCancel = sal_False );
    void        Hide();
};

inline SvViewDataItem* SvTreeListBox::GetViewDataItem( SvListEntry* pEntry,
    SvLBoxItem* pItem) const
{
    SvViewDataEntry* pEntryData =
        (SvViewDataEntry*)SvListView::GetViewData(pEntry);
    DBG_ASSERT(pEntryData,"Entry not in View");
    DBG_ASSERT(pEntryData->pItemData,"No ItemData");
    sal_uInt16 nItemPos = ((SvLBoxEntry*)pEntry)->GetPos( pItem );
    return (pEntryData->pItemData+nItemPos);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
