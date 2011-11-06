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

#include "querytemplate.hxx"
#include "sfx2/sfxresid.hxx"
#include "doc.hrc"
#include "helpid.hrc"
#include <vcl/svapp.hxx>

namespace sfx2
{

QueryTemplateBox::QueryTemplateBox( Window* pParent, const String& rMessage ) :
    MessBox ( pParent, 0, Application::GetDisplayName(), rMessage )
{
    SetImage( QueryBox::GetStandardImage() );
    SetHelpId( HID_QUERY_LOAD_TEMPLATE );

    AddButton( String( SfxResId( STR_QRYTEMPL_UPDATE_BTN ) ), RET_YES,
            BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
    AddButton( String( SfxResId( STR_QRYTEMPL_KEEP_BTN ) ), RET_NO, BUTTONDIALOG_CANCELBUTTON );
}

} // end of namespace sfx2

