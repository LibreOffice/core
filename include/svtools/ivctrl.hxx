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

#ifndef _ICNVW_HXX
#define _ICNVW_HXX

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
#include <vcl/ctrl.hxx>
#include <tools/link.hxx>
#include <tools/contnr.hxx>
#include <vcl/image.hxx>
#include <vcl/seleng.hxx>

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

    OUString aText;
    OUString aQuickHelpText;
    void*  pUserData;

    friend class SvxIconChoiceCtrl_Impl;
    friend class IcnCursor_Impl;
    friend class EntryList_Impl;
    friend class IcnGridMap_Impl;

    Rectangle               aRect;              // Bounding-Rect des Entries
    Rectangle               aGridRect;          // nur gesetzt im Grid-Modus
    sal_uLong                   nPos;

    // die Eintragsposition in der Eintragsliste entspricht der beim Insert vorgegebenen
    // [Sortier-]Reihenfolge (->Reihenfolge der Anker in der Ankerliste!). Im AutoArrange-Modus
    // kann die sichtbare Reihenfolge aber anders sein. Die Eintraege werden deshalb dann
    // verkettet
    SvxIconChoiceCtrlEntry*         pblink;     // backward (linker Nachbar)
    SvxIconChoiceCtrlEntry*         pflink;     // forward  (rechter Nachbar)

    SvxIconChoiceCtrlTextMode       eTextMode;
    sal_uInt16                  nX,nY;      // fuer Tastatursteuerung
    sal_uInt16                  nFlags;

    void                    ClearFlags( sal_uInt16 nMask ) { nFlags &= (~nMask); }
    void                    SetFlags( sal_uInt16 nMask ) { nFlags |= nMask; }
    void                    AssignFlags( sal_uInt16 _nFlags ) { nFlags = _nFlags; }

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
                            SvxIconChoiceCtrlEntry( const OUString& rText, const Image& rImage, sal_uInt16 nFlags = 0 );
                            ~SvxIconChoiceCtrlEntry () {}

    void                    SetImage ( const Image& rImage ) { aImage = rImage; }
    Image                   GetImage () const { return aImage; }
    void                    SetText ( const OUString& rText ) { aText = rText; }
    OUString                GetText () const { return aText; }
    OUString SVT_DLLPUBLIC  GetDisplayText() const;
    void                    SetQuickHelpText( const OUString& rText ) { aQuickHelpText = rText; }
    OUString                GetQuickHelpText() const { return aQuickHelpText; }
    void                    SetUserData ( void* _pUserData ) { pUserData = _pUserData; }
    void*                   GetUserData () { return pUserData; }

    const Rectangle &       GetBoundRect() const { return aRect; }

    void                    SetFocus ( sal_Bool bSet )
                                     { nFlags = ( bSet ? nFlags | ICNVIEW_FLAG_FOCUSED : nFlags & ~ICNVIEW_FLAG_FOCUSED ); }

    SvxIconChoiceCtrlTextMode       GetTextMode() const { return eTextMode; }
    sal_uInt16                  GetFlags() const { return nFlags; }
    sal_Bool                    IsSelected() const { return (sal_Bool)((nFlags & ICNVIEW_FLAG_SELECTED) !=0); }
    sal_Bool                    IsFocused() const { return (sal_Bool)((nFlags & ICNVIEW_FLAG_FOCUSED) !=0); }
    sal_Bool                    IsInUse() const { return (sal_Bool)((nFlags & ICNVIEW_FLAG_IN_USE) !=0); }
    sal_Bool                    IsCursored() const { return (sal_Bool)((nFlags & ICNVIEW_FLAG_CURSORED) !=0); }
    sal_Bool                    IsDropTarget() const { return (sal_Bool)((nFlags & ICNVIEW_FLAG_DROP_TARGET) !=0); }
    sal_Bool                    IsBlockingEmphasis() const { return (sal_Bool)((nFlags & ICNVIEW_FLAG_BLOCK_EMPHASIS) !=0); }
    sal_Bool                    IsPosLocked() const { return (sal_Bool)((nFlags & ICNVIEW_FLAG_POS_LOCKED) !=0); }

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
    OUString                aColText;
    Image                   aColImage;
    long                    nWidth;
    SvxIconChoiceCtrlColumnAlign    eAlignment;
    sal_uInt16                  nSubItem;

public:
                            SvxIconChoiceCtrlColumnInfo( sal_uInt16 nSub, long nWd,
                                SvxIconChoiceCtrlColumnAlign eAlign ) :
                                nWidth( nWd ), eAlignment( eAlign ), nSubItem( nSub ) {}
                            SvxIconChoiceCtrlColumnInfo( const SvxIconChoiceCtrlColumnInfo& );

    void                    SetText( const OUString& rText ) { aColText = rText; }
    void                    SetImage( const Image& rImg ) { aColImage = rImg; }
    void                    SetWidth( long nWd ) { nWidth = nWd; }
    void                    SetAlignment( SvxIconChoiceCtrlColumnAlign eAlign ) { eAlignment = eAlign; }
    void                    SetSubItem( sal_uInt16 nSub) { nSubItem = nSub; }

    const OUString&         GetText() const { return aColText; }
    const Image&            GetImage() const { return aColImage; }
    long                    GetWidth() const { return nWidth; }
    SvxIconChoiceCtrlColumnAlign    GetAlignment() const { return eAlignment; }
    sal_uInt16                  GetSubItem() const { return nSubItem; }
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

class MnemonicGenerator;

class SVT_DLLPUBLIC SvtIconChoiceCtrl : public Control
{
    friend class SvxIconChoiceCtrl_Impl;

    Link                    _aClickIconHdl;
    Link                    _aDocRectChangedHdl;
    Link                    _aVisRectChangedHdl;
    KeyEvent*               _pCurKeyEvent;
    SvxIconChoiceCtrl_Impl* _pImp;
    sal_Bool                    _bAutoFontColor;

protected:

    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual sal_Bool    EditedEntry( SvxIconChoiceCtrlEntry*, const OUString& rNewText, sal_Bool bCancelled );
    virtual void        DocumentRectChanged();
    virtual void        VisibleRectChanged();
    virtual sal_Bool        EditingEntry( SvxIconChoiceCtrlEntry* pEntry );
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

    OUString            GetEntryText(
                            SvxIconChoiceCtrlEntry* pEntry,
                            sal_Bool bInplaceEdit );

    virtual void        FillLayoutData() const;

    void                CallImplEventListeners(sal_uLong nEvent, void* pData);

public:

                        SvtIconChoiceCtrl( Window* pParent, WinBits nWinStyle = WB_ICON | WB_BORDER );
    virtual             ~SvtIconChoiceCtrl();

    void                SetStyle( WinBits nWinStyle );
    WinBits             GetStyle() const;

    sal_Bool                SetChoiceWithCursor ( sal_Bool bDo = sal_True );

    void                SetFont( const Font& rFont );
    void                SetPointFont( const Font& rFont );

    void                SetClickHdl( const Link& rLink ) { _aClickIconHdl = rLink; }
    const Link&         GetClickHdl() const { return _aClickIconHdl; }

    using OutputDevice::SetBackground;
    void                SetBackground( const Wallpaper& rWallpaper );

    void                ArrangeIcons();


    SvxIconChoiceCtrlEntry* InsertEntry( const OUString& rText,
                                         const Image& rImage,
                                         sal_uLong nPos = LIST_APPEND,
                                         const Point* pPos = 0,
                                         sal_uInt16 nFlags = 0
                                       );

    /** creates automatic mnemonics for all icon texts in the control
    */
    void                CreateAutoMnemonics( void );

    /** creates automatic mnemonics for all icon texts in the control

    @param _rUsedMnemonics
        a MnemonicGenerator at which some other mnemonics are already registered.
        This can be used if the control needs to share the "mnemonic space" with other elements,
        such as a menu bar.
    */
    void                CreateAutoMnemonics( MnemonicGenerator& _rUsedMnemonics );

    sal_Bool                DoKeyInput( const KeyEvent& rKEvt );

    sal_Bool                IsEntryEditing() const;

    sal_uLong                   GetEntryCount() const;
    SvxIconChoiceCtrlEntry* GetEntry( sal_uLong nPos ) const;
    sal_uLong                   GetEntryListPos( SvxIconChoiceCtrlEntry* pEntry ) const;
    using Window::SetCursor;
    void                    SetCursor( SvxIconChoiceCtrlEntry* pEntry );
    SvxIconChoiceCtrlEntry* GetCursor() const;

    // Neu-Berechnung gecachter View-Daten und Invalidierung im Fenster
    void                    InvalidateEntry( SvxIconChoiceCtrlEntry* pEntry );

    // bHit==sal_False: Eintrag gilt als getroffen, wenn Position im BoundRect liegt
    //     ==sal_True : Bitmap oder Text muss getroffen sein
    SvxIconChoiceCtrlEntry* GetEntry( const Point& rPosPixel, sal_Bool bHit = sal_False ) const;

    // in dem sal_uLong wird die Position in der Liste des gefunden Eintrags zurueckgegeben
    SvxIconChoiceCtrlEntry* GetSelectedEntry( sal_uLong& rPos ) const;

    void                        SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry = 0 );

    virtual sal_Bool        HasBackground() const;
    virtual sal_Bool        HasFont() const;
    virtual sal_Bool        HasFontTextColor() const;
    virtual sal_Bool        HasFontFillColor() const;

    void                SetFontColorToBackground ( sal_Bool bDo = sal_True ) { _bAutoFontColor = bDo; }
    sal_Bool                AutoFontColor () { return _bAutoFontColor; }

    Point               GetPixelPos( const Point& rPosLogic ) const;
    void                SetSelectionMode( SelectionMode eMode );

    Rectangle           GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const;
    Rectangle           GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const;

    void                SetNoSelection();

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

#endif // _ICNVW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
