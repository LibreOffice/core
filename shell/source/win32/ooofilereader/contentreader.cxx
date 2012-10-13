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

#undef OSL_DEBUG_LEVEL


#include <osl/diagnose.h>

#include "internal/contentreader.hxx"
#include "dummytag.hxx"
#include "simpletag.hxx"
#include "autostyletag.hxx"

#include "assert.h"

/** constructor.
*/
CContentReader::CContentReader( const std::string& DocumentName, LocaleSet_t const & DocumentLocale ):
CBaseReader( DocumentName )
{
    try
    {
        m_DefaultLocale = DocumentLocale;
        Initialize( DOC_CONTENT_NAME );
    }
    catch(xml_parser_exception&
    #if OSL_DEBUG_LEVEL > 0
    ex
    #endif
    )
    {
        OSL_ENSURE(false, ex.what());
    }
    catch(...)
    {
        OSL_ENSURE(false, "Unknown error");
    }
}

CContentReader::CContentReader( StreamInterface* stream, LocaleSet_t const & DocumentLocale ) :
CBaseReader( stream )
{
try
    {
        m_DefaultLocale = DocumentLocale;
        Initialize( DOC_CONTENT_NAME );
    }
    catch(xml_parser_exception&
    #if OSL_DEBUG_LEVEL > 0
    ex
    #endif
    )
    {
        OSL_ENSURE(false, ex.what());
    }
    catch(...)
    {
        OSL_ENSURE(false, "Unknown error");
    }
}


/** destructor.
*/

CContentReader::~CContentReader( void )
{
}

/***********************   helper functions  ***********************/

/** choose an appropriate tag reader
*/

ITag* CContentReader::chooseTagReader( const std::wstring& tag_name, const XmlTagAttributes_t& XmlAttributes )
{
    if (( tag_name == CONTENT_TEXT_A )||( tag_name == CONTENT_TEXT_P )||
        ( tag_name == CONTENT_TEXT_SPAN ) ||( tag_name == CONTENT_TEXT_H )||
        ( tag_name == CONTENT_TEXT_SEQUENCE ) ||( tag_name == CONTENT_TEXT_BOOKMARK_REF )||
        ( tag_name == CONTENT_TEXT_INDEX_TITLE_TEMPLATE ) )
        return new CSimpleTag(XmlAttributes);
    else if ( tag_name == CONTENT_STYLE_STYLE )
    {
        // if style:style | style:name is exist,, fill the style field, otherwise do nothing;
        if  ( XmlAttributes.find(CONTENT_STYLE_STYLE_NAME) != XmlAttributes.end())
            return new CAutoStyleTag(XmlAttributes);
       else
            return new CDummyTag();
    }
    else if ( ( tag_name == CONTENT_STYLE_PROPERTIES ) || ( tag_name == CONTENT_TEXT_STYLE_PROPERTIES ) )
    {
        assert( !m_TagBuilderStack.empty() );

        //here we presume that if CONTENT_STYLE_PROPERTIES tag is present, it just follow CONTENT_STYLE_STYLE;
        ITag* pTagBuilder = m_TagBuilderStack.top();
        pTagBuilder->addAttributes( XmlAttributes );

        return new CDummyTag();
    }
    else
        return new CDummyTag();
}

/** get style of the current content.
*/
::std::wstring CContentReader::getCurrentContentStyle( void )
{
    assert( !m_TagBuilderStack.empty() );
    ITag* pTagBuilder = m_TagBuilderStack.top();

    return ( pTagBuilder->getTagAttribute(CONTENT_TEXT_STYLENAME) );
}

/** add chunk into Chunk Buffer.
*/
void CContentReader::addChunk( LocaleSet_t const & Locale, Content_t const & Content )
{
    if ( Content == EMPTY_STRING )
        return;

    if ( ( ( m_ChunkBuffer.empty() ) || ( m_ChunkBuffer.back().first != Locale ) ) &&
         ( ( Content != SPACE )  && ( Content != LF ) ) )
    {
        // if met a new locale, add a blank new chunk;
        Chunk_t Chunk;
        Chunk.first = Locale;
        Chunk.second = EMPTY_STRING;
        m_ChunkBuffer.push_back( Chunk );
    }

    if ( !m_ChunkBuffer.empty() )
        m_ChunkBuffer.back().second += Content;
}

/** get a style's locale field.
*/

LocaleSet_t const & CContentReader::getLocale( const StyleName_t Style )
{
    if ( m_StyleMap.empty() )
        return m_DefaultLocale;

    StyleLocaleMap_t :: const_iterator style_Iter;

    if ( ( style_Iter = m_StyleMap.find( Style ) ) == m_StyleMap.end( ) )
        return m_DefaultLocale;
    else
        return style_Iter->second;

}

/***********************   event handler functions  ***********************/

//------------------------------
// start_element occurs when a tag is start
//------------------------------

void CContentReader::start_element(
    const std::wstring& /*raw_name*/,
    const std::wstring& local_name,
    const XmlTagAttributes_t& attributes)
{
    //get appropriate Xml Tag Builder using MetaInfoBuilderFactory;
    ITag* pTagBuilder = chooseTagReader( local_name,attributes );
    assert( pTagBuilder != NULL );
    pTagBuilder->startTag( );
    m_TagBuilderStack.push( pTagBuilder );

}

//------------------------------
// end_element occurs when a tag is closed
//------------------------------

void CContentReader::end_element(const std::wstring& /*raw_name*/, const std::wstring& local_name)
{
    assert( !m_TagBuilderStack.empty() );
    ITag* pTagBuilder = m_TagBuilderStack.top();

    if ( local_name == CONTENT_STYLE_STYLE )
    {
        StyleLocalePair_t StyleLocalePair = static_cast<CAutoStyleTag * >( pTagBuilder)->getStyleLocalePair();
        if ( ( static_cast<CAutoStyleTag * >( pTagBuilder)->isFull() ) && ( StyleLocalePair.second != m_DefaultLocale ) )
                m_StyleMap.insert( StyleLocalePair );
    }
    if (( local_name == CONTENT_TEXT_A )||( local_name == CONTENT_TEXT_SPAN ) ||
        ( local_name == CONTENT_TEXT_SEQUENCE )||( local_name == CONTENT_TEXT_BOOKMARK_REF ))
        addChunk( getLocale( getCurrentContentStyle() ), ::std::wstring( SPACE ) );
    if ((( local_name == CONTENT_TEXT_P )||( local_name == CONTENT_TEXT_H ) ||
         ( local_name == CONTENT_TEXT_INDEX_TITLE_TEMPLATE ) )&&
        ( EMPTY_STRING != pTagBuilder->getTagContent( ) ) )
        addChunk( getLocale( getCurrentContentStyle() ), ::std::wstring( LF ) );

    m_TagBuilderStack.pop();
    pTagBuilder->endTag();
    delete pTagBuilder;

}

//------------------------------
// characters occurs when receiving characters
//------------------------------

void CContentReader::characters( const std::wstring& character )
{
    if ( character.length() > 0 && !HasOnlySpaces( character ) )
    {
        addChunk( getLocale( getCurrentContentStyle() ), ::std::wstring( character ) );

        ITag* pTagBuilder = m_TagBuilderStack.top();
        pTagBuilder->addCharacters( character );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
