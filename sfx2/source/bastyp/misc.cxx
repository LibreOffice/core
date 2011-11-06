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

#include "sfxtypes.hxx"

//--------------------------------------------------------------------

#if defined(DBG_UTIL) && ( defined(WNT) || defined(OS2) )
unsigned SfxStack::nLevel = 0;
#endif

//--------------------------------------------------------------------

String SearchAndReplace( const String &rSource,
                         const String &rToReplace,
                         const String &rReplacement )
{
    String aTarget( rSource );
    sal_uInt16 nPos = rSource.Search( rToReplace );
    if ( nPos != STRING_NOTFOUND )
    {
        aTarget.Erase( nPos, rToReplace.Len() );
        return aTarget.Insert( rReplacement, nPos );
    }
    return rSource;
}

//--------------------------------------------------------------------

String SfxStringEncode( const String &rSource, const char *  )
{
    String aRet;
    String aCoded(rSource);
    return aRet;
}

//--------------------------------------------------------------------

String SfxStringDecode( const String &, const char *  )
{
    String aRet;
    return aRet;
}


