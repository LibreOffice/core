/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

