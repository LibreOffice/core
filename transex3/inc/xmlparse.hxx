/*************************************************************************
 *
 *  $RCSfile: xmlparse.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-18 11:18:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef BOOTSTRP_XMLPARSE_HXX
#define BOOTSTRP_XMLPARSE_HXX

#include <signal.h>
//#include "osl/signal.h"
#ifdef SYSTEM_EXPAT
#include <expat.h>
#else
#include <external/expat/xmlparse.h>
#endif
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

/*
typedef void (*Sigfunc)(int);
Sigfunc signal(int, Sigfunc*);
*/
typedef void Sigfunc(int);
Sigfunc* signal(int, Sigfunc*);

using namespace ::rtl;
using namespace std;

#include <hash_map> /* std::hashmap*/
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


//#define TESTDRIVER        /* use xml2gsi testclass */
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

DECLARE_LIST( XMLAttributeList, XMLAttribute * );

//-------------------------------------------------------------------------

/** Virtual base to handle different kinds of XML nodes
 */
class XMLNode
{
protected:
    XMLNode() {}
    XMLNode( const XMLNode& obj);
    XMLNode& operator=(const XMLNode& obj);

public:
    virtual USHORT GetNodeType() = 0;
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
    //XMLChildNode( const XMLChildNode& );
public:
    virtual USHORT GetNodeType() = 0;

    /// returns the parent of this node
    XMLParentNode *GetParent() { return pParent; }
};

DECLARE_LIST( XMLChildNodeList, XMLChildNode * );

//-------------------------------------------------------------------------

/** Virtual base to handle different kinds of parent nodes
 */
class XMLData;

class XMLParentNode : public XMLChildNode
{
private:
    XMLChildNodeList *pChildList;
    //int         nParentPos;
protected:
    XMLParentNode( XMLParentNode *pPar )
                : XMLChildNode( pPar ), pChildList( NULL )
                //, nParentPos( 0 )
                {}
    XMLParentNode(){}
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
        XMLChildNode *pChild , int pos  /// the new child
    );
    int GetPos( ByteString id );
    int RemoveChild( XMLElement *pRefElement );
    void RemoveAndDeleteAllChilds();
    //int     ParentPosInc(){ fprintf(stdout,"%d ",nParentPos);return 0;}//nParentPos++;}

//    void AddContent( XMLElement& rXMLEelement_in , XMLData& rData_in );
/*  {
        if ( pChildList ) {
            for ( ULONG i = 0; i < pChildList->Count(); i++ )
                delete pChildList->GetObject( i );
        pChildList->Clear();
    }
*/
    /// returns a child element which matches the given one
    XMLElement *GetChildElement(
        XMLElement *pRefElement // the reference elelement
    );
};

//-------------------------------------------------------------------------
/*
struct eqstr{
  BOOL operator()(const char* s1, const char* s2) const{
    return strcmp(s1,s2)==0;
  }
};

struct equalByteString{
    bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
        return rKey1.CompareTo(rKey2)==COMPARE_EQUAL;
    }
};
struct hashByteString{
    size_t operator()( const ByteString& rName ) const{
        std::hash< const char* > myHash;
        return myHash( rName.GetBuffer() );
    }
};
*/
DECLARE_LIST( XMLStringList, XMLElement* );



/// Mapping numeric Language code <-> XML Element
//typedef std::hash_map<int,XMLElement*>                                    LangHashMap;
typedef std::hash_map< ByteString ,XMLElement* , hashByteString,equalByteString > LangHashMap;

/// Mapping XML Element string identifier <-> Language Map
typedef std::hash_map<ByteString , LangHashMap* ,
                      hashByteString,equalByteString>                   XMLHashMap;

/// Mapping iso alpha string code <-> iso numeric code
typedef std::hash_map<ByteString, int, hashByteString,equalByteString>  HashMap;

/// Mapping XML tag names <-> have localizable strings
typedef std::hash_map<ByteString , BOOL ,
                      hashByteString,equalByteString>                   TagMap;

/** Holds information of a XML file, is root node of tree
 */


class XMLFile : public XMLParentNode
{
public:
    XMLFile(
                const String &rFileName // the file name, empty if created from memory stream
    );
    ByteString* GetGroupID(std::deque<ByteString> &groupid);
    void        Print( XMLNode *pCur = NULL, USHORT nLevel = 0 );
    void        SearchL10NElements( XMLParentNode *pCur  , int pos = 0);
    void        Extract( XMLFile *pCur = NULL );
    void        View();
    void static Signal_handler(int signo);//void*,oslSignalInfo * pInfo);
    void        showType(XMLParentNode* node);

    XMLHashMap* GetStrings(){return XMLStrings;}
    BOOL        Write( String &rFileName );
    BOOL        Write() { return Write( sFileName ); }
    BOOL        Write( ofstream &rStream , XMLNode *pCur = NULL );

    bool        CheckExportStatus( XMLParentNode *pCur = NULL , int pos = 0 );

    //XMLFile& XMLFile::operator=(const XMLFile& obj);

    virtual USHORT  GetNodeType();

    /// returns file name
    const String &GetName() { return sFileName; }
    const std::vector<ByteString> getOrder(){ return order; }

private:
    // writes a string as UTF8 with dos line ends to a given stream
    void        WriteString( ofstream &rStream, const String &sString );

    // quotes the given text for writing to a file
    void        QuotHTML( String &rString );

    void        InsertL10NElement( XMLElement* pElement);

    // DATA
    String      sFileName;

    const ByteString ID,XML_LANG,OLDREF;

    //HashMap nodes_include,nodes_localize,nodes_print;
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
    USHORT              GetLangByIsoLang( const ByteString &rIsoLang );

    /// Return the alpha strings representation
    ByteString          GetIsoLangByIndex( USHORT nIndex );

    static XMLUtil&     XMLUtil::Instance();
    ~XMLUtil();

    void         dump();

private:
    /// Mapping iso alpha string code <-> iso numeric code
    HashMap      lMap;

    /// Mapping iso numeric code      <-> iso alpha string code
    ByteString   isoArray[MAX_LANGUAGES];

    //static bool inline XMLUtil::hasPrefix( String& rString , int pos);
    static void UnQuotData( String &rString );
    static void UnQuotTags( String &rString );

    XMLUtil();                  // private Constructor
    XMLUtil(const XMLUtil&);    // private Copyconstructor

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
        XMLParentNode *pParent  // parent node of this element
    ):          XMLParentNode( pParent ),
                sElementName( rName ),
                pAttributes( NULL ),
                project(""),
                filename(""),
                id(""),
                sOldRef(""),
                resourceType(""),
                languageId(""),
                nPos(0)
                   {}
    ~XMLElement();
    /// Copyconstructor
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

    int        GetPos()         { return nPos;         }
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
    /// craete a data node
    XMLData(
        const String &rData,    // the initial data
        XMLParentNode *pParent  // the parent node of this data, typically a element node
    )
                : XMLChildNode( pParent ), sData( rData ) , isNewCreated ( false ){}
    XMLData(
        const String &rData,    // the initial data
        XMLParentNode *pParent, // the parent node of this data, typically a element node
        bool newCreated
    )
                : XMLChildNode( pParent ), sData( rData ) , isNewCreated ( newCreated ){}

    XMLData(const XMLData& obj);

    XMLData& operator=(const XMLData& obj);
    /// returns node type XML_NODE_TYPE_DATA
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
        XMLParentNode *pParent  // the parent node of this comemnt, typically a element node
    )
                : XMLChildNode( pParent ), sComment( rComment ) {}

    /// returns node type XML_NODE_TYPE_COMMENT
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
        XMLParentNode *pParent  // the parent node of this comemnt, typically a element node
    )
                : XMLChildNode( pParent ), sDefault( rDefault ) {}

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
        const String &rFileName // the file name
    );

    /// parse a memory stream, returns NULL on criticall errors
    XMLFile *Execute(
        SvMemoryStream *pStream // the stream
    );

    /// returns an error struct
    const XMLError &GetError() { return aErrorInformation; }
};

#endif
