/*************************************************************************
 *
 *  $RCSfile: stringhash.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-10-22 08:10:02 $
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
 *  Contributor(s): Martin Gallwey ( gallwey@sun.com )
 *
 *
 ************************************************************************/
#ifndef _STRINGHASH_HXX
#define _STRINGHASH_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

struct StringEq
{
    sal_Bool operator() ( const String *r1,
                            const String *r2) const
    {
        return r1->Equals(*r2);
    }
};

struct StringEqRef
{
    sal_Bool operator() (const String &r1, const String &r2) const
    {
        return r1.Equals(r2);
    }
};

struct StringHash
{
    size_t operator() ( const String *rString) const
    {
        sal_Int32 h, nLen;
        h = nLen = rString->Len();
        const sal_Unicode *pStr = rString->GetBuffer();

        if ( nLen < 16 )
            while ( nLen-- > 0 )
                h = (h*37) + *(pStr++);
        else
        {
            sal_Int32               nSkip;
            const sal_Unicode* pEndStr = pStr+nLen-5;

            /* only sample some characters */
            /* the first 3, some characters between, and the last 5 */
            h = (h*39) + *(pStr++);
            h = (h*39) + *(pStr++);
            h = (h*39) + *(pStr++);

            nSkip = nLen / nLen < 32 ? 4 : 8;
            nLen -= 8;
            while ( nLen > 0 )
            {
                h = (h*39) + ( *pStr );
                pStr += nSkip;
                nLen -= nSkip;
            }

            h = (h*39) + *(pEndStr++);
            h = (h*39) + *(pEndStr++);
            h = (h*39) + *(pEndStr++);
            h = (h*39) + *(pEndStr++);
            h = (h*39) + *(pEndStr++);
        }
        return h;
    }

    size_t operator() (const String & rStr) const
    {
        return (*this)(&rStr);
    }
};

struct StringHashRef
{
    size_t operator () (const String &rStr) const
    {
        StringHash aStrHash;

        return aStrHash(&rStr);
    }
};
#endif // _STRINGHASH_HXX
