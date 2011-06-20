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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "TextDirectionListBox.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include <svl/languageoptions.hxx>
#include <vcl/window.hxx>

namespace chart
{

TextDirectionListBox::TextDirectionListBox( Window* pParent, const ResId& rResId, Window* pWindow1, Window* pWindow2 ) :
    svx::FrameDirectionListBox( pParent, rResId )
{
    InsertEntryValue( String( SchResId( STR_TEXT_DIRECTION_LTR ) ), FRMDIR_HORI_LEFT_TOP );
    InsertEntryValue( String( SchResId( STR_TEXT_DIRECTION_RTL ) ), FRMDIR_HORI_RIGHT_TOP );
    InsertEntryValue( String( SchResId( STR_TEXT_DIRECTION_SUPER ) ), FRMDIR_ENVIRONMENT );

    if( !SvtLanguageOptions().IsCTLFontEnabled() )
    {
        Hide();
        if( pWindow1 ) pWindow1->Hide();
        if( pWindow2 ) pWindow2->Hide();
    }
}

TextDirectionListBox::~TextDirectionListBox()
{
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
