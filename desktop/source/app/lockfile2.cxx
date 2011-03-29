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
    aConfig.SetGroup( LOCKFILE_GROUP );
    ByteString aHost  = aConfig.ReadKey( LOCKFILE_HOSTKEY );
    ByteString aUser  = aConfig.ReadKey( LOCKFILE_USERKEY );
    ByteString aStamp = aConfig.ReadKey( LOCKFILE_STAMPKEY );
    ByteString aTime  = aConfig.ReadKey( LOCKFILE_TIMEKEY );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
