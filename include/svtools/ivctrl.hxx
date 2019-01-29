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

#include <memory>
#include <svtools/svtdllapi.h>
#include <vcl/ctrl.hxx>
#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <o3tl/deleter.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvxIconChoiceCtrl_Impl;

enum class SvxIconViewFlags
{
    NONE           = 0x0000,
    POS_LOCKED     = 0x0001,
    SELECTED       = 0x0002,
    FOCUSED        = 0x0004,
    POS_MOVED      = 0x0008, // Moved by Drag and Drop, but not logged
};
namespace o3tl
{
    template<> struct typed_flags<SvxIconViewFlags> : is_typed_flags<SvxIconViewFlags, 0x000f> {};
}

enum class SvxIconChoiceCtrlTextMode
{
    Full = 1,        //  Enlarge BoundRect southwards
    Short            // Shorten with "..."
};

enum class SvxIconChoiceCtrlPositionMode
{
    Free,                // Free pixel-perfect positioning
    AutoArrange,         // Auto arrange
};

class SvxIconChoiceCtrlEntry
{
    Image const aImage;

    OUString aText;
    OUString aQuickHelpText;
    void*  pUserData;

    friend class SvxIconChoiceCtrl_Impl;
    friend class IcnCursor_Impl;
    friend class EntryList_Impl;
    friend class IcnGridMap_Impl;

    tools::Rectangle               aRect;              // Bounding-Rectangle of the entry
    tools::Rectangle               aGridRect;          // Only valid in Grid-mode
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

    void                    ClearFlags( SvxIconViewFlags nMask ) { nFlags &= ~nMask; }
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
                                this->pflink = nullptr;
                                this->pblink = nullptr;
                            }

public:
                            SvxIconChoiceCtrlEntry( const OUString& rText, const Image& rImage );

    const Image&            GetImage () const { return aImage; }
    void                    SetText ( const OUString& rText ) { aText = rText; }
    const OUString&         GetText () const { return aText; }
    OUString SVT_DLLPUBLIC  GetDisplayText() const;
    void                    SetQuickHelpText( const OUString& rText ) { aQuickHelpText = rText; }
    const OUString&         GetQuickHelpText() const { return aQuickHelpText; }
    void                    SetUserData ( void* _pUserData ) { pUserData = _pUserData; }
    void*                   GetUserData () { return pUserData; }

    SvxIconChoiceCtrlTextMode   GetTextMode() const { return eTextMode; }
    SvxIconViewFlags        GetFlags() const { return nFlags; }
    bool                    IsSelected() const { return bool(nFlags & SvxIconViewFlags::SELECTED); }
    bool                    IsFocused() const { return bool(nFlags & SvxIconViewFlags::FOCUSED); }
    bool                    IsPosLocked() const { return bool(nFlags & SvxIconViewFlags::POS_LOCKED); }
};

class SvxIconChoiceCtrlColumnInfo
{
    long                    nWidth;

public:
                            SvxIconChoiceCtrlColumnInfo() :
                                nWidth( 100 ) {}
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
        WB_NOHIDESELECTION  // Draw selection inactively, if not focused.
        WB_NOHSCROLL
        WB_NOVSCROLL
        WB_NOSELECTION
        WB_SMART_ARRANGE    // Keep Visible-Area at arrange
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
#define WB_SMART_ARRANGE        0x01000000 // used to be WB_PASSWORD
#define WB_ALIGN_TOP            WB_TOP
#define WB_ALIGN_LEFT           WB_LEFT
#define WB_NOCOLUMNHEADER       WB_CENTER
#define WB_HIGHLIGHTFRAME       WB_IGNORETAB
#define WB_NOASYNCSELECTHDL     WB_NOLABEL

class MnemonicGenerator;

class SVT_DLLPUBLIC SvtIconChoiceCtrl : public Control
{
    friend class SvxIconChoiceCtrl_Impl;

    Link<SvtIconChoiceCtrl*,void>  _aClickIconHdl;
    std::unique_ptr<SvxIconChoiceCtrl_Impl, o3tl::default_delete<SvxIconChoiceCtrl_Impl>> _pImpl;

protected:

    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        Resize() override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    void                ClickIcon();
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    static void         DrawEntryImage(
                            SvxIconChoiceCtrlEntry const * pEntry,
                            const Point& rPos,
                            OutputDevice& rDev );

    static OUString     GetEntryText( SvxIconChoiceCtrlEntry const * pEntry );

    virtual void        FillLayoutData() const override;

    void                CallImplEventListeners(VclEventId nEvent, void* pData);

public:

                        SvtIconChoiceCtrl( vcl::Window* pParent, WinBits nWinStyle );
    virtual             ~SvtIconChoiceCtrl() override;
    virtual void        dispose() override;

    void                SetStyle( WinBits nWinStyle );
    WinBits             GetStyle() const;

    void                SetChoiceWithCursor();

    void                SetFont( const vcl::Font& rFont );
    void                SetPointFont( const vcl::Font& rFont );

    void                SetClickHdl( const Link<SvtIconChoiceCtrl*,void>& rLink ) { _aClickIconHdl = rLink; }
    const Link<SvtIconChoiceCtrl*,void>& GetClickHdl() const { return _aClickIconHdl; }

    using OutputDevice::SetBackground;
    void                SetBackground( const Wallpaper& rWallpaper );

    void                ArrangeIcons();


    SvxIconChoiceCtrlEntry* InsertEntry( const OUString& rText,
                                         const Image& rImage);

    /** creates automatic mnemonics for all icon texts in the control

    @param _rUsedMnemonics
        a MnemonicGenerator at which some other mnemonics are already registered.
        This can be used if the control needs to share the "mnemonic space" with other elements,
        such as a menu bar.
    */
    void                CreateAutoMnemonics( MnemonicGenerator& _rUsedMnemonics );

    bool                DoKeyInput( const KeyEvent& rKEvt );

    sal_Int32               GetEntryCount() const;
    SvxIconChoiceCtrlEntry* GetEntry( sal_Int32 nPos ) const;
    sal_Int32               GetEntryListPos( SvxIconChoiceCtrlEntry const * pEntry ) const;
    using Window::SetCursor;
    void                    SetCursor( SvxIconChoiceCtrlEntry* pEntry );
    SvxIconChoiceCtrlEntry* GetCursor() const;

    // Re-calculation of cached view-data and invalidation of those in the view
    void                    InvalidateEntry( SvxIconChoiceCtrlEntry* pEntry );

    // Entry is selected, if the BoundRect is selected
    SvxIconChoiceCtrlEntry* GetEntry( const Point& rPosPixel ) const;

    // sal_uLong is the position of the selected element in the list
    SvxIconChoiceCtrlEntry* GetSelectedEntry() const;

#ifdef DBG_UTIL
    void                    SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry );
#endif

    void                SetSelectionMode( SelectionMode eMode );

    tools::Rectangle           GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const;
    tools::Rectangle           GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const;

    void                SetNoSelection();

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
};

#endif // INCLUDED_SVTOOLS_IVCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
