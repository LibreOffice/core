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
#include "precompiled_sfx2.hxx"

#include "querytemplate.hxx"
#include "sfx2/sfxresid.hxx"
#include "doc.hrc"
#include "helpid.hrc"
#include <vcl/svapp.hxx>

namespace sfx2
{

QueryTemplateBox::QueryTemplateBox( Window* pParent, const String& rMessage ) :
    MessBox ( pParent, 0, Application::GetDisplayName(), rMessage )
{
    SetImage( QueryBox::GetStandardImage() );
    SetHelpId( HID_QUERY_LOAD_TEMPLATE );

    AddButton( String( SfxResId( STR_QRYTEMPL_UPDATE_BTN ) ), RET_YES,
            BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
    AddButton( String( SfxResId( STR_QRYTEMPL_KEEP_BTN ) ), RET_NO, BUTTONDIALOG_CANCELBUTTON );
}

} // end of namespace sfx2

