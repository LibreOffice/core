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

#ifndef LAYOUT_AWT_VCLXBUTTON_HXX
#define LAYOUT_AWT_VCLXBUTTON_HXX

#include <toolkit/awt/vclxwindows.hxx>

/* Replacements for broken toolkit/ impls. of ok, cancel, help button, etc. */

namespace layoutimpl
{

class VCLXIconButton : public VCLXButton
{
public:
    VCLXIconButton( Window* p, rtl::OUString aDefaultLabel, const char *pGraphName );
    void Show ();
};

class VCLXOKButton : public VCLXIconButton
{
public:
    VCLXOKButton( Window *p );
};

class VCLXCancelButton : public VCLXIconButton
{
public:
    VCLXCancelButton( Window *p );
};

class VCLXYesButton : public VCLXIconButton
{
public:
    VCLXYesButton( Window *p );
};

class VCLXNoButton : public VCLXIconButton
{
public:
    VCLXNoButton( Window *p );
};

class VCLXRetryButton : public VCLXIconButton
{
public:
    VCLXRetryButton( Window *p );
};

class VCLXIgnoreButton : public VCLXIconButton
{
public:
    VCLXIgnoreButton( Window *p );
};

class VCLXResetButton : public VCLXIconButton
{
public:
    VCLXResetButton( Window *p );
};

class VCLXApplyButton : public VCLXIconButton
{
public:
    VCLXApplyButton( Window *p );
};

class VCLXHelpButton : public VCLXIconButton
{
public:
    VCLXHelpButton( Window *p );
};

// TODO.  Reuse vcl/Morebutton, or make AdvancedButton reuse me?
class VCLXMoreButton : public VCLXIconButton
{
public:
    VCLXMoreButton( Window *p );
};

class VCLXAdvancedButton : public VCLXIconButton
{
public:
    VCLXAdvancedButton( Window *p );
};

} // namespace layoutimpl

#endif // LAYOUT_AWT_VCLXBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
