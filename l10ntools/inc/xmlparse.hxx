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

#ifndef BOOTSTRP_XMLPARSE_HXX
#define BOOTSTRP_XMLPARSE_HXX

#include <signal.h>
#include <expat.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "tools/string.hxx"
#include "tools/list.hxx"
#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include "tools/stream.hxx"
#include "tools/isofallback.hxx"
#include "export.hxx"
#include "xmlutil.hxx"

#include <fstream>
#include <iostream>

class XMLParentNode;
class XMLElement;


using namespace ::rtl;
using namespace std;

#include <boost/unordered_map.hpp>
#include <deque>    /* std::deque*/
#include <iterator> /* std::iterator*/
#include <list>     /* std::list*/
#include <vector>   /* std::vector*/
#define XML_NODE_TYPE_FILE          0x001
#define XML_NODE_TYPE_ELEMENT       0x002
#define XML_NODE_TYPE_DATA          0x003
#define XML_NODE_TYPE_COMMENT       0x004
#define XML_NODE_TYPE_DEFAULT       0x005
#define MAX_LANGUAGES               99


//-------------------------------------------------------------------------

/** Holds data of Attributes
 */
class XMLAttribute : public String
{
private:
    String sValue;

public:
    /// creates an attribute
    XMLAttribute(
        const String &rName,    // attributes name
        const String &rValue    // attributes data
    )
                : String( rName ), sValue( rValue ) {}

    /// getting value of an attribue
    const String &GetValue() { return sValue; }

    void setValue(const String &rValue){sValue=rValue;}

    /// returns true if two attributes are equal and have the same value
    BOOL IsEqual(
        const XMLAttribute &rAttribute  // the attribute which has to be equal
    )
    {
        return (( rAttribute == *this ) && ( rAttribute.sValue == sValue ));
    }
};

typedef ::std::vector< XMLAttribute* > XMLAttributeList;

//-------------------------------------------------------------------------

/** Virtual base to handle different kinds of XML nodes
 */
class XMLNode
{
protected:
    XMLNode() {}

public:
    virtual USHORT GetNodeType() = 0;
    virtual ~XMLNode() {}
};

//-------------------------------------------------------------------------

/** Virtual base to handle different kinds of child nodes
 */
class XMLChildNode : public XMLNode
{
private:
    XMLParentNode *pParent;

protected:
    XMLChildNode( XMLParentNode *pPar );
    XMLChildNode():pParent( NULL ){};
    XMLChildNode( const XMLChildNode& obj);
    XMLChildNode& operator=(const XMLChildNode& obj);
public:
    virtual USHORT GetNodeType() = 0;

    /// returns the parent of this node
    XMLParentNode *GetParent() { return pParent; }
    virtual ~XMLChildNode(){};
};

typedef ::std::vector< XMLChildNode* > XMLChildNodeList;

//-------------------------------------------------------------------------

/** Virtual base to handle different kinds of parent nodes
 */
class XMLData;

class XMLParentNode : public XMLChildNode
{
private:
    XMLChildNodeList* pChildList;
    static int dbgcnt;

protected:
    XMLParentNode( XMLParentNode *pPar )
                : XMLChildNode( pPar ), pChildList( NULL )
              {
              }
    XMLParentNode(): pChildList(NULL){
    }
    /// Copyconstructor
    XMLParentNode( const XMLParentNode& );

    XMLParentNode& operator=(const XMLParentNode& obj);
    virtual ~XMLParentNode();


public:
    virtual USHORT GetNodeType() = 0;

    /// returns child list of this node
    XMLChildNodeList *GetChildList() { return pChildList; }

    /// adds a new child
    void AddChild(
        XMLChildNode *pChild    /// the new child
    );

    void AddChild(
        XMLChildNode *pChild , size_t pos   /// the new child
    );

    virtual int GetPosition( ByteString id );
    size_t RemoveChild( XMLElement *pRefElement );
    void RemoveAndDeleteAllChilds();

    /// returns a child element which matches the given one
    XMLElement *GetChildElement(
        XMLElement *pRefElement // the reference elelement
    );
};

//-------------------------------------------------------------------------

/// Mapping numeric Language code <-> XML Element
typedef boost::unordered_map< ByteString ,XMLElement* , hashByteString,equalByteString > LangHashMap;

/// Mapping XML Element string identifier <-> Language Map
typedef boost::unordered_map<ByteString , LangHashMap* ,
                      hashByteString,equalByteString>                   XMLHashMap;

/// Mapping iso alpha string code <-> iso numeric code
typedef boost::unordered_map<ByteString, int, hashByteString,equalByteString>   HashMap;

/// Mapping XML tag names <-> have localizable strings
typedef boost::unordered_map<ByteString , BOOL ,
                      hashByteString,equalByteString>                   TagMap;

/** Holds information of a XML file, is root node of tree
 */


class XMLFile : public XMLParentNode
{
public:
    XMLFile() ;
    XMLFile(
                const String &rFileName // the file name, empty if created from memory stream
    );
    XMLFile( const XMLFile& obj ) ;
    ~XMLFile();

    ByteString* GetGroupID(std::deque<ByteString> &groupid);
    void        Print( XMLNode *pCur = NULL, USHORT nLevel = 0 );
    virtual void SearchL10NElements( XMLParentNode *pCur, int pos = 0 );
    void        Extract( XMLFile *pCur = NULL );
    void        View();
//  void static Signal_handler(int signo);//void*,oslSignalInfo * pInfo);
    void        showType(XMLParentNode* node);

    XMLHashMap* GetStrings(){return XMLStrings;}
    BOOL        Write( ByteString &rFilename );
    BOOL        Write( ofstream &rStream , XMLNode *pCur = NULL );

    bool        CheckExportStatus( XMLParentNode *pCur = NULL );// , int pos = 0 );

    XMLFile&    operator=(const XMLFile& obj);

    virtual USHORT  GetNodeType();

    /// returns file name
    const String &GetName() { return sFileName; }
    void          SetName( const String &rFilename ) { sFileName = rFilename; }
    void          SetFullName( const String &rFullFilename ) { sFullName = rFullFilename; }
    const std::vector<ByteString> getOrder(){ return order; }

protected:
    // writes a string as UTF8 with dos line ends to a given stream
    void        WriteString( ofstream &rStream, const String &sString );

    // quotes the given text for writing to a file
    void        QuotHTML( String &rString );

    void        InsertL10NElement( XMLElement* pElement);

    // DATA
    String      sFileName;
    String      sFullName;

    const ByteString ID,OLDREF,XML_LANG;

    TagMap      nodes_localize;
    XMLHashMap* XMLStrings;

    std::vector <ByteString> order;
};

/// An Utility class for XML
/// See RFC 3066 / #i8252# for ISO codes
class XMLUtil{

public:
    /// Quot the XML characters and replace \n \t
    static void         QuotHTML( String &rString );

    /// UnQuot the XML characters and restore \n \t
    static void         UnQuotHTML  ( String &rString );

    /// Return the numeric iso language code
    //USHORT                GetLangByIsoLang( const ByteString &rIsoLang );

    /// Return the alpha strings representation
    ByteString          GetIsoLangByIndex( USHORT nIndex );

    static XMLUtil&     Instance();
    ~XMLUtil();

    void         dump();

private:
    /// Mapping iso alpha string code <-> iso numeric code
    HashMap      lMap;

    /// Mapping iso numeric code      <-> iso alpha string code
    ByteString   isoArray[MAX_LANGUAGES];

    static void UnQuotData( String &rString );
    static void UnQuotTags( String &rString );

    XMLUtil();
    XMLUtil(const XMLUtil&);

};



//-------------------------------------------------------------------------

/** Hold information of an element node
 */
class XMLElement : public XMLParentNode
{
private:
    String sElementName;
    XMLAttributeList *pAttributes;
    ByteString   project,
                 filename,
                 id,
                 sOldRef,
                 resourceType,
                 languageId;
    int          nPos;

protected:
    void Print(XMLNode *pCur, OUStringBuffer& buffer , bool rootelement);
public:
    /// create a element node
    XMLElement(){}
    XMLElement(
        const String &rName,    // the element name
        XMLParentNode *Parent   // parent node of this element
    ):          XMLParentNode( Parent ),
                sElementName( rName ),
                pAttributes( NULL ),
                project(""),
                filename(""),
                id(""),
                sOldRef(""),
                resourceType(""),
                languageId(""),
                nPos(0)
                   {
                }
    ~XMLElement();
    XMLElement(const XMLElement&);

    XMLElement& operator=(const XMLElement& obj);
    /// returns node type XML_NODE_ELEMENT
    virtual USHORT GetNodeType();

    /// returns element name
    const String &GetName() { return sElementName; }

    /// returns list of attributes of this element
    XMLAttributeList *GetAttributeList() { return pAttributes; }

    /// adds a new attribute to this element, typically used by parser
    void AddAttribute( const String &rAttribute, const String &rValue );

    void ChangeLanguageTag( const String &rValue );
    // Return a ASCII String representation of this object
    OString ToOString();

    // Return a Unicode String representation of this object
    OUString ToOUString();

    bool    Equals(OUString refStr);

    /// returns a attribute
    XMLAttribute *GetAttribute(
        const String &rName // the attribute name
    );
    void SetProject         ( ByteString prj        ){ project = prj;        }
    void SetFileName        ( ByteString fn         ){ filename = fn;        }
    void SetId              ( ByteString theId      ){ id = theId;           }
    void SetResourceType    ( ByteString rt         ){ resourceType = rt;    }
    void SetLanguageId      ( ByteString lid        ){ languageId = lid;     }
    void SetPos             ( int nPos_in           ){ nPos = nPos_in;       }
    void SetOldRef          ( ByteString sOldRef_in ){ sOldRef = sOldRef_in; }

    virtual int        GetPos()         { return nPos;         }
    ByteString GetProject()     { return project;      }
    ByteString GetFileName()    { return filename;     }
    ByteString GetId()          { return id;           }
    ByteString GetOldref()      { return sOldRef;      }
    ByteString GetResourceType(){ return resourceType; }
    ByteString GetLanguageId()  { return languageId;   }


};
//-------------------------------------------------------------------------


/** Holds character data
 */
class XMLData : public XMLChildNode
{
private:
    String sData;
    bool   isNewCreated;

public:
    /// create a data node
    XMLData(
        const String &rData,    // the initial data
        XMLParentNode *Parent   // the parent node of this data, typically a element node
    )
                : XMLChildNode( Parent ), sData( rData ) , isNewCreated ( false ){}
    XMLData(
        const String &rData,    // the initial data
        XMLParentNode *Parent,  // the parent node of this data, typically a element node
        bool newCreated
    )
                : XMLChildNode( Parent ), sData( rData ) , isNewCreated ( newCreated ){}

    XMLData(const XMLData& obj);

    XMLData& operator=(const XMLData& obj);
    virtual USHORT GetNodeType();

    /// returns the data
    const String &GetData() { return sData; }

    bool isNew() { return isNewCreated; }
    /// adds new character data to the existing one
    void AddData(
        const String &rData // the new data
    );



};

//-------------------------------------------------------------------------

/** Holds comments
 */
class XMLComment : public XMLChildNode
{
private:
    String sComment;

public:
    /// create a comment node
    XMLComment(
        const String &rComment, // the comment
        XMLParentNode *Parent   // the parent node of this comemnt, typically a element node
    )
                : XMLChildNode( Parent ), sComment( rComment ) {}

    virtual USHORT GetNodeType();

    XMLComment( const XMLComment& obj );

    XMLComment& operator=(const XMLComment& obj);

    /// returns the comment
    const String &GetComment()  { return sComment; }
};

//-------------------------------------------------------------------------

/** Holds additional file content like those for which no handler exists
 */
class XMLDefault : public XMLChildNode
{
private:
    String sDefault;

public:
    /// create a comment node
    XMLDefault(
        const String &rDefault, // the comment
        XMLParentNode *Parent   // the parent node of this comemnt, typically a element node
    )
                : XMLChildNode( Parent ), sDefault( rDefault ) {}

    XMLDefault(const XMLDefault& obj);

    XMLDefault& operator=(const XMLDefault& obj);

    /// returns node type XML_NODE_TYPE_COMMENT
    virtual USHORT GetNodeType();

    /// returns the comment
    const String &GetDefault()  { return sDefault; }
};

//-------------------------------------------------------------------------

/** struct for error information, used by class SimpleXMLParser
 */
struct XMLError {
    XML_Error eCode;    // the error code
    ULONG nLine;        // error line number
    ULONG nColumn;      // error column number
    String sMessage;    // readable error message
};

//-------------------------------------------------------------------------

/** validating xml parser, creates a document tree with xml nodes
 */


class SimpleXMLParser
{
private:
    XML_Parser aParser;
    XMLError aErrorInformation;

    XMLFile *pXMLFile;
    XMLParentNode *pCurNode;
    XMLData *pCurData;


    static void StartElementHandler( void *userData, const XML_Char *name, const XML_Char **atts );
    static void EndElementHandler( void *userData, const XML_Char *name );
    static void CharacterDataHandler( void *userData, const XML_Char *s, int len );
    static void CommentHandler( void *userData, const XML_Char *data );
    static void DefaultHandler( void *userData, const XML_Char *s, int len );


    void StartElement( const XML_Char *name, const XML_Char **atts );
    void EndElement( const XML_Char *name );
    void CharacterData( const XML_Char *s, int len );
    void Comment( const XML_Char *data );
    void Default( const XML_Char *s, int len );


public:
    /// creates a new parser
    SimpleXMLParser();
    ~SimpleXMLParser();

    /// parse a file, returns NULL on criticall errors
    XMLFile *Execute(
        const String &rFullFileName,
        const String &rFileName,    // the file name
        XMLFile *pXMLFileIn         // the XMLFile
    );

    /// parse a memory stream, returns NULL on criticall errors
    XMLFile *Execute(
        SvMemoryStream *pStream // the stream
    );

    /// returns an error struct
    const XMLError &GetError() { return aErrorInformation; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
