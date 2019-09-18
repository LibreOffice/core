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

#ifndef INCLUDED_SAX_INC_XML2UTF_HXX
#define INCLUDED_SAX_INC_XML2UTF_HXX

#include <sal/config.h>

#include <memory>

#include <sal/types.h>
#include <rtl/string.hxx>

#include <com/sun/star/io/XInputStream.hpp>

namespace sax_expatwrap {

class Text2UnicodeConverter
{

public:
    Text2UnicodeConverter( const OString & sEncoding );
    ~Text2UnicodeConverter();

    css::uno::Sequence < sal_Unicode > convert( const css::uno::Sequence<sal_Int8> & );
    bool canContinue() const { return m_bCanContinue; }

private:
    void init( rtl_TextEncoding encoding );

    rtl_TextToUnicodeConverter   m_convText2Unicode;
    rtl_TextToUnicodeContext     m_contextText2Unicode;
    bool                         m_bCanContinue;
    bool                         m_bInitialized;
    css::uno::Sequence<sal_Int8> m_seqSource;
};

/*----------------------------------------
*
* Unicode2TextConverter
*
**-----------------------------------------*/
class Unicode2TextConverter
{
public:
    Unicode2TextConverter( rtl_TextEncoding encoding );
    ~Unicode2TextConverter();

    css::uno::Sequence<sal_Int8> convert( const sal_Unicode * , sal_Int32 nLength );

private:
    rtl_UnicodeToTextConverter      m_convUnicode2Text;
    rtl_UnicodeToTextContext        m_contextUnicode2Text;
    css::uno::Sequence<sal_Unicode> m_seqSource;
};


/*----------------------------------------
*
* XMLFile2UTFConverter
*
**-----------------------------------------*/
class XMLFile2UTFConverter
{
public:
    XMLFile2UTFConverter( ):
        m_bStarted( false )
        {}

    void setInputStream( css::uno::Reference< css::io::XInputStream > const &r ) { m_in = r; }
    void setEncoding( const OString &s ) { m_sEncoding = s; }


    // @param nMaxToRead The number of chars, that should be read. Note that this is no exact number. There
    //                   may be returned less or more bytes than ordered.
    /// @throws css::io::IOException
    /// @throws css::io::NotConnectedException
    /// @throws css::io::BufferSizeExceededException
    /// @throws css::uno::RuntimeException
    sal_Int32 readAndConvert( css::uno::Sequence<sal_Int8> &seq , sal_Int32 nMaxToRead );

private:

    // Called only on first Sequence of bytes. Tries to figure out file format and encoding information.
    // @return TRUE, when encoding information could be retrieved
    // @return FALSE, when no encoding information was found in file
    bool scanForEncoding( css::uno::Sequence<sal_Int8> &seq );

    // Called only on first Sequence of bytes. Tries to figure out
    // if enough data is available to scan encoding
    // @return TRUE, when encoding is retrievable
    // @return FALSE, when more data is needed
    static bool isEncodingRecognizable( const css::uno::Sequence< sal_Int8 > & seq );

    // When encoding attribute is within the text (in the first line), it is removed.
    static void removeEncoding( css::uno::Sequence<sal_Int8> &seq );

    // Initializes decoding depending on m_sEncoding setting
    void initializeDecoding();
private:
    css::uno::Reference< css::io::XInputStream >  m_in;

    bool m_bStarted;
    OString m_sEncoding;

    std::unique_ptr<Text2UnicodeConverter> m_pText2Unicode;
    std::unique_ptr<Unicode2TextConverter> m_pUnicode2Text;
};
}

#endif // INCLUDED_SAX_INC_XML2UTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
