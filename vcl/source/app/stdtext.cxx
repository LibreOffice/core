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

#include <vcl/msgbox.hxx>
#include <vcl/stdtext.hxx>

#include <svids.hrc>
#include <svdata.hxx>


// =======================================================================

XubString GetStandardText( sal_uInt16 nStdText )
{
    ResMgr* pResMgr = ImplGetResMgr();
    XubString aText;
    if( pResMgr )
        aText = XubString( ResId( nStdText-STANDARD_TEXT_FIRST+SV_STDTEXT_FIRST, *pResMgr ) );
    return aText;
}

// =======================================================================

void ShowServiceNotAvailableError( Window* pParent,
                                   const XubString& rServiceName, sal_Bool bError )
{
    XubString aText( GetStandardText( STANDARD_TEXT_SERVICE_NOT_AVAILABLE ) );
    aText.SearchAndReplaceAscii( "%s", rServiceName );
    if ( bError )
    {
        ErrorBox aBox( pParent, WB_OK | WB_DEF_OK, aText );
        aBox.Execute();
    }
    else
    {
        WarningBox aBox( pParent, WB_OK | WB_DEF_OK, aText );
        aBox.Execute();
    }
}
