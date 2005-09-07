/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servres.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:20:01 $
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
#include <string.h>

#include "servres.hrc"
#include "servuid.hxx"
#include "servres.hxx"


ModalDialogGROSSER_TEST_DLG::ModalDialogGROSSER_TEST_DLG( Window * pParent, const ResId & rResId, BOOL bFreeRes )
    : ModalDialog( pParent, rResId ),
    aCheckBox1( this, ResId( 1 ) ),
    aTriStateBox1( this, ResId( 1 ) ),
    aOKButton1( this, ResId( 1 ) ),
    aTimeField1( this, ResId( 1 ) ),
    aMultiLineEdit1( this, ResId( 1 ) ),
    aGroupBox1( this, ResId( 1 ) ),
    aRadioButton1( this, ResId( 1 ) ),
    aRadioButton2( this, ResId( 2 ) ),
    aMultiListBox1( this, ResId( 1 ) ),
    aComboBox1( this, ResId( 1 ) ),
    aDateBox1( this, ResId( 1 ) ),
    aFixedText1( this, ResId( 1 ) )
{
    if( bFreeRes ) FreeResource();
}

MenuMENU_CLIENT::MenuMENU_CLIENT( const ResId & rResId, BOOL )
    : MenuBar( rResId )
{
    // No subresources, automatic free resource
}

