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


#include "binding.hxx"

#include "model.hxx"
#include "unohelper.hxx"
#include "NameContainer.hxx"
#include "evaluationcontext.hxx"
#include "convert.hxx"
#include "resourcehelper.hxx"
#include "xmlhelper.hxx"
#include "xformsevent.hxx"
#include <strings.hrc>

#include <rtl/ustrbuf.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>

#include <comphelper/diagnose_ex.hxx>

#include <algorithm>
#include <functional>

#include <com/sun/star/form/binding/IncompatibleTypesException.hpp>
#include <com/sun/star/form/binding/InvalidBindingStateException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <comphelper/servicehelper.hxx>

using namespace com::sun::star::xml::xpath;
using namespace com::sun::star::xml::dom::events;

using std::vector;
using xforms::Binding;
using xforms::MIP;
using xforms::Model;
using xforms::getResource;
using xforms::EvaluationContext;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameAccess;
using com::sun::star::form::binding::IncompatibleTypesException;
using com::sun::star::form::binding::InvalidBindingStateException;
using com::sun::star::form::binding::XValueBinding;
using com::sun::star::lang::EventObject;
using com::sun::star::lang::IndexOutOfBoundsException;
using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::util::XModifyListener;
using com::sun::star::xforms::XDataTypeRepository;
using com::sun::star::xml::dom::NodeType_ATTRIBUTE_NODE;
using com::sun::star::xml::dom::NodeType_TEXT_NODE;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XNodeList;
using com::sun::star::xml::dom::events::XEventListener;
using com::sun::star::xml::dom::events::XEventTarget;
using com::sun::star::xsd::XDataType;

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
    mxNamespaces( new NameContainer<OUString>() ),
    mbInCalculate( false ),
    mnDeferModifyNotifications( 0 ),
    mbValueModified( false ),
    mbBindingModified( false )
{
    initializePropertySet();
}

Binding::~Binding()
{
    _setModel(nullptr);
}

void Binding::_setModel( const rtl::Reference<Model>& xModel )
{
    PropertyChangeNotifier aNotifyModelChange( *this, HANDLE_Model );
    PropertyChangeNotifier aNotifyModelIDChange( *this, HANDLE_ModelID );

    // prepare binding for removal of old model
    clear(); // remove all cached data (e.g. XPath evaluation results)
    css::uno::Reference<css::container::XNameContainer> xNamespaces = getModelNamespaces(); // save namespaces

    mxModel = xModel;

    // set namespaces (and move to model, if appropriate)
    setBindingNamespaces( xNamespaces );
    _checkBindingID();

    notifyAndCachePropertyValue( HANDLE_ExternalData );
}


OUString Binding::getModelID() const
{
    return ( mxModel == nullptr ) ? OUString() : mxModel->getID();
}


css::uno::Reference<css::xml::dom::XNodeList> Binding::getXNodeList()
{
    // first make sure we are bound
    if( ! maBindingExpression.hasValue() )
        bind();

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

bool Binding::isValid() const
{
    // TODO: determine whether node is suitable, not just whether it exists
    return maBindingExpression.getNode().is() &&
        (
            // tdf#155121, validity rules should be apply when field is required or
            // when the field is not required but not empty
            // so if the field is not required and empty, do not check validity
            (! maMIP.isRequired() && maBindingExpression.hasValue()
               && maBindingExpression.getString().isEmpty() ) ||
            isValid_DataType()
        ) &&
        maMIP.isConstraint() &&
        ( ! maMIP.isRequired() ||
             ( maBindingExpression.hasValue() &&
               !maBindingExpression.getString().isEmpty() ) );
}

bool Binding::isUseful() const
{
    // we are useful, if
    // 0) we don't have a model
    //    (at least, in this case we shouldn't be removed from the model)
    // 1) we have a proper name
    // 2) we have some MIPs,
    // 3) we are bound to some control
    //    (this can be assumed if some listeners are set)
    bool bUseful =
        mxModel == nullptr
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
    OSL_ENSURE( mxModel != nullptr, "need model impl" );
    EvaluationContext aContext = mxModel->getEvaluationContext();
    aContext.mxNamespaces = getBindingNamespaces();
    return aContext;
}

::std::vector<EvaluationContext> Binding::getMIPEvaluationContexts()
{
    OSL_ENSURE( mxModel != nullptr, "need model impl" );

    // bind (in case we were not bound before)
    bind();
    return _getMIPEvaluationContexts();
}


css::uno::Sequence<sal_Int8> Binding::getUnoTunnelId()
{
    static const comphelper::UnoIdInit aImplementationId;
    return aImplementationId.getSeq();
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


void Binding::setType( const OUString& sTypeName )
{
    msTypeName = sTypeName;
    bindingModified();
}

void Binding::setBindingNamespaces( const css::uno::Reference<css::container::XNameContainer>& rNamespaces )
{
    _setNamespaces( rNamespaces, true );
}

css::uno::Reference<css::container::XNameContainer> Binding::getModelNamespaces() const
{
    return _getNamespaces();
}

void Binding::setModelNamespaces( const css::uno::Reference<css::container::XNameContainer>& rNamespaces )
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
        OSL_VERIFY(
            mxModel->getPropertyValue( u"ExternalData"_ustr ) >>= bExternalData );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.xforms");
    }
    return bExternalData;
}


void Binding::checkLive()
{
    if( ! isLive() )
        throw RuntimeException(u"Binding not initialized"_ustr, static_cast<XValueBinding*>(this));
}

bool Binding::isLive() const
{
    return mxModel && mxModel->isInitialized();
}


static void lcl_addListenerToNode( const Reference<XNode>& xNode,
                                   const Reference<XEventListener>& xListener )
{
    Reference<XEventTarget> xTarget( xNode, UNO_QUERY );
    if( !xTarget.is() )
        return;

    xTarget->addEventListener( u"DOMCharacterDataModified"_ustr,
                               xListener, false );
    xTarget->addEventListener( u"DOMCharacterDataModified"_ustr,
                               xListener, true );
    xTarget->addEventListener( u"DOMAttrModified"_ustr,
                               xListener, false );
    xTarget->addEventListener( u"DOMAttrModified"_ustr,
                               xListener, true );
    xTarget->addEventListener( u"xforms-generic"_ustr,
                               xListener, true );
}

static void lcl_removeListenerFromNode( const Reference<XNode>& xNode,
                                        const Reference<XEventListener>& xListener )
{
    Reference<XEventTarget> xTarget( xNode, UNO_QUERY );
    if( !xTarget.is() )
        return;

    xTarget->removeEventListener( u"DOMCharacterDataModified"_ustr,
                                  xListener, false );
    xTarget->removeEventListener( u"DOMCharacterDataModified"_ustr,
                                  xListener, true );
    xTarget->removeEventListener( u"DOMAttrModified"_ustr,
                                  xListener, false );
    xTarget->removeEventListener( u"DOMAttrModified"_ustr,
                                  xListener, true );
    xTarget->removeEventListener( u"xforms-generic"_ustr,
                                  xListener, true );
}

::std::vector<EvaluationContext> Binding::_getMIPEvaluationContexts() const
{
    OSL_ENSURE( mxModel != nullptr, "need model impl" );

    // iterate over nodes of bind expression and create
    // EvaluationContext for each
    PathExpression::NodeVector_t aNodes = maBindingExpression.getNodeList();
    ::std::vector<EvaluationContext> aVector;
    for (auto const& node : aNodes)
    {
        OSL_ENSURE( node.is(), "no node?" );

        // create proper evaluation context for this MIP
        aVector.emplace_back( node, mxModel, getBindingNamespaces() );
    }
    return aVector;
}

void Binding::bind( bool bForceRebind )
{
    if( ! mxModel.is() )
        throw RuntimeException(u"Binding has no Model"_ustr, static_cast<XValueBinding*>(this));

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
                    aContext.mxContextNode->getOwnerDocument()->createElement(
                        maBindingExpression.getExpression() ) );
            maBindingExpression.evaluate( aContext );
            OSL_ENSURE( maBindingExpression.getNode().is(),
                        "we should bind to the newly inserted node!" );
        }
    }
    PathExpression::NodeVector_t aNodes = maBindingExpression.getNodeList();

    // 2) register suitable listeners on the instance (and remove old ones)
    if( maEventNodes.empty() || bForceRebind )
    {
        for (auto const& eventNode : maEventNodes)
            lcl_removeListenerFromNode( eventNode, this );
        maEventNodes.clear();
        if( isSimpleBinding() )
            maEventNodes.insert(maEventNodes.end(), aNodes.begin(), aNodes.end());
        else
            maEventNodes.emplace_back( aContext.mxContextNode->getOwnerDocument(),
                                  UNO_QUERY_THROW );
        for (auto const& eventNode : maEventNodes)
            lcl_addListenerToNode( eventNode, this );
    }

    // 3) remove old MIPs defined by this binding
    OSL_ENSURE( mxModel != nullptr, "need model" );
    mxModel->removeMIPs( this );

    // 4) calculate all MIPs
    ::std::vector<EvaluationContext> aMIPContexts = _getMIPEvaluationContexts();
    for (auto & context : aMIPContexts)
    {
        EvaluationContext& rContext = context;

        // evaluate calculate expression (and push value into instance)
        // (prevent recursion using mbInCalculate
        if( ! maCalculate.isEmptyExpression() )
        {
            if( ! mbInCalculate )
            {
                mbInCalculate = true;
                maCalculate.evaluate( rContext );
                mxModel->setSimpleContent( rContext.mxContextNode,
                                          maCalculate.getString() );
                mbInCalculate = false;
            }
        }

        // now evaluate remaining MIPs in the appropriate context
        maReadonly.evaluate( rContext );
        maRelevant.evaluate( rContext );
        maRequired.evaluate( rContext );
        maConstraint.evaluate( rContext );
        // type is static; does not need updating

        // evaluate the locally defined MIPs, and push them to the model
        mxModel->addMIP( this, rContext.mxContextNode, getLocalMIP() );
    }
}


// helper for Binding::valueModified
static void lcl_modified( const css::uno::Reference<css::util::XModifyListener>& xListener,
                   const Reference<XInterface>& xSource )
{
    OSL_ENSURE( xListener.is(), "no listener?" );
    xListener->modified( EventObject( xSource ) );
}

// helper for Binding::valueModified
static void lcl_listentry( const css::uno::Reference<css::form::binding::XListEntryListener>& xListener,
                    const Reference<XInterface>& xSource )
{
    OSL_ENSURE( xListener.is(), "no listener?" );
    // TODO: send fine granular events
    xListener->allEntriesChanged( EventObject( xSource ) );
}

// helper for Binding::valueModified
static void lcl_validate( const css::uno::Reference<css::form::validation::XValidityConstraintListener>& xListener,
                   const Reference<XInterface>& xSource )
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
    maMIP = mxModel->queryMIP( xNode );

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
                     ::std::bind( lcl_modified, std::placeholders::_1, xSource ) );
    ::std::for_each( maListEntryListeners.begin(),
              maListEntryListeners.end(),
                     ::std::bind( lcl_listentry, std::placeholders::_1, xSource ) );
    ::std::for_each( maValidityListeners.begin(),
              maValidityListeners.end(),
                     ::std::bind( lcl_validate, std::placeholders::_1, xSource ) );

    // now distribute MIPs to children
    if( xNode.is() )
        distributeMIP( xNode->getFirstChild() );
}

void Binding::distributeMIP( const css::uno::Reference<css::xml::dom::XNode> & rxNode ) {

    rtl::Reference<css::xforms::XFormsEventConcrete> pEvent = new css::xforms::XFormsEventConcrete;
    pEvent->initXFormsEvent(u"xforms-generic"_ustr, true, false);

    // naive depth-first traversal
    css::uno::Reference<css::xml::dom::XNode> xNode( rxNode );
    while(xNode.is()) {

        // notifications should be triggered at the
        // leaf nodes first, bubbling upwards the hierarchy.
        css::uno::Reference<css::xml::dom::XNode> child(xNode->getFirstChild());
        if(child.is())
            distributeMIP(child);

        // we're standing at a particular node somewhere
        // below the one which changed a property (MIP).
        // bindings which are listening at this node will receive
        // a notification message about what exactly happened.
        Reference< XEventTarget > target(xNode,UNO_QUERY);
        target->dispatchEvent(pEvent);

        xNode = xNode->getNextSibling();
    }
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
        aMIP.setReadonly( maReadonly.getBool() );
    if( maRelevant.hasValue() )
        aMIP.setRelevant( maRelevant.getBool( true ) );
    if( maRequired.hasValue() )
        aMIP.setRequired( maRequired.getBool() );
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

css::uno::Reference<css::xsd::XDataType> Binding::getDataType() const
{
    OSL_ENSURE( mxModel.is(), "need model" );
    OSL_ENSURE( mxModel->getDataTypeRepository().is(), "need types" );

    Reference<XDataTypeRepository> xRepository =
        mxModel->getDataTypeRepository();
    OUString sTypeName = maMIP.getTypeName();

    return ( xRepository.is() && xRepository->hasByName( sTypeName ) )
        ? Reference<XDataType>( xRepository->getByName( sTypeName ), UNO_QUERY)
        : Reference<XDataType>( nullptr );
}

bool Binding::isValid_DataType() const
{
    Reference<XDataType> xDataType = getDataType();
    return !xDataType.is()
        || xDataType->validate( maBindingExpression.getString() );
}

OUString Binding::explainInvalid_DataType()
{
    Reference<XDataType> xDataType = getDataType();
    return xDataType.is()
        ? xDataType->explainInvalid( maBindingExpression.getString() )
        : OUString();
}

void Binding::clear()
{
    // remove MIPs contributed by this binding
    if( mxModel != nullptr )
        mxModel->removeMIPs( this );

    // remove all references
    for (auto const& eventNode : maEventNodes)
        lcl_removeListenerFromNode( eventNode, this );
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


static void lcl_removeOtherNamespaces( const css::uno::Reference<css::container::XNameContainer>& xFrom,
                                css::uno::Reference<css::container::XNameContainer> const & xTo )
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
static void lcl_copyNamespaces( const css::uno::Reference<css::container::XNameContainer>& xFrom,
                         css::uno::Reference<css::container::XNameContainer> const & xTo,
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
css::uno::Reference<css::container::XNameContainer> Binding::_getNamespaces() const
{
    css::uno::Reference<css::container::XNameContainer> xNamespaces = new NameContainer<OUString>();
    lcl_copyNamespaces( mxNamespaces, xNamespaces, true );

    // merge model's with binding's own namespaces
    if( mxModel != nullptr )
        lcl_copyNamespaces( mxModel->getNamespaces(), xNamespaces, false );

    return xNamespaces;
}

// implement set*Namespaces()
// bBinding = true: setBindingNamespaces, otherwise: setModelNamespaces
void Binding::_setNamespaces( const css::uno::Reference<css::container::XNameContainer>& rNamespaces,
                              bool bBinding )
{
    css::uno::Reference<css::container::XNameContainer> xModelNamespaces = ( mxModel != nullptr )
                                            ? mxModel->getNamespaces()
                                            : nullptr;
    OSL_ENSURE( ( mxModel != nullptr ) == xModelNamespaces.is(), "no model nmsp?");

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
        css::uno::Reference<css::container::XNameContainer>& rWhich = bLocal ? mxNamespaces : xModelNamespaces;
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
    if( !mxModel.is() )
        return;

    Reference<XNameAccess> xBindings( mxModel->getBindings(), UNO_QUERY_THROW );
    if( !msBindingID.isEmpty() )
        return;

    // no binding ID? then make one up!
    OUString sIDPrefix = getResource( RID_STR_XFORMS_BINDING_UI_NAME ) + " ";
    sal_Int32 nNumber = 0;
    OUString sName;
    do
    {
        nNumber++;
        sName = sIDPrefix + OUString::number( nNumber );
    }
    while( xBindings->hasByName( sName ) );
    setBindingID( sName );
}


// XValueBinding


css::uno::Sequence<css::uno::Type> Binding::getSupportedValueTypes()
{
    return Convert::get().getTypes();
}

sal_Bool Binding::supportsType( const css::uno::Type& rType )
{
    return Convert::get().hasType( rType );
}

css::uno::Any Binding::getValue( const css::uno::Type& rType )
{
    // first, check for model
    checkLive();

    // second, check for type
    if( ! supportsType( rType ) )
        throw IncompatibleTypesException(u"type unsupported"_ustr, static_cast<XValueBinding*>(this));

    // return string value (if present; else return empty Any)
    css::uno::Any result;
    if(maBindingExpression.hasValue()) {
        OUString pathExpr(maBindingExpression.getString());
        Convert &rConvert = Convert::get();
        result = rConvert.toAny(pathExpr,rType);
    }

    return result;
}

void Binding::setValue( const css::uno::Any& aValue )
{
    // first, check for model
    checkLive();

    // check for supported type
    if( ! supportsType( aValue.getValueType() ) )
        throw IncompatibleTypesException(u"type unsupported"_ustr, static_cast<XValueBinding*>(this));

    if( !maBindingExpression.hasValue() )
        throw InvalidBindingStateException(u"no suitable node found"_ustr, static_cast<XValueBinding*>(this));

    css::uno::Reference<css::xml::dom::XNode> xNode = maBindingExpression.getNode();
    if( !xNode.is() )
        throw InvalidBindingStateException(u"no suitable node found"_ustr, static_cast<XValueBinding*>(this));

    OUString sValue = Convert::get().toXSD( aValue );
    bool bSuccess = mxModel->setSimpleContent( xNode, sValue );
    if( ! bSuccess )
        throw InvalidBindingStateException(u"can't set value"_ustr, static_cast<XValueBinding*>(this));


}


// XListEntry Source


sal_Int32 Binding::getListEntryCount()
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
{
    // first, check for model
    checkLive();

    // check bounds and return proper item
    PathExpression::NodeVector_t aNodes = maBindingExpression.getNodeList();
    if( nPosition < 0 || o3tl::make_unsigned(nPosition) >= aNodes.size() )
        throw IndexOutOfBoundsException(u""_ustr, static_cast<XValueBinding*>(this));
    return lcl_getString( aNodes[ nPosition ] );
}

Sequence<OUString> Binding::getAllListEntries()
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

void Binding::addListEntryListener( const css::uno::Reference<css::form::binding::XListEntryListener>& xListener )
{
    OSL_ENSURE( xListener.is(), "need listener!" );
    if( ::std::find( maListEntryListeners.begin(),
              maListEntryListeners.end(),
              xListener)
        == maListEntryListeners.end() )
        maListEntryListeners.push_back( xListener );
}

void Binding::removeListEntryListener( const css::uno::Reference<css::form::binding::XListEntryListener>& xListener )
{
    XListEntryListeners_t::iterator aIter =
        ::std::find( maListEntryListeners.begin(), maListEntryListeners.end(),
              xListener );
    if( aIter != maListEntryListeners.end() )
        maListEntryListeners.erase( aIter );
}


// XValidator


sal_Bool Binding::isValid( const css::uno::Any& )
{
    // first, check for model
    checkLive();

    // ignore value; determine validate only on current data
    return isValid();
}

OUString Binding::explainInvalid(
    const css::uno::Any& /*Value*/ )
{
    // first, check for model
    checkLive();

    // ignore value; determine explanation  only on current data
    return explainInvalid();
}

void Binding::addValidityConstraintListener(
    const css::uno::Reference<css::form::validation::XValidityConstraintListener>& xListener )
{
    OSL_ENSURE( xListener.is(), "need listener!" );
    if( ::std::find(maValidityListeners.begin(), maValidityListeners.end(), xListener)
        == maValidityListeners.end() )
        maValidityListeners.push_back( xListener );
}

void Binding::removeValidityConstraintListener(
    const css::uno::Reference<css::form::validation::XValidityConstraintListener>& xListener )
{
    XValidityConstraintListeners_t::iterator aIter =
        ::std::find( maValidityListeners.begin(), maValidityListeners.end(),
              xListener );
    if( aIter != maValidityListeners.end() )
        maValidityListeners.erase( aIter );
}


// xml::dom::event::XEventListener


void Binding::handleEvent( const css::uno::Reference<css::xml::dom::events::XEvent>& xEvent )
{
    OUString sType(xEvent->getType());
    //OUString sEventMIPChanged("xforms-generic");
    //if(sType.equals(sEventMIPChanged)) {
    if(sType == "xforms-generic") {

        // the modification of the 'mnDeferModifyNotifications'-member
        // is necessary to prevent infinite notification looping.
        // This can happened in case the binding which caused
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
    bind();

    // our value was maybe modified
    valueModified();
}


// lang::XUnoTunnel


sal_Int64 Binding::getSomething( const css::uno::Sequence<sal_Int8>& xId )
{
    return comphelper::getSomethingImpl(xId, this);
}


// XCloneable


css::uno::Reference<css::util::XCloneable> SAL_CALL Binding::createClone()
{
    Reference< XPropertySet > xClone;

    if ( mxModel )
        xClone = mxModel->cloneBinding( this );
    else
    {
        xClone = new Binding;
        copy( this, xClone );
    }
    return css::uno::Reference<css::util::XCloneable>( xClone, UNO_QUERY );
}

css::uno::Reference<css::xforms::XModel> Binding::getModel() const
{
    return mxModel;
}

// property set implementations

void Binding::initializePropertySet()
{
    registerProperty( css::beans::Property(u"BindingID"_ustr, HANDLE_BindingID, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, OUString >(this, &Binding::setBindingID, &Binding::getBindingID));

    registerProperty( css::beans::Property(u"BindingExpression"_ustr, HANDLE_BindingExpression, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, OUString >(this, &Binding::setBindingExpression, &Binding::getBindingExpression));

    registerProperty( css::beans::Property(u"Model"_ustr, HANDLE_Model, cppu::UnoType<css::uno::Reference<css::xforms::XModel>>::get(), css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
    new DirectPropertyAccessor< Binding, css::uno::Reference<css::xforms::XModel> >(this, nullptr, &Binding::getModel));

    registerProperty( css::beans::Property(u"BindingNamespaces"_ustr, HANDLE_BindingNamespaces, cppu::UnoType<css::uno::Reference<css::container::XNameContainer>>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, css::uno::Reference<css::container::XNameContainer> >(this, &Binding::setBindingNamespaces, &Binding::getBindingNamespaces));

    registerProperty( css::beans::Property(u"ModelNamespaces"_ustr, HANDLE_ModelNamespaces, cppu::UnoType<css::uno::Reference<css::container::XNameContainer>>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, css::uno::Reference<css::container::XNameContainer> >(this, &Binding::setModelNamespaces, &Binding::getModelNamespaces));

    registerProperty( css::beans::Property(u"ModelID"_ustr, HANDLE_ModelID, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
    new DirectPropertyAccessor< Binding, OUString >(this, nullptr, &Binding::getModelID));

    registerProperty( css::beans::Property(u"ReadonlyExpression"_ustr, HANDLE_ReadonlyExpression, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, OUString >(this, &Binding::setReadonlyExpression, &Binding::getReadonlyExpression));

    registerProperty( css::beans::Property(u"RelevantExpression"_ustr, HANDLE_RelevantExpression, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, OUString >(this, &Binding::setRelevantExpression, &Binding::getRelevantExpression));

    registerProperty( css::beans::Property(u"RequiredExpression"_ustr, HANDLE_RequiredExpression, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, OUString >(this, &Binding::setRequiredExpression, &Binding::getRequiredExpression));

    registerProperty( css::beans::Property(u"ConstraintExpression"_ustr, HANDLE_ConstraintExpression, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, OUString >(this, &Binding::setConstraintExpression, &Binding::getConstraintExpression));

    registerProperty( css::beans::Property(u"CalculateExpression"_ustr, HANDLE_CalculateExpression, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, OUString >(this, &Binding::setCalculateExpression, &Binding::getCalculateExpression));

    registerProperty( css::beans::Property(u"Type"_ustr, HANDLE_Type, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Binding, OUString >(this, &Binding::setType, &Binding::getType));

    registerProperty( css::beans::Property(u"ReadOnly"_ustr, HANDLE_ReadOnly, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
    new DirectPropertyAccessor< Binding, bool >(this, nullptr, &Binding::getReadOnly));

    registerProperty( css::beans::Property(u"Relevant"_ustr, HANDLE_Relevant, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
    new DirectPropertyAccessor< Binding, bool >(this, nullptr, &Binding::getRelevant));

    registerProperty( css::beans::Property(u"ExternalData"_ustr, HANDLE_ExternalData, cppu::UnoType<sal_Bool>::get(), css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
    new BooleanPropertyAccessor< Binding >(this, nullptr, &Binding::getExternalData));

    initializePropertyValueCache( HANDLE_ReadOnly );
    initializePropertyValueCache( HANDLE_Relevant );
    initializePropertyValueCache( HANDLE_ExternalData );
}

void Binding::addModifyListener(
    const css::uno::Reference<css::util::XModifyListener>& xListener )
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
    const css::uno::Reference<css::util::XModifyListener>& xListener )
{
    ModifyListeners_t::iterator aIter =
        ::std::find( maModifyListeners.begin(), maModifyListeners.end(), xListener );
    if( aIter != maModifyListeners.end() )
        maModifyListeners.erase( aIter );
}


OUString Binding::getName()
{
    return getBindingID();
}

void SAL_CALL Binding::setName( const OUString& rName )
{
    // use the XPropertySet methods, so the change in the name is notified to the
    // property listeners
    setFastPropertyValue( HANDLE_BindingID, Any( rName ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
