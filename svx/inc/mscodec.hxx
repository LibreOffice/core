/*************************************************************************
 *
 *  $RCSfile: mscodec.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 08:54:04 $
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

#ifndef SVX_MSCODEC_HXX
#define SVX_MSCODEC_HXX

#ifndef _RTL_CIPHER_H_
#include "rtl/cipher.h"
#endif
#ifndef _RTL_DIGEST_H_
#include "rtl/digest.h"
#endif

namespace svx {

// ============================================================================

/** Encodes and decodes data from protected MSO 95- documents.
 */
class MSCodec_Xor95
{
public:
    explicit            MSCodec_Xor95(int nRotateDistance);
    virtual            ~MSCodec_Xor95();

    /** Initializes the algorithm with the specified password.

        @param pPassData
            Character array containing the password. Must be zero terminated,
            which results in a maximum length of 15 characters.
     */
    void                InitKey( const sal_uInt8 pnPassData[ 16 ] );

    /** Verifies the validity of the password using the passed key and hash.

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param nKey
            Password key value read from the file.
        @param nHash
            Password hash value read from the file.

        @return
            true = Test was successful.
     */
    bool                VerifyKey( sal_uInt16 nKey, sal_uInt16 nHash ) const;

    /** Reinitializes the codec to start a new memory block.

        Resets the internal key offset to 0.

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.
     */
    void                InitCipher();

    /** Decodes a block of memory inplace.

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param pnData
            Encrypted data block. Will contain the decrypted data afterwards.
        @param nBytes
            Size of the passed data block.
    */
    virtual void                Decode( sal_uInt8* pnData, sal_Size nBytes )=0;

    /** Lets the cipher skip a specific amount of bytes.

        This function sets the cipher to the same state as if the specified
        amount of data has been decoded with one or more calls of Decode().

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param nBytes
            Number of bytes to be skipped (cipher "seeks" forward).
     */
    void                Skip( sal_Size nBytes );

    // static -----------------------------------------------------------------

    /** Calculates the 16-bit hash value for the given password.

        The password data may be longer than 16 bytes. The array does not need
        to be terminated with a NULL byte (but it can without invalidating the
        result).
     */
    static sal_uInt16   GetHash( const sal_uInt8* pnPassData, sal_Size nSize );

protected:
    sal_uInt8           mpnKey[ 16 ];   /// Encryption key.
    sal_Size            mnOffset;       /// Key offset.

private:
                        MSCodec_Xor95( const MSCodec_Xor95& );
    MSCodec_Xor95&      operator=( const MSCodec_Xor95& );

    sal_uInt16          mnKey;          /// Base key from password.
    sal_uInt16          mnHash;         /// Hash value from password.
    int         mnRotateDistance;
};

/** Encodes and decodes data from protected MSO XLS 95- documents.
 */
class MSCodec_XorXLS95 : public MSCodec_Xor95
{
public:
    explicit            MSCodec_XorXLS95() : MSCodec_Xor95(2) {}

    /** Decodes a block of memory inplace.

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param pnData
            Encrypted data block. Will contain the decrypted data afterwards.
        @param nBytes
            Size of the passed data block.
    */
    virtual void                Decode( sal_uInt8* pnData, sal_Size nBytes );
};

/** Encodes and decodes data from protected MSO Word 95- documents.
 */
class MSCodec_XorWord95 : public MSCodec_Xor95
{
public:
    explicit            MSCodec_XorWord95() : MSCodec_Xor95(7) {}

    /** Decodes a block of memory inplace.

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param pnData
            Encrypted data block. Will contain the decrypted data afterwards.
        @param nBytes
            Size of the passed data block.
    */
    virtual void                Decode( sal_uInt8* pnData, sal_Size nBytes );
};


// ============================================================================

/** Encodes and decodes data from protected MSO 97+ documents.

    This is a wrapper class around low level cryptographic functions from RTL.
    Implementation is based on the wvDecrypt package by Caolan McNamara:
    http://www.csn.ul.ie/~caolan/docs/wvDecrypt.html
 */
class MSCodec_Std97
{
public:
    explicit            MSCodec_Std97();
                        ~MSCodec_Std97();

    /** Initializes the algorithm with the specified password and document ID.

        @param pPassData
            Wide character array containing the password. Must be zero
            terminated, which results in a maximum length of 15 characters.
        @param pUnique
            Unique document identifier read from or written to the file.
     */
    void                InitKey(
                            const sal_uInt16 pPassData[ 16 ],
                            const sal_uInt8 pUnique[ 16 ] );

    /** Verifies the validity of the password using the passed salt data.

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param pSaltData
            Salt data block read from the file.
        @param pSaltDigest
            Salt digest read from the file.

        @return
            true = Test was successful.
     */
    bool                VerifyKey(
                            const sal_uInt8 pSaltData[ 16 ],
                            const sal_uInt8 pSaltDigest[ 16 ] );

    /** Rekeys the codec using the specified counter.

        After reading a specific amount of data the cipher algorithm needs to
        be rekeyed using a counter that counts the data blocks.

        The block size is for example 512 Bytes for Word files and 1024 Bytes
        for Excel files.

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param nCounter
            Block counter used to rekey the cipher.
     */
    bool                InitCipher( sal_uInt32 nCounter );

    /** Decodes a block of memory.

        @see rtl_cipher_decode()

        @precond
            The codec must be initialized with InitKey() before this function
            can be used. The destination buffer must be able to take all
            encoded data from the source buffer (usually this means it must be
            as long as or longer than the source buffer).

        @param pData
            Encrypted source data block.
        @param nDatLen
            Size of the passed source data block.
        @param pBuffer
            Destination buffer for the decrypted data.
        @param nBufLen
            Size of the destination buffer.

        @return
            true = Decoding was successful (no error occured).
    */
    bool                Decode(
                            const void* pData, sal_Size nDatLen,
                            sal_uInt8* pBuffer, sal_Size nBufLen );

    /** Lets the cipher skip a specific amount of bytes.

        This function sets the cipher to the same state as if the specified
        amount of data has been decoded with one or more calls of Decode().

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param nDatLen
            Number of bytes to be skipped (cipher "seeks" forward).
     */
    bool                Skip( sal_Size nDatLen );

private:
                        MSCodec_Std97( const MSCodec_Std97& );
    MSCodec_Std97&      operator=( const MSCodec_Std97& );

    rtlCipher           m_hCipher;
    rtlDigest           m_hDigest;
    sal_uInt8           m_pDigestValue[ RTL_DIGEST_LENGTH_MD5 ];
};

// ============================================================================

} // namespace svx

#endif

