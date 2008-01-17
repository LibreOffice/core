/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binarycodec.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:44 $
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

#ifndef OOX_CORE_BINARYCODEC_HXX
#define OOX_CORE_BINARYCODEC_HXX

#include <rtl/cipher.h>
#include <rtl/digest.h>

namespace oox {
namespace core {

// ============================================================================

/** Encodes and decodes data from/to protected MS Office documents.

    Implements a simple XOR encoding/decoding algorithm used in MS Office
    versions up to MSO 95.
 */
class BinaryCodec_XOR
{
public:
    /** Enumerates codec types supported by this XOR codec implementation. */
    enum CodecType
    {
        CODEC_WORD,     /// MS Word XOR codec.
        CODEC_EXCEL     /// MS Excel XOR codec.
    };

public:
    /** Default constructor.

        Two-step construction in conjunction with the initKey() and verifyKey()
        functions allows to try to initialize with different passwords (e.g.
        built-in default password used for Excel workbook protection).
     */
    explicit            BinaryCodec_XOR( CodecType eCodecType );

                        ~BinaryCodec_XOR();

    /** Initializes the algorithm with the specified password.

        @param pnPassData
            Character array containing the password. Must be zero terminated,
            which results in a maximum length of 15 characters.
     */
    void                initKey( const sal_uInt8 pnPassData[ 16 ] );

    /** Verifies the validity of the password using the passed key and hash.

        @precond
            The codec must be initialized with the initKey() function before
            this function can be used.

        @param nKey
            Password key value read from the file.
        @param nHash
            Password hash value read from the file.

        @return
            True = test was successful.
     */
    bool                verifyKey( sal_uInt16 nKey, sal_uInt16 nHash ) const;

    /** Reinitializes the codec to start a new memory block.

        Resets the internal key offset to 0.

        @precond
            The codec must be initialized with the initKey() function before
            this function can be used.
     */
    void                startBlock();

    /** Decodes a block of memory inplace.

        @precond
            The codec must be initialized with the initKey() function before
            this function can be used.

        @param pnDestData
            Destination buffer. Will contain the decrypted data afterwards.
        @param pnSrcData
            Encrypted data block.
        @param nBytes
            Size of the passed data blocks. pnDestData and pnSrcData must be of
            this size.

        @return
            True = decoding was successful (no error occured).
    */
    bool                decode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int32 nBytes );

    /** Lets the cipher skip a specific amount of bytes.

        This function sets the cipher to the same state as if the specified
        amount of data has been decoded with one or more calls of decode().

        @precond
            The codec must be initialized with the initKey() function before
            this function can be used.

        @param nBytes
            Number of bytes to be skipped (cipher "seeks" forward).

        @return
            True = skip was successful (no error occured).
     */
    bool                skip( sal_Int32 nBytes );

    // static -----------------------------------------------------------------

    /** Calculates the 16-bit hash value for the given password.

        The password data may be longer than 16 bytes. The array does not need
        to be terminated with a null byte (but it can without invalidating the
        result).
     */
    static sal_uInt16   getHash( const sal_uInt8* pnPassData, sal_Int32 nSize );

private:
    CodecType           meCodecType;        /// Codec type.
    sal_uInt8           mpnKey[ 16 ];       /// Encryption key.
    sal_Int32           mnOffset;           /// Key offset.
    sal_uInt16          mnBaseKey;          /// Base key from password.
    sal_uInt16          mnHash;             /// Hash value from password.
};

// ============================================================================

/** Encodes and decodes data from protected MSO 97+ documents.

    This is a wrapper class around low level cryptographic functions from RTL.
    Implementation is based on the wvDecrypt package by Caolan McNamara:
    http://www.csn.ul.ie/~caolan/docs/wvDecrypt.html
 */
class BinaryCodec_RCF
{
public:
    /** Default constructor.

        Two-step construction in conjunction with the initKey() and verifyKey()
        functions allows to try to initialize with different passwords (e.g.
        built-in default password used for Excel workbook protection).
     */
    explicit            BinaryCodec_RCF();

                        ~BinaryCodec_RCF();

    /** Initializes the algorithm with the specified password and document ID.

        @param pnPassData
            Unicode character array containing the password. Must be zero
            terminated, which results in a maximum length of 15 characters.
        @param pnUnique
            Unique document identifier read from or written to the file.
     */
    void                initKey(
                            const sal_uInt16 pnPassData[ 16 ],
                            const sal_uInt8 pnUnique[ 16 ] );

    /** Verifies the validity of the password using the passed salt data.

        @precond
            The codec must be initialized with the initKey() function before
            this function can be used.

        @param pnSaltData
            Salt data block read from the file.
        @param pnSaltDigest
            Salt digest read from the file.

        @return
            True = test was successful.
     */
    bool                verifyKey(
                            const sal_uInt8 pnSaltData[ 16 ],
                            const sal_uInt8 pnSaltDigest[ 16 ] );

    /** Rekeys the codec using the specified counter.

        After reading a specific amount of data the cipher algorithm needs to
        be rekeyed using a counter that counts the data blocks.

        The block size is for example 512 bytes for MS Word files and 1024
        bytes for MS Excel files.

        @precond
            The codec must be initialized with the initKey() function before
            this function can be used.

        @param nCounter
            Block counter used to rekey the cipher.
     */
    bool                startBlock( sal_Int32 nCounter );

    /** Decodes a block of memory.

        @see rtl_cipher_decode()

        @precond
            The codec must be initialized with the initKey() function before
            this function can be used.

        @param pnDestData
            Destination buffer. Will contain the decrypted data afterwards.
        @param pnSrcData
            Encrypted data block.
        @param nBytes
            Size of the passed data blocks. pnDestData and pnSrcData must be of
            this size.

        @return
            True = decoding was successful (no error occured).
    */
    bool                decode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int32 nBytes );

    /** Lets the cipher skip a specific amount of bytes.

        This function sets the cipher to the same state as if the specified
        amount of data has been decoded with one or more calls of decode().

        @precond
            The codec must be initialized with the initKey() function before
            this function can be used.

        @param nBytes
            Number of bytes to be skipped (cipher "seeks" forward).

        @return
            True = skip was successful (no error occured).
     */
    bool                skip( sal_Int32 nBytes );

private:
    rtlCipher           mhCipher;
    rtlDigest           mhDigest;
    sal_uInt8           mpnDigestValue[ RTL_DIGEST_LENGTH_MD5 ];
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

