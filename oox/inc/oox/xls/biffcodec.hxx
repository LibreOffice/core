/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef OOX_XLS_BIFFCODEC_HXX
#define OOX_XLS_BIFFCODEC_HXX

#include <vector>
#include <comphelper/docpasswordhelper.hxx>
#include "oox/core/binarycodec.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

const sal_Int64 BIFF_RCF_BLOCKSIZE          = 1024;

// ============================================================================

/** Base class for BIFF stream decoders. */
class BiffDecoderBase : public ::comphelper::IDocPasswordVerifier
{
public:
    explicit            BiffDecoderBase();
    virtual             ~BiffDecoderBase();

    /** Derived classes return a clone of the decoder for usage in new streams. */
    inline BiffDecoderBase* clone() { return implClone(); }

    /** Implementation of the ::comphelper::IDocPasswordVerifier interface,
        calls the new virtual function implVerify(). */
    virtual ::comphelper::DocPasswordVerifierResult
                        verifyPassword( const ::rtl::OUString& rPassword );

    /** Returns true, if the decoder has been initialized correctly. */
    inline bool         isValid() const { return mbValid; }

    /** Decodes nBytes bytes and writes encrypted data into the buffer pnDestData. */
    void                decode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes );

private:
    /** Derived classes return a clone of the decoder for usage in new streams. */
    virtual BiffDecoderBase* implClone() = 0;

    /** Derived classes implement password verification and initialization of
        the decoder. */
    virtual bool        implVerify( const ::rtl::OUString& rPassword ) = 0;

    /** Implementation of decryption of a memory block. */
    virtual void        implDecode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes ) = 0;

private:
    bool                mbValid;        /// True = decoder is correctly initialized.
};

typedef ::boost::shared_ptr< BiffDecoderBase > BiffDecoderRef;

// ============================================================================

/** Decodes BIFF stream contents that are encoded using the old XOR algorithm. */
class BiffDecoder_XOR : public BiffDecoderBase
{
public:
    explicit            BiffDecoder_XOR( sal_uInt16 nKey, sal_uInt16 nHash );

private:
    /** Copy constructor for cloning. */
                        BiffDecoder_XOR( const BiffDecoder_XOR& rDecoder );

    /** Returns a clone of the decoder for usage in new streams. */
    virtual BiffDecoder_XOR* implClone();

    /** Implements password verification and initialization of the decoder. */
    virtual bool        implVerify( const ::rtl::OUString& rPassword );

    /** Implementation of decryption of a memory block. */
    virtual void        implDecode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes );

private:
    ::oox::core::BinaryCodec_XOR maCodec;   /// Cipher algorithm implementation.
    ::std::vector< sal_uInt8 > maPassword;
    sal_uInt16          mnKey;
    sal_uInt16          mnHash;
};

// ============================================================================

/** Decodes BIFF stream contents that are encoded using the RC4 algorithm. */
class BiffDecoder_RCF : public BiffDecoderBase
{
public:
    explicit            BiffDecoder_RCF(
                            sal_uInt8 pnSalt[ 16 ],
                            sal_uInt8 pnVerifier[ 16 ],
                            sal_uInt8 pnVerifierHash[ 16 ] );

private:
    /** Copy constructor for cloning. */
                        BiffDecoder_RCF( const BiffDecoder_RCF& rDecoder );

    /** Returns a clone of the decoder for usage in new streams. */
    virtual BiffDecoder_RCF* implClone();

    /** Implements password verification and initialization of the decoder. */
    virtual bool        implVerify( const ::rtl::OUString& rPassword );

    /** Implementation of decryption of a memory block. */
    virtual void        implDecode(
                            sal_uInt8* pnDestData,
                            const sal_uInt8* pnSrcData,
                            sal_Int64 nStreamPos,
                            sal_uInt16 nBytes );

private:
    ::oox::core::BinaryCodec_RCF maCodec;   /// Cipher algorithm implementation.
    ::std::vector< sal_uInt16 > maPassword;
    ::std::vector< sal_uInt8 > maSalt;
    ::std::vector< sal_uInt8 > maVerifier;
    ::std::vector< sal_uInt8 > maVerifierHash;
};

// ============================================================================

/** Helper for BIFF stream codecs. Holds the used codec object. */
class BiffCodecHelper : public WorkbookHelper
{
public:
    explicit            BiffCodecHelper( const WorkbookHelper& rHelper );

    /** Implementation helper, reads the FILEPASS and returns a decoder object. */
    static BiffDecoderRef implReadFilePass( BiffInputStream& rStrm, BiffType eBiff );

    /** Imports the FILEPASS record, asks for a password and sets a decoder at the stream. */
    bool                importFilePass( BiffInputStream& rStrm );
    /** Clones the contained decoder object if existing and sets it at the passed stream. */
    void                cloneDecoder( BiffInputStream& rStrm );

private:
    BiffDecoderRef      mxDecoder;          /// The decoder for import filter.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
