/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: metainforeader.hxx,v $
 * $Revision: 1.4 $
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