/*************************************************************************
 *
 *  $RCSfile: ivctrl.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 15:41:42 $
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

#ifndef _ICNVW_HXX
#define _ICNVW_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SV_SELENG_HXX
#include <vcl/seleng.hxx>
#endif

#pragma hdrstop

class SvPtrarr;
class ResId;
class Point;
class SvxIconChoiceCtrl_Impl;
class Image;

#define ICNVIEW_FLAG_POS_LOCKED     0x0001
#define ICNVIEW_FLAG_SELECTED       0x0002
#define ICNVIEW_FLAG_FOCUSED        0x0004
#define ICNVIEW_FLAG_IN_USE         0x0008
#define ICNVIEW_FLAG_CURSORED       0x0010 // Rahmen um Image
#define ICNVIEW_FLAG_POS_MOVED      0x0020 // per D&D verschoben aber nicht gelockt
#define ICNVIEW_FLAG_DROP_TARGET    0x0040 // im QueryDrop gesetzt
#define ICNVIEW_FLAG_BLOCK_EMPHASIS 0x0080 // Emphasis nicht painten
#define ICNVIEW_FLAG_USER1          0x0100
#define ICNVIEW_FLAG_USER2          0x0200
#define ICNVIEW_FLAG_PRED_SET       0x0400 // Predecessor wurde umgesetzt

enum SvxIconChoiceCtrlTextMode
{
    IcnShowTextFull = 1,        // BoundRect nach unten aufplustern
    IcnShowTextShort,           // Abkuerzung mit "..."
    IcnShowTextSmart,           // Text komplett anzeigen, wenn moeglich (n.i.)
    IcnShowTextDontKnow         // Einstellung der View
};

enum SvxIconChoiceCtrlPositionMode
{
    IcnViewPositionModeFree = 0,                // freies pixelgenaues Positionieren
    IcnViewPositionModeAutoArrange = 1,         // automatisches Ausrichten
    IcnViewPositionModeAutoAdjust = 2,          // automatisches Anordnen
    IcnViewPositionModeLast = IcnViewPositionModeAutoAdjust
};

class SvxIconChoiceCtrlEntry
{
    Image aImage;
    Image aImageHC;

    String aText;
    String aQuickHelpText;
    void*  pUserData;

    friend class SvxIconChoiceCtrl_Impl;
    friend class IcnCursor_Impl;
    friend class EntryList_Impl;
    friend class IcnGridMap_Impl;

    Rectangle               aRect;              // Bounding-Rect des Entries
    Rectangle               aGridRect;          // nur gesetzt im Grid-Modus
    ULONG                   nPos;

    // die Eintragsposition in der Eintragsliste entspricht der beim Insert vorgegebenen
    // [Sortier-]Reihenfolge (->Reihenfolge der Anker in der Ankerliste!). Im AutoArrange-Modus
    // kann die sichtbare Reihenfolge aber anders sein. Die Eintraege werden deshalb dann
    // verkettet
    SvxIconChoiceCtrlEntry*         pblink;     // backward (linker Nachbar)
    SvxIconChoiceCtrlEntry*         pflink;     // forward  (rechter Nachbar)

    SvxIconChoiceCtrlTextMode       eTextMode;
    USHORT                  nX,nY;      // fuer Tastatursteuerung
    USHORT                  nFlags;

    void                    ClearFlags( USHORT nMask ) { nFlags &= (~nMask); }
    void                    SetFlags( USHORT nMask ) { nFlags |= nMask; }
    void                    AssignFlags( USHORT _nFlags ) { nFlags = _nFlags; }

    // setzt den linken Nachbarn (A <-> B  ==>  A <-> this <-> B)
    void                    SetBacklink( SvxIconChoiceCtrlEntry* pA )
                            {
                                pA->pflink->pblink = this;      // X <- B
                                this->pflink = pA->pflink;      // X -> B
                                this->pblink = pA;              // A <- X
                                pA->pflink = this;              // A -> X
                            }
    // loest eine Verbindung (A <-> this <-> B  ==>  A <-> B)
    void                    Unlink()
                            {
                                this->pblink->pflink = this->pflink;
                                this->pflink->pblink = this->pblink;
                                this->pflink = 0;
                                this->pblink = 0;
                            }

public:
                            SvxIconChoiceCtrlEntry( USHORT nFlags = 0 );
                            SvxIconChoiceCtrlEntry( const String& rText, const Image& rImage, USHORT nFlags = 0 );
                            SvxIconChoiceCtrlEntry( const String& rText, const Image& rImage, const Image& rImageHC, USHORT nFlags = 0 );
                            ~SvxIconChoiceCtrlEntry () {}

    void                    SetImage ( const Image& rImage ) { aImage = rImage; }
    void                    SetImageHC ( const Image& rImage ) { aImageHC = rImage; }
    Image                   GetImage () const { return aImage; }
    Image                   GetImageHC () const { return aImageHC; }
    void                    SetText ( const String& rText ) { aText = rText; }
    String                  GetText () const { return aText; }
    String                  GetDisplayText() const;
    void                    SetQuickHelpText( const String& rText ) { aQuickHelpText = rText; }
    String                  GetQuickHelpText() const { return aQuickHelpText; }
    void                    SetUserData ( void* _pUserData ) { pUserData = _pUserData; }
    void*                   GetUserData () { return pUserData; }

    const Rectangle &       GetBoundRect() const { return aRect; }

    void                    SetFocus ( BOOL bSet )
                                     { nFlags = ( bSet ? nFlags | ICNVIEW_FLAG_FOCUSED : nFlags & ~ICNVIEW_FLAG_FOCUSED ); }

    SvxIconChoiceCtrlTextMode       GetTextMode() const { return eTextMode; }
    USHORT                  GetFlags() const { return nFlags; }
    BOOL                    IsSelected() const { return (BOOL)((nFlags & ICNVIEW_FLAG_SELECTED) !=0); }
    BOOL                    IsFocused() const { return (BOOL)((nFlags & ICNVIEW_FLAG_FOCUSED) !=0); }
    BOOL                    IsInUse() const { return (BOOL)((nFlags & ICNVIEW_FLAG_IN_USE) !=0); }
    BOOL                    IsCursored() const { return (BOOL)((nFlags & ICNVIEW_FLAG_CURSORED) !=0); }
    BOOL                    IsDropTarget() const { return (BOOL)((nFlags & ICNVIEW_FLAG_DROP_TARGET) !=0); }
    BOOL                    IsBlockingEmphasis() const { return (BOOL)((nFlags & ICNVIEW_FLAG_BLOCK_EMPHASIS) !=0); }
    BOOL                    WasMoved() const { return (BOOL)((nFlags & ICNVIEW_FLAG_POS_MOVED) !=0); }
    void                    SetMoved( BOOL bMoved );
    BOOL                    IsPosLocked() const { return (BOOL)((nFlags & ICNVIEW_FLAG_POS_LOCKED) !=0); }
    void                    LockPos( BOOL bLock );
    // Nur bei AutoArrange gesetzt. Den Kopf der Liste gibts per SvxIconChoiceCtrl::GetPredecessorHead
    SvxIconChoiceCtrlEntry*         GetSuccessor() const { return pflink; }
    SvxIconChoiceCtrlEntry*         GetPredecessor() const { return pblink; }

//  sal_Unicode             GetMnemonicChar() const;
};

enum SvxIconChoiceCtrlColumnAlign
{
    IcnViewAlignLeft = 1,
    IcnViewAlignRight,
    IcnViewAlignCenter
};

class SvxIconChoiceCtrlColumnInfo
{
    String                  aColText;
    Image                   aColImage;
    long                    nWidth;
    SvxIconChoiceCtrlColumnAlign    eAlignment;
    USHORT                  nSubItem;

public:
                            SvxIconChoiceCtrlColumnInfo( USHORT nSub, long nWd,
                                SvxIconChoiceCtrlColumnAlign eAlign ) : nSubItem( nSub ),
                                nWidth( nWd ), eAlignment( eAlign ) {}
                            SvxIconChoiceCtrlColumnInfo( const SvxIconChoiceCtrlColumnInfo& );

    void                    SetText( const String& rText ) { aColText = rText; }
    void                    SetImage( const Image& rImg ) { aColImage = rImg; }
    void                    SetWidth( long nWd ) { nWidth = nWd; }
    void                    SetAlignment( SvxIconChoiceCtrlColumnAlign eAlign ) { eAlignment = eAlign; }
    void                    SetSubItem( USHORT nSub) { nSubItem = nSub; }

    const String&           GetText() const { return aColText; }
    const Image&            GetImage() const { return aColImage; }
    long                    GetWidth() const { return nWidth; }
    SvxIconChoiceCtrlColumnAlign    GetAlignment() const { return eAlignment; }
    USHORT                  GetSubItem() const { return nSubItem; }
};

//###################################################################################################################################
/*
    Window-Bits:
        WB_ICON             // Text unter dem Icon
        WB_SMALL_ICON       // Text rechts neben Icon, beliebige Positionierung
        WB_DETAILS          // Text rechts neben Icon, eingeschraenkte Posit.
        WB_BORDER
        WB_NOHIDESELECTION  // Selektion inaktiv zeichnen, wenn kein Fokus
        WB_NOHSCROLL
        WB_NOVSCROLL
        WB_NOSELECTION
        WB_SMART_ARRANGE    // im Arrange die Vis-Area beibehalten
        WB_ALIGN_TOP        // Anordnung zeilenweise von links nach rechts
        WB_ALIGN_LEFT       // Anordnung spaltenweise von oben nach unten
        WB_NODRAGSELECTION  // Keine Selektion per Tracking-Rect
        WB_NOCOLUMNHEADER   // keine Headerbar in Detailsview (Headerbar not implemented)
        WB_NOPOINTERFOCUS   // Kein GrabFocus im MouseButtonDown
        WB_HIGHLIGHTFRAME   // der unter der Maus befindliche Eintrag wird hervorgehoben
        WB_NOASYNCSELECTHDL // Selektionshandler synchron aufrufen, d.h. Events nicht sammeln
*/

#ifdef MAC
#undef WB_ICON
#undef WB_SMALLICON
#undef WB_DETAILS
#endif
#define WB_ICON                 WB_RECTSTYLE
#define WB_SMALLICON            WB_SMALLSTYLE
#define WB_DETAILS              WB_VCENTER
#define WB_NOHSCROLL            WB_SPIN
#define WB_NOVSCROLL            WB_DRAG
#define WB_NOSELECTION          WB_REPEAT
#define WB_NODRAGSELECTION      WB_PATHELLIPSIS
#define WB_SMART_ARRANGE        WB_PASSWORD
#define WB_ALIGN_TOP            WB_TOP
#define WB_ALIGN_LEFT           WB_LEFT
#define WB_NOCOLUMNHEADER       WB_CENTER
#define WB_HIGHLIGHTFRAME       WB_INFO
#define WB_NOASYNCSELECTHDL     WB_NOLABEL

class SvtIconChoiceCtrl : public Control
{
    friend class SvxIconChoiceCtrl_Impl;

    Link                    _aClickIconHdl;
    Link                    _aDocRectChangedHdl;
    Link                    _aVisRectChangedHdl;
    KeyEvent*               _pCurKeyEvent;
    SvxIconChoiceCtrl_Impl* _pImp;
    BOOL                    _bAutoFontColor;

protected:

    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual BOOL        EditedEntry( SvxIconChoiceCtrlEntry*, const XubString& rNewText, BOOL bCancelled );
    virtual void        DocumentRectChanged();
    virtual void        VisibleRectChanged();
    virtual BOOL        EditingEntry( SvxIconChoiceCtrlEntry* pEntry );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        Resize();
    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual void        ClickIcon();
    virtual void        StateChanged( StateChangedType nType );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        DrawEntryImage(
                            SvxIconChoiceCtrlEntry* pEntry,
                            const Point& rPos,
                            OutputDevice& rDev );

    virtual String      GetEntryText(
                            SvxIconChoiceCtrlEntry* pEntry,
                            BOOL bInplaceEdit );

    virtual void        FillLayoutData() const;

public:

                        SvtIconChoiceCtrl( Window* pParent, WinBits nWinStyle = WB_ICON | WB_BORDER );
                        SvtIconChoiceCtrl( Window* pParent, const ResId& rResId );
    virtual             ~SvtIconChoiceCtrl();

    void                SetStyle( WinBits nWinStyle );
    WinBits             GetStyle() const;

    BOOL                SetChoiceWithCursor ( BOOL bDo = TRUE );

    void                SetUpdateMode( BOOL bUpdateMode );
    void                SetFont( const Font& rFont );
    void                SetPointFont( const Font& rFont );

    void                SetClickHdl( const Link& rLink ) { _aClickIconHdl = rLink; }
    const Link&         GetClickHdl() const { return _aClickIconHdl; }

    void                SetBackground( const Wallpaper& rWallpaper );

    void                ArrangeIcons();


    SvxIconChoiceCtrlEntry* InsertEntry( ULONG nPos = LIST_APPEND,
                                     const Point* pPos = 0,
                                     USHORT nFlags = 0 );
    SvxIconChoiceCtrlEntry* InsertEntry( const String& rText, const Image& rImage,
                                     ULONG nPos = LIST_APPEND,
                                     const Point* pPos = 0,
                                     USHORT nFlags = 0 );
    SvxIconChoiceCtrlEntry* InsertEntry( const String& rText, const Image& rImage, const Image& rImageHC,
                                     ULONG nPos = LIST_APPEND,
                                     const Point* pPos = 0,
                                     USHORT nFlags = 0 );

    void                CreateAutoMnemonics( void );

    void                RemoveEntry( SvxIconChoiceCtrlEntry* pEntry );

    BOOL                DoKeyInput( const KeyEvent& rKEvt );

    BOOL                IsEntryEditing() const;
    void                Clear();

    ULONG                   GetEntryCount() const;
    SvxIconChoiceCtrlEntry* GetEntry( ULONG nPos ) const;
    ULONG                   GetEntryListPos( SvxIconChoiceCtrlEntry* pEntry ) const;
    void                    SetCursor( SvxIconChoiceCtrlEntry* pEntry );
    SvxIconChoiceCtrlEntry* GetCursor() const;

    // Neu-Berechnung gecachter View-Daten und Invalidierung im Fenster
    void                    InvalidateEntry( SvxIconChoiceCtrlEntry* pEntry );

    // bHit==FALSE: Eintrag gilt als getroffen, wenn Position im BoundRect liegt
    //     ==TRUE : Bitmap oder Text muss getroffen sein
    SvxIconChoiceCtrlEntry* GetEntry( const Point& rPosPixel, BOOL bHit = FALSE ) const;
    // Gibt den naechsten ueber pCurEntry liegenden Eintrag (ZOrder)
    SvxIconChoiceCtrlEntry* GetNextEntry( const Point& rPosPixel, SvxIconChoiceCtrlEntry* pCurEntry, BOOL  ) const;
    // Gibt den naechsten unter pCurEntry liegenden Eintrag (ZOrder)
    SvxIconChoiceCtrlEntry* GetPrevEntry( const Point& rPosPixel, SvxIconChoiceCtrlEntry* pCurEntry, BOOL  ) const;

    // in dem ULONG wird die Position in der Liste des gefunden Eintrags zurueckgegeben
    SvxIconChoiceCtrlEntry* GetSelectedEntry( ULONG& rPos ) const;

    void                        SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry = 0 );
    SvxIconChoiceCtrlTextMode   GetEntryTextMode( const SvxIconChoiceCtrlEntry* pEntry = 0 ) const;

    // offene asynchron abzuarbeitende Aktionen ausfuehren. Muss vor dem Speichern von
    // Eintragspositionen etc. gerufen werden
    void                Flush();


    virtual BOOL        HasBackground() const;
    virtual BOOL        HasFont() const;
    virtual BOOL        HasFontTextColor() const;
    virtual BOOL        HasFontFillColor() const;

    void                SetFontColorToBackground ( BOOL bDo = TRUE ) { _bAutoFontColor = bDo; }
    BOOL                AutoFontColor () { return _bAutoFontColor; }

    Point               GetLogicPos( const Point& rPosPixel ) const;
    Point               GetPixelPos( const Point& rPosLogic ) const;
    void                SetSelectionMode( SelectionMode eMode );

    BOOL                HandleShortCutKey( const KeyEvent& rKeyEvent );

    Rectangle           GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const;
    Rectangle           GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const;

    void                SetNoSelection();

    // ACCESSIBILITY ==========================================================

    void                AddEventListener( const Link& rEventListener );
    void                RemoveEventListener( const Link& rEventListener );

    /** Creates and returns the accessible object of the Box. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

#endif // _ICNVW_HXX

