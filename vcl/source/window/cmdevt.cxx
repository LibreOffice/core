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

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _SV_CMDEVT_HXX
#include <vcl/cmdevt.hxx>
#endif

// =======================================================================

CommandExtTextInputData::CommandExtTextInputData()
{
    mpTextAttr      = NULL;
    mnCursorPos     = 0;
    mnDeltaStart    = 0;
    mnOldTextLen    = 0;
    mnCursorFlags   = 0;
    mbOnlyCursor    = sal_False;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::CommandExtTextInputData( const XubString& rText,
                                                  const sal_uInt16* pTextAttr,
                                                  xub_StrLen nCursorPos,
                                                  sal_uInt16 nCursorFlags,
                                                  xub_StrLen nDeltaStart,
                                                  xub_StrLen nOldTextLen,
                                                  sal_Bool bOnlyCursor ) :
    maText( rText )
{
    if ( pTextAttr && maText.Len() )
    {
        mpTextAttr = new sal_uInt16[maText.Len()];
        memcpy( mpTextAttr, pTextAttr, maText.Len()*sizeof(sal_uInt16) );
    }
    else
        mpTextAttr = NULL;
    mnCursorPos     = nCursorPos;
    mnDeltaStart    = nDeltaStart;
    mnOldTextLen    = nOldTextLen;
    mnCursorFlags   = nCursorFlags;
    mbOnlyCursor    = bOnlyCursor;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::CommandExtTextInputData( const CommandExtTextInputData& rData ) :
    maText( rData.maText )
{
    if ( rData.mpTextAttr && maText.Len() )
    {
        mpTextAttr = new sal_uInt16[maText.Len()];
        memcpy( mpTextAttr, rData.mpTextAttr, maText.Len()*sizeof(sal_uInt16) );
    }
    else
        mpTextAttr = NULL;
    mnCursorPos     = rData.mnCursorPos;
    mnDeltaStart    = rData.mnDeltaStart;
    mnOldTextLen    = rData.mnOldTextLen;
    mnCursorFlags   = rData.mnCursorFlags;
    mbOnlyCursor    = rData.mbOnlyCursor;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::~CommandExtTextInputData()
{
    if ( mpTextAttr )
        delete [] mpTextAttr;
}
