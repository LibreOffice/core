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

#define XML_NODE_TYPE_FILE          0x001
#define XML_NODE_TYPE_ELEMENT       0x002
#define XML_NODE_TYPE_DATA          0x003
#define XML_NODE_TYPE_COMMENT       0x004
#define XML_NODE_TYPE_DEFAULT       0x005

/** Holds data of Attributes
 */
class XMLAttribute
{
private:
    OString m_sName;
    OString m_sValue;

public:
    /// creates an attribute
    XMLAttribute(
        const OString &rName,    // attributes name
        const OString &rValue    // attributes data
    )
    : m_sName( rName ), m_sValue( rValue ) {}

    OString GetName() const { return m_sName; }
    OString GetValue() const { return m_sValue; }

    void setValue( const OString &rValue ){ m_sValue = rValue; }
};


typedef std::vector< XMLAttribute* > XMLAttributeList;

/** Virtual base to handle different kinds of XML nodes
 */
class XMLNode
{
protected:
    XMLNode(){}

public:
    virtual sal_uInt16 GetNodeType() const = 0;
    virtual ~XMLNode(){}
};


/** Virtual base to handle different kinds of child nodes
 */
class XMLChildNode : public XMLNode
{
private:
    XMLParentNode *m_pParent;

protected:
    XMLChildNode( XMLParentNode *pPar );
    XMLChildNode(): m_pParent( NULL ){};
    XMLChildNode( const XMLChildNode& rObj);
    XMLChildNode& operator=(const XMLChildNode& rObj);
public:
    /// returns the parent of this node
    XMLParentNode *GetParent() { return m_pParent; }
    virtual ~XMLChildNode(){};
};

typedef std::vector< XMLChildNode* > XMLChildNodeList;

class XMLData;

/** Virtual base to handle different kinds of parent nodes
 */

class XMLParentNode : public XMLChildNode
{
private:
    XMLChildNodeList* m_pChildList;

protected:
    XMLParentNode( XMLParentNode *pPar )
        : XMLChildNode( pPar ), m_pChildList( NULL ){}
    XMLParentNode(): m_pChildList(NULL){}

    XMLParentNode( const XMLParentNode& );

    XMLParentNode& operator=(const XMLParentNode& rObj);
    virtual ~XMLParentNode();

public:
    /// returns child list of this node
    XMLChildNodeList *GetChildList() { return m_pChildList; }

    /// adds a new child
    void AddChild(
        XMLChildNode *pChild    /// the new child
    );

    void RemoveAndDeleteAllChildren();
};

/// Mapping numeric Language code <-> XML Element
typedef boost::unordered_map<OString, XMLElement*, OStringHash> LangHashMap;

/// Mapping XML Element string identifier <-> Language Map
typedef boost::unordered_map<OString, LangHashMap*, OStringHash> XMLHashMap;

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
    XMLFile( const XMLFile& rObj ) ;
    ~XMLFile();

    void Print( XMLNode *pCur = NULL, sal_uInt16 nLevel = 0 );
    virtual void SearchL10NElements( XMLParentNode *pCur, int pos = 0 );
    void Extract( XMLFile *pCur = NULL );

    XMLHashMap* GetStrings(){ return m_pXMLStrings; }
    void Write( OString const &rFilename );
    sal_Bool Write( std::ofstream &rStream, XMLNode *pCur = NULL );

    bool CheckExportStatus( XMLParentNode *pCur = NULL );

    XMLFile& operator=(const XMLFile& rObj);

    virtual sal_uInt16 GetNodeType() const { return XML_NODE_TYPE_FILE; }

    /// returns file name
    OString GetName() const { return m_sFileName; }
    void SetName( const OString &rFilename ) { m_sFileName = rFilename; }
    const std::vector<OString>& getOrder() const { return m_vOrder; }

protected:

    void InsertL10NElement( XMLElement* pElement);

    // DATA
    OString m_sFileName;

    TagMap m_aNodes_localize;
    XMLHashMap* m_pXMLStrings;

    std::vector <OString> m_vOrder;
};

/// An Utility class for XML
class XMLUtil
{
public:
    /// Quot the XML characters
    static OString QuotHTML( const OString& rString );
};


/** Hold information of an element node
 */
class XMLElement : public XMLParentNode
{
private:
    OString m_sElementName;
    XMLAttributeList *m_pAttributes;
    OString m_sProject;
    OString m_sFilename;
    OString m_sId;
    OString m_sOldRef;
    OString m_sResourceType;
    OString m_sLanguageId;
    int m_nPos;

protected:
    void Print(XMLNode *pCur, OStringBuffer& rBuffer, bool bRootelement) const;
public:
    /// create an element node
    XMLElement(){}
    XMLElement(
        const OString &rName,    // the element name
        XMLParentNode *pParent   // parent node of this element
    );

    ~XMLElement();
    XMLElement(const XMLElement&);

    XMLElement& operator=(const XMLElement& rObj);
    virtual sal_uInt16 GetNodeType() const { return XML_NODE_TYPE_ELEMENT; }

    /// returns element name
    OString GetName() const { return m_sElementName; }

    /// returns list of attributes of this element
    XMLAttributeList *GetAttributeList() { return m_pAttributes; }

    /// adds a new attribute to this element, typically used by parser
    void AddAttribute( const OString &rAttribute, const OString &rValue );

    void ChangeLanguageTag( const OString &rValue );

    /// Return a Unicode String representation of this object
    OString ToOString();

    void SetProject         ( OString const & sPrj )        { m_sProject = sPrj; }
    void SetFileName        ( OString const & sFileName )   { m_sFilename = sFileName; }
    void SetId              ( OString const & sTheId )      { m_sId = sTheId; }
    void SetResourceType    ( OString const & sResType )    { m_sResourceType = sResType; }
    void SetLanguageId      ( OString const & sLangId )     { m_sLanguageId = sLangId; }
    void SetPos             ( int nPos )                    { m_nPos = nPos; }
    void SetOldRef          ( OString const & sOldRef )     { m_sOldRef = sOldRef; }

    virtual int GetPos()            { return m_nPos;         }
    OString GetProject() const      { return m_sProject;      }
    OString GetFileName() const     { return m_sFilename;     }
    OString GetId() const           { return m_sId;           }
    OString GetOldref() const       { return m_sOldRef;      }
    OString GetResourceType() const { return m_sResourceType; }
    OString GetLanguageId() const   { return m_sLanguageId;   }


};

/** Holds character data
 */
class XMLData : public XMLChildNode
{
private:
    OString m_sData;
    bool    m_bIsNewCreated;

public:
    /// create a data node
    XMLData(
        const OString &rData,    // the initial data
        XMLParentNode *pParent,   // the parent node of this data, typically a element node
        bool bNewCreated = false
    )
        : XMLChildNode( pParent ), m_sData( rData ), m_bIsNewCreated( bNewCreated ){}

    // Default copy constructor and copy operator work well.

    virtual sal_uInt16 GetNodeType() const { return XML_NODE_TYPE_DATA; }

    /// returns the data
    OString GetData() const { return m_sData; }

    bool isNew() const { return m_bIsNewCreated; }

    /// adds new character data to the existing one
    void AddData( const OString &rData ) { m_sData += rData; }
};

/** Holds comments
 */
class XMLComment : public XMLChildNode
{
private:
    OString m_sComment;

public:
    /// create a comment node
    XMLComment(
        const OString &rComment, // the comment
        XMLParentNode *pParent   // the parent node of this comemnt, typically a element node
    )
        : XMLChildNode( pParent ), m_sComment( rComment ) {}

    // Default copy constructor and copy operator work well.

    virtual sal_uInt16 GetNodeType() const { return XML_NODE_TYPE_COMMENT; }

    /// returns the comment
    OString GetComment() const { return m_sComment; }
};

/** Holds additional file content like those for which no handler exists
 */
class XMLDefault : public XMLChildNode
{
private:
    OString m_sDefault;

public:
    /// create a comment node
    XMLDefault(
        const OString &rDefault, // the comment
        XMLParentNode *pParent   // the parent node of this comemnt, typically a element node
    )
        : XMLChildNode( pParent ), m_sDefault( rDefault ) {}

    // Default copy constructor and copy operator work well.

    virtual sal_uInt16 GetNodeType() const { return XML_NODE_TYPE_DEFAULT; }

    /// returns the comment
    OString GetDefault() const { return m_sDefault; }
};

/** struct for error information, used by class SimpleXMLParser
 */
struct XMLError {
    XML_Error m_eCode;    ///< the error code
    std::size_t m_nLine; ///< error line number
    std::size_t m_nColumn; ///< error column number
    OString m_sMessage;    ///< readable error message
};

/** validating xml parser, creates a document tree with xml nodes
 */

class SimpleXMLParser
{
private:
    XML_Parser m_aParser;
    XMLError m_aErrorInformation;

    XMLFile *m_pXMLFile;
    XMLParentNode *m_pCurNode;
    XMLData *m_pCurData;


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
    const XMLError &GetError() const { return m_aErrorInformation; }
};

#endif // BOOTSTRP_XMLPARSE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
