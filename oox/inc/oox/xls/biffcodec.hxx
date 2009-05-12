/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: biffcodec.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_XLS_BIFFCODEC_HXX
#define OOX_XLS_BIFFCODEC_HXX

#include <vector>
#include "oox/core/binarycodec.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

const sal_Int32 CODEC_OK                    = 0;
const sal_Int32 CODEC_ERROR_WRONG_PASS      = 1;
const sal_Int32 CODEC_ERROR_UNSUPP_CRYPT    = 2;

const sal_Int64 BIFF_RCF_BLOCKSIZE          = 1024;

// ============================================================================

/** Base class for BIFF stream decoders. */
class BiffDecoderBase : public WorkbookHelper
{
public:
    explicit            BiffDecoderBase( const WorkbookHelper& rHelper );
    virtual             ~BiffDecoderBase();

    /** Derived classes return a clone of the decoder for usage in new streams. */
    inline BiffDecoderBase* clone() { return implClone(); }

    /** Returns the current error code of the decoder. */
    inline sal_Int32    getErrorCode() const { return mnError; }
    /** Returns true, if the decoder has been constructed successfully.
        This means especially that construction happened with a valid password. */
    inline bool         isValid() const { return mnError == CODEC_OK; }

    /** Decodes nBytes bytes and writes encrypted data into the buffer pnDestData. */
    void                decode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes );

protected:
    /** Copy constructor for cloning. */
                        BiffDecoderBase( const BiffDecoderBase& rDecoder );

    /** Sets the decoder to a state showing whether the password was correct. */
    void                setHasValidPassword( bool bValid );

private:
    /** Derived classes return a clone of the decoder for usage in new streams. */
    virtual BiffDecoderBase* implClone() = 0;

    /** Implementation of decryption of a memory block. */
    virtual void        implDecode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes ) = 0;

private:
    sal_Int32           mnError;        /// Decoder error code.
};

typedef ::boost::shared_ptr< BiffDecoderBase > BiffDecoderRef;

// ============================================================================

/** Decodes BIFF stream contents that are encoded using the old XOR algorithm. */
class BiffDecoder_XOR : public BiffDecoderBase
{
public:
    /** Constructs the decoder.

        Checks if the passed key and hash specify workbook protection. Asks for
        a password otherwise.

        @param nKey
            Password key from FILEPASS record to verify password.
        @param nHash
            Password hash value from FILEPASS record to verify password.
     */
    explicit            BiffDecoder_XOR(
                            const WorkbookHelper& rHelper,
                            sal_uInt16 nKey, sal_uInt16 nHash );

private:
    /** Copy constructor for cloning. */
                        BiffDecoder_XOR( const BiffDecoder_XOR& rDecoder );

    /** Initializes the wrapped codec object. After that, internal status can
        be querried with isValid(). */
    void                init( const ::rtl::OString& rPass );

    /** Returns a clone of the decoder for usage in new streams. */
    virtual BiffDecoder_XOR* implClone();

    /** Implementation of decryption of a memory block. */
    virtual void        implDecode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes );

private:
    ::oox::core::BinaryCodec_XOR maCodec;   /// Cipher algorithm implementation.
    ::rtl::OString      maPass;
    sal_uInt16          mnKey;
    sal_uInt16          mnHash;
};

// ============================================================================

/** Decodes BIFF stream contents that are encoded using the RC4 algorithm. */
class BiffDecoder_RCF : public BiffDecoderBase
{
public:
    /** Constructs the decoder.

        Checks if the passed salt data specifies workbook protection. Asks for
        a password otherwise.

        @param pnDocId
            Unique document identifier from FILEPASS record.
        @param pnSaltData
            Salt data from FILEPASS record.
        @param pnSaltHash
            Salt hash value from FILEPASS record.
     */
    explicit            BiffDecoder_RCF(
                            const WorkbookHelper& rHelper,
                            sal_uInt8 pnDocId[ 16 ],
                            sal_uInt8 pnSaltData[ 16 ],
                            sal_uInt8 pnSaltHash[ 16 ] );

private:
    /** Copy constructor for cloning. */
                        BiffDecoder_RCF( const BiffDecoder_RCF& rDecoder );

    /** Initializes the wrapped codec object. After that, internal status can
        be querried with isValid(). */
    void                init( const ::rtl::OUString& rPass );

    /** Returns a clone of the decoder for usage in new streams. */
    virtual BiffDecoder_RCF* implClone();

    /** Implementation of decryption of a memory block. */
    virtual void        implDecode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes );

private:
    ::oox::core::BinaryCodec_RCF maCodec;   /// Cipher algorithm implementation.
    ::rtl::OUString     maPass;
    ::std::vector< sal_uInt8 > maDocId;
    ::std::vector< sal_uInt8 > maSaltData;
    ::std::vector< sal_uInt8 > maSaltHash;
};

// ============================================================================

/** Helper for BIFF stream codecs. Holds the used codec object. */
class BiffCodecHelper : public WorkbookHelper
{
public:
    explicit            BiffCodecHelper( const WorkbookHelper& rHelper );

    /** Returns the fixed password for workbook protection. */
    static const ::rtl::OString& getBiff5WbProtPassword();
    /** Returns the fixed password for workbook protection. */
    static const ::rtl::OUString& getBiff8WbProtPassword();

    /** Looks for a password provided via API, or queries it via GUI. */
    ::rtl::OUString     queryPassword();

    /** Imports the FILEPASS record and sets a decoder at the stream. */
    bool                importFilePass( BiffInputStream& rStrm );
    /** Clones the contained decoder object if existing and sets it at the passed stream. */
    void                cloneDecoder( BiffInputStream& rStrm );

private:
    void                importFilePass_XOR( BiffInputStream& rStrm );
    void                importFilePass_RCF( BiffInputStream& rStrm );
    void                importFilePass_Strong( BiffInputStream& rStrm );
    void                importFilePass2( BiffInputStream& rStrm );
    void                importFilePass8( BiffInputStream& rStrm );

private:
    BiffDecoderRef      mxDecoder;          /// The decoder for import filter.
    ::rtl::OUString     maPassword;         /// Password for stream encoder/decoder.
    bool                mbHasPassword;      /// True = password already querried.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

