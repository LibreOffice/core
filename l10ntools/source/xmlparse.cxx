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
#include "sal/config.h"

#include <iterator> /* std::iterator*/

#include <cassert>
#include <stdio.h>
#include <sal/alloca.h>

#include "helper.hxx"
#include "common.hxx"
#include "xmlparse.hxx"
#include <fstream>
#include <iostream>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <unicode/regex.h>

using namespace U_ICU_NAMESPACE;
using namespace std;
using namespace osl;

#define XML_LANG    "xml-lang"
#define ID          "id"
#define OLDREF      "oldref"

//
// class XMLChildNode
//

XMLChildNode::XMLChildNode( XMLParentNode *pPar )
    : m_pParent( pPar )
{
    if ( m_pParent )
        m_pParent->AddChild( this );
}


XMLChildNode::XMLChildNode( const XMLChildNode& rObj)
    : XMLNode(rObj),
      m_pParent(rObj.m_pParent)
{
}

XMLChildNode& XMLChildNode::operator=(const XMLChildNode& rObj)
{
    if(this != &rObj)
    {
        m_pParent=rObj.m_pParent;
    }
    return *this;
}

//
// class XMLParentNode
//

XMLParentNode::~XMLParentNode()
{
    if( m_pChildList )
    {
            RemoveAndDeleteAllChildren();
            delete m_pChildList;
    }
    m_pChildList = NULL;
}
XMLParentNode::XMLParentNode( const XMLParentNode& rObj)
: XMLChildNode( rObj )
{
    if( rObj.m_pChildList )
    {
        m_pChildList=new XMLChildNodeList();
        XMLChildNode* pNode = NULL;
        for ( size_t i = 0; i < rObj.m_pChildList->size(); i++ )
        {
            pNode = (*rObj.m_pChildList)[ i ];
            if( pNode != NULL)
            {
                switch(pNode->GetNodeType())
                {
                    case XML_NODE_TYPE_ELEMENT:
                        AddChild( new XMLElement( *static_cast<XMLElement* >(pNode) ) ); break;
                    case XML_NODE_TYPE_DATA:
                        AddChild( new XMLData   ( *static_cast<XMLData* >   (pNode) ) ); break;
                    case XML_NODE_TYPE_COMMENT:
                        AddChild( new XMLComment( *static_cast<XMLComment* >(pNode) ) ); break;
                    case XML_NODE_TYPE_DEFAULT:
                        AddChild( new XMLDefault( *static_cast<XMLDefault* >(pNode) ) ); break;
                    default:    fprintf(stdout,"XMLParentNode::XMLParentNode( const XMLParentNode& rObj) strange obj");
                }
            }
        }
    }
    else
        m_pChildList = NULL;
}
XMLParentNode& XMLParentNode::operator=(const XMLParentNode& rObj)
{
    if(this!=&rObj)
    {
        XMLChildNode::operator=(rObj);
        if( m_pChildList )
        {
            RemoveAndDeleteAllChildren();
            delete m_pChildList;
            m_pChildList = NULL;
        }
        if( rObj.m_pChildList )
        {
            m_pChildList=new XMLChildNodeList();
            for ( size_t i = 0; i < rObj.m_pChildList->size(); i++ )
                AddChild( (*rObj.m_pChildList)[ i ] );
        }
        else
            m_pChildList = NULL;

    }
    return *this;
}
void XMLParentNode::AddChild( XMLChildNode *pChild )
{
    if ( !m_pChildList )
        m_pChildList = new XMLChildNodeList();
    m_pChildList->push_back( pChild );
}

void XMLParentNode::RemoveAndDeleteAllChildren()
{
    if ( m_pChildList )
    {
        for ( size_t i = 0; i < m_pChildList->size(); i++ )
            delete (*m_pChildList)[ i ];
        m_pChildList->clear();
    }
}

//
// class XMLFile
//

void XMLFile::Write( OString const &aFilename )
{
    std::ofstream s(
        aFilename.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!s.is_open())
    {
        std::cerr
            << "Error: helpex cannot create file " << aFilename.getStr()
            << '\n';
        std::exit(EXIT_FAILURE);
    }
    Write(s);
    s.close();
}

sal_Bool XMLFile::Write( ofstream &rStream , XMLNode *pCur )
{
    if ( !pCur )
        Write( rStream, this );
    else {
        switch( pCur->GetNodeType())
        {
            case XML_NODE_TYPE_FILE:
            {
                if( GetChildList())
                    for ( size_t i = 0; i < GetChildList()->size(); i++ )
                        Write( rStream, (*GetChildList())[ i ] );
            }
            break;
            case XML_NODE_TYPE_ELEMENT:
            {
                XMLElement *pElement = ( XMLElement * ) pCur;
                rStream  << "<";
                rStream << pElement->GetName().getStr();
                if ( pElement->GetAttributeList())
                    for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ )
                    {
                        rStream << " ";
                        OString sData( (*pElement->GetAttributeList())[ j ]->GetName() );
                        rStream << XMLUtil::QuotHTML( sData ).getStr();
                        rStream << "=\"";
                        sData = (*pElement->GetAttributeList())[ j ]->GetValue();
                        rStream << XMLUtil::QuotHTML( sData ).getStr();
                        rStream << "\"";
                    }
                if ( !pElement->GetChildList())
                    rStream << "/>";
                else
                {
                    rStream << ">";
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        Write( rStream, (*pElement->GetChildList())[ k ] );
                    rStream << "</";
                    rStream << pElement->GetName().getStr();
                    rStream << ">";
                }
            }
            break;
            case XML_NODE_TYPE_DATA:
            {
                OString sData( static_cast<const XMLData*>(pCur)->GetData());
                rStream << XMLUtil::QuotHTML( sData ).getStr();
            }
            break;
            case XML_NODE_TYPE_COMMENT:
            {
                const XMLComment *pComment = static_cast<const XMLComment*>(pCur);
                rStream << "<!--";
                rStream <<  pComment->GetComment().getStr();
                rStream << "-->";
            }
            break;
            case XML_NODE_TYPE_DEFAULT:
            {
                const XMLDefault *pDefault = static_cast<const XMLDefault*>(pCur);
                rStream <<  pDefault->GetDefault().getStr();
            }
            break;
        }
    }
    return sal_True;
}

void XMLFile::Print( XMLNode *pCur, sal_uInt16 nLevel )
{
    if ( !pCur )
        Print( this );
    else
    {
        switch( pCur->GetNodeType())
        {
            case XML_NODE_TYPE_FILE:
            {
                if( GetChildList())
                    for ( size_t i = 0; i < GetChildList()->size(); i++ )
                        Print( (*GetChildList())[ i ] );
            }
            break;
            case XML_NODE_TYPE_ELEMENT:
            {
                XMLElement *pElement = ( XMLElement * ) pCur;

                fprintf( stdout, "<%s", pElement->GetName().getStr());
                if ( pElement->GetAttributeList())
                {
                    for (size_t j = 0; j < pElement->GetAttributeList()->size(); ++j)
                    {
                        const OString aAttrName((*pElement->GetAttributeList())[j]->GetName());
                        if (!aAttrName.equalsIgnoreAsciiCase(XML_LANG))
                        {
                            fprintf( stdout, " %s=\"%s\"",
                                aAttrName.getStr(),
                                (*pElement->GetAttributeList())[ j ]->GetValue().getStr());
                        }
                    }
                }
                if ( !pElement->GetChildList())
                    fprintf( stdout, "/>" );
                else
                {
                    fprintf( stdout, ">" );
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        Print( (*pElement->GetChildList())[ k ], nLevel + 1 );
                    fprintf( stdout, "</%s>", pElement->GetName().getStr());
                }
            }
            break;
            case XML_NODE_TYPE_DATA:
            {
                const XMLData *pData = static_cast<const XMLData*>(pCur);
                fprintf( stdout, "%s", pData->GetData().getStr());
            }
            break;
            case XML_NODE_TYPE_COMMENT:
            {
                const XMLComment *pComment = static_cast<const XMLComment*>(pCur);
                fprintf( stdout, "<!--%s-->", pComment->GetComment().getStr());
            }
            break;
            case XML_NODE_TYPE_DEFAULT:
            {
                const XMLDefault *pDefault = static_cast<const XMLDefault*>(pCur);
                fprintf( stdout, "%s", pDefault->GetDefault().getStr());
            }
            break;
        }
    }
}
XMLFile::~XMLFile()
{
    if( m_pXMLStrings != NULL )
    {
        XMLHashMap::iterator pos = m_pXMLStrings->begin();
        for( ; pos != m_pXMLStrings->end() ; ++pos )
        {
            delete pos->second;             // Check and delete content also ?
        }
        delete m_pXMLStrings;
        m_pXMLStrings = NULL;
    }
}
XMLFile::XMLFile( const OString &rFileName ) // the file name, empty if created from memory stream
    : XMLParentNode( NULL )
    , m_sFileName( rFileName )
    , m_pXMLStrings( NULL )
{
    m_aNodes_localize.insert( TagMap::value_type(OString("bookmark") , sal_True) );
    m_aNodes_localize.insert( TagMap::value_type(OString("variable") , sal_True) );
    m_aNodes_localize.insert( TagMap::value_type(OString("paragraph") , sal_True) );
    m_aNodes_localize.insert( TagMap::value_type(OString("alt") , sal_True) );
    m_aNodes_localize.insert( TagMap::value_type(OString("caption") , sal_True) );
    m_aNodes_localize.insert( TagMap::value_type(OString("title") , sal_True) );
    m_aNodes_localize.insert( TagMap::value_type(OString("link") , sal_True) );
}

void XMLFile::Extract( XMLFile *pCur )
{
    if( m_pXMLStrings )
        delete m_pXMLStrings; // Elements ?

    m_pXMLStrings = new XMLHashMap();
    if ( !pCur )
        SearchL10NElements( this );
    else
    {
        if( pCur->GetNodeType() == XML_NODE_TYPE_FILE )
        {
            SearchL10NElements(pCur);
        }
    }
}

void XMLFile::InsertL10NElement( XMLElement* pElement )
{
    OString sId, sLanguage("");
    LangHashMap* pElem;

    if( pElement->GetAttributeList() != NULL )
    {
        for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ )
        {
            const OString sTempStr((*pElement->GetAttributeList())[ j ]->GetName());
            // Get the "id" Attribute
            if (sTempStr == ID)
            {
                sId = (*pElement->GetAttributeList())[ j ]->GetValue();
            }
            // Get the "xml-lang" Attribute
            if (sTempStr == XML_LANG)
            {
                sLanguage = (*pElement->GetAttributeList())[j]->GetValue();
            }

        }
    }
    else
    {
        fprintf(stdout,"XMLFile::InsertL10NElement: No AttributeList found");
        fprintf(stdout,"++++++++++++++++++++++++++++++++++++++++++++++++++");
        Print( pElement , 0 );
        fprintf(stdout,"++++++++++++++++++++++++++++++++++++++++++++++++++");
    }

    XMLHashMap::iterator pos = m_pXMLStrings->find( sId );
    if( pos == m_pXMLStrings->end() ) // No instanze , create new one
    {
        pElem = new LangHashMap();
        (*pElem)[ sLanguage ]=pElement;
        m_pXMLStrings->insert( XMLHashMap::value_type( sId , pElem ) );
        m_vOrder.push_back( sId );
    }
    else        // Already there
    {
        pElem=pos->second;
        if ( (*pElem)[ sLanguage ] )
        {
            fprintf(stdout,"Error: Duplicated entry. ID = %s  LANG = %s in File %s\n", sId.getStr(), sLanguage.getStr(), m_sFileName.getStr() );
            exit( -1 );
        }
        (*pElem)[ sLanguage ]=pElement;
    }
}

XMLFile::XMLFile( const XMLFile& rObj )
    : XMLParentNode( rObj )
    , m_sFileName( rObj.m_sFileName )
    , m_pXMLStrings( 0 )
{
    if( this != &rObj )
    {
        m_aNodes_localize = rObj.m_aNodes_localize;
        m_vOrder = rObj.m_vOrder;
    }
}

XMLFile& XMLFile::operator=(const XMLFile& rObj)
{
    if( this != &rObj )
    {
        XMLParentNode::operator=(rObj);

        m_aNodes_localize = rObj.m_aNodes_localize;
        m_vOrder = rObj.m_vOrder;

        if( m_pXMLStrings )
            delete m_pXMLStrings;

        if( rObj.m_pXMLStrings )
        {
            m_pXMLStrings = new XMLHashMap();
            for( XMLHashMap::iterator pos = rObj.m_pXMLStrings->begin() ; pos != rObj.m_pXMLStrings->end() ; ++pos )
            {
                LangHashMap* pElem=pos->second;
                LangHashMap* pNewelem = new LangHashMap();
                for(LangHashMap::iterator pos2=pElem->begin(); pos2!=pElem->end();++pos2)
                {
                    (*pNewelem)[ pos2->first ] = new XMLElement( *pos2->second );
                }
                (*m_pXMLStrings)[ pos->first ] = pNewelem;
            }
        }
    }
    return *this;
}

void XMLFile::SearchL10NElements( XMLParentNode *pCur, int nPos )
{
    bool bInsert = true;
    if ( !pCur )
        SearchL10NElements( this  );
    else
    {
        switch( pCur->GetNodeType())
        {
            case XML_NODE_TYPE_FILE:
            {
                XMLChildNode* pElement;
                if( GetChildList())
                {
                    for ( size_t i = 0; i < GetChildList()->size(); i++ )
                    {
                        pElement = (*GetChildList())[ i ];
                        if( pElement->GetNodeType() ==  XML_NODE_TYPE_ELEMENT )
                            SearchL10NElements( (XMLParentNode*) pElement , i);
                    }
                }
            }
            break;
            case XML_NODE_TYPE_ELEMENT:
            {
                XMLElement *pElement = ( XMLElement * ) pCur;
                const OString sName(pElement->GetName().toAsciiLowerCase());
                OString sLanguage, sTmpStrVal, sOldref;
                if ( pElement->GetAttributeList())
                {
                    for ( size_t j = 0 , cnt = pElement->GetAttributeList()->size(); j < cnt && bInsert; ++j )
                    {
                        const OString sTmpStr = (*pElement->GetAttributeList())[j]->GetName();
                        if (sTmpStr == ID)
                        {
                            sTmpStrVal=(*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                        if (sTmpStr == "localize")
                        {
                            bInsert=false;
                        }
                        if (sTmpStr == XML_LANG) // Get the "xml-lang" Attribute
                        {
                            sLanguage=(*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                        if (sTmpStr == OLDREF) // Get the "oldref" Attribute
                        {
                            sOldref=(*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                    }
                    pElement->SetLanguageId( sLanguage );
                    pElement->SetId( sTmpStrVal );
                    pElement->SetOldRef( sOldref );
                    pElement->SetPos( nPos );
                }

                if ( bInsert && ( m_aNodes_localize.find( sName ) != m_aNodes_localize.end() ) )
                    InsertL10NElement(pElement);
                else if ( bInsert && pElement->GetChildList() )
                {
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        SearchL10NElements( (XMLParentNode*)(*pElement->GetChildList())[ k ], k);
                }
            }
            break;
            case XML_NODE_TYPE_DATA:
            break;
            case XML_NODE_TYPE_COMMENT:
            break;
            case XML_NODE_TYPE_DEFAULT:
            break;
        }
    }
}

bool XMLFile::CheckExportStatus( XMLParentNode *pCur )
{
    static bool bStatusExport = true;

    bool bInsert = true;
    if ( !pCur )
        CheckExportStatus( this );
    else {
        switch( pCur->GetNodeType())
        {
            case XML_NODE_TYPE_FILE:
            {
                XMLParentNode* pElement;
                if( GetChildList())
                {
                    for ( size_t i = 0; i < GetChildList()->size(); i++ )
                    {
                        pElement = (XMLParentNode*)(*GetChildList())[ i ];
                        if( pElement->GetNodeType() ==  XML_NODE_TYPE_ELEMENT ) CheckExportStatus( pElement );//, i);
                    }
                }
            }
            break;
            case XML_NODE_TYPE_ELEMENT:
            {
                XMLElement *pElement = ( XMLElement * ) pCur;
                if (pElement->GetName().equalsIgnoreAsciiCase("TOPIC"))
                {
                    if ( pElement->GetAttributeList())
                    {
                        for (size_t j = 0 , cnt = pElement->GetAttributeList()->size(); j < cnt && bInsert; ++j)
                        {
                            const OString tmpStr((*pElement->GetAttributeList())[j]->GetName());
                            if (tmpStr.equalsIgnoreAsciiCase("STATUS"))
                            {
                                const OString tmpStrVal((*pElement->GetAttributeList())[j]->GetValue());
                                if (!tmpStrVal.equalsIgnoreAsciiCase("PUBLISH") &&
                                    !tmpStrVal.equalsIgnoreAsciiCase("DEPRECATED"))
                                {
                                    bStatusExport = false;
                                }
                            }

                        }
                    }
                }
                else if ( pElement->GetChildList() )
                {
                    for (size_t k = 0; k < pElement->GetChildList()->size(); ++k)
                        CheckExportStatus( (XMLParentNode*)(*pElement->GetChildList())[k] );
                }
            }
            break;
        }
    }
    return bStatusExport;
}

XMLElement::XMLElement(
    const OString &rName,    // the element name
    XMLParentNode *pParent   // parent node of this element
)
    : XMLParentNode( pParent )
    , m_sElementName( rName )
    , m_pAttributes( NULL )
    , m_sProject(OString())
    , m_sFilename(OString())
    , m_sId(OString())
    , m_sOldRef(OString())
    , m_sResourceType(OString())
    , m_sLanguageId(OString())
    , m_nPos(0)
{
}

XMLElement::XMLElement(const XMLElement& rObj)
    : XMLParentNode( rObj )
    , m_sElementName( rObj.m_sElementName )
    , m_pAttributes( 0 )
    , m_sProject( rObj.m_sProject )
    , m_sFilename( rObj.m_sFilename )
    , m_sId( rObj.m_sId )
    , m_sOldRef( rObj.m_sOldRef )
    , m_sResourceType( rObj.m_sResourceType )
    , m_sLanguageId( rObj.m_sLanguageId )
    , m_nPos( rObj.m_nPos )
{
    if ( rObj.m_pAttributes )
    {
        m_pAttributes = new XMLAttributeList();
        for ( size_t i = 0; i < rObj.m_pAttributes->size(); i++ )
            AddAttribute( (*rObj.m_pAttributes)[ i ]->GetName(), (*rObj.m_pAttributes)[ i ]->GetValue() );
    }
}

XMLElement& XMLElement::operator=(const XMLElement& rObj)
{
    if( this !=& rObj )
    {
        XMLParentNode::operator=(rObj);
        m_sElementName = rObj.m_sElementName;
        m_sProject = rObj.m_sProject;
        m_sFilename = rObj.m_sFilename;
        m_sId = rObj.m_sId;
        m_sOldRef = rObj.m_sOldRef;
        m_sResourceType = rObj.m_sResourceType;
        m_sLanguageId = rObj.m_sLanguageId;
        m_nPos = rObj.m_nPos;

        if ( m_pAttributes )
        {
            for ( size_t i = 0; i < m_pAttributes->size(); i++ )
                delete (*m_pAttributes)[ i ];
            delete m_pAttributes;
        }
        if ( rObj.m_pAttributes )
        {
            m_pAttributes = new XMLAttributeList();
            for ( size_t i = 0; i < rObj.m_pAttributes->size(); i++ )
                AddAttribute( (*rObj.m_pAttributes)[ i ]->GetName(), (*rObj.m_pAttributes)[ i ]->GetValue() );
        }
    }
    return *this;
}

void XMLElement::AddAttribute( const OString &rAttribute, const OString &rValue )
{
    if ( !m_pAttributes )
        m_pAttributes = new XMLAttributeList();
    m_pAttributes->push_back( new XMLAttribute( rAttribute, rValue ) );
}

void XMLElement::ChangeLanguageTag( const OString &rValue )
{
    SetLanguageId(rValue);
    if ( m_pAttributes )
    {
        for (size_t i = 0; i < m_pAttributes->size(); ++i)
        {
            if ( (*m_pAttributes)[ i ]->GetName() == "xml-lang" )
                (*m_pAttributes)[ i ]->setValue(rValue);
        }
    }
    XMLChildNode* pNode  = NULL;
    XMLElement*   pElem  = NULL;
    XMLChildNodeList* pCList = GetChildList();

    if( pCList )
    {
        for ( size_t i = 0; i < pCList->size(); i++ )
        {
            pNode = (*pCList)[ i ];
            if( pNode && pNode->GetNodeType() == XML_NODE_TYPE_ELEMENT )
            {
                pElem = static_cast< XMLElement* >(pNode);
                pElem->ChangeLanguageTag( rValue );
                pElem->SetLanguageId(rValue);
                pElem  = NULL;
                pNode  = NULL;
            }
        }
        pCList = NULL;
    }
}

XMLElement::~XMLElement()
{
    if ( m_pAttributes )
    {
        for ( size_t i = 0; i < m_pAttributes->size(); i++ )
            delete (*m_pAttributes)[ i ];

        delete m_pAttributes;
        m_pAttributes = NULL;
    }
}

OString XMLElement::ToOString()
{
    OStringBuffer sBuffer;
    Print(this,sBuffer,true);
    return sBuffer.makeStringAndClear();
}

void XMLElement::Print(XMLNode *pCur, OStringBuffer& rBuffer, bool bRootelement ) const
{
    if( pCur )
    {
        if( bRootelement )
        {
            XMLElement *pElement = static_cast<XMLElement*>(pCur);
            if ( pElement->GetAttributeList())
            {
                if ( pElement->GetChildList())
                {
                    XMLChildNode* pTmp = 0;
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                    {
                        pTmp = (*pElement->GetChildList())[ k ];
                        Print( pTmp, rBuffer , false);
                    }
                }
            }
        }
        else
        {
            switch( pCur->GetNodeType())
            {
                case XML_NODE_TYPE_ELEMENT:
                {
                    XMLElement *pElement = static_cast<XMLElement*>(pCur);

                    if( !pElement->GetName().equalsIgnoreAsciiCase("comment") )
                    {
                        rBuffer.append( "<" );
                        rBuffer.append( pElement->GetName() );
                        if ( pElement->GetAttributeList())
                        {
                            for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ )
                            {
                                const OString aAttrName( (*pElement->GetAttributeList())[ j ]->GetName() );
                                if( !aAttrName.equalsIgnoreAsciiCase( "xml-lang" ) )
                                {
                                    rBuffer.append(
                                        " " + aAttrName + "=\"" +
                                        (*pElement->GetAttributeList())[ j ]->GetValue() + "\"" );
                                }
                            }
                        }
                        if ( !pElement->GetChildList())
                            rBuffer.append( "/>" );
                        else
                        {
                            rBuffer.append( ">" );
                            XMLChildNode* pTmp = 0;
                            for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                            {
                                pTmp = (*pElement->GetChildList())[ k ];
                                Print( pTmp, rBuffer , false);
                            }
                            rBuffer.append( "</" + pElement->GetName() + ">" );
                        }
                    }
                }
                break;
                case XML_NODE_TYPE_DATA:
                {
                    const XMLData *pData = static_cast<const XMLData*>(pCur);
                    rBuffer.append( pData->GetData() );
                }
                break;
                case XML_NODE_TYPE_COMMENT:
                {
                    const XMLComment *pComment = static_cast<const XMLComment*>(pCur);
                    rBuffer.append( "<!--" + pComment->GetComment() + "-->" );
                }
                break;
                case XML_NODE_TYPE_DEFAULT:
                {
                    const XMLDefault *pDefault = static_cast<const XMLDefault*>(pCur);
                    rBuffer.append( pDefault->GetDefault() );
                }
                break;
            }
        }
    }
    else
    {
        fprintf(stdout,"\n#+------Error: NULL Pointer in XMLELement::Print------+#\n");
        return;
    }
}

//
// class SimpleXMLParser
//

namespace
{

static OUString lcl_pathnameToAbsoluteUrl(const OString& rPathname)
{
    OUString sPath = OStringToOUString(rPathname, RTL_TEXTENCODING_UTF8 );
    OUString sUrl;
    if (osl::FileBase::getFileURLFromSystemPath(sPath, sUrl)
        != osl::FileBase::E_None)
    {
        std::cerr << "Error: Cannot convert input pathname to URL\n";
        std::exit(EXIT_FAILURE);
    }
    OUString sCwd;
    if (osl_getProcessWorkingDir(&sCwd.pData) != osl_Process_E_None)
    {
        std::cerr << "Error: Cannot determine cwd\n";
        std::exit(EXIT_FAILURE);
    }
    if (osl::FileBase::getAbsoluteFileURL(sCwd, sUrl, sUrl)
        != osl::FileBase::E_None)
    {
        std::cerr << "Error: Cannot convert input URL to absolute URL\n";
        std::exit(EXIT_FAILURE);
    }
    return sUrl;
}
}


SimpleXMLParser::SimpleXMLParser()
    : m_pXMLFile( NULL )
{
    m_aParser = XML_ParserCreate( NULL );
    XML_SetUserData( m_aParser, this );
    XML_SetElementHandler( m_aParser, (XML_StartElementHandler) StartElementHandler, (XML_EndElementHandler) EndElementHandler );
    XML_SetCharacterDataHandler( m_aParser, (XML_CharacterDataHandler) CharacterDataHandler );
    XML_SetCommentHandler( m_aParser, (XML_CommentHandler) CommentHandler );
    XML_SetDefaultHandler( m_aParser, (XML_DefaultHandler) DefaultHandler );
}

SimpleXMLParser::~SimpleXMLParser()
{
    XML_ParserFree( m_aParser );
}

void SimpleXMLParser::StartElementHandler(
    void *userData, const XML_Char *name, const XML_Char **atts )
{
    (( SimpleXMLParser * ) userData )->StartElement( name, atts );
}

void SimpleXMLParser::EndElementHandler(
    void *userData, const XML_Char *name )
{
    (( SimpleXMLParser * ) userData )->EndElement( name );
}

void SimpleXMLParser::CharacterDataHandler(
    void *userData, const XML_Char *s, int len )
{
    (( SimpleXMLParser * ) userData )->CharacterData( s, len );
}

void SimpleXMLParser::CommentHandler(
    void *userData, const XML_Char *data )
{
    (( SimpleXMLParser * ) userData )->Comment( data );
}

void SimpleXMLParser::DefaultHandler(
    void *userData, const XML_Char *s, int len )
{
    (( SimpleXMLParser * ) userData )->Default( s, len );
}

void SimpleXMLParser::StartElement(
    const XML_Char *name, const XML_Char **atts )
{
    XMLElement *pElement = new XMLElement( OString(name), ( XMLParentNode * ) m_pCurNode );
    m_pCurNode = pElement;
    m_pCurData = NULL;

    int i = 0;
    while( atts[i] )
    {
        pElement->AddAttribute( atts[ i ], atts[ i + 1 ] );
        i += 2;
    }
}

void SimpleXMLParser::EndElement( const XML_Char * /*name*/ )
{
    m_pCurNode = m_pCurNode->GetParent();
    m_pCurData = NULL;
}

void SimpleXMLParser::CharacterData( const XML_Char *s, int len )
{
    if ( !m_pCurData )
    {
        OString x( s, len );
        m_pCurData = new XMLData( helper::UnQuotHTML(x) , m_pCurNode );
    }
    else
    {
        OString x( s, len );
        m_pCurData->AddData( helper::UnQuotHTML(x) );

    }
}

void SimpleXMLParser::Comment( const XML_Char *data )
{
    m_pCurData = NULL;
    new XMLComment( OString( data ), m_pCurNode );
}

void SimpleXMLParser::Default( const XML_Char *s, int len )
{
    m_pCurData = NULL;
    new XMLDefault(OString( s, len ), m_pCurNode );
}

XMLFile *SimpleXMLParser::Execute( const OString &rFileName, XMLFile* pXMLFileIn )
{
    m_aErrorInformation.m_eCode = XML_ERROR_NONE;
    m_aErrorInformation.m_nLine = 0;
    m_aErrorInformation.m_nColumn = 0;
    m_aErrorInformation.m_sMessage = "ERROR: Unable to open file ";
    m_aErrorInformation.m_sMessage += rFileName;

    OUString aFileURL(lcl_pathnameToAbsoluteUrl(rFileName));

    oslFileHandle h;
    if (osl_openFile(aFileURL.pData, &h, osl_File_OpenFlag_Read)
        != osl_File_E_None)
    {
        return 0;
    }

    sal_uInt64 s;
    oslFileError e = osl_getFileSize(h, &s);
    void * p = NULL;
    if (e == osl_File_E_None)
    {
        e = osl_mapFile(h, &p, s, 0, 0);
    }
    if (e != osl_File_E_None)
    {
        osl_closeFile(h);
        return 0;
    }

    m_pXMLFile = pXMLFileIn;
    m_pXMLFile->SetName( rFileName );

    m_pCurNode = m_pXMLFile;
    m_pCurData = NULL;

    m_aErrorInformation.m_eCode = XML_ERROR_NONE;
    m_aErrorInformation.m_nLine = 0;
    m_aErrorInformation.m_nColumn = 0;
    if ( !m_pXMLFile->GetName().isEmpty())
    {
        m_aErrorInformation.m_sMessage = "File " + m_pXMLFile->GetName() + " parsed successfully";
    }
    else
        m_aErrorInformation.m_sMessage = "XML-File parsed successfully";

    if (!XML_Parse(m_aParser, reinterpret_cast< char * >(p), s, true))
    {
        m_aErrorInformation.m_eCode = XML_GetErrorCode( m_aParser );
        m_aErrorInformation.m_nLine = XML_GetErrorLineNumber( m_aParser );
        m_aErrorInformation.m_nColumn = XML_GetErrorColumnNumber( m_aParser );

        m_aErrorInformation.m_sMessage = "ERROR: ";
        if ( !m_pXMLFile->GetName().isEmpty())
            m_aErrorInformation.m_sMessage += m_pXMLFile->GetName();
        else
            m_aErrorInformation.m_sMessage += OString( "XML-File (");

        m_aErrorInformation.m_sMessage +=
            OString::number(sal::static_int_cast< sal_Int64 >(m_aErrorInformation.m_nLine)) + "," +
            OString::number(sal::static_int_cast< sal_Int64 >(m_aErrorInformation.m_nColumn)) + "): ";

        switch (m_aErrorInformation.m_eCode)
        {
        case XML_ERROR_NO_MEMORY:
            m_aErrorInformation.m_sMessage += "No memory";
            break;
        case XML_ERROR_SYNTAX:
            m_aErrorInformation.m_sMessage += "Syntax";
            break;
        case XML_ERROR_NO_ELEMENTS:
            m_aErrorInformation.m_sMessage += "No elements";
            break;
        case XML_ERROR_INVALID_TOKEN:
            m_aErrorInformation.m_sMessage += "Invalid token";
            break;
        case XML_ERROR_UNCLOSED_TOKEN:
            m_aErrorInformation.m_sMessage += "Unclosed token";
            break;
        case XML_ERROR_PARTIAL_CHAR:
            m_aErrorInformation.m_sMessage += "Partial char";
            break;
        case XML_ERROR_TAG_MISMATCH:
            m_aErrorInformation.m_sMessage += "Tag mismatch";
            break;
        case XML_ERROR_DUPLICATE_ATTRIBUTE:
            m_aErrorInformation.m_sMessage += "Dublicat attribute";
            break;
        case XML_ERROR_JUNK_AFTER_DOC_ELEMENT:
            m_aErrorInformation.m_sMessage += "Junk after doc element";
            break;
        case XML_ERROR_PARAM_ENTITY_REF:
            m_aErrorInformation.m_sMessage += "Param entity ref";
            break;
        case XML_ERROR_UNDEFINED_ENTITY:
            m_aErrorInformation.m_sMessage += "Undefined entity";
            break;
        case XML_ERROR_RECURSIVE_ENTITY_REF:
            m_aErrorInformation.m_sMessage += "Recursive entity ref";
            break;
        case XML_ERROR_ASYNC_ENTITY:
            m_aErrorInformation.m_sMessage += "Async_entity";
            break;
        case XML_ERROR_BAD_CHAR_REF:
            m_aErrorInformation.m_sMessage += "Bad char ref";
            break;
        case XML_ERROR_BINARY_ENTITY_REF:
            m_aErrorInformation.m_sMessage += "Binary entity";
            break;
        case XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF:
            m_aErrorInformation.m_sMessage += "Attribute external entity ref";
            break;
        case XML_ERROR_MISPLACED_XML_PI:
            m_aErrorInformation.m_sMessage += "Misplaced xml pi";
            break;
        case XML_ERROR_UNKNOWN_ENCODING:
            m_aErrorInformation.m_sMessage += "Unknown encoding";
            break;
        case XML_ERROR_INCORRECT_ENCODING:
            m_aErrorInformation.m_sMessage += "Incorrect encoding";
            break;
        case XML_ERROR_UNCLOSED_CDATA_SECTION:
            m_aErrorInformation.m_sMessage += "Unclosed cdata section";
            break;
        case XML_ERROR_EXTERNAL_ENTITY_HANDLING:
            m_aErrorInformation.m_sMessage += "External entity handling";
            break;
        case XML_ERROR_NOT_STANDALONE:
            m_aErrorInformation.m_sMessage += "Not standalone";
            break;
        case XML_ERROR_NONE:
            break;
        default:
            break;
        }
        delete m_pXMLFile;
        m_pXMLFile = NULL;
    }

    osl_unmapMappedFile(h, p, s);
    osl_closeFile(h);

    return m_pXMLFile;
}

namespace
{

static icu::UnicodeString lcl_QuotRange(
    const icu::UnicodeString& rString, const sal_Int32 nStart,
    const sal_Int32 nEnd, bool bInsideTag = false )
{
    icu::UnicodeString sReturn;
    assert( nStart < nEnd );
    assert( nStart >= 0 );
    assert( nEnd <= rString.length() );
    for (sal_Int32 i = nStart; i < nEnd; ++i)
    {
        switch (rString[i])
        {
            case '<':
                sReturn.append("&lt;");
                break;
            case '>':
                sReturn.append("&gt;");
                break;
            case '"':
                if( !bInsideTag )
                    sReturn.append("&quot;");
                else
                    sReturn.append(rString[i]);
                break;
            case '&':
                if (rString.startsWith("&amp;", i, 5))
                    sReturn.append('&');
                else
                    sReturn.append("&amp;");
                break;
            default:
                sReturn.append(rString[i]);
                break;
        }
    }
    return sReturn;
}

static bool lcl_isTag( const icu::UnicodeString& rString )
{
    static const int nSize = 12;
    static const icu::UnicodeString vTags[nSize] = {
        "ahelp", "link", "item", "emph", "defaultinline",
        "switchinline", "caseinline", "variable",
        "bookmark_value", "image", "embedvar", "alt" };

    for( int nIndex = 0; nIndex < nSize; ++nIndex )
    {
        if( rString.startsWith("<" + vTags[nIndex]) ||
             rString == "</" + vTags[nIndex] + ">" )
            return true;
    }

    return rString == "<br/>" || rString =="<help-id-missing/>";
}

} /// anonymous namespace

OString XMLUtil::QuotHTML( const OString &rString )
{
    if( rString.trim().isEmpty() )
        return rString;
    UErrorCode nIcuErr = U_ZERO_ERROR;
    static const sal_uInt32 nSearchFlags =
        UREGEX_DOTALL | UREGEX_CASE_INSENSITIVE;
    static const OUString sPattern(
        "<[/]\?\?[a-z_-]+?(?:| +[a-z]+?=\".*?\") *[/]\?\?>");
    static const UnicodeString sSearchPat(
        reinterpret_cast<const UChar*>(sPattern.getStr()),
        sPattern.getLength() );

    const OUString sOUSource = OStringToOUString(rString, RTL_TEXTENCODING_UTF8);
    icu::UnicodeString sSource(
        reinterpret_cast<const UChar*>(
            sOUSource.getStr()), sOUSource.getLength() );

    RegexMatcher aRegexMatcher( sSearchPat, nSearchFlags, nIcuErr );
    aRegexMatcher.reset( sSource );

    icu::UnicodeString sReturn;
    int32_t nEndPos = 0;
    int32_t nStartPos = 0;
    while( aRegexMatcher.find(nStartPos, nIcuErr) && nIcuErr == U_ZERO_ERROR )
    {
        nStartPos = aRegexMatcher.start(nIcuErr);
        if ( nEndPos < nStartPos )
            sReturn.append(lcl_QuotRange(sSource, nEndPos, nStartPos));
        nEndPos = aRegexMatcher.end(nIcuErr);
        icu::UnicodeString sMatch = aRegexMatcher.group(nIcuErr);
        if( lcl_isTag(sMatch) )
        {
            sReturn.append("<");
            sReturn.append(lcl_QuotRange(sSource, nStartPos+1, nEndPos-1, true));
            sReturn.append(">");
        }
        else
            sReturn.append(lcl_QuotRange(sSource, nStartPos, nEndPos));
        ++nStartPos;
    }
    if( nEndPos < sSource.length() )
        sReturn.append(lcl_QuotRange(sSource, nEndPos, sSource.length()));
    sReturn.append('\0');
    return
        OUStringToOString(
            OUString(reinterpret_cast<const sal_Unicode*>(sReturn.getBuffer())),
            RTL_TEXTENCODING_UTF8);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
