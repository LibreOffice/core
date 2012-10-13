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
#include <cppuhelper/typeprovider.hxx>

#include <algorithm>

// UNO classes
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XCharacterData.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>


using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::lang::XUnoTunnel;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::PropertyValue;
using rtl::OUString;
using rtl::OUStringBuffer;
using com::sun::star::beans::PropertyVetoException;
using com::sun::star::beans::UnknownPropertyException;
using com::sun::star::util::VetoException;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::ucb::XSimpleFileAccess2;
using com::sun::star::ucb::SimpleFileAccess;
using com::sun::star::io::XInputStream;

using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace xforms;


#if OSL_DEBUG_LEVEL > 1
#define DBG_INVARIANT_TYPE(TYPE) class DBG_##TYPE { const TYPE* mpT; void check() { mpT->dbg_assertInvariant(); } public: DBG_##TYPE(const TYPE* pT) : mpT(pT) { check(); } ~DBG_##TYPE() { check(); } } _DBG_##TYPE(this);

#define DBG_INVARIANT() DBG_INVARIANT_TYPE(Model)
#else
#define DBG_INVARIANT_TYPE(TYPE)
#define DBG_INVARIANT()
#endif



//
// The Model
//

void Model::ensureAtLeastOneInstance()
{
    if( ! mpInstances->hasItems() )
    {
        // create a default instance
        newInstance( OUString(), OUString(), true );
    }
}



/** Model default constructor; create empty model */
Model::Model() :
    msID(),
    mpBindings( NULL ),
    mpSubmissions( NULL ),
    mpInstances( new InstanceCollection ),
    mxNamespaces( new NameContainer<OUString>() ),
    mxBindings( mpBindings ),
    mxSubmissions( mpSubmissions ),
    mxInstances( mpInstances ),
    mbInitialized( false ),
    mbExternalData( true )
{
    initializePropertySet();

    // initialize bindings collections
    // (not in initializer list to avoid use of incomplete 'this')
    mpBindings = new BindingCollection( this );
    mxBindings = mpBindings;

    mpSubmissions = new SubmissionCollection( this );
    mxSubmissions = mpSubmissions;

    // invariant only holds after construction
    DBG_INVARIANT();
}

Model::~Model() throw()
{
    // give up bindings & submissions; the mxBindings/mxSubmissions
    // references will then delete them
    mpBindings = NULL;
    mpSubmissions = NULL;
}

static Model* lcl_getModel( const Reference<XUnoTunnel>& xTunnel )
{
    Model* pModel = NULL;
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
        xElement = Reference<XNode>(
                       xInstance->createElement( OUSTRING("instanceData") ),
                       UNO_QUERY_THROW );
        Reference<XNode>( xInstance, UNO_QUERY_THROW)->appendChild( xElement );
    }

    OSL_ENSURE( xElement.is() &&
                xElement->getNodeType() == NodeType_ELEMENT_NODE,
                "no element in evaluation context" );

    return EvaluationContext( xElement, this, mxNamespaces, 0, 1 );
}


Model::IntSequence_t Model::getUnoTunnelID()
{
    static cppu::OImplementationId aImplementationId;
    return aImplementationId.getImplementationId();
}

Model::XDocument_t Model::getForeignSchema() const
{
    return mxForeignSchema;
}

void Model::setForeignSchema( const XDocument_t& rDocument )
{
    mxForeignSchema = rDocument;
}

rtl::OUString Model::getSchemaRef() const
{
    return msSchemaRef;
}

void Model::setSchemaRef( const rtl::OUString& rSchemaRef )
{
    msSchemaRef = rSchemaRef;
}

Model::XNameContainer_t Model::getNamespaces() const
{
    return mxNamespaces;
}

void Model::setNamespaces( const XNameContainer_t& rNamespaces )
{
    if( rNamespaces.is() )
        mxNamespaces = rNamespaces;
}

bool Model::getExternalData() const
{
    return mbExternalData;
}

void Model::setExternalData( bool _bData )
{
    mbExternalData = _bData;
}

#if OSL_DEBUG_LEVEL > 1
void Model::dbg_assertInvariant() const
{
    OSL_ENSURE( mpInstances != NULL, "no instances found" );
    OSL_ENSURE( mxInstances.is(), "No instance container!" );

    OSL_ENSURE( mpBindings != NULL, "no bindings element" );
    OSL_ENSURE( mxBindings.is(), "No Bindings container" );

    OSL_ENSURE( mpSubmissions != NULL, "no submissions element" );
    OSL_ENSURE( mxSubmissions.is(), "No Submission container" );
}
#endif


//
// MIP managment
//

void Model::addMIP( void* pTag, const XNode_t& xNode, const MIP& rMIP )
{
    OSL_ENSURE( pTag != NULL, "empty tag?" );
    OSL_ENSURE( xNode.is(), "no node" );

    MIPs_t::value_type aValue( xNode, ::std::pair<void*,MIP>( pTag, rMIP ) );
    maMIPs.insert( aValue );
}

void Model::removeMIPs( void* pTag )
{
    OSL_ENSURE( pTag != NULL, "empty tag?" );

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
    OSL_ENSURE( mpBindings != NULL, "bindings?" );

    // iterate over all bindings and call update
    sal_Int32 nCount = mpBindings->countItems();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        Binding* pBind = Binding::getBinding( mpBindings->Collection<XPropertySet_t>::getItem( i ) );
        OSL_ENSURE( pBind != NULL, "binding?" );
        pBind->update();
    }
}



void Model::deferNotifications( bool bDefer )
{
    // iterate over all bindings and defer notifications
    sal_Int32 nCount = mpBindings->countItems();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        Binding* pBind = Binding::getBinding( mpBindings->Collection<XPropertySet_t>::getItem( i ) );
        OSL_ENSURE( pBind != NULL, "binding?" );
        pBind->deferNotifications( bDefer );
    }
}


bool Model::setSimpleContent( const XNode_t& xConstNode,
                              const rtl::OUString& sValue )
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
                xChild = Reference<XNode>(
                    xNode->getOwnerDocument()->createTextNode( OUString() ),
                    UNO_QUERY_THROW );
                xNode->appendChild( xChild );
            }
            xNode = xChild;

            OSL_ENSURE( xNode.is() &&
                        xNode->getNodeType() == NodeType_TEXT_NODE,
                        "text node creation failed?" );
        }
        // no break; continue as with text node:

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
    Sequence<PropertyValue> aSequence = mpInstances->getItem( nInstance );

    // find URL from instance
    OUString sURL;
    bool bOnce = false;
    getInstanceData( aSequence, NULL, NULL, &sURL, &bOnce );

    // if we have a URL, load the document and set it into the instance
    if( !sURL.isEmpty() )
    {
        try
        {
            Reference<XInputStream> xInput =
                Reference<XSimpleFileAccess2>( SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) )->openFileRead( sURL );
            if( xInput.is() )
            {
                Reference<XDocument> xInstance =
                    getDocumentBuilder()->parse( xInput );
                if( xInstance.is() )
                {
                    OUString sEmpty;
                    setInstanceData( aSequence, NULL, &xInstance,
                                     bOnce ? &sEmpty : &sURL, NULL);
                    mpInstances->setItem( nInstance, aSequence );
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
    const sal_Int32 nInstances = mpInstances->countItems();
    for( sal_Int32 nInstance = 0; nInstance < nInstances; nInstance++ )
    {
        loadInstance( nInstance );
    }
}

bool Model::isInitialized() const
{
    return mbInitialized;
}

bool Model::isValid() const
{
    bool bValid = true;
    sal_Int32 nCount = mpBindings->countItems();
    for( sal_Int32 i = 0; bValid && i < nCount; i++ )
    {
        Binding* pBind = Binding::getBinding( mpBindings->Collection<XPropertySet_t>::getItem( i ) );
        OSL_ENSURE( pBind != NULL, "binding?" );
        bValid = pBind->isValid();
    }
    return bValid;
}



//
// implement xforms::XModel
//

rtl::OUString Model::getID()
    throw( RuntimeException )
{
    DBG_INVARIANT();
    return msID;
}

void Model::setID( const rtl::OUString& sID )
    throw( RuntimeException )
{
    DBG_INVARIANT();
    msID = sID;
}

void Model::initialize()
    throw( RuntimeException )
{
    DBG_ASSERT( ! mbInitialized, "model already initialized" );

    // load instances
    loadInstances();

    // let's pretend we're initialized and rebind all bindings
    mbInitialized = true;
    rebind();
}

void Model::rebuild()
    throw( RuntimeException )
{
    if( ! mbInitialized )
        initialize();
    else
        rebind();
}

void Model::recalculate()
    throw( RuntimeException )
{
    rebind();
}

void Model::revalidate()
    throw( RuntimeException )
{
    // do nothing. We don't validate anyways!
}

void Model::refresh()
    throw( RuntimeException )
{
    rebind();
}


void SAL_CALL Model::submitWithInteraction(
    const rtl::OUString& sID,
    const XInteractionHandler_t& _rxHandler )
    throw( VetoException,
           WrappedTargetException,
           RuntimeException )
{
    DBG_INVARIANT();

    if( mpSubmissions->hasItem( sID ) )
    {
        Submission* pSubmission =
            Submission::getSubmission( mpSubmissions->getItem( sID ) );
        OSL_ENSURE( pSubmission != NULL, "no submission?" );
        OSL_ENSURE( pSubmission->getModel() == Reference<XModel>( this ),
                    "wrong model" );

        // submit. All exceptions are allowed to leave.
        pSubmission->submitWithInteraction( _rxHandler );
    }
}

void Model::submit( const rtl::OUString& sID )
    throw( VetoException, WrappedTargetException, RuntimeException )
{
    submitWithInteraction( sID, NULL );
}

Model::XDataTypeRepository_t SAL_CALL Model::getDataTypeRepository(  )
    throw( RuntimeException )
{
    if ( !mxDataTypes.is() )
        mxDataTypes = new ODataTypeRepository;

    return mxDataTypes;
}

//
// instance management
//

Model::XSet_t Model::getInstances()
    throw( RuntimeException )
{
    return mxInstances;
}

Model::XDocument_t Model::getInstanceDocument( const rtl::OUString& rName )
    throw( RuntimeException )
{
    ensureAtLeastOneInstance();
    Reference<XDocument> aInstance;
    sal_Int32 nInstance = lcl_findInstance( mpInstances, rName );
    if( nInstance != -1 )
        getInstanceData( mpInstances->getItem( nInstance ),
                         NULL, &aInstance, NULL, NULL );
    return aInstance;
}

Model::XDocument_t SAL_CALL Model::getDefaultInstance()
    throw( RuntimeException )
{
    ensureAtLeastOneInstance();
    DBG_ASSERT( mpInstances->countItems() > 0, "no instance?" );
    Reference<XDocument> aInstance;
    getInstanceData( mpInstances->getItem( 0 ), NULL, &aInstance, NULL, NULL );
    return aInstance;
}



//
// bindings management
//

Model::XPropertySet_t SAL_CALL Model::createBinding()
    throw( RuntimeException )
{
    DBG_INVARIANT();
    return new Binding();
}

Model::XPropertySet_t Model::cloneBinding( const XPropertySet_t& xBinding )
    throw( RuntimeException )
{
    DBG_INVARIANT();
    XPropertySet_t xNewBinding = createBinding();
    copy( xBinding, xNewBinding );
    return xNewBinding;
}

Model::XPropertySet_t Model::getBinding( const rtl::OUString& sId )
    throw( RuntimeException )
{
    DBG_INVARIANT();
    return mpBindings->hasItem( sId ) ? mpBindings->getItem( sId ) : NULL;
}

Model::XSet_t Model::getBindings()
    throw( RuntimeException )
{
    DBG_INVARIANT();
    return mxBindings;
}



//
// submission management
//

Model::XSubmission_t Model::createSubmission()
    throw( RuntimeException )
{
    DBG_INVARIANT();
    return new Submission();
}

Model::XSubmission_t Model::cloneSubmission(const XPropertySet_t& xSubmission)
    throw( RuntimeException )
{
    DBG_INVARIANT();
    XSubmission_t xNewSubmission = createSubmission();
    XPropertySet_t xAsPropertySet( xNewSubmission.get() );
    copy( xSubmission.get(), xAsPropertySet );
    return xNewSubmission;
}

Model::XSubmission_t Model::getSubmission( const rtl::OUString& sId )
    throw( RuntimeException )
{
    DBG_INVARIANT();
    XSubmission_t xSubmission;
    if ( mpSubmissions->hasItem( sId ) )
        xSubmission = xSubmission.query( mpSubmissions->getItem( sId ) );
    return xSubmission;
}

Model::XSet_t Model::getSubmissions()
    throw( RuntimeException )
{
    DBG_INVARIANT();
    return mxSubmissions;
}

//
// implement XPropertySet & friends
//

#define HANDLE_ID 0
#define HANDLE_Instance 1
#define HANDLE_InstanceURL 2
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
    new BooleanPropertyAccessor< Model, bool >( this, &Model::set##property, &Model::get##property ) );

void Model::initializePropertySet()
{
    REGISTER_PROPERTY_API ( ID,            OUString );
    REGISTER_PROPERTY     ( ForeignSchema, XDocument_t );
    REGISTER_PROPERTY     ( SchemaRef,     OUString );
    REGISTER_PROPERTY     ( Namespaces,    XNameContainer_t );
    REGISTER_BOOL_PROPERTY( ExternalData );
}

void Model::update()
    throw( RuntimeException )
{
    rebuild();
}


sal_Int64 Model::getSomething( const IntSequence_t& xId )
    throw( RuntimeException )
{
    return reinterpret_cast<sal_Int64>( ( xId == getUnoTunnelID() ) ? this : NULL );
}

Sequence<sal_Int8> Model::getImplementationId()
    throw( RuntimeException )
{
    return getUnoTunnelID();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
