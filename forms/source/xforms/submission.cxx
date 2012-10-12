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




using rtl::OUString;
using rtl::OUStringBuffer;
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
using std::auto_ptr;

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
    msReplace( OUSTRING("none") ),
    msSeparator(),
    msIncludeNamespacePrefixes(),
    m_aFactory(comphelper::getProcessServiceFactory())
{
    initializePropertySet();
}

Submission::~Submission() throw()
{
}

Reference<XModel> Submission::getModel() const
{
    return mxModel;
}

void Submission::setModel( const Reference<XModel>& xModel )
{
    mxModel = xModel;
}

OUString Submission::getID() const
{
    return msID;
}

void Submission::setID( const OUString& sID )
{
    msID = sID;
}

OUString Submission::getBind() const
{
    return msBind;
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

OUString Submission::getAction() const
{
    return msAction;
}

void Submission::setAction( const OUString& sAction )
{
    msAction = sAction;
}

OUString Submission::getMethod() const
{
    return msMethod;
}

void Submission::setMethod( const OUString& sMethod )
{
    msMethod = sMethod;
}

OUString Submission::getVersion() const
{
    return msVersion;
}

void Submission::setVersion( const OUString& sVersion )
{
    msVersion = sVersion;
}

bool Submission::getIndent() const
{
    return mbIndent;
}

void Submission::setIndent( bool bIndent )
{
    mbIndent = bIndent;
}

OUString Submission::getMediaType() const
{
    return msMediaType;
}

void Submission::setMediaType( const OUString& sMediaType )
{
    msMediaType = sMediaType;
}

OUString Submission::getEncoding() const
{
    return msEncoding;
}

void Submission::setEncoding( const OUString& sEncoding )
{
    msEncoding = sEncoding;
}

bool Submission::getOmitXmlDeclaration() const
{
    return mbOmitXmlDeclaration;
}

void Submission::setOmitXmlDeclaration( bool bOmitXmlDeclaration )
{
    mbOmitXmlDeclaration = bOmitXmlDeclaration;
}

bool Submission::getStandalone() const
{
    return mbStandalone;
}

void Submission::setStandalone( bool bStandalone )
{
    mbStandalone = bStandalone;
}

OUString Submission::getCDataSectionElement() const
{
    return msCDataSectionElement;
}

void Submission::setCDataSectionElement( const OUString& sCDataSectionElement )
{
    msCDataSectionElement = sCDataSectionElement;
}

OUString Submission::getReplace() const
{
    return msReplace;
}

void Submission::setReplace( const OUString& sReplace )
{
    msReplace = sReplace;
}

OUString Submission::getSeparator() const
{
    return msSeparator;
}

void Submission::setSeparator( const OUString& sSeparator )
{
    msSeparator = sSeparator;
}

Sequence< OUString > Submission::getIncludeNamespacePrefixes() const
{
    return msIncludeNamespacePrefixes;
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
        if( pBinding != NULL )
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
        aExpression.setExpression( OUSTRING( "/" ) );
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
        xResult, aMethod.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("get")));

    // submit result; set encoding, etc.
    auto_ptr<CSubmission> xSubmission;
    if (aMethod.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("PUT")))
        xSubmission = auto_ptr<CSubmission>(
            new CSubmissionPut( getAction(), aFragment));
    else if (aMethod.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("post")))
        xSubmission = auto_ptr<CSubmission>(
            new CSubmissionPost( getAction(), aFragment));
    else if (aMethod.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("get")))
        xSubmission = auto_ptr<CSubmission>(
            new CSubmissionGet( getAction(), aFragment));
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
        : NULL;
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
    Model* pModel = NULL;
    if( mxModel.is() )
        pModel = Model::getModel( mxModel );
    return pModel;
}


//
// Property-Set implementation
//

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
        // for convinience reasons (????), we accept a string which contains
        // a comma-separated list of namespace prefixes
        ::rtl::OUString sTokenList;
        if ( rValue >>= sTokenList )
        {
            std::vector< OUString > aPrefixes;
            sal_Int32 p = 0;
            while ( p >= 0 )
                aPrefixes.push_back( sTokenList.getToken( 0, ',', p ) );

            Sequence< ::rtl::OUString > aConvertedPrefixes( &aPrefixes[0], aPrefixes.size() );
            return PropertySetBase::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, makeAny( aConvertedPrefixes ) );
        }
    }

    return PropertySetBase::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
}

OUString SAL_CALL Submission::getName()
    throw( RuntimeException )
{
    return getID();
}

void SAL_CALL Submission::setName( const OUString& sID )
    throw( RuntimeException )
{
    setID( sID );
}



sal_Int64 SAL_CALL Submission::getSomething(
    const Sequence<sal_Int8>& aId )
    throw( RuntimeException )
{
    return ( aId == getUnoTunnelID() ) ? reinterpret_cast<sal_Int64>(this) : 0;
}


static OUString lcl_message( const OUString& rID, const OUString& rText )
{
    OUStringBuffer aMessage;
    aMessage.append( OUSTRING("XForms submission '") );
    aMessage.append( rID );
    aMessage.append( OUSTRING("' failed") );
    aMessage.append( rText );
    aMessage.append( OUSTRING(".") );
    return aMessage.makeStringAndClear();
}

void SAL_CALL Submission::submitWithInteraction(
    const Reference<XInteractionHandler>& _rxHandler )
    throw ( VetoException,
            WrappedTargetException,
            RuntimeException )
{
    // as long as this class is not really threadsafe, we need to copy
    // the members we're interested in
    Reference< XModel > xModel( mxModel );
    ::rtl::OUString sID( msID );

    if ( !xModel.is() || msID.isEmpty() )
        throw RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "This is not a valid submission object." ) ),
                *this
              );

    Model* pModel = Model::getModel( xModel );
    OSL_ENSURE( pModel != NULL, "illegal model?" );

    // #i36765# #i47248# warning on submission of illegal data
    // check for validity (and query user if invalid)
    bool bValid = pModel->isValid();
    if( ! bValid )
    {
        InvalidDataOnSubmitException aInvalidDataException(
            lcl_message(sID, OUSTRING(" due to invalid data") ), *this );

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
            lcl_message( sID, OUSTRING(" due to exception being thrown") ),
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

void SAL_CALL Submission::submit( ) throw ( VetoException, WrappedTargetException, RuntimeException )
{
    submitWithInteraction( NULL );
}

void SAL_CALL Submission::addSubmissionVetoListener( const Reference< XSubmissionVetoListener >& /*listener*/ ) throw (NoSupportException, RuntimeException)
{
    // TODO
    throw NoSupportException();
}

void SAL_CALL Submission::removeSubmissionVetoListener( const Reference< XSubmissionVetoListener >& /*listener*/ ) throw (NoSupportException, RuntimeException)
{
    // TODO
    throw NoSupportException();
}

static sal_Bool _isIgnorable(const Reference< XNode >& aNode)
{
    // ignore whitespace-only textnodes
    if (aNode->getNodeType() == NodeType_TEXT_NODE)
    {
        OUString aTrimmedValue = aNode->getNodeValue().trim();
        if (aTrimmedValue.isEmpty()) return sal_True;
    }

    return sal_False;
}

// recursively copy relevant nodes from A to B
static void _cloneNodes(Model& aModel, const Reference< XNode >& dstParent, const Reference< XNode >& source, sal_Bool bRemoveWSNodes)
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
    using namespace com::sun::star::xml::xpath;
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

Reference< XDocumentFragment > Submission::createSubmissionDocument(const Reference< XXPathObject >& aObj, sal_Bool bRemoveWSNodes)
{
    using namespace com::sun::star::xml::xpath;
    Reference< XDocumentBuilder > aDocBuilder(DocumentBuilder::create(comphelper::getComponentContext(m_aFactory)));
    Reference< XDocument > aDocument = aDocBuilder->newDocument();
    Reference< XDocumentFragment > aFragment = aDocument->createDocumentFragment();

    //
    if (aObj->getObjectType() == XPathObjectType_XPATH_NODESET)
    {
        Reference< XNodeList > aList = aObj->getNodeList();
        Reference< XNode > aListItem;
        for (sal_Int32 i=0; i < aList->getLength(); i++)
        {
            aListItem = aList->item(i);
            if (aListItem->getNodeType()==NodeType_DOCUMENT_NODE)
                aListItem = Reference< XNode >(
                    (Reference< XDocument >(aListItem, UNO_QUERY))->getDocumentElement(), UNO_QUERY);
            // copy relevant nodes from instance into fragment
            _cloneNodes(*getModelImpl(), Reference< XNode >(aFragment, UNO_QUERY), aListItem, bRemoveWSNodes);
        }
    }
    return aFragment;
}

// some forwarding: XPropertySet is implemented in our base class,
// but also available as base of XSubmission
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL Submission::getPropertySetInfo(  ) throw(RuntimeException)
{
    return PropertySetBase::getPropertySetInfo();
}
void SAL_CALL Submission::setPropertyValue( const ::rtl::OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    PropertySetBase::setPropertyValue( aPropertyName, aValue );
}
Any SAL_CALL Submission::getPropertyValue( const ::rtl::OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    return PropertySetBase::getPropertyValue( PropertyName );
}
void SAL_CALL Submission::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetBase::addPropertyChangeListener( aPropertyName, xListener );
}
void SAL_CALL Submission::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetBase::removePropertyChangeListener( aPropertyName, aListener );
}
void SAL_CALL Submission::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetBase::addVetoableChangeListener( PropertyName, aListener );
}
void SAL_CALL Submission::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetBase::removeVetoableChangeListener( PropertyName, aListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
