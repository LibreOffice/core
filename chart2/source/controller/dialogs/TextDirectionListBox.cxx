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
#include "precompiled_chart2.hxx"

#include "TextDirectionListBox.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include <svl/languageoptions.hxx>
#include <vcl/window.hxx>

namespace chart
{

TextDirectionListBox::TextDirectionListBox( Window* pParent, const ResId& rResId, Window* pWindow1, Window* pWindow2 ) :
    svx::FrameDirectionListBox( pParent, rResId )
{
    InsertEntryValue( String( SchResId( STR_TEXT_DIRECTION_LTR ) ), FRMDIR_HORI_LEFT_TOP );
    InsertEntryValue( String( SchResId( STR_TEXT_DIRECTION_RTL ) ), FRMDIR_HORI_RIGHT_TOP );
    InsertEntryValue( String( SchResId( STR_TEXT_DIRECTION_SUPER ) ), FRMDIR_ENVIRONMENT );

    if( !SvtLanguageOptions().IsCTLFontEnabled() )
    {
        Hide();
        if( pWindow1 ) pWindow1->Hide();
        if( pWindow2 ) pWindow2->Hide();
    }
}

TextDirectionListBox::~TextDirectionListBox()
{
}

} //namespace chart

