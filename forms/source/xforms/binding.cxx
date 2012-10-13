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


#include "binding.hxx"

#include "model.hxx"
#include "unohelper.hxx"
#include "NameContainer.hxx"
#include "evaluationcontext.hxx"
#include "convert.hxx"
#include "resourcehelper.hxx"
#include "xmlhelper.hxx"
#include "xformsevent.hxx"

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <tools/diagnose_ex.h>

#include <algorithm>
#include <functional>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <com/sun/star/xml/dom/events/XDocumentEvent.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <comphelper/propertysetinfo.hxx>
#include <unotools/textsearch.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace com::sun::star::xml::xpath;
using namespace com::sun::star::xml::dom::events;

using rtl::OUString;
using rtl::OUStringBuffer;
using std::vector;
using xforms::Binding;
using xforms::MIP;
using xforms::Model;
using xforms::getResource;
using xforms::EvaluationContext;
using com::sun::star::beans::PropertyVetoException;
using com::sun::star::beans::UnknownPropertyException;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XSet;
using com::sun::star::container::XNameAccess;
using com::sun::star::form::binding::IncompatibleTypesException;
using com::sun::star::form::binding::InvalidBindingStateException;
using com::sun::star::form::binding::XValueBinding;
using com::sun::star::lang::EventObject;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::IndexOutOfBoundsException;
using com::sun::star::lang::NoSupportException;
using com::sun::star::lang::NullPointerException;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::lang::XUnoTunnel;
using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::makeAny;
using com::sun::star::util::XModifyListener;
using com::sun::star::xforms::XDataTypeRepository;
using com::sun::star::xml::dom::NodeType_ATTRIBUTE_NODE;
using com::sun::star::xml::dom::NodeType_TEXT_NODE;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XNodeList;
using com::sun::star::xml::dom::events::XEventListener;
using com::sun::star::xml::dom::events::XEventTarget;
using com::sun::star::xsd::XDataType;




#define EXCEPT(msg) OUSTRING(msg),static_cast<XValueBinding*>(this)

#define HANDLE_BindingID 0
#define HANDLE_BindingExpression 1
#define HANDLE_Model 2
#define HANDLE_ModelID 3
#define HANDLE_BindingNamespaces 4
#define HANDLE_ReadonlyExpression 5
#define HANDLE_RelevantExpression 6
#define HANDLE_RequiredExpression 7
#define HANDLE_ConstraintExpression 8
#define HANDLE_CalculateExpression 9
#define HANDLE_Type 10
#define HANDLE_ReadOnly 11  // from com.sun.star.form.binding.ValueBinding, for interaction with a bound form control
#define HANDLE_Relevant 12  // from com.sun.star.form.binding.ValueBinding, for interaction with a bound form control
#define HANDLE_ModelNamespaces 13
#define HANDLE_ExternalData 14


Binding::Binding() :
    mxModel(),
    msBindingID(),
    maBindingExpression(),
    maReadonly(),
    mxNamespaces( new NameContainer<OUString>() ),
    mbInCalculate( false ),
    mnDeferModifyNotifications( 0 ),
    mbValueModified( false ),
    mbBindingModified( false )

{
    initializePropertySet();
}

Binding::~Binding() throw()
{
    _setModel(NULL);
}


Binding::Model_t Binding::getModel() const
{
    return mxModel;
}

void Binding::_setModel( const Model_t& xModel )
{
    PropertyChangeNotifier aNotifyModelChange( *this, HANDLE_Model );
    PropertyChangeNotifier aNotifyModelIDChange( *this, HANDLE_ModelID );

    // prepare binding for removal of old model
    clear(); // remove all cached data (e.g. XPath evaluation results)
    XNameContainer_t xNamespaces = getModelNamespaces(); // save namespaces

    mxModel = xModel;

    // set namespaces (and move to model, if appropriate)
    setBindingNamespaces( xNamespaces );
    _checkBindingID();

    notifyAndCachePropertyValue( HANDLE_ExternalData );
}


OUString Binding::getModelID() const
{
    Model* pModel = getModelImpl();
    return ( pModel == NULL ) ? OUString() : pModel->getID();
}


Binding::XNodeList_t Binding::getXNodeList()
{
    // first make sure we are bound
    if( ! maBindingExpression.hasValue() )
        bind( sal_False );

    return maBindingExpression.getXNodeList();
}

bool Binding::isSimpleBinding() const
{
    return maBindingExpression.isSimpleExpression()
        && maReadonly.isSimpleExpression()
        && maRelevant.isSimpleExpression()
        && maRequired.isSimpleExpression()
        && maConstraint.isSimpleExpression()
        && maCalculate.isSimpleExpression();
}

bool Binding::isSimpleBindingExpression() const
{
    return maBindingExpression.isSimpleExpression();
}

void Binding::update()
{
    // clear all expressions (to remove cached node references)
    maBindingExpression.clear();
    maReadonly.clear();
    maRelevant.clear();
    maRequired.clear();
    maConstraint.clear();
    maCalculate.clear();

    // let's just pretend the binding has been modified -> full rebind()
    bindingModified();
}

void Binding::deferNotifications( bool bDefer )
{
    mnDeferModifyNotifications += ( bDefer ? 1 : -1 );
    OSL_ENSURE( mnDeferModifyNotifications >= 0, "you're deferring too much" );

    if( mnDeferModifyNotifications == 0 )
    {
        if( mbBindingModified )
            bindingModified();
        if( mbValueModified )
            valueModified();
    }

    OSL_ENSURE( ( mnDeferModifyNotifications > 0 )
                || ( ! mbBindingModified  &&  ! mbValueModified ),
                "deferred modifications not delivered?" );
}

bool Binding::isValid()
{
    // TODO: determine whether node is suitable, not just whether it exists
    return maBindingExpression.getNode().is() &&
        isValid_DataType() &&
        maMIP.isConstraint() &&
        ( ! maMIP.isRequired() ||
             ( maBindingExpression.hasValue() &&
               !maBindingExpression.getString().isEmpty() ) );
}

bool Binding::isUseful()
{
    // we are useful, if
    // 0) we don't have a model
    //    (at least, in this case we shouldn't be removed from the model)
    // 1) we have a proper name
    // 2) we have some MIPs,
    // 3) we are bound to some control
    //    (this can be assumed if some listeners are set)
    bool bUseful =
        getModelImpl() == NULL
//        || msBindingID.getLength() > 0
        || ! msTypeName.isEmpty()
        || ! maReadonly.isEmptyExpression()
        || ! maRelevant.isEmptyExpression()
        || ! maRequired.isEmptyExpression()
        || ! maConstraint.isEmptyExpression()
        || ! maCalculate.isEmptyExpression()
        || ! maModifyListeners.empty()
        || ! maListEntryListeners.empty()
        || ! maValidityListeners.empty();

    return bUseful;
}

OUString Binding::explainInvalid()
{
    OUString sReason;
    if( ! maBindingExpression.getNode().is() )
    {
        sReason = ( maBindingExpression.getExpression().isEmpty() )
            ? getResource( RID_STR_XFORMS_NO_BINDING_EXPRESSION )
            : getResource( RID_STR_XFORMS_INVALID_BINDING_EXPRESSION );
    }
    else if( ! isValid_DataType() )
    {
        sReason = explainInvalid_DataType();
        if( sReason.isEmpty() )
        {
            // no explanation given by data type? Then give generic message
            sReason = getResource( RID_STR_XFORMS_INVALID_VALUE,
                                   maMIP.getTypeName() );
        }
    }
    else if( ! maMIP.isConstraint() )
    {
        sReason = maMIP.getConstraintExplanation();
    }
    else if( maMIP.isRequired() && maBindingExpression.hasValue() &&
        maBindingExpression.getString().isEmpty() )
    {
        sReason = getResource( RID_STR_XFORMS_REQUIRED );
    }
    // else: no explanation given; should only happen if data is valid

    OSL_ENSURE( sReason.isEmpty() == isValid(),
                "invalid data should have an explanation!" );

    return sReason;
}



EvaluationContext Binding::getEvaluationContext() const
{
    OSL_ENSURE( getModelImpl() != NULL, "need model impl" );
    EvaluationContext aContext = getModelImpl()->getEvaluationContext();
    aContext.mxNamespaces = getBindingNamespaces();
    return aContext;
}

::std::vector<EvaluationContext> Binding::getMIPEvaluationContexts()
{
    OSL_ENSURE( getModelImpl() != NULL, "need model impl" );

    // bind (in case we were not bound before)
    bind( sal_False );
    return _getMIPEvaluationContexts();
}


Binding::IntSequence_t Binding::getUnoTunnelID()
{
    static cppu::OImplementationId aImplementationId;
    return aImplementationId.getImplementationId();
}

Binding* SAL_CALL Binding::getBinding( const Reference<XPropertySet>& xPropertySet )
{
    Reference<XUnoTunnel> xTunnel( xPropertySet, UNO_QUERY );
    return xTunnel.is()
        ? reinterpret_cast<Binding*>( xTunnel->getSomething(getUnoTunnelID()))
        : NULL;
}




OUString Binding::getBindingID() const
{
    return msBindingID;
}

void Binding::setBindingID( const OUString& sBindingID )
{
    msBindingID = sBindingID;
}

OUString Binding::getBindingExpression() const
{
    return maBindingExpression.getExpression();
}

void Binding::setBindingExpression( const OUString& sBindingExpression)
{
    maBindingExpression.setExpression( sBindingExpression );
    bindingModified();
}

OUString Binding::getReadonlyExpression() const
{
    return maReadonly.getExpression();
}

void Binding::setReadonlyExpression( const OUString& sReadonly)
{
    maReadonly.setExpression( sReadonly );
    bindingModified();
}

OUString Binding::getRelevantExpression() const
{
    return maRelevant.getExpression();
}

void Binding::setRelevantExpression( const OUString& sRelevant )
{
    maRelevant.setExpression( sRelevant );
    bindingModified();
}

OUString Binding::getRequiredExpression() const
{
    return maRequired.getExpression();
}

void Binding::setRequiredExpression( const OUString& sRequired )
{
    maRequired.setExpression( sRequired );
    bindingModified();
}

OUString Binding::getConstraintExpression() const
{
    return maConstraint.getExpression();
}

void Binding::setConstraintExpression( const OUString& sConstraint )
{
    maConstraint.setExpression( sConstraint );
    msExplainConstraint = getResource( RID_STR_XFORMS_INVALID_CONSTRAINT,
                                       sConstraint );

    // TODO: This should only re-evaluate the constraint, and notify
    // the validity constraint listeners; instead we currently pretend
    // the entire binding was notified, which does a little too much.
    bindingModified();
}

OUString Binding::getCalculateExpression() const
{
    return maCalculate.getExpression();
}

void Binding::setCalculateExpression( const OUString& sCalculate )
{
    maCalculate.setExpression( sCalculate );
    bindingModified();
}

OUString Binding::getType() const
{
    return msTypeName;
}

void Binding::setType( const OUString& sTypeName )
{
    msTypeName = sTypeName;
    bindingModified();
}

Binding::XNameContainer_t Binding::getBindingNamespaces() const
{
    //    return _getNamespaces();
    return mxNamespaces;
}

void Binding::setBindingNamespaces( const XNameContainer_t& rNamespaces )
{
    _setNamespaces( rNamespaces, true );
}

Binding::XNameContainer_t Binding::getModelNamespaces() const
{
    return _getNamespaces();
}

void Binding::setModelNamespaces( const XNameContainer_t& rNamespaces )
{
    _setNamespaces( rNamespaces, false );
}

bool Binding::getReadOnly() const
{
    return maMIP.isReadonly();
}

bool Binding::getRelevant() const
{
    return maMIP.isRelevant();
}

bool Binding::getExternalData() const
{
    bool bExternalData = true;
    if ( !mxModel.is() )
        return bExternalData;

    try
    {
        Reference< XPropertySet > xModelProps( mxModel, UNO_QUERY_THROW );
        OSL_VERIFY(
            xModelProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ExternalData" ) ) ) >>= bExternalData );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return bExternalData;
}


void Binding::checkLive()
    throw( RuntimeException )
{
    if( ! isLive() )
        throw RuntimeException( EXCEPT("Binding not initialized") );
}

void Binding::checkModel()
    throw( RuntimeException )
{
    if( ! mxModel.is() )
        throw RuntimeException( EXCEPT("Binding has no Model") );
}

bool Binding::isLive() const
{
    const Model* pModel = getModelImpl();
    return ( pModel != NULL ) ? pModel->isInitialized() : false;
}

Model* Binding::getModelImpl() const
{
    return getModelImpl( mxModel );
}

Model* Binding::getModelImpl( const Model_t& xModel ) const
{
    Reference<XUnoTunnel> xTunnel( xModel, UNO_QUERY );
    Model* pModel = xTunnel.is()
        ? reinterpret_cast<Model*>(
            xTunnel->getSomething( Model::getUnoTunnelID() ) )
        : NULL;
    return pModel;
}

static void lcl_addListenerToNode( Reference<XNode> xNode,
                            Reference<XEventListener> xListener )
{
    Reference<XEventTarget> xTarget( xNode, UNO_QUERY );
    if( xTarget.is() )
    {
        xTarget->addEventListener( OUSTRING("DOMCharacterDataModified"),
                                   xListener, false );
        xTarget->addEventListener( OUSTRING("DOMCharacterDataModified"),
                                   xListener, true );
        xTarget->addEventListener( OUSTRING("DOMAttrModified"),
                                   xListener, false );
        xTarget->addEventListener( OUSTRING("DOMAttrModified"),
                                   xListener, true );
        xTarget->addEventListener( OUSTRING("DOMAttrModified"),
                                   xListener, true );
        xTarget->addEventListener( OUSTRING("xforms-generic"),
                                   xListener, true );
    }
}

static void lcl_removeListenerFromNode( Reference<XNode> xNode,
                                 Reference<XEventListener> xListener )
{
    Reference<XEventTarget> xTarget( xNode, UNO_QUERY );
    if( xTarget.is() )
    {
        xTarget->removeEventListener( OUSTRING("DOMCharacterDataModified"),
                                      xListener, false );
        xTarget->removeEventListener( OUSTRING("DOMCharacterDataModified"),
                                      xListener, true );
        xTarget->removeEventListener( OUSTRING("DOMAttrModified"),
                                      xListener, false );
        xTarget->removeEventListener( OUSTRING("DOMAttrModified"),
                                      xListener, true );
        xTarget->removeEventListener( OUSTRING("xforms-generic"),
                                      xListener, true );
    }
}

::std::vector<EvaluationContext> Binding::_getMIPEvaluationContexts() const
{
    OSL_ENSURE( getModelImpl() != NULL, "need model impl" );

    // iterate over nodes of bind expression and create
    // EvaluationContext for each
    PathExpression::NodeVector_t aNodes = maBindingExpression.getNodeList();
    ::std::vector<EvaluationContext> aVector;
    sal_Int32 nCount = 0; // count nodes for context position
    for( PathExpression::NodeVector_t::iterator aIter = aNodes.begin();
         aIter != aNodes.end();
         ++aIter, ++nCount )
    {
        OSL_ENSURE( aIter->is(), "no node?" );

        // create proper evaluation context for this MIP
        aVector.push_back( EvaluationContext( *aIter, getModel(),
                                              getBindingNamespaces(),
                                              nCount, aNodes.size() ) );
    }
    return aVector;
}

void Binding::bind( bool bForceRebind )
{
    checkModel();

    // bind() will evaluate this binding as follows:
    // 1) evaluate the binding expression
    // 1b) if necessary, create node according to 'lazy author' rules
    // 2) register suitable listeners on the instance (and remove old ones)
    // 3) remove old MIPs defined by this binding
    // 4) for every node in the binding nodeset do:
    //    1) create proper evaluation context for this MIP
    //    2) evaluate calculate expression (and push value into instance)
    //    3) evaluate remaining MIPs
    //    4) evaluate the locally defined MIPs, and push them to the model


    // 1) evaluate the binding expression
    EvaluationContext aContext = getEvaluationContext();
    maBindingExpression.evaluate( aContext );
    if( ! maBindingExpression.getNode().is() )
    {
        // 1b) create node (if valid element name)
        if( isValidQName( maBindingExpression.getExpression(),
                          aContext.mxNamespaces ) )
        {
            aContext.mxContextNode->appendChild(
                Reference<XNode>(
                    aContext.mxContextNode->getOwnerDocument()->createElement(
                        maBindingExpression.getExpression() ),
                    UNO_QUERY ) );
            maBindingExpression.evaluate( aContext );
            OSL_ENSURE( maBindingExpression.getNode().is(),
                        "we should bind to the newly inserted node!" );
        }
    }
    PathExpression::NodeVector_t aNodes = maBindingExpression.getNodeList();

    // 2) register suitable listeners on the instance (and remove old ones)
    if( maEventNodes.empty() || bForceRebind )
    {
        for( XNodes_t::iterator aIter = maEventNodes.begin();
             aIter != maEventNodes.end();
             ++aIter )
            lcl_removeListenerFromNode( *aIter, this );
        maEventNodes.clear();
        if( isSimpleBinding() )
            for( PathExpression::NodeVector_t::iterator aIter = aNodes.begin();
                 aIter != aNodes.end();
                 ++aIter )
                maEventNodes.push_back( *aIter );
        else
            maEventNodes.push_back(
                Reference<XNode>( aContext.mxContextNode->getOwnerDocument(),
                                  UNO_QUERY_THROW ) );
        for( PathExpression::NodeVector_t::iterator aIter2 = maEventNodes.begin();
             aIter2 != maEventNodes.end();
             ++aIter2 )
            lcl_addListenerToNode( *aIter2, this );
    }

    // 3) remove old MIPs defined by this binding
    Model* pModel = getModelImpl();
    OSL_ENSURE( pModel != NULL, "need model" );
    pModel->removeMIPs( this );

    // 4) calculate all MIPs
    ::std::vector<EvaluationContext> aMIPContexts = _getMIPEvaluationContexts();
    for( ::std::vector<EvaluationContext>::iterator aIter = aMIPContexts.begin();
         aIter != aMIPContexts.end();
         ++aIter )
    {
        EvaluationContext& rContext = *aIter;

        // evaluate calculate expression (and push value into instance)
        // (prevent recursion using mbInCalculate
        if( ! maCalculate.isEmptyExpression() )
        {
            if( ! mbInCalculate )
            {
                mbInCalculate = true;
                maCalculate.evaluate( rContext );
                pModel->setSimpleContent( rContext.mxContextNode,
                                          maCalculate.getString() );
                mbInCalculate = false;
            }
        }

        // now evaluate remaining MIPs in the apropriate context
        maReadonly.evaluate( rContext );
        maRelevant.evaluate( rContext );
        maRequired.evaluate( rContext );
        maConstraint.evaluate( rContext );
        // type is static; does not need updating

        // evaluate the locally defined MIPs, and push them to the model
        pModel->addMIP( this, rContext.mxContextNode, getLocalMIP() );
    }
}


// helper for Binding::valueModified
static void lcl_modified( const Binding::XModifyListener_t xListener,
                   const Reference<XInterface> xSource )
{
    OSL_ENSURE( xListener.is(), "no listener?" );
    xListener->modified( EventObject( xSource ) );
}

// helper for Binding::valueModified
static void lcl_listentry( const Binding::XListEntryListener_t xListener,
                    const Reference<XInterface> xSource )
{
    OSL_ENSURE( xListener.is(), "no listener?" );
    // TODO: send fine granular events
    xListener->allEntriesChanged( EventObject( xSource ) );
}

// helper for Binding::valueModified
static void lcl_validate( const Binding::XValidityConstraintListener_t xListener,
                   const Reference<XInterface> xSource )
{
    OSL_ENSURE( xListener.is(), "no listener?" );
    xListener->validityConstraintChanged( EventObject( xSource ) );
}


void Binding::valueModified()
{
    // defer notifications, if so desired
    if( mnDeferModifyNotifications > 0 )
    {
        mbValueModified = true;
        return;
    }
    mbValueModified = false;

    // query MIP used by our first node (also note validity)
    Reference<XNode> xNode = maBindingExpression.getNode();
    maMIP = getModelImpl()->queryMIP( xNode );

    // distribute MIPs _used_ by this binding
    if( xNode.is() )
    {
        notifyAndCachePropertyValue( HANDLE_ReadOnly );
        notifyAndCachePropertyValue( HANDLE_Relevant );
    }

    // iterate over _value_ listeners and send each a modified signal,
    // using this object as source (will also update validity, because
    // control will query once the value has changed)
    Reference<XInterface> xSource = static_cast<XPropertySet*>( this );
    ::std::for_each( maModifyListeners.begin(),
              maModifyListeners.end(),
              ::std::bind2nd( ::std::ptr_fun( lcl_modified ), xSource ) );
    ::std::for_each( maListEntryListeners.begin(),
              maListEntryListeners.end(),
              ::std::bind2nd( ::std::ptr_fun( lcl_listentry ), xSource ) );
    ::std::for_each( maValidityListeners.begin(),
              maValidityListeners.end(),
              ::std::bind2nd( ::std::ptr_fun( lcl_validate ), xSource ) );

    // now distribute MIPs to children
    if( xNode.is() )
        distributeMIP( xNode->getFirstChild() );
}

void Binding::distributeMIP( const XNode_t & rxNode ) {

    typedef com::sun::star::xforms::XFormsEventConcrete XFormsEvent_t;
    OUString sEventName( RTL_CONSTASCII_USTRINGPARAM("xforms-generic") );
    XFormsEvent_t *pEvent = new XFormsEvent_t;
    pEvent->initXFormsEvent(sEventName, sal_True, sal_False);
    Reference<XEvent> xEvent(pEvent);

    // naive depth-first traversal
    XNode_t xNode( rxNode );
    while(xNode.is()) {

        // notifications should be triggered at the
        // leaf nodes first, bubbling upwards the hierarchy.
        XNode_t child(xNode->getFirstChild());
        if(child.is())
            distributeMIP(child);

        // we're standing at a particular node somewhere
        // below the one which changed a property (MIP).
        // bindings which are listening at this node will receive
        // a notification message about what exactly happened.
        Reference< XEventTarget > target(xNode,UNO_QUERY);
        target->dispatchEvent(xEvent);

        xNode = xNode->getNextSibling();
    };
}

void Binding::bindingModified()
{
    // defer notifications, if so desired
    if( mnDeferModifyNotifications > 0 )
    {
        mbBindingModified = true;
        return;
    }
    mbBindingModified = false;

    // rebind (if live); then call valueModified
    // A binding should be inert until its model is fully constructed.
    if( isLive() )
    {
        bind( true );
        valueModified();
    }
}


MIP Binding::getLocalMIP() const
{
    MIP aMIP;

    if( maReadonly.hasValue() )
        aMIP.setReadonly( maReadonly.getBool( false ) );
    if( maRelevant.hasValue() )
        aMIP.setRelevant( maRelevant.getBool( true ) );
    if( maRequired.hasValue() )
        aMIP.setRequired( maRequired.getBool( false ) );
    if( maConstraint.hasValue() )
    {
        aMIP.setConstraint( maConstraint.getBool( true ) );
        if( ! aMIP.isConstraint() )
            aMIP.setConstraintExplanation( msExplainConstraint );
    }
    if( !msTypeName.isEmpty() )
        aMIP.setTypeName( msTypeName );

    // calculate: only handle presence of calculate; value set elsewhere
    aMIP.setHasCalculate( !maCalculate.isEmptyExpression() );

    return aMIP;
}

Binding::XDataType_t Binding::getDataType()
{
    OSL_ENSURE( getModel().is(), "need model" );
    OSL_ENSURE( getModel()->getDataTypeRepository().is(), "need types" );

    Reference<XDataTypeRepository> xRepository(
        getModel()->getDataTypeRepository(), UNO_QUERY );
    OUString sTypeName = maMIP.getTypeName();

    return ( xRepository.is() && xRepository->hasByName( sTypeName ) )
        ? Reference<XDataType>( xRepository->getByName( sTypeName ), UNO_QUERY)
        : Reference<XDataType>( NULL );
}

bool Binding::isValid_DataType()
{
    Reference<XDataType> xDataType = getDataType();
    return xDataType.is()
        ? xDataType->validate( maBindingExpression.getString() )
        : true;
}

rtl::OUString Binding::explainInvalid_DataType()
{
    Reference<XDataType> xDataType = getDataType();
    return xDataType.is()
        ? xDataType->explainInvalid( maBindingExpression.getString() )
        : OUString();
}

void Binding::clear()
{
    // remove MIPs contributed by this binding
    Model* pModel = getModelImpl();
    if( pModel != NULL )
        pModel->removeMIPs( this );

    // remove all references
    for( XNodes_t::iterator aIter = maEventNodes.begin();
         aIter != maEventNodes.end();
         ++aIter )
        lcl_removeListenerFromNode( *aIter, this );
    maEventNodes.clear();

    // clear expressions
    maBindingExpression.clear();
    maReadonly.clear();
    maRelevant.clear();
    maRequired.clear();
    maConstraint.clear();
    maCalculate.clear();

    // TODO: what about our listeners?
}


static void lcl_removeOtherNamespaces( const Binding::XNameContainer_t& xFrom,
                                Binding::XNameContainer_t& xTo )
{
    OSL_ENSURE( xFrom.is(), "no source" );
    OSL_ENSURE( xTo.is(), "no target" );

    // iterate over name in source
    Sequence<OUString> aNames = xTo->getElementNames();
    sal_Int32 nNames = aNames.getLength();
    const OUString* pNames = aNames.getConstArray();
    for( sal_Int32 i = 0; i < nNames; i++ )
    {
        const OUString& rName = pNames[i];

        if( ! xFrom->hasByName( rName ) )
            xTo->removeByName( rName );
    }
}

/** copy namespaces from one namespace container into another
 * @param bOverwrite true: overwrite namespaces in target
 *                   false: do not overwrite namespaces in target
 * @param bMove true: move namespaces (i.e., delete in source)
 *              false: copy namespaces (do not modify source)
 * @param bFromSource true: use elements from source
 *                    false: use only elements from target
 */
static void lcl_copyNamespaces( const Binding::XNameContainer_t& xFrom,
                         Binding::XNameContainer_t& xTo,
                         bool bOverwrite )
{
    OSL_ENSURE( xFrom.is(), "no source" );
    OSL_ENSURE( xTo.is(), "no target" );

    // iterate over name in source
    Sequence<OUString> aNames = xFrom->getElementNames();
    sal_Int32 nNames = aNames.getLength();
    const OUString* pNames = aNames.getConstArray();
    for( sal_Int32 i = 0; i < nNames; i++ )
    {
        const OUString& rName = pNames[i];

        // determine whether to copy the value, and whether to delete
        // it in the source:

        bool bInTarget = xTo->hasByName( rName );

        // we copy: if property is in target, and
        //          if bOverwrite is set, or when the namespace prefix is free
        bool bCopy = bOverwrite || ! bInTarget;

        // and now... ACTION!
        if( bCopy )
        {
            if( bInTarget )
                xTo->replaceByName( rName, xFrom->getByName( rName ) );
            else
                xTo->insertByName( rName, xFrom->getByName( rName ) );
        }
    }
}

// implement get*Namespaces()
// (identical for both variants)
Binding::XNameContainer_t Binding::_getNamespaces() const
{
    XNameContainer_t xNamespaces = new NameContainer<OUString>();
    lcl_copyNamespaces( mxNamespaces, xNamespaces, true );

    // merge model's with binding's own namespaces
    Model* pModel = getModelImpl();
    if( pModel != NULL )
        lcl_copyNamespaces( pModel->getNamespaces(), xNamespaces, false );

    return xNamespaces;
}

// implement set*Namespaces()
// bBinding = true: setBindingNamespaces, otherwise: setModelNamespaces
void Binding::_setNamespaces( const XNameContainer_t& rNamespaces,
                              bool bBinding )
{
    Model* pModel = getModelImpl();
    XNameContainer_t xModelNamespaces = ( pModel != NULL )
                                            ? pModel->getNamespaces()
                                            : NULL;
    OSL_ENSURE( ( pModel != NULL ) == xModelNamespaces.is(), "no model nmsp?");

    // remove deleted namespaces
    lcl_removeOtherNamespaces( rNamespaces, mxNamespaces );
    if( !bBinding && xModelNamespaces.is() )
        lcl_removeOtherNamespaces( rNamespaces, xModelNamespaces );

    // copy namespaces as appropriate
    Sequence<OUString> aNames = rNamespaces->getElementNames();
    sal_Int32 nNames = aNames.getLength();
    const OUString* pNames = aNames.getConstArray();
    for( sal_Int32 i = 0; i < nNames; i++ )
    {
        const OUString& rName = pNames[i];
        Any aValue = rNamespaces->getByName( rName );

        // determine whether the namespace should go into model's or
        // into binding's namespaces
        bool bLocal =
            ! xModelNamespaces.is()
            || mxNamespaces->hasByName( rName )
            || ( bBinding
                 && xModelNamespaces.is()
                 && xModelNamespaces->hasByName( rName ) );

        // write namespace into the appropriate namespace container
        XNameContainer_t& rWhich = bLocal ? mxNamespaces : xModelNamespaces;
        OSL_ENSURE( rWhich.is(), "whoops" );
        if( rWhich->hasByName( rName ) )
            rWhich->replaceByName( rName, aValue );
        else
            rWhich->insertByName( rName, aValue );

        // always 'promote' namespaces from binding to model, if equal
        if( xModelNamespaces.is()
            && xModelNamespaces->hasByName( rName )
            && mxNamespaces->hasByName( rName )
            && xModelNamespaces->getByName( rName ) == mxNamespaces->getByName( rName ) )
        {
            mxNamespaces->removeByName( rName );
        }
    }

    // ... done. But we modified the binding!
    bindingModified();
}

void Binding::_checkBindingID()
{
    if( getModel().is() )
    {
        Reference<XNameAccess> xBindings( getModel()->getBindings(), UNO_QUERY_THROW );
        if( msBindingID.isEmpty() )
        {
            // no binding ID? then make one up!
            OUString sIDPrefix = getResource( RID_STR_XFORMS_BINDING_UI_NAME );
            sIDPrefix += rtl::OUString(" ");
            sal_Int32 nNumber = 0;
            OUString sName;
            do
            {
                nNumber++;
                sName = sIDPrefix + OUString::valueOf( nNumber );
            }
            while( xBindings->hasByName( sName ) );
            setBindingID( sName );
        }
    }
}




//
// XValueBinding
//

Binding::Sequence_Type_t Binding::getSupportedValueTypes()
    throw( RuntimeException )
{
    return Convert::get().getTypes();
}

sal_Bool Binding::supportsType( const Type_t& rType )
    throw( RuntimeException )
{
    return Convert::get().hasType( rType );
}

Binding::Any_t Binding::getValue( const Type_t& rType )
    throw( IncompatibleTypesException,
           RuntimeException )
{
    // first, check for model
    checkLive();

    // second, check for type
    if( ! supportsType( rType ) )
        throw IncompatibleTypesException( EXCEPT( "type unsupported" ) );

    // return string value (if present; else return empty Any)
        Binding::Any_t result = Any();
        if(maBindingExpression.hasValue()) {
            rtl::OUString pathExpr(maBindingExpression.getString());
            Convert &rConvert = Convert::get();
            result = rConvert.toAny(pathExpr,rType);
        }

//      return maBindingExpression.hasValue()
  //      ? Convert::get().toAny( maBindingExpression.getString(), rType )
    //    : Any();

        return result;
}

void Binding::setValue( const Any_t& aValue )
    throw( IncompatibleTypesException,
           InvalidBindingStateException,
           NoSupportException,
           RuntimeException )
{
    // first, check for model
    checkLive();

    // check for supported type
    if( ! supportsType( aValue.getValueType() ) )
        throw IncompatibleTypesException( EXCEPT( "type unsupported" ) );

    if( maBindingExpression.hasValue() )
    {
        Binding::XNode_t xNode = maBindingExpression.getNode();
        if( xNode.is() )
        {
            OUString sValue = Convert::get().toXSD( aValue );
            bool bSuccess = getModelImpl()->setSimpleContent( xNode, sValue );
            if( ! bSuccess )
                throw InvalidBindingStateException( EXCEPT( "can't set value" ) );
        }
        else
            throw InvalidBindingStateException( EXCEPT( "no suitable node found" ) );
    }
    else
        throw InvalidBindingStateException( EXCEPT( "no suitable node found" ) );
}


//
// XListEntry Source
//

sal_Int32 Binding::getListEntryCount()
    throw( RuntimeException )
{
    // first, check for model
    checkLive();

    // return size of node list
    return maBindingExpression.getNodeList().size();
}

static void lcl_getString( const Reference<XNode>& xNode, OUStringBuffer& rBuffer )
{
    if( xNode->getNodeType() == NodeType_TEXT_NODE
        || xNode->getNodeType() == NodeType_ATTRIBUTE_NODE )
    {
        rBuffer.append( xNode->getNodeValue() );
    }
    else
    {
        for( Reference<XNode> xChild = xNode->getFirstChild();
             xChild.is();
             xChild = xChild->getNextSibling() )
        {
            lcl_getString( xChild, rBuffer );
        }
    }
}

static OUString lcl_getString( const Reference<XNode>& xNode )
{
    OUStringBuffer aBuffer;
    lcl_getString( xNode, aBuffer );
    return aBuffer.makeStringAndClear();
}

OUString Binding::getListEntry( sal_Int32 nPosition )
    throw( IndexOutOfBoundsException,
           RuntimeException )
{
    // first, check for model
    checkLive();

    // check bounds and return proper item
    PathExpression::NodeVector_t aNodes = maBindingExpression.getNodeList();
    if( nPosition < 0 || nPosition >= static_cast<sal_Int32>( aNodes.size() ) )
        throw IndexOutOfBoundsException( EXCEPT("") );
    return lcl_getString( aNodes[ nPosition ] );
}

Sequence<OUString> Binding::getAllListEntries()
    throw( RuntimeException )
{
    // first, check for model
    checkLive();

    // create sequence of string values
    PathExpression::NodeVector_t aNodes = maBindingExpression.getNodeList();
    Sequence<OUString> aSequence( aNodes.size() );
    OUString* pSequence = aSequence.getArray();
    for( sal_Int32 n = 0; n < aSequence.getLength(); n++ )
    {
        pSequence[n] = lcl_getString( aNodes[n] );
    }

    return aSequence;
}

void Binding::addListEntryListener( const XListEntryListener_t& xListener )
    throw( NullPointerException,
           RuntimeException )
{
    OSL_ENSURE( xListener.is(), "need listener!" );
    if( ::std::find( maListEntryListeners.begin(),
              maListEntryListeners.end(),
              xListener)
        == maListEntryListeners.end() )
        maListEntryListeners.push_back( xListener );
}

void Binding::removeListEntryListener( const XListEntryListener_t& xListener )
    throw( NullPointerException,
           RuntimeException )
{
    XListEntryListeners_t::iterator aIter =
        ::std::find( maListEntryListeners.begin(), maListEntryListeners.end(),
              xListener );
    if( aIter != maListEntryListeners.end() )
        maListEntryListeners.erase( aIter );
}


//
// XValidator
//

sal_Bool Binding::isValid( const Any_t& )
    throw( RuntimeException )
{
    // first, check for model
    checkLive();

    // ignore value; determine validate only on current data
    return isValid();
}

rtl::OUString Binding::explainInvalid(
    const Any_t& /*Value*/ )
    throw( RuntimeException )
{
    // first, check for model
    checkLive();

    // ignore value; determine explanation  only on current data
    return explainInvalid();
}

void Binding::addValidityConstraintListener(
    const XValidityConstraintListener_t& xListener )
    throw( NullPointerException,
           RuntimeException )
{
    OSL_ENSURE( xListener.is(), "need listener!" );
    if( ::std::find(maValidityListeners.begin(), maValidityListeners.end(), xListener)
        == maValidityListeners.end() )
        maValidityListeners.push_back( xListener );
}

void Binding::removeValidityConstraintListener(
    const XValidityConstraintListener_t& xListener )
    throw( NullPointerException,
           RuntimeException )
{
    XValidityConstraintListeners_t::iterator aIter =
        ::std::find( maValidityListeners.begin(), maValidityListeners.end(),
              xListener );
    if( aIter != maValidityListeners.end() )
        maValidityListeners.erase( aIter );
}



//
// xml::dom::event::XEventListener
//

void Binding::handleEvent( const XEvent_t& xEvent )
    throw( RuntimeException )
{
    OUString sType(xEvent->getType());
    //OUString sEventMIPChanged(RTL_CONSTASCII_USTRINGPARAM("xforms-generic"));
    //if(sType.equals(sEventMIPChanged)) {
    if(!sType.compareToAscii("xforms-generic")) {

        // the modification of the 'mnDeferModifyNotifications'-member
        // is necessary to prevent infinite notication looping.
        // This can happend in case the binding which caused
        // the notification chain is listening to those events
        // as well...
        bool bPreserveValueModified = mbValueModified;
        mnDeferModifyNotifications++;
        valueModified();
        --mnDeferModifyNotifications;
        mbValueModified = bPreserveValueModified;
        return;
    }

    // if we're a dynamic binding, we better re-bind, too!
    bind( false );

    // our value was maybe modified
    valueModified();
}


//
// lang::XUnoTunnel
//

sal_Int64 Binding::getSomething( const IntSequence_t& xId )
    throw( RuntimeException )
{
    return reinterpret_cast<sal_Int64>( ( xId == getUnoTunnelID() ) ? this : NULL );
}

//
// XCloneable
//

Binding::XCloneable_t SAL_CALL Binding::createClone()
    throw( RuntimeException )
{
    Reference< XPropertySet > xClone;

    Model* pModel = getModelImpl();
    if ( pModel )
        xClone = pModel->cloneBinding( this );
    else
    {
        xClone = new Binding;
        copy( this, xClone );
    }
    return XCloneable_t( xClone, UNO_QUERY );
}

//
// property set implementations
//

#define REGISTER_PROPERTY( property, type )   \
    registerProperty( PROPERTY( property, type ), \
    new DirectPropertyAccessor< Binding, type >( this, &Binding::set##property, &Binding::get##property ) );

#define REGISTER_PROPERTY_RO( property, type )   \
    registerProperty( PROPERTY_RO( property, type ), \
    new DirectPropertyAccessor< Binding, type >( this, NULL, &Binding::get##property ) );

#define REGISTER_BOOL_PROPERTY_RO( property )   \
    registerProperty( PROPERTY_RO( property, sal_Bool ), \
    new BooleanPropertyAccessor< Binding, bool >( this, NULL, &Binding::get##property ) );

void Binding::initializePropertySet()
{
    REGISTER_PROPERTY        ( BindingID,            OUString );
    REGISTER_PROPERTY        ( BindingExpression,    OUString );
    REGISTER_PROPERTY_RO     ( Model,                Model_t );
    REGISTER_PROPERTY        ( BindingNamespaces,    XNameContainer_t );
    REGISTER_PROPERTY        ( ModelNamespaces,      XNameContainer_t );
    REGISTER_PROPERTY_RO     ( ModelID,              OUString );
    REGISTER_PROPERTY        ( ReadonlyExpression,   OUString );
    REGISTER_PROPERTY        ( RelevantExpression,   OUString );
    REGISTER_PROPERTY        ( RequiredExpression,   OUString );
    REGISTER_PROPERTY        ( ConstraintExpression, OUString );
    REGISTER_PROPERTY        ( CalculateExpression,  OUString );
    REGISTER_PROPERTY        ( Type,                 OUString );
    REGISTER_PROPERTY_RO     ( ReadOnly,             bool );
    REGISTER_PROPERTY_RO     ( Relevant,             bool );
    REGISTER_BOOL_PROPERTY_RO( ExternalData               );

    initializePropertyValueCache( HANDLE_ReadOnly );
    initializePropertyValueCache( HANDLE_Relevant );
    initializePropertyValueCache( HANDLE_ExternalData );
}

void Binding::addModifyListener(
    const XModifyListener_t& xListener )
    throw( RuntimeException )
{
    OSL_ENSURE( xListener.is(), "need listener!" );
    if( ::std::find( maModifyListeners.begin(), maModifyListeners.end(), xListener )
          == maModifyListeners.end() )
        maModifyListeners.push_back( xListener );

    // HACK: currently, we have to 'push' some MIPs to the control
    // (read-only, relevant, etc.) To enable this, we need to update
    // the control at least once when it registers here.
    valueModified();
}

void Binding::removeModifyListener(
    const XModifyListener_t& xListener )
    throw( RuntimeException )
{
    ModifyListeners_t::iterator aIter =
        ::std::find( maModifyListeners.begin(), maModifyListeners.end(), xListener );
    if( aIter != maModifyListeners.end() )
        maModifyListeners.erase( aIter );
}




rtl::OUString Binding::getName()
    throw( RuntimeException )
{
    return getBindingID();
}

void SAL_CALL Binding::setName( const rtl::OUString& rName )
    throw( RuntimeException )
{
    // use the XPropertySet methods, so the change in the name is notified to the
    // property listeners
    setFastPropertyValue( HANDLE_BindingID, makeAny( rName ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
