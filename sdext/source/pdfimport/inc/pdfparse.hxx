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

#ifndef INCLUDED_PDFI_PDFPARSE_HXX
#define INCLUDED_PDFI_PDFPARSE_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#include <vector>
#include <boost/unordered_map.hpp>

namespace pdfparse
{

struct EmitImplData;
struct PDFContainer;
class EmitContext
{
    public:
    virtual bool write( const void* pBuf, unsigned int nLen ) = 0;
    virtual unsigned int getCurPos() = 0;
    virtual bool copyOrigBytes( unsigned int nOrigOffset, unsigned int nLen ) = 0;
    virtual unsigned int readOrigBytes( unsigned int nOrigOffset, unsigned int nLen, void* pBuf ) = 0;

    EmitContext( const PDFContainer* pTop = NULL );
    virtual ~EmitContext();

    // set this to deflate contained streams
    bool m_bDeflate;
    // set this to decrypt the PDF file
    bool m_bDecrypt;

    private:
    friend struct PDFEntry;
    EmitImplData* m_pImplData;
};

struct PDFEntry
{
    PDFEntry() {}
    virtual ~PDFEntry();

    virtual bool emit( EmitContext& rWriteContext ) const = 0;
    virtual PDFEntry* clone() const = 0;

    protected:
    EmitImplData* getEmitData( EmitContext& rContext ) const;
    void setEmitData( EmitContext& rContext, EmitImplData* pNewEmitData ) const;
};

struct PDFComment : public PDFEntry
{
    rtl::OString  m_aComment;

    PDFComment( const rtl::OString& rComment )
    : PDFEntry(), m_aComment( rComment ) {}
    virtual ~PDFComment();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;
};

struct PDFValue : public PDFEntry
{
    // abstract base class for simple values
    PDFValue() : PDFEntry() {}
    virtual ~PDFValue();
};

struct PDFName : public PDFValue
{
    rtl::OString  m_aName;

    PDFName( const rtl::OString& rName )
    : PDFValue(), m_aName( rName ) {}
    virtual ~PDFName();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;

    rtl::OUString getFilteredName() const;
};

struct PDFString : public PDFValue
{
    rtl::OString  m_aString;

    PDFString( const rtl::OString& rString )
    : PDFValue(), m_aString( rString ) {}
    virtual ~PDFString();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;

    rtl::OString getFilteredString() const;
};

struct PDFNumber : public PDFValue
{
    double m_fValue;

    PDFNumber( double fVal )
    : PDFValue(), m_fValue( fVal ) {}
    virtual ~PDFNumber();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;
};

struct PDFBool : public PDFValue
{
    bool m_bValue;

    PDFBool( bool bVal )
    : PDFValue(), m_bValue( bVal ) {}
    virtual ~PDFBool();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;
};

struct PDFObjectRef : public PDFValue
{
    unsigned int    m_nNumber;
    unsigned int    m_nGeneration;

    PDFObjectRef( unsigned int nNr, unsigned int nGen )
    : PDFValue(), m_nNumber( nNr ), m_nGeneration( nGen ) {}
    virtual ~PDFObjectRef();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;
};

struct PDFNull : public PDFValue
{
    PDFNull() {}
    virtual ~PDFNull();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;
};

struct PDFObject;
struct PDFContainer : public PDFEntry
{
    sal_Int32              m_nOffset;
    std::vector<PDFEntry*> m_aSubElements;

    // this is an abstract base class for identifying
    // entries that can contain sub elements besides comments
    PDFContainer() : PDFEntry(), m_nOffset( 0 ) {}
    virtual ~PDFContainer();
    virtual bool emitSubElements( EmitContext& rWriteContext ) const;
    virtual void cloneSubElements( std::vector<PDFEntry*>& rNewSubElements ) const;

    PDFObject* findObject( unsigned int nNumber, unsigned int nGeneration ) const;
    PDFObject* findObject( PDFObjectRef* pRef ) const
    { return findObject( pRef->m_nNumber, pRef->m_nGeneration ); }
};

struct PDFArray : public PDFContainer
{
    PDFArray() {}
    virtual ~PDFArray();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;
};

struct PDFDict : public PDFContainer
{
    typedef boost::unordered_map<rtl::OString,PDFEntry*,rtl::OStringHash> Map;
    Map m_aMap;

    PDFDict() {}
    virtual ~PDFDict();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;

    // inserting a value of NULL will remove rName and the previous value
    // from the dictionary
    void insertValue( const rtl::OString& rName, PDFEntry* pValue );
    // removes a name/value pair from the dict
    void eraseValue( const rtl::OString& rName );
    // builds new map as of sub elements
    // returns NULL if successfull, else the first offending element
    PDFEntry* buildMap();
};

struct PDFStream : public PDFEntry
{
    unsigned int    m_nBeginOffset;
    unsigned int    m_nEndOffset; // offset of the byte after the stream
    PDFDict*        m_pDict;

    PDFStream( unsigned int nBegin, unsigned int nEnd, PDFDict* pStreamDict )
    : PDFEntry(), m_nBeginOffset( nBegin ), m_nEndOffset( nEnd ), m_pDict( pStreamDict ) {}
    virtual ~PDFStream();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;

    unsigned int getDictLength( const PDFContainer* pObjectContainer = NULL ) const; // get contents of the "Length" entry of the dict
};

struct PDFTrailer : public PDFContainer
{
    PDFDict*        m_pDict;

    PDFTrailer() : PDFContainer(), m_pDict( NULL ) {}
    virtual ~PDFTrailer();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;
};

struct PDFFileImplData;
struct PDFFile : public PDFContainer
{
    private:
    mutable PDFFileImplData*    m_pData;
    PDFFileImplData*            impl_getData() const;
    public:
    unsigned int        m_nMajor;           // PDF major
    unsigned int        m_nMinor;           // PDF minor

    PDFFile()
    : PDFContainer(),
      m_pData( NULL ),
      m_nMajor( 0 ), m_nMinor( 0 )
    {}
    virtual ~PDFFile();

    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;

    bool isEncrypted() const;

    bool usesSupportedEncryptionFormat() const;

    // this method checks whether rPwd is compatible with
    // either user or owner password and sets up decrypt data in that case
    // returns true if decryption can be done
    bool setupDecryptionData( const rtl::OString& rPwd ) const;

    bool decrypt( const sal_uInt8* pInBuffer, sal_uInt32 nLen,
                  sal_uInt8* pOutBuffer,
                  unsigned int nObject, unsigned int nGeneration ) const;

    rtl::OUString getDecryptionKey() const;
};

struct PDFObject : public PDFContainer
{
    PDFEntry*       m_pObject;
    PDFStream*      m_pStream;
    unsigned int    m_nNumber;
    unsigned int    m_nGeneration;

    PDFObject( unsigned int nNr, unsigned int nGen )
    : m_pObject( NULL ), m_pStream( NULL ), m_nNumber( nNr ), m_nGeneration( nGen ) {}
    virtual ~PDFObject();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;

    // writes only the contained stream, deflated if necessary
    bool writeStream( EmitContext& rContext, const PDFFile* pPDFFile ) const;

    private:
    // returns true if stream is deflated
    // fills *ppStream and *pBytes with start of stream and count of bytes
    // memory returned in *ppStream must be freed with rtl_freeMemory afterwards
    // fills in NULL and 0 in case of error
    bool getDeflatedStream( char** ppStream, unsigned int* pBytes, const PDFContainer* pObjectContainer, EmitContext& rContext ) const;
};

struct PDFPart : public PDFContainer
{
    PDFPart() : PDFContainer() {}
    virtual ~PDFPart();
    virtual bool emit( EmitContext& rWriteContext ) const;
    virtual PDFEntry* clone() const;
};

class PDFReader
{
    public:
    PDFReader() {}
    ~PDFReader() {}

    PDFEntry* read( const char* pFileName );
#ifdef WIN32
    PDFEntry* read( const char* pBuffer, unsigned int nLen );
#endif
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
