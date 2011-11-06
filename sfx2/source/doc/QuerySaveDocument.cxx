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
#include "sfx2/QuerySaveDocument.hxx"

#include <sfx2/sfx.hrc>
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxuno.hxx>
#include "doc.hrc"
#include <vcl/msgbox.hxx>

// -----------------------------------------------------------------------------
short ExecuteQuerySaveDocument(Window* _pParent,const String& _rTitle)
{
    String aText( SfxResId( STR_QUERY_SAVE_DOCUMENT ) );
    aText.SearchAndReplace( DEFINE_CONST_UNICODE( "$(DOC)" ),
                            _rTitle );
    QueryBox aQBox( _pParent, WB_YES_NO_CANCEL | WB_DEF_YES, aText );
    aQBox.SetButtonText( BUTTONID_NO, SfxResId( STR_NOSAVEANDCLOSE ) );
    aQBox.SetButtonText( BUTTONID_YES, SfxResId( STR_SAVEDOC ) );
    return aQBox.Execute();
}
// -----------------------------------------------------------------------------
