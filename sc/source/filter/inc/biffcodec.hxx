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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_BIFFCODEC_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_BIFFCODEC_HXX

#include <vector>
#include <comphelper/docpasswordhelper.hxx>
#include <oox/core/binarycodec.hxx>
#include "workbookhelper.hxx"

namespace oox {
namespace xls {

const sal_Int64 BIFF_RCF_BLOCKSIZE          = 1024;

/** Base class for BIFF stream decoders. */
class BiffDecoderBase : public ::comphelper::IDocPasswordVerifier
{
public:
    explicit            BiffDecoderBase();
    virtual             ~BiffDecoderBase() override;

    /** Implementation of the ::comphelper::IDocPasswordVerifier interface. */
    virtual ::comphelper::DocPasswordVerifierResult verifyPassword( const OUString& rPassword, css::uno::Sequence< css::beans::NamedValue >& o_rEncryptionData ) override;
    virtual ::comphelper::DocPasswordVerifierResult verifyEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& o_rEncryptionData ) override;

    /** Returns true, if the decoder has been initialized correctly. */
    bool         isValid() const { return mbValid; }

private:
    /** Derived classes implement password verification and initialization of
        the decoder. */
    virtual css::uno::Sequence< css::beans::NamedValue > implVerifyPassword( const OUString& rPassword ) = 0;
    virtual bool implVerifyEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& rEncryptionData ) = 0;

private:
    bool                mbValid;        /// True = decoder is correctly initialized.
};

/** Decodes BIFF stream contents that are encoded using the old XOR algorithm. */
class BiffDecoder_XOR : public BiffDecoderBase
{
private:
    /** Copy constructor for cloning. */
                        BiffDecoder_XOR( const BiffDecoder_XOR& rDecoder );

    /** Implements password verification and initialization of the decoder. */
    virtual css::uno::Sequence< css::beans::NamedValue > implVerifyPassword( const OUString& rPassword ) override;
    virtual bool implVerifyEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& rEncryptionData ) override;

private:
    ::oox::core::BinaryCodec_XOR maCodec;   /// Cipher algorithm implementation.
    css::uno::Sequence< css::beans::NamedValue > maEncryptionData;
    sal_uInt16          mnKey;
    sal_uInt16          mnHash;
};

/** Decodes BIFF stream contents that are encoded using the RC4 algorithm. */
class BiffDecoder_RCF : public BiffDecoderBase
{
private:
    /** Copy constructor for cloning. */
                        BiffDecoder_RCF( const BiffDecoder_RCF& rDecoder );

    /** Implements password verification and initialization of the decoder. */
    virtual css::uno::Sequence< css::beans::NamedValue > implVerifyPassword( const OUString& rPassword ) override;
    virtual bool implVerifyEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& rEncryptionData ) override;

private:
    ::oox::core::BinaryCodec_RCF maCodec;   /// Cipher algorithm implementation.
    css::uno::Sequence< css::beans::NamedValue > maEncryptionData;
    ::std::vector< sal_uInt8 > maSalt;
    ::std::vector< sal_uInt8 > maVerifier;
    ::std::vector< sal_uInt8 > maVerifierHash;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
