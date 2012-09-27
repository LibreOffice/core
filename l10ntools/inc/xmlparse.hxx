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

#ifndef BOOTSTRP_XMLPARSE_HXX
#define BOOTSTRP_XMLPARSE_HXX

#include "sal/config.h"

#include <cstddef>
#include <vector>

#include <signal.h>
#include <expat.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "boost/unordered_map.hpp"
#include "export.hxx"

class XMLParentNode;
class XMLElement;


using namespace ::rtl;
using namespace std;

#define XML_NODE_TYPE_FILE          0x001
#define XML_NODE_TYPE_ELEMENT       0x002
#define XML_NODE_TYPE_DATA          0x003
#define XML_NODE_TYPE_COMMENT       0x004
#define XML_NODE_TYPE_DEFAULT       0x005


//-------------------------------------------------------------------------

/** Holds data of Attributes
 */
class XMLAttribute
{
private:
    rtl::OUString sName;
    rtl::OUString sValue;

public:
    /// creates an attribute
    XMLAttribute(
        const rtl::OUString &rName,    // attributes name
        const rtl::OUString &rValue    // attributes data
    )
                : sName( rName ), sValue( rValue ) {}

    rtl::OUString GetName() const { return sName; }
    rtl::OUString GetValue() const { return sValue; }

    void setValue(const rtl::OUString &rValue){sValue=rValue;}

    /// returns true if two attributes are equal and have the same value
    sal_Bool IsEqual(
        const XMLAttribute &rAttribute  // the attribute which has to be equal
    )
    {
        return (( rAttribute.sName == sName ) && ( rAttribute.sValue == sValue ));
    }
};

typedef std::vector< XMLAttribute* > XMLAttributeList;

//-------------------------------------------------------------------------

/** Virtual base to handle different kinds of XML nodes
 */
class XMLNode
{
protected:
    XMLNode() {}

public:
    virtual sal_uInt16 GetNodeType() = 0;
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
    virtual sal_uInt16 GetNodeType() = 0;

    /// returns the parent of this node
    XMLParentNode *GetParent() { return pParent; }
    virtual ~XMLChildNode(){};
};

typedef std::vector< XMLChildNode* > XMLChildNodeList;

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
    virtual sal_uInt16 GetNodeType() = 0;

    /// returns child list of this node
    XMLChildNodeList *GetChildList() { return pChildList; }

    /// adds a new child
    void AddChild(
        XMLChildNode *pChild    /// the new child
    );

    void RemoveAndDeleteAllChildren();
};

//-------------------------------------------------------------------------

/// Mapping numeric Language code <-> XML Element
typedef boost::unordered_map<rtl::OString, XMLElement*, rtl::OStringHash> LangHashMap;

/// Mapping XML Element string identifier <-> Language Map
typedef boost::unordered_map<rtl::OString, LangHashMap*, rtl::OStringHash> XMLHashMap;

/// Mapping iso alpha string code <-> iso numeric code
typedef boost::unordered_map<rtl::OString, int, rtl::OStringHash> HashMap;

/// Mapping XML tag names <-> have localizable strings
typedef boost::unordered_map<rtl::OString, sal_Bool, rtl::OStringHash> TagMap;

/** Holds information of a XML file, is root node of tree
 */


class XMLFile : public XMLParentNode
{
public:
    XMLFile(
        const rtl::OUString &rFileName // the file name, empty if created from memory stream
    );
    XMLFile( const XMLFile& obj ) ;
    ~XMLFile();

    void        Print( XMLNode *pCur = NULL, sal_uInt16 nLevel = 0 );
    virtual void SearchL10NElements( XMLParentNode *pCur, int pos = 0 );
    void        Extract( XMLFile *pCur = NULL );

    XMLHashMap* GetStrings(){return XMLStrings;}
    void Write( rtl::OString const &rFilename );
    sal_Bool        Write( ofstream &rStream , XMLNode *pCur = NULL );

    bool        CheckExportStatus( XMLParentNode *pCur = NULL );// , int pos = 0 );

    XMLFile&    operator=(const XMLFile& obj);

    virtual sal_uInt16  GetNodeType();

    /// returns file name
    rtl::OUString GetName() { return sFileName; }
    void          SetName( const rtl::OUString &rFilename ) { sFileName = rFilename; }
    const std::vector<rtl::OString> getOrder(){ return order; }

protected:
    // writes a string as UTF8 with dos line ends to a given stream
    void        WriteString( ofstream &rStream, const rtl::OUString &sString );

    void        InsertL10NElement( XMLElement* pElement);

    // DATA
    rtl::OUString      sFileName;

    const rtl::OString ID, OLDREF, XML_LANG;

    TagMap      nodes_localize;
    XMLHashMap* XMLStrings;

    std::vector <rtl::OString> order;
};

/// An Utility class for XML
/// See RFC 3066 / #i8252# for ISO codes
class XMLUtil{

public:
    /// Quot the XML characters and replace \n \t
    static void         QuotHTML( rtl::OUString &rString );

    /// UnQuot the XML characters and restore \n \t
    static void         UnQuotHTML  ( rtl::OUString &rString );
};



//-------------------------------------------------------------------------

/** Hold information of an element node
 */
class XMLElement : public XMLParentNode
{
private:
    rtl::OUString sElementName;
    XMLAttributeList *pAttributes;
    rtl::OString   project,
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
        const rtl::OUString &rName,    // the element name
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
    virtual sal_uInt16 GetNodeType();

    /// returns element name
    rtl::OUString GetName() { return sElementName; }

    /// returns list of attributes of this element
    XMLAttributeList *GetAttributeList() { return pAttributes; }

    /// adds a new attribute to this element, typically used by parser
    void AddAttribute( const rtl::OUString &rAttribute, const rtl::OUString &rValue );

    void ChangeLanguageTag( const rtl::OUString &rValue );

    // Return a Unicode String representation of this object
    OUString ToOUString();

    void SetProject         ( rtl::OString const & prj        ){ project = prj;        }
    void SetFileName        ( rtl::OString const & fn         ){ filename = fn;        }
    void SetId              ( rtl::OString const & theId      ){ id = theId;           }
    void SetResourceType    ( rtl::OString const & rt         ){ resourceType = rt;    }
    void SetLanguageId      ( rtl::OString const & lid        ){ languageId = lid;     }
    void SetPos             ( int nPos_in           ){ nPos = nPos_in;       }
    void SetOldRef          ( rtl::OString const & sOldRef_in ){ sOldRef = sOldRef_in; }

    virtual int        GetPos()         { return nPos;         }
    rtl::OString GetProject()     { return project;      }
    rtl::OString GetFileName()    { return filename;     }
    rtl::OString GetId()          { return id;           }
    rtl::OString GetOldref()      { return sOldRef;      }
    rtl::OString GetResourceType(){ return resourceType; }
    rtl::OString GetLanguageId()  { return languageId;   }


};
//-------------------------------------------------------------------------


/** Holds character data
 */
class XMLData : public XMLChildNode
{
private:
    rtl::OUString sData;
    bool   isNewCreated;

public:
    /// create a data node
    XMLData(
        const rtl::OUString &rData,    // the initial data
        XMLParentNode *Parent   // the parent node of this data, typically a element node
    )
                : XMLChildNode( Parent ), sData( rData ) , isNewCreated ( false ){}
    XMLData(
        const rtl::OUString &rData,    // the initial data
        XMLParentNode *Parent,  // the parent node of this data, typically a element node
        bool newCreated
    )
                : XMLChildNode( Parent ), sData( rData ) , isNewCreated ( newCreated ){}

    XMLData(const XMLData& obj);

    XMLData& operator=(const XMLData& obj);
    virtual sal_uInt16 GetNodeType();

    /// returns the data
    rtl::OUString GetData() { return sData; }

    bool isNew() { return isNewCreated; }
    /// adds new character data to the existing one
    void AddData(
        const rtl::OUString &rData // the new data
    );



};

//-------------------------------------------------------------------------

/** Holds comments
 */
class XMLComment : public XMLChildNode
{
private:
    rtl::OUString sComment;

public:
    /// create a comment node
    XMLComment(
        const rtl::OUString &rComment, // the comment
        XMLParentNode *Parent   // the parent node of this comemnt, typically a element node
    )
                : XMLChildNode( Parent ), sComment( rComment ) {}

    virtual sal_uInt16 GetNodeType();

    XMLComment( const XMLComment& obj );

    XMLComment& operator=(const XMLComment& obj);

    /// returns the comment
    rtl::OUString GetComment()  { return sComment; }
};

//-------------------------------------------------------------------------

/** Holds additional file content like those for which no handler exists
 */
class XMLDefault : public XMLChildNode
{
private:
    rtl::OUString sDefault;

public:
    /// create a comment node
    XMLDefault(
        const rtl::OUString &rDefault, // the comment
        XMLParentNode *Parent   // the parent node of this comemnt, typically a element node
    )
                : XMLChildNode( Parent ), sDefault( rDefault ) {}

    XMLDefault(const XMLDefault& obj);

    XMLDefault& operator=(const XMLDefault& obj);

    /// returns node type XML_NODE_TYPE_COMMENT
    virtual sal_uInt16 GetNodeType();

    /// returns the comment
    rtl::OUString GetDefault()  { return sDefault; }
};

//-------------------------------------------------------------------------

/** struct for error information, used by class SimpleXMLParser
 */
struct XMLError {
    XML_Error eCode;    // the error code
    std::size_t nLine; // error line number
    std::size_t nColumn; // error column number
    rtl::OUString sMessage;    // readable error message
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
        const rtl::OUString &rFileName,    // the file name
        XMLFile *pXMLFileIn         // the XMLFile
    );

    /// returns an error struct
    const XMLError &GetError() { return aErrorInformation; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
