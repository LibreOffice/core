/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strimp.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:05:10 $
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

#include "strimp.h"

sal_Int16 rtl_ImplGetDigit( sal_Unicode ch, sal_Int16 nRadix )
{
    sal_Int16 n = -1;
    if ( (ch >= '0') && (ch <= '9') )
        n = ch-'0';
    else if ( (ch >= 'a') && (ch <= 'z') )
        n = ch-'a'+10;
    else if ( (ch >= 'A') && (ch <= 'Z') )
        n = ch-'A'+10;
    return (n < nRadix) ? n : -1;
}

sal_Bool rtl_ImplIsWhitespace( sal_Unicode c )
{
    /* Space or Control character? */
    if ( (c <= 32) && c )
        return sal_True;

    /* Only in the General Punctuation area Space or Control characters are included? */
    if ( (c < 0x2000) || (c > 0x206F) )
        return sal_False;

    if ( ((c >= 0x2000) && (c <= 0x200B)) ||    /* All Spaces           */
         (c == 0x2028) ||                       /* LINE SEPARATOR       */
         (c == 0x2029) )                        /* PARAGRAPH SEPARATOR  */
        return sal_True;

    return sal_False;
}
