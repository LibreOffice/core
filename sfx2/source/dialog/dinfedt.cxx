/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dinfedt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:15:03 $
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

#ifndef GCC
#pragma hdrstop
#endif

#include "dinfedt.hxx"
#include "sfxresid.hxx"

#include "sfx.hrc"
#include "dinfedt.hrc"

// class InfoEdit_Impl ---------------------------------------------------

void InfoEdit_Impl::KeyInput( const KeyEvent& rKEvent )
{
    if ( rKEvent.GetCharCode() != '~' )
        Edit::KeyInput( rKEvent );
}

// class SfxDocInfoEditDlg -----------------------------------------------

SfxDocInfoEditDlg::SfxDocInfoEditDlg( Window* pParent ) :

    ModalDialog( pParent, SfxResId( DLG_DOCINFO_EDT ) ),

    aInfoFL     ( this, ResId( FL_INFO ) ),
    aInfo1ED    ( this, ResId( ED_INFO1 ) ),
    aInfo2ED    ( this, ResId( ED_INFO2 ) ),
    aInfo3ED    ( this, ResId( ED_INFO3 ) ),
    aInfo4ED    ( this, ResId( ED_INFO4 ) ),
    aOkBT       ( this, ResId( BT_OK ) ),
    aCancelBT   ( this, ResId( BT_CANCEL ) ),
    aHelpBtn    ( this, ResId( BTN_HELP ) )

{
    FreeResource();
}

