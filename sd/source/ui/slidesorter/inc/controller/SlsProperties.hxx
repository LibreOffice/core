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

#ifndef SD_SLIDESORTER_PROPERTIES_HEADER
#define SD_SLIDESORTER_PROPERTIES_HEADER

#include <tools/color.hxx>

namespace sd { namespace slidesorter { namespace controller {

/** An extensible set of properties used throughout the slide sorter.
*/
class Properties
{
public:
    Properties (void);
    ~Properties (void);

    /** Call this method after receiving a VCLEVENT_APPLICATION_DATACHANGED
        event.
    */
    void HandleDataChangeEvent (void);

    /** When this method returns <TRUE/> then the current slide is
        highlighted in the view.  The default value is <FALSE/>.
    */
    bool IsHighlightCurrentSlide (void) const;
    void SetHighlightCurrentSlide (const bool bIsHighlightCurrentSlide);

    /** When this method returns <TRUE/> then the selection is indicated in
        the view (typically by drawing rectangles around the selected
        slides.)  The default value is <TRUE/>.
    */
    bool IsShowSelection (void) const;
    void SetShowSelection (const bool bIsShowSelection);

    /** When this method returns <TRUE/> then the focusdselection is indicated in
        the view (typically by drawing dotted rectangles around the selected
        slides.)  The default value is <TRUE/>.
    */
    bool IsShowFocus (void) const;
    void SetShowFocus (const bool bIsShowFocus);

    /** When this method returns <TRUE/> then on a selection change the
        visible area is adapted so that the selected slides are shown
        centered in the view.  This can be used to center the current slide
        by selecting only the current slide.  The default value is <FALSE/>.
    */
    bool IsCenterSelection (void) const;
    void SetCenterSelection (const bool bIsCenterSelection);

    /** When this mehod returns <TRUE/> then the view may try to change the
        visible area by scrolling it smoothly on the screen.  Experimental.
        Default value is <FALSE/>.
    */
    bool IsSmoothSelectionScrolling (void) const;
    void SetSmoothSelectionScrolling (const bool bIsSmoothSelectionScrolling);

    /** When this method returns <TRUE/> then during a full screen
        presentation the previews in a slide sorter are not updated.
        Default value is <TRUE/>.
    */
    bool IsSuspendPreviewUpdatesDuringFullScreenPresentation (void) const;
    void SetSuspendPreviewUpdatesDuringFullScreenPresentation (const bool bFlag);

    /** Return the background color.
    */
    Color GetBackgroundColor (void) const;
    void SetBackgroundColor (const Color& rColor);

    /** Return the text color.
    */
    Color GetTextColor (void) const;
    void SetTextColor (const Color& rColor);

    /** Return the color in which selections are to be painted.
    */
    Color GetSelectionColor (void) const;
    void SetSelectionColor (const Color& rColor);

    /** Return the color used for highlighting e.g. the current slide.
    */
    Color GetHighlightColor (void) const;
    void SetHighlightColor (const Color& rColor);

    /** The UI can be set to be read only indepently from the model status.
        Used for instance in the presenter view.
    */
    bool IsUIReadOnly (void) const;
    void SetUIReadOnly (const bool bIsUIReadOnly);

    /** The mouse over effect (and whether a mouse motion starts a multi
        selection or a drag-and-drop) can be triggered by just the preview
        area or the whole page object area.
    */
    bool IsOnlyPreviewTriggersMouseOver (void) const;


private:
    bool mbIsHighlightCurrentSlide;
    bool mbIsShowSelection;
    bool mbIsShowFocus;
    bool mbIsCenterSelection;
    bool mbIsSmoothSelectionScrolling;
    bool mbIsSuspendPreviewUpdatesDuringFullScreenPresentation;
    Color maBackgroundColor;
    Color maTextColor;
    Color maSelectionColor;
    Color maHighlightColor;
    bool mbIsUIReadOnly;
    bool mbIsOnlyPreviewTriggersMouseOver;
    bool mbIsHighContrastModeActive;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
