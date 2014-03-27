/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UI_INC_SWRULER_HXX
#define INCLUDED_SW_SOURCE_UI_INC_SWRULER_HXX

#include <svx/ruler.hxx>

class SwViewShell;
class View;
class Window;
class SwEditWin;

/**
 * An horizontal ruler with a control for comment panel visibility fo Writer.
 *
 * The comment control only appears when the document has comments already.
 */
class SwCommentRuler
    : public SvxRuler
{
public:
    SwCommentRuler (
        SwViewShell* pViewSh,
        Window* pParent,
        SwEditWin* pWin,
        sal_uInt16 nRulerFlags,
        SfxBindings& rBindings,
        WinBits nWinStyle);
    virtual ~SwCommentRuler ();

    /**
     * Paint the ruler.
     * \param rRect ignored
     */
    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;

protected:
    SwViewShell * mpViewShell;     //< Shell to check if there is any comments on doc and their visibility
    SwEditWin * mpSwWin;         //< Used to get SwView to change the SideBar visibility
    bool        mbIsHighlighted; //< If comment control is highlighted (mouse is over it)
    Timer       maFadeTimer;     //< Timer for high/'low'light fading
    int         mnFadeRate;      //< From 0 to 100. 0 means not highlighted.
    VirtualDevice maVirDev;      //< VirtualDevice of this window. Just for convenience.

    /**
     * Callback function to handle a mouse button down event.
     *
     * When on comment control, it toggles the comment panel visibility.
     */
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    /**
     * Callback function to handle a mouse move event.
     *
     * When on comment control, it let the control responsive by highlighting.
     */
    virtual void MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    /**
     * Callback function to handle a context menu call (mouse right button click).
     *
     * When on comment control, it does not do anything.
     */
    virtual void Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    /**
     * Update the view.
     *
     * Update the ruler appearance according to the document view and
     * current cursor position.
     */
    virtual void Update() SAL_OVERRIDE;

    /**
     * Get the rectangle area that should be used to draw the comment control.
     *
     * It is horizontally aligned to the SideBar panel.
     * \return The area where the comment control is.
     */
    Rectangle GetCommentControlRegion();

    /**
     * Paint the comment control on VirtualDevice.
     */
    void DrawCommentControl();
    /**
     * Draw a little horizontal arrow tip on VirtualDevice.
     * \param nX left coordinate of arrow
     * \param nY top coordinate of arrow
     * \param Color arrow color
     * \param bPointRight if arrow should point to right. Otherwise, it will point left.
     */
    void ImplDrawArrow(long nX, long nY, const Color& rColor, bool bPointRight);

    /**
     * Update the tooltip text.
     */
    void UpdateCommentHelpText();

    /**
     * Get the proper color between two options, according to current status.
     *
     * The return color can be one of the given colors, or a merged one.
     * It depends on highlight fadind status.
     *
     * \param rHighColor color used to highlight status
     * \param rLowColor color used to normal status
     * \return The proper color to used in moment
     */
    Color GetFadedColor(const Color &rHighColor, const Color &rLowColor);

    /// Fade timer callback.
    DECL_LINK(FadeHandler, void *);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
