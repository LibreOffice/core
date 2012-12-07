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

#include <stdio.h>
#include <sal/alloca.h>

#include "common.hxx"
#include "xmlparse.hxx"
#include <fstream>
#include <iostream>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <rtl/strbuf.hxx>

using namespace std;
using namespace osl;

//
// class XMLChildNode
//

/*****************************************************************************/
XMLChildNode::XMLChildNode( XMLParentNode *pPar )
/*****************************************************************************/
                : pParent( pPar )
{
    if ( pParent )
        pParent->AddChild( this );
}


/*****************************************************************************/
XMLChildNode::XMLChildNode( const XMLChildNode& obj)
/*****************************************************************************/
:   XMLNode(obj),
    pParent(obj.pParent){}

/*****************************************************************************/
XMLChildNode& XMLChildNode::operator=(const XMLChildNode& obj){
/*****************************************************************************/
    if(this != &obj){
        pParent=obj.pParent;
    }
    return *this;
}
//
// class XMLParentNode
//


/*****************************************************************************/
XMLParentNode::~XMLParentNode()
/*****************************************************************************/
{
    if( pChildList )
    {
            RemoveAndDeleteAllChildren();
            delete pChildList;
            pChildList = NULL;
    }
    pChildList = NULL;
}
/*****************************************************************************/
XMLParentNode::XMLParentNode( const XMLParentNode& obj)
/*****************************************************************************/
: XMLChildNode( obj )
{
    if( obj.pChildList ){
        pChildList=new XMLChildNodeList();
        XMLChildNode* pNode = NULL;
        for ( size_t i = 0; i < obj.pChildList->size(); i++ ){
            pNode = (*obj.pChildList)[ i ];
            if( pNode != NULL){
                switch(pNode->GetNodeType()){
                    case XML_NODE_TYPE_ELEMENT:
                        AddChild( new XMLElement( *static_cast<XMLElement* >(pNode) ) ); break;
                    case XML_NODE_TYPE_DATA:
                        AddChild( new XMLData   ( *static_cast<XMLData* >   (pNode) ) ); break;
                    case XML_NODE_TYPE_COMMENT:
                        AddChild( new XMLComment( *static_cast<XMLComment* >(pNode) ) ); break;
                    case XML_NODE_TYPE_DEFAULT:
                        AddChild( new XMLDefault( *static_cast<XMLDefault* >(pNode) ) ); break;
                    default:    fprintf(stdout,"XMLParentNode::XMLParentNode( const XMLParentNode& obj) strange obj");
                }
            }
        }
    }else pChildList = NULL;
}
/*****************************************************************************/
XMLParentNode& XMLParentNode::operator=(const XMLParentNode& obj){
/*****************************************************************************/
    if(this!=&obj){
        XMLChildNode::operator=(obj);
        if( pChildList ){
            RemoveAndDeleteAllChildren();
            delete pChildList;
            pChildList = NULL;
        }
        if( obj.pChildList ){
            pChildList=new XMLChildNodeList();
            for ( size_t i = 0; i < obj.pChildList->size(); i++ )
                AddChild( (*obj.pChildList)[ i ] );
        }else pChildList = NULL;

    }
    return *this;
}
/*****************************************************************************/
void XMLParentNode::AddChild( XMLChildNode *pChild )
/*****************************************************************************/
{
    if ( !pChildList )
        pChildList = new XMLChildNodeList();
    pChildList->push_back( pChild );
}

/*****************************************************************************/
void XMLParentNode::RemoveAndDeleteAllChildren(){
/*****************************************************************************/
    if ( pChildList ) {
        for ( size_t i = 0; i < pChildList->size(); i++ )
            delete (*pChildList)[ i ];
        pChildList->clear();
    }
}

//
// class XMLFile
//

/*****************************************************************************/
sal_uInt16 XMLFile::GetNodeType()
/*****************************************************************************/
{
    return XML_NODE_TYPE_FILE;
}

void XMLFile::Write( rtl::OString const &aFilename )
{
    std::ofstream s(
        aFilename.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!s.is_open()) {
        std::cerr
            << "Error: helpex cannot create file " << aFilename.getStr()
            << '\n';
        std::exit(EXIT_FAILURE);
    }
    Write(s);
    s.close();
}

void XMLFile::WriteString( ofstream &rStream, const rtl::OString &sText )
{
    rStream << sText.getStr();
}

sal_Bool XMLFile::Write( ofstream &rStream , XMLNode *pCur )
{
    if ( !pCur )
        Write( rStream, this );
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                if( GetChildList())
                    for ( size_t i = 0; i < GetChildList()->size(); i++ )
                        Write( rStream, (*GetChildList())[ i ] );
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;
                rStream  << "<";
                WriteString( rStream, pElement->GetName());
                if ( pElement->GetAttributeList())
                    for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ ) {
                        rStream << " ";
                        rtl::OString sData( (*pElement->GetAttributeList())[ j ]->GetName() );
                        XMLUtil::QuotHTML( sData );
                        WriteString( rStream , sData );
                        rStream << "=\"";
                        sData = (*pElement->GetAttributeList())[ j ]->GetValue();
                        XMLUtil::QuotHTML( sData );
                        WriteString( rStream , sData  );
                        rStream << "\"";
                    }
                if ( !pElement->GetChildList())
                    rStream << "/>";
                else {
                    rStream << ">";
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        Write( rStream, (*pElement->GetChildList())[ k ] );
                    rStream << "</";
                    WriteString( rStream, pElement->GetName());
                    rStream << ">";
                   }
            }
            break;
            case XML_NODE_TYPE_DATA: {
                XMLData *pData = ( XMLData * ) pCur;
                rtl::OString sData( pData->GetData());
                XMLUtil::QuotHTML( sData );
                WriteString( rStream, sData );
            }
            break;
            case XML_NODE_TYPE_COMMENT: {
                XMLComment *pComment = ( XMLComment * ) pCur;
                rStream << "<!--";
                WriteString( rStream, pComment->GetComment());
                rStream << "-->";
            }
            break;
            case XML_NODE_TYPE_DEFAULT: {
                XMLDefault *pDefault = ( XMLDefault * ) pCur;
                WriteString( rStream, pDefault->GetDefault());
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
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                if( GetChildList())
                    for ( size_t i = 0; i < GetChildList()->size(); i++ )
                        Print( (*GetChildList())[ i ] );
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;

                fprintf( stdout, "<%s", pElement->GetName().getStr());
                if ( pElement->GetAttributeList())
                {
                    for (size_t j = 0; j < pElement->GetAttributeList()->size(); ++j)
                    {
                        rtl::OString aAttrName((*pElement->GetAttributeList())[j]->GetName());
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
                else {
                    fprintf( stdout, ">" );
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        Print( (*pElement->GetChildList())[ k ], nLevel + 1 );
                    fprintf( stdout, "</%s>", pElement->GetName().getStr());
                }
            }
            break;
            case XML_NODE_TYPE_DATA: {
                XMLData *pData = ( XMLData * ) pCur;
                fprintf( stdout, "%s", pData->GetData().getStr());
            }
            break;
            case XML_NODE_TYPE_COMMENT: {
                XMLComment *pComment = ( XMLComment * ) pCur;
                fprintf( stdout, "<!--%s-->", pComment->GetComment().getStr());
            }
            break;
            case XML_NODE_TYPE_DEFAULT: {
                XMLDefault *pDefault = ( XMLDefault * ) pCur;
                fprintf( stdout, "%s", pDefault->GetDefault().getStr());
            }
            break;
        }
    }
}
XMLFile::~XMLFile()
{
    if( XMLStrings != NULL ){
        XMLHashMap::iterator pos = XMLStrings->begin();
        for( ; pos != XMLStrings->end() ; ++pos ){
            delete pos->second;             // Check and delete content also ?
        }
        delete XMLStrings;
        XMLStrings = NULL;
    }
}
/*****************************************************************************/
XMLFile::XMLFile( const rtl::OString &rFileName ) // the file name, empty if created from memory stream
/*****************************************************************************/
                : XMLParentNode( NULL ),
                  sFileName    ( rFileName ),
                  ID           ( "id" ),
                  OLDREF       ( "oldref" ),
                  XML_LANG     ( "xml-lang" ),
                  XMLStrings   ( NULL )

{
    nodes_localize.insert( TagMap::value_type(rtl::OString(RTL_CONSTASCII_STRINGPARAM("bookmark")) , sal_True) );
    nodes_localize.insert( TagMap::value_type(rtl::OString(RTL_CONSTASCII_STRINGPARAM("variable")) , sal_True) );
    nodes_localize.insert( TagMap::value_type(rtl::OString(RTL_CONSTASCII_STRINGPARAM("paragraph")) , sal_True) );
    nodes_localize.insert( TagMap::value_type(rtl::OString(RTL_CONSTASCII_STRINGPARAM("alt")) , sal_True) );
    nodes_localize.insert( TagMap::value_type(rtl::OString(RTL_CONSTASCII_STRINGPARAM("caption")) , sal_True) );
    nodes_localize.insert( TagMap::value_type(rtl::OString(RTL_CONSTASCII_STRINGPARAM("title")) , sal_True) );
    nodes_localize.insert( TagMap::value_type(rtl::OString(RTL_CONSTASCII_STRINGPARAM("link")) , sal_True) );
}
/*****************************************************************************/
void XMLFile::Extract( XMLFile *pCur )
/*****************************************************************************/
{
    if( XMLStrings != NULL ) delete XMLStrings; // Elements ?

    XMLStrings = new XMLHashMap();
       if ( !pCur )
        SearchL10NElements( this );
    else {
        if( pCur->GetNodeType()==XML_NODE_TYPE_FILE) {
            SearchL10NElements(pCur);
        }
    }
}

/*****************************************************************************/
void XMLFile::InsertL10NElement( XMLElement* pElement ){
/*****************************************************************************/
    rtl::OString id,oldref,language("");
    LangHashMap* elem;

    if( pElement->GetAttributeList() != NULL ){
        for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ )
        {
            if ((*pElement->GetAttributeList())[ j ]->GetName() == ID)
            {
                // Get the "id" Attribute
                id = (*pElement->GetAttributeList())[ j ]->GetValue();
            }
            if ((*pElement->GetAttributeList())[ j ]->GetName() == XML_LANG)
            {
                // Get the "xml-lang" Attribute
                language = (*pElement->GetAttributeList())[j]->GetValue();
            }

        }
    }else{
        fprintf(stdout,"XMLFile::InsertL10NElement: No AttributeList found");
        fprintf(stdout,"++++++++++++++++++++++++++++++++++++++++++++++++++");
        Print( pElement , 0 );
        fprintf(stdout,"++++++++++++++++++++++++++++++++++++++++++++++++++");
    }

    XMLHashMap::iterator pos = XMLStrings->find( id );
    if( pos == XMLStrings->end() ){             // No instanze , create new one
        elem = new LangHashMap();
        (*elem)[ language ]=pElement;
        XMLStrings->insert( XMLHashMap::value_type( id , elem ) );
        order.push_back( id );
    }else{                                  // Already there
        elem=pos->second;
        if ( (*elem)[ language ] )
        {
            fprintf(stdout,"Error: Duplicated entry. ID = %s  LANG = %s in File %s\n", id.getStr(), language.getStr(),
                    sFileName.getStr() );
            exit( -1 );
        }
        (*elem)[ language ]=pElement;
    }
}

XMLFile::XMLFile( const XMLFile& obj )
/*****************************************************************************/
                : XMLParentNode( obj ),
                  sFileName    ( obj.sFileName ),
                  ID           ( "id" ),
                  OLDREF       ( "oldref" ),
                  XML_LANG     ( "xml-lang" ),
                  XMLStrings   ( NULL )
{
    if( this!=&obj )
    {
        nodes_localize  =obj.nodes_localize;
        order           =obj.order;

    }
}
/*****************************************************************************/
XMLFile& XMLFile::operator=(const XMLFile& obj){
/*****************************************************************************/
    if( this!=&obj ){

        XMLParentNode::operator=(obj);

        nodes_localize  =obj.nodes_localize;
        order           =obj.order;

        if( XMLStrings )    delete XMLStrings;

        if( obj.XMLStrings )
        {
            XMLStrings = new XMLHashMap();
            for( XMLHashMap::iterator pos = obj.XMLStrings->begin() ; pos != obj.XMLStrings->end() ; ++pos )
            {
                LangHashMap* elem=pos->second;
                LangHashMap* newelem = new LangHashMap();
                for(LangHashMap::iterator pos2=elem->begin(); pos2!=elem->end();++pos2){
                    (*newelem)[ pos2->first ] = new XMLElement( *pos2->second );
                }
                (*XMLStrings)[ pos->first ] = newelem;
            }
        }
    }
    return *this;
}


/*****************************************************************************/
void XMLFile::SearchL10NElements( XMLParentNode *pCur , int pos)
/*****************************************************************************/
{
    static const rtl::OString LOCALIZE("localize");
    static const rtl::OString THEID("id");
    bool bInsert    = true;
    if ( !pCur )
        SearchL10NElements( this  );
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                XMLParentNode* pElement;
                if( GetChildList()){
                    for ( size_t i = 0; i < GetChildList()->size(); i++ ){
                        pElement = (XMLParentNode*) (*GetChildList())[ i ];
                        if( pElement->GetNodeType() ==  XML_NODE_TYPE_ELEMENT ) SearchL10NElements( pElement , i);
                    }
                }
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;
                rtl::OString language,tmpStrVal,oldref;
                if ( pElement->GetAttributeList())
                {
                    for ( size_t j = 0 , cnt = pElement->GetAttributeList()->size(); j < cnt && bInsert; ++j )
                    {
                        if ((*pElement->GetAttributeList())[j]->GetName() == THEID)
                        {
                            // Get the "id" Attribute
                            tmpStrVal= (*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                        else if ((*pElement->GetAttributeList())[j]->GetName() == LOCALIZE)
                        {
                            // Get the "localize" Attribute
                            bInsert=false;
                        }
                        else if ((*pElement->GetAttributeList())[j]->GetName() == XML_LANG)
                        {
                            // Get the "xml-lang" Attribute
                            language= (*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                        else if ((*pElement->GetAttributeList())[j]->GetName() == OLDREF)
                        {
                            // Get the "oldref" Attribute
                            oldref=(*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                    }
                    pElement->SetLanguageId ( language );
                    pElement->SetId(tmpStrVal);
                    pElement->SetOldRef     ( oldref  );
                    pElement->SetPos( pos );
                }

                if ( bInsert && ( nodes_localize.find( pElement->GetName() ) != nodes_localize.end() ) )
                    InsertL10NElement(pElement);
                else if ( bInsert && pElement->GetChildList() ){
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        SearchL10NElements( (XMLParentNode*)(*pElement->GetChildList())[ k ], k);
                }
            }
            break;
            case XML_NODE_TYPE_DATA: {
            }
            break;
            case XML_NODE_TYPE_COMMENT: {
            }
            break;
            case XML_NODE_TYPE_DEFAULT: {
            }
            break;
        }
    }
}

/*****************************************************************************/
bool XMLFile::CheckExportStatus( XMLParentNode *pCur )
/*****************************************************************************/
{
    static bool bStatusExport = true;
    const rtl::OString LOCALIZE("localize");
    const rtl::OString STATUS("status");
    const rtl::OString PUBLISH("PUBLISH");
    const rtl::OString DEPRECATED("DEPRECATED");
    const rtl::OString TOPIC("topic");

    bool bInsert    = true;
    if ( !pCur )
        CheckExportStatus( this );
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                XMLParentNode* pElement;
                if( GetChildList()){
                    for ( size_t i = 0; i < GetChildList()->size(); i++ ){
                        pElement = (XMLParentNode*)(*GetChildList())[ i ];
                        if( pElement->GetNodeType() ==  XML_NODE_TYPE_ELEMENT ) CheckExportStatus( pElement );//, i);
                    }
                }
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;
                const rtl::OString sName = pElement->GetName();
                if (sName.equalsIgnoreAsciiCase(TOPIC))
                {
                    if ( pElement->GetAttributeList())
                    {
                        for (size_t j = 0 , cnt = pElement->GetAttributeList()->size(); j < cnt && bInsert; ++j)
                        {
                            if ((*pElement->GetAttributeList())[j]->GetName().equalsIgnoreAsciiCase(STATUS))
                            {
                                const rtl::OString tmpStrVal((*pElement->GetAttributeList())[j]->GetValue());
                                if (!tmpStrVal.equalsIgnoreAsciiCase(PUBLISH) &&
                                    !tmpStrVal.equalsIgnoreAsciiCase(DEPRECATED))
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

/*****************************************************************************/
sal_uInt16 XMLElement::GetNodeType()
/*****************************************************************************/
{
    return XML_NODE_TYPE_ELEMENT;
}

/*****************************************************************************/
XMLElement::XMLElement(const XMLElement& obj)
/*****************************************************************************/
    :
                XMLParentNode   ( obj ),
                sElementName    ( obj.sElementName ),
                pAttributes     ( NULL ),
                project         ( obj.project ),
                filename        ( obj.filename ),
                id              ( obj.id ),
                sOldRef         ( obj.sOldRef ),
                resourceType    ( obj.resourceType ),
                languageId      ( obj.languageId ),
                nPos            ( obj.nPos )

{
    if ( obj.pAttributes ){
        pAttributes = new XMLAttributeList();
        for ( size_t i = 0; i < obj.pAttributes->size(); i++ )
            AddAttribute( (*obj.pAttributes)[ i ]->GetName(), (*obj.pAttributes)[ i ]->GetValue() );
    }
}

/*****************************************************************************/
XMLElement& XMLElement::operator=(const XMLElement& obj){
/*****************************************************************************/
    if( this!=&obj ){
        XMLParentNode::operator=(obj);
        sElementName    =obj.sElementName;
        project         =obj.project;
        filename        =obj.filename;
        id              =obj.id;
        sOldRef         =obj.sOldRef;
        resourceType    =obj.resourceType;
        languageId      =obj.languageId;
        nPos            =obj.nPos;

        if ( pAttributes ){
            for ( size_t i = 0; i < pAttributes->size(); i++ )
                delete (*pAttributes)[ i ];
            delete pAttributes;
        }
        if ( obj.pAttributes ){
            pAttributes         =new XMLAttributeList();
            for ( size_t i = 0; i < obj.pAttributes->size(); i++ )
                AddAttribute( (*obj.pAttributes)[ i ]->GetName(), (*obj.pAttributes)[ i ]->GetValue() );
        }
    }
    return *this;
}

/*****************************************************************************/
void XMLElement::AddAttribute( const rtl::OString &rAttribute, const rtl::OString &rValue )
/*****************************************************************************/
{
    if ( !pAttributes )
        pAttributes = new XMLAttributeList();
    pAttributes->push_back( new XMLAttribute( rAttribute, rValue ) );
}

/*****************************************************************************/
void XMLElement::ChangeLanguageTag( const rtl::OString &rValue )
{
    SetLanguageId(rValue);
    if ( pAttributes )
    {
        for (size_t i = 0; i < pAttributes->size(); ++i)
        {
            if ( (*pAttributes)[ i ]->GetName() == "xml-lang" )
                (*pAttributes)[ i ]->setValue(rValue);
        }
    }
    XMLChildNode* pNode  = NULL;
    XMLElement*   pElem  = NULL;
    XMLChildNodeList* pCList = GetChildList();

    if( pCList != NULL )
    {
        for ( size_t i = 0; i < pCList->size(); i++ )
        {
            pNode = (*pCList)[ i ];
            if( pNode != NULL && pNode->GetNodeType() == XML_NODE_TYPE_ELEMENT )
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

/*****************************************************************************/
XMLElement::~XMLElement()
/*****************************************************************************/
{
    if ( pAttributes ) {
        for ( size_t i = 0; i < pAttributes->size(); i++ )
            delete (*pAttributes)[ i ];

        delete pAttributes;
        pAttributes = NULL;
    }
}

/*****************************************************************************/
OString XMLElement::ToOString(){
/*****************************************************************************/
    OStringBuffer* buffer = new OStringBuffer();
    Print(this,*buffer,true);
    OString result=buffer->makeStringAndClear();
    delete buffer;
    return result;
}
/*****************************************************************************/
void XMLElement::Print(XMLNode *pCur, OStringBuffer& buffer , bool rootelement ){
/*****************************************************************************/
    static const OString XML_LANG ( "xml-lang" );

    if(pCur!=NULL)
    {
        if(rootelement)
        {
            XMLElement *pElement = ( XMLElement * ) pCur;
            if ( pElement->GetAttributeList())
            {
                if ( pElement->GetChildList())
                {
                    XMLChildNode* tmp=NULL;
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                    {
                        tmp = (*pElement->GetChildList())[ k ];
                        Print( tmp, buffer , false);
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
                XMLElement *pElement = ( XMLElement * ) pCur;

                if( !pElement->GetName().equalsIgnoreAsciiCase("comment") )
                {
                    buffer.append( "\\<");
                    buffer.append( pElement->GetName() );
                    if ( pElement->GetAttributeList())
                    {
                        for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ )
                        {

                            const OString aAttrName = (*pElement->GetAttributeList())[ j ]->GetName();
                            if( !aAttrName.equalsIgnoreAsciiCase( XML_LANG ) )
                            {
                                buffer.append( " ");
                                buffer.append( aAttrName );
                                buffer.append( "=\\\"");
                                buffer.append( (*pElement->GetAttributeList())[ j ]->GetValue() );
                                buffer.append( "\\\"");
                            }
                        }
                    }
                    if ( !pElement->GetChildList())
                    {
                        buffer.append( "/\\>");
                    }
                    else
                    {
                        buffer.append( "\\>" );
                        XMLChildNode* tmp=NULL;
                        for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        {
                            tmp = (*pElement->GetChildList())[ k ];
                            Print( tmp, buffer , false);
                        }
                        buffer.append( "\\</" );
                        buffer.append( pElement->GetName() );
                        buffer.append( "\\>" );
                    }
                }
            }
            break;
            case XML_NODE_TYPE_DATA:
            {
                XMLData *pData = ( XMLData * ) pCur;
                buffer.append( pData->GetData() );
            }
            break;
            case XML_NODE_TYPE_COMMENT:
            {
                XMLComment *pComment = ( XMLComment * ) pCur;
                buffer.append( "<!--" );
                buffer.append( pComment->GetComment() );
                buffer.append( "-->" );
            }
            break;
            case XML_NODE_TYPE_DEFAULT:
            {
                XMLDefault *pDefault = ( XMLDefault * ) pCur;
                buffer.append( pDefault->GetDefault() );
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
// class XMLData
//
/*****************************************************************************/
XMLData::XMLData(const XMLData& obj)
/*****************************************************************************/
    : XMLChildNode( obj ),
      sData( obj.sData ) ,
      isNewCreated ( obj.isNewCreated ){}

/*****************************************************************************/
XMLData& XMLData::operator=(const XMLData& obj){
/*****************************************************************************/
    if( this!=&obj ){
        XMLChildNode::operator=( obj );
        sData           = obj.sData;
        isNewCreated    = obj.isNewCreated;
    }
    return *this;
}
/*****************************************************************************/
void XMLData::AddData( const rtl::OString &rData) {
/*****************************************************************************/
    sData += rData;
}

/*****************************************************************************/
sal_uInt16 XMLData::GetNodeType()
/*****************************************************************************/
{
    return XML_NODE_TYPE_DATA;
}

//
// class XMLComment
//

/*****************************************************************************/
sal_uInt16 XMLComment::GetNodeType()
/*****************************************************************************/
{
    return XML_NODE_TYPE_COMMENT;
}
/*****************************************************************************/
XMLComment::XMLComment(const XMLComment& obj)
/*****************************************************************************/
    : XMLChildNode( obj ),
      sComment( obj.sComment ){}

/*****************************************************************************/
XMLComment& XMLComment::operator=(const XMLComment& obj){
/*****************************************************************************/
    if( this!=&obj ){
        XMLChildNode::operator=( obj );
        sComment        = obj.sComment;
    }
    return *this;
}

//
// class XMLDefault
//

/*****************************************************************************/
sal_uInt16 XMLDefault::GetNodeType()
/*****************************************************************************/
{
    return XML_NODE_TYPE_DEFAULT;
}
/*****************************************************************************/
XMLDefault::XMLDefault(const XMLDefault& obj)
/*****************************************************************************/
    : XMLChildNode( obj ),
      sDefault( obj.sDefault){}

/*****************************************************************************/
XMLDefault& XMLDefault::operator=(const XMLDefault& obj){
/*****************************************************************************/
    if( this!=&obj ){
        XMLChildNode::operator=( obj );
        sDefault        = obj.sDefault;
    }
    return *this;
}


//
// class SimpleXMLParser
//

//#define XML_CHAR_TO_OUSTRING(x) OStringToOUString(OString(x), RTL_TEXTENCODING_UTF8)
//#define XML_CHAR_N_TO_OUSTRING(x,n) OStringToOUString(OString(x,n), RTL_TEXTENCODING_UTF8 )


/*****************************************************************************/
SimpleXMLParser::SimpleXMLParser()
/*****************************************************************************/
                : pXMLFile( NULL )
{
    aParser = XML_ParserCreate( NULL );
    XML_SetUserData( aParser, this );
    XML_SetElementHandler( aParser, (XML_StartElementHandler) StartElementHandler, (XML_EndElementHandler) EndElementHandler );
    XML_SetCharacterDataHandler( aParser, (XML_CharacterDataHandler) CharacterDataHandler );
    XML_SetCommentHandler( aParser, (XML_CommentHandler) CommentHandler );
    XML_SetDefaultHandler( aParser, (XML_DefaultHandler) DefaultHandler );
}

/*****************************************************************************/
SimpleXMLParser::~SimpleXMLParser()
/*****************************************************************************/
{
    XML_ParserFree( aParser );
}

/*****************************************************************************/
void SimpleXMLParser::StartElementHandler(
    void *userData, const XML_Char *name, const XML_Char **atts )
/*****************************************************************************/
{
    (( SimpleXMLParser * ) userData )->StartElement( name, atts );
}


/*****************************************************************************/
void SimpleXMLParser::EndElementHandler(
    void *userData, const XML_Char *name )
/*****************************************************************************/
{
    (( SimpleXMLParser * ) userData )->EndElement( name );
}

/*****************************************************************************/
void SimpleXMLParser::CharacterDataHandler(
    void *userData, const XML_Char *s, int len )
/*****************************************************************************/
{
    (( SimpleXMLParser * ) userData )->CharacterData( s, len );
}

/*****************************************************************************/
void SimpleXMLParser::CommentHandler(
    void *userData, const XML_Char *data )
/*****************************************************************************/
{
    (( SimpleXMLParser * ) userData )->Comment( data );
}

/*****************************************************************************/
void SimpleXMLParser::DefaultHandler(
    void *userData, const XML_Char *s, int len )
/*****************************************************************************/
{
    (( SimpleXMLParser * ) userData )->Default( s, len );
}

/*****************************************************************************/
void SimpleXMLParser::StartElement(
    const XML_Char *name, const XML_Char **atts )
/*****************************************************************************/
{
    rtl::OString sElementName( name );
    XMLElement *pElement = new XMLElement( sElementName, ( XMLParentNode * ) pCurNode );
    pCurNode = pElement;
    pCurData = NULL;

    int i = 0;
    while( atts[i] ) {
        pElement->AddAttribute(
            rtl::OString(  atts[ i ] ),
            rtl::OString( atts[ i + 1 ] ));
        i += 2;
    }
}

/*****************************************************************************/
void SimpleXMLParser::EndElement( const XML_Char *name )
/*****************************************************************************/
{
    // This variable is not used at all, but the the sax C interface can't be changed
    // To prevent warnings this dummy assignment is used
    // +++
    (void) name;

    pCurNode = pCurNode->GetParent();
    pCurData = NULL;
}

/*****************************************************************************/
void SimpleXMLParser::CharacterData(
    const XML_Char *s, int len )
/*****************************************************************************/
{
    if ( !pCurData ){
        rtl::OString x( s, len );
        XMLUtil::UnQuotHTML(x);
        pCurData = new XMLData( x , pCurNode );
    }else{
        rtl::OString x( s, len );
        XMLUtil::UnQuotHTML(x);
        pCurData->AddData( x );

    }
}

/*****************************************************************************/
void SimpleXMLParser::Comment(
    const XML_Char *data )
/*****************************************************************************/
{
    pCurData = NULL;
    new XMLComment( OString( data ), pCurNode );
}

/*****************************************************************************/
void SimpleXMLParser::Default(
    const XML_Char *s, int len )
/*****************************************************************************/
{
    pCurData = NULL;
    new XMLDefault( OString( s, len), pCurNode );
}

/*****************************************************************************/
XMLFile *SimpleXMLParser::Execute( const rtl::OString &rFileName, XMLFile* pXMLFileIn )
/*****************************************************************************/
{
    aErrorInformation.eCode = XML_ERROR_NONE;
    aErrorInformation.nLine = 0;
    aErrorInformation.nColumn = 0;
    aErrorInformation.sMessage = "ERROR: Unable to open file ";
    aErrorInformation.sMessage += rFileName;

    rtl::OUString aFileURL(common::pathnameToAbsoluteUrl(rFileName));

    oslFileHandle h;
    if (osl_openFile(aFileURL.pData, &h, osl_File_OpenFlag_Read)
        != osl_File_E_None)
    {
        return 0;
    }

    sal_uInt64 s;
    oslFileError e = osl_getFileSize(h, &s);
    void * p = NULL;
    if (e == osl_File_E_None) {
        e = osl_mapFile(h, &p, s, 0, 0);
    }
    if (e != osl_File_E_None) {
        osl_closeFile(h);
        return 0;
    }

    pXMLFile = pXMLFileIn;
    pXMLFile->SetName( rFileName );

    pCurNode = pXMLFile;
    pCurData = NULL;

    aErrorInformation.eCode = XML_ERROR_NONE;
    aErrorInformation.nLine = 0;
    aErrorInformation.nColumn = 0;
    if ( !pXMLFile->GetName().isEmpty()) {
        aErrorInformation.sMessage = "File " + pXMLFile->GetName() + " parsed successfully";
    }
    else
        aErrorInformation.sMessage = "XML-File parsed successfully";

    if (!XML_Parse(aParser, reinterpret_cast< char * >(p), s, true))
    {
        aErrorInformation.eCode = XML_GetErrorCode( aParser );
        aErrorInformation.nLine = XML_GetErrorLineNumber( aParser );
        aErrorInformation.nColumn = XML_GetErrorColumnNumber( aParser );

        aErrorInformation.sMessage = rtl::OString( "ERROR: ");
        if ( !pXMLFile->GetName().isEmpty())
            aErrorInformation.sMessage += pXMLFile->GetName();
        else
            aErrorInformation.sMessage += "XML-File (";

        aErrorInformation.sMessage += rtl::OString::valueOf( sal::static_int_cast< sal_Int64 >(aErrorInformation.nLine)) +
            "," + rtl::OString::valueOf( sal::static_int_cast< sal_Int64 >(aErrorInformation.nColumn)) + "): ";

        switch (aErrorInformation.eCode)
        {
        case XML_ERROR_NO_MEMORY:
            aErrorInformation.sMessage += "No memory";
            break;
        case XML_ERROR_SYNTAX:
            aErrorInformation.sMessage += "Syntax";
            break;
        case XML_ERROR_NO_ELEMENTS:
            aErrorInformation.sMessage += "No elements";
            break;
        case XML_ERROR_INVALID_TOKEN:
            aErrorInformation.sMessage += "Invalid token";
            break;
        case XML_ERROR_UNCLOSED_TOKEN:
            aErrorInformation.sMessage += "Unclosed token";
            break;
        case XML_ERROR_PARTIAL_CHAR:
            aErrorInformation.sMessage += "Partial char";
            break;
        case XML_ERROR_TAG_MISMATCH:
            aErrorInformation.sMessage += "Tag mismatch";
            break;
        case XML_ERROR_DUPLICATE_ATTRIBUTE:
            aErrorInformation.sMessage += "Dublicat attribute";
            break;
        case XML_ERROR_JUNK_AFTER_DOC_ELEMENT:
            aErrorInformation.sMessage += "Junk after doc element";
            break;
        case XML_ERROR_PARAM_ENTITY_REF:
            aErrorInformation.sMessage += "Param entity ref";
            break;
        case XML_ERROR_UNDEFINED_ENTITY:
            aErrorInformation.sMessage += "Undefined entity";
            break;
        case XML_ERROR_RECURSIVE_ENTITY_REF:
            aErrorInformation.sMessage += "Recursive entity ref";
            break;
        case XML_ERROR_ASYNC_ENTITY:
            aErrorInformation.sMessage += "Async_entity";
            break;
        case XML_ERROR_BAD_CHAR_REF:
            aErrorInformation.sMessage += "Bad char ref";
            break;
        case XML_ERROR_BINARY_ENTITY_REF:
            aErrorInformation.sMessage += "Binary entity";
            break;
        case XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF:
            aErrorInformation.sMessage += "Attribute external entity ref";
            break;
        case XML_ERROR_MISPLACED_XML_PI:
            aErrorInformation.sMessage += "Misplaced xml pi";
            break;
        case XML_ERROR_UNKNOWN_ENCODING:
            aErrorInformation.sMessage += "Unknown encoding";
            break;
        case XML_ERROR_INCORRECT_ENCODING:
            aErrorInformation.sMessage += "Incorrect encoding";
            break;
        case XML_ERROR_UNCLOSED_CDATA_SECTION:
            aErrorInformation.sMessage += "Unclosed cdata section";
            break;
        case XML_ERROR_EXTERNAL_ENTITY_HANDLING:
            aErrorInformation.sMessage += "External entity handling";
            break;
        case XML_ERROR_NOT_STANDALONE:
            aErrorInformation.sMessage += "Not standalone";
            break;
        case XML_ERROR_NONE:
            break;
        default:
            break;
        }
        delete pXMLFile;
        pXMLFile = NULL;
    }

    osl_unmapMappedFile(h, p, s);
    osl_closeFile(h);

    return pXMLFile;
}

/*****************************************************************************/
void XMLUtil::QuotHTML( rtl::OString &rString )
/*****************************************************************************/
{
    OStringBuffer sReturn;
    for (sal_Int32 i = 0; i < rString.getLength(); ++i) {
        switch (rString[i]) {
        case '\\':
            if (i < rString.getLength()) {
                switch (rString[i + 1]) {
                case '"':
                case '<':
                case '>':
                case '\\':
                    ++i;
                    break;
                }
            }
            // fall through
        default:
            sReturn.append(rString[i]);
            break;

        case '<':
            sReturn.append("&lt;");
            break;

        case '>':
            sReturn.append("&gt;");
            break;

        case '"':
            sReturn.append("&quot;");
            break;

        case '&':
            if (rString.match("&amp;", i))
                sReturn.append("&");
            else
                sReturn.append("&amp;");
            break;
        }
    }
    rString = sReturn.makeStringAndClear();
}

void XMLUtil::UnQuotHTML( rtl::OString &sString )
{
    sal_Int32 i;
    sal_Int32 len = sString.getLength();
    const sal_Char* cursor = sString.getStr();

    for(i = 0; i < len; ++i, ++cursor)
    {
        if(*cursor == '\\')
        {
            break;
        }
        else if(*cursor == '&')
        {
            if(i < len - 5)
            {
                if(cursor[3] == ';')
                {
                    if(cursor[2] == 't')
                    {
                        if(cursor[1] == 'l' || cursor[1] == 'g')
                        {
                            break;
                        }
                    }
                }
                else if(cursor[4] == ';')
                {
                    if(!memcmp(cursor + 1, "amp;" , 4))
                    {
                        break;
                    }
                }
                else if(cursor[5] == ';')
                {
                    if(cursor[3] == 'o')
                    {
                        if(!memcmp(cursor + 1, "quot" , 4))
                        {
                            break;
                        }
                        if(!memcmp(cursor + 1, "apos" , 4))
                        {
                            break;
                        }
                    }
                }
            }
            else if (i < len - 4)
            {
                if(cursor[3] == ';')
                {
                    if(cursor[2] == 't')
                    {
                        if(cursor[1] == 'l' || cursor[1] == 'g')
                        {
                            break;
                        }
                    }
                }
                else if(cursor[4] == ';')
                {
                    if(!memcmp(cursor + 1, "amp;" , 4))
                    {
                        break;
                    }
                }
            }
            else if (i < len - 3)
            {
                if(cursor[3] == ';')
                {
                    if(cursor[2] == 't')
                    {
                        if(cursor[1] == 'l' || cursor[1] == 'g')
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    if(i == len)
    {
        return;
    }
    /* here we have some unquoting to do */
    /* at worse we have only '\' and we need to double the size */
    OStringBuffer sBuff(sString.getLength() * 2);
    sBuff.append(sString.getStr(), i);
    const sal_Char* in = sString.getStr() + i;
    sal_Char* out = &sBuff[i];

    while(*in)
    {
        if(*in == '\\')
        {
            *out++ = '\\';
            *out++ = '\\';
            in += 1;
            i += 1;
            continue;
        }
        else if(*in == '&')
        {
            if(i < len - 5)
            {
                if(in[3] == ';')
                {
                    if(in[2] == 't')
                    {
                        if(in[1] == 'l')
                        {
                            *out++ = '<';
                            in += 4;
                            i += 4;
                            continue;
                        }
                        else if (in[1] == 'g')
                        {
                            *out++ = '>';
                            in += 4;
                            i += 4;
                            continue;
                        }
                    }
                }
                else if(in[4] == ';')
                {
                    if(!memcmp(in + 1, "amp;" , 4))
                    {
                        *out++ = '&';
                        in += 5;
                        i += 5;
                        continue;
                    }
                }
                else if(in[5] == ';')
                {
                    if(in[3] == 'o')
                    {
                        if(!memcmp(in + 1, "quot" , 4))
                        {
                            *out++ ='"';
                            in += 6;
                            i += 6;
                            break;
                        }
                        if(!memcmp(in + 1, "apos" , 4))
                        {
                            *out++ ='\'';
                            in += 6;
                            i += 6;
                            break;
                        }
                    }
                }
            }
            else if (i < len - 4)
            {
                if(in[3] == ';')
                {
                    if(in[2] == 't')
                    {
                        if(in[1] == 'l')
                        {
                            *out++ = '<';
                            in += 4;
                            i += 4;
                            continue;
                        }
                        else if (in[1] == 'g')
                        {
                            *out++ = '>';
                            in += 4;
                            i += 4;
                            continue;
                        }
                    }
                }
                else if(in[4] == ';')
                {
                    if(!memcmp(in + 1, "amp;" , 4))
                    {
                        *out++ = '&';
                        in += 5;
                        i += 5;
                        continue;
                    }
                }
            }
            else if (i < len - 3)
            {
                if(in[3] == ';')
                {
                    if(in[2] == 't')
                    {
                        if(in[1] == 'l')
                        {
                            *out++ = '<';
                            in += 4;
                            i += 4;
                            continue;
                        }
                        else if (in[1] == 'g')
                        {
                            *out++ = '>';
                            in += 4;
                            i += 4;
                            continue;
                        }
                    }
                }
            }
        }
        *out++ = *in++;
        i += 1;
    }
    *out = 0;
    sString = OString(sBuff.getStr());

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
