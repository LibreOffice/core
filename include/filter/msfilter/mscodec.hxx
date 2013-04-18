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

#ifndef SVX_MSCODEC_HXX
#define SVX_MSCODEC_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>

#include <rtl/cipher.h>
#include <rtl/digest.h>
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

    /** Initializes the algorithm with the encryption data.

        @param aData
            The sequence contains the necessary data to initialize
            the codec.
     */
    sal_Bool                InitCodec( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aData );

    /** Retrieves the encryption data

        @return
            The sequence contains the necessary data to initialize
            the codec.
     */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetEncryptionData();


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

protected:
    sal_uInt8           mpnKey[ 16 ];   /// Encryption key.
    sal_Size            mnOffset;       /// Key offset.

private:
                        MSFILTER_DLLPRIVATE MSCodec_Xor95( const MSCodec_Xor95& );
    MSFILTER_DLLPRIVATE MSCodec_Xor95&      operator=( const MSCodec_Xor95& );

    sal_uInt16          mnKey;          /// Base key from password.
    sal_uInt16          mnHash;         /// Hash value from password.
    int         mnRotateDistance;
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

    /** Initializes the algorithm with the encryption data.

        @param aData
            The sequence contains the necessary data to initialize
            the codec.
     */
    sal_Bool                InitCodec( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aData );

    /** Retrieves the encryption data

        @return
            The sequence contains the necessary data to initialize
            the codec.
     */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetEncryptionData();


    /** Initializes the algorithm with the specified password and document ID.

        @param pPassData
            Wide character array containing the password. Must be zero
            terminated, which results in a maximum length of 15 characters.
        @param pDocId
            Unique document identifier read from or written to the file.
     */
    void                InitKey(
                            const sal_uInt16 pPassData[ 16 ],
                            const sal_uInt8 pDocId[ 16 ] );

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
            true = Encoding was successful (no error occurred).
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
            true = Decoding was successful (no error occurred).
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
    void                GetEncryptKey (
                            const sal_uInt8 pSalt[16],
                            sal_uInt8 pSaltData[16],
                            sal_uInt8 pSaltDigest[16]);

    /* allows to get the unique document id from the codec
     */
    void                GetDocId( sal_uInt8 pDocId[16] );

    void                GetDigestFromSalt( const sal_uInt8 pSaltData[16], sal_uInt8 pDigest[16] );

private:
    void                InitKeyImpl(
                            const sal_uInt8 pKeyData[64],
                            const sal_uInt8 pDocId[16] );


private:
                        MSFILTER_DLLPRIVATE MSCodec_Std97( const MSCodec_Std97& );
    MSFILTER_DLLPRIVATE MSCodec_Std97&      operator=( const MSCodec_Std97& );

    rtlCipher           m_hCipher;
    rtlDigest           m_hDigest;
    sal_uInt8           m_pDigestValue[ RTL_DIGEST_LENGTH_MD5 ];
    sal_uInt8           m_pDocId[16];
};

// ============================================================================

} // namespace msfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
