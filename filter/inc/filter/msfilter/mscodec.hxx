/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SVX_MSCODEC_HXX
#define SVX_MSCODEC_HXX

#include "rtl/cipher.h"
#include "rtl/digest.h"
#include "filter/msfilter/msfilterdllapi.h"

namespace msfilter {

// ============================================================================

/** Encodes and decodes data from protected MSO 95- documents.
 */
class MSFILTER_DLLPUBLIC MSCodec_Xor95
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
                        MSFILTER_DLLPRIVATE MSCodec_Xor95( const MSCodec_Xor95& );
    MSFILTER_DLLPRIVATE MSCodec_Xor95&      operator=( const MSCodec_Xor95& );

    sal_uInt16          mnKey;          /// Base key from password.
    sal_uInt16          mnHash;         /// Hash value from password.
    int 		mnRotateDistance;
};

/** Encodes and decodes data from protected MSO XLS 95- documents.
 */
class MSFILTER_DLLPUBLIC MSCodec_XorXLS95 : public MSCodec_Xor95
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
class MSFILTER_DLLPUBLIC MSCodec_XorWord95 : public MSCodec_Xor95
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
class MSFILTER_DLLPUBLIC MSCodec_Std97
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

    /** Creates an MD5 digest of salt digest. */
    bool                CreateSaltDigest( 
                            const sal_uInt8 nSaltData[16], sal_uInt8 nSaltDigest[16] );

    /** Encodes a block of memory.

        @see rtl_cipher_encode()

        @precond
            The codec must be initialized with InitKey() before this function
            can be used. The destination buffer must be able to take all
            unencoded data from the source buffer (usually this means it must be
            as long as or longer than the source buffer).

        @param pData
            Unencrypted source data block.
        @param nDatLen
            Size of the passed source data block.
        @param pBuffer
            Destination buffer for the encrypted data.
        @param nBufLen
            Size of the destination buffer.

        @return
            true = Encoding was successful (no error occured).
    */
    bool                Encode(
                            const void* pData, sal_Size nDatLen,
                            sal_uInt8* pBuffer, sal_Size nBufLen );

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

    /** Gets salt data and salt digest.

        @precond
            The codec must be initialized with InitKey() before this function
            can be used.

        @param pSalt
            Salt, a random number.
        @param pSaltData
            Salt data block generated from the salt.
        @param pSaltDigest
            Salt digest generated from the salt.
     */
    void				GetEncryptKey (	
                            const sal_uInt8 pSalt[16], 
                            sal_uInt8 pSaltData[16], 
                            sal_uInt8 pSaltDigest[16]);
    
private:
    void                GetDigestFromSalt( const sal_uInt8 pSaltData[16], sal_uInt8 pDigest[16] );

private:
                        MSFILTER_DLLPRIVATE MSCodec_Std97( const MSCodec_Std97& );
    MSFILTER_DLLPRIVATE MSCodec_Std97&      operator=( const MSCodec_Std97& );

    rtlCipher           m_hCipher;
    rtlDigest           m_hDigest;
    sal_uInt8           m_pDigestValue[ RTL_DIGEST_LENGTH_MD5 ];
};

// ============================================================================

} // namespace msfilter

#endif

