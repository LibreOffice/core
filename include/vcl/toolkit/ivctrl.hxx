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

#pragma once

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <memory>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <o3tl/deleter.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvxIconChoiceCtrl_Impl;

enum class SvxIconViewFlags
{
    NONE           = 0x0000,
    SELECTED       = 0x0001,
    FOCUSED        = 0x0002,
};
namespace o3tl
{
    template<> struct typed_flags<SvxIconViewFlags> : is_typed_flags<SvxIconViewFlags, 0x000f> {};
}

class SvxIconChoiceCtrlEntry
{
    Image aImage;

    OUString aText;
    OUString aQuickHelpText;

    friend class SvxIconChoiceCtrl_Impl;
    friend class IcnCursor_Impl;
    friend class EntryList_Impl;
    friend class IcnGridMap_Impl;

    tools::Rectangle               aRect;              // Bounding-Rectangle of the entry

    sal_uInt16                      nX,nY;      // for keyboard control
    SvxIconViewFlags                nFlags;

    void                    ClearFlags( SvxIconViewFlags nMask ) { nFlags &= ~nMask; }
    void                    SetFlags( SvxIconViewFlags nMask ) { nFlags |= nMask; }
    void                    AssignFlags( SvxIconViewFlags _nFlags ) { nFlags = _nFlags; }

public:
                            SvxIconChoiceCtrlEntry( OUString aText, Image aImage );

    const Image&            GetImage () const { return aImage; }
    void                    SetText ( const OUString& rText ) { aText = rText; }
    const OUString&         GetText () const { return aText; }
    OUString UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC)  GetDisplayText() const;
    void                    SetQuickHelpText( const OUString& rText ) { aQuickHelpText = rText; }
    const OUString&         GetQuickHelpText() const { return aQuickHelpText; }

    SvxIconViewFlags        GetFlags() const { return nFlags; }
    bool                    IsSelected() const { return bool(nFlags & SvxIconViewFlags::SELECTED); }
    bool                    IsFocused() const { return bool(nFlags & SvxIconViewFlags::FOCUSED); }
};


/*
    Window-Bits:
        WB_ICON             // Text beneath the icon
        WB_SMALLICON        // Text right to the icon, position does not mind
        WB_DETAILS          // Text right to the icon, limited positioning
        WB_BORDER
        WB_NOHIDESELECTION  // Draw selection inactively, if not focused.
        WB_NOHSCROLL
        WB_NOVSCROLL
        WB_NODRAGSELECTION  // No selection with tracking rectangle
        WB_NOCOLUMNHEADER   // No Headerbar in Details view (Headerbar not implemented)
        WB_NOPOINTERFOCUS   // No GrabFocus at MouseButtonDown
*/

#define WB_ICON                 WB_RECTSTYLE
#define WB_SMALLICON            WB_SMALLSTYLE
#define WB_DETAILS              WB_VCENTER
#define WB_NOHSCROLL            WB_SPIN
#define WB_NOVSCROLL            WB_DRAG
#define WB_NODRAGSELECTION      WB_PATHELLIPSIS
#define WB_NOCOLUMNHEADER       WB_CENTER

class MnemonicGenerator;

class UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC) SvtIconChoiceCtrl final : public Control
{
    friend class SvxIconChoiceCtrl_Impl;

    Link<SvtIconChoiceCtrl*,void>  _aClickIconHdl;
    std::unique_ptr<SvxIconChoiceCtrl_Impl, o3tl::default_delete<SvxIconChoiceCtrl_Impl>> _pImpl;

    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        Resize() override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    void                ClickIcon();
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;

    virtual void        FillLayoutData() const override;

    void                CallImplEventListeners(VclEventId nEvent, void* pData);
    long                m_nWidth;

public:

                        SvtIconChoiceCtrl( vcl::Window* pParent, WinBits nWinStyle );
    virtual             ~SvtIconChoiceCtrl() override;
    virtual void        dispose() override;

    void                SetFont( const vcl::Font& rFont );
    void                SetPointFont( const vcl::Font& rFont );

    void                SetClickHdl( const Link<SvtIconChoiceCtrl*,void>& rLink ) { _aClickIconHdl = rLink; }

    using Window::SetBackground;
    void                SetBackground( const Wallpaper& rWallpaper );

    void                ArrangeIcons();
    long                AdjustWidth(const long nWidth); // returns the effective width


    SvxIconChoiceCtrlEntry* InsertEntry( const OUString& rText,
                                         const Image& rImage);

    void RemoveEntry(sal_Int32 nEntryListPos);

    bool                DoKeyInput( const KeyEvent& rKEvt );

    sal_Int32               GetEntryCount() const;
    SvxIconChoiceCtrlEntry* GetEntry( sal_Int32 nPos ) const;
    sal_Int32               GetEntryListPos( SvxIconChoiceCtrlEntry const * pEntry ) const;
    using Window::SetCursor;
    void                    SetCursor( SvxIconChoiceCtrlEntry* pEntry );
    SvxIconChoiceCtrlEntry* GetCursor() const;

    SvxIconChoiceCtrlEntry* GetSelectedEntry() const;

    tools::Rectangle           GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const;
    tools::Rectangle           GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const;

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual rtl::Reference<comphelper::OAccessible> CreateAccessible() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
