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

#ifndef INCLUDED_L10NTOOLS_INC_XMLPARSE_HXX
#define INCLUDED_L10NTOOLS_INC_XMLPARSE_HXX

#include <sal/config.h>

#include <cstddef>
#include <memory>
#include <vector>

#include <signal.h>

#include <libxml/xmlexports.h>
#include <expat.h>

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include "export.hxx"
#include <unordered_map>

class XMLParentNode;
class XMLElement;

enum class XMLNodeType{
    XFILE   = 0x001,
    ELEMENT = 0x002,
    DATA    = 0x003,
    COMMENT = 0x004,
    DEFAULT = 0x005
};

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

    const OString& GetName() const { return m_sName; }
    const OString& GetValue() const { return m_sValue; }

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
    virtual XMLNodeType GetNodeType() const = 0;
    virtual ~XMLNode(){}

    XMLNode(XMLNode const &) = default;
    XMLNode(XMLNode &&) = default;
    XMLNode & operator =(XMLNode const &) = default;
    XMLNode & operator =(XMLNode &&) = default;
};


/** Virtual base to handle different kinds of child nodes
 */
class XMLChildNode : public XMLNode
{
private:
    XMLParentNode *m_pParent;

protected:
    XMLChildNode( XMLParentNode *pPar );
    XMLChildNode( const XMLChildNode& rObj);
    XMLChildNode& operator=(const XMLChildNode& rObj);
public:
    /// returns the parent of this node
    XMLParentNode *GetParent() { return m_pParent; }
};

typedef std::vector< XMLChildNode* > XMLChildNodeList;

class XMLData;

/** Virtual base to handle different kinds of parent nodes
 */

class XMLParentNode : public XMLChildNode
{
private:
    std::unique_ptr<XMLChildNodeList> m_pChildList;

protected:
    XMLParentNode( XMLParentNode *pPar )
        : XMLChildNode( pPar ) {}

    XMLParentNode( const XMLParentNode& );

    XMLParentNode& operator=(const XMLParentNode& rObj);
    virtual ~XMLParentNode() override;

public:
    /// returns child list of this node
    XMLChildNodeList *GetChildList() { return m_pChildList.get(); }

    /// adds a new child
    void AddChild(
        XMLChildNode *pChild    /// the new child
    );

    void RemoveAndDeleteAllChildren();
};

/// Mapping numeric Language code <-> XML Element
typedef std::unordered_map<OString, XMLElement*> LangHashMap;

/// Mapping XML Element string identifier <-> Language Map
typedef std::unordered_map<OString, LangHashMap*> XMLHashMap;

/** Holds information of a XML file, is root node of tree
 */
class XMLFile final : public XMLParentNode
{
public:
    XMLFile(
        const OString &rFileName // the file name, empty if created from memory stream
    );
    XMLFile( const XMLFile& rObj ) ;
    virtual ~XMLFile() override;

    void Print( XMLNode *pCur, sal_uInt16 nLevel = 0 );
    void SearchL10NElements( XMLChildNode *pCur );
    void Extract();

    XMLHashMap* GetStrings(){ return m_pXMLStrings.get(); }
    void Write( OString const &rFilename );
    void Write( std::ofstream &rStream, XMLNode *pCur = nullptr );

    bool CheckExportStatus( XMLParentNode *pCur = nullptr );

    XMLFile& operator=(const XMLFile& rObj);

    virtual XMLNodeType GetNodeType() const override { return XMLNodeType::XFILE; }

    /// returns file name
    const OString& GetName() const { return m_sFileName; }
    void SetName( const OString &rFilename ) { m_sFileName = rFilename; }
    const std::vector<OString>& getOrder() const { return m_vOrder; }

private:

    void InsertL10NElement( XMLElement* pElement);

    // DATA
    OString m_sFileName;

    /// Mapping XML tag names <-> have localizable strings
    std::unordered_map<OString, bool> m_aNodes_localize;

    std::unique_ptr<XMLHashMap> m_pXMLStrings;

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
    std::unique_ptr<XMLAttributeList> m_pAttributes;

protected:
    void Print(XMLNode *pCur, OStringBuffer& rBuffer, bool bRootelement) const;
public:
    /// create an element node
    XMLElement(
        const OString &rName,    // the element name
        XMLParentNode *pParent   // parent node of this element
    );

    virtual ~XMLElement() override;
    XMLElement(const XMLElement&);

    XMLElement& operator=(const XMLElement& rObj);
    virtual XMLNodeType GetNodeType() const override { return XMLNodeType::ELEMENT; }

    /// returns element name
    const OString& GetName() const { return m_sElementName; }

    /// returns list of attributes of this element
    XMLAttributeList *GetAttributeList() { return m_pAttributes.get(); }

    /// adds a new attribute to this element, typically used by parser
    void AddAttribute( const OString &rAttribute, const OString &rValue );

    void ChangeLanguageTag( const OString &rValue );

    /// Return a Unicode String representation of this object
    OString ToOString();
};

/** Holds character data
 */
class XMLData : public XMLChildNode
{
private:
    OString m_sData;

public:
    /// create a data node
    XMLData(
        const OString &rData,    // the initial data
        XMLParentNode *pParent   // the parent node of this data, typically a element node
    )
        : XMLChildNode( pParent ), m_sData( rData ) {}

    // Default copy constructor and copy operator work well.

    virtual XMLNodeType GetNodeType() const override { return XMLNodeType::DATA; }

    /// returns the data
    const OString& GetData() const { return m_sData; }

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

    virtual XMLNodeType GetNodeType() const override { return XMLNodeType::COMMENT; }

    /// returns the comment
    const OString& GetComment() const { return m_sComment; }
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

    virtual XMLNodeType GetNodeType() const override { return XMLNodeType::DEFAULT; }

    /// returns the comment
    const OString& GetDefault() const { return m_sDefault; }
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

    XMLParentNode *m_pCurNode;
    XMLData *m_pCurData;


    static void StartElementHandler( void *userData, const XML_Char *name, const XML_Char **atts );
    static void EndElementHandler( void *userData, const XML_Char *name );
    static void CharacterDataHandler( void *userData, const XML_Char *s, int len );
    static void CommentHandler( void *userData, const XML_Char *data );
    static void DefaultHandler( void *userData, const XML_Char *s, int len );


    void StartElement( const XML_Char *name, const XML_Char **atts );
    void EndElement();
    void CharacterData( const XML_Char *s, int len );
    void Comment( const XML_Char *data );
    void Default( const XML_Char *s, int len );

public:
    /// creates a new parser
    SimpleXMLParser();
    ~SimpleXMLParser();

    /// parse a file, return false on critical errors
    bool Execute(
        const OString &rFileName,    // the file name
        XMLFile* pXMLFile  // the XMLFile
    );

    /// returns an error struct
    const XMLError &GetError() const { return m_aErrorInformation; }
};

#endif // INCLUDED_L10NTOOLS_INC_XMLPARSE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
