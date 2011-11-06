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
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif

#include "sfx2/dinfedt.hxx"
#include "sfx2/sfxresid.hxx"

#include <sfx2/sfx.hrc>
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

    aInfoFL     ( this, SfxResId( FL_INFO ) ),
    aInfo1ED    ( this, SfxResId( ED_INFO1 ) ),
    aInfo2ED    ( this, SfxResId( ED_INFO2 ) ),
    aInfo3ED    ( this, SfxResId( ED_INFO3 ) ),
    aInfo4ED    ( this, SfxResId( ED_INFO4 ) ),
    aOkBT       ( this, SfxResId( BT_OK ) ),
    aCancelBT   ( this, SfxResId( BT_CANCEL ) ),
    aHelpBtn    ( this, SfxResId( BTN_HELP ) )

{
    FreeResource();
}

