/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlstring.h>

#include "export.hxx"
#include "helper.hxx"
#include "common.hxx"
#include "po.hxx"
#include "stringmerge.hxx"

//Parse strings.xml file
StringParser::StringParser(
    const OString& rInputFile, const OString& rLang )
    : m_pSource( 0 )
    , m_sLang( rLang )
    , m_bIsInitialized( false )
{
    m_pSource = xmlParseFile( rInputFile.getStr() );
    if ( !m_pSource ) {
        std::cerr
            << "Stringx error: Cannot open source file: "
            << rInputFile.getStr() << std::endl;
        return;
    }
    if( !m_pSource->name )
    {
        m_pSource->name = new char[strlen(rInputFile.getStr())+1];
        strcpy( m_pSource->name, rInputFile.getStr() );
    }
    m_bIsInitialized = true;
}

StringParser::~StringParser()
{
}

//Extract strings form source file
void StringParser::Extract( const OString& rPOFile )
{
    assert( m_bIsInitialized );
    PoOfstream aPOStream( rPOFile, PoOfstream::APP );
    if( !aPOStream.isOpen() )
    {
        std::cerr
            << "stringex error: Cannot open po file for extract: "
            << rPOFile.getStr() << std::endl;
        return;
    }

    xmlNodePtr pRootNode = xmlDocGetRootElement( m_pSource ); // <resource>
    for( xmlNodePtr pCurrent = pRootNode->children->next;
        pCurrent; pCurrent = pCurrent->next)
    {
        if (!xmlStrcmp(pCurrent->name, (const xmlChar*)("string")))
        {
            xmlChar* pID = xmlGetProp(pCurrent, (const xmlChar*)("name"));
            xmlChar* pText = xmlNodeGetContent(pCurrent);
            const OString sTemp =
                helper::unEscapeAll(helper::xmlStrToOString( pText ),"\\n""\\t","\n""\t");
            common::writePoEntry(
                "Stringex", aPOStream, m_pSource->name, "string",
                helper::xmlStrToOString( pID ), OString(), OString(),
                sTemp);

            xmlFree( pID );
            xmlFree( pText );
        }
    }

    xmlFreeDoc( m_pSource );
    xmlCleanupParser();
    aPOStream.close();
    m_bIsInitialized = false;
}

//Merge strings to localized strings.xml file
void StringParser::Merge(
    const OString &rMergeSrc, const OString &rDestinationFile )
{
    assert( m_bIsInitialized );

    if( (m_sLang == "en-US") || (m_sLang == "qtz") )
    {
        return;
    }

    MergeDataFile aMergeDataFile(
        rMergeSrc, static_cast<OString>( m_pSource->name ), false );
    const std::vector<OString> vLanguages = aMergeDataFile.GetLanguages();
    if( vLanguages.size()>=2 &&
        vLanguages[vLanguages[0]=="qtz" ? 1 : 0] != m_sLang )
    {
        std::cerr
            << "stringex error: given language conflicts with "
            << "language of Mergedata file: "
            << m_sLang.getStr() << " - "
            << vLanguages[vLanguages[0]=="qtz" ? 1 : 0].getStr() << std::endl;
        return;
    }

    xmlNodePtr pRootNode = xmlDocGetRootElement( m_pSource ); //<resource>

    for( xmlNodePtr pCurrent = pRootNode->children;
        pCurrent; pCurrent = pCurrent->next)
    {
        if (!xmlStrcmp(pCurrent->name, (const xmlChar*)("string")))
        {
            xmlChar* pID = xmlGetProp(pCurrent, (const xmlChar*)("name"));
            ResData  aResData(
                helper::xmlStrToOString( pID ),
                static_cast<OString>(m_pSource->name) );
            xmlFree( pID );
            aResData.sResTyp = "string";
            MergeEntrys* pEntrys =
                (&aMergeDataFile)->GetMergeEntrys( &aResData );
            if( pEntrys )
            {
                OString sNewText;
                pEntrys->GetText( sNewText, STRING_TYP_TEXT, m_sLang );
                sNewText = helper::escapeAll(sNewText, "\n""\t""\'""\"","\\n""\\t""\\\'""\\\"");
                xmlNodeSetContent(
                    pCurrent,
                    xmlEncodeSpecialChars( NULL,
                        reinterpret_cast<const xmlChar*>(
                            sNewText.getStr() )));
            }
        }
    }

    xmlSaveFile( rDestinationFile.getStr(), m_pSource );
    xmlFreeDoc( m_pSource );
    xmlCleanupParser();
    m_bIsInitialized = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
