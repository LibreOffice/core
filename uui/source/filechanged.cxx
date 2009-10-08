/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: openlocked.cxx,v $
 * $Revision: 1.3 $
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

#include "ids.hrc"
#include "filechanged.hxx"

FileChangedQueryBox::FileChangedQueryBox( Window* pParent, ResMgr* pResMgr ) :
    MessBox(pParent, 0,
            String( ResId( STR_FILECHANGED_TITLE, *pResMgr ) ),
            String::EmptyString() )
{
    SetImage( QueryBox::GetStandardImage() );

    AddButton( String( ResId( STR_FILECHANGED_SAVEANYWAY_BTN, *pResMgr ) ), RET_YES,
            BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
    AddButton( BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON );

    SetButtonHelpText( RET_YES, String::EmptyString() );
    SetMessText( String( ResId( STR_FILECHANGED_MSG, *pResMgr ) ) );
}

FileChangedQueryBox::~FileChangedQueryBox()
{
}

