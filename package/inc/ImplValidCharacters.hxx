/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImplValidCharacters.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _IMPL_VALID_CHARACTERS_HXX_
#define _IMPL_VALID_CHARACTERS_HXX_

#include <sal/types.h>

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
