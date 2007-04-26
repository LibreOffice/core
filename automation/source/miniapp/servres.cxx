/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servres.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 09:39:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"
#include <string.h>

#include "servres.hrc"
#include "servuid.hxx"
#include "servres.hxx"


ModalDialogGROSSER_TEST_DLG::ModalDialogGROSSER_TEST_DLG( Window * pParent, const ResId & rResId, BOOL bFreeRes )
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

MenuMENU_CLIENT::MenuMENU_CLIENT( const ResId & rResId, BOOL )
    : MenuBar( rResId )
{
    // No subresources, automatic free resource
}

