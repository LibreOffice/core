/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlparse.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 15:00:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <hash_map> /* std::hashmap*/
#include <deque>    /* std::deque*/
#include <iterator> /* std::iterator*/


#include <stdio.h>
#ifdef WIN32
#include <malloc.h>
#else
#if !(defined(MACOSX) || defined(FREEBSD))
#include <alloca.h>
#endif
#endif

#include "xmlparse.hxx"
#include <fstream>
#include <iostream>
#include "osl/mutex.hxx"

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
XMLNode::XMLNode( const XMLNode& obj)
/*****************************************************************************/
{}

/*****************************************************************************/
XMLNode& XMLNode::operator=(const XMLNode& obj){
/*****************************************************************************/
    return *this;
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
    if ( pChildList != NULL ) {
        XMLChildNode* aChild = NULL;
        for ( ULONG i = 0; i < pChildList->Count(); i++ ){
            aChild = pChildList->GetObject( i );
            if( aChild != NULL )delete aChild;
        }
        delete pChildList;
    }
}
/*****************************************************************************/
XMLParentNode::XMLParentNode( const XMLParentNode& obj)
/*****************************************************************************/
: XMLChildNode( obj )
{
    if( obj.pChildList ){
        pChildList=new XMLChildNodeList();
        XMLChildNode* pNode = NULL;
        for ( ULONG i = 0; i < obj.pChildList->Count(); i++ ){
            pNode = obj.pChildList->GetObject( i );
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
            for ( ULONG i = 0; i < obj.pChildList->Count(); i++ )
                AddChild( obj.pChildList->GetObject( i ) );
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
    pChildList->Insert( pChild, LIST_APPEND );
}

/*****************************************************************************/
void XMLParentNode::AddChild( XMLChildNode *pChild , int pos )
/*****************************************************************************/
{
    if ( !pChildList )
        pChildList = new XMLChildNodeList();
    pChildList->Insert( pChild, pos );
}

/*****************************************************************************/
int XMLParentNode::GetPos( ByteString id ){
/*****************************************************************************/
    XMLElement* a;
    const ByteString sEnusStr = ByteString(String::CreateFromAscii(ENGLISH_US_ISO).ToLowerAscii() , RTL_TEXTENCODING_ASCII_US ).ToLowerAscii();
    const ByteString sDeStr   = ByteString(String::CreateFromAscii(GERMAN_ISO2).ToLowerAscii()    , RTL_TEXTENCODING_ASCII_US ).ToLowerAscii();
    if ( pChildList ){
        for ( ULONG i = 0; i < pChildList->Count(); i++ ) {
            XMLChildNode *pChild = pChildList->GetObject( i );
            if ( pChild->GetNodeType() == XML_NODE_TYPE_ELEMENT ){
                a = static_cast<XMLElement* >(pChild);
                if (   a->GetId().ToLowerAscii().Equals( id.ToLowerAscii() ) ){
                    if( a->GetLanguageId().ToLowerAscii().Equals( sEnusStr) ) {
                        return i;
                    }
                    else if( a->GetLanguageId().ToLowerAscii().Equals( sDeStr) ) {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}

/*****************************************************************************/
int XMLParentNode::RemoveChild( XMLElement *pRefElement )
/*****************************************************************************/
{
    XMLElement* a;
    if ( pChildList ){
        for ( ULONG i = 0; i < pChildList->Count(); i++ ) {
            XMLChildNode *pChild = pChildList->GetObject( i );
            if ( pChild->GetNodeType() == XML_NODE_TYPE_ELEMENT ){
                a = static_cast<XMLElement* >(pChild);
                if (   a->GetId().Equals(pRefElement->GetId())
                    && a->GetLanguageId().ToLowerAscii().Equals( pRefElement->GetLanguageId().ToLowerAscii()) )
                {
                    if( pRefElement->ToOString().compareTo( a->ToOString() )==0 ){
                        pChildList->Remove( i );
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
        for ( ULONG i = 0; i < pChildList->Count(); i++ )
            delete pChildList->GetObject( i );
        pChildList->Clear();
    }
}

/*****************************************************************************/
XMLElement *XMLParentNode::GetChildElement( XMLElement *pRefElement )
/*****************************************************************************/
{
    for ( ULONG i = 0; i < pChildList->Count(); i++ ) {
        XMLChildNode *pChild = pChildList->GetObject( i );
        if ( pChild->GetNodeType() == XML_NODE_TYPE_ELEMENT )
            if ((( XMLElement * ) pChild )->GetName() ==
                pRefElement->GetName())
            {
                XMLAttributeList *pList = pRefElement->GetAttributeList();
                if ( !pList )
                    return ( XMLElement * ) pChild;

                BOOL bMatch = FALSE;
                for ( ULONG j = 0; j < pList->Count() && bMatch; j++ ) {
                    XMLAttribute *pAttribute = pList->GetObject( j );
                    XMLAttribute *pCandidate =
                        (( XMLElement * ) pChild )->GetAttribute(
                            *pAttribute );
                    if ( !pCandidate || !pAttribute->IsEqual( *pCandidate ))
                        bMatch = FALSE;
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
USHORT XMLFile::GetNodeType()
/*****************************************************************************/
{
    return XML_NODE_TYPE_FILE;
}

/*****************************************************************************/
extern "C" void Signal_handler(int signo){
/*****************************************************************************/
    if( signo == SIGINT ){
        printf("Helpex: Currently Writing!!! Ctrl+C aborted\n");
    }
}
/*****************************************************************************/
BOOL XMLFile::Write( String &rFileName )
/*****************************************************************************/
{
    ByteString aFilename( rFileName , RTL_TEXTENCODING_ASCII_US );
    if ( rFileName.Len()) {

        //signal( SIGINT, &::Signal_handler );

        // retry if there is a NFS problem
        bool isOk = false;
        for( int x = 0 ; !isOk && x < 200 ; x++ ){
            DirEntry aTemp( Export::GetTempFile());
            ByteString sTempFile( aTemp.GetFull(), RTL_TEXTENCODING_ASCII_US );

            ofstream aFStream( sTempFile.GetBuffer() , ios::out | ios::trunc );

            if( !aFStream ){
                cerr << "ERROR: - helpex - Can't create tempfile " << sTempFile.GetBuffer() << " No#" << x << "\n";
                //  signal(SIGINT,SIG_DFL); // Enable Ctrl+C
                // isOk = false;
                // exit ( -1 );
            }
            else
            {
                Write( aFStream );
                aFStream.close();
                // signal(SIGINT,SIG_DFL); // Enable Ctrl+C

                DirEntry aTarget( aFilename );
                aTarget.Kill();
                aTemp.MoveTo( aTarget ) ;

                FileStat aFileStat( aTarget );
                if( aFileStat.GetSize() < 1 ){
                    cerr << "WARNING: - helpex - Can't create file " << aFilename.GetBuffer() << " No#" << x << "\n";
                    //exit( -1 );
                    aTarget.Kill();
                    aTemp.Kill();
                }else{
                    return true;
                }

            }
        }
        cerr << "ERROR: - helpex - Can't create file " << aFilename.GetBuffer() << "\nDisk full ? Mounted NFS volume broken ?\n";
        exit( -1 );
        return FALSE;
    } // for
    cerr << "ERROR: - helpex - Empty file name\n";
    exit( -1 );
    return FALSE;
}


/*****************************************************************************/
//void XMLFile::WriteString( SvStream &rStream, const String &sString )
void XMLFile::WriteString( ofstream &rStream, const String &sString )
    /*****************************************************************************/
{
    ByteString sText( sString, RTL_TEXTENCODING_UTF8 );
      //sText.ConvertLineEnd(LINEEND_CRLF);
    rStream << sText.GetBuffer();
}

/*****************************************************************************/
//BOOL XMLFile::Write( SvStream &rStream , XMLNode *pCur )
BOOL XMLFile::Write( ofstream &rStream , XMLNode *pCur )
/*****************************************************************************/
{
    XMLUtil& xmlutil=XMLUtil::Instance();
    if ( !pCur )
        Write( rStream, this );
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                if( GetChildList())
                    for ( ULONG i = 0; i < GetChildList()->Count(); i++ )
                        Write( rStream, GetChildList()->GetObject( i ));
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;
                rStream  << "<";
                WriteString( rStream, pElement->GetName());
                if ( pElement->GetAttributeList())
                    for ( ULONG j = 0; j < pElement->GetAttributeList()->Count(); j++ ) {
                        rStream << " ";
                        String sData(*pElement->GetAttributeList()->GetObject( j ));
                        xmlutil.QuotHTML( sData );
                        WriteString( rStream , sData );
                        rStream << "=\"";
                        sData=pElement->GetAttributeList()->GetObject( j )->GetValue();
                        xmlutil.QuotHTML(  sData );
                        WriteString( rStream , sData  );
                        rStream << "\"";
                    }
                if ( !pElement->GetChildList())
                    rStream << "/>";
                else {
                    rStream << ">";
                    for ( ULONG k = 0; k < pElement->GetChildList()->Count(); k++ )
                        Write( rStream, pElement->GetChildList()->GetObject( k ));
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
    return TRUE;
}

/*****************************************************************************/
void XMLFile::Print( XMLNode *pCur, USHORT nLevel )
/*****************************************************************************/
{
//  const ByteString L10N( "l10n" , RTL_TEXTENCODING_UTF8 );
//  const ByteString ID  ( "id" , RTL_TEXTENCODING_UTF8 );

    if ( !pCur )
        Print( this );
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                if( GetChildList())
                    for ( ULONG i = 0; i < GetChildList()->Count(); i++ )
                        Print( GetChildList()->GetObject( i ));
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;

                fprintf( stdout, "<%s", ByteString( pElement->GetName(), RTL_TEXTENCODING_UTF8 ).GetBuffer());
                if ( pElement->GetAttributeList())
                    for ( ULONG j = 0; j < pElement->GetAttributeList()->Count(); j++ ){
                        ByteString aAttrName( *pElement->GetAttributeList()->GetObject( j ), RTL_TEXTENCODING_UTF8 );
                        //if( !( bNo_XML_Lang && aAttrName.EqualsIgnoreCaseAscii( XML_LANG ) ) ){
                        //if( !aAttrName.EqualsIgnoreCaseAscii( XML_LANG ) ){
                        if( !aAttrName.EqualsIgnoreCaseAscii( XML_LANG ) ) {
                            fprintf( stdout, " %s=\"%s\"",
                                //ByteString( *pElement->GetAttributeList()->GetObject( j ),
                                //    RTL_TEXTENCODING_UTF8 ).GetBuffer(),
                                aAttrName.GetBuffer(),
                                ByteString( pElement->GetAttributeList()->GetObject( j )->GetValue(),
                                    RTL_TEXTENCODING_UTF8 ).GetBuffer());
                        }
                    }
                if ( !pElement->GetChildList())
                    fprintf( stdout, "/>" );
                else {
                    fprintf( stdout, ">" );
                    for ( ULONG k = 0; k < pElement->GetChildList()->Count(); k++ )
                        Print( pElement->GetChildList()->GetObject( k ), nLevel + 1 );
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
/*****************************************************************************/
XMLFile::XMLFile( const String &rFileName ) // the file name, empty if created from memory stream
/*****************************************************************************/
                : XMLParentNode( NULL ),
                  sFileName    ( rFileName ),
                  ID           ( "id" ),
                  OLDREF       ( "oldref" ),
                  XML_LANG     ( "xml-lang" )

{
//  nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("bookmark_value"),RTL_TEXTENCODING_ASCII_US) , TRUE) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("bookmark"),RTL_TEXTENCODING_ASCII_US) , TRUE) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("variable"),RTL_TEXTENCODING_ASCII_US) , TRUE) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("paragraph"),RTL_TEXTENCODING_ASCII_US) , TRUE) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("alt"),RTL_TEXTENCODING_ASCII_US) , TRUE) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("caption"),RTL_TEXTENCODING_ASCII_US) , TRUE) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("title"),RTL_TEXTENCODING_ASCII_US) , TRUE) );
    nodes_localize.insert( TagMap::value_type(ByteString(String::CreateFromAscii("link"),RTL_TEXTENCODING_ASCII_US) , TRUE) );
}

/*****************************************************************************/
void XMLFile::Extract( XMLFile *pCur )
/*****************************************************************************/
{
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
    XMLUtil& rXMLUtil = XMLUtil::Instance();   // Get Singleton
    //int langid=1; // ENGLISH define benutzen !!!
    //ByteString sLangId;
    if( pElement->GetAttributeList() != NULL ){
        for ( ULONG j = 0; j < pElement->GetAttributeList()->Count(); j++ ){
            tmpStr=ByteString( *pElement->GetAttributeList()->GetObject( j ),RTL_TEXTENCODING_UTF8 );
            if( tmpStr.CompareTo(ID)==COMPARE_EQUAL  ){ // Get the "id" Attribute
                id = ByteString( pElement->GetAttributeList()->GetObject( j )->GetValue(),RTL_TEXTENCODING_UTF8 );
            }
            if( tmpStr.CompareTo( XML_LANG ) == COMPARE_EQUAL ){    // Get the "xml-lang" Attribute
                language = ByteString( pElement->GetAttributeList()->GetObject( j )->GetValue(),RTL_TEXTENCODING_UTF8 );
                //langid=rXMLUtil.GetLangByIsoLang(language);
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
        //(*elem)[langid]=pElement;
        (*elem)[ language ]=pElement;
        XMLStrings->insert( XMLHashMap::value_type( id , elem ) );
        order.push_back( id );
    }else{                                  // Already there
        elem=pos->second;
        //(*elem)[langid]=pElement;
        if ( (*elem)[ language ] )
        {
            fprintf(stdout,"Error: Entry for language double. ID = %s  LANG = %s in File %s\n", id.GetBuffer(), language.GetBuffer(), ByteString( sFileName,RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
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
/*****************************************************************************/
void XMLFile::SearchL10NElements( XMLParentNode *pCur , int pos)
/*****************************************************************************/
{
    const ByteString LOCALIZE("localize");
    const ByteString ID("id");
    bool bInsert    = true;
    if ( !pCur )
        SearchL10NElements( this  );
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                XMLParentNode* pElement;
                if( GetChildList()){
                    for ( ULONG i = 0; i < GetChildList()->Count(); i++ ){
                        //SearchL10NElements( (XMLParentNode*) GetChildList()->GetObject( i ) , i);
                        pElement = (XMLParentNode*) GetChildList()->GetObject( i );
                        if( pElement->GetNodeType() ==  XML_NODE_TYPE_ELEMENT ) SearchL10NElements( pElement , i);
                    }
                }
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;
                ByteString sName(pElement->GetName(),RTL_TEXTENCODING_ASCII_US);
                ByteString language,tmpStrVal,tmpStr,oldref;
                if ( pElement->GetAttributeList()){
                    for ( ULONG j = 0 , cnt = pElement->GetAttributeList()->Count(); j < cnt && bInsert; j++ ){
                        const ByteString tmpStr( *pElement->GetAttributeList()->GetObject( j ),RTL_TEXTENCODING_UTF8 );
                        if( tmpStr.CompareTo(ID)==COMPARE_EQUAL  ){ // Get the "id" Attribute
                            tmpStrVal=ByteString( pElement->GetAttributeList()->GetObject( j )->GetValue(),RTL_TEXTENCODING_UTF8 );
                            //printf("Checking id = %s\n",tmpStrVal.GetBuffer() );
                        }
                        if( tmpStr.CompareTo(LOCALIZE)==COMPARE_EQUAL  ){   // Get the "localize" Attribute
                            bInsert=false;
                        }
                        if( tmpStr.CompareTo(XML_LANG)==COMPARE_EQUAL ){    // Get the "xml-lang" Attribute
                            language=ByteString( pElement->GetAttributeList()->GetObject( j )->GetValue(),RTL_TEXTENCODING_UTF8 );
                        }
                        if( tmpStr.CompareTo(OLDREF)==COMPARE_EQUAL ){  // Get the "oldref" Attribute
                            oldref=ByteString( pElement->GetAttributeList()->GetObject( j )->GetValue(),RTL_TEXTENCODING_UTF8 );
                        }
                    }
                    pElement->SetLanguageId ( language );
                    pElement->SetId         ( tmpStrVal.GetBuffer() );
                    pElement->SetOldRef     ( oldref  );
                    pElement->SetPos( pos );
                    //Print( pElement , 0 );
                }

                if ( bInsert && ( nodes_localize.find( sName.ToLowerAscii() ) != nodes_localize.end() ) )
                    InsertL10NElement(pElement);
                else if ( bInsert && pElement->GetChildList() ){
                    for ( ULONG k = 0; k < pElement->GetChildList()->Count(); k++ )
                        SearchL10NElements( (XMLParentNode*) pElement->GetChildList()->GetObject( k ) , k);
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
bool XMLFile::CheckExportStatus( XMLParentNode *pCur , int pos)
/*****************************************************************************/
{
    static bool bStatusExport = true;
    const ByteString LOCALIZE("localize");
    const ByteString STATUS("status");
    const ByteString PUBLISH("PUBLISH");
    const ByteString DEPRECATED("DEPRECATED");

    const ByteString TOPIC("topic");
    const ByteString ID("id");
    bool bInsert    = true;
    if ( !pCur )
        CheckExportStatus( this , 0 );
    else {
        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_FILE: {
                XMLParentNode* pElement;
                if( GetChildList()){
                    for ( ULONG i = 0; i < GetChildList()->Count(); i++ ){
                        pElement = (XMLParentNode*) GetChildList()->GetObject( i );
                        if( pElement->GetNodeType() ==  XML_NODE_TYPE_ELEMENT ) CheckExportStatus( pElement , i);
                    }
                }
            }
            break;
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;
                ByteString sName(pElement->GetName(),RTL_TEXTENCODING_ASCII_US);
                if( sName.EqualsIgnoreCaseAscii( TOPIC ) ){
                    if ( pElement->GetAttributeList()){
                        for ( ULONG j = 0 , cnt = pElement->GetAttributeList()->Count(); j < cnt && bInsert; j++ ){
                            const ByteString tmpStr( *pElement->GetAttributeList()->GetObject( j ),RTL_TEXTENCODING_UTF8 );
                            if( tmpStr.EqualsIgnoreCaseAscii( STATUS ) ){
                                ByteString tmpStrVal=ByteString( pElement->GetAttributeList()->GetObject( j )->GetValue(),RTL_TEXTENCODING_UTF8 );
                                if( !tmpStrVal.EqualsIgnoreCaseAscii( PUBLISH )  &&
                                    !tmpStrVal.EqualsIgnoreCaseAscii( DEPRECATED )){
                                    bStatusExport = false;
                                }
                            }

                        }
                    }
                }
                else if ( pElement->GetChildList() ){
                    for ( ULONG k = 0; k < pElement->GetChildList()->Count(); k++ )
                        CheckExportStatus( (XMLParentNode*) pElement->GetChildList()->GetObject( k ) , k);
                }
            }
            break;
        }
    }
    return bStatusExport;
}

/*****************************************************************************/
USHORT XMLElement::GetNodeType()
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
        for ( ULONG i = 0; i < obj.pAttributes->Count(); i++ )
            AddAttribute( *obj.pAttributes->GetObject( i ) , obj.pAttributes->GetObject( i )->GetValue() );
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
            for ( ULONG i = 0; i < pAttributes->Count(); i++ )
                delete pAttributes->GetObject( i );
            delete pAttributes;
        }
        if ( obj.pAttributes ){
            pAttributes         =new XMLAttributeList();
            for ( ULONG i = 0; i < obj.pAttributes->Count(); i++ )
                AddAttribute( *obj.pAttributes->GetObject( i ) , obj.pAttributes->GetObject( i )->GetValue() );
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
    pAttributes->Insert( new XMLAttribute( rAttribute, rValue ), LIST_APPEND );
}

/*****************************************************************************/
void XMLElement::ChangeLanguageTag( const String &rValue ){
/*****************************************************************************/
    String rName = String::CreateFromAscii("xml-lang");
    SetLanguageId( ByteString(rValue,RTL_TEXTENCODING_UTF8) );
    if ( pAttributes ){
        for ( ULONG i = 0; i < pAttributes->Count(); i++ ){
            if ( *pAttributes->GetObject( i ) == rName ){
                pAttributes->GetObject( i )->setValue(rValue);
            }
        }
    }
    XMLChildNode* pNode  = NULL;
    XMLElement*   pElem  = NULL;
    XMLChildNodeList* pCList = GetChildList();

    if( pCList != NULL ){
        for ( ULONG i = 0; i < pCList->Count(); i++ ){
            pNode = pCList->GetObject( i );
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
        for ( ULONG i = 0; i < pAttributes->Count(); i++ )
            if ( *pAttributes->GetObject( i ) == rName )
                return pAttributes->GetObject( i );

    return NULL;
}

/*****************************************************************************/
XMLElement::~XMLElement()
/*****************************************************************************/
{
    if ( pAttributes ) {
        for ( ULONG i = 0; i < pAttributes->Count(); i++ )
            delete pAttributes->GetObject( i );

        delete pAttributes;
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
    OString test(ouEmpty.getStr(), ouEmpty.getLength(), RTL_TEXTENCODING_UTF8);

    OUStringBuffer* buffer = new OUStringBuffer();
    Print( this, *buffer , true );

    const sal_Unicode* pDbg1 = buffer->getStr();
    sal_uInt32 nDbg2 = buffer->getLength();
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
    const String COMMENT = String::CreateFromAscii("comment");
    XMLUtil& xmlutil=XMLUtil::Instance();
    const OUString XML_LANG ( OUString::createFromAscii("xml-lang") );

    if(pCur!=NULL){
        if(rootelement){
            XMLElement *pElement = ( XMLElement * ) pCur;
            if ( pElement->GetAttributeList()){
                if ( pElement->GetChildList()){
                    XMLChildNode* tmp=NULL;
                    for ( ULONG k = 0; k < pElement->GetChildList()->Count(); k++ ){
                        tmp=pElement->GetChildList()->GetObject( k );
                        Print( tmp, buffer , false);
                    }
                }
            }
        }
        else{

        switch( pCur->GetNodeType()) {
            case XML_NODE_TYPE_ELEMENT: {
                XMLElement *pElement = ( XMLElement * ) pCur;

                if(  !pElement->GetName().EqualsIgnoreCaseAscii( COMMENT ) ){
                    buffer.append( OUString::createFromAscii("\\<") );
                    buffer.append( pElement->GetName() );
                    if ( pElement->GetAttributeList()){
                        for ( ULONG j = 0; j < pElement->GetAttributeList()->Count(); j++ ){

                            OUString aAttrName( *pElement->GetAttributeList()->GetObject( j ) );
                            if( !aAttrName.equalsIgnoreAsciiCase( XML_LANG ) ) {
                                //buffer.append( *pElement->GetAttributeList()->GetObject( j ) );
                                buffer.append( OUString::createFromAscii(" ") );
                                buffer.append( aAttrName );
                                buffer.append( OUString::createFromAscii("=") );
                                buffer.append( OUString::createFromAscii("\\\"") );
                                buffer.append( pElement->GetAttributeList()->GetObject( j )->GetValue() );
                                buffer.append( OUString::createFromAscii("\\\"") );
                            }
                        }
                    }
                    if ( !pElement->GetChildList())
                        buffer.append( OUString::createFromAscii("/\\>") );
                    else {
                        buffer.append( OUString::createFromAscii("\\>") );
                        XMLChildNode* tmp=NULL;
                        for ( ULONG k = 0; k < pElement->GetChildList()->Count(); k++ ){
                            tmp=pElement->GetChildList()->GetObject( k );
                            Print( tmp, buffer , false);
                        }
                        buffer.append( OUString::createFromAscii("\\</") );
                        buffer.append( pElement->GetName() );
                        buffer.append( OUString::createFromAscii("\\>") );
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
                buffer.append( OUString::createFromAscii("<!--") );
                buffer.append( pComment->GetComment() );
                buffer.append( OUString::createFromAscii("-->") );
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
USHORT XMLData::GetNodeType()
/*****************************************************************************/
{
    return XML_NODE_TYPE_DATA;
}

//
// class XMLComment
//

/*****************************************************************************/
USHORT XMLComment::GetNodeType()
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
USHORT XMLDefault::GetNodeType()
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
    XML_SetElementHandler( aParser, StartElementHandler, EndElementHandler );
    XML_SetCharacterDataHandler( aParser, CharacterDataHandler );
    XML_SetCommentHandler( aParser, CommentHandler );
    XML_SetDefaultHandler( aParser, DefaultHandler );

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
void SimpleXMLParser::EndElement(
    const XML_Char *name )
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
    //String z;
    if ( !pCurData ){
        String x=String( XML_CHAR_N_TO_OUSTRING( s, len ));
        XMLUtil::UnQuotHTML(x);
        pCurData = new XMLData( x , pCurNode );
        /*pCurData = new XMLData(
            String( XML_CHAR_N_TO_OUSTRING( s, len )), pCurNode );*/
    }else{
        String x=String( XML_CHAR_N_TO_OUSTRING( s, len ));
        XMLUtil::UnQuotHTML(x);
        pCurData->AddData( x );
        //      pCurData->AddData( String( XML_CHAR_N_TO_OUSTRING( s, len )));
    }
}

/*****************************************************************************/
void SimpleXMLParser::Comment(
    const XML_Char *data )
/*****************************************************************************/
{
    pCurData = NULL;
    new XMLComment(
        String( XML_CHAR_TO_OUSTRING( data )), pCurNode );
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
XMLFile *SimpleXMLParser::Execute( const String &rFileName )
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

    pXMLFile = new XMLFile( rFileName );

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

    ULONG nPos = pStream->Tell();
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
        aParser, ( char * ) pStream->GetData() + nPos, pStream->Tell() - nPos, TRUE ))
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
    ByteString sString( rString, RTL_TEXTENCODING_UTF8 );

    ByteString sReturn;
    BOOL bBreak = FALSE;
    for ( USHORT i = 0; i < sString.Len(); i++) {
        if ( i < sString.Len()) {
            switch ( sString.GetChar( i )) {
                //case '\\': if( i+1 <= sString.Len() ){
                case '\\': if( i+1 <= sString.Len() ){
                            switch( sString.GetChar( i+1 ) ){
                             case '<':  sReturn += "<";i++;break;
                             case '>':  sReturn += ">";i++;break;
                             case '\\': sReturn += "\\";i++;break;
                             case '\"': sReturn += "\"";i++;break;
                             //case '\'': sReturn += "\'";i++;break;
                             //case '&' : sRetrun += "&";i++;break;
                             default:   sReturn += "\\";break;

                           }
                          }
                        break;

                case '<':
                    sReturn += "&lt;";

                break;

                case '>':
                    sReturn += "&gt;";
                break;

                case '\"':
                    sReturn += "&quot;";
                break;

/*              case '\'':
                    sReturn += "&apos;";
                    break;
*/
                case '&':
                    if ((( i + 4 ) < sString.Len()) &&
                        ( sString.Copy( i, 5 ) == "&amp;" ))
                            sReturn += sString.GetChar( i );
                    else
                        sReturn += "&amp;";
                break;

                default:
                    sReturn += sString.GetChar( i );
                break;
            }
        }
    }
    rString = String( sReturn, RTL_TEXTENCODING_UTF8 );
}

void XMLUtil::UnQuotHTML( String &rString ){
    //UnQuotTags( rString );
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
/*****************************************************************************/
USHORT XMLUtil::GetLangByIsoLang( const ByteString &rIsoLang )
/*****************************************************************************/
{
    return lMap[ ByteString(rIsoLang).ToLowerAscii().GetBuffer() ];
}

XMLUtil::XMLUtil(){
}


/*****************************************************************************/
void XMLUtil::dump(){
/*****************************************************************************/
    int cnt=1;
    printf("size=%d\n",lMap.size());
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
ByteString XMLUtil::GetIsoLangByIndex( USHORT nIndex )
/*****************************************************************************/
{
    if(nIndex > 0 && MAX_LANGUAGES >= nIndex )
        return isoArray[nIndex];
    return "";
}

