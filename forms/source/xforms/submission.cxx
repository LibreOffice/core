/*************************************************************************
 *
 *  $RCSfile: submission.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:56:45 $
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
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <unotools/processfactory.hxx>
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
using com::sun::star::xforms::XModel;
using com::sun::star::container::XNameAccess;
using com::sun::star::xml::xpath::XXPathObject;
using com::sun::star::xml::xpath::XPathObjectType;
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
    m_aFactory(utl::getProcessServiceFactory())
{
    setInfo( _getPropertySetInfo() );
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

void Submission::setIncludeNamespacePrefixes( const OUString& sIncludeNamespacePrefixes )
{
    std::vector< OUString > vPrefixes;
    sal_Int32 p = 0;
    while ( p >= 0 );
    {
        vPrefixes.push_back(sIncludeNamespacePrefixes.getToken(0, ',', p));
    }
    msIncludeNamespacePrefixes = Sequence< OUString >(vPrefixes.begin(), vPrefixes.size());
}

bool Submission::doSubmit( const Reference< XInteractionHandler >& aHandler )
{
    liveCheck();

    // construct XXPathObject for submission doc; use bind in preference of ref
    EvaluationContext aEvalContext;
    ComputedExpression aExpression;
    if( msBind.getLength() != 0 )
    {
        Binding* pBinding = Binding::getBinding( mxModel->getBinding(msBind) );
        if( pBinding != NULL )
        {
            aExpression.setExpression( pBinding->getBindingExpression() );
            aExpression.setNamespaces( pBinding->getBindingNamespaces() );
            aEvalContext = pBinding->getEvaluationContext();
        }
        // TODO: else: illegal binding name -> raise error
    }
    else if( maRef.getExpression().getLength() != 0 )
    {
        aExpression.setExpression( maRef.getExpression() );
        // TODO: store namespaces for ref expression!!!!
        aExpression.setNamespaces( maRef.getNamespaces() );
        aEvalContext = Model::getModel( mxModel )->getEvaluationContext();
    }
    else
    {
        aExpression.setExpression( OUSTRING( "/" ) );
        // we don't need namespaces for "/".
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
        xResult, aMethod.equalsIgnoreAsciiCaseAscii("get"));

    // submit result; set encoding, etc.
    auto_ptr<CSubmission> xSubmission;
    if (aMethod.equalsIgnoreAsciiCaseAscii("PUT"))
        xSubmission = auto_ptr<CSubmission>(
            new CSubmissionPut( getAction(), aFragment));
    else if (aMethod.equalsIgnoreAsciiCaseAscii("post"))
        xSubmission = auto_ptr<CSubmission>(
            new CSubmissionPost( getAction(), aFragment));
    else if (aMethod.equalsIgnoreAsciiCaseAscii("get"))
        xSubmission = auto_ptr<CSubmission>(
            new CSubmissionGet( getAction(), aFragment));
    else
    {
        OSL_ENSURE(sal_False, "Unsupported xforms submission method");
        return false;
    }

    xSubmission->setEncoding(getEncoding());
    CSubmission::SubmissionResult aResult = xSubmission->submit();

    return ( aResult == CSubmission::SUCCESS );
}


void Submission::releaseModel()
{
    mxModel = NULL;
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

#define ENTRY_FLAGS(NAME,TYPE,FLAG) { #NAME, sizeof(#NAME)-1, HANDLE_##NAME, &getCppuType(static_cast<TYPE*>(NULL)), FLAG, 0 }
#define ENTRY(NAME,TYPE) ENTRY_FLAGS(NAME,TYPE,0)
#define ENTRY_RO(NAME,TYPE) ENTRY_FLAGS(NAME,TYPE,com::sun::star::beans::PropertyAttribute::READONLY)
#define ENTRY_END { NULL, 0, NULL, 0, 0}

comphelper::PropertySetInfo* Submission::_getPropertySetInfo()
{
    static comphelper::PropertySetInfo* pInfo = NULL;

    static comphelper::PropertyMapEntry pEntries[] =
    {
        ENTRY( ID, OUString ),
        ENTRY( Bind, OUString ),
        ENTRY( Ref, OUString ),
        ENTRY( Action, OUString ),
        ENTRY( Method, OUString ),
        ENTRY( Version, OUString ),
        ENTRY( Indent, bool ),
        ENTRY( MediaType, OUString ),
        ENTRY( Encoding, OUString ),
        ENTRY( OmitXmlDeclaration, bool ),
        ENTRY( Standalone, bool ),
        ENTRY( CDataSectionElement, OUString ),
        ENTRY( Replace, OUString ),
        ENTRY( Separator, OUString ),
        ENTRY( IncludeNamespacePrefixes, OUString ),
        ENTRY( Model, Reference<XModel> ),
        ENTRY_END
    };

    if( pInfo == NULL )
    {
        pInfo = new comphelper::PropertySetInfo( pEntries );
        pInfo->acquire();
    }

    return pInfo;
}

void Submission::_setPropertyValues(
    const comphelper::PropertyMapEntry** ppEntries,
    const Any* pValues )
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
            case HANDLE_ID:
                setAny( this, &Submission::setID, *pValues );
                break;
            case HANDLE_Bind:
                setAny( this, &Submission::setBind, *pValues );
                break;
            case HANDLE_Ref:
                setAny( this, &Submission::setRef, *pValues );
                break;
            case HANDLE_Action:
                setAny( this, &Submission::setAction, *pValues );
                break;
            case HANDLE_Method:
                setAny( this, &Submission::setMethod, *pValues );
                break;
            case HANDLE_Version:
                setAny( this, &Submission::setVersion, *pValues );
                break;
            case HANDLE_Indent:
                setAny( this, &Submission::setIndent, *pValues );
                break;
            case HANDLE_MediaType:
                setAny( this, &Submission::setMediaType, *pValues );
                break;
            case HANDLE_Encoding:
                setAny( this, &Submission::setEncoding, *pValues );
                break;
            case HANDLE_OmitXmlDeclaration:
                setAny( this, &Submission::setOmitXmlDeclaration, *pValues );
                break;
            case HANDLE_Standalone:
                setAny( this, &Submission::setStandalone, *pValues );
                break;
            case HANDLE_CDataSectionElement:
                setAny( this, &Submission::setCDataSectionElement, *pValues );
                break;
            case HANDLE_Replace:
                setAny( this, &Submission::setReplace, *pValues );
                break;
            case HANDLE_Separator:
                setAny( this, &Submission::setSeparator, *pValues );
                break;
            case HANDLE_IncludeNamespacePrefixes:
                setAny( this, &Submission::setIncludeNamespacePrefixes, *pValues );
                break;
            case HANDLE_Model:
                setAny( this, &Submission::setModel, *pValues );
                break;
            default:
                OSL_ENSURE( false, "Unknown HANDLE" );
                break;
        }
    }
}

void Submission::_getPropertyValues(
    const comphelper::PropertyMapEntry** ppEntries,
    Any* pValues )
    throw( UnknownPropertyException,
           WrappedTargetException )
{
    // iterate over all PropertyMapEntry/Any pairs
    for( ; *ppEntries != NULL; ppEntries++, pValues++ )
    {
        // delegate each property to the suitable handler method
        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_ID:
                getAny( this, &Submission::getID, *pValues );
                break;
            case HANDLE_Bind:
                getAny( this, &Submission::getBind, *pValues );
                break;
            case HANDLE_Ref:
                getAny( this, &Submission::getRef, *pValues );
                break;
            case HANDLE_Action:
                getAny( this, &Submission::getAction, *pValues );
                break;
            case HANDLE_Method:
                getAny( this, &Submission::getMethod, *pValues );
                break;
            case HANDLE_Version:
                getAny( this, &Submission::getVersion, *pValues );
                break;
            case HANDLE_Indent:
                getAny( this, &Submission::getIndent, *pValues );
                break;
            case HANDLE_MediaType:
                getAny( this, &Submission::getMediaType, *pValues );
                break;
            case HANDLE_Encoding:
                getAny( this, &Submission::getEncoding, *pValues );
                break;
            case HANDLE_OmitXmlDeclaration:
                getAny( this, &Submission::getOmitXmlDeclaration, *pValues );
                break;
            case HANDLE_Standalone:
                getAny( this, &Submission::getStandalone, *pValues );
                break;
            case HANDLE_CDataSectionElement:
                getAny( this, &Submission::getCDataSectionElement, *pValues );
                break;
            case HANDLE_Replace:
                getAny( this, &Submission::getReplace, *pValues );
                break;
            case HANDLE_Separator:
                getAny( this, &Submission::getSeparator, *pValues );
                break;
            case HANDLE_IncludeNamespacePrefixes:
                getAny( this, &Submission::getIncludeNamespacePrefixes, *pValues );
                break;
            case HANDLE_Model:
                getAny( this, &Submission::getModel, *pValues );
                break;
            default:
                OSL_ENSURE( false, "Unknown HANDLE" );
                break;
        }
    }
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



void SAL_CALL Submission::submitWithInteraction( const Reference< XInteractionHandler >& _rxHandler ) throw ( VetoException, WrappedTargetException, RuntimeException )
{
    // as long as this class is not really threadsafe, we need to copy the members we're interested
    // in
    Reference< XModel > xModel( mxModel );
    ::rtl::OUString sID( msID );

    if ( !xModel.is() || !msID.getLength() )
        throw RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "This is not a valid submission object." ) ),
                *this
              );

    try
    {
        xModel->submitWithInteraction( sID, _rxHandler );
    }
    catch( const RuntimeException& e )
    {
        // exception caught: re-throw as wrapped target exception
        OUStringBuffer aMessage;
        aMessage.append( OUSTRING("XForms submission '") );
        aMessage.append( sID );
        aMessage.append( OUSTRING("' failed due to exception being thrown.") );
        throw WrappedTargetException( aMessage.makeStringAndClear(), *this, makeAny( e ) );
    }
}

void SAL_CALL Submission::submit( ) throw ( VetoException, WrappedTargetException, RuntimeException )
{
    submitWithInteraction( NULL );
}

void SAL_CALL Submission::addSubmissionVetoListener( const Reference< XSubmissionVetoListener >& listener ) throw (NoSupportException, RuntimeException)
{
    // TODO
    throw NoSupportException();
}

void SAL_CALL Submission::removeSubmissionVetoListener( const Reference< XSubmissionVetoListener >& listener ) throw (NoSupportException, RuntimeException)
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
        if (aTrimmedValue.getLength() == 0) return sal_True;
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

    while (cur.is())
    {
        //  is this node relevant?
        MIP mip = aModel.queryMIP(cur);
        if(mip.isRelevant() && !(bRemoveWSNodes && _isIgnorable(cur)))
        {
            imported = dstDoc->importNode(cur, sal_False);
            imported = dstParent->appendChild(imported);
            // append source children to new imported parent
            _cloneNodes(aModel, imported, cur->getFirstChild(), bRemoveWSNodes);
        }
        cur = cur->getNextSibling();
    }
}

Reference< XDocumentFragment > Submission::createSubmissionDocument(const Reference< XXPathObject >& aObj, sal_Bool bRemoveWSNodes)
{
    using namespace com::sun::star::xml::xpath;
    Reference< XDocumentBuilder > aDocBuilder(m_aFactory->createInstance(
        OUString::createFromAscii("com.sun.star.xml.dom.DocumentBuilder")), UNO_QUERY);
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

