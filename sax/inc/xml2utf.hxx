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

// TODO: Woher?
#define Max( a, b )     (((a)>(b)) ? (a) : (b) )
#define Min( a, b )     (((a)<(b)) ? (a) : (b) )

/*
*
* Text2UnicodeConverter
*
**/
namespace sax_expatwrap {

class Text2UnicodeConverter
{

public:
    Text2UnicodeConverter( const ::rtl::OString & sEncoding );
    ~Text2UnicodeConverter();

    ::com::sun::star::uno::Sequence < sal_Unicode > convert( const ::com::sun::star::uno::Sequence<sal_Int8> & );
    sal_Bool canContinue() {  return m_bCanContinue; }

private:
    void init( rtl_TextEncoding encoding );

    rtl_TextToUnicodeConverter  m_convText2Unicode;
    rtl_TextToUnicodeContext    m_contextText2Unicode;
    sal_Bool                    m_bCanContinue;
    sal_Bool                    m_bInitialized;
    rtl_TextEncoding            m_rtlEncoding;
    ::com::sun::star::uno::Sequence<sal_Int8> m_seqSource;
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

    inline ::com::sun::star::uno::Sequence<sal_Int8> convert( const ::rtl::OUString &s )
        {
            return convert( s.getStr() , s.getLength() );
        }
    ::com::sun::star::uno::Sequence<sal_Int8> convert( const sal_Unicode * , sal_Int32 nLength );
    sal_Bool canContinue() {  return m_bCanContinue; }

private:
    void init( rtl_TextEncoding encoding );

    rtl_UnicodeToTextConverter  m_convUnicode2Text;
    rtl_UnicodeToTextContext    m_contextUnicode2Text;
    sal_Bool                    m_bCanContinue;
    sal_Bool                    m_bInitialized;
    rtl_TextEncoding            m_rtlEncoding;
    ::com::sun::star::uno::Sequence<sal_Unicode>        m_seqSource;
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
        m_bStarted( sal_False ),
        m_pText2Unicode( 0 ),
        m_pUnicode2Text( 0 )
        {}

    ~XMLFile2UTFConverter();

    void setInputStream( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > &r ) { m_in = r; }
    void setEncoding( const ::rtl::OString &s ) { m_sEncoding = s; }



    // @param nMaxToRead The number of chars, that should be read. Note that this is no exact number. There
    //                   may be returned less or more bytes than ordered.
    sal_Int32 readAndConvert( ::com::sun::star::uno::Sequence<sal_Int8> &seq , sal_Int32 nMaxToRead )
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::io::NotConnectedException ,
                ::com::sun::star::io::BufferSizeExceededException ,
                ::com::sun::star::uno::RuntimeException );

private:

    // Called only on first Sequence of bytes. Tries to figure out file format and encoding information.
    // @return TRUE, when encoding information could be retrieved
    // @return FALSE, when no encoding information was found in file
    sal_Bool scanForEncoding( ::com::sun::star::uno::Sequence<sal_Int8> &seq );

    // Called only on first Sequence of bytes. Tries to figure out
    // if enough data is available to scan encoding
    // @return TRUE, when encoding is retrievable
    // @return FALSE, when more data is needed
    sal_Bool isEncodingRecognizable( const ::com::sun::star::uno::Sequence< sal_Int8 > & seq );

    // When encoding attribute is within the text (in the first line), it is removed.
    void removeEncoding( ::com::sun::star::uno::Sequence<sal_Int8> &seq );

    // Initializes decoding depending on m_sEncoding setting
    void initializeDecoding();
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  m_in;

    sal_Bool m_bStarted;
    ::rtl::OString m_sEncoding;

    Text2UnicodeConverter *m_pText2Unicode;
    Unicode2TextConverter *m_pUnicode2Text;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
