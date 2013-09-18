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

#include <libxml/xmlexports.h> // define XMLCALL so expat.h does not redefine it
#include <expat.h>

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
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
    OString sName;
    OString sValue;

public:
    /// creates an attribute
    XMLAttribute(
        const OString &rName,    // attributes name
        const OString &rValue    // attributes data
    )
    : sName( rName ), sValue( rValue ) {}

    OString GetName() const { return sName; }
    OString GetValue() const { return sValue; }

    void setValue(const OString &rValue){sValue=rValue;}

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

class XMLData;

/** Virtual base to handle different kinds of parent nodes
 */

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
typedef boost::unordered_map<OString, XMLElement*, OStringHash> LangHashMap;

/// Mapping XML Element string identifier <-> Language Map
typedef boost::unordered_map<OString, LangHashMap*, OStringHash> XMLHashMap;

/// Mapping iso alpha string code <-> iso numeric code
typedef boost::unordered_map<OString, int, OStringHash> HashMap;

/// Mapping XML tag names <-> have localizable strings
typedef boost::unordered_map<OString, sal_Bool, OStringHash> TagMap;

/** Holds information of a XML file, is root node of tree
 */

class XMLFile : public XMLParentNode
{
public:
    XMLFile(
        const OString &rFileName // the file name, empty if created from memory stream
    );
    XMLFile( const XMLFile& obj ) ;
    ~XMLFile();

    void        Print( XMLNode *pCur = NULL, sal_uInt16 nLevel = 0 );
    virtual void SearchL10NElements( XMLParentNode *pCur, int pos = 0 );
    void        Extract( XMLFile *pCur = NULL );

    XMLHashMap* GetStrings(){return XMLStrings;}
    void Write( OString const &rFilename );
    sal_Bool        Write( ofstream &rStream , XMLNode *pCur = NULL );

    bool        CheckExportStatus( XMLParentNode *pCur = NULL );// , int pos = 0 );

    XMLFile&    operator=(const XMLFile& obj);

    virtual sal_uInt16  GetNodeType();

    /// returns file name
    OString GetName() const { return sFileName; }
    void SetName( const OString &rFilename ) { sFileName = rFilename; }
    const std::vector<OString>& getOrder() const { return order; }

protected:

    void        InsertL10NElement( XMLElement* pElement);

    // DATA
    OString      sFileName;

    TagMap      nodes_localize;
    XMLHashMap* XMLStrings;

    std::vector <OString> order;
};

/// An Utility class for XML
class XMLUtil{

public:
    /// Quot the XML characters
    static OString QuotHTML( const OString& rString );
};


//-------------------------------------------------------------------------

/** Hold information of an element node
 */
class XMLElement : public XMLParentNode
{
private:
    OString sElementName;
    XMLAttributeList *pAttributes;
    OString   project,
                 filename,
                 id,
                 sOldRef,
                 resourceType,
                 languageId;
    int          nPos;

protected:
    void Print(XMLNode *pCur, OStringBuffer& buffer , bool rootelement);
public:
    /// create an element node
    XMLElement(){}
    XMLElement(
        const OString &rName,    // the element name
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
    OString GetName() const { return sElementName; }

    /// returns list of attributes of this element
    XMLAttributeList *GetAttributeList() { return pAttributes; }

    /// adds a new attribute to this element, typically used by parser
    void AddAttribute( const OString &rAttribute, const OString &rValue );

    void ChangeLanguageTag( const OString &rValue );

    /// Return a Unicode String representation of this object
    OString ToOString();

    void SetProject         ( OString const & prj        ){ project = prj;        }
    void SetFileName        ( OString const & fn         ){ filename = fn;        }
    void SetId              ( OString const & theId      ){ id = theId;           }
    void SetResourceType    ( OString const & rt         ){ resourceType = rt;    }
    void SetLanguageId      ( OString const & lid        ){ languageId = lid;     }
    void SetPos             ( int nPos_in           ){ nPos = nPos_in;       }
    void SetOldRef          ( OString const & sOldRef_in ){ sOldRef = sOldRef_in; }

    virtual int        GetPos()     { return nPos;         }
    OString GetProject() const      { return project;      }
    OString GetFileName() const     { return filename;     }
    OString GetId() const           { return id;           }
    OString GetOldref() const       { return sOldRef;      }
    OString GetResourceType() const { return resourceType; }
    OString GetLanguageId() const   { return languageId;   }


};
//-------------------------------------------------------------------------


/** Holds character data
 */
class XMLData : public XMLChildNode
{
private:
    OString sData;
    bool   isNewCreated;

public:
    /// create a data node
    XMLData(
        const OString &rData,    // the initial data
        XMLParentNode *Parent   // the parent node of this data, typically a element node
    )
                : XMLChildNode( Parent ), sData( rData ) , isNewCreated ( false ){}
    XMLData(
        const OString &rData,    // the initial data
        XMLParentNode *Parent,  // the parent node of this data, typically a element node
        bool newCreated
    )
                : XMLChildNode( Parent ), sData( rData ) , isNewCreated ( newCreated ){}

    XMLData(const XMLData& obj);

    XMLData& operator=(const XMLData& obj);
    virtual sal_uInt16 GetNodeType();

    /// returns the data
    OString GetData() const { return sData; }

    bool isNew() { return isNewCreated; }
    /// adds new character data to the existing one
    void AddData(
        const OString &rData // the new data
    );



};

//-------------------------------------------------------------------------

/** Holds comments
 */
class XMLComment : public XMLChildNode
{
private:
    OString sComment;

public:
    /// create a comment node
    XMLComment(
        const OString &rComment, // the comment
        XMLParentNode *Parent   // the parent node of this comemnt, typically a element node
    )
                : XMLChildNode( Parent ), sComment( rComment ) {}

    virtual sal_uInt16 GetNodeType();

    XMLComment( const XMLComment& obj );

    XMLComment& operator=(const XMLComment& obj);

    /// returns the comment
    OString GetComment() const  { return sComment; }
};

//-------------------------------------------------------------------------

/** Holds additional file content like those for which no handler exists
 */
class XMLDefault : public XMLChildNode
{
private:
    OString sDefault;

public:
    /// create a comment node
    XMLDefault(
        const OString &rDefault, // the comment
        XMLParentNode *Parent   // the parent node of this comemnt, typically a element node
    )
                : XMLChildNode( Parent ), sDefault( rDefault ) {}

    XMLDefault(const XMLDefault& obj);

    XMLDefault& operator=(const XMLDefault& obj);

    /// returns node type XML_NODE_TYPE_COMMENT
    virtual sal_uInt16 GetNodeType();

    /// returns the comment
    OString GetDefault() const  { return sDefault; }
};

//-------------------------------------------------------------------------

/** struct for error information, used by class SimpleXMLParser
 */
struct XMLError {
    XML_Error eCode;    ///< the error code
    std::size_t nLine; ///< error line number
    std::size_t nColumn; ///< error column number
    OString sMessage;    ///< readable error message
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
        const OString &rFileName,    // the file name
        XMLFile *pXMLFileIn         // the XMLFile
    );

    /// returns an error struct
    const XMLError &GetError() { return aErrorInformation; }
};

#endif // BOOTSTRP_XMLPARSE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
