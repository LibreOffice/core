/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringhash.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:10:34 $
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
