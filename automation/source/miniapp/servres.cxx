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
#include "precompiled_automation.hxx"
#include <string.h>

#include "servres.hrc"
#include "servuid.hxx"
#include "servres.hxx"


ModalDialogGROSSER_TEST_DLG::ModalDialogGROSSER_TEST_DLG( Window * pParent, const ResId & rResId, sal_Bool bFreeRes )
    : ModalDialog( pParent, rResId ),
    aCheckBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aTriStateBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aOKButton1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aTimeField1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aMultiLineEdit1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aGroupBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aRadioButton1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aRadioButton2( this, ResId( 2, *rResId.GetResMgr() ) ),
    aMultiListBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aComboBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aDateBox1( this, ResId( 1, *rResId.GetResMgr() ) ),
    aFixedText1( this, ResId( 1, *rResId.GetResMgr() ) )
{
    if( bFreeRes ) FreeResource();
}

MenuMENU_CLIENT::MenuMENU_CLIENT( const ResId & rResId, sal_Bool )
    : MenuBar( rResId )
{
    // No subresources, automatic free resource
}

