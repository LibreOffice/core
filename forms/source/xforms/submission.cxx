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
#include "submission/submission_put.hxx"
#include "submission/submission_post.hxx"
#include "submission/submission_get.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/xforms/InvalidDataOnSubmitException.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <frm_resource.hxx>
#include <strings.hrc>
#include <memory>
#include <string_view>

using com::sun::star::util::VetoException;
using com::sun::star::form::submission::XSubmissionVetoListener;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::lang::NoSupportException;
using com::sun::star::task::XInteractionHandler;
using com::sun::star::xforms::XModel;
using com::sun::star::xforms::InvalidDataOnSubmitException;
using com::sun::star::xml::xpath::XXPathObject;
using com::sun::star::frame::XFrame;
using xforms::Submission;
using xforms::Model;
using xforms::MIP;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;

Submission::Submission() :
    mbIndent(),
    mbOmitXmlDeclaration(),
    mbStandalone(),
    msReplace( "none" )
{
    initializePropertySet();
}

Submission::~Submission() noexcept
{
}


void Submission::setModel( const Reference<XModel>& xModel )
{
    mxModel = dynamic_cast<Model*>(xModel.get());
    assert(bool(mxModel)==bool(xModel) && "we only support an instance of Model here");
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
        Binding* pBinding = comphelper::getFromUnoTunnel<Binding>( mxModel->getBinding(msBind) );
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
        aEvalContext = mxModel->getEvaluationContext();
    }
    else
    {
        aExpression.setExpression( "/" );
        aEvalContext = mxModel->getEvaluationContext();
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

    const INetURLObject& rURLObject = xSubmission->GetURLObject();
    INetProtocol eProtocol = rURLObject.GetProtocol();
    // tdf#154337 continue to allow submitting to http[s]: without further
    // interaction. Don't allow for other protocols, except for file:
    // where the user has to agree first.
    if (eProtocol != INetProtocol::Http && eProtocol != INetProtocol::Https)
    {
        if (eProtocol != INetProtocol::File)
            return false;
        else
        {
            Reference<css::form::runtime::XFormController> xFormController(xHandler, UNO_QUERY);
            Reference<css::awt::XControl> xContainerControl(xFormController ? xFormController->getContainer() : nullptr, UNO_QUERY);
            Reference<css::awt::XWindow> xParent(xContainerControl ? xContainerControl->getPeer() : nullptr, UNO_QUERY);

            OUString aFileName(rURLObject.getFSysPath(FSysStyle::Detect));
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(Application::GetFrameWeld(xParent),
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           frm::ResourceManager::loadString(RID_STR_XFORMS_WARN_TARGET_IS_FILE).replaceFirst("$", aFileName)));
            xQueryBox->set_default_response(RET_NO);

            if (xQueryBox->run() != RET_YES)
                return false;
        }
    }

    CSubmission::SubmissionResult aResult = xSubmission->submit( xHandler );

    if (aResult == CSubmission::SUCCESS)
    {
        Reference< XDocument > aInstanceDoc = getInstanceDocument(xResult);
        aResult = xSubmission->replace(getReplace(), aInstanceDoc, Reference< XFrame >());
    }

    return ( aResult == CSubmission::SUCCESS );
}

void Submission::liveCheck()
{
    bool bValid = mxModel.is();

    if( ! bValid )
        throw RuntimeException("model not set");
}

css::uno::Reference<XModel> Submission::getModel() const
{
    return mxModel;
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

void Submission::initializePropertySet()
{
    registerProperty( css::beans::Property("ID", HANDLE_ID, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setID, &Submission::getID) );

    registerProperty( css::beans::Property("Bind", HANDLE_Bind, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setBind, &Submission::getBind) );

    registerProperty( css::beans::Property("Ref", HANDLE_Ref, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setRef, &Submission::getRef) );

    registerProperty( css::beans::Property("Action", HANDLE_Action, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setAction, &Submission::getAction) );

    registerProperty( css::beans::Property("Method", HANDLE_Method, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setMethod, &Submission::getMethod) );

    registerProperty( css::beans::Property("Version", HANDLE_Version, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setVersion, &Submission::getVersion) );

    registerProperty( css::beans::Property("Indent", HANDLE_Indent, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::BOUND ),
    new BooleanPropertyAccessor< Submission >(this, &Submission::setIndent, &Submission::getIndent));

    registerProperty( css::beans::Property("MediaType", HANDLE_MediaType, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setMediaType, &Submission::getMediaType) );

    registerProperty( css::beans::Property("Encoding", HANDLE_Encoding, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setEncoding, &Submission::getEncoding) );

    registerProperty( css::beans::Property("OmitXmlDeclaration", HANDLE_OmitXmlDeclaration, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::BOUND ),
    new BooleanPropertyAccessor< Submission >(this, &Submission::setOmitXmlDeclaration, &Submission::getOmitXmlDeclaration));

    registerProperty( css::beans::Property("Standalone", HANDLE_Standalone, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::BOUND ),
    new BooleanPropertyAccessor< Submission >(this, &Submission::setStandalone, &Submission::getStandalone));

    registerProperty( css::beans::Property("CDataSectionElement", HANDLE_CDataSectionElement, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setCDataSectionElement, &Submission::getCDataSectionElement) );

    registerProperty( css::beans::Property("Replace", HANDLE_Replace, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setReplace, &Submission::getReplace) );

    registerProperty( css::beans::Property("Separator", HANDLE_Separator, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, OUString >(this, &Submission::setSeparator, &Submission::getSeparator) );

    registerProperty( css::beans::Property("IncludeNamespacePrefixes", HANDLE_IncludeNamespacePrefixes, cppu::UnoType<Sequence<OUString>>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, Sequence<OUString> >(this, &Submission::setIncludeNamespacePrefixes, &Submission::getIncludeNamespacePrefixes) );

    registerProperty( css::beans::Property("Model", HANDLE_Model, cppu::UnoType<Reference<XModel>>::get(), css::beans::PropertyAttribute::BOUND ),
    new DirectPropertyAccessor< Submission, Reference<XModel> >(this, &Submission::setModel, &Submission::getModel) );

    initializePropertyValueCache( HANDLE_Indent );
    initializePropertyValueCache( HANDLE_OmitXmlDeclaration );
    initializePropertyValueCache( HANDLE_Standalone );
}

sal_Bool SAL_CALL Submission::convertFastPropertyValue(
    Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue )
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

            Sequence< OUString > aConvertedPrefixes( aPrefixes.data(), aPrefixes.size() );
            return PropertySetBase::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, Any( aConvertedPrefixes ) );
        }
    }

    return PropertySetBase::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
}

OUString SAL_CALL Submission::getName()
{
    return getID();
}

void SAL_CALL Submission::setName( const OUString& sID )
{
    setID( sID );
}


static OUString lcl_message( std::u16string_view rID, std::u16string_view rText )
{
    OUString aMessage = OUString::Concat("XForms submission '") + rID + "' failed" + rText + ".";
    return aMessage;
}

void SAL_CALL Submission::submitWithInteraction(
    const Reference<XInteractionHandler>& _rxHandler )
{
    // as long as this class is not really threadsafe, we need to copy
    // the members we're interested in
    rtl::Reference< Model > xModel( mxModel );
    OUString sID( msID );

    if ( !xModel.is() || msID.isEmpty() )
        throw RuntimeException(
                "This is not a valid submission object.",
                *this
              );

    // #i36765# #i47248# warning on submission of illegal data
    // check for validity (and query user if invalid)
    bool bValid = xModel->isValid();
    if( ! bValid )
    {
        InvalidDataOnSubmitException aInvalidDataException(
            lcl_message(sID, u" due to invalid data" ), *this );

        if( _rxHandler.is() )
        {
            // laboriously create interaction request
            rtl::Reference<comphelper::OInteractionRequest> pRequest
                = new comphelper::OInteractionRequest(
                    Any( aInvalidDataException ) );

            rtl::Reference<comphelper::OInteractionApprove> pContinue
                = new comphelper::OInteractionApprove();
            pRequest->addContinuation( pContinue );

            rtl::Reference<comphelper::OInteractionDisapprove> pCancel
                = new comphelper::OInteractionDisapprove();
            pRequest->addContinuation( pCancel );

            // ask the handler...
            _rxHandler->handle( pRequest );
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
    catch( const Exception& )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        // exception caught: re-throw as wrapped target exception
        throw WrappedTargetException(
            lcl_message( sID, u" due to exception being thrown" ),
            *this, anyEx );
    }

    if( !bResult )
    {
        // other failure: throw wrapped target exception, too.
        throw WrappedTargetException(
            lcl_message( sID, std::u16string_view() ), *this, Any() );
    }
    mxModel->rebuild();
}

void SAL_CALL Submission::submit( )
{
    submitWithInteraction( nullptr );
}

void SAL_CALL Submission::addSubmissionVetoListener( const Reference< XSubmissionVetoListener >& /*listener*/ )
{
    // TODO
    throw NoSupportException();
}

void SAL_CALL Submission::removeSubmissionVetoListener( const Reference< XSubmissionVetoListener >& /*listener*/ )
{
    // TODO
    throw NoSupportException();
}

static bool isIgnorable(const Reference< XNode >& aNode)
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
static void cloneNodes(Model& aModel, const Reference< XNode >& dstParent, const Reference< XNode >& source, bool bRemoveWSNodes)
{
    if (!source.is()) return;

    Reference< XNode > cur = source;
    Reference< XDocument > dstDoc = dstParent->getOwnerDocument();
    Reference< XNode > imported;

    if (!cur.is())
        return;

    //  is this node relevant?
    MIP mip = aModel.queryMIP(cur);
    if(mip.isRelevant() && !(bRemoveWSNodes && isIgnorable(cur)))
    {
        imported = dstDoc->importNode(cur, false);
        imported = dstParent->appendChild(imported);
        // append source children to new imported parent
        for( cur = cur->getFirstChild(); cur.is(); cur = cur->getNextSibling() )
            cloneNodes(aModel, imported, cur, bRemoveWSNodes);
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
                aListItem = (Reference< XDocument >(aListItem, UNO_QUERY))->getDocumentElement();
            // copy relevant nodes from instance into fragment
            cloneNodes(*getModelImpl(), aFragment, aListItem, bRemoveWSNodes);
        }
    }
    return aFragment;
}

// some forwarding: XPropertySet is implemented in our base class,
// but also available as base of XSubmission
Reference< css::beans::XPropertySetInfo > SAL_CALL Submission::getPropertySetInfo(  )
{
    return PropertySetBase::getPropertySetInfo();
}
void SAL_CALL Submission::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    PropertySetBase::setPropertyValue( aPropertyName, aValue );
}
Any SAL_CALL Submission::getPropertyValue( const OUString& PropertyName )
{
    return PropertySetBase::getPropertyValue( PropertyName );
}
void SAL_CALL Submission::addPropertyChangeListener( const OUString& aPropertyName, const Reference< css::beans::XPropertyChangeListener >& xListener )
{
    PropertySetBase::addPropertyChangeListener( aPropertyName, xListener );
}
void SAL_CALL Submission::removePropertyChangeListener( const OUString& aPropertyName, const Reference< css::beans::XPropertyChangeListener >& aListener )
{
    PropertySetBase::removePropertyChangeListener( aPropertyName, aListener );
}
void SAL_CALL Submission::addVetoableChangeListener( const OUString& PropertyName, const Reference< css::beans::XVetoableChangeListener >& aListener )
{
    PropertySetBase::addVetoableChangeListener( PropertyName, aListener );
}
void SAL_CALL Submission::removeVetoableChangeListener( const OUString& PropertyName, const Reference< css::beans::XVetoableChangeListener >& aListener )
{
    PropertySetBase::removeVetoableChangeListener( PropertyName, aListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
