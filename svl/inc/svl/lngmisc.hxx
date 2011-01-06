/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
