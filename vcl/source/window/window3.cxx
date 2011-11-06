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
#include "precompiled_vcl.hxx"

#include "vcl/window.hxx"
#include "vcl/waitobj.hxx"
#include "vcl/button.hxx"

// -----------------------------------------------------------------------

WaitObject::~WaitObject()
{
    if ( mpWindow )
        mpWindow->LeaveWait();
}

// -----------------------------------------------------------------------

Size Window::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return Size();
    case WINDOWSIZE_PREFERRED:
        return GetOptimalSize( WINDOWSIZE_MINIMUM );
    case WINDOWSIZE_MAXIMUM:
    default:
        return Size( LONG_MAX, LONG_MAX );
    }
}

// -----------------------------------------------------------------------

void Window::ImplAdjustNWFSizes()
{
    switch( GetType() )
    {
    case WINDOW_CHECKBOX:
        ((CheckBox*)this)->ImplSetMinimumNWFSize();
        break;
    case WINDOW_RADIOBUTTON:
        ((RadioButton*)this)->ImplSetMinimumNWFSize();
        break;
    default:
        {
            // iterate over children
            Window* pWin = GetWindow( WINDOW_FIRSTCHILD );
            while( pWin )
            {
                pWin->ImplAdjustNWFSizes();
                pWin = pWin->GetWindow( WINDOW_NEXT );
            }
        }
        break;
    }
}
