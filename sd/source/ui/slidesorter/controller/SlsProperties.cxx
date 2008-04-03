/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsProperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:26:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "controller/SlsProperties.hxx"
#include <vcl/svapp.hxx>

namespace sd { namespace slidesorter { namespace controller {

Properties::Properties (void)
    : mbIsHighlightCurrentSlide(false),
      mbIsShowSelection(true),
      mbIsShowFocus(true),
      mbIsCenterSelection(false),
      mbIsSmoothSelectionScrolling(false),
      mbIsSuspendPreviewUpdatesDuringFullScreenPresentation(true),
      maBackgroundColor(Application::GetSettings().GetStyleSettings().GetWindowColor()),
      maTextColor(Application::GetSettings().GetStyleSettings().GetActiveTextColor()),
      maSelectionColor(Application::GetSettings().GetStyleSettings().GetMenuHighlightColor()),
      maHighlightColor(Application::GetSettings().GetStyleSettings().GetMenuHighlightColor()),
      mbIsUIReadOnly(false)
{
}




Properties::~Properties (void)
{
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


} } } // end of namespace ::sd::slidesorter::controller
