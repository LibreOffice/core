/*************************************************************************
 *
 *  $RCSfile: XMLParser.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 15:52:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#include "statemnt.hxx"
#include "rcontrol.hxx"
#include "retstrm.hxx"
#include <svtools/svtmsg.hrc>

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#include <com/sun/star/xml/sax/SAXParseException.hpp>

using namespace com::sun::star::xml::sax;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace rtl;

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
    ULONG nCurrent = pStream->Tell();
    ULONG nSize = pStream->Seek( STREAM_SEEK_TO_END );
    ULONG nAvailable = nSize - nCurrent;
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
    USHORT GetChildCount(){ return aDocumentNodeList.Count(); }
    NodeRef GetChild( USHORT nIndex ){ return aDocumentNodeList.GetObject( nIndex ); }
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
            DBG_ERROR("Unable to clone AttributeList")
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
    void AddToList( sal_Char* cuType, const ::com::sun::star::uno::Any& aSAXParseException );
    String aErrors;

    NodeRef xTreeRoot;
    NodeRef xCurrentNode;
    ULONG nTimestamp;
    ParseAction aAction;

public:
    SAXParser( const String &rFilename );
    ~SAXParser();

    // Access Methods
    NodeRef GetCurrentNode(){ return xCurrentNode; }
    void SetCurrentNode( NodeRef xCurrent ){ xCurrentNode = xCurrent; }
    NodeRef GetRootNode(){ return xTreeRoot; }
    ULONG GetTimestamp(){ return nTimestamp; }
    void Touch(){ nTimestamp = Time::GetSystemTicks(); }

    // Methods SAXParser
    BOOL Parse( ParseAction aAct );
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

BOOL SAXParser::Parse( ParseAction aAct )
{
    aAction = aAct;
    Touch();
    SvStream* pStream = new SvFileStream( aFilename, STREAM_STD_READ );
    if ( pStream->GetError() )
        return FALSE;

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
        return FALSE;
    return TRUE;
}


// Helper Methods XErrorHandler
void SAXParser::AddToList( sal_Char* cuType, const ::com::sun::star::uno::Any& aSAXParseException )
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
    xCurrentNode = xCurrentNode->GetParent();
}

void SAXParser::characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    if ( aAction == COLLECT_DATA_IGNORE_WHITESPACE )
    {   // check for whitespace
        BOOL bAllWhitespace = TRUE;
        for ( int i = 0 ; bAllWhitespace && i < aChars.getLength() ; i++ )
            if ( aChars[i] != 10 // LF
              && aChars[i] != 13 // CR
              && aChars[i] != ' ' // Blank
              && aChars[i] != '\t' ) // Tab
                bAllWhitespace = FALSE;
        if ( bAllWhitespace )
            return;
    }
    NodeRef xNewNode = new CharacterNode ( String(aChars) );
    ((ElementNode*)(&xCurrentNode))->AppendNode( xNewNode );
}

void SAXParser::ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
}

void SAXParser::processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
}

void SAXParser::setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
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
                   pRet->GenReturn ( RET_Value, nMethodId, (ULONG)pSAXParser->GetCurrentNode()->GetNodeType() );
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
                            pRet->GenReturn ( RET_Value, nMethodId, (ULONG)pElementNode->GetChildCount() );
                            break;
                        case RC_SAXGetAttributeCount:
                            if ( xAttributeList.is() )
                                pRet->GenReturn ( RET_Value, nMethodId, (ULONG)xAttributeList->getLength() );
                            else
                                pRet->GenReturn ( RET_Value, nMethodId, (ULONG)0 );
                            break;
                        case RC_SAXGetAttributeName:
                            {
                                if( (nParams & PARAM_USHORT_1) && ValueOK( 0, RcString( nMethodId ), nNr1, xAttributeList.is()?xAttributeList->getLength():0 ) )
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
                                if( (nParams & PARAM_USHORT_1) && ValueOK( 0, RcString( nMethodId ), nNr1, xAttributeList.is()?xAttributeList->getLength():0 ) )
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
                BOOL bCheckOnly = nMethodId == RC_SAXHasElement;

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
                        pRet->GenReturn ( RET_Value, nMethodId, ValueOK( 0, String(), nNr1, pElementNode->GetChildCount() ) );
                    else if ( ValueOK( 0, RcString( nMethodId ), nNr1, pElementNode->GetChildCount() ) )
                        pSAXParser->SetCurrentNode( pElementNode->GetChild( nNr1-1 ) );
                }
                else if( (nParams & PARAM_STR_1) )
                {
                    if ( aString1.EqualsAscii( "/" ) )
                    {
                        if ( bCheckOnly )
                            pRet->GenReturn ( RET_Value, nMethodId, (BOOL)TRUE );
                        else
                            pSAXParser->SetCurrentNode( pSAXParser->GetRootNode() );
                    }
                    else if ( aString1.Copy(0,2).EqualsAscii( "*:" ) )
                    {
                        ULONG nTimestamp = (ULONG)aString1.GetToken( 1, ':' ).ToInt64();
                        ULONG nPointer = (ULONG)aString1.GetToken( 2, ':' ).ToInt64();
                        if ( bCheckOnly )
                            pRet->GenReturn ( RET_Value, nMethodId, (BOOL)(pSAXParser->GetTimestamp() == nTimestamp) );
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
                        USHORT nNthOccurance;
                        if( (nParams & PARAM_USHORT_1) )
                            nNthOccurance = nNr1;
                        else
                            nNthOccurance = 1;

                        USHORT i;
                        NodeRef xNew;
                        for ( i = 0 ; i < pElementNode->GetChildCount() && !xNew.Is() ; i++ )
                        {
                            xNew = pElementNode->GetChild( i );
                            if ( xNew->GetNodeType() == NODE_ELEMENT )
                            {
                                ElementNode* pNewElement = (ElementNode*)(&xNew);
                                if ( aString1.Equals( pNewElement->GetNodeName() ) )
                                {
                                    if ( nNthOccurance > 1 )
                                    {
                                        xNew.Clear();
                                        nNthOccurance--;
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
                            pRet->GenReturn ( RET_Value, nMethodId, (BOOL)FALSE );
                        else
                            ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
                }
                else
                    ReportError( GEN_RES_STR0( S_INVALID_PARAMETERS ) );
            }
            break;
        case RC_SAXGetElementPath:
            {
                DBG_ASSERT( sizeof( ULONG ) == sizeof ( void* ), "Pointertype has different size than ULONG")
                String aPath;
                aPath.AppendAscii( "*:" );
                aPath.Append( String::CreateFromInt64( pSAXParser->GetTimestamp() ) );
                aPath.AppendAscii( ":" );
                NodeRef xNode=pSAXParser->GetCurrentNode();
                Node* pNode = (Node*)(&xNode);
                aPath.Append( String::CreateFromInt64( (ULONG)pNode ) );
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

