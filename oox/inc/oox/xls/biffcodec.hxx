/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biffcodec.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:48 $
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

#ifndef OOX_XLS_BIFFCODEC_HXX
#define OOX_XLS_BIFFCODEC_HXX

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

/** Helper functions for BIFF stream codecs. */
class BiffCodecHelper
{
public:
    /** Returns the fixed password for workbook protection. */
    static const ::rtl::OString& getBiff5WbProtPassword();

    /** Returns the fixed password for workbook protection. */
    static const ::rtl::OUString& getBiff8WbProtPassword();
};

// ============================================================================

/** Base class for BIFF stream decoders. */
class BiffDecoderBase : public WorkbookHelper
{
public:
    explicit            BiffDecoderBase( const WorkbookHelper& rHelper );
    virtual             ~BiffDecoderBase();

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
    /** Sets the decoder to a state showing whether the password was correct. */
    void                setHasValidPassword( bool bValid );

private:
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
    /** Initializes the wrapped codec object. After that, internal status can
        be querried with isValid(). */
    void                init(
                            const ::rtl::OString& rPass,
                            sal_uInt16 nKey, sal_uInt16 nHash );

    /** Implementation of decryption of a memory block. */
    virtual void        implDecode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes );

private:
    ::oox::core::BinaryCodec_XOR maCodec;   /// Cipher algorithm implementation.
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
    /** Initializes the wrapped codec object. After that, internal status can
        be querried with isValid(). */
    void                init(
                            const ::rtl::OUString& rPass,
                            sal_uInt8 pnDocId[ 16 ],
                            sal_uInt8 pnSaltData[ 16 ],
                            sal_uInt8 pnSaltHash[ 16 ] );

    /** Implementation of decryption of a memory block. */
    virtual void        implDecode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes );

private:
    ::oox::core::BinaryCodec_RCF maCodec;   /// Cipher algorithm implementation.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

