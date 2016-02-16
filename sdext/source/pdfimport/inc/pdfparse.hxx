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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_PDFPARSE_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_PDFPARSE_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#include <unordered_map>
#include <vector>

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

    explicit EmitContext( const PDFContainer* pTop = nullptr );
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
    static EmitImplData* getEmitData( EmitContext& rContext );
    static void setEmitData( EmitContext& rContext, EmitImplData* pNewEmitData );
};

struct PDFComment : public PDFEntry
{
    OString  m_aComment;

    explicit PDFComment( const OString& rComment )
    : PDFEntry(), m_aComment( rComment ) {}
    virtual ~PDFComment();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;
};

struct PDFValue : public PDFEntry
{
    // abstract base class for simple values
    PDFValue() : PDFEntry() {}
    virtual ~PDFValue();
};

struct PDFName : public PDFValue
{
    OString  m_aName;

    explicit PDFName( const OString& rName )
    : PDFValue(), m_aName( rName ) {}
    virtual ~PDFName();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;

    OUString getFilteredName() const;
};

struct PDFString : public PDFValue
{
    OString  m_aString;

    explicit PDFString( const OString& rString )
    : PDFValue(), m_aString( rString ) {}
    virtual ~PDFString();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;

    OString getFilteredString() const;
};

struct PDFNumber : public PDFValue
{
    double m_fValue;

    explicit PDFNumber( double fVal )
    : PDFValue(), m_fValue( fVal ) {}
    virtual ~PDFNumber();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;
};

struct PDFBool : public PDFValue
{
    bool m_bValue;

    explicit PDFBool( bool bVal )
    : PDFValue(), m_bValue( bVal ) {}
    virtual ~PDFBool();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;
};

struct PDFObjectRef : public PDFValue
{
    unsigned int    m_nNumber;
    unsigned int    m_nGeneration;

    PDFObjectRef( unsigned int nNr, unsigned int nGen )
    : PDFValue(), m_nNumber( nNr ), m_nGeneration( nGen ) {}
    virtual ~PDFObjectRef();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;
};

struct PDFNull : public PDFValue
{
    PDFNull() {}
    virtual ~PDFNull();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;
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
    bool emitSubElements( EmitContext& rWriteContext ) const;
    void cloneSubElements( std::vector<PDFEntry*>& rNewSubElements ) const;

    PDFObject* findObject( unsigned int nNumber, unsigned int nGeneration ) const;
    PDFObject* findObject( PDFObjectRef* pRef ) const
    { return findObject( pRef->m_nNumber, pRef->m_nGeneration ); }
};

struct PDFArray : public PDFContainer
{
    PDFArray() {}
    virtual ~PDFArray();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;
};

struct PDFDict : public PDFContainer
{
    typedef std::unordered_map<OString,PDFEntry*,OStringHash> Map;
    Map m_aMap;

    PDFDict() {}
    virtual ~PDFDict();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;

    // inserting a value of NULL will remove rName and the previous value
    // from the dictionary
    void insertValue( const OString& rName, PDFEntry* pValue );
    // removes a name/value pair from the dict
    void eraseValue( const OString& rName );
    // builds new map as of sub elements
    // returns NULL if successful, else the first offending element
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
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;

    unsigned int getDictLength( const PDFContainer* pObjectContainer = nullptr ) const; // get contents of the "Length" entry of the dict
};

struct PDFTrailer : public PDFContainer
{
    PDFDict*        m_pDict;

    PDFTrailer() : PDFContainer(), m_pDict( nullptr ) {}
    virtual ~PDFTrailer();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;
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
      m_pData( nullptr ),
      m_nMajor( 0 ), m_nMinor( 0 )
    {}
    virtual ~PDFFile();

    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;

    bool isEncrypted() const;

    bool usesSupportedEncryptionFormat() const;

    // this method checks whether rPwd is compatible with
    // either user or owner password and sets up decrypt data in that case
    // returns true if decryption can be done
    bool setupDecryptionData( const OString& rPwd ) const;

    bool decrypt( const sal_uInt8* pInBuffer, sal_uInt32 nLen,
                  sal_uInt8* pOutBuffer,
                  unsigned int nObject, unsigned int nGeneration ) const;

    OUString getDecryptionKey() const;
};

struct PDFObject : public PDFContainer
{
    PDFEntry*       m_pObject;
    PDFStream*      m_pStream;
    unsigned int    m_nNumber;
    unsigned int    m_nGeneration;

    PDFObject( unsigned int nNr, unsigned int nGen )
    : m_pObject( nullptr ), m_pStream( nullptr ), m_nNumber( nNr ), m_nGeneration( nGen ) {}
    virtual ~PDFObject();
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;

    // writes only the contained stream, deflated if necessary
    void writeStream( EmitContext& rContext, const PDFFile* pPDFFile ) const;

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
    virtual bool emit( EmitContext& rWriteContext ) const override;
    virtual PDFEntry* clone() const override;
};

class PDFReader
{
public:
    PDFReader() {}
    ~PDFReader() {}

    static PDFEntry* read( const char* pFileName );
#ifdef _WIN32
    static PDFEntry* read( const char* pBuffer, unsigned int nLen );
#endif
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
