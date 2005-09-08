/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lngmisc.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:54:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVTOOLS_LNGMISC_HXX_
#define _SVTOOLS_LNGMISC_HXX_

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

///////////////////////////////////////////////////////////////////////////

#define SVT_SOFT_HYPHEN ((sal_Unicode) 0x00AD)
#define SVT_HARD_HYPHEN ((sal_Unicode) 0x2011)

// the non-breaking space
#define SVT_HARD_SPACE  ((sal_Unicode) 0x00A0)

namespace linguistic
{

inline BOOL IsHyphen( sal_Unicode cChar )
{
    return cChar == SVT_SOFT_HYPHEN  ||  cChar == SVT_HARD_HYPHEN;
}


inline BOOL IsControlChar( sal_Unicode cChar )
{
    return cChar < (sal_Unicode) ' ';
}


inline BOOL HasHyphens( const rtl::OUString &rTxt )
{
    return  rTxt.indexOf( SVT_SOFT_HYPHEN ) != -1  ||
            rTxt.indexOf( SVT_HARD_HYPHEN ) != -1;
}

SVL_DLLPUBLIC INT32 GetNumControlChars( const rtl::OUString &rTxt );
SVL_DLLPUBLIC BOOL  RemoveHyphens( rtl::OUString &rTxt );
SVL_DLLPUBLIC BOOL  RemoveControlChars( rtl::OUString &rTxt );

SVL_DLLPUBLIC BOOL  ReplaceControlChars( rtl::OUString &rTxt, sal_Char aRplcChar = ' ' );

} // namespace linguistic

#endif
