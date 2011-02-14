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
#include "precompiled_automation.hxx"
#include <string.h>

#include "servres.hrc"
#include "servuid.hxx"
#include "servres.hxx"


ModalDialogGROSSER_TEST_DLG::ModalDialogGROSSER_TEST_DLG( Window * pParent, const ResId & rResId, sal_Bool bFreeRes )
    : ModalDialog( pParent, rResId ),
    aCheckBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aTriStateBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aOKButton1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aTimeField1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aMultiLineEdit1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aGroupBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aRadioButton1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aRadioButton2( this, ResId( 2, *rResId.GetResMgr() ) ),
    aMultiListBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aComboBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aDateBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aFixedText1( this, ResId( 1, *rResId.GetResMgr() ) )
{
    if( bFreeRes ) FreeResource();
}

MenuMENU_CLIENT::MenuMENU_CLIENT( const ResId & rResId, sal_Bool )
    : MenuBar( rResId )
{
    // No subresources, automatic free resource
}

