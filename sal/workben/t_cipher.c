/*************************************************************************
 *
 *  $RCSfile: t_cipher.c,v $
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
#include <sal/types.h>
#include <osl/diagnose.h>
#include <rtl/cipher.h>

#include <stdio.h>
#include <string.h>

#define NUM_VARIABLE_KEY_TESTS  34
#define NUM_SET_KEY_TESTS       24

/* plaintext bytes -- left halves */
unsigned long plaintext_l[NUM_VARIABLE_KEY_TESTS + NUM_SET_KEY_TESTS] = {
   0x00000000l, 0xFFFFFFFFl, 0x10000000l, 0x11111111l, 0x11111111l,
   0x01234567l, 0x00000000l, 0x01234567l, 0x01A1D6D0l, 0x5CD54CA8l,
   0x0248D438l, 0x51454B58l, 0x42FD4430l, 0x059B5E08l, 0x0756D8E0l,
   0x762514B8l, 0x3BDD1190l, 0x26955F68l, 0x164D5E40l, 0x6B056E18l,
   0x004BD6EFl, 0x480D3900l, 0x437540C8l, 0x072D43A0l, 0x02FE5577l,
   0x1D9D5C50l, 0x30553228l, 0x01234567l, 0x01234567l, 0x01234567l,
   0xFFFFFFFFl, 0x00000000l, 0x00000000l, 0xFFFFFFFFl, 0xFEDCBA98l,
   0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l,
   0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l,
   0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l,
   0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l,
   0xFEDCBA98l, 0xFEDCBA98l, 0xFEDCBA98l };

/* plaintext bytes -- right halves */
unsigned long plaintext_r[NUM_VARIABLE_KEY_TESTS + NUM_SET_KEY_TESTS] = {
   0x00000000l, 0xFFFFFFFFl, 0x00000001l, 0x11111111l, 0x11111111l,
   0x89ABCDEFl, 0x00000000l, 0x89ABCDEFl, 0x39776742l, 0x3DEF57DAl,
   0x06F67172l, 0x2DDF440Al, 0x59577FA2l, 0x51CF143Al, 0x774761D2l,
   0x29BF486Al, 0x49372802l, 0x35AF609Al, 0x4F275232l, 0x759F5CCAl,
   0x09176062l, 0x6EE762F2l, 0x698F3CFAl, 0x77075292l, 0x8117F12Al,
   0x18F728C2l, 0x6D6F295Al, 0x89ABCDEFl, 0x89ABCDEFl, 0x89ABCDEFl,
   0xFFFFFFFFl, 0x00000000l, 0x00000000l, 0xFFFFFFFFl, 0x76543210l,
   0x76543210l, 0x76543210l, 0x76543210l, 0x76543210l, 0x76543210l,
   0x76543210l, 0x76543210l, 0x76543210l, 0x76543210l, 0x76543210l,
   0x76543210l, 0x76543210l, 0x76543210l, 0x76543210l, 0x76543210l,
   0x76543210l, 0x76543210l, 0x76543210l, 0x76543210l, 0x76543210l,
   0x76543210l, 0x76543210l, 0x76543210l };

/* key bytes for variable key tests */
unsigned char variable_key[NUM_VARIABLE_KEY_TESTS][8] = {
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
   { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
   { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
   { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 },
   { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF },
   { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 },
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
   { 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 },
   { 0x7C, 0xA1, 0x10, 0x45, 0x4A, 0x1A, 0x6E, 0x57 },
   { 0x01, 0x31, 0xD9, 0x61, 0x9D, 0xC1, 0x37, 0x6E },
   { 0x07, 0xA1, 0x13, 0x3E, 0x4A, 0x0B, 0x26, 0x86 },
   { 0x38, 0x49, 0x67, 0x4C, 0x26, 0x02, 0x31, 0x9E },
   { 0x04, 0xB9, 0x15, 0xBA, 0x43, 0xFE, 0xB5, 0xB6 },
   { 0x01, 0x13, 0xB9, 0x70, 0xFD, 0x34, 0xF2, 0xCE },
   { 0x01, 0x70, 0xF1, 0x75, 0x46, 0x8F, 0xB5, 0xE6 },
   { 0x43, 0x29, 0x7F, 0xAD, 0x38, 0xE3, 0x73, 0xFE },
   { 0x07, 0xA7, 0x13, 0x70, 0x45, 0xDA, 0x2A, 0x16 },
   { 0x04, 0x68, 0x91, 0x04, 0xC2, 0xFD, 0x3B, 0x2F },
   { 0x37, 0xD0, 0x6B, 0xB5, 0x16, 0xCB, 0x75, 0x46 },
   { 0x1F, 0x08, 0x26, 0x0D, 0x1A, 0xC2, 0x46, 0x5E },
   { 0x58, 0x40, 0x23, 0x64, 0x1A, 0xBA, 0x61, 0x76 },
   { 0x02, 0x58, 0x16, 0x16, 0x46, 0x29, 0xB0, 0x07 },
   { 0x49, 0x79, 0x3E, 0xBC, 0x79, 0xB3, 0x25, 0x8F },
   { 0x4F, 0xB0, 0x5E, 0x15, 0x15, 0xAB, 0x73, 0xA7 },
   { 0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF },
   { 0x01, 0x83, 0x10, 0xDC, 0x40, 0x9B, 0x26, 0xD6 },
   { 0x1C, 0x58, 0x7F, 0x1C, 0x13, 0x92, 0x4F, 0xEF },
   { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
   { 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E },
   { 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE },
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
   { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
   { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF },
   { 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 }};

/* key bytes for set key tests */
unsigned char set_key[24] = {
   0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0x96, 0x87,
   0x78, 0x69, 0x5A, 0x4B, 0x3C, 0x2D, 0x1E, 0x0F,
   0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };

/* ciphertext bytes -- left halves */
unsigned long ciphertext_l[NUM_VARIABLE_KEY_TESTS + NUM_SET_KEY_TESTS] = {
   0x4EF99745l, 0x51866FD5l, 0x7D856F9Al, 0x2466DD87l, 0x61F9C380l,
   0x7D0CC630l, 0x4EF99745l, 0x0ACEAB0Fl, 0x59C68245l, 0xB1B8CC0Bl,
   0x1730E577l, 0xA25E7856l, 0x353882B1l, 0x48F4D088l, 0x432193B7l,
   0x13F04154l, 0x2EEDDA93l, 0xD887E039l, 0x5F99D04Fl, 0x4A057A3Bl,
   0x452031C1l, 0x7555AE39l, 0x53C55F9Cl, 0x7A8E7BFAl, 0xCF9C5D7Al,
   0xD1ABB290l, 0x55CB3774l, 0xFA34EC48l, 0xA7907951l, 0xC39E072Dl,
   0x014933E0l, 0xF21E9A77l, 0x24594688l, 0x6B5C5A9Cl, 0xF9AD597Cl,
   0xE91D21C1l, 0xE9C2B70Al, 0xBE1E6394l, 0xB39E4448l, 0x9457AA83l,
   0x8BB77032l, 0xE87A244El, 0x15750E7Al, 0x122BA70Bl, 0x3A833C9Al,
   0x9409DA87l, 0x884F8062l, 0x1F85031Cl, 0x79D9373Al, 0x93142887l,
   0x03429E83l, 0xA4299E27l, 0xAFD5AED1l, 0x10851C0El, 0xE6F51ED7l,
   0x64A6E14Al, 0x80C7D7D4l, 0x05044B62l };

/* ciphertext bytes -- right halves */
unsigned long ciphertext_r[NUM_VARIABLE_KEY_TESTS + NUM_SET_KEY_TESTS] = {
   0x6198DD78l, 0xB85ECB8Al, 0x613063F2l, 0x8B963C9Dl, 0x2281B096l,
   0xAFDA1EC7l, 0x6198DD78l, 0xC6A0A28Dl, 0xEB05282Bl, 0x250F09A0l,
   0x8BEA1DA4l, 0xCF2651EBl, 0x09CE8F1Al, 0x4C379918l, 0x8951FC98l,
   0xD69D1AE5l, 0xFFD39C79l, 0x3C2DA6E3l, 0x5B163969l, 0x24D3977Bl,
   0xE4FADA8El, 0xF59B87BDl, 0xB49FC019l, 0x937E89A3l, 0x4986ADB5l,
   0x658BC778l, 0xD13EF201l, 0x47B268B2l, 0x08EA3CAEl, 0x9FAC631Dl,
   0xCDAFF6E4l, 0xB71C49BCl, 0x5754369Al, 0x5D9E0A5Al, 0x49DB005El,
   0xD961A6D6l, 0x1BC65CF3l, 0x08640F05l, 0x1BDB1E6El, 0xB1928C0Dl,
   0xF960629Dl, 0x2CC85E82l, 0x4F4EC577l, 0x3AB64AE0l, 0xFFC537F6l,
   0xA90F6BF2l, 0x5060B8B4l, 0x19E11968l, 0x714CA34Fl, 0xEE3BE15Cl,
   0x8CE2D14Bl, 0x469FF67Bl, 0xC1BC96A8l, 0x3858DA9Fl, 0x9B9DB21Fl,
   0xFD36B46Fl, 0x5A5479ADl, 0xFA52D080l };


static sal_uInt8 cbc_key[16] =
{
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87
};
static sal_uInt8 cbc_iv[8] =
{
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
};

static sal_Char cbc_data[40] = "7654321 Now is the time for ";

static sal_uInt8 ecb_ok[32] =
{
    0x2A, 0xFD, 0x7D, 0xAA, 0x60, 0x62, 0x6B, 0xA3,
    0x86, 0x16, 0x46, 0x8C, 0xC2, 0x9C, 0xF6, 0xE1,
    0x29, 0x1E, 0x81, 0x7C, 0xC7, 0x40, 0x98, 0x2D,
    0x6F, 0x87, 0xAC, 0x5F, 0x17, 0x1A, 0xAB, 0xEA
};
static sal_uInt8 cbc_ok[32] =
{
    0x6B, 0x77, 0xB4, 0xD6, 0x30, 0x06, 0xDE, 0xE6,
    0x05, 0xB1, 0x56, 0xE2, 0x74, 0x03, 0x97, 0x93,
    0x58, 0xDE, 0xB9, 0xE7, 0x15, 0x46, 0x16, 0xD9,
    0x59, 0xF1, 0x65, 0x2B, 0xD5, 0xFF, 0x92, 0xCC
};
static sal_uInt8 cfb_ok[] =
{
    0xE7, 0x32, 0x14, 0xA2, 0x82, 0x21, 0x39, 0xCA,
    0xF2, 0x6E, 0xCF, 0x6D, 0x2E, 0xB9, 0xE7, 0x6E,
    0x3D, 0xA3, 0xDE, 0x04, 0xD1, 0x51, 0x72, 0x00,
    0x51, 0x9D, 0x57, 0xA6, 0xC3
};

int SAL_CALL main (int argc, char *argv)
{
    rtlCipher cipher;

    /* ECB */
    cipher = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeECB);
    if (cipher)
    {
        rtlCipherError result;
        sal_uInt8      ecb_in[40], ecb_out[40];
        sal_uInt32     length = strlen(cbc_data) + 1;

        result = rtl_cipher_init (
            cipher, rtl_Cipher_DirectionBoth,
            cbc_key, sizeof(cbc_key), NULL, 0);
        OSL_ASSERT(result == rtl_Cipher_E_None);

        memset (ecb_out, 0, sizeof(ecb_out));
        result = rtl_cipher_encode (
            cipher, cbc_data, length, ecb_out, sizeof(ecb_out));
        OSL_ASSERT(result == rtl_Cipher_E_None);
        OSL_ASSERT(memcmp (ecb_out, ecb_ok, sizeof(ecb_ok)) == 0);

        memset (ecb_in,  0, sizeof(ecb_in));
        result = rtl_cipher_decode (
            cipher, ecb_out, length, ecb_in, sizeof(ecb_in));
        OSL_ASSERT(result == rtl_Cipher_E_None);
        OSL_ASSERT(memcmp (ecb_in, cbc_data, length) == 0);

        rtl_cipher_destroy (cipher);
    }

    /* CBC */
    cipher = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeCBC);
    if (cipher)
    {
        rtlCipherError result;
        sal_uInt8      cbc_in[40], cbc_out[40];
        sal_uInt32     length = strlen(cbc_data) + 1;

        result = rtl_cipher_init (
            cipher, rtl_Cipher_DirectionEncode,
            cbc_key, sizeof(cbc_key), cbc_iv, sizeof(cbc_iv));
        OSL_ASSERT(result == rtl_Cipher_E_None);

        memset (cbc_out, 0, sizeof(cbc_out));
        result = rtl_cipher_encode (
            cipher, cbc_data, length, cbc_out, sizeof(cbc_out));
        OSL_ASSERT(result == rtl_Cipher_E_None);
        OSL_ASSERT(memcmp (cbc_out, cbc_ok, sizeof(cbc_ok)) == 0);

        result = rtl_cipher_init (
            cipher, rtl_Cipher_DirectionDecode,
            cbc_key, sizeof(cbc_key), cbc_iv, sizeof(cbc_iv));
        OSL_ASSERT(result == rtl_Cipher_E_None);

        memset (cbc_in,  0, sizeof(cbc_in));
        result = rtl_cipher_decode (
            cipher, cbc_out, length, cbc_in, sizeof(cbc_in));
        OSL_ASSERT(result == rtl_Cipher_E_None);
        OSL_ASSERT(memcmp (cbc_in, cbc_data, length) == 0);

        rtl_cipher_destroy (cipher);
     }

    /* CFB */
    cipher = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream);
    if (cipher)
    {
        rtlCipherError result;
        sal_uInt8      cfb_in[40], cfb_out[40];
        sal_uInt32     length = strlen(cbc_data) + 1;

        result = rtl_cipher_init (
            cipher, rtl_Cipher_DirectionEncode,
            cbc_key, sizeof(cbc_key), cbc_iv, sizeof(cbc_iv));
        OSL_ASSERT(result == rtl_Cipher_E_None);

        memset (cfb_out, 0, sizeof(cfb_out));
        result = rtl_cipher_encode (
            cipher, cbc_data, length, cfb_out, sizeof(cfb_out));
        OSL_ASSERT(result == rtl_Cipher_E_None);
        OSL_ASSERT(memcmp (cfb_out, cfb_ok, sizeof(cfb_ok)) == 0);

        result = rtl_cipher_init (
            cipher, rtl_Cipher_DirectionDecode,
            cbc_key, sizeof(cbc_key), cbc_iv, sizeof(cbc_iv));
        OSL_ASSERT(result == rtl_Cipher_E_None);

        memset (cfb_in,  0, sizeof(cfb_in));
        result = rtl_cipher_decode (
            cipher, cfb_out, length, cfb_in, sizeof(cfb_in));
        OSL_ASSERT(result == rtl_Cipher_E_None);
        OSL_ASSERT(memcmp (cfb_in, cbc_data, length) == 0);

        rtl_cipher_destroy (cipher);
    }

    /* Done */
    return 0;
}
