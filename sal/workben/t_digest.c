/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

static const char digest_key_HMAC_MD5_1[] =
{
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x00
};
static const char digest_key_HMAC_MD5_2[] =
{
    /* "Jefe" */
    'J', 'e', 'f', 'e',
    0x00
};
static const unsigned char digest_key_HMAC_MD5_3[] =
{
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0x00
};
static const char *digest_key_HMAC_MD5[] =
{
    (const char*)&digest_key_HMAC_MD5_1,
    (const char*)&digest_key_HMAC_MD5_2, /* "Jefe", */
    (const char*)&digest_key_HMAC_MD5_3,
    NULL
};

static const unsigned char digest_in_HMAC_MD5_3[] =
{
    0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
    0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
    0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
    0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
    0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
    0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
    0xDD, 0xDD,
    0x00
};
static const char *digest_in_HMAC_MD5[] =
{
    "Hi There",
    "what do ya want for nothing?",
    (const char*)&digest_in_HMAC_MD5_3,
    NULL
};

static const char *digest_out_HMAC_MD5[] =
{
    "9294727a3638bb1c13f48ef8158bfc9d",
    "750c783e6ab0b503eaa86e310a5db738",
    "56be34521d144c88dbb8c733f0e8b3f6",
    NULL
};
static const char *digest_out_HMAC_SHA1[] =
{
    /* unofficial, i.e. not verified */
    "675b0b3a1b4ddf4e124872da6c2f632bfed957e9",
    "effcdf6ae5eb2fa2d27416d5f184df9c259a7c79",
    "d730594d167e35d5956fd8003d0db3d3f46dc7bb",
    NULL
};

static char *pt (unsigned char *md, int length)
{
    int i;
    static char buf[80];

    for (i=0; i<length; i++)
        sprintf(&(buf[i*2]),"%02x",md[i]);

    return(buf);
}

int SAL_CALL main (void)
{
    const char **P,**R, **Q;
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
        printf("error calculating SHA-1 on '%s'\n",p);
        printf("got %s instead of %s\n",p,*R);
        err++;
    }
    else
        printf("test (SHA-1) n ok\n");

    P=digest_in_HMAC_MD5;
    Q=digest_key_HMAC_MD5;
    R=digest_out_HMAC_MD5;
    Digest = rtl_digest_createHMAC_MD5();
    i = 1;
    while (*P)
    {
        rtl_digest_initHMAC_MD5 (Digest, (const sal_uInt8*)(*Q), strlen(*Q));
        rtl_digest_updateHMAC_MD5 (Digest, *P, strlen(*P));
        rtl_digest_getHMAC_MD5 (Digest, md, sizeof(md));

        p=pt (md, RTL_DIGEST_LENGTH_HMAC_MD5);
        if (strcmp (p, *R))
        {
            printf("error calculating HMAC-MD5 on '%s'\n",*P);
            printf("got %s instead of %s\n",p,*R);
            err++;
        }
        else
            printf("test (HMAC-MD5) %d ok\n",i);
        i++;
        R++;
        P++;
        Q++;
    }
    rtl_digest_destroyHMAC_MD5 (Digest);

    P=digest_in_HMAC_MD5;
    Q=digest_key_HMAC_MD5;
    R=digest_out_HMAC_SHA1;
    Digest = rtl_digest_createHMAC_SHA1();
    i = 1;
    while (*P)
    {
        rtl_digest_initHMAC_SHA1 (Digest, (const sal_uInt8*)(*Q), strlen(*Q));
        rtl_digest_updateHMAC_SHA1 (Digest, (const sal_uInt8*)(*P), strlen(*P));
        rtl_digest_getHMAC_SHA1 (Digest, md, sizeof(md));

        p=pt (md, RTL_DIGEST_LENGTH_HMAC_SHA1);
        if (strcmp (p, *R))
        {
            printf("error calculating HMAC-SHA-1 on '%s'\n",*P);
            printf("got %s instead of %s\n",p,*R);
            err++;
        }
        else
            printf("test (HMAC-SHA-1) %d ok\n",i);
        i++;
        P++;
        Q++;
        R++;
    }
    rtl_digest_destroyHMAC_SHA1 (Digest);

    P=digest_in_HMAC_MD5;
    Q=digest_key_HMAC_MD5;
    rtl_digest_PBKDF2 (
        md, RTL_DIGEST_LENGTH_MD5, /* [out] derived key     */
        (const sal_uInt8*)(Q[1]), strlen(Q[1]),        /* [in]  password        */
        (const sal_uInt8*)(P[1]), strlen(P[1]),        /* [in]  salt            */
        1000);                     /* [in]  iteration count */

    p=pt (md, RTL_DIGEST_LENGTH_MD5);
    if (strcmp (p, "6349e09cb6b8c1485cfa9780ee3264df"))
    {
        printf("error calculating PBKDF2 on '%s'\n", P[1]);
        err++;
    }
    else
        printf("test (PBKDF2) %d ok\n", 1);

    return (err);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
