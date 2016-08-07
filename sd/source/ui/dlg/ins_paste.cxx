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
#include "precompiled_sd.hxx"

#include "sdresid.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "ins_paste.hrc"
#include "ins_paste.hxx"

// --------------------
// - SdInsertPasteDlg -
// --------------------

SdInsertPasteDlg::SdInsertPasteDlg( Window* pWindow ) :
    ModalDialog( pWindow, SdResId( DLG_INSERT_PASTE ) ),
    aFlPosition( this, SdResId( FL_POSITION ) ),
    aRbBefore( this, SdResId( RB_BEFORE ) ),
    aRbAfter( this, SdResId( RB_AFTER ) ),
    aBtnOK( this, SdResId( BTN_OK ) ),
    aBtnCancel( this, SdResId( BTN_CANCEL ) ),
    aBtnHelp( this, SdResId( BTN_HELP ) )
{
    FreeResource();
    aRbAfter.Check( sal_True );
}

// -----------------------------------------------------------------------------

SdInsertPasteDlg::~SdInsertPasteDlg()
{
}

// -----------------------------------------------------------------------------

sal_Bool SdInsertPasteDlg::IsInsertBefore() const
{
    return( aRbBefore.IsChecked() );
}
