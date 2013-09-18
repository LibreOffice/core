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
    if( pChildList ){
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

void XMLFile::Write( OString const &aFilename )
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
                rStream << pElement->GetName().getStr();
                if ( pElement->GetAttributeList())
                    for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ ) {
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
                else {
                    rStream << ">";
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        Write( rStream, (*pElement->GetChildList())[ k ] );
                    rStream << "</";
                    rStream << pElement->GetName().getStr();
                    rStream << ">";
                   }
            }
            break;
            case XML_NODE_TYPE_DATA: {
                XMLData *pData = ( XMLData * ) pCur;
                OString sData( pData->GetData());
                rStream << XMLUtil::QuotHTML( sData ).getStr();
            }
            break;
            case XML_NODE_TYPE_COMMENT: {
                XMLComment *pComment = ( XMLComment * ) pCur;
                rStream << "<!--";
                rStream <<  pComment->GetComment().getStr();
                rStream << "-->";
            }
            break;
            case XML_NODE_TYPE_DEFAULT: {
                XMLDefault *pDefault = ( XMLDefault * ) pCur;
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
XMLFile::XMLFile( const OString &rFileName ) // the file name, empty if created from memory stream
/*****************************************************************************/
                : XMLParentNode( NULL ),
                  sFileName    ( rFileName ),
                  XMLStrings   ( NULL )

{
    nodes_localize.insert( TagMap::value_type(OString("bookmark") , sal_True) );
    nodes_localize.insert( TagMap::value_type(OString("variable") , sal_True) );
    nodes_localize.insert( TagMap::value_type(OString("paragraph") , sal_True) );
    nodes_localize.insert( TagMap::value_type(OString("alt") , sal_True) );
    nodes_localize.insert( TagMap::value_type(OString("caption") , sal_True) );
    nodes_localize.insert( TagMap::value_type(OString("title") , sal_True) );
    nodes_localize.insert( TagMap::value_type(OString("link") , sal_True) );
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
    OString id,language("");
    LangHashMap* elem;

    if( pElement->GetAttributeList() != NULL ){
        for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ )
        {
            const OString tmpStr((*pElement->GetAttributeList())[ j ]->GetName());
            if (tmpStr == ID) { // Get the "id" Attribute
                id = (*pElement->GetAttributeList())[ j ]->GetValue();
            }
            if (tmpStr == XML_LANG) { // Get the "xml-lang" Attribute
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
            fprintf(stdout,"Error: Duplicated entry. ID = %s  LANG = %s in File %s\n", id.getStr(), language.getStr(), sFileName.getStr() );
            exit( -1 );
        }
        (*elem)[ language ]=pElement;
    }
}

XMLFile::XMLFile( const XMLFile& obj )
/*****************************************************************************/
                : XMLParentNode( obj ),
                  sFileName    ( obj.sFileName ),
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
    bool bInsert    = true;
    if ( !pCur )
        SearchL10NElements( this  );
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                XMLChildNode* pElement;
                if( GetChildList()){
                    for ( size_t i = 0; i < GetChildList()->size(); i++ ){
                        pElement = (*GetChildList())[ i ];
                        if( pElement->GetNodeType() ==  XML_NODE_TYPE_ELEMENT ) SearchL10NElements( (XMLParentNode*) pElement , i);
                    }
                }
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;
                const OString sName(pElement->GetName().toAsciiLowerCase());
                OString language,tmpStrVal,oldref;
                if ( pElement->GetAttributeList())
                {
                    for ( size_t j = 0 , cnt = pElement->GetAttributeList()->size(); j < cnt && bInsert; ++j )
                    {
                        const OString tmpStr = (*pElement->GetAttributeList())[j]->GetName();
                        if (tmpStr == ID) {
                            tmpStrVal=(*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                        if (tmpStr == "localize") {
                            bInsert=false;
                        }
                        if (tmpStr == XML_LANG) { // Get the "xml-lang" Attribute
                            language=(*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                        if (tmpStr == OLDREF) { // Get the "oldref" Attribute
                            oldref=(*pElement->GetAttributeList())[ j ]->GetValue();
                        }
                    }
                    pElement->SetLanguageId ( language );
                    pElement->SetId(tmpStrVal);
                    pElement->SetOldRef     ( oldref  );
                    pElement->SetPos( pos );
                }

                if ( bInsert && ( nodes_localize.find( sName ) != nodes_localize.end() ) )
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
void XMLElement::AddAttribute( const OString &rAttribute, const OString &rValue )
/*****************************************************************************/
{
    if ( !pAttributes )
        pAttributes = new XMLAttributeList();
    pAttributes->push_back( new XMLAttribute( rAttribute, rValue ) );
}

/*****************************************************************************/
void XMLElement::ChangeLanguageTag( const OString &rValue )
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
    OString xy(result.getStr());
    result=OString(xy);
    delete buffer;
    return result;
}
/*****************************************************************************/
void XMLElement::Print(XMLNode *pCur, OStringBuffer& buffer , bool rootelement ){
/*****************************************************************************/
    if(pCur!=NULL){
        if(rootelement){
            XMLElement *pElement = ( XMLElement * ) pCur;
            if ( pElement->GetAttributeList()){
                if ( pElement->GetChildList()){
                    XMLChildNode* tmp=NULL;
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ ){
                        tmp = (*pElement->GetChildList())[ k ];
                        Print( tmp, buffer , false);
                    }
                }
            }
        }
        else{

        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;

                if( !pElement->GetName().equalsIgnoreAsciiCase("comment") ){
                    buffer.append( "<" );
                    buffer.append( pElement->GetName() );
                    if ( pElement->GetAttributeList()){
                        for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ ){

                            const OString aAttrName( (*pElement->GetAttributeList())[ j ]->GetName() );
                            if( !aAttrName.equalsIgnoreAsciiCase( "xml-lang" ) ) {
                                buffer.append( " " );
                                buffer.append( aAttrName );
                                buffer.append( "=" );
                                buffer.append( "\"" );
                                buffer.append( (*pElement->GetAttributeList())[ j ]->GetValue() );
                                buffer.append( "\"" );
                            }
                        }
                    }
                    if ( !pElement->GetChildList())
                        buffer.append( "/>" );
                    else {
                        buffer.append( ">" );
                        XMLChildNode* tmp=NULL;
                        for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ ){
                            tmp = (*pElement->GetChildList())[ k ];
                            Print( tmp, buffer , false);
                        }
                        buffer.append( "</" );
                        buffer.append( pElement->GetName() );
                        buffer.append( ">" );
                    }
                }
            }
            break;
            case XML_NODE_TYPE_DATA: {
                XMLData *pData = ( XMLData * ) pCur;
                buffer.append( pData->GetData() );
            }
            break;
            case XML_NODE_TYPE_COMMENT: {
                XMLComment *pComment = ( XMLComment * ) pCur;
                buffer.append( "<!--" );
                buffer.append( pComment->GetComment() );
                buffer.append( "-->" );
            }
            break;
            case XML_NODE_TYPE_DEFAULT: {
                XMLDefault *pDefault = ( XMLDefault * ) pCur;
                buffer.append( pDefault->GetDefault() );
            }
            break;
        }
        }
    }else {
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
void XMLData::AddData( const OString &rData) {
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

namespace
{

static OUString lcl_pathnameToAbsoluteUrl(const OString& rPathname) {
    OUString sPath = OStringToOUString(rPathname, RTL_TEXTENCODING_UTF8 );
    OUString sUrl;
    if (osl::FileBase::getFileURLFromSystemPath(sPath, sUrl)
        != osl::FileBase::E_None)
    {
        std::cerr << "Error: Cannot convert input pathname to URL\n";
        std::exit(EXIT_FAILURE);
    }
    OUString sCwd;
    if (osl_getProcessWorkingDir(&sCwd.pData) != osl_Process_E_None) {
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
    XMLElement *pElement = new XMLElement( OString(name), ( XMLParentNode * ) pCurNode );
    pCurNode = pElement;
    pCurData = NULL;

    int i = 0;
    while( atts[i] ) {
        pElement->AddAttribute( atts[ i ], atts[ i + 1 ] );
        i += 2;
    }
}

/*****************************************************************************/
void SimpleXMLParser::EndElement( const XML_Char * /*name*/ )
/*****************************************************************************/
{
    pCurNode = pCurNode->GetParent();
    pCurData = NULL;
}

/*****************************************************************************/
void SimpleXMLParser::CharacterData(
    const XML_Char *s, int len )
/*****************************************************************************/
{
    if ( !pCurData ){
        OString x( s, len );
        pCurData = new XMLData( helper::UnQuotHTML(x) , pCurNode );
    }else{
        OString x( s, len );
        pCurData->AddData( helper::UnQuotHTML(x) );

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
    new XMLDefault(OString( s, len ), pCurNode );
}

/*****************************************************************************/
XMLFile *SimpleXMLParser::Execute( const OString &rFileName, XMLFile* pXMLFileIn )
/*****************************************************************************/
{
    aErrorInformation.eCode = XML_ERROR_NONE;
    aErrorInformation.nLine = 0;
    aErrorInformation.nColumn = 0;
    aErrorInformation.sMessage = "ERROR: Unable to open file ";
    aErrorInformation.sMessage += rFileName;

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

        aErrorInformation.sMessage = "ERROR: ";
        if ( !pXMLFile->GetName().isEmpty())
            aErrorInformation.sMessage += pXMLFile->GetName();
        else
            aErrorInformation.sMessage += OString( "XML-File (");
        aErrorInformation.sMessage +=
            OString::number(sal::static_int_cast< sal_Int64 >(aErrorInformation.nLine)) + "," +
            OString::number(sal::static_int_cast< sal_Int64 >(aErrorInformation.nColumn)) + "): ";

        switch (aErrorInformation.eCode) {
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
