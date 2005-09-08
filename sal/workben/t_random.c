/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: t_random.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:53:22 $
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rtl/random.h>

static char *pt (unsigned char *md, int length)
{
    int i;
    static char buf[80];

    for (i=0; i<length; i++)
        sprintf(&(buf[i*2]),"%02x",md[i]);

    return(buf);
}

/*
 * main.
 */
#ifdef WIN32
int __cdecl main (int argc, char **argv)
#else
int main (int argc, char **argv)
#endif
{
    rtlRandomPool pool;
    pool = rtl_random_createPool();
    if (pool)
    {
        unsigned char buffer[1000];

        rtl_random_getBytes (pool, buffer, 8);
        printf ("random: %s\n", pt (buffer, 8));
    }
    rtl_random_destroyPool (pool);
    return(0);
}

