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

#ifndef INCLUDED_SVTOOLS_IVCTRL_HXX
#define INCLUDED_SVTOOLS_IVCTRL_HXX

#include <svtools/svtdllapi.h>
#include <vcl/ctrl.hxx>
#include <tools/link.hxx>
#include <tools/contnr.hxx>
#include <vcl/image.hxx>
#include <vcl/seleng.hxx>
#include <o3tl/typed_flags_set.hxx>

class ResId;
class Point;
class SvxIconChoiceCtrl_Impl;
class Image;

enum class SvxIconViewFlags
{
    NONE           = 0x0000,
    POS_LOCKED     = 0x0001,
    SELECTED       = 0x0002,
    FOCUSED        = 0x0004,
    IN_USE         = 0x0008,
    CURSORED       = 0x0010, // Border around image
    POS_MOVED      = 0x0020, // Moved by Drag and Drop, but not logged
    DROP_TARGET    = 0x0040, // Set in QueryDrop
    BLOCK_EMPHASIS = 0x0080, // Do not paint Emphasis
    PRED_SET       = 0x0400, // Predecessor moved
};
namespace o3tl
{
    template<> struct typed_flags<SvxIconViewFlags> : is_typed_flags<SvxIconViewFlags, 0x04ff> {};
}

enum SvxIconChoiceCtrlTextMode
{
    IcnShowTextFull = 1,        //  Enlarge BoundRect southwards
    IcnShowTextShort,           // Shorten with "..."
    IcnShowTextSmart,           // Show all text (not implemented)
    IcnShowTextDontKnow         // Settings of the View
};

enum SvxIconChoiceCtrlPositionMode
{
    IcnViewPositionModeFree = 0,                // Free pixel-perfekt positioning
    IcnViewPositionModeAutoArrange = 1,         // Auto arrange
    IcnViewPositionModeAutoAdjust = 2,          // Auto adjust
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

    Rectangle               aRect;              // Bounding-Rect of the entry
    Rectangle               aGridRect;          // Only valid in Grid-mode
    sal_Int32               nPos;

    /*
        The insert position in the Insertlist is equal to the (sort) order stated at the Insert
        (-> Order of the anchors in the anchors-list!). In "AutoArrange" mode the visible order
        can differ. The entries will be linked because of this.
    */
    SvxIconChoiceCtrlEntry*         pblink;     // backward (linker neighbour)
    SvxIconChoiceCtrlEntry*         pflink;     // forward  (rechter neighbour)

    SvxIconChoiceCtrlTextMode       eTextMode;
    sal_uInt16                      nX,nY;      // for keyboard control
    SvxIconViewFlags                nFlags;

    void                    ClearFlags( SvxIconViewFlags nMask ) { nFlags &= (~nMask); }
    void                    SetFlags( SvxIconViewFlags nMask ) { nFlags |= nMask; }
    void                    AssignFlags( SvxIconViewFlags _nFlags ) { nFlags = _nFlags; }

    // set left neighbour (A <-> B  ==>  A <-> this <-> B)
    void                    SetBacklink( SvxIconChoiceCtrlEntry* pA )
                            {
                                pA->pflink->pblink = this;      // X <- B
                                this->pflink = pA->pflink;      // X -> B
                                this->pblink = pA;              // A <- X
                                pA->pflink = this;              // A -> X
                            }
    // Unlink (A <-> this <-> B  ==>  A <-> B)
    void                    Unlink()
                            {
                                this->pblink->pflink = this->pflink;
                                this->pflink->pblink = this->pblink;
                                this->pflink = 0;
                                this->pblink = 0;
                            }

public:
                            SvxIconChoiceCtrlEntry( const OUString& rText, const Image& rImage, SvxIconViewFlags nFlags = SvxIconViewFlags::NONE );
                            ~SvxIconChoiceCtrlEntry () {}

    Image                   GetImage () const { return aImage; }
    void                    SetText ( const OUString& rText ) { aText = rText; }
    OUString                GetText () const { return aText; }
    OUString SVT_DLLPUBLIC  GetDisplayText() const;
    void                    SetQuickHelpText( const OUString& rText ) { aQuickHelpText = rText; }
    OUString                GetQuickHelpText() const { return aQuickHelpText; }
    void                    SetUserData ( void* _pUserData ) { pUserData = _pUserData; }
    void*                   GetUserData () { return pUserData; }

    SvxIconChoiceCtrlTextMode   GetTextMode() const { return eTextMode; }
    SvxIconViewFlags        GetFlags() const { return nFlags; }
    bool                    IsSelected() const { return bool(nFlags & SvxIconViewFlags::SELECTED); }
    bool                    IsFocused() const { return bool(nFlags & SvxIconViewFlags::FOCUSED); }
    bool                    IsCursored() const { return bool(nFlags & SvxIconViewFlags::CURSORED); }
    bool                    IsDropTarget() const { return bool(nFlags & SvxIconViewFlags::DROP_TARGET); }
    bool                    IsBlockingEmphasis() const { return bool(nFlags & SvxIconViewFlags::BLOCK_EMPHASIS); }
    bool                    IsPosLocked() const { return bool(nFlags & SvxIconViewFlags::POS_LOCKED); }
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

    void                    SetWidth( long nWd ) { nWidth = nWd; }
    long                    GetWidth() const { return nWidth; }
};


/*
    Window-Bits:
        WB_ICON             // Text beneath the icon
        WB_SMALL_ICON       // Text right to the icon, position does not mind
        WB_DETAILS          // Text right to the icon, limited positioning
        WB_BORDER
        WB_NOHIDESELECTION  // Draw selection inaktively, if not focused.
        WB_NOHSCROLL
        WB_NOVSCROLL
        WB_NOSELECTION
        WB_SMART_ARRANGE    // Keep Vis-Area at arrange
        WB_ALIGN_TOP        // Align line vy line LTR
        WB_ALIGN_LEFT       // Align columns from top to bottom
        WB_NODRAGSELECTION  // No selection with tracking rectangle
        WB_NOCOLUMNHEADER   // No Headerbar in Details view (Headerbar not implemented)
        WB_NOPOINTERFOCUS   // No GrabFocus at MouseButtonDown
        WB_HIGHLIGHTFRAME   // The entry beneath the mouse will be highlighted
        WB_NOASYNCSELECTHDL // Do not collect events -> Selection handlers will be called synchronously
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

    Link<SvtIconChoiceCtrl*,void>  _aClickIconHdl;
    KeyEvent*                      _pCurKeyEvent;
    SvxIconChoiceCtrl_Impl*        _pImp;
    bool                           _bAutoFontColor;

protected:

    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        Resize() override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    void                ClickIcon();
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    static void         DrawEntryImage(
                            SvxIconChoiceCtrlEntry* pEntry,
                            const Point& rPos,
                            OutputDevice& rDev );

    static OUString      GetEntryText(
                            SvxIconChoiceCtrlEntry* pEntry,
                            bool bInplaceEdit );

    virtual void        FillLayoutData() const override;

    void                CallImplEventListeners(sal_uLong nEvent, void* pData);

public:

                        SvtIconChoiceCtrl( vcl::Window* pParent, WinBits nWinStyle = WB_ICON | WB_BORDER );
    virtual             ~SvtIconChoiceCtrl();
    virtual void        dispose() override;

    void                SetStyle( WinBits nWinStyle );
    WinBits             GetStyle() const;

    bool                SetChoiceWithCursor ( bool bDo = true );

    void                SetFont( const vcl::Font& rFont );
    void                SetPointFont( const vcl::Font& rFont );

    void                SetClickHdl( const Link<SvtIconChoiceCtrl*,void>& rLink ) { _aClickIconHdl = rLink; }
    const Link<SvtIconChoiceCtrl*,void>& GetClickHdl() const { return _aClickIconHdl; }

    using OutputDevice::SetBackground;
    void                SetBackground( const Wallpaper& rWallpaper );

    void                ArrangeIcons();


    SvxIconChoiceCtrlEntry* InsertEntry( const OUString& rText,
                                         const Image& rImage,
                                         sal_uLong nPos = CONTAINER_APPEND,
                                         const Point* pPos = 0,
                                         SvxIconViewFlags nFlags = SvxIconViewFlags::NONE
                                       );

    /** creates automatic mnemonics for all icon texts in the control

    @param _rUsedMnemonics
        a MnemonicGenerator at which some other mnemonics are already registered.
        This can be used if the control needs to share the "mnemonic space" with other elements,
        such as a menu bar.
    */
    void                CreateAutoMnemonics( MnemonicGenerator& _rUsedMnemonics );

    bool                DoKeyInput( const KeyEvent& rKEvt );

    bool                IsEntryEditing() const;

    sal_Int32               GetEntryCount() const;
    SvxIconChoiceCtrlEntry* GetEntry( sal_Int32 nPos ) const;
    sal_Int32               GetEntryListPos( SvxIconChoiceCtrlEntry* pEntry ) const;
    using Window::SetCursor;
    void                    SetCursor( SvxIconChoiceCtrlEntry* pEntry );
    SvxIconChoiceCtrlEntry* GetCursor() const;

    // Re-calculation of cached view-data and invalidatiopn of those in the view
    void                    InvalidateEntry( SvxIconChoiceCtrlEntry* pEntry );

    // bHit == false: Entry is selectd, if the BoundRect is selected
    //      == true : Bitmap or Text must be selected
    SvxIconChoiceCtrlEntry* GetEntry( const Point& rPosPixel, bool bHit = false ) const;

    // sal_uLong is the position of the selected element in the list
    SvxIconChoiceCtrlEntry* GetSelectedEntry() const;

#ifdef DBG_UTIL
    void                    SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry = 0 );
#endif

    bool                AutoFontColor () { return _bAutoFontColor; }

    Point               GetPixelPos( const Point& rPosLogic ) const;
    void                SetSelectionMode( SelectionMode eMode );

    Rectangle           GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const;
    Rectangle           GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const;

    void                SetNoSelection();

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
};

#endif // INCLUDED_SVTOOLS_IVCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
