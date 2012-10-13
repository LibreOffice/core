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

#ifndef CONTENTREADER_HXX_INCLUDED
#define CONTENTREADER_HXX_INCLUDED

#include "internal/basereader.hxx"

class ITag;
class StreamInterface;

class CContentReader : public CBaseReader
{
public:
    virtual ~CContentReader();

    CContentReader( const std::string& DocumentName, LocaleSet_t const & DocumentLocale );

    CContentReader( StreamInterface* stream, LocaleSet_t const & DocumentLocale );


    /** Get the chunkbuffer.

        @return
        the chunkbuffer of the document.
    */
    inline ChunkBuffer_t const & getChunkBuffer( ) const{ return m_ChunkBuffer; };

protected: // protected because its only an implementation relevant class

    /** start_element occurs when a tag is start.

        @param raw_name
        raw name of the tag.
        @param local_name
        local name of the tag.
        @param attributes
        attribute structure.
    */
    virtual void start_element(
        const std::wstring& raw_name,
        const std::wstring& local_name,
        const XmlTagAttributes_t& attributes);

    /** end_element occurs when a tag is closed

        @param raw_name
        raw name of the tag.
        @param local_name
        local name of the tag.
    */
    virtual void end_element(
        const std::wstring& raw_name, const std::wstring& local_name);

    /** characters occurs when receiving characters

        @param character
        content of the information received.
    */
    virtual void characters(const std::wstring& character);

protected:
    /** choose an appropriate tag reader to handle the tag.

        @param tag_name
        the name of the tag.
        @param XmlAttributes
        attribute structure of the tag to save in.
    */
    ITag* chooseTagReader(
        const std::wstring& tag_name, const XmlTagAttributes_t& XmlAttributes );

    /** Get the list of style locale pair.

        @return
        the Style-Locale map
    */
    inline StyleLocaleMap_t const & getStyleMap( ) const{ return m_StyleMap; };

    /** get style of the current content.

        @return style of the current content.
    */
    ::std::wstring getCurrentContentStyle( void );

    /** add chunk into Chunk Buffer.
    */
    void addChunk( LocaleSet_t const & Locale, Content_t const & Content );

    /** get a style's locale field.
    */
    LocaleSet_t const & getLocale( const StyleName_t Style );

private:
    std::stack<ITag*> m_TagBuilderStack;

    ChunkBuffer_t   m_ChunkBuffer;
    StyleLocaleMap_t      m_StyleMap;
    LocaleSet_t m_DefaultLocale;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
