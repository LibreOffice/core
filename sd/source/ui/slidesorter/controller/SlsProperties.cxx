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


#include "controller/SlsProperties.hxx"
#include <vcl/svapp.hxx>

namespace sd { namespace slidesorter { namespace controller {

Properties::Properties (void)
    : mbIsHighlightCurrentSlide(false),
      mbIsShowSelection(true),
      mbIsShowFocus(true),
      mbIsCenterSelection(false),
      mbIsSmoothSelectionScrolling(true),
      mbIsSuspendPreviewUpdatesDuringFullScreenPresentation(true),
      maBackgroundColor(Application::GetSettings().GetStyleSettings().GetWindowColor()),
      maTextColor(Application::GetSettings().GetStyleSettings().GetActiveTextColor()),
      maSelectionColor(Application::GetSettings().GetStyleSettings().GetHighlightColor()),
      maHighlightColor(Application::GetSettings().GetStyleSettings().GetMenuHighlightColor()),
      mbIsUIReadOnly(false),
      mbIsOnlyPreviewTriggersMouseOver(true),
      mbIsHighContrastModeActive(
          Application::GetSettings().GetStyleSettings().GetHighContrastMode())
{
}




Properties::~Properties (void)
{
}




void Properties::HandleDataChangeEvent (void)
{
    maBackgroundColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    maTextColor = Application::GetSettings().GetStyleSettings().GetActiveTextColor();
    maSelectionColor = Application::GetSettings().GetStyleSettings().GetHighlightColor();
    maHighlightColor = Application::GetSettings().GetStyleSettings().GetMenuHighlightColor();
    mbIsHighContrastModeActive
        = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
}




bool Properties::IsHighlightCurrentSlide (void) const
{
    return mbIsHighlightCurrentSlide;
}




void Properties::SetHighlightCurrentSlide (const bool bIsHighlightCurrentSlide)
{
    mbIsHighlightCurrentSlide = bIsHighlightCurrentSlide;
}




bool Properties::IsShowSelection (void) const
{
    return mbIsShowSelection;
}




void Properties::SetShowSelection (const bool bIsShowSelection)
{
    mbIsShowSelection = bIsShowSelection;
}




bool Properties::IsShowFocus (void) const
{
    return mbIsShowFocus;
}




void Properties::SetShowFocus (const bool bIsShowFocus)
{
    mbIsShowFocus = bIsShowFocus;
}




bool Properties::IsCenterSelection (void) const
{
    return mbIsCenterSelection;
}




void Properties::SetCenterSelection (const bool bIsCenterSelection)
{
    mbIsCenterSelection = bIsCenterSelection;
}




bool Properties::IsSmoothSelectionScrolling (void) const
{
    return mbIsSmoothSelectionScrolling;
}




void Properties::SetSmoothSelectionScrolling (const bool bIsSmoothSelectionScrolling)
{
    mbIsSmoothSelectionScrolling = bIsSmoothSelectionScrolling;
}




bool Properties::IsSuspendPreviewUpdatesDuringFullScreenPresentation (void) const
{
    return mbIsSuspendPreviewUpdatesDuringFullScreenPresentation;
}




void Properties::SetSuspendPreviewUpdatesDuringFullScreenPresentation (const bool bFlag)
{
    mbIsSuspendPreviewUpdatesDuringFullScreenPresentation = bFlag;
}




Color Properties::GetBackgroundColor (void) const
{
    return maBackgroundColor;
}




void Properties::SetBackgroundColor (const Color& rColor)
{
    maBackgroundColor = rColor;
}



Color Properties::GetTextColor (void) const
{
    return maTextColor;
}




void Properties::SetTextColor (const Color& rColor)
{
    maTextColor = rColor;
}




Color Properties::GetSelectionColor (void) const
{
    return maSelectionColor;
}




void Properties::SetSelectionColor (const Color& rColor)
{
    maSelectionColor = rColor;
}




Color Properties::GetHighlightColor (void) const
{
    return maHighlightColor;
}




void Properties::SetHighlightColor (const Color& rColor)
{
    maHighlightColor = rColor;
}




bool Properties::IsUIReadOnly (void) const
{
    return mbIsUIReadOnly;
}




void Properties::SetUIReadOnly (const bool bIsUIReadOnly)
{
    mbIsUIReadOnly = bIsUIReadOnly;
}




bool Properties::IsOnlyPreviewTriggersMouseOver (void) const
{
    return mbIsOnlyPreviewTriggersMouseOver;
}


} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
