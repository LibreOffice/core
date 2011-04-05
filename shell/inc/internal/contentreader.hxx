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

#ifndef CONTENTREADER_HXX_INCLUDED
#define CONTENTREADER_HXX_INCLUDED

#include "internal/basereader.hxx"

class ITag;

class CContentReader : public CBaseReader
{
public:
    virtual ~CContentReader();

    CContentReader( const std::string& DocumentName, LocaleSet_t const & DocumentLocale );

    CContentReader( void* stream, LocaleSet_t const & DocumentLocale, zlib_filefunc_def* fa );


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
