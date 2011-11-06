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



#ifndef _SVTOOLS_LNGMISC_HXX_
#define _SVTOOLS_LNGMISC_HXX_

#include "svl/svldllapi.h"
#include <tools/solar.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <tools/string.hxx>

///////////////////////////////////////////////////////////////////////////

#define SVT_SOFT_HYPHEN ((sal_Unicode) 0x00AD)
#define SVT_HARD_HYPHEN ((sal_Unicode) 0x2011)

// the non-breaking space
#define SVT_HARD_SPACE  ((sal_Unicode) 0x00A0)

namespace linguistic
{

inline sal_Bool IsHyphen( sal_Unicode cChar )
{
    return cChar == SVT_SOFT_HYPHEN  ||  cChar == SVT_HARD_HYPHEN;
}


inline sal_Bool IsControlChar( sal_Unicode cChar )
{
    return cChar < (sal_Unicode) ' ';
}


inline sal_Bool HasHyphens( const rtl::OUString &rTxt )
{
    return  rTxt.indexOf( SVT_SOFT_HYPHEN ) != -1  ||
            rTxt.indexOf( SVT_HARD_HYPHEN ) != -1;
}

SVL_DLLPUBLIC sal_Int32 GetNumControlChars( const rtl::OUString &rTxt );
SVL_DLLPUBLIC sal_Bool  RemoveHyphens( rtl::OUString &rTxt );
SVL_DLLPUBLIC sal_Bool  RemoveControlChars( rtl::OUString &rTxt );

SVL_DLLPUBLIC sal_Bool  ReplaceControlChars( rtl::OUString &rTxt, sal_Char aRplcChar = ' ' );

SVL_DLLPUBLIC String GetThesaurusReplaceText( const String &rText );

} // namespace linguistic

#endif
