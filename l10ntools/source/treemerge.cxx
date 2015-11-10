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
#include "treemerge.hxx"


namespace
{
    // Extract strings from nodes on all level recursively
    static void lcl_ExtractLevel(
        const xmlDocPtr pSource, const xmlNodePtr pRoot,
        const xmlChar* pNodeName, PoOfstream& rPOStream )
    {
        if( !pRoot->children )
        {
            return;
        }
        for( xmlNodePtr pCurrent = pRoot->children->next;
            pCurrent; pCurrent = pCurrent->next)
        {
            if (!xmlStrcmp(pCurrent->name, pNodeName))
            {
                xmlChar* pID = xmlGetProp(pCurrent, reinterpret_cast<const xmlChar*>("id"));
                xmlChar* pText =
                    xmlGetProp(pCurrent, reinterpret_cast<const xmlChar*>("title"));

                common::writePoEntry(
                    "Treex", rPOStream, pSource->name, helper::xmlStrToOString( pNodeName ),
                    helper::xmlStrToOString( pID ), OString(), OString(), helper::xmlStrToOString( pText ));

                xmlFree( pID );
                xmlFree( pText );

                lcl_ExtractLevel(
                    pSource, pCurrent, reinterpret_cast<const xmlChar *>("node"),
                    rPOStream );
            }
        }
    }

    // Update id and content of the topic
    static xmlNodePtr lcl_UpdateTopic(
        const xmlNodePtr pCurrent, const OString& rXhpRoot )
    {
        xmlNodePtr pReturn = pCurrent;
        xmlChar* pID = xmlGetProp(pReturn, reinterpret_cast<const xmlChar*>("id"));
        const OString sID =
            helper::xmlStrToOString( pID );
        xmlFree( pID );

        const sal_Int32 nFirstSlash = sID.indexOf('/');
        // Update id attribute of topic
        {
            OString sNewID =
                sID.copy( 0, nFirstSlash + 1 ) +
                rXhpRoot.copy( rXhpRoot.lastIndexOf('/') + 1 ) +
                sID.copy( sID.indexOf( '/', nFirstSlash + 1 ) );
            xmlSetProp(
                pReturn, reinterpret_cast<const xmlChar*>("id"),
                reinterpret_cast<const xmlChar*>(sNewID.getStr()));
        }

        const OString sXhpPath =
            rXhpRoot +
            sID.copy(sID.indexOf('/', nFirstSlash + 1));
        xmlDocPtr pXhpFile = xmlParseFile( sXhpPath.getStr() );
        // if xhpfile is missing than put this topic into comment
        if ( !pXhpFile )
        {
            xmlNodePtr pTemp = pReturn;
            xmlChar* sNewID =
                xmlGetProp(pReturn, reinterpret_cast<const xmlChar*>("id"));
            xmlChar* sComment =
                xmlStrcat( xmlCharStrdup("removed "), sNewID );
            pReturn = xmlNewComment( sComment );
            xmlReplaceNode( pTemp, pReturn );
            xmlFree( pTemp );
            xmlFree( sNewID );
            xmlFree( sComment );
        }
        // update topic's content on the basis of xhpfile's title
        else
        {
            xmlNodePtr pXhpNode = xmlDocGetRootElement( pXhpFile );
            for( pXhpNode = pXhpNode->children;
                pXhpNode; pXhpNode = pXhpNode->children )
            {
                while( pXhpNode->type != XML_ELEMENT_NODE )
                {
                    pXhpNode = pXhpNode->next;
                }
                if(!xmlStrcmp(pXhpNode->name, reinterpret_cast<const xmlChar *>("title")))
                {
                    xmlChar* sTitle =
                        xmlNodeListGetString(pXhpFile, pXhpNode->children, 1);
                    OString sNewTitle =
                        helper::xmlStrToOString( sTitle ).
                            replaceAll("$[officename]","%PRODUCTNAME").
                                replaceAll("$[officeversion]","%PRODUCTVERSION");
                    xmlNodeSetContent(
                        pReturn,
                        xmlEncodeSpecialChars( nullptr,
                            reinterpret_cast<const xmlChar*>(
                                sNewTitle.getStr() )));
                    xmlFree( sTitle );
                    break;
                }
            }
            if( !pXhpNode )
            {
                std::cerr
                    << "Treex error: Cannot find title in "
                    << sXhpPath.getStr() << std::endl;
                return nullptr;
            }
            xmlFree( pXhpFile );
            xmlCleanupParser();
        }
        return pReturn;
    }
    // Localize title attribute of help_section and node tags
    static void lcl_MergeLevel(
        xmlDocPtr io_pSource, const xmlNodePtr pRoot,
        const xmlChar * pNodeName, MergeDataFile* pMergeDataFile,
        const OString& rLang, const OString& rXhpRoot )
    {
        if( !pRoot->children )
        {
            return;
        }
        for( xmlNodePtr pCurrent = pRoot->children;
            pCurrent; pCurrent = pCurrent->next)
        {
            if( !xmlStrcmp(pCurrent->name, pNodeName) )
            {
                if( rLang != "en-US" )
                {
                    OString sNewText;
                    xmlChar* pID = xmlGetProp(pCurrent, reinterpret_cast<const xmlChar*>("id"));
                    ResData  aResData(
                        helper::xmlStrToOString( pID ),
                        static_cast<OString>(io_pSource->name) );
                    xmlFree( pID );
                    aResData.sResTyp = helper::xmlStrToOString( pNodeName );
                    if( pMergeDataFile )
                    {
                        MergeEntrys* pEntrys =
                            pMergeDataFile->GetMergeEntrys( &aResData );
                        if( pEntrys )
                        {
                            pEntrys->GetText( sNewText, STRING_TYP_TEXT, rLang );
                        }
                    }
                    else if( rLang == "qtz" )
                    {
                        xmlChar* pText = xmlGetProp(pCurrent, reinterpret_cast<const xmlChar*>("title"));
                        const OString sOriginText = helper::xmlStrToOString(pText);
                        xmlFree( pText );
                        sNewText = MergeEntrys::GetQTZText(aResData, sOriginText);
                    }
                    if( !sNewText.isEmpty() )
                    {
                        xmlSetProp(
                            pCurrent, reinterpret_cast<const xmlChar*>("title"),
                            reinterpret_cast<const xmlChar*>(sNewText.getStr()));
                    }
                }

                lcl_MergeLevel(
                    io_pSource, pCurrent, reinterpret_cast<const xmlChar *>("node"),
                    pMergeDataFile, rLang, rXhpRoot );
            }
            else if( !xmlStrcmp(pCurrent->name, reinterpret_cast<const xmlChar *>("topic")) )
            {
                pCurrent = lcl_UpdateTopic( pCurrent, rXhpRoot );
            }
        }
    }
}

TreeParser::TreeParser(
    const OString& rInputFile, const OString& rLang )
    : m_pSource( nullptr )
    , m_sLang( rLang )
    , m_bIsInitialized( false )
{
    m_pSource = xmlParseFile( rInputFile.getStr() );
    if ( !m_pSource ) {
        std::cerr
            << "Treex error: Cannot open source file: "
            << rInputFile.getStr() << std::endl;
        return;
    }
    if( !m_pSource->name )
    {
        m_pSource->name = static_cast<char *>(xmlMalloc(strlen(rInputFile.getStr())+1));
        strcpy( m_pSource->name, rInputFile.getStr() );
    }
    m_bIsInitialized = true;
}

TreeParser::~TreeParser()
{
}

void TreeParser::Extract( const OString& rPOFile )
{
    assert( m_bIsInitialized );
    PoOfstream aPOStream( rPOFile, PoOfstream::APP );
    if( !aPOStream.isOpen() )
    {
        std::cerr
            << "Treex error: Cannot open po file for extract: "
            << rPOFile.getStr() << std::endl;
        return;
    }

    xmlNodePtr pRootNode = xmlDocGetRootElement( m_pSource );
    lcl_ExtractLevel(
        m_pSource, pRootNode, reinterpret_cast<const xmlChar *>("help_section"),
        aPOStream );

    xmlFreeDoc( m_pSource );
    xmlCleanupParser();
    aPOStream.close();
    m_bIsInitialized = false;
}

void TreeParser::Merge(
    const OString &rMergeSrc, const OString &rDestinationFile,
    const OString &rXhpRoot )
{
    assert( m_bIsInitialized );

    const xmlNodePtr pRootNode = xmlDocGetRootElement( m_pSource );
    MergeDataFile* pMergeDataFile = nullptr;
    if( m_sLang != "qtz" && m_sLang != "en-US" )
    {
        pMergeDataFile = new MergeDataFile(
            rMergeSrc, static_cast<OString>( m_pSource->name ), false, false );
        const std::vector<OString> vLanguages = pMergeDataFile->GetLanguages();
        if( vLanguages.size()>=1 && vLanguages[0] != m_sLang )
        {
            std::cerr
                << ("Treex error: given language conflicts with language of"
                    " Mergedata file: ")
                << m_sLang.getStr() << " - "
                << vLanguages[0].getStr() << std::endl;
            delete pMergeDataFile;
            return;
        }
    }
    lcl_MergeLevel(
        m_pSource, pRootNode, reinterpret_cast<const xmlChar *>("help_section"),
        pMergeDataFile, m_sLang, rXhpRoot );

    delete pMergeDataFile;
    xmlSaveFile( rDestinationFile.getStr(), m_pSource );
    xmlFreeDoc( m_pSource );
    xmlCleanupParser();
    m_bIsInitialized = false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
