/*************************************************************************
 *
 *  $RCSfile: binding.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:48:20 $
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

#include "binding.hxx"

#include "model.hxx"
#include "unohelper.hxx"
#include "NameContainer.hxx"
#include "evaluationcontext.hxx"
#include "convert.hxx"
#include "resourcehelper.hxx"

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <functional>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <comphelper/propertysetinfo.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <unotools/textsearch.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace com::sun::star::xml::xpath;

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
using com::sun::star::form::binding::IncompatibleTypesException;
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
using com::sun::star::uno::makeAny;
using com::sun::star::util::XModifyListener;
using com::sun::star::xforms::XDataTypeRepository;
using com::sun::star::xml::dom::NodeType_ATTRIBUTE_NODE;
using com::sun::star::xml::dom::NodeType_TEXT_NODE;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XNodeList;
using com::sun::star::xml::dom::events::XEventListener;
using com::sun::star::xml::dom::events::XEventTarget;
using com::sun::star::xml::xpath::XXPathObject;
using com::sun::star::xsd::XDataType;




#define EXCEPT(msg) OUSTRING(msg),static_cast<XValueBinding*>(this)



Binding::Binding() :
    mxModel(),
    msBindingID(),
    maBindingExpression(),
    maReadonly(),
    mbInCalculate( false ),
    mnDeferModifyNotifications( 0 ),
    mbValueModified( false ),
    mbBindingModified( false )

{
    setInfo( _getPropertySetInfo() );

    // use the same namespace container on all expressions
    setBindingNamespaces( getBindingNamespaces() );
}

Binding::~Binding() throw()
{
}


Binding::Model_t Binding::getModel() const
{
    return mxModel;
}

void Binding::_setModel( const Model_t& xModel )
{
    clear();
    mxModel = xModel;
}


Reference<XValueBinding> lcl_asXValueBinding( Binding* pThis )
{
    return Reference<XValueBinding>( static_cast<XValueBinding*>( pThis ) );
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
               maBindingExpression.getString().getLength() > 0 ) );
}

OUString Binding::explainInvalid()
{
    OUString sReason;
    if( ! maBindingExpression.getNode().is() )
    {
        sReason = ( maBindingExpression.getExpression().getLength() == 0 )
            ? getResource( RID_STR_XFORMS_NO_BINDING_EXPRESSION )
            : getResource( RID_STR_XFORMS_INVALID_BINDING_EXPRESSION );
    }
    else if( ! isValid_DataType() )
    {
        sReason = explainInvalid_DataType();
        if( sReason.getLength() == 0 )
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
        ( maBindingExpression.getString().getLength() == 0 )  )
    {
        sReason = getResource( RID_STR_XFORMS_REQUIRED );
    }
    // else: no explanation given; should only happen if data is valid

    OSL_ENSURE( ( sReason.getLength() == 0 ) == isValid(),
                "invalid data should have an explanation!" );

    return sReason;
}



EvaluationContext Binding::getEvaluationContext() const
{
    OSL_ENSURE( getModelImpl() != NULL, "need model impl" );
    return getModelImpl()->getEvaluationContext();
}

vector<EvaluationContext> Binding::getMIPEvaluationContexts()
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

Binding* Binding::getBinding( const Reference<XPropertySet>& xPropertySet )
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

OUString Binding::getTypeName() const
{
    return msTypeName;
}

void Binding::setTypeName( const OUString& sTypeName )
{
    msTypeName = sTypeName;
    bindingModified();
}

Binding::XNameContainer_t Binding::getBindingNamespaces() const
{
    return maBindingExpression.getNamespaces();
}

void Binding::setBindingNamespaces( const XNameContainer_t& xBindingNamespaces )
{
    maBindingExpression.setNamespaces( xBindingNamespaces );

    // also use namespaces on all other computed expressions
    maReadonly.setNamespaces( xBindingNamespaces );
    maRelevant.setNamespaces( xBindingNamespaces );
    maRequired.setNamespaces( xBindingNamespaces );
    maConstraint.setNamespaces( xBindingNamespaces );
    maCalculate.setNamespaces( xBindingNamespaces );

    bindingModified();
}

bool Binding::getReadOnly() const
{
    return maMIP.isReadonly();
}

bool Binding::getEnabled() const
{
    return maMIP.isRelevant();
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

void lcl_addListenerToNode( Reference<XNode> xNode,
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
    }
}

void lcl_removeListenerFromNode( Reference<XNode> xNode,
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
    }
}

vector<EvaluationContext> Binding::_getMIPEvaluationContexts() const
{
    OSL_ENSURE( getModelImpl() != NULL, "need model impl" );

    // iterate over nodes of bind expression and create
    // EvaluationContext for each
    PathExpression::NodeVector_t aNodes = maBindingExpression.getNodeList();
    vector<EvaluationContext> aVector;
    sal_Int32 nCount = 0; // count nodes for context position
    for( PathExpression::NodeVector_t::iterator aIter = aNodes.begin();
         aIter != aNodes.end();
         aIter++, nCount++ )
    {
        OSL_ENSURE( aIter->is(), "no node?" );

        // create proper evaluation context for this MIP
        aVector.push_back( EvaluationContext( *aIter, getModel(),
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
        if( maBindingExpression.isElementName() )
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
             aIter ++ )
            lcl_removeListenerFromNode( *aIter, this );
        maEventNodes.clear();
        if( isSimpleBinding() )
            for( PathExpression::NodeVector_t::iterator aIter = aNodes.begin();
                 aIter != aNodes.end();
                 aIter++ )
                maEventNodes.push_back( *aIter );
        else
            maEventNodes.push_back(
                Reference<XNode>( aContext.mxContextNode->getOwnerDocument(),
                                  UNO_QUERY_THROW ) );
        for( PathExpression::NodeVector_t::iterator aIter2 = maEventNodes.begin();
             aIter2 != maEventNodes.end();
             aIter2 ++ )
            lcl_addListenerToNode( *aIter2, this );
    }

    // 3) remove old MIPs defined by this binding
    Model* pModel = getModelImpl();
    OSL_ENSURE( pModel != NULL, "need model" );
    pModel->removeMIPs( this );

    // 4) calculate all MIPs
    vector<EvaluationContext> aMIPContexts = _getMIPEvaluationContexts();
    for( vector<EvaluationContext>::iterator aIter = aMIPContexts.begin();
         aIter != aMIPContexts.end();
         aIter++ )
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
void lcl_modified( const Binding::XModifyListener_t xListener,
                   const Reference<XInterface> xSource )
{
    OSL_ENSURE( xListener.is(), "no listener?" );
    xListener->modified( EventObject( xSource ) );
}

// helper for Binding::valueModified
void lcl_listentry( const Binding::XListEntryListener_t xListener,
                    const Reference<XInterface> xSource )
{
    OSL_ENSURE( xListener.is(), "no listener?" );
    // TODO: send fine granular events
    xListener->allEntriesChanged( EventObject( xSource ) );
}

// helper for Binding::valueModified
void lcl_validate( const Binding::XValidityConstraintListener_t xListener,
                   const Reference<XInterface> xSource )
{
    OSL_ENSURE( xListener.is(), "no listener?" );
    xListener->validityConstraintChanged( EventObject( xSource ) );
}


// helper for Binding::valueModified()
// TODO: remove this methods once the attribute can be set properly
void lcl_setBooleanMIP( Reference<XPropertySet>& xPropertySet,
                        OUString sName,
                        bool bNewValue )
{
    OSL_ENSURE( xPropertySet.is(), "no prop set?" );
    OSL_ENSURE( sName.getLength() > 0, "no name?" );

    if( xPropertySet->getPropertySetInfo()->hasPropertyByName( sName ) )
    {
        // force refresh if we can't get a value
        bool bOldValue = !bNewValue;
        xPropertySet->getPropertyValue( sName ) >>= bOldValue;

        if( bNewValue != bOldValue )
            xPropertySet->setPropertyValue( sName, makeAny( bNewValue ) );
    }
}

// helper for Binding::valueModified()
// TODO: remove this methods once the attribute can be set properly
void lcl_setInt32MIP( Reference<XPropertySet>& xPropertySet,
                      OUString sName,
                      sal_Int32 nNewValue )
{
    if( xPropertySet->getPropertySetInfo()->hasPropertyByName( sName ) )
    {
        // force refresh if we can't get a value
        sal_Int32 nOldValue = ~nNewValue;
        xPropertySet->getPropertyValue( sName ) >>= nOldValue;

        if( nNewValue != nOldValue )
            xPropertySet->setPropertyValue( sName, makeAny( nNewValue ) );
    }
}


// helper for Binding::valueModified()
void lcl_setMIP( Binding::XModifyListener_t xListener,
                 MIP aMIP )
{
    OSL_ENSURE( xListener.is(), "no listener?" );

    // set readonly
    Reference<XPropertySet> xPropSet( xListener, UNO_QUERY );
    if( xPropSet.is() )
    {
        lcl_setBooleanMIP( xPropSet, OUSTRING("ReadOnly"), aMIP.isReadonly() );
        lcl_setBooleanMIP( xPropSet, OUSTRING("Enabled"),  aMIP.isRelevant() );
        //        lcl_setInt32MIP( xPropSet, OUSTRING("BackgroundColor"),
        //                         aMIP.isConstraint() ? 0xffffff : 0xff0000 );
        //        lcl_setInt32MIP( xPropSet, OUSTRING("TextColor"),
        //                         aMIP.isRequired() ? 0xff0000 : 0x000000 );
    }
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
        for_each( maModifyListeners.begin(),
                  maModifyListeners.end(),
                  bind2nd( ptr_fun( lcl_setMIP ), maMIP ));

    // iterate over _value_ listeners and send each a modified signal,
    // using this object as source (will also update validity, because
    // control will query once the value has changed)
    Reference<XInterface> xSource = static_cast<XPropertySet*>( this );
    for_each( maModifyListeners.begin(),
              maModifyListeners.end(),
              bind2nd( ptr_fun( lcl_modified ), xSource ) );
    for_each( maListEntryListeners.begin(),
              maListEntryListeners.end(),
              bind2nd( ptr_fun( lcl_listentry ), xSource ) );
    for_each( maValidityListeners.begin(),
              maValidityListeners.end(),
              bind2nd( ptr_fun( lcl_validate ), xSource ) );
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
    if( msTypeName.getLength() > 0 )
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
         aIter ++ )
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
    return maBindingExpression.hasValue()
        ? Convert::get().toAny( maBindingExpression.getString(), rType )
        : Any();
}

void Binding::setValue( const Any_t& aValue )
    throw( IncompatibleTypesException,
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
                throw NoSupportException( EXCEPT( "can't set value" ) );
        }
        else
            throw NoSupportException( EXCEPT( "no suitable node found" ) );
    }
    else
        throw NoSupportException( EXCEPT( "no suitable node found" ) );
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

void lcl_getString( const Reference<XNode>& xNode, OUStringBuffer& rBuffer )
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

OUString lcl_getString( const Reference<XNode>& xNode )
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
        throw EXCEPT("");
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
    if( find( maListEntryListeners.begin(),
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
        find( maListEntryListeners.begin(), maListEntryListeners.end(),
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
    const Any_t& Value )
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
    if( find(maValidityListeners.begin(), maValidityListeners.end(), xListener)
        == maValidityListeners.end() )
        maValidityListeners.push_back( xListener );
}

void Binding::removeValidityConstraintListener(
    const XValidityConstraintListener_t& xListener )
    throw( NullPointerException,
           RuntimeException )
{
    XValidityConstraintListeners_t::iterator aIter =
        find( maValidityListeners.begin(), maValidityListeners.end(),
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
#define HANDLE_ReadOnly 11  // for control data-source
#define HANDLE_Enabled 12 // for control data-source

#define ENTRY_FLAGS(NAME,TYPE,FLAG) { #NAME, sizeof(#NAME)-1, HANDLE_##NAME, &getCppuType(static_cast<TYPE*>(NULL)), FLAG, 0 }
#define ENTRY(NAME,TYPE) ENTRY_FLAGS(NAME,TYPE,0)
#define ENTRY_RO(NAME,TYPE) ENTRY_FLAGS(NAME,TYPE,com::sun::star::beans::PropertyAttribute::READONLY)
#define ENTRY_END { NULL, 0, NULL, 0, 0}

comphelper::PropertySetInfo* Binding::_getPropertySetInfo()
{
    static comphelper::PropertySetInfo* pInfo = NULL;

    static comphelper::PropertyMapEntry pEntries[] =
    {
        ENTRY( BindingID, OUString ),
        ENTRY( BindingExpression, OUString ),
        ENTRY_RO( Model, Model_t ),
        ENTRY( BindingNamespaces, XNameContainer_t ),
        ENTRY_RO( ModelID, OUString ),
        ENTRY( ReadonlyExpression, OUString ),
        ENTRY( RelevantExpression, OUString ),
        ENTRY( RequiredExpression, OUString ),
        ENTRY( ConstraintExpression, OUString ),
        ENTRY( CalculateExpression, OUString ),
        ENTRY( Type, OUString ),
        ENTRY_RO( ReadOnly, bool ),
        ENTRY_RO( Enabled, bool ),
        ENTRY_END
    };

    if( pInfo == NULL )
    {
        pInfo = new comphelper::PropertySetInfo( pEntries );
        pInfo->acquire();
    }

    return pInfo;
}



void Binding::_setPropertyValues(
    const comphelper::PropertyMapEntry** ppEntries,
    const Any_t* pValues )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException )
{
    // iterate over all PropertyMapEntry/Any pairs
    for( ; *ppEntries != NULL; ppEntries++, pValues++ )
    {
        // delegate each property to the suitable handler method
        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_BindingID:
                setAny( this, &Binding::setBindingID, *pValues );
                break;
            case HANDLE_BindingExpression:
                setAny( this, &Binding::setBindingExpression, *pValues );
                break;
            case HANDLE_BindingNamespaces:
                setAny( this, &Binding::setBindingNamespaces, *pValues );
                break;
            case HANDLE_ModelID:
                OSL_ENSURE( false, "Read-Only properties!" );
                break;
            case HANDLE_ReadonlyExpression:
                setAny( this, &Binding::setReadonlyExpression, *pValues );
                break;
            case HANDLE_RelevantExpression:
                setAny( this, &Binding::setRelevantExpression, *pValues );
                break;
            case HANDLE_RequiredExpression:
                setAny( this, &Binding::setRequiredExpression, *pValues );
                break;
            case HANDLE_ConstraintExpression:
                setAny( this, &Binding::setConstraintExpression, *pValues );
                break;
            case HANDLE_CalculateExpression:
                setAny( this, &Binding::setCalculateExpression, *pValues );
                break;
            case HANDLE_Type:
                setAny( this, &Binding::setTypeName, *pValues );
                break;
            case HANDLE_Model:
            case HANDLE_ReadOnly:
            case HANDLE_Enabled:
                OSL_ENSURE( false, "Read-Only properties!" );
                break;
            default:
                OSL_ENSURE( false, "Unknown HANDLE" );
                break;
        }
    }
}


void Binding::_getPropertyValues(
    const comphelper::PropertyMapEntry** ppEntries,
    Any_t* pValues )
    throw( UnknownPropertyException,
           WrappedTargetException )
{
    // iterate over all PropertyMapEntry/Any pairs
    for( ; *ppEntries != NULL; ppEntries++, pValues++ )
    {
        // delegate each property to the suitable handler method
        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_BindingID:
                getAny( this, &Binding::getBindingID, *pValues );
                break;
            case HANDLE_BindingExpression:
                getAny( this, &Binding::getBindingExpression, *pValues );
                break;
            case HANDLE_Model:
                getAny( this, &Binding::getModel, *pValues );
                break;
            case HANDLE_BindingNamespaces:
                getAny( this, &Binding::getBindingNamespaces, *pValues );
                break;
            case HANDLE_ModelID:
                getAny( this, &Binding::getModelID, *pValues );
                break;
            case HANDLE_ReadonlyExpression:
                getAny( this, &Binding::getReadonlyExpression, *pValues );
                break;
            case HANDLE_RelevantExpression:
                getAny( this, &Binding::getRelevantExpression, *pValues );
                break;
            case HANDLE_RequiredExpression:
                getAny( this, &Binding::getRequiredExpression, *pValues );
                break;
            case HANDLE_ConstraintExpression:
                getAny( this, &Binding::getConstraintExpression, *pValues );
                break;
            case HANDLE_CalculateExpression:
                getAny( this, &Binding::getCalculateExpression, *pValues );
                break;
            case HANDLE_Type:
                getAny( this, &Binding::getTypeName, *pValues );
                break;
            case HANDLE_ReadOnly:
                getAny( this, &Binding::getReadOnly, *pValues );
                break;
            case HANDLE_Enabled:
                getAny( this, &Binding::getEnabled, *pValues );
                break;
            default:
                OSL_ENSURE( false, "Unknown HANDLE" );
                break;
        }
    }
}



void Binding::addModifyListener(
    const XModifyListener_t& xListener )
    throw( RuntimeException )
{
    OSL_ENSURE( xListener.is(), "need listener!" );
    if( find( maModifyListeners.begin(), maModifyListeners.end(), xListener )
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
        find( maModifyListeners.begin(), maModifyListeners.end(), xListener );
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
    setBindingID( rName );
}
