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
#include "precompiled_tools.hxx"

#include <tools/isofallback.hxx>

// -----------------------------------------------------------------------

// Return true if valid fallback found
sal_Bool GetIsoFallback( ByteString& rLanguage )
{
    rLanguage.EraseLeadingAndTrailingChars();
    if( rLanguage.Len() ){
        xub_StrLen nSepPos = rLanguage.Search( '-' );
        if ( nSepPos == STRING_NOTFOUND ){
            if ( rLanguage.Equals("en"))
            {
                // en -> ""
                rLanguage.Erase();
                return false;
            }
            else
            {
                // de -> en-US ;
                rLanguage = ByteString("en-US");
                return true;
            }
        }
        else if( !( nSepPos == 1 && ( rLanguage.GetChar(0) == 'x' || rLanguage.GetChar(0) == 'X' ) ) )
        {
            // de-CH -> de ;
            // try erase from -
            rLanguage = rLanguage.GetToken( 0, '-');
            return true;
        }
    }
    // "" -> ""; x-no-translate -> ""
    rLanguage.Erase();
    return false;
}

