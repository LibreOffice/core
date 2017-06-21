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

#include "controller/SlsProperties.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

namespace sd { namespace slidesorter { namespace controller {

Properties::Properties()
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
      mbIsUIReadOnly(false)
{
}

Properties::~Properties()
{
}

void Properties::HandleDataChangeEvent()
{
    maBackgroundColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    maTextColor = Application::GetSettings().GetStyleSettings().GetActiveTextColor();
    maSelectionColor = Application::GetSettings().GetStyleSettings().GetHighlightColor();
    maHighlightColor = Application::GetSettings().GetStyleSettings().GetMenuHighlightColor();
}

void Properties::SetHighlightCurrentSlide (const bool bIsHighlightCurrentSlide)
{
    mbIsHighlightCurrentSlide = bIsHighlightCurrentSlide;
}

void Properties::SetShowSelection (const bool bIsShowSelection)
{
    mbIsShowSelection = bIsShowSelection;
}

void Properties::SetShowFocus (const bool bIsShowFocus)
{
    mbIsShowFocus = bIsShowFocus;
}

void Properties::SetCenterSelection (const bool bIsCenterSelection)
{
    mbIsCenterSelection = bIsCenterSelection;
}

void Properties::SetSmoothSelectionScrolling (const bool bIsSmoothSelectionScrolling)
{
    mbIsSmoothSelectionScrolling = bIsSmoothSelectionScrolling;
}

void Properties::SetSuspendPreviewUpdatesDuringFullScreenPresentation (const bool bFlag)
{
    mbIsSuspendPreviewUpdatesDuringFullScreenPresentation = bFlag;
}

void Properties::SetBackgroundColor (const Color& rColor)
{
    maBackgroundColor = rColor;
}

void Properties::SetTextColor (const Color& rColor)
{
    maTextColor = rColor;
}

void Properties::SetSelectionColor (const Color& rColor)
{
    maSelectionColor = rColor;
}

void Properties::SetHighlightColor (const Color& rColor)
{
    maHighlightColor = rColor;
}

void Properties::SetUIReadOnly (const bool bIsUIReadOnly)
{
    mbIsUIReadOnly = bIsUIReadOnly;
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
