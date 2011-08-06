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

#include <com/sun/star/awt/WindowAttribute.hpp>
#include <sal/types.h>
#include <sfx2/basedlgs.hxx>
#include <toolkit/awt/vclxtoolkit.hxx>
#include <toolkit/awt/vclxdialog.hxx>

using rtl::OUString;

class SfxModelessDialog_unprotect : public SfxModelessDialog
{
public:
    SfxModelessDialog_unprotect (Window *parent, WinBits attributes);
};

SFX2_DLLPUBLIC SfxBindings* sfx2_global_bindings;
SFX2_DLLPUBLIC SfxChildWindow* sfx2_global_child;

SfxModelessDialog_unprotect::SfxModelessDialog_unprotect (Window *parent, WinBits bits)
    : SfxModelessDialog (sfx2_global_bindings, sfx2_global_child, parent, bits)
{
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT Window* CreateWindow (VCLXWindow** vcl, OUString const& name, Window* parent, long& attributes)
{
    Window* window = 0;
    if (0)
    {
        ;
    }
    else if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sfxmodelessdialog")))
    {
        window = new SfxModelessDialog_unprotect (parent, ImplGetWinBits (attributes, 0));
        *vcl = new layoutimpl::VCLXDialog ();
    }
    return window;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
