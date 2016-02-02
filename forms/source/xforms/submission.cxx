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

#include "submission.hxx"

#include "model.hxx"
#include "binding.hxx"
#include "mip.hxx"
#include "evaluationcontext.hxx"
#include "unohelper.hxx"
#include "submission/submission_put.hxx"
#include "submission/submission_post.hxx"
#include "submission/submission_get.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/xforms/InvalidDataOnSubmitException.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <memory>

using com::sun::star::beans::UnknownPropertyException;
using com::sun::star::beans::PropertyVetoException;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::util::VetoException;
using com::sun::star::form::submission::XSubmissionVetoListener;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::lang::NoSupportException;
using com::sun::star::task::XInteractionHandler;
using com::sun::star::task::XInteractionRequest;
using com::sun::star::task::XInteractionContinuation;
using com::sun::star::xforms::XModel;
using com::sun::star::xforms::InvalidDataOnSubmitException;
using com::sun::star::container::XNameAccess;
using com::sun::star::xml::xpath::XXPathObject;
using com::sun::star::xml::xpath::XPathObjectType;
using com::sun::star::frame::XFrame;
using xforms::Submission;
using xforms::Model;
using xforms::MIP;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;

Submission::Submission() :
    msID(),
    msBind(),
    maRef(),
    msAction(),
    msMethod(),
    msVersion(),
    mbIndent(),
    msMediaType(),
    msEncoding(),
    mbOmitXmlDeclaration(),
    mbStandalone(),
    msCDataSectionElement(),
    msReplace( "none" ),
    msSeparator(),
    msIncludeNamespacePrefixes()
{
    initializePropertySet();
}

Submission::~Submission() throw()
{
}


void Submission::setModel( const Reference<XModel>& xModel )
{
    mxModel = xModel;
}


void Submission::setID( const OUString& sID )
{
    msID = sID;
}


void Submission::setBind( const OUString& sBind )
{
    msBind = sBind;
}

OUString Submission::getRef() const
{
    return maRef.getExpression();
}

void Submission::setRef( const OUString& sRef )
{
    maRef.setExpression( sRef );
}


void Submission::setAction( const OUString& sAction )
{
    msAction = sAction;
}


void Submission::setMethod( const OUString& sMethod )
{
    msMethod = sMethod;
}


void Submission::setVersion( const OUString& sVersion )
{
    msVersion = sVersion;
}


void Submission::setIndent( bool bIndent )
{
    mbIndent = bIndent;
}


void Submission::setMediaType( const OUString& sMediaType )
{
    msMediaType = sMediaType;
}


void Submission::setEncoding( const OUString& sEncoding )
{
    msEncoding = sEncoding;
}


void Submission::setOmitXmlDeclaration( bool bOmitXmlDeclaration )
{
    mbOmitXmlDeclaration = bOmitXmlDeclaration;
}


void Submission::setStandalone( bool bStandalone )
{
    mbStandalone = bStandalone;
}


void Submission::setCDataSectionElement( const OUString& sCDataSectionElement )
{
    msCDataSectionElement = sCDataSectionElement;
}


void Submission::setReplace( const OUString& sReplace )
{
    msReplace = sReplace;
}


void Submission::setSeparator( const OUString& sSeparator )
{
    msSeparator = sSeparator;
}


void Submission::setIncludeNamespacePrefixes( const Sequence< OUString >& rIncludeNamespacePrefixes )
{
    msIncludeNamespacePrefixes = rIncludeNamespacePrefixes;
}

bool Submission::doSubmit( const Reference< XInteractionHandler >& xHandler )
{
    liveCheck();

    // construct XXPathObject for submission doc; use bind in preference of ref
    EvaluationContext aEvalContext;
    ComputedExpression aExpression;
    if( !msBind.isEmpty() )
    {
        Binding* pBinding = Binding::getBinding( mxModel->getBinding(msBind) );
        if( pBinding != nullptr )
        {
            aExpression.setExpression( pBinding->getBindingExpression() );
            aEvalContext = pBinding->getEvaluationContext();
        }
        // TODO: else: illegal binding name -> raise error
    }
    else if( !maRef.getExpression().isEmpty() )
    {
        aExpression.setExpression( maRef.getExpression() );
        aEvalContext = Model::getModel( mxModel )->getEvaluationContext();
    }
    else
    {
        aExpression.setExpression( "/" );
        aEvalContext = Model::getModel( mxModel )->getEvaluationContext();
    }
    aExpression.evaluate( aEvalContext );
    Reference<XXPathObject> xResult = aExpression.getXPath();
    OSL_ENSURE( xResult.is(), "no result?" );

    // early out if we have not obtained any result
    if( ! xResult.is() )
        return false;


    // Reference< XNodeList > aList = xResult->getNodeList();
    OUString aMethod = getMethod();

    // strip whitespace-only text node for get submission
    Reference< XDocumentFragment > aFragment = createSubmissionDocument(
        xResult, aMethod.equalsIgnoreAsciiCase("get"));

    // submit result; set encoding, etc.
    std::unique_ptr<CSubmission> xSubmission;
    if (aMethod.equalsIgnoreAsciiCase("PUT"))
        xSubmission.reset(new CSubmissionPut( getAction(), aFragment));
    else if (aMethod.equalsIgnoreAsciiCase("post"))
        xSubmission.reset(new CSubmissionPost( getAction(), aFragment));
    else if (aMethod.equalsIgnoreAsciiCase("get"))
        xSubmission.reset(new CSubmissionGet( getAction(), aFragment));
    else
    {
        OSL_FAIL("Unsupported xforms submission method");
        return false;
    }

    xSubmission->setEncoding(getEncoding());
    CSubmission::SubmissionResult aResult = xSubmission->submit( xHandler );

    if (aResult == CSubmission::SUCCESS)
    {
        Reference< XDocument > aInstanceDoc = getInstanceDocument(xResult);
        aResult = xSubmission->replace(getReplace(), aInstanceDoc, Reference< XFrame >());
    }

    return ( aResult == CSubmission::SUCCESS );
}

Sequence<sal_Int8> Submission::getUnoTunnelID()
{
    static cppu::OImplementationId aImplementationId;
    return aImplementationId.getImplementationId();
}

Submission* Submission::getSubmission(
    const Reference<XPropertySet>& xPropertySet )
{
    Reference<XUnoTunnel> xTunnel( xPropertySet, UNO_QUERY );
    return xTunnel.is()
        ? reinterpret_cast<Submission*>(
            xTunnel->getSomething( getUnoTunnelID() ) )
        : nullptr;
}


void Submission::liveCheck()
    throw( RuntimeException )
{
    bool bValid = mxModel.is();

    if( ! bValid )
        throw RuntimeException();
}

Model* Submission::getModelImpl() const
{
    Model* pModel = nullptr;
    if( mxModel.is() )
        pModel = Model::getModel( mxModel );
    return pModel;
}


// Property-Set implementation


#define HANDLE_ID 0
#define HANDLE_Bind 1
#define HANDLE_Ref 2
#define HANDLE_Action 3
#define HANDLE_Method 4
#define HANDLE_Version 5
#define HANDLE_Indent 6
#define HANDLE_MediaType 7
#define HANDLE_Encoding 8
#define HANDLE_OmitXmlDeclaration 9
#define HANDLE_Standalone 10
#define HANDLE_CDataSectionElement 11
#define HANDLE_Replace 12
#define HANDLE_Separator 13
#define HANDLE_IncludeNamespacePrefixes 14
#define HANDLE_Model 15

#define REGISTER_PROPERTY( property, type )   \
    registerProperty( PROPERTY( property, type ), \
    new DirectPropertyAccessor< Submission, type >( this, &Submission::set##property, &Submission::get##property ) );

#define REGISTER_PROPERTY_BOOL( property )   \
    registerProperty( PROPERTY( property, bool ), \
    new BooleanPropertyAccessor< Submission, bool >( this, &Submission::set##property, &Submission::get##property ) );

void Submission::initializePropertySet()
{
    REGISTER_PROPERTY     ( ID,                         OUString );
    REGISTER_PROPERTY     ( Bind,                       OUString );
    REGISTER_PROPERTY     ( Ref,                        OUString );
    REGISTER_PROPERTY     ( Action,                     OUString );
    REGISTER_PROPERTY     ( Method,                     OUString );
    REGISTER_PROPERTY     ( Version,                    OUString );
    REGISTER_PROPERTY_BOOL( Indent );
    REGISTER_PROPERTY     ( MediaType,                  OUString );
    REGISTER_PROPERTY     ( Encoding,                   OUString );
    REGISTER_PROPERTY_BOOL( OmitXmlDeclaration );
    REGISTER_PROPERTY_BOOL( Standalone );
    REGISTER_PROPERTY     ( CDataSectionElement,        OUString );
    REGISTER_PROPERTY     ( Replace,                    OUString );
    REGISTER_PROPERTY     ( Separator,                  OUString );
    REGISTER_PROPERTY     ( IncludeNamespacePrefixes,   Sequence< OUString > );
    REGISTER_PROPERTY     ( Model,                      Reference<XModel> );

    initializePropertyValueCache( HANDLE_Indent );
    initializePropertyValueCache( HANDLE_OmitXmlDeclaration );
    initializePropertyValueCache( HANDLE_Standalone );
}

sal_Bool SAL_CALL Submission::convertFastPropertyValue(
    Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue )
    throw ( IllegalArgumentException )
{
    if ( nHandle == HANDLE_IncludeNamespacePrefixes )
    {
        // for convenience reasons (????), we accept a string which contains
        // a comma-separated list of namespace prefixes
        OUString sTokenList;
        if ( rValue >>= sTokenList )
        {
            std::vector< OUString > aPrefixes;
            sal_Int32 p = 0;
            while ( p >= 0 )
                aPrefixes.push_back( sTokenList.getToken( 0, ',', p ) );

            Sequence< OUString > aConvertedPrefixes( &aPrefixes[0], aPrefixes.size() );
            return PropertySetBase::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, makeAny( aConvertedPrefixes ) );
        }
    }

    return PropertySetBase::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
}

OUString SAL_CALL Submission::getName()
    throw( RuntimeException, std::exception )
{
    return getID();
}

void SAL_CALL Submission::setName( const OUString& sID )
    throw( RuntimeException, std::exception )
{
    setID( sID );
}


sal_Int64 SAL_CALL Submission::getSomething(
    const Sequence<sal_Int8>& aId )
    throw( RuntimeException, std::exception )
{
    return ( aId == getUnoTunnelID() ) ? reinterpret_cast<sal_Int64>(this) : 0;
}


static OUString lcl_message( const OUString& rID, const OUString& rText )
{
    OUString aMessage = "XForms submission '" + rID + "' failed" + rText + ".";
    return aMessage;
}

void SAL_CALL Submission::submitWithInteraction(
    const Reference<XInteractionHandler>& _rxHandler )
    throw ( VetoException,
            WrappedTargetException,
            RuntimeException, std::exception )
{
    // as long as this class is not really threadsafe, we need to copy
    // the members we're interested in
    Reference< XModel > xModel( mxModel );
    OUString sID( msID );

    if ( !xModel.is() || msID.isEmpty() )
        throw RuntimeException(
                "This is not a valid submission object.",
                *this
              );

    Model* pModel = Model::getModel( xModel );
    OSL_ENSURE( pModel != nullptr, "illegal model?" );

    // #i36765# #i47248# warning on submission of illegal data
    // check for validity (and query user if invalid)
    bool bValid = pModel->isValid();
    if( ! bValid )
    {
        InvalidDataOnSubmitException aInvalidDataException(
            lcl_message(sID, " due to invalid data" ), *this );

        if( _rxHandler.is() )
        {
            // labouriously create interaction request
            comphelper::OInteractionRequest* pRequest
                = new comphelper::OInteractionRequest(
                    makeAny( aInvalidDataException ) );
            Reference<XInteractionRequest> xRequest = pRequest;

            comphelper::OInteractionApprove* pContinue
                = new comphelper::OInteractionApprove();
            Reference<XInteractionContinuation> xContinue = pContinue;
            pRequest->addContinuation( xContinue );

            comphelper::OInteractionDisapprove* pCancel
                = new comphelper::OInteractionDisapprove();
            Reference<XInteractionContinuation> xCancel = pCancel;
            pRequest->addContinuation( xCancel );

            // ask the handler...
            _rxHandler->handle( xRequest );
            OSL_ENSURE( pContinue->wasSelected() || pCancel->wasSelected(),
                        "handler didn't select" );

            // and continue, if user chose 'continue'
            if( pContinue->wasSelected() )
                bValid = true;
        }

        // abort if invalid (and user didn't tell us to continue)
        if( ! bValid )
            throw aInvalidDataException;
    }

    // attempt submission
    bool bResult = false;
    try
    {
        bResult = doSubmit( _rxHandler );
    }
    catch( const VetoException& )
    {
        OSL_FAIL( "Model::submit: Hmm. How can a single submission have a veto right?" );
        // allowed to leave
        throw;
    }
    catch( const Exception& e )
    {
        // exception caught: re-throw as wrapped target exception
        throw WrappedTargetException(
            lcl_message( sID, " due to exception being thrown" ),
            *this, makeAny( e ) );
    }

    if( bResult )
    {
        mxModel->rebuild();
    }
    else
    {
        // other failure: throw wrapped target exception, too.
        throw WrappedTargetException(
            lcl_message( sID, OUString() ), *this, Any() );
    }
}

void SAL_CALL Submission::submit( ) throw ( VetoException, WrappedTargetException, RuntimeException, std::exception )
{
    submitWithInteraction( nullptr );
}

void SAL_CALL Submission::addSubmissionVetoListener( const Reference< XSubmissionVetoListener >& /*listener*/ ) throw (NoSupportException, RuntimeException, std::exception)
{
    // TODO
    throw NoSupportException();
}

void SAL_CALL Submission::removeSubmissionVetoListener( const Reference< XSubmissionVetoListener >& /*listener*/ ) throw (NoSupportException, RuntimeException, std::exception)
{
    // TODO
    throw NoSupportException();
}

static bool _isIgnorable(const Reference< XNode >& aNode)
{
    // ignore whitespace-only textnodes
    if (aNode->getNodeType() == NodeType_TEXT_NODE)
    {
        OUString aTrimmedValue = aNode->getNodeValue().trim();
        if (aTrimmedValue.isEmpty()) return true;
    }

    return false;
}

// recursively copy relevant nodes from A to B
static void _cloneNodes(Model& aModel, const Reference< XNode >& dstParent, const Reference< XNode >& source, bool bRemoveWSNodes)
{
    if (!source.is()) return;

    Reference< XNode > cur = source;
    Reference< XDocument > dstDoc = dstParent->getOwnerDocument();
    Reference< XNode > imported;

    if (cur.is())
    {
        //  is this node relevant?
        MIP mip = aModel.queryMIP(cur);
        if(mip.isRelevant() && !(bRemoveWSNodes && _isIgnorable(cur)))
        {
            imported = dstDoc->importNode(cur, sal_False);
            imported = dstParent->appendChild(imported);
            // append source children to new imported parent
            for( cur = cur->getFirstChild(); cur.is(); cur = cur->getNextSibling() )
                _cloneNodes(aModel, imported, cur, bRemoveWSNodes);
        }
    }
}
Reference< XDocument > Submission::getInstanceDocument(const Reference< XXPathObject >& aObj)
{
    using namespace css::xml::xpath;
    // result
    Reference< XDocument > aDocument;

    if (aObj->getObjectType() == XPathObjectType_XPATH_NODESET)
    {
        Reference< XNodeList > aList = aObj->getNodeList();
        if (aList->getLength() > 0)
            aDocument = aList->item(0)->getOwnerDocument();
    }
    return aDocument;
}

Reference< XDocumentFragment > Submission::createSubmissionDocument(const Reference< XXPathObject >& aObj, bool bRemoveWSNodes)
{
    using namespace css::xml::xpath;
    Reference< XDocumentBuilder > aDocBuilder = DocumentBuilder::create(comphelper::getProcessComponentContext());
    Reference< XDocument > aDocument = aDocBuilder->newDocument();
    Reference< XDocumentFragment > aFragment = aDocument->createDocumentFragment();


    if (aObj->getObjectType() == XPathObjectType_XPATH_NODESET)
    {
        Reference< XNodeList > aList = aObj->getNodeList();
        Reference< XNode > aListItem;
        for (sal_Int32 i=0; i < aList->getLength(); i++)
        {
            aListItem = aList->item(i);
            if (aListItem->getNodeType()==NodeType_DOCUMENT_NODE)
                aListItem.set( (Reference< XDocument >(aListItem, UNO_QUERY))->getDocumentElement(), UNO_QUERY);
            // copy relevant nodes from instance into fragment
            _cloneNodes(*getModelImpl(), aFragment, aListItem, bRemoveWSNodes);
        }
    }
    return aFragment;
}

// some forwarding: XPropertySet is implemented in our base class,
// but also available as base of XSubmission
Reference< css::beans::XPropertySetInfo > SAL_CALL Submission::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
{
    return PropertySetBase::getPropertySetInfo();
}
void SAL_CALL Submission::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetBase::setPropertyValue( aPropertyName, aValue );
}
Any SAL_CALL Submission::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    return PropertySetBase::getPropertyValue( PropertyName );
}
void SAL_CALL Submission::addPropertyChangeListener( const OUString& aPropertyName, const Reference< css::beans::XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetBase::addPropertyChangeListener( aPropertyName, xListener );
}
void SAL_CALL Submission::removePropertyChangeListener( const OUString& aPropertyName, const Reference< css::beans::XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetBase::removePropertyChangeListener( aPropertyName, aListener );
}
void SAL_CALL Submission::addVetoableChangeListener( const OUString& PropertyName, const Reference< css::beans::XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetBase::addVetoableChangeListener( PropertyName, aListener );
}
void SAL_CALL Submission::removeVetoableChangeListener( const OUString& PropertyName, const Reference< css::beans::XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    PropertySetBase::removeVetoableChangeListener( PropertyName, aListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
