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
#include "precompiled_automation.hxx"
#include <tools/stream.hxx>
#include "statemnt.hxx"
#include "rcontrol.hxx"
#include "retstrm.hxx"
#include <basic/svtmsg.hrc>

#include <basic/ttstrhlp.hxx>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

using namespace com::sun::star::xml::sax;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;

using ::rtl::OUString;

class SVInputStream : public cppu::WeakImplHelper1< XInputStream >
{
    SvStream* pStream;
public:
    SVInputStream( SvStream* pSt ):pStream( pSt ){};
    ~SVInputStream(){ delete pStream; pStream=NULL; }

    // Methods XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};


sal_Int32 SAL_CALL SVInputStream::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    aData.realloc( nBytesToRead  );
    sal_Int32 nRead = pStream->Read( aData.getArray(), nBytesToRead );
    aData.realloc( nRead );
    return nRead;
}

sal_Int32 SAL_CALL SVInputStream::readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    return readBytes( aData, nMaxBytesToRead );
}

void SAL_CALL SVInputStream::skipBytes( sal_Int32 nBytesToSkip ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    if ( nBytesToSkip > 0 )
        pStream->SeekRel( nBytesToSkip );
}

sal_Int32 SAL_CALL SVInputStream::available(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    sal_uLong nCurrent = pStream->Tell();
    sal_uLong nSize = pStream->Seek( STREAM_SEEK_TO_END );
    sal_uLong nAvailable = nSize - nCurrent;
    pStream->Seek( nCurrent );
    return nAvailable;
}

void SAL_CALL SVInputStream::closeInput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
//  pStream->Close(); // automatically done in destructor
    delete pStream;
    pStream = NULL;
}

class Node;
SV_DECL_REF(Node)

enum NodeType { NODE_CHARACTER = CONST_NodeTypeCharacter,
                NODE_ELEMENT = CONST_NodeTypeElement,
                NODE_COMMENT = CONST_NodeTypeComment };

class Node : public SvRefBase
{
    NodeType aNodeType;
    Node* pParent;  // Use pointer to prevent cyclic references resulting in undeleted objects

protected:
    Node( NodeType aType ): aNodeType( aType ), pParent( NULL ){};
    virtual ~Node();

public:
    NodeType GetNodeType() { return aNodeType; }
    void SetParent( NodeRef xNewParent );
    NodeRef GetParent();
};

SV_IMPL_REF(Node)
// generate NodeRefMemberList
SV_DECL_IMPL_REF_LIST( NodeRef, Node* )

Node::~Node()
{
}

void Node::SetParent( NodeRef xNewParent )
{
    pParent = &xNewParent;
}

NodeRef Node::GetParent()
{
    return NodeRef( pParent );
}

class CharacterNode : public Node
{
    String aCharacters;
public:
    CharacterNode( const String& aChars ): Node( NODE_CHARACTER ), aCharacters( aChars ){};

    String GetCharacters() { return aCharacters; }
};

class ElementNode : public Node
{
    String aNodeName;
    Reference < XAttributeList > xAttributeList;
    NodeRefMemberList aDocumentNodeList;
public:
    ElementNode( const String& aName, Reference < XAttributeList > xAttributes );
    void AppendNode( NodeRef xNewNode );
    sal_uLong GetChildCount(){ return aDocumentNodeList.Count(); }
    NodeRef GetChild( sal_uInt16 nIndex ){ return aDocumentNodeList.GetObject( nIndex ); }
    Reference < XAttributeList > GetAttributes(){ return xAttributeList; }

    String GetNodeName() { return aNodeName; }
};

ElementNode::ElementNode( const String& aName, Reference < XAttributeList > xAttributes )
: Node( NODE_ELEMENT )
, aNodeName( aName )
{
    if ( xAttributes.is() )
    {
        Reference < XCloneable > xAttributeCloner( xAttributes, UNO_QUERY );
        if ( xAttributeCloner.is() )
            xAttributeList = Reference < XAttributeList > ( xAttributeCloner->createClone() , UNO_QUERY );
        else
        {
            OSL_FAIL("Unable to clone AttributeList");
        }
    }
};

void ElementNode::AppendNode( NodeRef xNewNode )
{
    aDocumentNodeList.Insert ( xNewNode, LIST_APPEND );
    xNewNode->SetParent( this );
}

//    XIndexAccess





enum ParseAction { COLLECT_DATA, COLLECT_DATA_IGNORE_WHITESPACE, PARSE_ONLY };

class SAXParser : public cppu::WeakImplHelper2< XErrorHandler, XDocumentHandler >
{
    String aFilename;
    Reference < XParser > xParser;

    // XErrorHandler
    void AddToList( const sal_Char* cuType, const ::com::sun::star::uno::Any& aSAXParseException );
    String aErrors;

    NodeRef xTreeRoot;
    NodeRef xCurrentNode;
    sal_uLong nTimestamp;
    ParseAction aAction;

public:
    SAXParser( const String &rFilename );
    ~SAXParser();

    // Access Methods
    NodeRef GetCurrentNode(){ return xCurrentNode; }
    void SetCurrentNode( NodeRef xCurrent ){ xCurrentNode = xCurrent; }
    NodeRef GetRootNode(){ return xTreeRoot; }
    sal_uLong GetTimestamp(){ return nTimestamp; }
    void Touch(){ nTimestamp = Time::GetSystemTicks(); }

    // Methods SAXParser
    sal_Bool Parse( ParseAction aAct );
    String GetErrors(){ return aErrors; }

    // Methods XErrorHandler
    virtual void SAL_CALL error( const ::com::sun::star::uno::Any& aSAXParseException ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL fatalError( const ::com::sun::star::uno::Any& aSAXParseException ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL warning( const ::com::sun::star::uno::Any& aSAXParseException ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    // Methods XDocumentHandler
    virtual void SAL_CALL startDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};


SAXParser::SAXParser( const String &rFilename )
: aFilename( rFilename )
{
    Touch();
}

SAXParser::~SAXParser()
{
    xParser.clear();
}

sal_Bool SAXParser::Parse( ParseAction aAct )
{
    aAction = aAct;
    Touch();
    SvStream* pStream = new SvFileStream( aFilename, STREAM_STD_READ );
    if ( pStream->GetError() )
        return sal_False;

    InputSource sSource;
    sSource.aInputStream = new SVInputStream( pStream );    // is refcounted and hence deletet appropriately
    sSource.sPublicId = OUString( aFilename );

    xParser = Reference < XParser > ( ::comphelper::getProcessServiceFactory()->createInstance( CUniString("com.sun.star.xml.sax.Parser") ), UNO_QUERY );
    if ( xParser.is() )
    {
        xParser->setErrorHandler( ( XErrorHandler*) this );
        if ( aAction == COLLECT_DATA || aAction == COLLECT_DATA_IGNORE_WHITESPACE )
            xParser->setDocumentHandler( ( XDocumentHandler*) this );

        try
        {
            xParser->parseStream ( sSource );
        }
        catch( class SAXParseException & rPEx)
        {
#ifdef DBG_ERROR
            String aMemo( rPEx.Message );
            aMemo = String( aMemo );
#endif
        }
        catch( class Exception & rEx)
        {
#ifdef DBG_ERROR
            String aMemo( rEx.Message );
            aMemo = String( aMemo );
#endif
        }
        xParser->setErrorHandler( NULL );   // otherwile Object holds itself
        if ( aAction == COLLECT_DATA || aAction == COLLECT_DATA_IGNORE_WHITESPACE )
            xParser->setDocumentHandler( NULL );    // otherwile Object holds itself
    }
    else
        return sal_False;
    return sal_True;
}


// Helper Methods XErrorHandler
void SAXParser::AddToList( const sal_Char* cuType, const ::com::sun::star::uno::Any& aSAXParseException )
{
    SAXParseException aException;
    aSAXParseException >>= aException;

    aErrors.Append( String( aException.PublicId ) );
    aErrors.AppendAscii( "(" );
    aErrors.Append( String::CreateFromInt64( aException.LineNumber ) );
    aErrors.AppendAscii( ":" );
    aErrors.Append( String::CreateFromInt64( aException.ColumnNumber ) );
    aErrors.AppendAscii( ") : " );
    aErrors.AppendAscii( cuType );
    aErrors.AppendAscii( ": " );
    aErrors.Append( String( aException.Message ) );
    aErrors.AppendAscii( "\n" );
}

// Methods XErrorHandler
void SAL_CALL SAXParser::error( const ::com::sun::star::uno::Any& aSAXParseException ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    AddToList( "error", aSAXParseException );
}

void SAL_CALL SAXParser::fatalError( const ::com::sun::star::uno::Any& aSAXParseException ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    AddToList( "fatal error", aSAXParseException );
}

void SAL_CALL SAXParser::warning( const ::com::sun::star::uno::Any& aSAXParseException ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    AddToList( "warning", aSAXParseException );
}


// Methods XDocumentHandler
void SAXParser::startDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    xTreeRoot = new ElementNode( CUniString("/"), Reference < XAttributeList > (NULL) );
    xCurrentNode = xTreeRoot;
    Touch();
}

void SAXParser::endDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
}

void SAXParser::startElement( const ::rtl::OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    NodeRef xNewNode = new ElementNode ( String(aName), xAttribs );
    ((ElementNode*)(&xCurrentNode))->AppendNode( xNewNode );
    xCurrentNode = xNewNode;
}

void SAXParser::endElement( const ::rtl::OUString& aName ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    (void) aName; /* avoid warning about unused parameter */
    xCurrentNode = xCurrentNode->GetParent();
}

void SAXParser::characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    if ( aAction == COLLECT_DATA_IGNORE_WHITESPACE )
    {   // check for whitespace
        sal_Bool bAllWhitespace = sal_True;
        for ( int i = 0 ; bAllWhitespace && i < aChars.getLength() ; i++ )
            if ( aChars[i] != 10 // LF
              && aChars[i] != 13 // CR
              && aChars[i] != ' ' // Blank
              && aChars[i] != '\t' ) // Tab
                bAllWhitespace = sal_False;
        if ( bAllWhitespace )
            return;
    }
    NodeRef xNewNode = new CharacterNode ( String(aChars) );
    ((ElementNode*)(&xCurrentNode))->AppendNode( xNewNode );
}

void SAXParser::ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    (void) aWhitespaces; /* avoid warning about unused parameter */
}

void SAXParser::processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    (void) aTarget; /* avoid warning about unused parameter */
    (void) aData; /* avoid warning about unused parameter */
}

void SAXParser::setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    (void) xLocator; /* avoid warning about unused parameter */
#if OSL_DEBUG_LEVEL > 1
    ::rtl::OUString aTester;
    aTester = xLocator->getPublicId();
    aTester = xLocator->getSystemId();
#endif
}




void StatementCommand::HandleSAXParser()
{
    static Reference < XReference > xParserKeepaliveReference;  // this is to keep the Object alive only
    static SAXParser* pSAXParser;

    // We need spechial prerequisites for these!

    ElementNode* pElementNode = NULL;
    switch ( nMethodId )
    {
        case RC_SAXGetNodeType:
        case RC_SAXGetAttributeCount:
        case RC_SAXGetAttributeName:
        case RC_SAXGetAttributeValue:
        case RC_SAXGetChildCount:
        case RC_SAXGetElementName:
        case RC_SAXGetChars:

        case RC_SAXSeekElement:
        case RC_SAXHasElement:
        case RC_SAXGetElementPath:
            {
                if ( xParserKeepaliveReference.is() && pSAXParser->GetCurrentNode().Is() )
                {
                    if ( pSAXParser->GetCurrentNode()->GetNodeType() == NODE_ELEMENT )
                    {
                        NodeRef xNode=pSAXParser->GetCurrentNode();
                        pElementNode = (ElementNode*)(&xNode);
                    }
                }
                else
                {
                    ReportError( GEN_RES_STR1( S_NO_SAX_PARSER, RcString( nMethodId ) ) );
                    return;
                }

            }
    }

    switch ( nMethodId )
    {
        case RC_SAXCheckWellformed:
            {
                if( (nParams & PARAM_STR_1) )
                {
                    xParserKeepaliveReference.clear();
                    pSAXParser = new SAXParser( aString1 );
                    xParserKeepaliveReference = ( XReference* )pSAXParser;
                    if ( !xParserKeepaliveReference.is() )
                        ReportError( GEN_RES_STR1( S_NO_SAX_PARSER, RcString( nMethodId ) ) );
                    else
                    {
                        if ( !pSAXParser->Parse( PARSE_ONLY ) )
                            ReportError( GEN_RES_STR1( S_NO_SAX_PARSER, RcString( nMethodId ) ) );
                        pRet->GenReturn ( RET_Value, nMethodId, pSAXParser->GetErrors() );
                    }

                    xParserKeepaliveReference.clear();
                }
                else
                    ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
            }
            break;

        case RC_SAXReadFile:
            {
                if( (nParams & PARAM_STR_1) )
                {
                    ParseAction aAction;
                       if( (nParams & PARAM_BOOL_1) && bBool1 )
                        aAction = COLLECT_DATA;
                    else
                        aAction = COLLECT_DATA_IGNORE_WHITESPACE;

                    xParserKeepaliveReference.clear();
                    pSAXParser = new SAXParser( aString1 );
                    xParserKeepaliveReference = ( XReference* )pSAXParser;
                    if ( !xParserKeepaliveReference.is() )
                        ReportError( GEN_RES_STR1( S_NO_SAX_PARSER, RcString( nMethodId ) ) );
                    else
                    {

                        if ( !pSAXParser->Parse( aAction ) )
                            ReportError( GEN_RES_STR1( S_NO_SAX_PARSER, RcString( nMethodId ) ) );
                        pRet->GenReturn ( RET_Value, nMethodId, pSAXParser->GetErrors() );
                    }
                }
                else
                    ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
            }
            break;
        case RC_SAXGetNodeType:
            {
                   pRet->GenReturn ( RET_Value, nMethodId, (comm_ULONG)pSAXParser->GetCurrentNode()->GetNodeType() );
            }
            break;
        case RC_SAXGetAttributeCount:
        case RC_SAXGetAttributeName:
        case RC_SAXGetAttributeValue:
        case RC_SAXGetChildCount:
        case RC_SAXGetElementName:
            {
                if ( pElementNode )
                {
                    Reference < XAttributeList > xAttributeList = pElementNode->GetAttributes();
                    switch ( nMethodId )
                    {
                        case RC_SAXGetElementName:
                            pRet->GenReturn ( RET_Value, nMethodId, pElementNode->GetNodeName() );
                            break;
                        case RC_SAXGetChildCount:
                            pRet->GenReturn ( RET_Value, nMethodId, (comm_ULONG)pElementNode->GetChildCount() );
                            break;
                        case RC_SAXGetAttributeCount:
                            if ( xAttributeList.is() )
                                pRet->GenReturn ( RET_Value, nMethodId, (comm_ULONG)xAttributeList->getLength() );
                            else
                                pRet->GenReturn ( RET_Value, nMethodId, (comm_ULONG)0 );
                            break;
                        case RC_SAXGetAttributeName:
                            {
                                if( (nParams & PARAM_USHORT_1) && ValueOK( rtl::OString(), RcString( nMethodId ), nNr1, xAttributeList.is()?xAttributeList->getLength():0 ) )
                                {
                                    String aRet( xAttributeList->getNameByIndex( nNr1-1 ) );
                                    pRet->GenReturn ( RET_Value, nMethodId, aRet );
                                }
                                else
                                    ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
                            }
                            break;
                        case RC_SAXGetAttributeValue:
                            // Number or String
                            {
                                if( (nParams & PARAM_USHORT_1) && ValueOK( rtl::OString(), RcString( nMethodId ), nNr1, xAttributeList.is()?xAttributeList->getLength():0 ) )
                                {
                                    String aRet( xAttributeList->getValueByIndex( nNr1-1 ) );
                                    pRet->GenReturn ( RET_Value, nMethodId, aRet );
                                }
                                else if( (nParams & PARAM_STR_1) && xAttributeList.is() )
                                {
                                    String aRet( xAttributeList->getValueByName( aString1 ) );
                                    pRet->GenReturn ( RET_Value, nMethodId, aRet );
                                }
                                else
                                    ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
                            }
                            break;

                        default:
                            ReportError( GEN_RES_STR1( S_INTERNAL_ERROR, RcString( nMethodId ) ) );
                    }
                }
                else
                    ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
            }
            break;
        case RC_SAXGetChars:
            {
                if ( pSAXParser->GetCurrentNode()->GetNodeType() == NODE_CHARACTER )
                {
                    NodeRef xNode=pSAXParser->GetCurrentNode();
                    CharacterNode* aCharacterNode = (CharacterNode*)(&xNode);
                       pRet->GenReturn ( RET_Value, nMethodId, aCharacterNode->GetCharacters() );
                }
                else
                    ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
            }
            break;

        case RC_SAXSeekElement:
        case RC_SAXHasElement:
            // Number or String
            {
                sal_Bool bCheckOnly = nMethodId == RC_SAXHasElement;

                if( (nParams & PARAM_USHORT_1) && !(nParams & PARAM_STR_1) )
                {
                    if ( nNr1 == 0 )
                    {
                        if ( bCheckOnly )
                            pRet->GenReturn ( RET_Value, nMethodId, pSAXParser->GetCurrentNode()->GetParent().Is() );
                        else if ( pSAXParser->GetCurrentNode()->GetParent().Is() )
                            pSAXParser->SetCurrentNode( pSAXParser->GetCurrentNode()->GetParent() );
                    }
                    else if ( !pElementNode )
                        ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
                    else if ( bCheckOnly )
                        pRet->GenReturn ( RET_Value, nMethodId, ValueOK( rtl::OString(), RcString( nMethodId ), nNr1, pElementNode->GetChildCount() ) );
                    else if ( ValueOK( rtl::OString(), RcString( nMethodId ), nNr1, pElementNode->GetChildCount() ) )
                        pSAXParser->SetCurrentNode( pElementNode->GetChild( nNr1-1 ) );
                }
                else if( (nParams & PARAM_STR_1) )
                {
                    if ( aString1.EqualsAscii( "/" ) )
                    {
                        if ( bCheckOnly )
                            pRet->GenReturn ( RET_Value, nMethodId, (comm_BOOL)sal_True );
                        else
                            pSAXParser->SetCurrentNode( pSAXParser->GetRootNode() );
                    }
                    else if ( aString1.Copy(0,2).EqualsAscii( "*:" ) )
                    {
                        sal_uLong nTimestamp = (sal_uLong)aString1.GetToken( 1, ':' ).ToInt64();
                        sal_uLong nPointer = (sal_uLong)aString1.GetToken( 2, ':' ).ToInt64();
                        if ( bCheckOnly )
                            pRet->GenReturn ( RET_Value, nMethodId, (comm_BOOL)(pSAXParser->GetTimestamp() == nTimestamp) );
                        else
                            if ( pSAXParser->GetTimestamp() == nTimestamp )
                            {
                                {
                                    Node* pNode = (Node*)nPointer;
                                    pSAXParser->SetCurrentNode( NodeRef( pNode ) );
                                }
                            }
                            else
                                ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
                    }
                    else if ( pElementNode )
                    {
                        sal_uInt16 nNthOccurrence;
                        if( (nParams & PARAM_USHORT_1) )
                            nNthOccurrence = nNr1;
                        else
                            nNthOccurrence = 1;

                        sal_uInt16 i;
                        NodeRef xNew;
                        for ( i = 0 ; i < pElementNode->GetChildCount() && !xNew.Is() ; i++ )
                        {
                            xNew = pElementNode->GetChild( i );
                            if ( xNew->GetNodeType() == NODE_ELEMENT )
                            {
                                ElementNode* pNewElement = (ElementNode*)(&xNew);
                                if ( aString1.Equals( pNewElement->GetNodeName() ) )
                                {
                                    if ( nNthOccurrence > 1 )
                                    {
                                        xNew.Clear();
                                        nNthOccurrence--;
                                    }
                                }
                                else
                                    xNew.Clear();
                            }
                            else
                                xNew.Clear();
                        }
                        if ( bCheckOnly )
                            pRet->GenReturn ( RET_Value, nMethodId, xNew.Is() );
                        else
                            if ( xNew.Is() )
                                pSAXParser->SetCurrentNode( xNew );
                            else
                                ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
                    }
                    else
                        if ( bCheckOnly )
                            pRet->GenReturn ( RET_Value, nMethodId, (comm_BOOL)sal_False );
                        else
                            ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
                }
                else
                    ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
            }
            break;
        case RC_SAXGetElementPath:
            {
                DBG_ASSERT( sizeof( sal_uIntPtr ) == sizeof ( void* ), "Pointertype has different size than sal_uIntPtr");
                String aPath;
                aPath.AppendAscii( "*:" );
                aPath.Append( String::CreateFromInt64( pSAXParser->GetTimestamp() ) );
                aPath.AppendAscii( ":" );
                NodeRef xNode=pSAXParser->GetCurrentNode();
                Node* pNode = (Node*)(&xNode);
                aPath.Append( String::CreateFromInt64( (sal_uIntPtr)pNode ) );
                pRet->GenReturn ( RET_Value, nMethodId, aPath );
            }
            break;

        case RC_SAXRelease:
            {
                xParserKeepaliveReference.clear();
            }
            break;
        default:
            ReportError( GEN_RES_STR1( S_INTERNAL_ERROR, RcString( nMethodId ) ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
