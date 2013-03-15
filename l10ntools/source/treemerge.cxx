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
#include "common.hxx"
#include "treemerge.hxx"


namespace
{
    //Convert xmlChar* to OString
    static OString lcl_xmlStrToOString( const xmlChar* pString )
    {
        xmlChar* pTemp = xmlStrdup( pString );
        OString sResult =
            static_cast<OString>(reinterpret_cast<sal_Char*>( pTemp ));
        xmlFree( pTemp );
        return sResult;
    }

    //Extract strings from nodes on all level recursively
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
                xmlChar* pID = xmlGetProp(pCurrent, (const xmlChar*)("id"));
                xmlChar* pText =
                    xmlGetProp(pCurrent, (const xmlChar*)("title"));

                Export::writePoEntry(
                    "Treex", rPOStream, pSource->name, lcl_xmlStrToOString( pNodeName ),
                    lcl_xmlStrToOString( pID ), OString(), OString(), lcl_xmlStrToOString( pText ));

                xmlFree( pID );
                xmlFree( pText );

                lcl_ExtractLevel(
                    pSource, pCurrent, (const xmlChar *)("node"),
                    rPOStream );
            }
        }
    }

    //Update id and content of the topic
    static xmlNodePtr lcl_UpdateTopic(
        const xmlNodePtr pCurrent, const OString& rXhpRoot )
    {
        xmlNodePtr pReturn = pCurrent;
        xmlChar* pID = xmlGetProp(pReturn, (const xmlChar*)("id"));
        const OString sID =
            lcl_xmlStrToOString( pID );
        xmlFree( pID );

        const sal_Int32 nFirstSlash = sID.indexOf("/");
        //Update id attribute of topic
        {
            OString sNewID =
                sID.copy( 0, nFirstSlash + 1 ) +
                rXhpRoot.copy( rXhpRoot.lastIndexOf("/") + 1 ) +
                sID.copy( sID.indexOf( "/", nFirstSlash + 1 ) );
            xmlSetProp(
                pReturn, (const xmlChar*)("id"),
                reinterpret_cast<const xmlChar*>(sNewID.getStr()));
        }

        const OString sXhpPath =
            rXhpRoot +
            sID.copy(sID.indexOf("/", nFirstSlash + 1));
        xmlDocPtr pXhpFile = xmlParseFile( sXhpPath.getStr() );
        //if xhpfile is missing than we put this topic into comment
        if ( !pXhpFile )
        {
            xmlNodePtr pTemp = pReturn;
            xmlChar* sNewID =
                xmlGetProp(pReturn, (const xmlChar*)("id"));
            xmlChar* sComment =
                xmlStrcat( xmlCharStrdup("removed "), sNewID );
            pReturn = xmlNewComment( sComment );
            xmlReplaceNode( pTemp, pReturn );
            xmlFree( pTemp );
            xmlFree( sNewID );
            xmlFree( sComment );
        }
        //update topic's content on the basis of xhpfile's title
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
                if(!xmlStrcmp(pXhpNode->name, (const xmlChar *)("title")))
                {
                    xmlChar* sTitle =
                        xmlNodeListGetString(pXhpFile, pXhpNode->children, 1);
                    OString sNewTitle =
                        lcl_xmlStrToOString( sTitle ).
                            replaceAll("$[officename]","%PRODUCTNAME").
                                replaceAll("$[officeversion]","%PRODUCTVERSION");
                    xmlNodeSetContent(
                        pReturn,
                        xmlEncodeSpecialChars( NULL,
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
                return 0;
            }
            xmlFree( pXhpFile );
            xmlCleanupParser();
        }
        return pReturn;
    }
    //Localize title attribute of help_section and node tags
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
                if( pMergeDataFile )
                {
                    xmlChar* pID = xmlGetProp(pCurrent, (const xmlChar*)("id"));
                    ResData  aResData(
                        "", lcl_xmlStrToOString( pID ),
                        static_cast<OString>(io_pSource->name) );
                    xmlFree( pID );
                    aResData.sResTyp = lcl_xmlStrToOString( pNodeName );
                    PFormEntrys* pEntrys =
                        pMergeDataFile->GetPFormEntrys( &aResData );
                    if( pEntrys )
                    {
                        OString sNewText;
                        pEntrys->GetText( sNewText, STRING_TYP_TEXT, rLang );
                        xmlSetProp(
                            pCurrent, (const xmlChar*)("title"),
                            (const xmlChar*)(sNewText.getStr()));
                    }
                }
                lcl_MergeLevel(
                    io_pSource, pCurrent, (const xmlChar *)("node"),
                    pMergeDataFile, rLang, rXhpRoot );
            }
            else if( !xmlStrcmp(pCurrent->name, (const xmlChar *)("topic")) )
            {
                pCurrent = lcl_UpdateTopic( pCurrent, rXhpRoot );
            }
        }
    }
}

//Parse tree file
TreeParser::TreeParser(
    const OString& rInputFile, const OString& rLang )
    : m_pSource( 0 )
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
        m_pSource->name = new char[strlen(rInputFile.getStr())+1];
        strcpy( m_pSource->name, rInputFile.getStr() );
    }
    m_bIsInitialized = true;
}

TreeParser::~TreeParser()
{
}

//Extract strings form source file
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
        m_pSource, pRootNode, (const xmlChar *)("help_section"),
        aPOStream );

    xmlFreeDoc( m_pSource );
    xmlCleanupParser();
    aPOStream.close();
    m_bIsInitialized = false;
}

//Merge strings to tree file and update reference to help files(xhp)
void TreeParser::Merge(
    const OString &rMergeSrc, const OString &rDestinationFile,
    const OString &rXhpRoot )
{
    assert( m_bIsInitialized );

    const xmlNodePtr pRootNode = xmlDocGetRootElement( m_pSource );
    if( m_sLang == "en-US" )
    {
        lcl_MergeLevel(
            m_pSource, pRootNode, (const xmlChar *)("help_section"),
            0, m_sLang, rXhpRoot );
    }
    else
    {
        MergeDataFile aMergeDataFile(
            rMergeSrc, static_cast<OString>( m_pSource->name ), false, false );
        const std::vector<OString> vLanguages = aMergeDataFile.GetLanguages();
        if( vLanguages.size()>=2 &&
            vLanguages[vLanguages[0]=="qtz" ? 1 : 0] != m_sLang )
        {
            std::cerr
                << "Treex error: given language conflicts with "
                << "language of Mergedata file: "
                << m_sLang.getStr() << " - "
                << vLanguages[vLanguages[0]=="qtz" ? 1 : 0].getStr() << std::endl;
            return;
        }
        lcl_MergeLevel(
            m_pSource, pRootNode, (const xmlChar *)("help_section"),
            &aMergeDataFile, m_sLang, rXhpRoot );
    }

    xmlSaveFile( rDestinationFile.getStr(), m_pSource );
    xmlFreeDoc( m_pSource );
    xmlCleanupParser();
    m_bIsInitialized = false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
