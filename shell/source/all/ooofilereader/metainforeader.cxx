/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: metainforeader.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
#include "internal/metainforeader.hxx"
#include "dummytag.hxx"
#include "simpletag.hxx"
#include "keywordstag.hxx"

#include "assert.h"

/** constructor.
*/
CMetaInfoReader::CMetaInfoReader( const std::string& DocumentName ):
CBaseReader( DocumentName )
{
    try
    {
        m_pKeywords_Builder = new CKeywordsTag( );
        m_pSimple_Builder = new CSimpleTag( );
        m_pDummy_Builder   = new CDummyTag( );

        //retrieve all infomation that is useful
        m_AllMetaInfo[META_INFO_AUTHOR]               = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_TITLE]                = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_SUBJECT]              = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_KEYWORDS]             = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_DESCRIPTION]          = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_DOCUMENT_STATISTIC]   = EMPTY_XML_TAG;

        m_AllMetaInfo[META_INFO_GENERATOR]            = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_CREATION]             = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_CREATOR]              = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_MODIFIED]             = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_LANGUAGE]             = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_DOCUMENT_NUMBER]      = EMPTY_XML_TAG;
        m_AllMetaInfo[META_INFO_EDITING_TIME]         = EMPTY_XML_TAG;

        Initialize( META_CONTENT_NAME );
    }
    catch(xml_parser_exception&
    #if OSL_DEBUG_LEVEL > 0
    ex
    #endif
    )
    {
        ENSURE(false, ex.what());
    }
    catch(...)
    {
        ENSURE(false, "Unknown error");
    }
}

/** destructor.
*/

CMetaInfoReader::~CMetaInfoReader( void )
{
    delete m_pKeywords_Builder;
    delete m_pSimple_Builder;
    delete m_pDummy_Builder;
}


/***********************   export functions  ***********************/

/** check if the Tag is in the target meta.xml file.

    @param TagName
    the name of the tag that will be retrive.
*/
bool CMetaInfoReader::hasTag( std::wstring TagName ) const
{
    return ( m_AllMetaInfo.find(TagName) != m_AllMetaInfo.end());
}

/** Get a specific tag content, compound tags will be returned as comma separated list.

    @param TagName
    the name of the tag that will be retrive.
*/
std::wstring CMetaInfoReader::getTagData( const std::wstring& TagName)
{
    if( hasTag( TagName ) )
        return m_AllMetaInfo[TagName].first;
    else
        return EMPTY_STRING;
}

/** check if the a tag has the specific attribute.

    @param TagName
    the name of the tag.
    @param AttributeName
    the name of the attribute.
*/
bool CMetaInfoReader::hasTagAttribute( const std::wstring TagName,  std::wstring AttributeName)
{
    return ( m_AllMetaInfo[TagName].second.find( AttributeName) != m_AllMetaInfo[TagName].second.end() );
}

/** Get a specific attribute content.

    @param TagName
    the name of the tag.
    @param AttributeName
    the name of the attribute.
*/
std::wstring CMetaInfoReader::getTagAttribute( const std::wstring TagName,  std::wstring AttributeName)
{
    if ( hasTagAttribute( TagName, AttributeName ) )
        return  m_AllMetaInfo[ TagName ].second[AttributeName];
    else
        return EMPTY_STRING;
}

/** helper function for getDefaultLocale().
*/
const LocaleSet_t EN_US_LOCALE( ::std::make_pair( ::std::wstring( L"en" ),  ::std::wstring( L"US" )));

bool isValidLocale ( ::std::wstring Locale )
{
    return ( Locale.length() == 5 );
}

/** Get the default language of the whole document, if no such field, en-US is returned.
*/
LocaleSet_t CMetaInfoReader::getDefaultLocale()
{
    if (hasTag(META_INFO_LANGUAGE))
    {
        ::std::wstring locale = m_AllMetaInfo[META_INFO_LANGUAGE].first;
        return isValidLocale(locale) ? ::std::make_pair(locale.substr( 0,2), locale.substr( 3,2)) : EN_US_LOCALE;
    }
    else
        return EN_US_LOCALE;
}

/***********************   tag related functions  ***********************/

/** choose an appropriate tag reader
*/

ITag* CMetaInfoReader::chooseTagReader( const std::wstring& tag_name, const XmlTagAttributes_t& XmlAttributes )
{
    if ( tag_name == META_INFO_KEYWORD )
    {
        m_AllMetaInfo[META_INFO_KEYWORDS].second = XmlAttributes;
        return m_pKeywords_Builder;
    }
    if (( tag_name == META_INFO_TITLE )||( tag_name == META_INFO_AUTHOR )||( tag_name == META_INFO_SUBJECT )||( tag_name == META_INFO_DESCRIPTION )||
        ( tag_name == META_INFO_DOCUMENT_STATISTIC )||( tag_name == META_INFO_GENERATOR )||( tag_name == META_INFO_CREATION )||( tag_name == META_INFO_CREATOR )||
        ( tag_name == META_INFO_MODIFIED )||( tag_name == META_INFO_LANGUAGE )||( tag_name == META_INFO_DOCUMENT_NUMBER )||( tag_name == META_INFO_EDITING_TIME ) )
    {
        m_AllMetaInfo[tag_name].second = XmlAttributes;
        return m_pSimple_Builder;
    }
    else
        return m_pDummy_Builder;

}

//------------------------------
// save the received content into structure.
//------------------------------
void CMetaInfoReader::saveTagContent( const std::wstring& tag_name )
{
    ITag* pTagBuilder;
    if ( tag_name == META_INFO_KEYWORDS )
        pTagBuilder = m_pKeywords_Builder;
    else if ( tag_name == META_INFO_KEYWORD )
    {
        // added for support for OASIS xml file format.
        m_AllMetaInfo[META_INFO_KEYWORDS].first =m_pKeywords_Builder->getTagContent( );
        return;
    }
    else if (( tag_name == META_INFO_TITLE )||( tag_name == META_INFO_AUTHOR )||( tag_name == META_INFO_SUBJECT )||( tag_name == META_INFO_DESCRIPTION )||
        ( tag_name == META_INFO_DOCUMENT_STATISTIC )||( tag_name == META_INFO_GENERATOR )||( tag_name == META_INFO_CREATION )||( tag_name == META_INFO_CREATOR )||
        ( tag_name == META_INFO_MODIFIED )||( tag_name == META_INFO_LANGUAGE )||( tag_name == META_INFO_DOCUMENT_NUMBER )||( tag_name == META_INFO_EDITING_TIME ) )
        pTagBuilder = m_pSimple_Builder;
    else
        pTagBuilder = m_pDummy_Builder;

    m_AllMetaInfo[tag_name].first =pTagBuilder->getTagContent( );
}


/***********************   event handler functions  ***********************/

//------------------------------
// start_element occurs when a tag is start
//------------------------------

void CMetaInfoReader::start_element(
    const std::wstring& /*raw_name*/,
    const std::wstring& local_name,
    const XmlTagAttributes_t& attributes)
{
    //get appropriate Xml Tag Builder using MetaInfoBuilderFactory;
    ITag* pTagBuilder = chooseTagReader( local_name,attributes );
    assert( pTagBuilder!= NULL );
    pTagBuilder->startTag( );
    m_TagBuilderStack.push( pTagBuilder );

}

//------------------------------
// end_element occurs when a tag is closed
//------------------------------

void CMetaInfoReader::end_element(const std::wstring& /*raw_name*/, const std::wstring& local_name)
{
    assert( !m_TagBuilderStack.empty() );
    ITag* pTagBuilder = m_TagBuilderStack.top();
    m_TagBuilderStack.pop();
    pTagBuilder->endTag();

    saveTagContent( local_name );

}

//------------------------------
// characters occurs when receiving characters
//------------------------------

void CMetaInfoReader::characters( const std::wstring& character )
{
    if ( character.length() > 0 &&  !HasOnlySpaces( character ) )
    {
        ITag* pTagBuilder = m_TagBuilderStack.top();
        pTagBuilder->addCharacters( character );
    }
}

