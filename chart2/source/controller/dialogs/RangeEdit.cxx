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

#include "RangeEdit.hxx"

namespace chart
{

//============================================================================
// class RangeEdit
//----------------------------------------------------------------------------
RangeEdit::RangeEdit( Window* pParent, const ResId& rResId):
    Edit( pParent , rResId )
{
    EnableUpdateData( 1000 );
}

RangeEdit::~RangeEdit()
{
}

void RangeEdit::SetKeyInputHdl( const Link& rKeyInputLink )
{
    m_aKeyInputLink = rKeyInputLink;
}

void RangeEdit::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if( !rKeyCode.GetModifier() && rKeyCode.GetCode() == KEY_F2 )
    {
        if ( m_aKeyInputLink.IsSet() )
        {
            m_aKeyInputLink.Call( NULL );
        }
    }
    else
       Edit::KeyInput( rKEvt );
}

//.............................................................................
} //namespace chart
//.............................................................................
