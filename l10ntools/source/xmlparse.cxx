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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_l10ntools.hxx"
#include <iterator> /* std::iterator*/

#include <stdio.h>
#include <sal/alloca.h>

#include "xmlparse.hxx"
#include <fstream>
#include <iostream>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#ifdef __MINGW32__
#include <prewin.h>
#include <postwin.h>
#endif
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
    if( pChildList ){
            RemoveAndDeleteAllChilds();
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
            RemoveAndDeleteAllChilds();
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
void XMLParentNode::AddChild( XMLChildNode *pChild , size_t pos )
/*****************************************************************************/
{
    if ( !pChildList )
        pChildList = new XMLChildNodeList();
    if ( pos < pChildList->size() )
    {
        XMLChildNodeList::iterator it = pChildList->begin();
        ::std::advance( it, pos );
        pChildList->insert( it, pChild );
    } else {
        pChildList->push_back( pChild );
    }
}

/*****************************************************************************/
int XMLParentNode::GetPosition( ByteString id ){
/*****************************************************************************/
    XMLElement* a;

    static const ByteString sEnusStr = ByteString(String::CreateFromAscii(ENGLISH_US_ISO).ToLowerAscii() , RTL_TEXTENCODING_ASCII_US ).ToLowerAscii();
    static const ByteString sDeStr   = ByteString(String::CreateFromAscii(GERMAN_ISO2).ToLowerAscii()    , RTL_TEXTENCODING_ASCII_US ).ToLowerAscii();

    if ( pChildList ){
        for ( size_t i = 0; i < pChildList->size(); i++ ) {
            XMLChildNode *pChild = (*pChildList)[ i ];
            if ( pChild->GetNodeType() == XML_NODE_TYPE_ELEMENT ){
                a = static_cast<XMLElement* >(pChild);
                ByteString elemid( a->GetId() );
                elemid.ToLowerAscii();
                if (   elemid.Equals( id.ToLowerAscii() ) ){
                    ByteString elemLID( a->GetLanguageId() );
                    elemLID.ToLowerAscii();
                    if( elemLID.Equals( sEnusStr) ) {
                        return i;
                    }
                    else if( elemLID.Equals( sDeStr) ) {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}

/*****************************************************************************/
size_t XMLParentNode::RemoveChild( XMLElement *pRefElement )
/*****************************************************************************/
{
    XMLElement* a;
    if ( pChildList ){
        for ( size_t i = 0; i < pChildList->size(); i++ ) {
            XMLChildNode *pChild = (*pChildList)[ i ];
            if ( pChild->GetNodeType() == XML_NODE_TYPE_ELEMENT ){
                a = static_cast<XMLElement* >(pChild);
                ByteString elemid( a->GetId() );
                elemid.ToLowerAscii();
                ByteString elemLID( a->GetLanguageId() );
                elemLID.ToLowerAscii();
                ByteString pRefLID( pRefElement->GetLanguageId() );
                pRefLID.ToLowerAscii();
                if ( elemid.Equals(pRefElement->GetId())
                    && elemLID.Equals( pRefLID ) )
                {
                    if( pRefElement->ToOString().compareTo( a->ToOString() )==0 ){
                        XMLChildNodeList::iterator it = pChildList->begin();
                        ::std::advance( it, i );
                        pChildList->erase( it );
                        delete a; // Test
                        return i;
                    }
                }
            }

        }
    }
    return -1;
}

/*****************************************************************************/
void XMLParentNode::RemoveAndDeleteAllChilds(){
/*****************************************************************************/
    if ( pChildList ) {
        for ( size_t i = 0; i < pChildList->size(); i++ )
            delete (*pChildList)[ i ];
        pChildList->clear();
    }
}

/*****************************************************************************/
XMLElement *XMLParentNode::GetChildElement( XMLElement *pRefElement )
/*****************************************************************************/
{
    for ( size_t i = 0; i < pChildList->size(); i++ ) {
        XMLChildNode *pChild = (*pChildList)[ i ];
        if ( pChild->GetNodeType() == XML_NODE_TYPE_ELEMENT )
            if ((( XMLElement * ) pChild )->GetName() ==
                pRefElement->GetName())
            {
                XMLAttributeList *pList = pRefElement->GetAttributeList();
                if ( !pList )
                    return ( XMLElement * ) pChild;

                sal_Bool bMatch = sal_False;
                for ( size_t j = 0; j < pList->size() && bMatch; j++ ) {
                    XMLAttribute *pAttribute = (*pList)[ j ];
                    XMLAttribute *pCandidate =
                        (( XMLElement * ) pChild )->GetAttribute(
                            *pAttribute );
                    if ( !pCandidate || !pAttribute->IsEqual( *pCandidate ))
                        bMatch = sal_False;
                }
                if ( bMatch )
                    return ( XMLElement * ) pChild;
            }
    }
    return NULL;
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

/*****************************************************************************/
sal_Bool XMLFile::Write( ByteString &aFilename )
/*****************************************************************************/
{

    if ( aFilename.Len()) {
        // retry harder if there is a NFS problem,
        for( int x = 1 ; x < 3 ; x++ ){ // this looks strange...yes!
            ofstream aFStream( aFilename.GetBuffer() , ios::out | ios::trunc );

            if( !aFStream )     // From time to time the stream can not be opened the first time on NFS volumes,
            {                   // I wasn't able to track this down. I think this is an NFS issue .....
                TimeValue aTime;
                aTime.Seconds = 3;
                aTime.Nanosec = 0;

                osl::Thread::wait( aTime );
            }
            else
            {
                // write out
                Write( aFStream );
                aFStream.close();

                // check!
                DirEntry aTarget( aFilename );
                FileStat aFileStat( aTarget );

                if( aFileStat.GetSize() < 1 )
                {
                    //retry
                    aTarget.Kill();
                }
                else
                {
                    //everything ok!
                    return true;
                }
            }
        }
        cerr << "ERROR: - helpex - Can't create file " << aFilename.GetBuffer() << "\nPossible reason: Disk full ? Mounted NFS volume broken ? Wrong permissions ?\n";
        exit( -1 );
    }
    cerr << "ERROR: - helpex - Empty file name\n";
    exit( -1 );
}



void XMLFile::WriteString( ofstream &rStream, const String &sString )
{
    ByteString sText( sString, RTL_TEXTENCODING_UTF8 );
    rStream << sText.GetBuffer();
}


sal_Bool XMLFile::Write( ofstream &rStream , XMLNode *pCur )
{
    XMLUtil& xmlutil = XMLUtil::Instance();
    (void) xmlutil;

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
                        String sData(* (*pElement->GetAttributeList())[ j ] );
                        xmlutil.QuotHTML( sData );
                        WriteString( rStream , sData );
                        rStream << "=\"";
                        sData = (*pElement->GetAttributeList())[ j ]->GetValue();
                        xmlutil.QuotHTML(  sData );
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
                String sData( pData->GetData());
                xmlutil.QuotHTML( sData );
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

                fprintf( stdout, "<%s", ByteString( pElement->GetName(), RTL_TEXTENCODING_UTF8 ).GetBuffer());
                if ( pElement->GetAttributeList())
                    for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ ){
                        ByteString aAttrName( *(*pElement->GetAttributeList())[ j ], RTL_TEXTENCODING_UTF8 );
                        if( !aAttrName.EqualsIgnoreCaseAscii( XML_LANG ) ) {
                            fprintf( stdout, " %s=\"%s\"",
                                aAttrName.GetBuffer(),
                                ByteString( (*pElement->GetAttributeList())[ j ]->GetValue(),
                                    RTL_TEXTENCODING_UTF8 ).GetBuffer());
                        }
                    }
                if ( !pElement->GetChildList())
                    fprintf( stdout, "/>" );
                else {
                    fprintf( stdout, ">" );
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        Print( (*pElement->GetChildList())[ k ], nLevel + 1 );
                    fprintf( stdout, "</%s>", ByteString( pElement->GetName(), RTL_TEXTENCODING_UTF8 ).GetBuffer());
                }
            }
            break;
            case XML_NODE_TYPE_DATA: {
                XMLData *pData = ( XMLData * ) pCur;
                String sData = pData->GetData();
                fprintf( stdout, "%s", ByteString( sData, RTL_TEXTENCODING_UTF8 ).GetBuffer());
            }
            break;
            case XML_NODE_TYPE_COMMENT: {
                XMLComment *pComment = ( XMLComment * ) pCur;
                fprintf( stdout, "<!--%s-->", ByteString( pComment->GetComment(), RTL_TEXTENCODING_UTF8 ).GetBuffer());
            }
            break;
            case XML_NODE_TYPE_DEFAULT: {
                XMLDefault *pDefault = ( XMLDefault * ) pCur;
                fprintf( stdout, "%s", ByteString( pDefault->GetDefault(), RTL_TEXTENCODING_UTF8 ).GetBuffer());
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
XMLFile::XMLFile( const String &rFileName ) // the file name, empty if created from memory stream
/*****************************************************************************/
                : XMLParentNode( NULL ),
                  sFileName    ( rFileName ),
                  ID           ( "id" ),
                  OLDREF       ( "oldref" ),
                  XML_LANG     ( "xml-lang" ),
                  XMLStrings   ( NULL )

{
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("bookmark"),RTL_TEXTENCODING_ASCII_US) , sal_True) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("variable"),RTL_TEXTENCODING_ASCII_US) , sal_True) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("paragraph"),RTL_TEXTENCODING_ASCII_US) , sal_True) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("alt"),RTL_TEXTENCODING_ASCII_US) , sal_True) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("caption"),RTL_TEXTENCODING_ASCII_US) , sal_True) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("title"),RTL_TEXTENCODING_ASCII_US) , sal_True) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("link"),RTL_TEXTENCODING_ASCII_US) , sal_True) );
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
void XMLFile::View(){
/*****************************************************************************/
    XMLElement* cur;
    for(XMLHashMap::iterator pos=XMLStrings->begin(); pos!=XMLStrings->end();++pos){
        fprintf(stdout,"\nid=%s\n",(pos->first).GetBuffer());
        LangHashMap* elem=pos->second;
        for(LangHashMap::iterator pos2=elem->begin(); pos2!=elem->end();++pos2){
            fprintf( stdout,"\nlanguage=%s\n",(pos2->first).GetBuffer() );
            cur=pos2->second;
            fprintf(stdout,"\n%s\n",((XMLElement*)cur)->ToOString().getStr());

        }
    }
}

/*****************************************************************************/
void XMLFile::InsertL10NElement( XMLElement* pElement ){
/*****************************************************************************/
    ByteString tmpStr,id,oldref,language("");
    LangHashMap* elem;

    if( pElement->GetAttributeList() != NULL ){
        for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ ){
            tmpStr=ByteString( *(*pElement->GetAttributeList())[ j ], RTL_TEXTENCODING_UTF8 );
            if( tmpStr.CompareTo(ID)==COMPARE_EQUAL  ){ // Get the "id" Attribute
                id = ByteString( (*pElement->GetAttributeList())[ j ]->GetValue(),RTL_TEXTENCODING_UTF8 );
            }
            if( tmpStr.CompareTo( XML_LANG ) == COMPARE_EQUAL ){    // Get the "xml-lang" Attribute
                language = ByteString( (*pElement->GetAttributeList())[ j ]->GetValue(),RTL_TEXTENCODING_UTF8 );
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
            fprintf(stdout,"Error: Duplicated entry. ID = %s  LANG = %s in File %s\n", id.GetBuffer(), language.GetBuffer(), ByteString( sFullName,RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
            exit( -1 );
        }
        (*elem)[ language ]=pElement;
    }
}
/*****************************************************************************/
void XMLFile::showType(XMLParentNode* node){
/*****************************************************************************/
    switch (node->GetNodeType()){
        case XML_NODE_TYPE_ELEMENT: fprintf(stdout,"ELEMENT\n") ;break;
        case XML_NODE_TYPE_FILE:    fprintf(stdout,"FILE\n")    ;break;
        case XML_NODE_TYPE_COMMENT: fprintf(stdout,"COMMENT\n") ;break;
        case XML_NODE_TYPE_DATA:    fprintf(stdout,"DATA\n")    ;break;
        case XML_NODE_TYPE_DEFAULT: fprintf(stdout,"DEFAULT\n") ;break;
        default: break;
    }
}
XMLFile::XMLFile()
/*****************************************************************************/
                : XMLParentNode( NULL ),
                  ID           ( "id" ),
                  OLDREF       ( "oldref" ),
                  XML_LANG     ( "xml-lang" ),
                  XMLStrings   ( NULL ){};


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
                    printf("*");
                }
                (*XMLStrings)[ pos->first ] = newelem;
            }
        }
    }
    printf("done!\n");
    return *this;
}


/*****************************************************************************/
void XMLFile::SearchL10NElements( XMLParentNode *pCur , int pos)
/*****************************************************************************/
{
    static const ByteString LOCALIZE("localize");
    static const ByteString THEID("id");
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
                ByteString sName(pElement->GetName(),RTL_TEXTENCODING_ASCII_US);
                ByteString language,tmpStrVal,oldref;
                if ( pElement->GetAttributeList()){
                    for ( size_t j = 0 , cnt = pElement->GetAttributeList()->size(); j < cnt && bInsert; j++ ){
                        const ByteString tmpStr( *(*pElement->GetAttributeList())[ j ],RTL_TEXTENCODING_UTF8 );
                        if( tmpStr.CompareTo(THEID)==COMPARE_EQUAL  ){  // Get the "id" Attribute
                            tmpStrVal=ByteString( (*pElement->GetAttributeList())[ j ]->GetValue(),RTL_TEXTENCODING_UTF8 );
                        }
                        if( tmpStr.CompareTo(LOCALIZE)==COMPARE_EQUAL  ){   // Get the "localize" Attribute
                            bInsert=false;
                        }
                        if( tmpStr.CompareTo(XML_LANG)==COMPARE_EQUAL ){    // Get the "xml-lang" Attribute
                            language=ByteString( (*pElement->GetAttributeList())[ j ]->GetValue(),RTL_TEXTENCODING_UTF8 );
                        }
                        if( tmpStr.CompareTo(OLDREF)==COMPARE_EQUAL ){  // Get the "oldref" Attribute
                            oldref=ByteString( (*pElement->GetAttributeList())[ j ]->GetValue(),RTL_TEXTENCODING_UTF8 );
                        }
                    }
                    pElement->SetLanguageId ( language );
                    pElement->SetId         ( tmpStrVal.GetBuffer() );
                    pElement->SetOldRef     ( oldref  );
                    pElement->SetPos( pos );
                }

                if ( bInsert && ( nodes_localize.find( sName.ToLowerAscii() ) != nodes_localize.end() ) )
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
    const ByteString LOCALIZE("localize");
    const ByteString STATUS("status");
    const ByteString PUBLISH("PUBLISH");
    const ByteString DEPRECATED("DEPRECATED");

    const ByteString TOPIC("topic");
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
                ByteString sName(pElement->GetName(),RTL_TEXTENCODING_ASCII_US);
                if( sName.EqualsIgnoreCaseAscii( TOPIC ) ){
                    if ( pElement->GetAttributeList()){
                        for ( size_t j = 0 , cnt = pElement->GetAttributeList()->size(); j < cnt && bInsert; j++ ){
                            const ByteString tmpStr( *(*pElement->GetAttributeList())[ j ],RTL_TEXTENCODING_UTF8 );
                            if( tmpStr.EqualsIgnoreCaseAscii( STATUS ) ){
                                ByteString tmpStrVal=ByteString( (*pElement->GetAttributeList())[ j ]->GetValue(),RTL_TEXTENCODING_UTF8 );
                                if( !tmpStrVal.EqualsIgnoreCaseAscii( PUBLISH )  &&
                                    !tmpStrVal.EqualsIgnoreCaseAscii( DEPRECATED )){
                                    bStatusExport = false;
                                }
                            }

                        }
                    }
                }
                else if ( pElement->GetChildList() ){
                    for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ )
                        CheckExportStatus( (XMLParentNode*)(*pElement->GetChildList())[ k ] );
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
            AddAttribute( *(*obj.pAttributes)[ i ], (*obj.pAttributes)[ i ]->GetValue() );
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
                AddAttribute( *(*obj.pAttributes)[ i ], (*obj.pAttributes)[ i ]->GetValue() );
        }
    }
    return *this;
}

/*****************************************************************************/
void XMLElement::AddAttribute( const String &rAttribute, const String &rValue )
/*****************************************************************************/
{
    if ( !pAttributes )
        pAttributes = new XMLAttributeList();
    pAttributes->push_back( new XMLAttribute( rAttribute, rValue ) );
}

/*****************************************************************************/
void XMLElement::ChangeLanguageTag( const String &rValue ){
/*****************************************************************************/
    static const String rName = String::CreateFromAscii("xml-lang");
    SetLanguageId( ByteString(rValue,RTL_TEXTENCODING_UTF8) );
    if ( pAttributes ){
        for ( size_t i = 0; i < pAttributes->size(); i++ ){
            if ( *(*pAttributes)[ i ] == rName ){
                (*pAttributes)[ i ]->setValue(rValue);
            }
        }
    }
    XMLChildNode* pNode  = NULL;
    XMLElement*   pElem  = NULL;
    XMLChildNodeList* pCList = GetChildList();

    if( pCList != NULL ){
        for ( size_t i = 0; i < pCList->size(); i++ ){
            pNode = (*pCList)[ i ];
            if( pNode != NULL && pNode->GetNodeType() == XML_NODE_TYPE_ELEMENT ){
                pElem = static_cast< XMLElement* >(pNode);
                pElem->ChangeLanguageTag( rValue );
                pElem->SetLanguageId( ByteString(rValue,RTL_TEXTENCODING_UTF8) );
                pElem  = NULL;
                pNode  = NULL;
            }
        }
        pCList = NULL;
    }
}
/*****************************************************************************/
XMLAttribute *XMLElement::GetAttribute( const String &rName )
/*****************************************************************************/
{
    if ( pAttributes )
        for ( size_t i = 0; i < pAttributes->size(); i++ )
            if ( *(*pAttributes)[ i ] == rName )
                return (*pAttributes)[ i ];

    return NULL;
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
bool    XMLElement::Equals(OUString refStr){
/*****************************************************************************/
    return  refStr.equals( ToOUString() );
}

/*****************************************************************************/
OString XMLElement::ToOString(){
/*****************************************************************************/
    OUString ouEmpty;

    OUStringBuffer* buffer = new OUStringBuffer();
    Print( this, *buffer , true );

    OString result( (sal_Unicode* )buffer->getStr(), buffer->getLength() , RTL_TEXTENCODING_UTF8 );
    delete buffer;
    return result;
}
/*****************************************************************************/
OUString XMLElement::ToOUString(){
/*****************************************************************************/
    OUStringBuffer* buffer = new OUStringBuffer();
    Print(this,*buffer,true);
    OUString result=buffer->makeStringAndClear();
    String xy(result.getStr());
    result=OUString(xy);
    delete buffer;
    return result;
}
/*****************************************************************************/
void XMLElement::Print(XMLNode *pCur, OUStringBuffer& buffer , bool rootelement ){
/*****************************************************************************/
    //YD FIXME somewhere COMMENT is defined as 4!
    static const String _COMMENT = String::CreateFromAscii("comment");
    static const OUString XML_LANG ( RTL_CONSTASCII_USTRINGPARAM("xml-lang") );

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

                if(  !pElement->GetName().EqualsIgnoreCaseAscii( _COMMENT ) ){
                    buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("\\<")) );
                    buffer.append( pElement->GetName() );
                    if ( pElement->GetAttributeList()){
                        for ( size_t j = 0; j < pElement->GetAttributeList()->size(); j++ ){

                            OUString aAttrName( *(*pElement->GetAttributeList())[ j ] );
                            if( !aAttrName.equalsIgnoreAsciiCase( XML_LANG ) ) {
                                buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM(" ")) );
                                buffer.append( aAttrName );
                                buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("=")) );
                                buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("\\\"")) );
                                buffer.append( (*pElement->GetAttributeList())[ j ]->GetValue() );
                                buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("\\\"")) );
                            }
                        }
                    }
                    if ( !pElement->GetChildList())
                        buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("/\\>")) );
                    else {
                        buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("\\>")) );
                        XMLChildNode* tmp=NULL;
                        for ( size_t k = 0; k < pElement->GetChildList()->size(); k++ ){
                            tmp = (*pElement->GetChildList())[ k ];
                            Print( tmp, buffer , false);
                        }
                        buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("\\</")) );
                        buffer.append( pElement->GetName() );
                        buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("\\>")) );
                    }
                }
            }
            break;
            case XML_NODE_TYPE_DATA: {
                XMLData *pData = ( XMLData * ) pCur;
                String sData = pData->GetData();
                buffer.append( sData );
            }
            break;
            case XML_NODE_TYPE_COMMENT: {
                XMLComment *pComment = ( XMLComment * ) pCur;
                buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("<!--")) );
                buffer.append( pComment->GetComment() );
                buffer.append( OUString(RTL_CONSTASCII_USTRINGPARAM("-->")) );
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
void XMLData::AddData( const String &rData) {
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

#define XML_CHAR_TO_OUSTRING(x) OStringToOUString(OString(x), RTL_TEXTENCODING_UTF8)
#define XML_CHAR_N_TO_OUSTRING(x,n) OStringToOUString(OString(x,n), RTL_TEXTENCODING_UTF8 )


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
    String sElementName = String( XML_CHAR_TO_OUSTRING( name ));
    XMLElement *pElement = new XMLElement( sElementName, ( XMLParentNode * ) pCurNode );
    pCurNode = pElement;
    pCurData = NULL;

    int i = 0;
    while( atts[i] ) {
        pElement->AddAttribute(
            String( XML_CHAR_TO_OUSTRING( atts[ i ] )),
            String( XML_CHAR_TO_OUSTRING( atts[ i + 1 ] )));
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
        String x=String( XML_CHAR_N_TO_OUSTRING( s, len ));
        XMLUtil::UnQuotHTML(x);
        pCurData = new XMLData( x , pCurNode );
    }else{
        String x=String( XML_CHAR_N_TO_OUSTRING( s, len ));
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
        new XMLComment( String( XML_CHAR_TO_OUSTRING( data )), pCurNode );
}

/*****************************************************************************/
void SimpleXMLParser::Default(
    const XML_Char *s, int len )
/*****************************************************************************/
{
    pCurData = NULL;
    new XMLDefault(
        String( XML_CHAR_N_TO_OUSTRING( s, len )), pCurNode );
}

/*****************************************************************************/
XMLFile *SimpleXMLParser::Execute( const String &rFullFileName , const String &rFileName, XMLFile* pXMLFileIn )
/*****************************************************************************/
{
//  printf("DBG: SimpleXMLParser::Execute( %s )", ByteString( rFileName , RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
    aErrorInformation.eCode = XML_ERROR_NONE;
    aErrorInformation.nLine = 0;
    aErrorInformation.nColumn = 0;
    aErrorInformation.sMessage = String::CreateFromAscii( "ERROR: Unable to open file " );
    aErrorInformation.sMessage += rFileName;

    SvFileStream aStream( rFileName, STREAM_STD_READ );

    if ( !aStream.IsOpen())
        return NULL;

    SvMemoryStream aMemStream;
    aStream >> aMemStream;
    aMemStream.Seek( 0 );

    aStream.Close();

    pXMLFile = pXMLFileIn;
    pXMLFile->SetName( rFileName );
    pXMLFile->SetFullName( rFullFileName );

    return Execute( &aMemStream );
}

/*****************************************************************************/
XMLFile *SimpleXMLParser::Execute( SvMemoryStream *pStream )
/*****************************************************************************/
{
    if ( !pXMLFile )
        pXMLFile = new XMLFile( String());

    pCurNode = pXMLFile;
    pCurData = NULL;

    sal_uLong nPos = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_END );

    aErrorInformation.eCode = XML_ERROR_NONE;
    aErrorInformation.nLine = 0;
    aErrorInformation.nColumn = 0;
    if ( pXMLFile->GetName().Len()) {
        aErrorInformation.sMessage = String::CreateFromAscii( "File " );
        aErrorInformation.sMessage += pXMLFile->GetName();
        aErrorInformation.sMessage += String::CreateFromAscii( " parsed succesfully" );
    }
    else
        aErrorInformation.sMessage = String::CreateFromAscii( "XML-File parsed successfully" );

    if ( !XML_Parse(
        aParser, ( char * ) pStream->GetData() + nPos, pStream->Tell() - nPos, sal_True ))
    {
        aErrorInformation.eCode = XML_GetErrorCode( aParser );
        aErrorInformation.nLine = XML_GetErrorLineNumber( aParser );
        aErrorInformation.nColumn = XML_GetErrorColumnNumber( aParser );

        aErrorInformation.sMessage = String::CreateFromAscii( "ERROR: " );
        if ( pXMLFile->GetName().Len())
            aErrorInformation.sMessage += pXMLFile->GetName();
        else
            aErrorInformation.sMessage += String::CreateFromAscii( "XML-File" );
        aErrorInformation.sMessage += String::CreateFromAscii( " (" );
        aErrorInformation.sMessage += String::CreateFromInt64( aErrorInformation.nLine );
        aErrorInformation.sMessage += String::CreateFromAscii( "," );
        aErrorInformation.sMessage += String::CreateFromInt64( aErrorInformation.nColumn );
        aErrorInformation.sMessage += String::CreateFromAscii( "): " );

        switch( aErrorInformation.eCode ) {
              case XML_ERROR_NO_MEMORY: aErrorInformation.sMessage += String::CreateFromAscii( "No memory" ); break;
              case XML_ERROR_SYNTAX: aErrorInformation.sMessage += String::CreateFromAscii( "Syntax" ); break;
              case XML_ERROR_NO_ELEMENTS: aErrorInformation.sMessage += String::CreateFromAscii( "No elements" ); break;
              case XML_ERROR_INVALID_TOKEN: aErrorInformation.sMessage += String::CreateFromAscii( "Invalid token" ); break;
              case XML_ERROR_UNCLOSED_TOKEN: aErrorInformation.sMessage += String::CreateFromAscii( "Unclosed token" ); break;
              case XML_ERROR_PARTIAL_CHAR: aErrorInformation.sMessage += String::CreateFromAscii( "Partial char" ); break;
              case XML_ERROR_TAG_MISMATCH: aErrorInformation.sMessage += String::CreateFromAscii( "Tag mismatch" ); break;
              case XML_ERROR_DUPLICATE_ATTRIBUTE: aErrorInformation.sMessage += String::CreateFromAscii( "Dublicat attribute" ); break;
              case XML_ERROR_JUNK_AFTER_DOC_ELEMENT: aErrorInformation.sMessage += String::CreateFromAscii( "Junk after doc element" ); break;
              case XML_ERROR_PARAM_ENTITY_REF: aErrorInformation.sMessage += String::CreateFromAscii( "Param entity ref" ); break;
              case XML_ERROR_UNDEFINED_ENTITY: aErrorInformation.sMessage += String::CreateFromAscii( "Undefined entity" ); break;
              case XML_ERROR_RECURSIVE_ENTITY_REF: aErrorInformation.sMessage += String::CreateFromAscii( "Recursive entity ref" ); break;
              case XML_ERROR_ASYNC_ENTITY: aErrorInformation.sMessage += String::CreateFromAscii( "Async_entity" ); break;
              case XML_ERROR_BAD_CHAR_REF: aErrorInformation.sMessage += String::CreateFromAscii( "Bad char ref" ); break;
              case XML_ERROR_BINARY_ENTITY_REF: aErrorInformation.sMessage += String::CreateFromAscii( "Binary entity" ); break;
              case XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF: aErrorInformation.sMessage += String::CreateFromAscii( "Attribute external entity ref" ); break;
              case XML_ERROR_MISPLACED_XML_PI: aErrorInformation.sMessage += String::CreateFromAscii( "Misplaced xml pi" ); break;
              case XML_ERROR_UNKNOWN_ENCODING: aErrorInformation.sMessage += String::CreateFromAscii( "Unknown encoding" ); break;
              case XML_ERROR_INCORRECT_ENCODING: aErrorInformation.sMessage += String::CreateFromAscii( "Incorrect encoding" ); break;
              case XML_ERROR_UNCLOSED_CDATA_SECTION: aErrorInformation.sMessage += String::CreateFromAscii( "Unclosed cdata section" ); break;
              case XML_ERROR_EXTERNAL_ENTITY_HANDLING: aErrorInformation.sMessage += String::CreateFromAscii( "External entity handling" ); break;
              case XML_ERROR_NOT_STANDALONE: aErrorInformation.sMessage += String::CreateFromAscii( "Not standalone" ); break;
            case XML_ERROR_NONE: break;
            default:
                break;

        }
        delete pXMLFile;
        pXMLFile = NULL;
    }
    pStream->Seek( nPos );

    return pXMLFile;
}

/*****************************************************************************/
void XMLUtil::QuotHTML( String &rString )
/*****************************************************************************/
{
    OUStringBuffer sReturn;
    static const String LT(String::CreateFromAscii("<"));
    static const String QLT(String::CreateFromAscii("&lt;"));
    static const String GT(String::CreateFromAscii(">"));
    static const String QGT(String::CreateFromAscii("&gt;"));
    static const String QUOT(String::CreateFromAscii("\\"));
    static const String QQUOT(String::CreateFromAscii("&quot;"));
    static const String APOS(String::CreateFromAscii("\""));
    static const String QAPOS(String::CreateFromAscii("&apos;"));
    static const String AMP(String::CreateFromAscii("&"));
    static const String QAMP(String::CreateFromAscii("&amp;"));
    static const String SLASH(String::CreateFromAscii("\\"));

    for ( sal_uInt16 i = 0; i < rString.Len(); i++) {
        if ( i < rString.Len()) {
            switch ( rString.GetChar( i )) {
                case '\\': if( i+1 <= rString.Len() ){
                            switch( rString.GetChar( i+1 ) ){
                             case '<':  sReturn.append( LT );i++;break;
                             case '>':  sReturn.append( GT );i++;break;
                             case '\\': sReturn.append( QUOT );i++;break;
                             case '\"': sReturn.append( APOS );i++;break;
                             default:   sReturn.append( SLASH );break;

                           }
                          }
                        break;

                case '<':
                    sReturn.append( QLT );
                    break;

                case '>':
                    sReturn.append( QGT );
                    break;

                case '\"':
                    sReturn.append( QQUOT );
                    break;

                case '&':
                    if (
                          ( ( i + 4 ) < rString.Len()) &&
                          ( String( rString.Copy( i, 5 ) ).Equals( QAMP ) )
                       )
                        sReturn.append( rString.GetChar( i ) );
                    else
                        sReturn.append( QAMP );
                break;

                default:
                    sReturn.append( rString.GetChar( i ) );
                break;
            }
        }
    }
    rString = String( sReturn.makeStringAndClear() );
}

void XMLUtil::UnQuotHTML( String &rString ){
    UnQuotData( rString );
}

void XMLUtil::UnQuotData( String &rString_in ){
    ByteString sReturn;
    ByteString sString( rString_in , RTL_TEXTENCODING_UTF8 );
    while ( sString.Len()) {
        if ( sString.Copy( 0, 1 ) == "\\" ) {
            sReturn += "\\\\";
            sString.Erase( 0, 1 );
        }
        else if ( sString.Copy( 0, 5 ) == "&amp;" ) {
            sReturn += "&";
            sString.Erase( 0, 5 );
        }
        else if ( sString.Copy( 0, 4 ) == "&lt;" ) {
            sReturn += "<";
            sString.Erase( 0, 4 );
        }
        else if ( sString.Copy( 0, 4 ) == "&gt;" ) {
            sReturn += ">";
            sString.Erase( 0, 4 );
        }
        else if ( sString.Copy( 0, 6 ) == "&quot;" ) {
            sReturn += "\"";
            sString.Erase( 0, 6 );
        }
        else if ( sString.Copy( 0, 6 ) == "&apos;" ) {
            sReturn += "\'";
            sString.Erase( 0, 6 );
        }
        else {
            sReturn += sString.GetChar( 0 );
            sString.Erase( 0, 1 );
        }
    }
    rString_in = String(sReturn , RTL_TEXTENCODING_UTF8 );


}

XMLUtil::XMLUtil(){
}


/*****************************************************************************/
void XMLUtil::dump(){
/*****************************************************************************/
    int cnt=1;
    printf("size=%lu\n",static_cast<unsigned long>(lMap.size()));
    for(HashMap::iterator pos = lMap.begin(); pos != lMap.end() ; ++pos){
        fprintf(stdout,"key=%s , value=%d , no=%d\n",pos->first.GetBuffer(),pos->second,cnt++);
    }
}
/*****************************************************************************/
XMLUtil&  XMLUtil::Instance(){
/*****************************************************************************/
    static XMLUtil instance;
    return instance;
}
/*****************************************************************************/
XMLUtil::~XMLUtil(){}
/*****************************************************************************/
/*****************************************************************************/
ByteString XMLUtil::GetIsoLangByIndex( sal_uInt16 nIndex )
/*****************************************************************************/
{
    if(nIndex > 0 && MAX_LANGUAGES >= nIndex )
        return isoArray[nIndex];
    return "";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
