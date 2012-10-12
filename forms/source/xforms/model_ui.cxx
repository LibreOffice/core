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


#include "model.hxx"
#include "model_helper.hxx"
#include "mip.hxx"
#include "evaluationcontext.hxx"
#include "unohelper.hxx"
#include "submission/serialization_app_xml.hxx"
#include "resourcehelper.hxx"
#include "xmlhelper.hxx"
#include "convert.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>

// UNO classes
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xsd/XDataType.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>


using rtl::OUString;
using rtl::OUStringBuffer;
using com::sun::star::beans::PropertyValue;
using com::sun::star::io::XInputStream;
using com::sun::star::io::XActiveDataSink;
using com::sun::star::io::XTextInputStream;
using com::sun::star::container::XEnumeration;
using com::sun::star::container::XNameContainer;
using com::sun::star::xforms::XFormsSupplier;

using namespace xforms;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::xpath;



//
// implement XFormsUIHelper1
//

OUString Model::getDefaultServiceNameForNode( const XNode_t& xNode )
    throw( RuntimeException )
{
    // determine service for control. string/text field is default.
    OUString sService = OUSTRING("com.sun.star.form.component.TextField");

    // query repository for suitable type
    OSL_ENSURE( mxDataTypes.is(), "no type repository?" );
    OUString sTypeName = queryMIP( xNode ).getTypeName();
    if( mxDataTypes->hasByName( sTypeName ) )
    {
        OSL_ENSURE( mxDataTypes->getDataType( sTypeName ).is(),
                    "has or has not?" );

        switch( mxDataTypes->getDataType( sTypeName )->getTypeClass() )
        {
        case com::sun::star::xsd::DataTypeClass::BOOLEAN:
            sService = OUSTRING("com.sun.star.form.component.CheckBox");
            break;
        case com::sun::star::xsd::DataTypeClass::DOUBLE:
        case com::sun::star::xsd::DataTypeClass::DECIMAL:
        case com::sun::star::xsd::DataTypeClass::FLOAT:
            sService = OUSTRING("com.sun.star.form.component.NumericField");
            break;

        case com::sun::star::xsd::DataTypeClass::STRING:
        case com::sun::star::xsd::DataTypeClass::DURATION:
        case com::sun::star::xsd::DataTypeClass::DATETIME:
        case com::sun::star::xsd::DataTypeClass::TIME:
        case com::sun::star::xsd::DataTypeClass::DATE:
        case com::sun::star::xsd::DataTypeClass::gYearMonth:
        case com::sun::star::xsd::DataTypeClass::gYear:
        case com::sun::star::xsd::DataTypeClass::gMonthDay:
        case com::sun::star::xsd::DataTypeClass::gDay:
        case com::sun::star::xsd::DataTypeClass::gMonth:
        case com::sun::star::xsd::DataTypeClass::hexBinary:
        case com::sun::star::xsd::DataTypeClass::base64Binary:
        case com::sun::star::xsd::DataTypeClass::anyURI:
        case com::sun::star::xsd::DataTypeClass::QName:
        case com::sun::star::xsd::DataTypeClass::NOTATION:
        default:
            // keep default
            break;
        }
    }

    return sService;
}


static void lcl_OutPosition( OUStringBuffer& rBuffer,
                      const Reference<XNode>& xNode )
{
    OSL_ENSURE( xNode->getParentNode().is(), "need parent" );

    // count # of occurrences of this node
    sal_Int32 nFound = 0;
    sal_Int32 nPosition = -1;
    if( xNode->getParentNode().is() )
    {
        for( Reference<XNode> xIter = xNode->getParentNode()->getFirstChild();
             xIter != NULL;
             xIter = xIter->getNextSibling() )
        {
            if( xIter->getNodeType() == xNode->getNodeType() &&
                xIter->getNodeName() == xNode->getNodeName() &&
                xIter->getNamespaceURI() == xNode->getNamespaceURI() )
            {
                nFound++;
                if( xIter == xNode )
                    nPosition = nFound;
            }
        }
    }
    OSL_ENSURE( nFound > 0  &&  nPosition > 0, "node not found???" );

    // output position (if necessary)
    if( nFound > 1 )
    {
        rBuffer.insert( 0, sal_Unicode(']') );
        rBuffer.insert( 0, nPosition );
        rBuffer.insert( 0, sal_Unicode('[') );
    }
}

static void lcl_OutName( OUStringBuffer& rBuffer,
                  const Reference<XNode>& xNode )
{
    rBuffer.insert( 0, xNode->getNodeName() );
    OUString sPrefix = xNode->getPrefix();
    if( !sPrefix.isEmpty() )
    {
        rBuffer.insert( 0, sal_Unicode(':') );
        rBuffer.insert( 0, sPrefix );
    }
}

static void lcl_OutInstance( OUStringBuffer& rBuffer,
                      const Reference<XNode>& xNode,
                      Model* pModel )
{
    Reference<XDocument> xDoc = xNode->getOwnerDocument();

    if( xDoc != pModel->getDefaultInstance() )
    {
        rBuffer.insert( 0, OUSTRING("')") );

        // iterate over instances, and find the right one
        OUString sInstanceName;
        Reference<XEnumeration> xEnum =
            pModel->getInstances()->createEnumeration();
        while( sInstanceName.isEmpty() && xEnum->hasMoreElements() )
        {
            Sequence<PropertyValue> aValues;
            xEnum->nextElement() >>= aValues;

            // get ID and instance
            OUString sId;
            Reference<XDocument> xInstance;
            getInstanceData( aValues, &sId, &xInstance, NULL, NULL );

            // now check whether this was our instance:
            if( xInstance == xDoc )
                sInstanceName = sId;
        }

        rBuffer.insert( 0, sInstanceName );
        rBuffer.insert( 0, OUSTRING("instance('") );
    }
}

OUString Model::getDefaultBindingExpressionForNode(
    const XNode_t& xNode,
    const EvaluationContext& rContext)
{
    OSL_ENSURE( xNode.is(), "need node" );

    // iterate upwards and put sections into the expression buffer.
    // Stop iteration either at context node (relative expression) or
    // at document root, whichever occurs first.
    OUStringBuffer aBuffer;
    for( Reference<XNode> xCurrent = xNode;
         xCurrent.is()  &&  xCurrent != rContext.mxContextNode;
         xCurrent = xCurrent->getParentNode() )
    {
        // insert a '/' for every step except the first
        if( aBuffer.getLength() > 0 )
            aBuffer.insert( 0, sal_Unicode('/') );

        switch( xCurrent->getNodeType() )
        {
        case NodeType_ELEMENT_NODE:
            lcl_OutPosition( aBuffer, xCurrent );
            lcl_OutName( aBuffer, xCurrent );
            break;

        case NodeType_TEXT_NODE:
            lcl_OutPosition( aBuffer, xCurrent );
            aBuffer.insert( 0, OUSTRING("text()") );
            break;

        case NodeType_ATTRIBUTE_NODE:
            lcl_OutName( aBuffer, xCurrent );
            aBuffer.insert( 0, sal_Unicode('@') );
            break;

        case NodeType_DOCUMENT_NODE:
            // check for which instance we have
            lcl_OutInstance( aBuffer, xCurrent, this );
            break;

        default:
            // unknown type? fail!
            OSL_FAIL( "unknown node type!" );
            xCurrent.set( NULL );
            aBuffer.makeStringAndClear();
            // we'll remove the slash below
            aBuffer.insert( 0, sal_Unicode('/') );
            break;
        }
    }

    return aBuffer.makeStringAndClear();
}



OUString Model::getDefaultBindingExpressionForNode( const XNode_t& xNode )
    throw( RuntimeException )
{
    return getDefaultBindingExpressionForNode( xNode, getEvaluationContext() );
}

static bool lcl_isWhitespace( const OUString& rString )
{
    sal_Int32 nLength = rString.getLength();
    const sal_Unicode* pStr = rString.getStr();

    bool bWhitespace = true;
    for( sal_Int32 i = 0; bWhitespace && ( i < nLength ); i++ )
    {
        sal_Unicode c = pStr[i];
        bWhitespace = ( c == sal_Unicode(0x09) ||
                        c == sal_Unicode(0x0A) ||
                        c == sal_Unicode(0x0D) ||
                        c == sal_Unicode(0x20) );
    }
    return bWhitespace;
}

OUString Model::getNodeDisplayName( const XNode_t& xNode,
                                    sal_Bool bDetail )
    throw( RuntimeException )
{
    OUStringBuffer aBuffer;

    switch( xNode->getNodeType() )
    {
    case NodeType_ELEMENT_NODE:
        lcl_OutName( aBuffer, xNode );
        break;

    case NodeType_TEXT_NODE:
        {
            OUString sContent = xNode->getNodeValue();
            if( bDetail || ! lcl_isWhitespace( sContent ) )
            {
                aBuffer.append( sal_Unicode('"') );
                aBuffer.append( Convert::collapseWhitespace( sContent ) );
                aBuffer.append( sal_Unicode('"') );
            }
        }
        break;

    case NodeType_ATTRIBUTE_NODE:
        lcl_OutName( aBuffer, xNode );
        aBuffer.insert( 0, sal_Unicode('@') );
        break;

    case NodeType_DOCUMENT_NODE:
        if( xNode == getDefaultInstance() )
            aBuffer.append( sal_Unicode('/') );
        else
            lcl_OutInstance( aBuffer, xNode, this );
        break;

    default:
        // unknown type? fail!
        OSL_FAIL( "unknown node type!" );
        break;
    }

    return aBuffer.makeStringAndClear();
}

OUString Model::getNodeName( const XNode_t& xNode )
    throw( RuntimeException )
{
    OUStringBuffer aBuffer;

    switch( xNode->getNodeType() )
    {
    case NodeType_ELEMENT_NODE:
    case NodeType_ATTRIBUTE_NODE:
        lcl_OutName( aBuffer, xNode );
        break;

    case NodeType_TEXT_NODE:
    case NodeType_DOCUMENT_NODE:
    default:
        // unknown type? fail!
        OSL_FAIL( "no name for this node type!" );
        break;
    }

    return aBuffer.makeStringAndClear();
}

OUString Model::getBindingName( const XPropertySet_t& xBinding,
                                sal_Bool /*bDetail*/ )
    throw( RuntimeException )
{
    OUString sID;
    xBinding->getPropertyValue( OUSTRING("BindingID" ) ) >>= sID;
    OUString sExpression;
    xBinding->getPropertyValue( OUSTRING("BindingExpression" ) ) >>= sExpression;

    OUStringBuffer aBuffer;
    if( !sID.isEmpty() )
    {
        aBuffer.append( sID );
        aBuffer.append( OUSTRING(" (" ));
        aBuffer.append( sExpression );
        aBuffer.append( OUSTRING(")" ));
    }
    else
        aBuffer.append( sExpression );

    return aBuffer.makeStringAndClear();
}

OUString Model::getSubmissionName( const XPropertySet_t& xSubmission,
                                   sal_Bool /*bDetail*/ )
    throw( RuntimeException )
{
    OUString sID;
    xSubmission->getPropertyValue( OUSTRING("ID") ) >>= sID;
    return sID;
}

Model::XPropertySet_t Model::cloneBindingAsGhost( const XPropertySet_t &xBinding )
    throw( RuntimeException )
{
    // Create a new binding instance first...
    Binding *pBinding = new Binding();

    // ...and bump up the "defered notification counter"
    // to prevent this binding from contributing to the
    // MIPs table...
    pBinding->deferNotifications(true);

    // Copy the propertyset and return result...
    XPropertySet_t xNewBinding(pBinding);
    copy( xBinding, xNewBinding );
    return xNewBinding;
}

void Model::removeBindingIfUseless( const XPropertySet_t& xBinding )
    throw( RuntimeException )
{
    Binding* pBinding = Binding::getBinding( xBinding );
    if( pBinding != NULL )
    {
        if( ! pBinding->isUseful() )
            mpBindings->removeItem( pBinding );
    }
}

Model::XDocument_t Model::newInstance( const rtl::OUString& sName,
                         const rtl::OUString& sURL,
                         sal_Bool bURLOnce )
    throw( RuntimeException )
{
    // create a default instance with <instanceData> element
    XDocument_t xInstance = getDocumentBuilder()->newDocument();
    DBG_ASSERT( xInstance.is(), "failed to create DOM instance" );

    Reference<XNode>( xInstance, UNO_QUERY_THROW )->appendChild(
        Reference<XNode>( xInstance->createElement( OUSTRING("instanceData") ),
                          UNO_QUERY_THROW ) );

    Sequence<PropertyValue> aSequence;
    bool bOnce = bURLOnce; // bool, so we can take address in setInstanceData
    setInstanceData( aSequence, &sName, &xInstance, &sURL, &bOnce );
    sal_Int32 nInstance = mpInstances->addItem( aSequence );
    loadInstance( nInstance );

    return xInstance;
}

static sal_Int32 lcl_findProp( const PropertyValue* pValues,
                        sal_Int32 nLength,
                        const rtl::OUString& rName )
{
    bool bFound = false;
    sal_Int32 n = 0;
    for( ; !bFound && n < nLength; n++ )
    {
        bFound = ( pValues[n].Name == rName );
    }
    return bFound ? ( n - 1) : -1;
}

sal_Int32 xforms::lcl_findInstance( const InstanceCollection* pInstances,
                                    const rtl::OUString& rName )
{
    sal_Int32 nLength = pInstances->countItems();
    sal_Int32 n = 0;
    bool bFound = false;
    for( ; !bFound  &&  n < nLength; n++ )
    {
        OUString sName;
        getInstanceData( pInstances->getItem( n ), &sName, NULL, NULL, NULL );
        bFound = ( sName == rName );
    }
    return bFound ? ( n - 1 ) : -1;
}

void Model::renameInstance( const rtl::OUString& sFrom,
                            const rtl::OUString& sTo,
                            const rtl::OUString& sURL,
                            sal_Bool bURLOnce )
    throw( RuntimeException )
{
    sal_Int32 nPos = lcl_findInstance( mpInstances, sFrom );
    if( nPos != -1 )
    {
        Sequence<PropertyValue> aSeq = mpInstances->getItem( nPos );
        PropertyValue* pSeq = aSeq.getArray();
        sal_Int32 nLength = aSeq.getLength();

        sal_Int32 nProp = lcl_findProp( pSeq, nLength, OUSTRING("ID") );
        if( nProp == -1 )
        {
            // add name property
            aSeq.realloc( nLength + 1 );
            pSeq = aSeq.getArray();
            pSeq[ nLength ].Name = OUSTRING("ID");
            nProp = nLength;
        }

        // change name
        pSeq[ nProp ].Value <<= sTo;

        // change url
        nProp = lcl_findProp( pSeq, nLength, OUSTRING("URL") );
        if(nProp != -1)
            pSeq[ nProp ].Value <<= sURL;

        // change urlonce
        nProp = lcl_findProp( pSeq, nLength, OUSTRING("URLOnce") );
        if(nProp != -1)
            pSeq[ nProp ].Value <<= bURLOnce;

        // set instance
        mpInstances->setItem( nPos, aSeq );
    }
}

void Model::removeInstance( const rtl::OUString& sName )
    throw( RuntimeException )
{
    sal_Int32 nPos = lcl_findInstance( mpInstances, sName );
    if( nPos != -1 )
        mpInstances->removeItem( mpInstances->getItem( nPos ) );
}

static Reference<XNameContainer> lcl_getModels(
    const Reference<com::sun::star::frame::XModel>& xComponent )
{
    Reference<XNameContainer> xRet;
    Reference<XFormsSupplier> xSupplier( xComponent, UNO_QUERY );
    if( xSupplier.is() )
    {
        xRet = xSupplier->getXForms();
    }
    return xRet;
}

Model::XModel_t Model::newModel( const Reference<com::sun::star::frame::XModel>& xCmp,
                                 const OUString& sName )
    throw( RuntimeException )
{
    Model::XModel_t xModel;
    Reference<XNameContainer> xModels = lcl_getModels( xCmp );
    if( xModels.is()
        && ! xModels->hasByName( sName ) )
    {
        Model* pModel = new Model();
        xModel.set( pModel );

        pModel->setID( sName );
        pModel->newInstance( OUString(), OUString(), sal_False );
        pModel->initialize();
        xModels->insertByName( sName, makeAny( xModel ) );
    }

    return xModel;
}

void Model::renameModel( const Reference<com::sun::star::frame::XModel>& xCmp,
                         const OUString& sFrom,
                         const OUString& sTo )
    throw( RuntimeException )
{
    Reference<XNameContainer> xModels = lcl_getModels( xCmp );
    if( xModels.is()
        && xModels->hasByName( sFrom )
        && ! xModels->hasByName( sTo ) )
    {
        Reference<XModel> xModel( xModels->getByName( sFrom ), UNO_QUERY );
        xModel->setID( sTo );
        xModels->insertByName( sTo, makeAny( xModel ) );
        xModels->removeByName( sFrom );
    }
}

void Model::removeModel( const Reference<com::sun::star::frame::XModel>& xCmp,
                         const OUString& sName )
    throw( RuntimeException )
{
    Reference<XNameContainer> xModels = lcl_getModels( xCmp );
    if( xModels.is()
        && xModels->hasByName( sName ) )
    {
        xModels->removeByName( sName );
    }
}

Model::XNode_t Model::createElement( const XNode_t& xParent,
                                     const OUString& sName )
    throw( RuntimeException )
{
    Reference<XNode> xNode;
    if( xParent.is()
        && isValidXMLName( sName ) )
    {
        // TODO: implement proper namespace handling
        xNode.set( xParent->getOwnerDocument()->createElement( sName ),
                   UNO_QUERY );
    }
    return xNode;
}

Model::XNode_t Model::createAttribute( const XNode_t& xParent,
                                       const OUString& sName )
    throw( RuntimeException )
{
    Reference<XNode> xNode;
    Reference<XElement> xElement( xParent, UNO_QUERY );
    if( xParent.is()
        && xElement.is()
        && isValidXMLName( sName ) )
    {
        // handle case where attribute already exists
        sal_Int32 nCount = 0;
        OUString sUniqueName = sName;
        while( xElement->hasAttribute( sUniqueName ) )
        {
            nCount++;
            sUniqueName = sName + OUString::valueOf( nCount );
        }

        // TODO: implement proper namespace handling
        xNode.set( xParent->getOwnerDocument()->createAttribute( sUniqueName ),
                   UNO_QUERY );
    }
    return xNode;
}

Model::XNode_t Model::renameNode( const XNode_t& xNode,
                                  const rtl::OUString& sName )
    throw( RuntimeException )
{
    // early out if we don't have to change the name
    if( xNode->getNodeName() == sName )
        return xNode;

    // refuse to change name if its an attribute, and the name is already used
    if( xNode->getNodeType() == NodeType_ATTRIBUTE_NODE
        && xNode->getParentNode().is()
        && Reference<XElement>(xNode->getParentNode(), UNO_QUERY_THROW)->hasAttribute( sName ) )
        return xNode;

    // note old binding expression so we can adjust bindings below
    OUString sOldDefaultBindingExpression =
        getDefaultBindingExpressionForNode( xNode );

    Reference<XDocument> xDoc = xNode->getOwnerDocument();
    Reference<XNode> xNew;
    if( xNode->getNodeType() == NodeType_ELEMENT_NODE )
    {
        Reference<XElement> xElem = xDoc->createElement( sName );
        xNew.set( xElem, UNO_QUERY );

        // iterate over all attributes and append them to the new element
        Reference<XElement> xOldElem( xNode, UNO_QUERY );
        OSL_ENSURE( xNode.is(), "no element?" );

        Reference<XNamedNodeMap> xMap = xNode->getAttributes();
        sal_Int32 nLength = xMap.is() ? xMap->getLength() : 0;
        for( sal_Int32 n = 0; n < nLength; n++ )
        {
            Reference<XAttr> xAttr( xMap->item(n), UNO_QUERY );
            xElem->setAttributeNode( xOldElem->removeAttributeNode( xAttr ) );
        }

        // iterate over all children and append them to the new element
        for( Reference<XNode> xCurrent = xNode->getFirstChild();
             xCurrent.is();
             xCurrent = xNode->getFirstChild() )
        {
            xNew->appendChild( xNode->removeChild( xCurrent ) );
        }

        xNode->getParentNode()->replaceChild( xNew, xNode );
    }
    else if( xNode->getNodeType() == NodeType_ATTRIBUTE_NODE )
    {
        // create new attribute
        Reference<XAttr> xAttr = xDoc->createAttribute( sName );
        xAttr->setValue( xNode->getNodeValue() );

        // replace node
        Reference<XNode> xParent = xNode->getParentNode();
        xParent->removeChild( xNode );
        xNew = xParent->appendChild( Reference<XNode>( xAttr, UNO_QUERY ) );
    }
    else
    {
        OSL_FAIL( "can't rename this node type" );
    }

    // adjust bindings (if necessary):
    if( xNew.is() )
    {
        // iterate over bindings and replace default expressions
        OUString sNewDefaultBindingExpression =
            getDefaultBindingExpressionForNode( xNew );
        for( sal_Int32 n = 0; n < mpBindings->countItems(); n++ )
        {
            Binding* pBinding = Binding::getBinding(
                mpBindings->Collection<XPropertySet_t>::getItem( n ) );

            if( pBinding->getBindingExpression()
                    == sOldDefaultBindingExpression )
                pBinding->setBindingExpression( sNewDefaultBindingExpression );
        }
    }

    // return node; return old node if renaming failed
    return xNew.is() ? xNew : xNode;
}

Model::XPropertySet_t Model::getBindingForNode( const XNode_t& xNode,
                                                sal_Bool bCreate )
    throw( RuntimeException )
{
    OSL_ENSURE( xNode.is(), "no node?" );

    // We will iterate over all bindings and determine the
    // appropriateness of the respective binding for this node. The
    // best one will be used. If we don't find any and bCreate is set,
    // then we will create a suitable binding.
    Binding* pBestBinding = NULL;
    sal_Int32 nBestScore = 0;

    for( sal_Int32 n = 0; n < mpBindings->countItems(); n++ )
    {
        Binding* pBinding = Binding::getBinding(
            mpBindings->Collection<XPropertySet_t>::getItem( n ) );

        OSL_ENSURE( pBinding != NULL, "no binding?" );
        Reference<XNodeList> xNodeList = pBinding->getXNodeList();

        sal_Int32 nNodes = xNodeList.is() ? xNodeList->getLength() : 0;
        if( nNodes > 0  &&  xNodeList->item( 0 ) == xNode )
        {
            // allright, we found a suitable node. Let's determine how
            // well it fits. Score:
            // - bind to exactly this node is better than whole nodeset
            // - simple binding expressions is better than complex ones
            sal_Int32 nScore = 0;
            if( nNodes == 1 )
                nScore ++;
            if( pBinding->isSimpleBindingExpression() )
                nScore ++;

            // if we found a better binding, remember it
            if( nScore > nBestScore )
            {
                pBestBinding = pBinding;
                nBestScore = nScore;
            }
        }
    }

    // create binding, if none was found and bCreate is set
    OSL_ENSURE( ( nBestScore == 0 ) == ( pBestBinding == NULL ),
                "score != binding?" );
    if( bCreate  &&  pBestBinding == NULL )
    {
        pBestBinding = new Binding();
        pBestBinding->setBindingExpression(
            getDefaultBindingExpressionForNode( xNode ) );
        mpBindings->addItem( pBestBinding );
    }

    return pBestBinding;
}

void Model::removeBindingForNode( const XNode_t& )
    throw( RuntimeException )
{
    // determine whether suitable binding is still used
}

static OUString lcl_serializeForDisplay( const Reference< XAttr >& _rxAttrNode )
{
    ::rtl::OUString sResult;
    OSL_ENSURE( _rxAttrNode.is(), "lcl_serializeForDisplay( attr ): invalid argument!" );
    if ( _rxAttrNode.is() )
    {
        ::rtl::OUStringBuffer aBuffer;
        aBuffer.append( _rxAttrNode->getName() );
        aBuffer.appendAscii( "=" );
        ::rtl::OUString sValue = _rxAttrNode->getValue();
        sal_Unicode nQuote = '"';
        if ( sValue.indexOf( nQuote ) >= 0 )
            nQuote = '\'';
        aBuffer.append( nQuote );
        aBuffer.append( sValue );
        aBuffer.append( nQuote );
        aBuffer.append( (sal_Unicode)' ' );
        sResult = aBuffer.makeStringAndClear();
    }
    return sResult;
}

static OUString lcl_serializeForDisplay( const Reference<XNodeList>& xNodes )
{
    ::rtl::OUString sResult;

    // create document fragment
    Reference<XDocument> xDocument( getDocumentBuilder()->newDocument() );
    Reference<XDocumentFragment> xFragment(
        xDocument->createDocumentFragment() );
    Reference<XNode> xNode( xFragment, UNO_QUERY );
    OSL_ENSURE( xFragment.is(), "xFragment" );
    OSL_ENSURE( xNode.is(), "xNode" );

    sal_Int32 nAttributeNodes = 0;

    // attach nodelist to fragment
    sal_Int32 nLength = xNodes->getLength();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        Reference<XNode> xCurrent = xNodes->item( i );

        switch ( xCurrent->getNodeType() )
        {
        case NodeType_DOCUMENT_NODE:
            // special-case documents: use top-level element instead
            xCurrent = xCurrent->getFirstChild();
            break;
        case NodeType_ATTRIBUTE_NODE:
        {
            Reference< XAttr > xAttr( xCurrent, UNO_QUERY );
            if ( xAttr.is() )
            {
                sResult += lcl_serializeForDisplay( xAttr );
                ++nAttributeNodes;
            }
        }
        continue;

        default:
            break;
        }

        // append node
        xNode->appendChild( xDocument->importNode( xCurrent, sal_True ) );
    }
    OSL_ENSURE( ( nAttributeNodes == 0 ) || ( nAttributeNodes == nLength ),
        "lcl_serializeForDisplay: mixed attribute and non-attribute nodes?" );
    if ( nAttributeNodes )
        // had only attribute nodes
        return sResult;

    // serialize fragment
    CSerializationAppXML aSerialization;
    aSerialization.setSource( xFragment );
    aSerialization.serialize();

    // copy stream into buffer
    Reference<XTextInputStream> xTextInputStream(
        createInstance( OUSTRING("com.sun.star.io.TextInputStream") ),
        UNO_QUERY );
    Reference<XActiveDataSink>( xTextInputStream, UNO_QUERY_THROW )
        ->setInputStream( aSerialization.getInputStream() );

    /* WORK AROUND for problem in serialization: currently, multiple
      XML delarations (<?xml...?>) are being written out and we don't
      want them. When this is fixed, the code below is nice and
      simple. The current code filters out the declarations.
    OUString sResult = xTextInputStream->readString( Sequence<sal_Unicode>(),
                                                     sal_True );
    */

    // well, the serialization prepends XML header(s) that we need to
    // remove first.
    OUStringBuffer aBuffer;
    while( ! xTextInputStream->isEOF() )
    {
        OUString sLine = xTextInputStream->readLine();
        if( !sLine.isEmpty()
            && sLine.compareToAscii( "<?xml", 5 ) != 0 )
        {
            aBuffer.append( sLine );
            aBuffer.append( sal_Unicode('\n') );
        }
    }
    sResult = aBuffer.makeStringAndClear();

    return sResult;
}

static OUString lcl_serializeForDisplay( const Reference<XXPathObject>& xResult )
{
    // error handling first
    if( ! xResult.is() )
        return getResource( RID_STR_XFORMS_CANT_EVALUATE );


    // TODO: localize
    OUStringBuffer aBuffer;

    switch( xResult->getObjectType() )
    {
    case XPathObjectType_XPATH_BOOLEAN:
        aBuffer.append( xResult->getBoolean()
                        ? OUSTRING("true")
                        : OUSTRING("false") );
        break;

    case XPathObjectType_XPATH_STRING:
        aBuffer.append( sal_Unicode('"') );
        aBuffer.append( xResult->getString() );
        aBuffer.append( sal_Unicode('"') );
        break;

    case XPathObjectType_XPATH_NODESET:
        aBuffer.append( lcl_serializeForDisplay( xResult->getNodeList() ) );
        break;

    case XPathObjectType_XPATH_NUMBER:
        aBuffer.append( xResult->getDouble() );
        break;

    case XPathObjectType_XPATH_UNDEFINED:
    case XPathObjectType_XPATH_POINT:
    case XPathObjectType_XPATH_RANGE:
    case XPathObjectType_XPATH_LOCATIONSET:
    case XPathObjectType_XPATH_USERS:
    case XPathObjectType_XPATH_XSLT_TREE:
    default:
        // TODO: localized error message?
        break;
    }

    return aBuffer.makeStringAndClear();
}

OUString Model::getResultForExpression(
    const XPropertySet_t& xBinding,
    sal_Bool bIsBindingExpression,
    const OUString& sExpression )
    throw( RuntimeException )
{
    Binding* pBinding = Binding::getBinding( xBinding );
    if( pBinding == NULL )
        throw RuntimeException();

    // prepare & evaluate expression
    OUStringBuffer aBuffer;
    ComputedExpression aExpression;
    aExpression.setExpression( sExpression );
    if( bIsBindingExpression )
    {
        // binding: use binding context and evaluation
        aExpression.evaluate( pBinding->getEvaluationContext() );
        aBuffer.append( lcl_serializeForDisplay( aExpression.getXPath() ) );
    }
    else
    {
        // MIP (not binding): iterate over bindings contexts
        std::vector<EvaluationContext> aContext =
            pBinding->getMIPEvaluationContexts();
        for( std::vector<EvaluationContext>::iterator aIter = aContext.begin();
             aIter != aContext.end();
             ++aIter )
        {
            aExpression.evaluate( *aIter );
            aBuffer.append( lcl_serializeForDisplay(aExpression.getXPath()) );
            aBuffer.append( sal_Unicode('\n') );
        }
    }
    return aBuffer.makeStringAndClear();
}

sal_Bool Model::isValidXMLName( const OUString& sName )
    throw( RuntimeException )
{
    return isValidQName( sName, NULL );
}

sal_Bool Model::isValidPrefixName( const OUString& sName )
    throw( RuntimeException )
{
    return ::isValidPrefixName( sName, NULL );
}

void Model::setNodeValue(
    const XNode_t& xNode,
    const rtl::OUString& sValue )
    throw( RuntimeException )
{
    setSimpleContent( xNode, sValue );
}


//
// helper functions from model_helper.hxx
//

void xforms::getInstanceData(
    const Sequence<PropertyValue>& aValues,
    OUString* pID,
    Reference<XDocument>* pInstance,
    OUString* pURL,
    bool* pURLOnce )
{
    sal_Int32 nValues = aValues.getLength();
    const PropertyValue* pValues = aValues.getConstArray();
    for( sal_Int32 n = 0; n < nValues; n++ )
    {
        const PropertyValue& rValue = pValues[n];
#define PROP(NAME) \
        if( p##NAME != NULL && \
            rValue.Name == #NAME ) \
            rValue.Value >>= (*p##NAME)
        PROP(ID);
        PROP(Instance);
        PROP(URL);
        PROP(URLOnce);
#undef PROP
    }
}

void xforms::setInstanceData(
    Sequence<PropertyValue>& aSequence,
    const OUString* _pID,
    const Reference<XDocument>* _pInstance,
    const OUString* _pURL,
    const bool* _pURLOnce )
{
    // get old instance data
    OUString sID;
    Reference<XDocument> xInstance;
    OUString sURL;
    bool bURLOnce = false;
    getInstanceData( aSequence, &sID, &xInstance, &sURL, &bURLOnce );
    const OUString* pID = !sID.isEmpty() ? &sID : NULL;
    const Reference<XDocument>* pInstance = xInstance.is() ? &xInstance : NULL;
    const OUString* pURL = !sURL.isEmpty() ? &sURL : NULL;
    const bool* pURLOnce = ( bURLOnce && pURL != NULL ) ? &bURLOnce : NULL;

    // determine new instance data
#define PROP(NAME) if( _p##NAME != NULL ) p##NAME = _p##NAME
    PROP(ID);
    PROP(Instance);
    PROP(URL);
    PROP(URLOnce);
#undef PROP

    // count # of values we want to set
    sal_Int32 nCount = 0;
#define PROP(NAME) if( p##NAME != NULL ) nCount++
    PROP(ID);
    PROP(Instance);
    PROP(URL);
    PROP(URLOnce);
#undef PROP

    // realloc sequence and enter values;
    aSequence.realloc( nCount );
    PropertyValue* pSequence = aSequence.getArray();
    sal_Int32 nIndex = 0;
#define PROP(NAME) \
    if( p##NAME != NULL ) \
    { \
        pSequence[ nIndex ].Name = OUSTRING(#NAME); \
        pSequence[ nIndex ].Value <<= *p##NAME; \
        nIndex++; \
    }
    PROP(ID);
    PROP(Instance);
    PROP(URL);
    PROP(URLOnce);
#undef PROP
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
