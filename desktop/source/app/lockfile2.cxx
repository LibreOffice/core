/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lockfile2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:37:14 $
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
#include "precompiled_desktop.hxx"

#include "vcl/msgbox.hxx"
#include "desktopresid.hxx"
#include "desktop.hrc"
#include "tools/config.hxx"
#include "lockfile.hxx"


namespace desktop {

bool Lockfile_execWarning( Lockfile * that )
{
    // read information from lock
    String aLockname = that->m_aLockname;
    Config aConfig(aLockname);
    aConfig.SetGroup(that->Group());
    ByteString aHost  = aConfig.ReadKey( that->Hostkey() );
    ByteString aUser  = aConfig.ReadKey( that->Userkey() );
    ByteString aStamp = aConfig.ReadKey( that->Stampkey() );
    ByteString aTime  = aConfig.ReadKey( that->Timekey() );

    // display warning and return response
    QueryBox aBox( NULL, DesktopResId( QBX_USERDATALOCKED ) );
    // set box title
    String aTitle = String( DesktopResId( STR_TITLE_USERDATALOCKED ));
    aBox.SetText( aTitle );
    // insert values...
    String aMsgText = aBox.GetMessText( );
    aMsgText.SearchAndReplaceAscii(
        "$u", String( aUser, RTL_TEXTENCODING_ASCII_US) );
    aMsgText.SearchAndReplaceAscii(
        "$h", String( aHost, RTL_TEXTENCODING_ASCII_US) );
    aMsgText.SearchAndReplaceAscii(
        "$t", String( aTime, RTL_TEXTENCODING_ASCII_US) );
    aBox.SetMessText(aMsgText);
    // do it
    return aBox.Execute( ) == RET_YES;
}

}

