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

#ifndef METAINFOREADER_HXX_INCLUDED
#define METAINFOREADER_HXX_INCLUDED

#include "internal/basereader.hxx"
#include "internal/types.hxx"

class ITag;
class CKeywordsTag;
class CSimpleTag;
class CDummyTag;

class CMetaInfoReader : public CBaseReader
{
public:
    virtual ~CMetaInfoReader();

    CMetaInfoReader( const std::string& DocumentName );

    CMetaInfoReader( StreamInterface* stream );

    /** check if the Tag is in the target meta.xml file.

        @param TagName
        the name of the tag that will be retrive.
    */
    bool hasTag(std::wstring TagName) const;


    /** Get a specific tag content, compound tags will be returned as comma separated list.

        @param TagName
        the name of the tag that will be retrive.
    */
    std::wstring getTagData( const std::wstring& TagName);

    /** check if the a tag has the specific attribute.

        @param TagName
        the name of the tag.
        @param AttributeName
        the name of the attribute.
    */
    bool hasTagAttribute( const std::wstring TagName,  std::wstring AttributeName);

    /** Get a specific attribute content.

        @param TagName
        the name of the tag.
        @param AttributeName
        the name of the attribute.
    */
    std::wstring getTagAttribute( const std::wstring TagName,  std::wstring AttributeName);

    /** Get the default language of the whole document.
    */
    LocaleSet_t getDefaultLocale( );

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

    /** save the received content into structure.

        @param tag_name
        the name of the tag.
    */
    void saveTagContent( const std::wstring& tag_name );

private:
    XmlTags_t      m_AllMetaInfo;

private:
    std::stack<ITag*> m_TagBuilderStack;

private:
    CKeywordsTag* m_pKeywords_Builder;
    CDummyTag*   m_pDummy_Builder;
    CSimpleTag* m_pSimple_Builder;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
