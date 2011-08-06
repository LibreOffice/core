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

#include "ids.hrc"
#include "lockfailed.hxx"

#include <vcl/button.hxx>

LockFailedQueryBox::LockFailedQueryBox( Window* pParent, ResMgr* pResMgr ) :
    MessBox(pParent, 0,
            String( ResId( STR_LOCKFAILED_TITLE, *pResMgr ) ),
            String::EmptyString() )
{
    SetImage( ErrorBox::GetStandardImage() );

    AddButton( BUTTON_OK, RET_OK, BUTTONDIALOG_OKBUTTON );
    AddButton( BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON );

    SetMessText( String( ResId( STR_LOCKFAILED_MSG, *pResMgr ) ) );
    SetCheckBoxText( String( ResId( STR_LOCKFAILED_DONTSHOWAGAIN, *pResMgr ) ) );
}

LockFailedQueryBox::~LockFailedQueryBox()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
