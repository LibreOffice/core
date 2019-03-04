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
#include <comphelper/processfactory.hxx>

// UNO classes
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/TextInputStream.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xsd/XDataType.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>


using com::sun::star::beans::PropertyValue;
using com::sun::star::io::TextInputStream;
using com::sun::star::io::XTextInputStream2;
using com::sun::star::container::XEnumeration;
using com::sun::star::container::XNameContainer;
using com::sun::star::xforms::XFormsSupplier;

using namespace xforms;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::xpath;


// implement XFormsUIHelper1


OUString Model::getDefaultServiceNameForNode( const css::uno::Reference<css::xml::dom::XNode>& xNode )
{
    // determine service for control. string/text field is default.
    OUString sService = "com.sun.star.form.component.TextField";

    // query repository for suitable type
    OSL_ENSURE( mxDataTypes.is(), "no type repository?" );
    OUString sTypeName = queryMIP( xNode ).getTypeName();
    if( mxDataTypes->hasByName( sTypeName ) )
    {
        OSL_ENSURE( mxDataTypes->getDataType( sTypeName ).is(),
                    "has or has not?" );

        switch( mxDataTypes->getDataType( sTypeName )->getTypeClass() )
        {
        case css::xsd::DataTypeClass::BOOLEAN:
            sService = "com.sun.star.form.component.CheckBox";
            break;
        case css::xsd::DataTypeClass::DOUBLE:
        case css::xsd::DataTypeClass::DECIMAL:
        case css::xsd::DataTypeClass::FLOAT:
            sService = "com.sun.star.form.component.NumericField";
            break;

        case css::xsd::DataTypeClass::STRING:
        case css::xsd::DataTypeClass::DURATION:
        case css::xsd::DataTypeClass::DATETIME:
        case css::xsd::DataTypeClass::TIME:
        case css::xsd::DataTypeClass::DATE:
        case css::xsd::DataTypeClass::gYearMonth:
        case css::xsd::DataTypeClass::gYear:
        case css::xsd::DataTypeClass::gMonthDay:
        case css::xsd::DataTypeClass::gDay:
        case css::xsd::DataTypeClass::gMonth:
        case css::xsd::DataTypeClass::hexBinary:
        case css::xsd::DataTypeClass::base64Binary:
        case css::xsd::DataTypeClass::anyURI:
        case css::xsd::DataTypeClass::QName:
        case css::xsd::DataTypeClass::NOTATION:
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
             xIter != nullptr;
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
        rBuffer.insert( 0, ']' );
        rBuffer.insert( 0, nPosition );
        rBuffer.insert( 0, '[' );
    }
}

static void lcl_OutName( OUStringBuffer& rBuffer,
                  const Reference<XNode>& xNode )
{
    rBuffer.insert( 0, xNode->getNodeName() );
    OUString sPrefix = xNode->getPrefix();
    if( !sPrefix.isEmpty() )
    {
        rBuffer.insert( 0, ':' );
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
        rBuffer.insert( 0, "')" );

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
            getInstanceData( aValues, &sId, &xInstance, nullptr, nullptr );

            // now check whether this was our instance:
            if( xInstance == xDoc )
                sInstanceName = sId;
        }

        rBuffer.insert( 0, sInstanceName );
        rBuffer.insert( 0, "instance('" );
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
        if( !aBuffer.isEmpty() )
            aBuffer.insert( 0, '/' );

        switch( xCurrent->getNodeType() )
        {
        case NodeType_ELEMENT_NODE:
            lcl_OutPosition( aBuffer, xCurrent );
            lcl_OutName( aBuffer, xCurrent );
            break;

        case NodeType_TEXT_NODE:
            lcl_OutPosition( aBuffer, xCurrent );
            aBuffer.insert( 0, "text()" );
            break;

        case NodeType_ATTRIBUTE_NODE:
            lcl_OutName( aBuffer, xCurrent );
            aBuffer.insert( 0, '@' );
            break;

        case NodeType_DOCUMENT_NODE:
            // check for which instance we have
            lcl_OutInstance( aBuffer, xCurrent, this );
            break;

        default:
            // unknown type? fail!
            OSL_FAIL( "unknown node type!" );
            return OUString();
        }
    }

    return aBuffer.makeStringAndClear();
}


OUString Model::getDefaultBindingExpressionForNode( const css::uno::Reference<css::xml::dom::XNode>& xNode )
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
        bWhitespace = ( c == u'\x0009' ||
                        c == u'\x000A' ||
                        c == u'\x000D' ||
                        c == u' ' );
    }
    return bWhitespace;
}

OUString Model::getNodeDisplayName( const css::uno::Reference<css::xml::dom::XNode>& xNode,
                                    sal_Bool bDetail )
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
                aBuffer.append("\"").append(Convert::collapseWhitespace( sContent )).append("\"");
            }
        }
        break;

    case NodeType_ATTRIBUTE_NODE:
        lcl_OutName( aBuffer, xNode );
        aBuffer.insert( 0, '@' );
        break;

    case NodeType_DOCUMENT_NODE:
        if( xNode == getDefaultInstance() )
            aBuffer.append( '/' );
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

OUString Model::getNodeName( const css::uno::Reference<css::xml::dom::XNode>& xNode )
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

OUString Model::getBindingName( const css::uno::Reference< ::css::beans::XPropertySet >& xBinding,
                                sal_Bool /*bDetail*/ )
{
    OUString sID;
    xBinding->getPropertyValue( "BindingID" ) >>= sID;
    OUString sExpression;
    xBinding->getPropertyValue( "BindingExpression" ) >>= sExpression;

    OUString sRet;
    if( !sID.isEmpty() )
    {
        sRet = sID + " (" + sExpression + ") ";
    }
    else
        sRet = sExpression;

    return sRet;
}

OUString Model::getSubmissionName( const css::uno::Reference< ::css::beans::XPropertySet >& xSubmission,
                                   sal_Bool /*bDetail*/ )
{
    OUString sID;
    xSubmission->getPropertyValue( "ID" ) >>= sID;
    return sID;
}

css::uno::Reference< ::css::beans::XPropertySet > Model::cloneBindingAsGhost( const css::uno::Reference< ::css::beans::XPropertySet > &xBinding )
{
    // Create a new binding instance first...
    Binding *pBinding = new Binding();

    // ...and bump up the "deferred notification counter"
    // to prevent this binding from contributing to the
    // MIPs table...
    pBinding->deferNotifications(true);

    // Copy the propertyset and return result...
    XPropertySet_t xNewBinding(pBinding);
    copy( xBinding, xNewBinding );
    return xNewBinding;
}

void Model::removeBindingIfUseless( const css::uno::Reference< ::css::beans::XPropertySet >& xBinding )
{
    Binding* pBinding = Binding::getBinding( xBinding );
    if( pBinding != nullptr )
    {
        if( ! pBinding->isUseful() )
            mxBindings->removeItem( pBinding );
    }
}

css::uno::Reference<css::xml::dom::XDocument> Model::newInstance( const OUString& sName,
                         const OUString& sURL,
                         sal_Bool bURLOnce )
{
    // create a default instance with <instanceData> element
    css::uno::Reference<css::xml::dom::XDocument> xInstance = getDocumentBuilder()->newDocument();
    DBG_ASSERT( xInstance.is(), "failed to create DOM instance" );

    Reference<XNode>( xInstance, UNO_QUERY_THROW )->appendChild(
        Reference<XNode>( xInstance->createElement( "instanceData" ),
                          UNO_QUERY_THROW ) );

    Sequence<PropertyValue> aSequence;
    bool bOnce = bURLOnce; // bool, so we can take address in setInstanceData
    setInstanceData( aSequence, &sName, &xInstance, &sURL, &bOnce );
    sal_Int32 nInstance = mxInstances->addItem( aSequence );
    loadInstance( nInstance );

    return xInstance;
}

static sal_Int32 lcl_findProp( const PropertyValue* pValues,
                        sal_Int32 nLength,
                        const OUString& rName )
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
                                    const OUString& rName )
{
    sal_Int32 nLength = pInstances->countItems();
    sal_Int32 n = 0;
    bool bFound = false;
    for( ; !bFound  &&  n < nLength; n++ )
    {
        OUString sName;
        getInstanceData( pInstances->getItem( n ), &sName, nullptr, nullptr, nullptr );
        bFound = ( sName == rName );
    }
    return bFound ? ( n - 1 ) : -1;
}

void Model::renameInstance( const OUString& sFrom,
                            const OUString& sTo,
                            const OUString& sURL,
                            sal_Bool bURLOnce )
{
    sal_Int32 nPos = lcl_findInstance( mxInstances.get(), sFrom );
    if( nPos != -1 )
    {
        Sequence<PropertyValue> aSeq = mxInstances->getItem( nPos );
        PropertyValue* pSeq = aSeq.getArray();
        sal_Int32 nLength = aSeq.getLength();

        sal_Int32 nProp = lcl_findProp( pSeq, nLength, "ID" );
        if( nProp == -1 )
        {
            // add name property
            aSeq.realloc( nLength + 1 );
            pSeq = aSeq.getArray();
            pSeq[ nLength ].Name = "ID";
            nProp = nLength;
        }

        // change name
        pSeq[ nProp ].Value <<= sTo;

        // change url
        nProp = lcl_findProp( pSeq, nLength, "URL" );
        if(nProp != -1)
            pSeq[ nProp ].Value <<= sURL;

        // change urlonce
        nProp = lcl_findProp( pSeq, nLength, "URLOnce" );
        if(nProp != -1)
            pSeq[ nProp ].Value <<= bURLOnce;

        // set instance
        mxInstances->setItem( nPos, aSeq );
    }
}

void Model::removeInstance( const OUString& sName )
{
    sal_Int32 nPos = lcl_findInstance( mxInstances.get(), sName );
    if( nPos != -1 )
        mxInstances->removeItem( mxInstances->getItem( nPos ) );
}

static Reference<XNameContainer> lcl_getModels(
    const Reference<css::frame::XModel>& xComponent )
{
    Reference<XNameContainer> xRet;
    Reference<XFormsSupplier> xSupplier( xComponent, UNO_QUERY );
    if( xSupplier.is() )
    {
        xRet = xSupplier->getXForms();
    }
    return xRet;
}

css::uno::Reference<css::xforms::XModel> Model::newModel( const Reference<css::frame::XModel>& xCmp,
                                 const OUString& sName )
{
    css::uno::Reference<css::xforms::XModel> xModel;
    Reference<XNameContainer> xModels = lcl_getModels( xCmp );
    if( xModels.is()
        && ! xModels->hasByName( sName ) )
    {
        Model* pModel = new Model();
        xModel.set( pModel );

        pModel->setID( sName );
        pModel->newInstance( OUString(), OUString(), false );
        pModel->initialize();
        xModels->insertByName( sName, makeAny( xModel ) );
    }

    return xModel;
}

void Model::renameModel( const Reference<css::frame::XModel>& xCmp,
                         const OUString& sFrom,
                         const OUString& sTo )
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

void Model::removeModel( const Reference<css::frame::XModel>& xCmp,
                         const OUString& sName )
{
    Reference<XNameContainer> xModels = lcl_getModels( xCmp );
    if( xModels.is()
        && xModels->hasByName( sName ) )
    {
        xModels->removeByName( sName );
    }
}

css::uno::Reference<css::xml::dom::XNode> Model::createElement( const css::uno::Reference<css::xml::dom::XNode>& xParent,
                                     const OUString& sName )
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

css::uno::Reference<css::xml::dom::XNode> Model::createAttribute( const css::uno::Reference<css::xml::dom::XNode>& xParent,
                                       const OUString& sName )
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
            sUniqueName = sName + OUString::number( nCount );
        }

        // TODO: implement proper namespace handling
        xNode.set( xParent->getOwnerDocument()->createAttribute( sUniqueName ),
                   UNO_QUERY );
    }
    return xNode;
}

css::uno::Reference<css::xml::dom::XNode> Model::renameNode( const css::uno::Reference<css::xml::dom::XNode>& xNode,
                                  const OUString& sName )
{
    // early out if we don't have to change the name
    if( xNode->getNodeName() == sName )
        return xNode;

    // refuse to change name if it's an attribute, and the name is already used
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
        xNew = xParent->appendChild( xAttr );
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
        for( sal_Int32 n = 0; n < mxBindings->countItems(); n++ )
        {
            Binding* pBinding = Binding::getBinding(
                mxBindings->Collection<XPropertySet_t>::getItem( n ) );

            if( pBinding->getBindingExpression()
                    == sOldDefaultBindingExpression )
                pBinding->setBindingExpression( sNewDefaultBindingExpression );
        }
    }

    // return node; return old node if renaming failed
    return xNew.is() ? xNew : xNode;
}

css::uno::Reference< ::css::beans::XPropertySet > Model::getBindingForNode( const css::uno::Reference<css::xml::dom::XNode>& xNode,
                                                sal_Bool bCreate )
{
    OSL_ENSURE( xNode.is(), "no node?" );

    // We will iterate over all bindings and determine the
    // appropriateness of the respective binding for this node. The
    // best one will be used. If we don't find any and bCreate is set,
    // then we will create a suitable binding.
    Binding* pBestBinding = nullptr;
    sal_Int32 nBestScore = 0;

    for( sal_Int32 n = 0; n < mxBindings->countItems(); n++ )
    {
        Binding* pBinding = Binding::getBinding(
            mxBindings->Collection<XPropertySet_t>::getItem( n ) );

        OSL_ENSURE( pBinding != nullptr, "no binding?" );
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
    OSL_ENSURE( ( nBestScore == 0 ) == ( pBestBinding == nullptr ),
                "score != binding?" );
    if( bCreate  &&  pBestBinding == nullptr )
    {
        pBestBinding = new Binding();
        pBestBinding->setBindingExpression(
            getDefaultBindingExpressionForNode( xNode ) );
        mxBindings->addItem( pBestBinding );
    }

    return pBestBinding;
}

void Model::removeBindingForNode( const css::uno::Reference<css::xml::dom::XNode>& )
{
    // determine whether suitable binding is still used
}

static OUString lcl_serializeForDisplay( const Reference< XAttr >& _rxAttrNode )
{
    OUString sResult;
    OSL_ENSURE( _rxAttrNode.is(), "lcl_serializeForDisplay( attr ): invalid argument!" );
    if ( _rxAttrNode.is() )
    {
        OUString sValue = _rxAttrNode->getValue();
        sal_Unicode nQuote = '"';
        if ( sValue.indexOf( nQuote ) >= 0 )
            nQuote = '\'';

        sResult = _rxAttrNode->getName() + "=" + OUStringLiteral1(nQuote) + sValue + OUStringLiteral1(nQuote) + " ";
    }
    return sResult;
}

static OUString lcl_serializeForDisplay( const Reference<XNodeList>& xNodes )
{
    OUStringBuffer sResult;

    // create document fragment
    Reference<XDocument> xDocument( getDocumentBuilder()->newDocument() );
    Reference<XDocumentFragment> xFragment(
        xDocument->createDocumentFragment() );
    OSL_ENSURE( xFragment.is(), "xFragment" );

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
                sResult.append(lcl_serializeForDisplay( xAttr ));
                ++nAttributeNodes;
            }
        }
        continue;

        default:
            break;
        }

        // append node
        xFragment->appendChild( xDocument->importNode( xCurrent, true ) );
    }
    OSL_ENSURE( ( nAttributeNodes == 0 ) || ( nAttributeNodes == nLength ),
        "lcl_serializeForDisplay: mixed attribute and non-attribute nodes?" );
    if ( nAttributeNodes )
        // had only attribute nodes
        return sResult.makeStringAndClear();

    // serialize fragment
    CSerializationAppXML aSerialization;
    aSerialization.setSource( xFragment );
    aSerialization.serialize();

    // copy stream into buffer
    Reference<XTextInputStream2> xTextInputStream = TextInputStream::create( comphelper::getProcessComponentContext() );
    xTextInputStream->setInputStream( aSerialization.getInputStream() );

    /* WORK AROUND for problem in serialization: currently, multiple
      XML declarations (<?xml...?>) are being written out and we don't
      want them. When this is fixed, the code below is nice and
      simple. The current code filters out the declarations.
    OUString sResult = xTextInputStream->readString( Sequence<sal_Unicode>(),
                                                     sal_True );
    */

    // well, the serialization prepends XML header(s) that we need to
    // remove first.
    sResult.setLength(0);
    while( ! xTextInputStream->isEOF() )
    {
        OUString sLine = xTextInputStream->readLine();
        if( !sLine.isEmpty()
            && !sLine.startsWith( "<?xml" ) )
        {
            sResult.append( sLine );
            sResult.append( '\n' );
        }
    }

    return sResult.makeStringAndClear();
}

static OUString lcl_serializeForDisplay( const Reference<XXPathObject>& xResult )
{
    // error handling first
    if( ! xResult.is() )
        return getResource( RID_STR_XFORMS_CANT_EVALUATE );

    // TODO: localize
    switch( xResult->getObjectType() )
    {
    case XPathObjectType_XPATH_BOOLEAN:
        return OUString::boolean(xResult->getBoolean());

    case XPathObjectType_XPATH_STRING:
        return "\"" + xResult->getString() + "\"";

    case XPathObjectType_XPATH_NODESET:
        return lcl_serializeForDisplay( xResult->getNodeList() );

    case XPathObjectType_XPATH_NUMBER:
        return OUString::number(xResult->getDouble());

    case XPathObjectType_XPATH_UNDEFINED:
    case XPathObjectType_XPATH_POINT:
    case XPathObjectType_XPATH_RANGE:
    case XPathObjectType_XPATH_LOCATIONSET:
    case XPathObjectType_XPATH_USERS:
    case XPathObjectType_XPATH_XSLT_TREE:
    default:
        // TODO: localized error message?
        return OUString();
    }
}

OUString Model::getResultForExpression(
    const css::uno::Reference< ::css::beans::XPropertySet >& xBinding,
    sal_Bool bIsBindingExpression,
    const OUString& sExpression )
{
    Binding* pBinding = Binding::getBinding( xBinding );
    if( pBinding == nullptr )
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
        for (auto const& elem : aContext)
        {
            aExpression.evaluate(elem);
            aBuffer.append( lcl_serializeForDisplay(aExpression.getXPath()) );
            aBuffer.append( '\n' );
        }
    }
    return aBuffer.makeStringAndClear();
}

sal_Bool Model::isValidXMLName( const OUString& sName )
{
    return isValidQName( sName, nullptr );
}

sal_Bool Model::isValidPrefixName( const OUString& sName )
{
    return ::isValidPrefixName( sName, nullptr );
}

void Model::setNodeValue(
    const css::uno::Reference< ::css::xml::dom::XNode >& xNode,
    const OUString& sValue )
{
    setSimpleContent( xNode, sValue );
}


// helper functions from model_helper.hxx


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
        if( p##NAME != nullptr && \
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
    const OUString* pID = !sID.isEmpty() ? &sID : nullptr;
    const Reference<XDocument>* pInstance = xInstance.is() ? &xInstance : nullptr;
    const OUString* pURL = !sURL.isEmpty() ? &sURL : nullptr;
    const bool* pURLOnce = ( bURLOnce && pURL != nullptr ) ? &bURLOnce : nullptr;

    // determine new instance data
#define PROP(NAME) if( _p##NAME != nullptr ) p##NAME = _p##NAME
    PROP(ID);
    PROP(Instance);
    PROP(URL);
    PROP(URLOnce);
#undef PROP

    // count # of values we want to set
    sal_Int32 nCount = 0;
#define PROP(NAME) if( p##NAME != nullptr ) nCount++
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
    if( p##NAME != nullptr ) \
    { \
        pSequence[ nIndex ].Name = #NAME; \
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
