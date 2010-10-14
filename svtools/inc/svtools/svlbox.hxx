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

#ifndef _SVLBOX_HXX
#define _SVLBOX_HXX

#include "svtools/svtdllapi.h"

#ifndef INCLUDED_DEQUE
#include <deque>
#define INCLUDED_DEQUE
#endif

#include <vcl/ctrl.hxx>
#include <vcl/seleng.hxx>
#include <vcl/edit.hxx>
#include <vcl/timer.hxx>
#include <vcl/accel.hxx>
#include <vcl/mnemonicengine.hxx>
#include <tools/gen.hxx>
#include <svtools/treelist.hxx>
#include <svl/svarray.hxx>
#include <svtools/transfer.hxx>

class Application;
class SvLBox;
class SvLBoxEntry;
class SvViewDataItem;
class SvViewDataEntry;
class SvInplaceEdit;
class SvInplaceEdit2;
class SvLBoxString;
class SvLBoxButton;
class SvLBoxContextBmp;
class SvLBoxBmp;
struct SvLBoxDDInfo;

namespace utl {
    class AccessibleStateSetHelper;
}

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

class SvLBoxTab
{
    long    nPos;
    void*   pUserData;
public:
            SvLBoxTab();
            SvLBoxTab( long nPos, USHORT nFlags=SV_LBOXTAB_ADJUST_LEFT );
            SvLBoxTab( const SvLBoxTab& );
            ~SvLBoxTab();

    USHORT  nFlags;

    void    SetUserData( void* pPtr ) { pUserData = pPtr; }
    void*   GetUserData() const { return pUserData; }
    BOOL    IsDynamic() const { return (BOOL)((nFlags & SV_LBOXTAB_DYNAMIC)!=0); }
    void    SetPos( long nNewPos) { nPos = nNewPos; }
    long    GetPos() const { return nPos; }
    long    CalcOffset( long nItemLength, long nTabWidth );
    // long CalcOffset( const String&, const OutputDevice& );
    BOOL    IsEditable() const { return (BOOL)((nFlags & SV_LBOXTAB_EDITABLE)!=0); }
    BOOL    IsPushable() const { return (BOOL)((nFlags & SV_LBOXTAB_PUSHABLE)!=0); }
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
// SvLBox::CreateViewData erzeugt. Die ViewDaten-Erzeugung von
// Items kann nicht veraendert werden (wg. Array)
class SvViewDataEntry : public SvViewData
{
public:
    SvViewDataItem* pItemData;  // ein Array von SvViewDataItems
    USHORT          nItmCnt;    // Anzahl Items fuer delete-operator

                    SvViewDataEntry();
    virtual         ~SvViewDataEntry();
};

// *********************************************************************
// ****************************** Items ********************************
// *********************************************************************

class SVT_DLLPUBLIC SvLBoxItem
{
public:
                        SvLBoxItem( SvLBoxEntry*, USHORT nFlags );
                        SvLBoxItem();
    virtual             ~SvLBoxItem();
    virtual USHORT      IsA() = 0;
    const Size&         GetSize( SvLBox* pView, SvLBoxEntry* pEntry );
    const Size&         GetSize( SvLBoxEntry*, SvViewDataEntry* );
    const Size&         GetSize( SvViewDataEntry* pData, USHORT nItemPos )
                        {
                            SvViewDataItem* pIData=pData->pItemData+nItemPos;
                            return pIData->aSize;
                        }

    virtual void        Paint( const Point& rPos, SvLBox& rOutDev,
                            USHORT nViewDataEntryFlags,
                            SvLBoxEntry* pEntry ) = 0;

    virtual void        InitViewData( SvLBox* pView, SvLBoxEntry* pEntry,
                            // wenn != 0 muss dieser Pointer benutzt werden!
                            // wenn == 0 muss er ueber die View geholt werden
                            SvViewDataItem* pViewData = 0) = 0;
    virtual SvLBoxItem* Create() const = 0;
    // view-abhaengige Daten werden nicht geklont
    virtual void        Clone( SvLBoxItem* pSource ) = 0;
};

// Flags, die am Model haengen
#define SV_ENTRYFLAG_CHILDS_ON_DEMAND   0x0001
#define SV_ENTRYFLAG_DISABLE_DROP       0x0002
#define SV_ENTRYFLAG_IN_USE             0x0004
// wird gesetzt, wenn RequestingChilds keine Childs gestzt hat
#define SV_ENTRYFLAG_NO_NODEBMP         0x0008
// Eintrag hatte oder hat Kinder
#define SV_ENTRYFLAG_HAD_CHILDREN       0x0010

#define SV_ENTRYFLAG_USER_FLAGS         0xF000
#define SV_ENTRYFLAG_SEMITRANSPARENT    0x8000      // draw semi-transparent entry bitmaps

class SVT_DLLPUBLIC SvLBoxEntry : public SvListEntry
{
    friend class SvLBox;

    SvPtrarr    aItems;
    void*       pUserData;
    USHORT      nEntryFlags;
//#if 0 // _SOLAR__PRIVATE
    SVT_DLLPRIVATE void         DeleteItems_Impl();
//#endif
public:

                SvLBoxEntry();
    virtual     ~SvLBoxEntry();

    USHORT      ItemCount() const { return (USHORT)aItems.Count(); }
    // DARF NUR GERUFEN WERDEN, WENN DER EINTRAG NOCH NICHT IM MODEL
    // EINGEFUEGT IST, DA SONST FUER DAS ITEM KEINE VIEW-ABHAENGIGEN
    // DATEN ALLOZIERT WERDEN!
    void        AddItem( SvLBoxItem* pItem );
    void        ReplaceItem( SvLBoxItem* pNewItem, USHORT nPos );
    SvLBoxItem* GetItem( USHORT nPos ) const { return (SvLBoxItem*)aItems.GetObject(nPos ); }
    SvLBoxItem* GetFirstItem( USHORT nId );
    USHORT      GetPos( SvLBoxItem* pItem ) const { return aItems.GetPos( pItem ); }
    void*       GetUserData() const { return pUserData; }
    void        SetUserData( void* pPtr ) { pUserData = pPtr; }
    virtual void Clone( SvListEntry* pSource );
    void        EnableChildsOnDemand( BOOL bEnable=TRUE );
    BOOL        HasChildsOnDemand() const { return (BOOL)((nEntryFlags & SV_ENTRYFLAG_CHILDS_ON_DEMAND)!=0); }
    BOOL        HasInUseEmphasis() const    { return (BOOL)((nEntryFlags & SV_ENTRYFLAG_IN_USE)!=0); }

    USHORT      GetFlags() const { return nEntryFlags; }
    void        SetFlags( USHORT nFlags ) { nEntryFlags = nFlags; }
};

// *********************************************************************
// ****************************** SvLBox *******************************
// *********************************************************************

#define WB_FORCE_SELECTION          ((WinBits)0x8000)

#define DragDropMode USHORT
#define SV_DRAGDROP_NONE            (DragDropMode)0x0000
#define SV_DRAGDROP_CTRL_MOVE       (DragDropMode)0x0001
#define SV_DRAGDROP_CTRL_COPY       (DragDropMode)0x0002
#define SV_DRAGDROP_APP_MOVE        (DragDropMode)0x0004
#define SV_DRAGDROP_APP_COPY        (DragDropMode)0x0008
#define SV_DRAGDROP_APP_DROP        (DragDropMode)0x0010
// Entries duerfen ueber den obersten Eintrag gedroppt werden.
// Das Drop-Target ist in diesem Fall 0
#define SV_DRAGDROP_ENABLE_TOP      (DragDropMode)0x0020

DECLARE_SVTREELIST(SvLBoxTreeList, SvLBoxEntry*)

#define SVLISTBOX_ID_LBOX 0   // fuer SvLBox::IsA()

#define SVLBOX_IN_EDT           0x0001
#define SVLBOX_EDT_ENABLED      0x0002
#define SVLBOX_IS_EXPANDING     0x0004
#define SVLBOX_IS_TRAVELSELECT  0x0008
#define SVLBOX_TARGEMPH_VIS     0x0010
#define SVLBOX_EDTEND_CALLED    0x0020

class SvLBox;
struct SvLBox_Impl
{
    bool                    m_bIsEmptyTextAllowed;
    bool                    m_bEntryMnemonicsEnabled;
    Link*                   m_pLink;
    ::vcl::MnemonicEngine   m_aMnemonicEngine;

    SvLBox_Impl( SvLBox& _rBox );
};

class SVT_DLLPUBLIC SvLBox
                :public Control
                ,public SvListView
                ,public DropTargetHelper
                ,public DragSourceHelper
                ,public ::vcl::IMnemonicEntryList
{
    friend class SvLBoxEntry;

//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( TextEditEndedHdl_Impl, SvInplaceEdit2 * );
    // Handler, der von TreeList zum Clonen eines Entries aufgerufen wird
    DECL_DLLPRIVATE_LINK( CloneHdl_Impl, SvListEntry* );

     // handler and methods for Drag - finished handler.
    // The with get GetDragFinishedHdl() get link can set on the
    // TransferDataContainer. This link is a callback for the DragFinished
    // call. AddBox method is called from the GetDragFinishedHdl() and the
    // remove is called in link callback and in the destructor. So it can't
    // called to a deleted object.
    SVT_DLLPRIVATE static void AddBoxToDDList_Impl( const SvLBox& rB );
    SVT_DLLPRIVATE static void RemoveBoxFromDDList_Impl( const SvLBox& rB );
    DECL_DLLPRIVATE_STATIC_LINK( SvLBox, DragFinishHdl_Impl, sal_Int8* );
//#endif

    DragDropMode nOldDragMode;

protected:

    WinBits         nWindowStyle;
    Link            aExpandedHdl;
    Link            aExpandingHdl;
    Link            aSelectHdl;
    Link            aDeselectHdl;
    Link            aDoubleClickHdl;
    SvLBoxEntry*    pHdlEntry;
    SvLBoxItem*     pHdlItem;
    SvLBoxEntry*    pTargetEntry;

    SvLBox_Impl*    pLBoxImpl;

    USHORT          nImpFlags;
    // Move/CopySelection: Position des aktuellen Eintrags in Selektionsliste
    USHORT          nCurEntrySelPos;

    DragDropMode    nDragDropMode;
    SelectionMode   eSelMode;
    sal_Int8        nDragOptions;

    BOOL            CheckDragAndDropMode( SvLBox* pSource, sal_Int8 );
    void            ImplShowTargetEmphasis( SvLBoxEntry* pEntry, BOOL bShow);
    void            EnableSelectionAsDropTarget( BOOL bEnable = TRUE,
                                                 BOOL bWithChilds = TRUE );
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
    virtual void StateChanged( StateChangedType );

    virtual ULONG Insert( SvLBoxEntry* pEnt,SvLBoxEntry* pPar,ULONG nPos=LIST_APPEND);
    virtual ULONG Insert( SvLBoxEntry* pEntry,ULONG nRootPos = LIST_APPEND );
    void            InsertTree( SvLBoxEntry* pTree, SvLBoxEntry* pTarget) {pModel->InsertTree( pTree, pTarget ); }

    // Inplace-Editing
    SvInplaceEdit2*  pEdCtrl;
    void            EnableInplaceEditing( BOOL bEnable){if(bEnable)nImpFlags|=SVLBOX_EDT_ENABLED; else nImpFlags&=~SVLBOX_EDT_ENABLED;}
    BOOL            IsInplaceEditingEnabled() const { return (BOOL)((nImpFlags&SVLBOX_EDT_ENABLED)!=0); }
    virtual void    EditingRequest( SvLBoxEntry* pEntry, SvLBoxItem* pItem,
                        const Point& rMousePos );
    void            EditText( const String&, const Rectangle&,const Selection&);
    void            EditText( const String&, const Rectangle&,const Selection&, BOOL bMulti);
    void            EditTextMultiLine( const String&, const Rectangle&,const Selection&);
    void            CancelTextEditing();
    BOOL            IsEditingActive() const { return (BOOL)((nImpFlags & SVLBOX_IN_EDT)!=0); }
    BOOL            EditingCanceled() const;
    void            EndEditing( BOOL bCancel = FALSE );
    bool            IsEmptyTextAllowed() const;
    void            ForbidEmptyText();
    // Callback EditText
    virtual void    EditedText( const String& );

    // Rueckgabewert muss von SvViewDataEntry abgeleitet sein!
    virtual SvViewData* CreateViewData( SvListEntry* );
    // InitViewData wird direkt nach CreateViewData aufgerufen
    // In InitViewData ist der Entry noch nicht in die View eingefuegt!
    virtual void InitViewData( SvViewData*, SvListEntry* pEntry );
    // ruft fuer Items aller Entries InitViewData auf
    void            RecalcViewData();
    // Callback von RecalcViewData
    virtual void    ViewDataInitialized( SvLBoxEntry* );

    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual SvLBoxEntry* GetEntry( const Point& rPos, BOOL bHit=FALSE ) const;
    virtual void    ModelHasEntryInvalidated( SvListEntry* pEntry );

     // handler and methods for Drag - finished handler. This link can be set
    // to the TransferDataContainer. The AddBox/RemoveBox methods must be
    // called before the StartDrag call.
    // The Remove will be called from the handler, which then called
    // DragFinish method. The Remove also called in the DTOR of the SvLBox -
    // so it can't called to a deleted object.
    Link GetDragFinishedHdl() const;

    // for asynchronous D&D
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt, SvLBox* pSourceView );

     // IMnemonicEntryList
     virtual const void* FirstSearchEntry( String& _rEntryText );
     virtual const void* NextSearchEntry( const void* _pCurrentSearchEntry, String& _rEntryText );
     virtual void        SelectSearchEntry( const void* _pEntry );
     virtual void        ExecuteSearchEntry( const void* _pEntry );

public:

    SvLBox( Window* pParent, WinBits nWinStyle=0 );
    SvLBox( Window* pParent, const ResId& rResId );
    ~SvLBox();

    SvLBoxTreeList* GetModel() const { return (SvLBoxTreeList*)pModel; }
    using SvListView::SetModel;
    void            SetModel( SvLBoxTreeList* );
    void            DisconnectFromModel();

    virtual USHORT  IsA();
    ULONG           GetEntryCount() const {return pModel->GetEntryCount();}
    SvLBoxEntry*    First() const { return (SvLBoxEntry*)(pModel->First()); }
    SvLBoxEntry*    Next( SvLBoxEntry* pEntry, USHORT* pDepth=0 ) const { return (SvLBoxEntry*)(pModel->Next(pEntry,pDepth));}
    SvLBoxEntry*    Prev( SvLBoxEntry* pEntry, USHORT* pDepth=0 ) const { return (SvLBoxEntry*)(pModel->Prev(pEntry,pDepth));}
    SvLBoxEntry*    Last( USHORT* pDepth=0 ) const { return (SvLBoxEntry*)(pModel->Last(pDepth));}

    SvLBoxEntry*    FirstChild(SvLBoxEntry* pParent ) const { return (SvLBoxEntry*)(pModel->FirstChild(pParent)); }
    SvLBoxEntry*    NextSibling(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->NextSibling( pEntry )); }
    SvLBoxEntry*    PrevSibling(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->PrevSibling( pEntry )); }

    SvLBoxEntry*    FirstSelected() const { return (SvLBoxEntry*)SvListView::FirstSelected(); }
    using SvListView::NextSelected;
    SvLBoxEntry*    NextSelected( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(SvListView::NextSelected(pEntry)); }
    using SvListView::PrevSelected;
    SvLBoxEntry*    PrevSelected( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(SvListView::PrevSelected(pEntry)); }
    SvLBoxEntry*    LastSelected() const { return (SvLBoxEntry*)(SvListView::LastSelected()); }

    BOOL            CopySelection( SvLBox* pSource, SvLBoxEntry* pTarget );
    BOOL            MoveSelection( SvLBox* pSource, SvLBoxEntry* pTarget );
    BOOL            MoveSelectionCopyFallbackPossible( SvLBox* pSource, SvLBoxEntry* pTarget, sal_Bool bAllowCopyFallback );
    void            RemoveSelection();

    DragDropMode    GetDragDropMode() const { return nDragDropMode; }
    virtual void    SetDragDropMode( DragDropMode );
    SelectionMode   GetSelectionMode() const { return eSelMode; }
    virtual void    SetSelectionMode( SelectionMode );

    // pParent==0 -> Root-Ebene
    SvLBoxEntry*    GetEntry( SvLBoxEntry* pParent, ULONG nPos ) const { return (SvLBoxEntry*)(pModel->GetEntry(pParent,nPos)); }
    SvLBoxEntry*    GetEntry( ULONG nRootPos ) const { return (SvLBoxEntry*)(pModel->GetEntry(nRootPos)); }

    SvLBoxEntry*    GetEntryFromPath( const ::std::deque< sal_Int32 >& _rPath ) const;
    void            FillEntryPath( SvLBoxEntry* pEntry, ::std::deque< sal_Int32 >& _rPath ) const;
    virtual String  GetEntryText( SvLBoxEntry* pEntry ) const;

    using Window::GetParent;
    SvLBoxEntry*    GetParent( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->GetParent(pEntry)); }
    SvLBoxEntry*    GetRootLevelParent(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->GetRootLevelParent( pEntry ));}

    using Window::GetChildCount;
    ULONG           GetChildCount( SvLBoxEntry* pParent ) const { return pModel->GetChildCount(pParent); }
    ULONG           GetLevelChildCount( SvLBoxEntry* pParent ) const;

    SvViewDataEntry* GetViewDataEntry( SvListEntry* pEntry ) const { return (SvViewDataEntry*)SvListView::GetViewData(pEntry); }
    SvViewDataItem*  GetViewDataItem( SvListEntry*, SvLBoxItem* ) const;

    virtual void    MakeVisible( SvLBoxEntry* pEntry );
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

    virtual BOOL    Expand( SvLBoxEntry* pParent );
    virtual BOOL    Collapse( SvLBoxEntry* pParent );
    virtual BOOL    Select( SvLBoxEntry* pEntry, BOOL bSelect=TRUE );
    virtual ULONG   SelectChilds( SvLBoxEntry* pParent, BOOL bSelect );
    virtual void    SelectAll( BOOL bSelect, BOOL bPaint=TRUE );

    virtual void    SetCurEntry( SvLBoxEntry* _pEntry ) = 0;
    virtual SvLBoxEntry*
                    GetCurEntry() const = 0;

    // Model
    void            SetInUseEmphasis( SvLBoxEntry* pEntry, BOOL bInUse=TRUE);
    // View
    void            SetCursorEmphasis( SvLBoxEntry* pEntry, BOOL bCursored=TRUE);
    BOOL            HasCursorEmphasis( SvLBoxEntry* pEntry ) const;

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
    virtual BOOL    DoubleClickHdl();
    BOOL            IsTravelSelect() const { return (BOOL)((nImpFlags&SVLBOX_IS_TRAVELSELECT)!=0);}
    SvLBoxEntry*    GetHdlEntry() const { return pHdlEntry; }
    SvLBoxItem*     GetHdlItem() const;

    // wird aufgerufen, wenn ein Eintrag mit gesetztem
    // ENTRYFLAG_CHILDS_ON_DEMAND expandiert wird.
    virtual void RequestingChilds( SvLBoxEntry* pParent );

    // Drag & Drop

    //JP 28.3.2001: new Drag & Drop API
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual DragDropMode    NotifyStartDrag( TransferDataContainer& rData,
                                         SvLBoxEntry* );
    virtual void        DragFinished( sal_Int8 nDropAction );
    virtual BOOL        NotifyAcceptDrop( SvLBoxEntry* );

    void            SetDragOptions( sal_Int8 nOptions ) { nDragOptions = nOptions; }
    sal_Int8        GetDragOptions() const { return nDragOptions; }

    SvLBox*         GetSourceView() const;
    SvLBox*         GetTargetView() const;

    virtual void    NotifyRemoving( SvLBoxEntry* );
    virtual void    ShowTargetEmphasis( SvLBoxEntry*, BOOL bShow );
    virtual SvLBoxEntry* CloneEntry( SvLBoxEntry* pSource );
    virtual SvLBoxEntry* CreateEntry() const; // zum 'new'en von Entries

    // Rueckgabe: TRUE==Ok, FALSE==Abbrechen
    virtual BOOL    NotifyMoving(
        SvLBoxEntry*  pTarget,       // D&D-Drop-Position in this->GetModel()
        SvLBoxEntry*  pEntry,        // Zu verschiebender Entry aus
                                     // GetSourceListBox()->GetModel()
        SvLBoxEntry*& rpNewParent,   // Neuer Target-Parent
        ULONG&        rNewChildPos); // Position in Childlist des Target-Parents

    // Rueckgabe: TRUE==Ok, FALSE==Abbrechen
    virtual BOOL    NotifyCopying(
        SvLBoxEntry*  pTarget,       // D&D-Drop-Position in this->GetModel()
        SvLBoxEntry*  pEntry,        // Zu kopierender Entry aus
                                     // GetSourceListBox()->GetModel()
        SvLBoxEntry*& rpNewParent,   // Neuer Target-Parent
        ULONG&        rNewChildPos); // Position in Childlist des Target-Parents

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    /** Fills the StateSet with all states (except DEFUNC, done by the accessible object). */
    virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet ) const;

    /** Calculate and returns the bounding rectangle of an entry.
        @param pEntry
            The entry.
        @return  The bounding rectangle of an entry. */
    virtual Rectangle   GetBoundingRect( SvLBoxEntry* pEntry );

protected:
    using SvListView::Expand;
    using SvListView::Collapse;
    using SvListView::Select;
    using SvListView::SelectChilds;
    using SvListView::SelectAll;
};

#define SV_LBOX_DD_FORMAT "SV_LBOX_DD_FORMAT"
struct SvLBoxDDInfo
{
    Application*    pApp;
    SvLBox*         pSource;
    SvLBoxEntry*    pDDStartEntry;
    // relative Position im Eintrag bei Drag-Beginn (IconView)
    long            nMouseRelX,nMouseRelY;
    ULONG           nRes1,nRes2,nRes3,nRes4;
};

class SvInplaceEdit : public Edit
{
    Link        aCallBackHdl;
    Accelerator aAccReturn;
    Accelerator aAccEscape;
    Timer       aTimer;
    BOOL        bCanceled;
    BOOL        bAlreadyInCallBack;

//#if 0 // _SOLAR__PRIVATE
    void        CallCallBackHdl_Impl();
    DECL_LINK( Timeout_Impl, Timer * );
    DECL_LINK( ReturnHdl_Impl, Accelerator * );
    DECL_LINK( EscapeHdl_Impl, Accelerator * );
//#endif

public:
    SvInplaceEdit( Window* pParent, const Point& rPos, const Size& rSize,
                   const String& rData, const Link& rNotifyEditEnd,
                   const Selection& );
    ~SvInplaceEdit();

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    LoseFocus();
    BOOL            EditingCanceled() const { return bCanceled; }
    String          GetText() const { return Edit::GetText(); }
    void            StopEditing( BOOL bCancel = FALSE );
};

class SvInplaceEdit2
{
    Link        aCallBackHdl;
    Accelerator aAccReturn;
    Accelerator aAccEscape;
    Timer       aTimer;
    Edit*       pEdit;
    BOOL        bCanceled;
    BOOL        bAlreadyInCallBack;
    BOOL        bMultiLine;

//#if 0 // _SOLAR__PRIVATE
    void        CallCallBackHdl_Impl();
    DECL_LINK( Timeout_Impl, Timer * );
    DECL_LINK( ReturnHdl_Impl, Accelerator * );
    DECL_LINK( EscapeHdl_Impl, Accelerator * );
//#endif

public:
                SvInplaceEdit2( Window* pParent, const Point& rPos, const Size& rSize,
                   const String& rData, const Link& rNotifyEditEnd,
                   const Selection&, BOOL bMultiLine = FALSE );
               ~SvInplaceEdit2();
    BOOL        KeyInput( const KeyEvent& rKEvt );
    void        LoseFocus();
    BOOL        EditingCanceled() const { return bCanceled; }
    String      GetText() const;
    String      GetSavedValue() const;
    void        StopEditing( BOOL bCancel = FALSE );
    void        Hide();
};

inline SvViewDataItem* SvLBox::GetViewDataItem( SvListEntry* pEntry,
    SvLBoxItem* pItem) const
{
    SvViewDataEntry* pEntryData =
        (SvViewDataEntry*)SvListView::GetViewData(pEntry);
    DBG_ASSERT(pEntryData,"Entry not in View");
    DBG_ASSERT(pEntryData->pItemData,"No ItemData");
    USHORT nItemPos = ((SvLBoxEntry*)pEntry)->GetPos( pItem );
    return (pEntryData->pItemData+nItemPos);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
