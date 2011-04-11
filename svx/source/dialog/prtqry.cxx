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
#include "precompiled_svx.hxx"

#include <svx/dialogs.hrc>
#include <svx/prtqry.hxx>
#include <svx/dialmgr.hxx>
#include <tools/shl.hxx>

SvxPrtQryBox::SvxPrtQryBox(Window* pParent) :
    MessBox(pParent, 0,
            String(SVX_RES(RID_SVXSTR_QRY_PRINT_TITLE)),
            String(SVX_RES(RID_SVXSTR_QRY_PRINT_MSG)))
{
    SetImage( QueryBox::GetStandardImage() );

    AddButton(String(SVX_RES(RID_SVXSTR_QRY_PRINT_SELECTION)), RET_OK,
            BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON);

    AddButton(String(SVX_RES(RID_SVXSTR_QRY_PRINT_ALL)), 2, 0);
    AddButton(BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON);
    SetButtonHelpText( RET_OK, String() );
}

SvxPrtQryBox::~SvxPrtQryBox()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
