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


#include <pdfparse.hxx>

// boost using obsolete stuff
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996)
#pragma warning(disable:4503)
#endif

// workaround windows compiler: do not include multi_pass.hpp
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_utility.hpp>
#include <boost/spirit/include/classic_error_handling.hpp>
#include <boost/spirit/include/classic_file_iterator.hpp>
#include <boost/bind.hpp>

#include <string.h>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/alloc.h>
#include <sal/log.hxx>

// disable warnings again because someone along the line has enabled them
// (we have  included boost headers, what did you expect?)
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996)
#pragma warning(disable:4503)
#endif


using namespace boost::spirit;
using namespace pdfparse;


class StringEmitContext : public EmitContext
{
    OStringBuffer m_aBuf;
    public:
    StringEmitContext() : EmitContext(), m_aBuf(256) {}

    virtual bool write( const void* pBuf, unsigned int nLen ) throw() override
    {
        m_aBuf.append( static_cast<const sal_Char*>(pBuf), nLen );
        return true;
    }
    virtual unsigned int getCurPos() throw() override { return m_aBuf.getLength(); }
    virtual bool copyOrigBytes( unsigned int nOrigOffset, unsigned int nLen ) throw() override
    { return (nOrigOffset+nLen < static_cast<unsigned int>(m_aBuf.getLength()) ) &&
             write( m_aBuf.getStr() + nOrigOffset, nLen ); }
    virtual unsigned int readOrigBytes( unsigned int nOrigOffset, unsigned int nLen, void* pBuf ) throw() override
    {
        if( nOrigOffset+nLen < static_cast<unsigned int>(m_aBuf.getLength()) )
        {
            memcpy( pBuf, m_aBuf.getStr()+nOrigOffset, nLen );
            return nLen;
        }
        return 0;
    }

    OString getString() { return m_aBuf.makeStringAndClear(); }
};

template< class iteratorT >
class PDFGrammar :  public grammar< PDFGrammar<iteratorT> >
{
public:

    explicit PDFGrammar( const iteratorT& first )
    : m_fDouble( 0.0 ), m_aGlobalBegin( first ) {}
    ~PDFGrammar()
    {
        if( !m_aObjectStack.empty() )
            delete m_aObjectStack.front();
    }

    double m_fDouble;
    std::vector< unsigned int > m_aUIntStack;
    std::vector< PDFEntry* >    m_aObjectStack;
    OString                m_aErrorString;
    iteratorT                   m_aGlobalBegin;

public:
    struct pdf_string_parser
    {
        typedef nil_t result_t;
        template <typename ScannerT>
        std::ptrdiff_t
        operator()(ScannerT const& scan, result_t&) const
        {
            std::ptrdiff_t len = 0;

            int nBraceLevel = 0;
            while( ! scan.at_end() )
            {
                char c = *scan;
                if( c == ')' )
                {
                    nBraceLevel--;
                    if( nBraceLevel < 0 )
                        break;
                }
                else if( c == '(' )
                    nBraceLevel++;
                else if( c == '\\' ) // ignore escaped braces
                {
                    ++len;
                    ++scan.first;                 // tdf#63054: avoid skipping spaces
                    if( scan.first == scan.last ) // tdf#63054: avoid skipping spaces
                        break;
                }
                ++len;
                ++scan;
            }
            return scan.at_end() ? -1 : len;
        }
    };

    template< typename ScannerT >
    struct definition
    {
        explicit definition( const PDFGrammar<iteratorT>& rSelf )
        {
            PDFGrammar<iteratorT>* pSelf = const_cast< PDFGrammar<iteratorT>* >( &rSelf );

            // workaround workshop compiler: comment_p doesn't work
            // comment     = comment_p("%")[boost::bind(&PDFGrammar::pushComment, pSelf, _1, _2 )];
            comment     = lexeme_d[ (ch_p('%') >> *(~ch_p('\r') & ~ch_p('\n')) >> eol_p)[boost::bind(&PDFGrammar::pushComment, pSelf, _1, _2 )] ];

            boolean     = (str_p("true") | str_p("false"))[boost::bind(&PDFGrammar::pushBool, pSelf, _1, _2)];

            // workaround workshop compiler: confix_p doesn't work
            //stream      = confix_p( "stream", *anychar_p, "endstream" )[boost::bind(&PDFGrammar::emitStream, pSelf, _1, _2 )];
            stream      = (str_p("stream") >> *(anychar_p - str_p("endstream")) >> str_p("endstream"))[boost::bind(&PDFGrammar::emitStream, pSelf, _1, _2 )];

            name        = lexeme_d[
                            ch_p('/')
                            >> (*(anychar_p-chset_p("\t\n\f\r ()<>[]{}/%")-ch_p('\0')))
                               [boost::bind(&PDFGrammar::pushName, pSelf, _1, _2)] ];

            // workaround workshop compiler: confix_p doesn't work
            //stringtype  = ( confix_p("(",*anychar_p, ")") |
            //                confix_p("<",*xdigit_p,  ">") )
            //              [boost::bind(&PDFGrammar::pushString,pSelf, _1, _2)];

            stringtype  = ( ( ch_p('(') >> functor_parser<pdf_string_parser>() >> ch_p(')') ) |
                            ( ch_p('<') >> *xdigit_p >> ch_p('>') ) )
                          [boost::bind(&PDFGrammar::pushString,pSelf, _1, _2)];

            null_object = str_p( "null" )[boost::bind(&PDFGrammar::pushNull, pSelf, _1, _2)];

            #ifdef USE_ASSIGN_ACTOR
            objectref   = ( uint_p[push_back_a(pSelf->m_aUIntStack)]
                            >> uint_p[push_back_a(pSelf->m_aUIntStack)]
                            >> ch_p('R')
                            >> eps_p
                          )[boost::bind(&PDFGrammar::pushObjectRef, pSelf, _1, _2)];
            #else
            objectref   = ( uint_p[boost::bind(&PDFGrammar::push_back_action_uint, pSelf, _1)]
                            >> uint_p[boost::bind(&PDFGrammar::push_back_action_uint, pSelf, _1)]
                            >> ch_p('R')
                            >> eps_p
                          )[boost::bind(&PDFGrammar::pushObjectRef, pSelf, _1, _2)];
            #endif

            #ifdef USE_ASSIGN_ACTOR
            simple_type = objectref | name |
                          ( real_p[assign_a(pSelf->m_fDouble)] >> eps_p )
                          [boost::bind(&PDFGrammar::pushDouble, pSelf, _1, _2)]
                          | stringtype | boolean | null_object;
            #else
            simple_type = objectref | name |
                          ( real_p[boost::bind(&PDFGrammar::assign_action_double, pSelf, _1)] >> eps_p )
                          [boost::bind(&PDFGrammar::pushDouble, pSelf, _1, _2)]
                          | stringtype | boolean | null_object;
            #endif

            dict_begin  = str_p( "<<" )[boost::bind(&PDFGrammar::beginDict, pSelf, _1, _2)];
            dict_end    = str_p( ">>" )[boost::bind(&PDFGrammar::endDict, pSelf, _1, _2)];

            array_begin = str_p("[")[boost::bind(&PDFGrammar::beginArray,pSelf, _1, _2)];
            array_end   = str_p("]")[boost::bind(&PDFGrammar::endArray,pSelf, _1, _2)];

            #ifdef USE_ASSIGN_ACTOR
            object_begin= uint_p[push_back_a(pSelf->m_aUIntStack)]
                          >> uint_p[push_back_a(pSelf->m_aUIntStack)]
                          >> str_p("obj" )[boost::bind(&PDFGrammar::beginObject, pSelf, _1, _2)];
            #else
            object_begin= uint_p[boost::bind(&PDFGrammar::push_back_action_uint, pSelf, _1)]
                          >> uint_p[boost::bind(&PDFGrammar::push_back_action_uint, pSelf, _1)]
                          >> str_p("obj" )[boost::bind(&PDFGrammar::beginObject, pSelf, _1, _2)];
            #endif
            object_end  = str_p( "endobj" )[boost::bind(&PDFGrammar::endObject, pSelf, _1, _2)];

            xref        = str_p( "xref" ) >> uint_p >> uint_p
                          >> lexeme_d[
                                +( repeat_p(10)[digit_p]
                                   >> blank_p
                                   >> repeat_p(5)[digit_p]
                                   >> blank_p
                                   >> ( ch_p('n') | ch_p('f') )
                                   >> repeat_p(2)[space_p]
                                 ) ];

            dict_element= dict_begin | comment | simple_type
                          | array_begin | array_end | dict_end;

            object      = object_begin
                          >> *dict_element
                          >> !stream
                          >> object_end;

            trailer     = str_p( "trailer" )[boost::bind(&PDFGrammar::beginTrailer,pSelf,_1,_2)]
                          >> *dict_element
                          >> str_p("startxref")
                          >> uint_p
                          >> str_p("%%EOF")[boost::bind(&PDFGrammar::endTrailer,pSelf,_1,_2)];

            #ifdef USE_ASSIGN_ACTOR
            pdfrule     = ! (lexeme_d[
                                str_p( "%PDF-" )
                                >> uint_p[push_back_a(pSelf->m_aUIntStack)]
                                >> ch_p('.')
                                >> uint_p[push_back_a(pSelf->m_aUIntStack)]
                                >> *((~ch_p('\r') & ~ch_p('\n')))
                                >> eol_p
                             ])[boost::bind(&PDFGrammar::haveFile,pSelf, _1, _2)]
                          >> *( comment | object | ( xref >> trailer ) );
            #else
            pdfrule     = ! (lexeme_d[
                                str_p( "%PDF-" )
                                >> uint_p[boost::bind(&PDFGrammar::push_back_action_uint, pSelf, _1)]
                                >> ch_p('.')
                                >> uint_p[boost::bind(&PDFGrammar::push_back_action_uint, pSelf, _1)]
                                >> *(~ch_p('\r') & ~ch_p('\n'))
                                >> eol_p
                             ])[boost::bind(&PDFGrammar::haveFile,pSelf, _1, _2)]
                          >> *( comment | object | ( xref >> trailer ) );
            #endif
        }
        rule< ScannerT > comment, stream, boolean, name, stringtype, null_object, simple_type,
                         objectref, array, value, dict_element, dict_begin, dict_end,
                         array_begin, array_end, object, object_begin, object_end,
                         xref, trailer, pdfrule;

        const rule< ScannerT >& start() const { return pdfrule; }
    };

    #ifndef USE_ASSIGN_ACTOR
    void push_back_action_uint( unsigned int i )
    {
        m_aUIntStack.push_back( i );
    }
    void assign_action_double( double d )
    {
        m_fDouble = d;
    }
    #endif

    static void parseError( const char* pMessage, iteratorT pLocation )
    {
        throw_( pLocation, pMessage );
    }

    OString iteratorToString( iteratorT first, iteratorT last ) const
    {
        OStringBuffer aStr( 32 );
        while( first != last )
        {
            aStr.append( *first );
            ++first;
        }
        return aStr.makeStringAndClear();
    }

    void haveFile( iteratorT pBegin, SAL_UNUSED_PARAMETER iteratorT /*pEnd*/ )
    {
        if( m_aObjectStack.empty() )
        {
            PDFFile* pFile = new PDFFile();
            pFile->m_nMinor = m_aUIntStack.back();
            m_aUIntStack.pop_back();
            pFile->m_nMajor = m_aUIntStack.back();
            m_aUIntStack.pop_back();
            m_aObjectStack.push_back( pFile );
        }
        else
            parseError( "found file header in unusual place", pBegin );
    }

    void pushComment( iteratorT first, iteratorT last )
    {
        // add a comment to the current stack element
        PDFComment* pComment =
            new PDFComment(iteratorToString(first,last));
        if( m_aObjectStack.empty() )
            m_aObjectStack.push_back( new PDFPart() );
        PDFContainer* pContainer = dynamic_cast<PDFContainer*>(m_aObjectStack.back());
        if( pContainer == nullptr )
            parseError( "comment without container", first );
        pContainer->m_aSubElements.emplace_back( pComment );
    }

    void insertNewValue( std::unique_ptr<PDFEntry> pNewValue, iteratorT pPos )
    {
        PDFContainer* pContainer = nullptr;
        const char* pMsg = nullptr;
        if( ! m_aObjectStack.empty() &&
            (pContainer = dynamic_cast<PDFContainer*>(m_aObjectStack.back())) != nullptr )
        {
            if( dynamic_cast<PDFDict*>(pContainer) == nullptr      &&
                dynamic_cast<PDFArray*>(pContainer) == nullptr )
            {
                PDFObject* pObj = dynamic_cast<PDFObject*>(pContainer);
                if( pObj )
                {
                    if( pObj->m_pObject == nullptr )
                        pObj->m_pObject = pNewValue.get();
                    else
                    {
                        pMsg = "second value for object";
                        pContainer = nullptr;
                    }
                }
                else if( dynamic_cast<PDFDict*>(pNewValue.get()) )
                {
                    PDFTrailer* pTrailer = dynamic_cast<PDFTrailer*>(pContainer);
                    if( pTrailer )
                    {
                        if( pTrailer->m_pDict == nullptr )
                            pTrailer->m_pDict = dynamic_cast<PDFDict*>(pNewValue.get());
                        else
                            pContainer = nullptr;
                    }
                    else
                        pContainer = nullptr;
                }
                else
                    pContainer = nullptr;
            }
        }
        if( pContainer )
            pContainer->m_aSubElements.emplace_back( std::move(pNewValue) );
        else
        {
            if( ! pMsg )
            {
                if( dynamic_cast<PDFContainer*>(pNewValue.get()) )
                    pMsg = "array without container";
                else
                    pMsg = "value without container";
            }
            parseError( pMsg, pPos );
        }
    }

    void pushName( iteratorT first, iteratorT last )
    {
        insertNewValue( std::make_unique<PDFName>(iteratorToString(first,last)), first );
    }

    void pushDouble( iteratorT first, SAL_UNUSED_PARAMETER iteratorT /*last*/ )
    {
        insertNewValue( std::make_unique<PDFNumber>(m_fDouble), first );
    }

    void pushString( iteratorT first, iteratorT last )
    {
        insertNewValue( std::make_unique<PDFString>(iteratorToString(first,last)), first );
    }

    void pushBool( iteratorT first, iteratorT last )
    {
        insertNewValue( std::make_unique<PDFBool>( last-first == 4 ), first );
    }

    void pushNull( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        insertNewValue( std::make_unique<PDFNull>(), first );
    }


    void beginObject( iteratorT first, SAL_UNUSED_PARAMETER iteratorT /*last*/ )
    {
        if( m_aObjectStack.empty() )
            m_aObjectStack.push_back( new PDFPart() );

        unsigned int nGeneration = m_aUIntStack.back();
        m_aUIntStack.pop_back();
        unsigned int nObject = m_aUIntStack.back();
        m_aUIntStack.pop_back();

        PDFObject* pObj = new PDFObject( nObject, nGeneration );
        pObj->m_nOffset = first - m_aGlobalBegin;

        PDFContainer* pContainer = dynamic_cast<PDFContainer*>(m_aObjectStack.back());
        if( pContainer &&
            ( dynamic_cast<PDFFile*>(pContainer) ||
              dynamic_cast<PDFPart*>(pContainer) ) )
        {
            pContainer->m_aSubElements.emplace_back( pObj );
            m_aObjectStack.push_back( pObj );
        }
        else
            parseError( "object in wrong place", first );
    }

    void endObject( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        if( m_aObjectStack.empty() )
            parseError( "endobj without obj", first );
        else if( dynamic_cast<PDFObject*>(m_aObjectStack.back()) == nullptr )
            parseError( "spurious endobj", first );
        else
            m_aObjectStack.pop_back();
    }

    void pushObjectRef( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        unsigned int nGeneration = m_aUIntStack.back();
        m_aUIntStack.pop_back();
        unsigned int nObject = m_aUIntStack.back();
        m_aUIntStack.pop_back();
        insertNewValue( std::make_unique<PDFObjectRef>(nObject,nGeneration), first );
    }

    void beginDict( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        PDFDict* pDict = new PDFDict();
        pDict->m_nOffset = first - m_aGlobalBegin;

        insertNewValue( std::unique_ptr<PDFEntry>(pDict), first );
        // will not come here if insertion fails (exception)
        m_aObjectStack.push_back( pDict );
    }
    void endDict( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        PDFDict* pDict = nullptr;
        if( m_aObjectStack.empty() )
            parseError( "dictionary end without begin", first );
        else if( (pDict = dynamic_cast<PDFDict*>(m_aObjectStack.back())) == nullptr )
            parseError( "spurious dictionary end", first );
        else
            m_aObjectStack.pop_back();

        PDFEntry* pOffender = pDict->buildMap();
        if( pOffender )
        {
            StringEmitContext aCtx;
            aCtx.write( "offending dictionary element: ", 30 );
            pOffender->emit( aCtx );
            m_aErrorString = aCtx.getString();
            parseError( m_aErrorString.getStr(), first );
        }
    }

    void beginArray( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        PDFArray* pArray = new PDFArray();
        pArray->m_nOffset = first - m_aGlobalBegin;

        insertNewValue( std::unique_ptr<PDFEntry>(pArray), first );
        // will not come here if insertion fails (exception)
        m_aObjectStack.push_back( pArray );
    }

    void endArray( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        if( m_aObjectStack.empty() )
            parseError( "array end without begin", first );
        else if( dynamic_cast<PDFArray*>(m_aObjectStack.back()) == nullptr )
            parseError( "spurious array end", first );
        else
            m_aObjectStack.pop_back();
    }

    void emitStream( iteratorT first, iteratorT last )
    {
        if( m_aObjectStack.empty() )
            parseError( "stream without object", first );
        PDFObject* pObj = dynamic_cast<PDFObject*>(m_aObjectStack.back());
        if( pObj && pObj->m_pObject )
        {
            if( pObj->m_pStream )
                parseError( "multiple streams in object", first );

            PDFDict* pDict = dynamic_cast<PDFDict*>(pObj->m_pObject);
            if( pDict )
            {
                PDFStream* pStream = new PDFStream( first - m_aGlobalBegin, last - m_aGlobalBegin, pDict );

                pObj->m_pStream = pStream;
                pObj->m_aSubElements.emplace_back( pStream );
            }
        }
        else
            parseError( "stream without object", first );
    }

    void beginTrailer( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        if( m_aObjectStack.empty() )
            m_aObjectStack.push_back( new PDFPart() );

        PDFTrailer* pTrailer = new PDFTrailer();
        pTrailer->m_nOffset = first - m_aGlobalBegin;

        PDFContainer* pContainer = dynamic_cast<PDFContainer*>(m_aObjectStack.back());
        if( pContainer &&
            ( dynamic_cast<PDFFile*>(pContainer) ||
              dynamic_cast<PDFPart*>(pContainer) ) )
        {
            pContainer->m_aSubElements.emplace_back( pTrailer );
            m_aObjectStack.push_back( pTrailer );
        }
        else
            parseError( "trailer in wrong place", first );
    }

    void endTrailer( iteratorT first, SAL_UNUSED_PARAMETER iteratorT )
    {
        if( m_aObjectStack.empty() )
            parseError( "%%EOF without trailer", first );
        else if( dynamic_cast<PDFTrailer*>(m_aObjectStack.back()) == nullptr )
            parseError( "spurious %%EOF", first );
        else
            m_aObjectStack.pop_back();
    }
};

#ifdef _WIN32
PDFEntry* PDFReader::read( const char* pBuffer, unsigned int nLen )
{
    PDFGrammar<const char*> aGrammar( pBuffer );

    try
    {
#if OSL_DEBUG_LEVEL > 0
        boost::spirit::parse_info<const char*> aInfo =
#endif
            boost::spirit::parse( pBuffer,
                                  pBuffer+nLen,
                                  aGrammar,
                                  boost::spirit::space_p );
#if OSL_DEBUG_LEVEL > 0
        SAL_INFO("sdext.pdfimport.pdfparse", "parseinfo: stop = " << aInfo.stop << " (buff=" << pBuffer << ", offset = " << aInfo.stop - pBuffer << "), hit = " << (aInfo.hit ? OUString("true") : OUString("false")) << ", full = " << (aInfo.full ? OUString("true") : OUString("false")) << ", length = " << static_cast<int>(aInfo.length) );
#endif
    }
    catch( const parser_error<const char*, const char*>& rError )
    {
#if OSL_DEBUG_LEVEL > 0
        OString aTmp;
        unsigned int nElem = aGrammar.m_aObjectStack.size();
        for( unsigned int i = 0; i < nElem; i++ )
            aTmp += "   " + OString(typeid( *(aGrammar.m_aObjectStack[i]) ).name());

        SAL_WARN("sdext.pdfimport.pdfparse", "parse error: " << rError.descriptor << " at buffer pos " << rError.where - pBuffer << ", object stack: " << aTmp);
#else
        (void)rError;
#endif
    }

    PDFEntry* pRet = nullptr;
    unsigned int nEntries = aGrammar.m_aObjectStack.size();
    if( nEntries == 1 )
    {
        pRet = aGrammar.m_aObjectStack.back();
        aGrammar.m_aObjectStack.pop_back();
    }
#if OSL_DEBUG_LEVEL > 0
    else if( nEntries > 1 )
        SAL_WARN("sdext.pdfimport.pdfparse", "error got " << nEntries << " stack objects in parse" );
#endif

    return pRet;
}
#endif

PDFEntry* PDFReader::read( const char* pFileName )
{
#ifdef _WIN32
    /* #i106583#
       since converting to boost 1.39 file_iterator does not work anymore on all Windows systems
       C++ stdlib istream_iterator does not allow "-" apparently
       using spirit 2.0 doesn't work in our environment with the MSC

       So for the time being bite the bullet and read the whole file.
       FIXME: give Spirit 2.x another try when we upgrade boost again.
    */
    PDFEntry* pRet = nullptr;
    FILE* fp = fopen( pFileName, "rb" );
    if( fp )
    {
        fseek( fp, 0, SEEK_END );
        unsigned int nLen = static_cast<unsigned int>(ftell( fp ));
        fseek( fp, 0, SEEK_SET );
        char* pBuf = static_cast<char*>(std::malloc( nLen ));
        if( pBuf )
        {
            fread( pBuf, 1, nLen, fp );
            pRet = read( pBuf, nLen );
            std::free( pBuf );
        }
        fclose( fp );
    }
    return pRet;
#else
    file_iterator<> file_start( pFileName );
    if( ! file_start )
        return nullptr;
    file_iterator<> file_end = file_start.make_end();
    PDFGrammar< file_iterator<> > aGrammar( file_start );

    try
    {
#if OSL_DEBUG_LEVEL > 0
        boost::spirit::parse_info< file_iterator<> > aInfo =
#endif
            boost::spirit::parse( file_start,
                                  file_end,
                                  aGrammar,
                                  boost::spirit::space_p );
#if OSL_DEBUG_LEVEL > 0
        SAL_INFO("sdext.pdfimport.pdfparse", "parseinfo: stop at offset = " << aInfo.stop - file_start << ", hit = " << (aInfo.hit ? "true" : "false") << ", full = " << (aInfo.full ? "true" : "false") << ", length = " << aInfo.length);
#endif
    }
    catch( const parser_error< const char*, file_iterator<> >& rError )
    {
        SAL_WARN("sdext.pdfimport.pdfparse", "parse error: " << rError.descriptor << " at buffer pos " << rError.where - file_start);
#if OSL_DEBUG_LEVEL > 0
        OUStringBuffer aTmp;
        unsigned int nElem = aGrammar.m_aObjectStack.size();
        for( unsigned int i = 0; i < nElem; i++ )
        {
            aTmp.append("   ");
            aTmp.appendAscii(typeid( *(aGrammar.m_aObjectStack[i]) ).name());
        }
        SAL_WARN("sdext.pdfimport.pdfparse", "parse error object stack: " << aTmp.makeStringAndClear());
#endif
    }

    PDFEntry* pRet = nullptr;
    unsigned int nEntries = aGrammar.m_aObjectStack.size();
    if( nEntries == 1 )
    {
        pRet = aGrammar.m_aObjectStack.back();
        aGrammar.m_aObjectStack.pop_back();
    }
#if OSL_DEBUG_LEVEL > 0
    else if( nEntries > 1 )
    {
        SAL_WARN("sdext.pdfimport.pdfparse", "error got " << nEntries << " stack objects in parse");
        for( unsigned int i = 0; i < nEntries; i++ )
        {
            SAL_WARN("sdext.pdfimport.pdfparse", typeid(*aGrammar.m_aObjectStack[i]).name());
            PDFObject* pObj = dynamic_cast<PDFObject*>(aGrammar.m_aObjectStack[i]);
            if( pObj )
                SAL_WARN("sdext.pdfimport.pdfparse", "   -> object " << pObj->m_nNumber << " generation " << pObj->m_nGeneration);
            else
                SAL_WARN("sdext.pdfimport.pdfparse", "(type " << typeid(*aGrammar.m_aObjectStack[i]).name() << ")");
        }
    }
#endif
    return pRet;
#endif // WIN32
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
