/*************************************************************************
 *
 *  $RCSfile: t_digest.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:30 $
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rtl/digest.h>

static const char *digest_in_MD[] =
{
    "",
    "a",
    "abc",
    "message digest",
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
    NULL,
};

static const char *digest_out_MD2[]=
{
    "8350e5a3e24c153df2275c9f80692773",
    "32ec01ec4a6dac72c0ab96fb34c0b5d1",
    "da853b0d3f88d99b30283a69e6ded6bb",
    "ab4f496bfb2a530b219ff33031fe06b0",
    "4e8ddff3650292ab5a4108c3aa47940b",
    "da33def2a42df13975352846c30338cd",
    "d5976f79d83d3a0dc9806c3c66f3efd8",
};

static const char *digest_out_MD5[]=
{
    "d41d8cd98f00b204e9800998ecf8427e",
    "0cc175b9c0f1b6a831c399e269772661",
    "900150983cd24fb0d6963f7d28e17f72",
    "f96b697d7cb7938d525a2f31aaf161d0",
    "c3fcd3d76192e4007dfb496cca67e13b",
    "d174ab98d277d9f5a5611c2c9f419d9f",
    "57edf4a22be3c955ac49da2e2107b67a",
};

static const char *digest_in_SHA[]=
{
    "abc",
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    NULL,
};

static const char *digest_out_SHA_0[]=
{
    "0164b8a914cd2a5e74c4f7ff082c4d97f1edf880",
    "d2516ee1acfa5baf33dfc1c471e438449ef134c8",
};

static const char *digest_out_SHA_1[]=
{
    "a9993e364706816aba3e25717850c26c9cd0d89d",
    "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
};

static const char *digest_bigout_SHA_0=
    "3232affa48628a26653b5aaa44541fd90d690603";

static const char *digest_bigout_SHA_1=
    "34aa973cd4c4daa4f61eeb2bdbad27316534016f";

static char *pt (unsigned char *md, int length)
{
    int i;
    static char buf[80];

    for (i=0; i<length; i++)
        sprintf(&(buf[i*2]),"%02x",md[i]);

    return(buf);
}

#ifdef WIN32
int __cdecl main (int argc, char **argv)
#else
int main (int argc, char **argv)
#endif
{
    const char **P,**R;
    char *p;
    int i=1, err=0;

    unsigned char md[80];
    unsigned char buffer[1000];

    rtlDigest Digest;

    P=digest_in_MD;
    R=digest_out_MD2;
    i = 1;
    while (*P)
    {
        rtl_digest_MD2 (*P, strlen(*P), md, sizeof(md));

        p=pt (md, RTL_DIGEST_LENGTH_MD2);
        if (strcmp (p, *R))
        {
            printf("error calculating MD2 on '%s'\n",*P);
            printf("got %s instead of %s\n",p,*R);
            err++;
        }
        else
            printf("test (MD2) %d ok\n",i);
        i++;
        R++;
        P++;
    }

    P=digest_in_MD;
    R=digest_out_MD5;
    i=1;
    while (*P)
    {
        rtl_digest_MD5 (*P, strlen(*P), md, sizeof(md));

        p=pt (md, RTL_DIGEST_LENGTH_MD5);
        if (strcmp (p, *R))
        {
            printf("error calculating MD5 on '%s'\n",*P);
            printf("got %s instead of %s\n",p,*R);
            err++;
        }
        else
            printf("test (MD5) %d ok\n",i);
        i++;
        R++;
        P++;
    }

    P=digest_in_SHA;
    R=digest_out_SHA_0;
    i=1;
    while (*P)
    {
        rtl_digest_SHA (*P, strlen(*P), md, sizeof(md));

        p=pt (md, RTL_DIGEST_LENGTH_SHA);
        if (strcmp (p, *R))
        {
            printf("error calculating SHA-0 on '%s'\n",*P);
            printf("got %s instead of %s\n",p,*R);
            err++;
        }
        else
            printf("test (SHA-0) %d ok\n",i);
        i++;
        R++;
        P++;
    }

    memset (buffer, 'a', sizeof(buffer));
    R = &digest_bigout_SHA_0;

    Digest = rtl_digest_createSHA();
    for (i=0; i<1000; i++)
        rtl_digest_updateSHA (Digest, buffer, sizeof(buffer));

    rtl_digest_getSHA (Digest, md, sizeof(md));
    rtl_digest_destroySHA (Digest);

    p=pt (md, RTL_DIGEST_LENGTH_SHA);
    if (strcmp (p, *R))
    {
        printf("error calculating SHA-0 on '%s'\n",p);
        printf("got %s instead of %s\n",p,*R);
        err++;
    }
    else
        printf("test (SHA-0) n ok\n");

    P=digest_in_SHA;
    R=digest_out_SHA_1;
    i=1;
    while (*P)
    {
        rtl_digest_SHA1 (*P, strlen(*P), md, sizeof(md));

        p=pt (md, RTL_DIGEST_LENGTH_SHA1);
        if (strcmp (p, *R))
        {
            printf("error calculating SHA-1 on '%s'\n",*P);
            printf("got %s instead of %s\n",p,*R);
            err++;
        }
        else
            printf("test (SHA-1) %d ok\n",i);
        i++;
        R++;
        P++;
    }

    memset (buffer, 'a', sizeof(buffer));
    R = &digest_bigout_SHA_1;

    Digest = rtl_digest_createSHA1();
    for (i=0; i<1000; i++)
        rtl_digest_updateSHA1 (Digest, buffer, sizeof(buffer));

    rtl_digest_getSHA1 (Digest, md, sizeof(md));
    rtl_digest_destroySHA1 (Digest);

    p=pt (md, RTL_DIGEST_LENGTH_SHA1);
    if (strcmp (p, *R))
    {
        printf("error calculating SHA-0 on '%s'\n",p);
        printf("got %s instead of %s\n",p,*R);
        err++;
    }
    else
        printf("test (SHA-1) n ok\n");

    exit(err);
    return(0);
}

