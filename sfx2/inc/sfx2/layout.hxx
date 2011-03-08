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

#ifndef _SFX2_LAYOUT_HXX
#define _SFX2_LAYOUT_HXX

#include <layout/layout.hxx>
#include <sfx2/tabdlg.hxx>
#if ENABLE_LAYOUT
#undef ENABLE_LAYOUT_SFX_TABDIALOG
#define ENABLE_LAYOUT_SFX_TABDIALOG 1
#include <sfx2/layout-tabdlg.hxx>
#endif

class SfxChildWindow;
class SfxBindings;

namespace layout
{

class SFX2_DLLPUBLIC SfxDialog
    : public Dialog
{
public:
    SfxDialog (::Window* parent, char const* xml_file, char const* id, SfxBindings* bindings=0, SfxChildWindow* child=0);
};

class SFX2_DLLPUBLIC SfxTabPage
    : public ::SfxTabPage
    , public InPlug
{
public:
    SfxTabPage (::Window *parent, char const* xml_file, char const* id, SfxItemSet const* set=0);
    ::Window* GetParent () const;
    using ::Window::GetWindow;
    ::Window* GetWindow () const;
    Size GetOptimalSize (WindowSizeType eType) const;
    void FreeResource ();
};
} // end namespace layout

#endif /* _SFX2_LAYOUT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
