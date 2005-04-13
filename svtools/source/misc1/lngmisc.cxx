/*************************************************************************
 *
 *  $RCSfile: lngmisc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:28:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVTOOLS_LNGMISC_HXX_
#include <lngmisc.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif


using namespace rtl;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

INT32 GetNumControlChars( const OUString &rTxt )
{
    INT32 nCnt = 0;
    INT32 nLen = rTxt.getLength();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        if (IsControlChar( rTxt[i] ))
            ++nCnt;
    }
    return nCnt;
}


BOOL RemoveHyphens( OUString &rTxt )
{
    BOOL bModified = FALSE;
    if (HasHyphens( rTxt ))
    {
        String aTmp( rTxt );
        aTmp.EraseAllChars( SVT_SOFT_HYPHEN );
        aTmp.EraseAllChars( SVT_HARD_HYPHEN );
        rTxt = aTmp;
        bModified = TRUE;
    }
    return bModified;
}


BOOL RemoveControlChars( OUString &rTxt )
{
    BOOL bModified = FALSE;
    INT32 nCtrlChars = GetNumControlChars( rTxt );
    if (nCtrlChars)
    {
        INT32 nLen  = rTxt.getLength();
        INT32 nSize = nLen - nCtrlChars;
        OUStringBuffer aBuf( nSize );
        aBuf.setLength( nSize );
        INT32 nCnt = 0;
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            sal_Unicode cChar = rTxt[i];
            if (!IsControlChar( cChar ))
            {
                DBG_ASSERT( nCnt < nSize, "index out of range" );
                aBuf.setCharAt( nCnt++, cChar );
            }
        }
        DBG_ASSERT( nCnt == nSize, "wrong size" );
        rTxt = aBuf.makeStringAndClear();
        bModified = TRUE;
    }
    return bModified;
}


// non breaking field character
#define CH_TXTATR_INWORD    ((sal_Char) 0x02)

BOOL ReplaceControlChars( rtl::OUString &rTxt, sal_Char aRplcChar )
{
    // the resulting string looks like this:
    // 1. non breaking field characters get removed
    // 2. remaining control characters will be replaced by ' '

    BOOL bModified = FALSE;
    INT32 nCtrlChars = GetNumControlChars( rTxt );
    if (nCtrlChars)
    {
        INT32 nLen  = rTxt.getLength();
        OUStringBuffer aBuf( nLen );
        INT32 nCnt = 0;
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            sal_Unicode cChar = rTxt[i];
            if (CH_TXTATR_INWORD != cChar)
            {
                if (IsControlChar( cChar ))
                    cChar = ' ';
                DBG_ASSERT( nCnt < nLen, "index out of range" );
                aBuf.setCharAt( nCnt++, cChar );
            }
        }
        aBuf.setLength( nCnt );
        rTxt = aBuf.makeStringAndClear();
        bModified = TRUE;
    }
    return bModified;
}

///////////////////////////////////////////////////////////////////////////

} // namespace linguistic

