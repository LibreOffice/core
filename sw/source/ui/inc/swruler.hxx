/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SW_COMMENT_RULER_HXX
#define SW_COMMENT_RULER_HXX

#include <svx/ruler.hxx>

class ViewShell;
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
        ViewShell* pViewSh,
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
    virtual void Paint( const Rectangle& rRect );

protected:
    ViewShell * mpViewShell;     //< Shell to check if there is any comments on doc and their visibility
    SwEditWin * mpSwWin;         //< Used to get SwView to change the SideBar visibility
    bool        mbIsHighlighted; //< If comment control is highlighted (mouse is over it)
    VirtualDevice maVirDev;      //< VirtualDevice of this window. Just for convenience.

    /**
     * Callback function to handle a mouse button down event.
     *
     * When on comment control, it toggles the comment panel visibility.
     */
    virtual void MouseButtonDown( const MouseEvent& rMEvt );
    /**
     * Callback function to handle a mouse move event.
     *
     * When on comment control, it let the control responsive by highlighting.
     */
    virtual void MouseMove(const MouseEvent& rMEvt);
    /**
     * Callback function to handle a context menu call (mouse right button click).
     *
     * When on comment control, it does not do anything.
     */
    virtual void Command( const CommandEvent& rCEvt );
    /**
     * Update the view.
     *
     * Update the ruler appearance according to the document view and
     * current cursor position.
     */
    virtual void Update();

    /**
     * Get the rectangle area that should be used to draw the comment control.
     *
     * It is horizontally aligned to the SideBar panel.
     * \param Rectangle The area where the comment control is.
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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
