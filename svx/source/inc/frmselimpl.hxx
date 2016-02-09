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

#ifndef INCLUDED_SVX_SOURCE_INC_FRMSELIMPL_HXX
#define INCLUDED_SVX_SOURCE_INC_FRMSELIMPL_HXX

#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <svx/frmsel.hxx>
#include <svx/framelinkarray.hxx>
#include <editeng/borderline.hxx>

namespace svx {

namespace a11y {
    class AccFrameSelector;
}

class FrameBorder
{
public:
    explicit FrameBorder(FrameBorderType eType);

    inline FrameBorderType GetType() const
    {
        return meType;
    }

    inline bool IsEnabled() const
    {
        return mbEnabled;
    }
    void Enable(FrameSelFlags nFlags);

    inline FrameBorderState GetState() const
    {
        return meState;
    }
    void SetState(FrameBorderState eState);

    inline bool IsSelected() const { return mbSelected; }
    inline void Select( bool bSelect ) { mbSelected = bSelect; }

    const editeng::SvxBorderLine& GetCoreStyle() const { return maCoreStyle; }
    void SetCoreStyle( const editeng::SvxBorderLine* pStyle );

    inline void SetUIColorPrim( const Color& rColor ) {maUIStyle.SetColorPrim( rColor ); }
    inline void SetUIColorSecn( const Color& rColor ) {maUIStyle.SetColorSecn( rColor ); }
    inline const frame::Style& GetUIStyle() const { return maUIStyle; }

    inline void ClearFocusArea() { maFocusArea.Clear(); }
    void AddFocusPolygon( const tools::Polygon& rFocus );
    void MergeFocusToPolyPolygon( tools::PolyPolygon& rPPoly ) const;

    inline void ClearClickArea() { maClickArea.Clear(); }
    void AddClickRect( const Rectangle& rRect );
    bool ContainsClickPoint( const Point& rPos ) const;
    Rectangle GetClickBoundRect() const;

    void SetKeyboardNeighbors(FrameBorderType eLeft, FrameBorderType eRight,
                              FrameBorderType eTop, FrameBorderType eBottom);
    FrameBorderType GetKeyboardNeighbor( sal_uInt16 nKeyCode ) const;

private:
    const FrameBorderType meType; /// Frame border type (position in control).
    FrameBorderState meState; /// Frame border state (on/off/don't care).
    editeng::SvxBorderLine maCoreStyle; /// Core style from application.
    frame::Style maUIStyle; /// Internal style to draw lines.
    FrameBorderType meKeyLeft; /// Left neighbor for keyboard control.
    FrameBorderType meKeyRight; /// Right neighbor for keyboard control.
    FrameBorderType meKeyTop; /// Upper neighbor for keyboard control.
    FrameBorderType meKeyBottom;    /// Lower neighbor for keyboard control.
    tools::PolyPolygon maFocusArea;    /// Focus drawing areas.
    tools::PolyPolygon maClickArea;    /// Mouse click areas.
    bool mbEnabled : 1; /// true = Border enabled in control.
    bool mbSelected : 1; /// true = Border selected in control.
};


typedef std::vector< FrameBorder* > FrameBorderPtrVec;

struct FrameSelectorImpl : public Resource
{
    FrameSelector&      mrFrameSel;     /// The control itself.
    ScopedVclPtr<VirtualDevice> mpVirDev; /// For all buffered drawing operations.
    ImageList           maILArrows;     /// Arrows in current system colors.
    Color               maBackCol;      /// Background color.
    Color               maArrowCol;     /// Selection arrow color.
    Color               maMarkCol;      /// Selection marker color.
    Color               maHCLineCol;    /// High contrast line color.
    Point               maVirDevPos;    /// Position of virtual device in the control.

    FrameBorder         maLeft;         /// All data of left frame border.
    FrameBorder         maRight;        /// All data of right frame border.
    FrameBorder         maTop;          /// All data of top frame border.
    FrameBorder         maBottom;       /// All data of bottom frame border.
    FrameBorder         maHor;          /// All data of inner horizontal frame border.
    FrameBorder         maVer;          /// All data of inner vertical frame border.
    FrameBorder         maTLBR;         /// All data of top-left to bottom-right frame border.
    FrameBorder         maBLTR;         /// All data of bottom-left to top-right frame border.
    editeng::SvxBorderLine maCurrStyle;    /// Current style and color for new borders.
    frame::Array        maArray;        /// Frame link array to draw an array of frame borders.

    FrameSelFlags       mnFlags;        /// Flags for enabled frame borders.
    FrameBorderPtrVec   maAllBorders;   /// Pointers to all frame borders.
    FrameBorderPtrVec   maEnabBorders;  /// Pointers to enables frame borders.
    Link<LinkParamNone*,void> maSelectHdl;    /// Selection handler.

    long                mnCtrlSize;     /// Size of the control (always square).
    long                mnArrowSize;    /// Size of an arrow image.
    long                mnLine1;        /// Middle of left/top frame borders.
    long                mnLine2;        /// Middle of inner frame borders.
    long                mnLine3;        /// Middle of right/bottom frame borders.
    long                mnFocusOffs;    /// Offset from frame border middle to draw focus.

    bool                mbHor;          /// true = Inner horizontal frame border enabled.
    bool                mbVer;          /// true = Inner vertical frame border enabled.
    bool                mbTLBR;         /// true = Top-left to bottom-right frame border enabled.
    bool                mbBLTR;         /// true = Bottom-left to top-right frame border enabled.
    bool                mbFullRepaint;  /// Used for repainting (false = only copy virtual device).
    bool                mbAutoSelect;   /// true = Auto select a frame border, if focus reaches control.
    bool                mbClicked;      /// true = The control has been clicked at least one time.
    bool                mbHCMode;       /// true = High contrast mode.

    a11y::AccFrameSelector* mpAccess;   /// Pointer to accessibility object of the control.
    css::uno::Reference<css::accessibility::XAccessible>
                        mxAccess;       /// Reference to accessibility object of the control.
    std::vector<a11y::AccFrameSelector*>
                        maChildVec;     /// Pointers to accessibility objects for frame borders.
    std::vector<css::uno::Reference<css::accessibility::XAccessible> >
                        mxChildVec;     /// References to accessibility objects for frame borders.

    explicit            FrameSelectorImpl( FrameSelector& rFrameSel );
                        ~FrameSelectorImpl();

    // initialization
    /** Initializes the control, enables/disables frame borders according to flags. */
    void                Initialize( FrameSelFlags nFlags );

    /** Fills all color members from current style settings. */
    void                InitColors();
    /** Creates the image list with selection arrows regarding current style settings. */
    void                InitArrowImageList();
    /** Initializes global coordinates. */
    void                InitGlobalGeometry();
    /** Initializes coordinates of all frame borders. */
    void                InitBorderGeometry();
    /** Draws the entire control into the internal virtual device. */
    void                InitVirtualDevice();
    /** call this to recalculate based on parent size */
    void                sizeChanged();

    // frame border access
    /** Returns the object representing the specified frame border. */
    const FrameBorder&  GetBorder( FrameBorderType eBorder ) const;
    /** Returns the object representing the specified frame border (write access). */
    FrameBorder&        GetBorderAccess( FrameBorderType eBorder );

    // drawing
    /** Draws the background of the entire control (the gray areas between borders). */
    void                DrawBackground();

    /** Draws selection arrows for the specified frame border. */
    void                DrawArrows( const FrameBorder& rBorder );
    /** Draws arrows in current selection state for all enabled frame borders. */
    void                DrawAllArrows();

    /** Returns the color that has to be used to draw a frame border. */
    Color               GetDrawLineColor( const Color& rColor ) const;
    /** Draws all frame borders. */
    void                DrawAllFrameBorders();

    /** Draws all contents of the control. */
    void                DrawVirtualDevice();
    /** Copies contents of the virtual device to the control. */
    void                CopyVirDevToControl(vcl::RenderContext& rRenderContext);

    /** Draws tracking rectangles for all selected frame borders. */
    void                DrawAllTrackingRects();

    /** Converts a mouse position to the virtual device position. */
    Point               GetDevPosFromMousePos( const Point& rMousePos ) const;

    /** Invalidates the control.
        @param bFullRepaint  true = Full repaint; false = update selection only. */
    void                DoInvalidate( bool bFullRepaint );

    // frame border state and style
    /** Sets the state of the specified frame border. */
    void                SetBorderState( FrameBorder& rBorder, FrameBorderState eState );
    /** Sets the core style of the specified frame border, or hides the frame border, if pStyle is 0. */
    void                SetBorderCoreStyle( FrameBorder& rBorder, const editeng::SvxBorderLine* pStyle );

    /** Changes the state of a frame border after a control event (mouse/keyboard). */
    void                ToggleBorderState( FrameBorder& rBorder );

    // frame border selection
    /** Selects a frame border and schedules redraw. */
    void                SelectBorder( FrameBorder& rBorder, bool bSelect );
    /** Grabs focus without auto-selection of a frame border, if no border selected. */
    void                SilentGrabFocus();

    /** Returns true, if all selected frame borders are equal (or if nothing is selected). */
    bool                SelectedBordersEqual() const;
};


/** Dummy predicate for frame border iterators to use all borders in a container. */
struct FrameBorderDummy_Pred
{
    inline bool operator()( const FrameBorder* ) const { return true; }
};

/** Predicate for frame border iterators to use only visible borders in a container. */
struct FrameBorderVisible_Pred
{
    inline bool operator()( const FrameBorder* pBorder ) const { return pBorder->GetState() == FRAMESTATE_SHOW; }
};

/** Predicate for frame border iterators to use only selected borders in a container. */
struct FrameBorderSelected_Pred
{
    inline bool operator()( const FrameBorder* pBorder ) const { return pBorder->IsSelected(); }
};

/** Template class for all types of frame border iterators. */
template< typename Cont, typename Iter, typename Pred >
class FrameBorderIterBase
{
public:
    typedef Cont container_type;
    typedef Iter iterator_type;
    typedef Pred predicate_type;
    typedef typename Cont::value_type value_type;
    typedef FrameBorderIterBase<Cont, Iter, Pred> this_type;

    explicit            FrameBorderIterBase( container_type& rCont );
    inline bool         Is() const { return maIt != maEnd; }
    this_type&          operator++();
    inline value_type   operator*() const { return *maIt; }

private:
    iterator_type       maIt;
    iterator_type       maEnd;
    predicate_type      maPred;
};

/** Iterator for constant svx::FrameBorder containers, iterates over all borders. */
typedef FrameBorderIterBase< const FrameBorderPtrVec, FrameBorderPtrVec::const_iterator, FrameBorderDummy_Pred >
    FrameBorderCIter;

/** Iterator for mutable svx::FrameBorder containers, iterates over all borders. */
typedef FrameBorderIterBase< FrameBorderPtrVec, FrameBorderPtrVec::iterator, FrameBorderDummy_Pred >
    FrameBorderIter;

/** Iterator for constant svx::FrameBorder containers, iterates over visible borders. */
typedef FrameBorderIterBase< const FrameBorderPtrVec, FrameBorderPtrVec::const_iterator, FrameBorderVisible_Pred >
    VisFrameBorderCIter;

/** Iterator for mutable svx::FrameBorder containers, iterates over visible borders. */
typedef FrameBorderIterBase< FrameBorderPtrVec, FrameBorderPtrVec::iterator, FrameBorderVisible_Pred >
    VisFrameBorderIter;

/** Iterator for constant svx::FrameBorder containers, iterates over selected borders. */
typedef FrameBorderIterBase< const FrameBorderPtrVec, FrameBorderPtrVec::const_iterator, FrameBorderSelected_Pred >
    SelFrameBorderCIter;

/** Iterator for mutable svx::FrameBorder containers, iterates over selected borders. */
typedef FrameBorderIterBase< FrameBorderPtrVec, FrameBorderPtrVec::iterator, FrameBorderSelected_Pred >
    SelFrameBorderIter;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
