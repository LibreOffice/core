/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImplValidCharacters.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:09:57 $
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
#ifndef _IMPL_VALID_CHARACTERS_HXX_
#define _IMPL_VALID_CHARACTERS_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

static sal_Bool Impl_IsValidChar ( const sal_Unicode *pChar, sal_Int16 nLength, sal_Bool bSlashAllowed )
{
    for ( sal_Int16 i = 0 ; i < nLength ; i++ )
    {
        switch ( pChar[i] )
        {
            case '\\':
            case '?':
            case '<':
            case '>':
            case '\"':
            case '|':
            case ':':
                return sal_False;
            case '/':
                if ( !bSlashAllowed )
                    return sal_False;
            break;
            default:
                if ( pChar[i] < 32  || pChar[i] > 127 )
                    return sal_False;
        }
    }
    return sal_True;
}
#endif
