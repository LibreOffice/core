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
#include "unohelper.hxx"
#include "binding.hxx"
#include "submission.hxx"
#include "mip.hxx"
#include "evaluationcontext.hxx"
#include "xmlhelper.hxx"
#include "datatyperepository.hxx"
#include "NameContainer.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>

#include <comphelper/propertysetinfo.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <algorithm>

// UNO classes
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XCharacterData.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>


using com::sun::star::lang::XUnoTunnel;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::PropertyValue;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::ucb::XSimpleFileAccess3;
using com::sun::star::ucb::SimpleFileAccess;
using com::sun::star::io::XInputStream;

using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace xforms;


#if OSL_DEBUG_LEVEL > 0
#define DBG_INVARIANT_TYPE(TYPE) class DBG_##TYPE { const TYPE* mpT; void check() { mpT->dbg_assertInvariant(); } public: DBG_##TYPE(const TYPE* pT) : mpT(pT) { check(); } ~DBG_##TYPE() { check(); } } _DBG_##TYPE(this);

#define DBG_INVARIANT() DBG_INVARIANT_TYPE(Model)
#else
#define DBG_INVARIANT()
#endif


// The Model


void Model::ensureAtLeastOneInstance()
{
    if( ! mxInstances->hasItems() )
    {
        // create a default instance
        newInstance( OUString(), OUString(), true );
    }
}


/** Model default constructor; create empty model */
Model::Model() :
    msID(),
    mxInstances( new InstanceCollection ),
    mxNamespaces( new NameContainer<OUString>() ),
    mbInitialized( false ),
    mbExternalData( true )
{
    initializePropertySet();

    // initialize bindings collections
    // (not in initializer list to avoid use of incomplete 'this')
    mxBindings = new BindingCollection( this );
    mxSubmissions = new SubmissionCollection( this );

    // invariant only holds after construction
    DBG_INVARIANT();
}

Model::~Model() throw()
{
}

static Model* lcl_getModel( const Reference<XUnoTunnel>& xTunnel )
{
    Model* pModel = nullptr;
    if( xTunnel.is() )
        pModel = reinterpret_cast<Model*>(
            xTunnel->getSomething( Model::getUnoTunnelID() ) );
    return pModel;
}

Model* Model::getModel( const Reference<XModel>& xModel )
{
    return lcl_getModel( Reference<XUnoTunnel>( xModel, UNO_QUERY ) );
}

EvaluationContext Model::getEvaluationContext()
{
    // the default context is the top-level element node. A default
    // node (instanceData' is inserted when there is no default node
    Reference<XDocument> xInstance = getDefaultInstance();
    Reference<XNode> xElement( xInstance->getDocumentElement(), UNO_QUERY );

    // no element found? Then insert default element 'instanceData'
    if( ! xElement.is() )
    {
        xElement.set( xInstance->createElement( "instanceData" ), UNO_QUERY_THROW );
        xInstance->appendChild( xElement );
    }

    OSL_ENSURE( xElement.is() &&
                xElement->getNodeType() == NodeType_ELEMENT_NODE,
                "no element in evaluation context" );

    return EvaluationContext( xElement, this, mxNamespaces );
}


css::uno::Sequence<sal_Int8> Model::getUnoTunnelID()
{
    static cppu::OImplementationId aImplementationId;
    return aImplementationId.getImplementationId();
}


void Model::setForeignSchema( const css::uno::Reference<css::xml::dom::XDocument>& rDocument )
{
    mxForeignSchema = rDocument;
}


void Model::setSchemaRef( const OUString& rSchemaRef )
{
    msSchemaRef = rSchemaRef;
}


void Model::setNamespaces( const css::uno::Reference<css::container::XNameContainer>& rNamespaces )
{
    if( rNamespaces.is() )
        mxNamespaces = rNamespaces;
}


void Model::setExternalData( bool _bData )
{
    mbExternalData = _bData;
}

#if OSL_DEBUG_LEVEL > 0
void Model::dbg_assertInvariant() const
{
    assert(mxInstances && "no instances found");
    assert(mxBindings && "no bindings element");
    assert(mxSubmissions && "no submissions element");
}
#endif


// MIP management
void Model::addMIP( void* pTag, const XNode_t& xNode, const MIP& rMIP )
{
    OSL_ENSURE( pTag != nullptr, "empty tag?" );
    OSL_ENSURE( xNode.is(), "no node" );

    MIPs_t::value_type aValue( xNode, ::std::pair<void*,MIP>( pTag, rMIP ) );
    maMIPs.insert( aValue );
}

void Model::removeMIPs( void* pTag )
{
    OSL_ENSURE( pTag != nullptr, "empty tag?" );

    for( MIPs_t::iterator aIter = maMIPs.begin();
         aIter != maMIPs.end(); )
    {
        if( aIter->second.first == pTag )
        {
            MIPs_t::iterator next( aIter ); ++next;
            maMIPs.erase( aIter );
            aIter = next;
        }
        else
            ++aIter;
    }
}

MIP Model::queryMIP( const XNode_t& xNode ) const
{
    // travel up inheritance chain and inherit MIPs
    MIP aRet;
    for( XNode_t xCurrent = xNode;
         xCurrent.is();
         xCurrent = xCurrent->getParentNode() )
    {
        // iterate over all MIPs for this node, and join MIPs
        MIP aMIP;
        MIPs_t::const_iterator aEnd = maMIPs.upper_bound( xCurrent );
        MIPs_t::const_iterator aIter = maMIPs.lower_bound( xCurrent );
        for( ; aIter != aEnd; ++aIter )
          aMIP.join( aIter->second.second );

        // inherit from current node (or set if we are at the start node)
        if( xCurrent == xNode )
            aRet = aMIP;
        else
            aRet.inherit( aMIP );
    }

    return aRet;
}


void Model::rebind()
{
    OSL_ENSURE( mxBindings, "bindings?" );

    // iterate over all bindings and call update
    sal_Int32 nCount = mxBindings->countItems();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        Binding* pBind = Binding::getBinding( mxBindings->Collection<XPropertySet_t>::getItem( i ) );
        OSL_ENSURE( pBind != nullptr, "binding?" );
        pBind->update();
    }
}


void Model::deferNotifications( bool bDefer )
{
    // iterate over all bindings and defer notifications
    sal_Int32 nCount = mxBindings->countItems();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        Binding* pBind = Binding::getBinding( mxBindings->Collection<XPropertySet_t>::getItem( i ) );
        OSL_ENSURE( pBind != nullptr, "binding?" );
        pBind->deferNotifications( bDefer );
    }
}


bool Model::setSimpleContent( const XNode_t& xConstNode,
                              const OUString& sValue )
{
    OSL_ENSURE( xConstNode.is(), "need node to set data" );

    bool bRet = false;
    if( xConstNode.is() )
    {
        // non-const node reference so we can assign children (if necessary)
        XNode_t xNode( xConstNode );

        switch( xNode->getNodeType() )
        {
        case NodeType_ELEMENT_NODE:
        {
            // find first text node child
            Reference<XNode> xChild;
            for( xChild = xNode->getFirstChild();
                 xChild.is() && xChild->getNodeType() != NodeType_TEXT_NODE;
                 xChild = xChild->getNextSibling() )
                ; // empty loop; only find first text node child

            // create text node, if none is found
            if( ! xChild.is() )
            {
                xChild.set(
                    xNode->getOwnerDocument()->createTextNode( OUString() ),
                    UNO_QUERY_THROW );
                xNode->appendChild( xChild );
            }
            xNode = xChild;

            OSL_ENSURE( xNode.is() &&
                        xNode->getNodeType() == NodeType_TEXT_NODE,
                        "text node creation failed?" );
            SAL_FALLTHROUGH; // continue as with text node:
        }

        case NodeType_TEXT_NODE:
        case NodeType_ATTRIBUTE_NODE:
        {
            // set the node value (defer notifications)
            if( xNode->getNodeValue() != sValue )
            {
                deferNotifications( true );
                xNode->setNodeValue( sValue );
                deferNotifications( false );
            }
            bRet = true;
        }
        break;

        default:
        {
            OSL_FAIL( "bound to unknown node type?" );
        }
        break;

        }
    }
    return bRet;
}

void Model::loadInstance( sal_Int32 nInstance )
{
    Sequence<PropertyValue> aSequence = mxInstances->getItem( nInstance );

    // find URL from instance
    OUString sURL;
    bool bOnce = false;
    getInstanceData( aSequence, nullptr, nullptr, &sURL, &bOnce );

    // if we have a URL, load the document and set it into the instance
    if( !sURL.isEmpty() )
    {
        try
        {
            Reference<XInputStream> xInput =
                Reference<XSimpleFileAccess3>( SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) )->openFileRead( sURL );
            if( xInput.is() )
            {
                Reference<XDocument> xInstance =
                    getDocumentBuilder()->parse( xInput );
                if( xInstance.is() )
                {
                    OUString sEmpty;
                    setInstanceData( aSequence, nullptr, &xInstance,
                                     bOnce ? &sEmpty : &sURL, nullptr);
                    mxInstances->setItem( nInstance, aSequence );
                }
            }
        }
        catch( const Exception& )
        {
            // couldn't load the instance -> ignore!
        }
    }
}

void Model::loadInstances()
{
    // iterate over instance array to get PropertyValue-Sequence
    const sal_Int32 nInstances = mxInstances->countItems();
    for( sal_Int32 nInstance = 0; nInstance < nInstances; nInstance++ )
    {
        loadInstance( nInstance );
    }
}


bool Model::isValid() const
{
    bool bValid = true;
    sal_Int32 nCount = mxBindings->countItems();
    for( sal_Int32 i = 0; bValid && i < nCount; i++ )
    {
        Binding* pBind = Binding::getBinding( mxBindings->Collection<XPropertySet_t>::getItem( i ) );
        OSL_ENSURE( pBind != nullptr, "binding?" );
        bValid = pBind->isValid();
    }
    return bValid;
}


// implement xforms::XModel


OUString Model::getID()
{
    DBG_INVARIANT();
    return msID;
}

void Model::setID( const OUString& sID )
{
    DBG_INVARIANT();
    msID = sID;
}

void Model::initialize()
{
    DBG_ASSERT( ! mbInitialized, "model already initialized" );

    // load instances
    loadInstances();

    // let's pretend we're initialized and rebind all bindings
    mbInitialized = true;
    rebind();
}

void Model::rebuild()
{
    if( ! mbInitialized )
        initialize();
    else
        rebind();
}

void Model::recalculate()
{
    rebind();
}

void Model::revalidate()
{
    // do nothing. We don't validate anyways!
}

void Model::refresh()
{
    rebind();
}


void SAL_CALL Model::submitWithInteraction(
    const OUString& sID,
    const css::uno::Reference<css::task::XInteractionHandler>& _rxHandler )
{
    DBG_INVARIANT();

    if( mxSubmissions->hasItem( sID ) )
    {
        Submission* pSubmission =
            Submission::getSubmission( mxSubmissions->getItem( sID ) );
        OSL_ENSURE( pSubmission != nullptr, "no submission?" );
        OSL_ENSURE( pSubmission->getModel() == Reference<XModel>( this ),
                    "wrong model" );

        // submit. All exceptions are allowed to leave.
        pSubmission->submitWithInteraction( _rxHandler );
    }
}

void Model::submit( const OUString& sID )
{
    submitWithInteraction( sID, nullptr );
}

css::uno::Reference<css::xforms::XDataTypeRepository> SAL_CALL Model::getDataTypeRepository(  )
{
    if ( !mxDataTypes.is() )
        mxDataTypes = new ODataTypeRepository;

    return mxDataTypes;
}


// instance management


css::uno::Reference<css::container::XSet> Model::getInstances()
{
    return mxInstances.get();
}

css::uno::Reference<css::xml::dom::XDocument> Model::getInstanceDocument( const OUString& rName )
{
    ensureAtLeastOneInstance();
    Reference<XDocument> aInstance;
    sal_Int32 nInstance = lcl_findInstance( mxInstances.get(), rName );
    if( nInstance != -1 )
        getInstanceData( mxInstances->getItem( nInstance ),
                         nullptr, &aInstance, nullptr, nullptr );
    return aInstance;
}

css::uno::Reference<css::xml::dom::XDocument> SAL_CALL Model::getDefaultInstance()
{
    ensureAtLeastOneInstance();
    DBG_ASSERT( mxInstances->countItems() > 0, "no instance?" );
    Reference<XDocument> aInstance;
    getInstanceData( mxInstances->getItem( 0 ), nullptr, &aInstance, nullptr, nullptr );
    return aInstance;
}


// bindings management


Model::XPropertySet_t SAL_CALL Model::createBinding()
{
    DBG_INVARIANT();
    return new Binding();
}

Model::XPropertySet_t Model::cloneBinding( const XPropertySet_t& xBinding )
{
    DBG_INVARIANT();
    XPropertySet_t xNewBinding = createBinding();
    copy( xBinding, xNewBinding );
    return xNewBinding;
}

Model::XPropertySet_t Model::getBinding( const OUString& sId )
{
    DBG_INVARIANT();
    return mxBindings->hasItem( sId ) ? mxBindings->getItem( sId ) : nullptr;
}

css::uno::Reference<css::container::XSet> Model::getBindings()
{
    DBG_INVARIANT();
    return mxBindings.get();
}


// submission management


css::uno::Reference<css::xforms::XSubmission> Model::createSubmission()
{
    DBG_INVARIANT();
    return new Submission();
}

css::uno::Reference<css::xforms::XSubmission> Model::cloneSubmission(const XPropertySet_t& xSubmission)
{
    DBG_INVARIANT();
    css::uno::Reference<css::xforms::XSubmission> xNewSubmission = createSubmission();
    XPropertySet_t xAsPropertySet( xNewSubmission.get() );
    copy( xSubmission.get(), xAsPropertySet );
    return xNewSubmission;
}

css::uno::Reference<css::xforms::XSubmission> Model::getSubmission( const OUString& sId )
{
    DBG_INVARIANT();
    css::uno::Reference<css::xforms::XSubmission> xSubmission;
    if ( mxSubmissions->hasItem( sId ) )
        xSubmission.set(mxSubmissions->getItem( sId ), css::uno::UNO_QUERY);
    return xSubmission;
}

css::uno::Reference<css::container::XSet> Model::getSubmissions()
{
    DBG_INVARIANT();
    return mxSubmissions.get();
}


// implement XPropertySet & friends


#define HANDLE_ID 0
#define HANDLE_ForeignSchema 3
#define HANDLE_SchemaRef 4
#define HANDLE_Namespaces 5
#define HANDLE_ExternalData 6

#define REGISTER_PROPERTY( property, type )   \
    registerProperty( PROPERTY( property, type ), \
    new DirectPropertyAccessor< Model, type >( this, &Model::set##property, &Model::get##property ) );

#define REGISTER_PROPERTY_API( property, type )   \
    registerProperty( PROPERTY( property, type ), \
    new APIPropertyAccessor< Model, type >( this, &Model::set##property, &Model::get##property ) );

#define REGISTER_BOOL_PROPERTY( property )   \
    registerProperty( PROPERTY( property, sal_Bool ), \
    new BooleanPropertyAccessor< Model >( this, &Model::set##property, &Model::get##property ) );

void Model::initializePropertySet()
{
    REGISTER_PROPERTY_API ( ID,            OUString );
    REGISTER_PROPERTY     ( ForeignSchema, css::uno::Reference<css::xml::dom::XDocument> );
    REGISTER_PROPERTY     ( SchemaRef,     OUString );
    REGISTER_PROPERTY     ( Namespaces,    css::uno::Reference<css::container::XNameContainer> );
    REGISTER_BOOL_PROPERTY( ExternalData );
}

void Model::update()
{
    rebuild();
}


sal_Int64 Model::getSomething( const css::uno::Sequence<sal_Int8>& xId )
{
    return reinterpret_cast<sal_Int64>( ( xId == getUnoTunnelID() ) ? this : nullptr );
}

Sequence<sal_Int8> Model::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

OUString Model::getImplementationName()
{
    return OUString("com.sun.star.form.Model");
}

sal_Bool Model::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> Model::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{"com.sun.star.xforms.Model"};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_Model_get_implementation(css::uno::XComponentContext*,
                                            css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new xforms::Model());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
