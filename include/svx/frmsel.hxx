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

#ifndef INCLUDED_SVX_FRMSEL_HXX
#define INCLUDED_SVX_FRMSEL_HXX

#include <memory>
#include <tools/color.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/bitmap.hxx>
#include <editeng/borderline.hxx>
#include <svx/framebordertype.hxx>
#include <svx/svxdllapi.h>

namespace editeng {
    class SvxBorderLine;
}

namespace svx {

struct FrameSelectorImpl;



typedef int FrameSelFlags;

const FrameSelFlags FRAMESEL_NONE       = 0x0000;
/** If set, the left frame border is enabled. */
const FrameSelFlags FRAMESEL_LEFT       = 0x0001;
/** If set, the right frame border is enabled. */
const FrameSelFlags FRAMESEL_RIGHT      = 0x0002;
/** If set, the top frame border is enabled. */
const FrameSelFlags FRAMESEL_TOP        = 0x0004;
/** If set, the bottom frame border is enabled. */
const FrameSelFlags FRAMESEL_BOTTOM     = 0x0008;
/** If set, the inner horizontal frame border is enabled. */
const FrameSelFlags FRAMESEL_INNER_HOR  = 0x0010;
/** If set, the inner vertical frame border is enabled. */
const FrameSelFlags FRAMESEL_INNER_VER  = 0x0020;
/** If set, the top-left to bottom-right diagonal frame border is enabled. */
const FrameSelFlags FRAMESEL_DIAG_TLBR  = 0x0040;
/** If set, the bottom-left to top-right diagonal frame border is enabled. */
const FrameSelFlags FRAMESEL_DIAG_BLTR  = 0x0080;

/** If set, all four outer frame borders are enabled. */
const FrameSelFlags FRAMESEL_OUTER      = FRAMESEL_LEFT|FRAMESEL_RIGHT|FRAMESEL_TOP|FRAMESEL_BOTTOM;
/** If set, both inner frame borders are enabled. */
const FrameSelFlags FRAMESEL_INNER      = FRAMESEL_INNER_HOR|FRAMESEL_INNER_VER;
/** If set, both diagonal frame borders are enabled. */
const FrameSelFlags FRAMESEL_DIAGONAL   = FRAMESEL_DIAG_TLBR|FRAMESEL_DIAG_BLTR;

/** If set, all frame borders will support the don't care state. */
const FrameSelFlags FRAMESEL_DONTCARE   = 0x0100;



/** All possible states of a frame border. */
enum FrameBorderState
{
    FRAMESTATE_SHOW,        /// Frame border has a visible style.
    FRAMESTATE_HIDE,        /// Frame border is hidden (off).
    FRAMESTATE_DONTCARE     /// Frame border is in don't care state (if enabled).
};



class SVX_DLLPUBLIC SAL_WARN_UNUSED FrameSelector : public Control
{
public:
    FrameSelector(vcl::Window* pParent);
    virtual ~FrameSelector();

    /** Initializes the control, enables/disables frame borders according to flags. */
    void                Initialize( FrameSelFlags nFlags );

    // enabled frame borders --------------------------------------------------

    /** Returns true, if the specified frame border is enabled. */
    bool                IsBorderEnabled( FrameBorderType eBorder ) const;
    /** Returns the number of enabled frame borders. */
    sal_Int32           GetEnabledBorderCount() const;
    /** Returns the border type from the passed index (counts only enabled frame borders). */
    FrameBorderType     GetEnabledBorderType( sal_Int32 nIndex ) const;
    /** Returns the index of a frame border (counts only enabled borders) from passed type. */
    sal_Int32           GetEnabledBorderIndex( FrameBorderType eBorder ) const;

    // frame border state and style -------------------------------------------

    /** Returns true, if the control supports the "don't care" frame border state. */
    bool                SupportsDontCareState() const;

    /** Returns the state (visible/hidden/don't care) of the specified frame border. */
    FrameBorderState    GetFrameBorderState( FrameBorderType eBorder ) const;
    /** Returns the style of the specified frame border, if it is visible. */
    const editeng::SvxBorderLine* GetFrameBorderStyle( FrameBorderType eBorder ) const;

    /** Shows the specified frame border using the passed style, or hides it, if pStyle is 0. */
    void                ShowBorder( FrameBorderType eBorder, const editeng::SvxBorderLine* pStyle );
    /** Sets the specified frame border to "don't care" state. */
    void                SetBorderDontCare( FrameBorderType eBorder );

    /** Returns true, if any enabled frame border has a visible style (not "don't care"). */
    bool                IsAnyBorderVisible() const;
    /** Hides all enabled frame borders. */
    void                HideAllBorders();

    /** Returns true, if all visible frame borders have equal widths.
        @descr  Ignores hidden and "don't care" frame borders. On success,
        returns the width in the passed parameter. */
    bool                GetVisibleWidth( long& rnWidth, editeng::SvxBorderStyle& rnStyle ) const;
    /** Returns true, if all visible frame borders have equal color.
        @descr  Ignores hidden and "don't care" frame borders. On success,
        returns the color in the passed parameter. */
    bool                GetVisibleColor( Color& rColor ) const;

    // frame border selection -------------------------------------------------

    /** Returns the current selection handler. */
    const Link<LinkParamNone*,void>&  GetSelectHdl() const;
    /** Sets the passed handler that is called if the selection of the control changes. */
    void                SetSelectHdl( const Link<LinkParamNone*,void>& rHdl );

    /** Returns true, if the specified frame border is selected. */
    bool                IsBorderSelected( FrameBorderType eBorder ) const;
    /** Selects or deselects the specified frame border. */
    void                SelectBorder( FrameBorderType eBorder, bool bSelect = true );
    /** Returns true, if any of the enabled frame borders is selected. */
    bool                IsAnyBorderSelected() const;
    /** Selects or deselects all frame borders. */
    void                SelectAllBorders( bool bSelect = true );
    /** Deselects all frame borders. */
    inline void         DeselectAllBorders() { SelectAllBorders( false ); }

    /** Selects or deselects all visible frame borders (ignores hidden and "don't care" borders). */
    void                SelectAllVisibleBorders( bool bSelect = true );

    /** Sets the passed line widths to all selected frame borders (in twips). */
    void                SetStyleToSelection( long nWidth, editeng::SvxBorderStyle nStyle );
    /** Sets the passed color to all selected frame borders. */
    void                SetColorToSelection( const Color& rColor );

    // accessibility ----------------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        CreateAccessible() override;

    /** Returns the accessibility child object of the specified frame border (if enabled). */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        GetChildAccessible( FrameBorderType eBorder );
    /** Returns the accessibility child object with specified index (counts enabled frame borders only). */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        GetChildAccessible( sal_Int32 nIndex );
    /** Returns the accessibility child object at the specified position (relative to control). */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        GetChildAccessible( const Point& rPos );

    /** Returns true, if the passed point is inside the click area of any enabled frame border. */
    bool                ContainsClickPoint( const Point& rPos ) const;
    /** Returns the bounding rectangle of the specified frame border (if enabled). */
    Rectangle           GetClickBoundRect( FrameBorderType eBorder ) const;


protected:
    virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void        Resize() override;
    virtual Size        GetOptimalSize() const override;

private:
    std::unique_ptr< FrameSelectorImpl > mxImpl;
};



}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
