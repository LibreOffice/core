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
 ***********************************************************************/

#include <sfx2/layout.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <toolkit/awt/vclxwindow.hxx>

namespace layout
{

using namespace ::com::sun::star;

SfxTabPage::SfxTabPage (::Window *parent, char const* xml_file, char const* id, SfxItemSet const* set)
    : ::SfxTabPage (parent, 0, *set)
    , InPlug (parent, xml_file, id)
{
    dynamic_cast< ::Window* > (this)->SetComponentInterface (GetVCLXWindow ());
}

::Window* SfxTabPage::GetParent () const
{
    return Window::GetParent ();
}

::Window* SfxTabPage::GetWindow () const
{
    return ::layout::Window::GetWindow();
}

void SfxTabPage::FreeResource ()
{
    //::Window::FreeResource ();
}

Size SfxTabPage::GetOptimalSize (WindowSizeType) const
{
    awt::Size s = GetVCLXWindow()->getMinimumSize ();
    return Size (s.Width, s.Height);
}

} // end namespace layout

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
