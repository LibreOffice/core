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

#include <stdio.h>
#include <ctype.h>

#ifndef _FRM_DATABASEFORM_HXX_
#include "DatabaseForm.hxx"
#endif
#ifndef _FRM_EVENT_THREAD_HXX_
#include "EventThread.hxx"
#endif
#ifndef _FORMS_LISTBOX_HXX_
#include "ListBox.hxx"
#endif
#ifndef _FRM_RESOURCE_HXX_
#include "frm_resource.hxx"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif

#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_DATASELECTIONTYPE_HPP_
#include <com/sun/star/form/DataSelectionType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWSETVETOEXCEPTION_HPP_
#include <com/sun/star/sdb/RowSetVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XPARAMETERSSUPPLIER_HPP_
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_TABULATORCYCLE_HPP_
#include <com/sun/star/form/TabulatorCycle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif



#ifndef _FRM_GROUPMANAGER_HXX_
#include "GroupManager.hxx"
#endif

#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _TOOLS_INETMSG_HXX
#include <tools/inetmsg.hxx>
#endif
#ifndef _INETSTRM_HXX //autogen
#include <bf_svtools/inetstrm.hxx>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _INETTYPE_HXX
#include <bf_svtools/inettype.hxx>
#endif
#ifndef _SV_SVAPP_HXX	// because of the solar mutex
#include <vcl/svapp.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _UNTOOLS_UCBLOCKBYTES_HXX
#include <unotools/ucblockbytes.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef FORMS_MODULE_HXX
#include "formsmodule.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

// compatiblity: DatabaseCursorType is dead, but for compatiblity reasons we still have to write it ...
namespace com {
namespace sun {
namespace star {
namespace data {

enum DatabaseCursorType
{
    DatabaseCursorType_FORWARD = 0,
    DatabaseCursorType_SNAPSHOT = 1,
    DatabaseCursorType_KEYSET = 2,
    DatabaseCursorType_DYNAMIC = 3,
    DatabaseCursorType_MAKE_FIXED_SIZE = SAL_MAX_ENUM
};

} } } }

#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XINTERACTIONSUPPLYPARAMETERS_HPP_
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_PARAMETERSREQUEST_HPP_
#include <com/sun/star/sdb/ParametersRequest.hpp>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

#define DATABASEFORM_IMPLEMENTATION_NAME	::rtl::OUString::createFromAscii("com.sun.star.comp.forms.ODatabaseForm")

using namespace ::dbtools;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::data;
using namespace ::com::sun::star::util;

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ODatabaseForm()
{
    static ::binfilter::frm::OMultiInstanceAutoRegistration< ::binfilter::frm::ODatabaseForm > aAutoRegistration;//STRIP008 	static ::frm::OMultiInstanceAutoRegistration< ::frm::ODatabaseForm > aAutoRegistration;
}

//.........................................................................
namespace frm
{
//.........................................................................


//------------------------------------------------------------------
Reference< XModel> getXModel(const Reference< XInterface>& xIface)
{
    Reference< XModel> xModel(xIface, UNO_QUERY);
    if (xModel.is())
        return xModel;
    else
    {
        Reference< XChild> xChild(xIface, UNO_QUERY);
        if (xChild.is())
        {
            Reference< XInterface> xParent( xChild->getParent());
            return getXModel(xParent);
        }
        else
            return NULL;
    }
}

//==================================================================
// OParameterContinuation
//==================================================================
class OParameterContinuation : public OInteraction< XInteractionSupplyParameters >
{
    Sequence< PropertyValue >		m_aValues;

public:
    OParameterContinuation() { }

    Sequence< PropertyValue >	getValues() const { return m_aValues; }

// XInteractionSupplyParameters
    virtual void SAL_CALL setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException);
};

//------------------------------------------------------------------
void SAL_CALL OParameterContinuation::setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException)
{
    m_aValues = _rValues;
}

//==================================================================
//= OParametersImpl
//=-----------------------------------------------------------------
//= class for the parameter event see approveParameter
//==================================================================

typedef ::cppu::WeakImplHelper2<XIndexAccess, XEnumerationAccess> OParametersImplBase;
class OParametersImpl : public OParametersImplBase
{
public:
    typedef ::std::vector<Reference<XPropertySet> > Parameters;
    typedef Parameters::iterator					ParametersIterator;

private:
    Parameters		m_aParameters;

protected:
    virtual ~OParametersImpl(){}
public:
    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OParametersImpl, OParametersImplBase);

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasElements() throw( RuntimeException );

    // XEnumerationAccess
    virtual Reference<XEnumeration> SAL_CALL createEnumeration() throw( RuntimeException );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( RuntimeException );
    virtual Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    Parameters& getParameters() { return m_aParameters; }
};

// XElementAccess
//------------------------------------------------------------------------------
Type SAL_CALL OParametersImpl::getElementType() throw( RuntimeException )
{
    return ::getCppuType(static_cast<Reference<XPropertySet>*>(NULL));
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OParametersImpl::hasElements() throw( RuntimeException )
{
    return !m_aParameters.empty();
}

// XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OParametersImpl::getCount() throw( RuntimeException )
{
    return m_aParameters.size();
}

//------------------------------------------------------------------------------
Any SAL_CALL OParametersImpl::getByIndex(sal_Int32 _nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if ((_nIndex < 0) || (_nIndex >= (sal_Int32)m_aParameters.size()))
        throw IndexOutOfBoundsException();

    return makeAny(m_aParameters[_nIndex]);
}

// XEnumerationAccess
//------------------------------------------------------------------------------
Reference<XEnumeration>  OParametersImpl::createEnumeration() throw( RuntimeException )
{
    return new OEnumerationByIndex(reinterpret_cast<XIndexAccess*>(this));
}

//==================================================================
//= OParameterInfoImpl
//=-----------------------------------------------------------------
//= class which collects all information for parameter filling
//==================================================================
typedef ::std::multimap< ::rtl::OUString,sal_Int32,::comphelper::UStringLess > MapUString2INT32;

struct OParameterInfoImpl
{
    sal_Int32					nCount; 				//	Number of Parameters
    Reference<XSQLQueryComposer>	xComposer;
    Reference<XIndexAccess>			xParamsAsIndex;
    OParametersImpl*		pParameters;
    MapUString2INT32		aParamMapping;

    OParameterInfoImpl():nCount(0),pParameters(NULL){}
    ~OParameterInfoImpl()
    {
        if (pParameters)
            pParameters->release();
    }
};

//==================================================================
//= OFormSubmitResetThread
//=-----------------------------------------------------------------
//= submitting and resetting html-forms asynchronously
//==================================================================

//------------------------------------------------------------------
class OFormSubmitResetThread: public OComponentEventThread
{
protected:

    // duplicate an event with respect to it's type
    virtual EventObject *cloneEvent( const EventObject *pEvt ) const;

    // process an event. while processing the mutex isn't locked, and pCompImpl
    // is made sure to remain valid
    virtual void processEvent( ::cppu::OComponentHelper* _pCompImpl,
                               const EventObject* _pEvt,
                               const Reference<XControl>& _rControl,
                               sal_Bool _bSubmit);

public:

    OFormSubmitResetThread(ODatabaseForm* pControl) : OComponentEventThread(pControl) { }
};

//------------------------------------------------------------------
EventObject* OFormSubmitResetThread::cloneEvent(
        const EventObject *pEvt ) const
{
    return new ::com::sun::star::awt::MouseEvent( *(::com::sun::star::awt::MouseEvent *)pEvt );
}

//------------------------------------------------------------------
void OFormSubmitResetThread::processEvent(
        ::cppu::OComponentHelper* pCompImpl,
        const EventObject *_pEvt,
        const Reference<XControl>& _rControl,
        sal_Bool _bSubmit)
{
    if (_bSubmit)
        ((ODatabaseForm *)pCompImpl)->submit_impl(_rControl, *reinterpret_cast<const ::com::sun::star::awt::MouseEvent*>(_pEvt), true);
    else
        ((ODatabaseForm *)pCompImpl)->reset_impl(true);
}

//==================================================================
//= ODatabaseForm
//==================================================================

//------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseForm::Create( const Reference< XMultiServiceFactory >& _rxFactory )
{
    return *( new ODatabaseForm( _rxFactory ) );
}

//------------------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL ODatabaseForm::getImplementationId() throw(RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------
Sequence<Type> SAL_CALL ODatabaseForm::getTypes() throw(RuntimeException)
{
    // ask the aggregate
    Sequence<Type> aAggregateTypes;
    Reference<XTypeProvider> xAggregateTypes;
    if (query_aggregation(m_xAggregate, xAggregateTypes))
        aAggregateTypes = xAggregateTypes->getTypes();

    Sequence<Type> aRet = concatSequences(aAggregateTypes, ODatabaseForm_BASE1::getTypes(), OFormComponents::getTypes());
    return concatSequences(aRet,ODatabaseForm_BASE2::getTypes(), ODatabaseForm_BASE3::getTypes());
}

//------------------------------------------------------------------
Any SAL_CALL ODatabaseForm::queryAggregation(const Type& _rType) throw(RuntimeException)
{
    Any aReturn = ODatabaseForm_BASE1::queryInterface(_rType);
    // our own interfaces
    if (!aReturn.hasValue())
    {
        aReturn = ODatabaseForm_BASE2::queryInterface(_rType);
        // property set related interfaces
        if (!aReturn.hasValue())
        {
            aReturn = OPropertySetAggregationHelper::queryInterface(_rType);

            // form component collection related interfaces
            if (!aReturn.hasValue())
            {
                aReturn = OFormComponents::queryAggregation(_rType);

                // interfaces already present in the aggregate which we want to reroute
                // only available if we could create the aggregate
                if (!aReturn.hasValue() && m_xAggregateAsRowSet.is())
                    aReturn = ODatabaseForm_BASE3::queryInterface(_rType);

                // aggregate interfaces
                // (ask the aggregated object _after_ the OComponentHelper (base of OFormComponents),
                // so calls to the XComponent interface reach us and not the aggreagtion)
                if (!aReturn.hasValue() && m_xAggregate.is())
                    aReturn = m_xAggregate->queryAggregation(_rType);
            }
        }
    }

    return aReturn;
}

DBG_NAME(ODatabaseForm)
//------------------------------------------------------------------
ODatabaseForm::ODatabaseForm(const Reference<XMultiServiceFactory>& _rxFactory)
        :OFormComponents(_rxFactory)
        ,OPropertySetAggregationHelper(OComponentHelper::rBHelper)
        ,OPropertyChangeListener(m_aMutex)
        ,m_aLoadListeners(m_aMutex)
        ,m_aRowSetApproveListeners(m_aMutex)
        ,m_aRowSetListeners(m_aMutex)
        ,m_aParameterListeners(m_aMutex)
        ,m_aResetListeners(m_aMutex)
        ,m_aSubmitListeners(m_aMutex)
        ,m_aErrorListeners(m_aMutex)
        ,m_bLoaded(sal_False)
        ,m_bSubForm(sal_False)
        ,m_eNavigation(NavigationBarMode_CURRENT)
        ,m_nPrivileges(0)
        ,m_pParameterInfo(NULL)
        ,m_pThread(NULL)
        ,m_eSubmitMethod(FormSubmitMethod_GET)
        ,m_eSubmitEncoding(FormSubmitEncoding_URL)
        ,m_bAllowDelete(sal_True)
        ,m_bAllowUpdate(sal_True)
        ,m_bAllowInsert(sal_True)
        ,m_pLoadTimer(NULL)
        ,m_nResetsPending(0)
        ,m_bForwardingConnection(sal_False)
        ,m_pAggregatePropertyMultiplexer(NULL)
        ,m_bSharingConnection( sal_False )
{
    DBG_CTOR(ODatabaseForm,NULL);

    // aggregate a row set
    increment(m_refCount);

    {
        m_xAggregate = Reference<XAggregation>(m_xServiceFactory->createInstance(SRV_SDB_ROWSET), UNO_QUERY);
        //	m_xAggregate = Reference<XAggregation>(m_xServiceFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.sdb.dbaccess.ORowSet")), UNO_QUERY);
        DBG_ASSERT(m_xAggregate.is(), "ODatabaseForm::ODatabaseForm : could not instantiate an SDB rowset !");
        m_xAggregateAsRowSet = Reference<XRowSet> (m_xAggregate,UNO_QUERY);
        setAggregation(m_xAggregate);
    }

    // listen for the properties, important for Parameters
    if (m_xAggregateSet.is())
    {
        m_pAggregatePropertyMultiplexer = new OPropertyChangeMultiplexer(this, m_xAggregateSet, sal_False);
        m_pAggregatePropertyMultiplexer->acquire();
        m_pAggregatePropertyMultiplexer->addProperty(PROPERTY_COMMAND);
        m_pAggregatePropertyMultiplexer->addProperty(PROPERTY_FILTER_CRITERIA);
        m_pAggregatePropertyMultiplexer->addProperty(PROPERTY_APPLYFILTER);
        m_pAggregatePropertyMultiplexer->addProperty(PROPERTY_ACTIVE_CONNECTION);
    }

    if (m_xAggregate.is())
    {
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }

    decrement(m_refCount);

    m_pGroupManager = new OGroupManager(this);
    m_pGroupManager->acquire();
}

//------------------------------------------------------------------
ODatabaseForm::~ODatabaseForm()
{
    DBG_DTOR(ODatabaseForm,NULL);

    m_pGroupManager->release();

    if (m_xAggregate.is())
        m_xAggregate->setDelegator(InterfaceRef());

    if (m_pAggregatePropertyMultiplexer)
    {
        m_pAggregatePropertyMultiplexer->dispose();
        m_pAggregatePropertyMultiplexer->release();
        m_pAggregatePropertyMultiplexer = NULL;
    }
}

//==============================================================================
// html tools
//------------------------------------------------------------------------
::rtl::OUString ODatabaseForm::GetDataURLEncoded(const Reference<XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{

    // Liste von successful Controls fuellen
    HtmlSuccessfulObjList aSuccObjList;
    FillSuccessfulList( aSuccObjList, SubmitButton, MouseEvt );


    // Liste zu ::rtl::OUString zusammensetzen
    ::rtl::OUString aResult;
    ::rtl::OUString aName;
    ::rtl::OUString aValue;

    for	(	HtmlSuccessfulObjListIterator pSuccObj = aSuccObjList.begin();
            pSuccObj < aSuccObjList.end();
            ++pSuccObj
        )
    {
        aName = pSuccObj->aName;
        aValue = pSuccObj->aValue;
        if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_FILE && aValue.getLength() )
        {
            // Bei File-URLs wird der Dateiname und keine URL uebertragen,
            // weil Netscape dies so macht.
            INetURLObject aURL;
            aURL.SetSmartProtocol(INET_PROT_FILE);
            aURL.SetSmartURL(aValue);
            if( INET_PROT_FILE == aURL.GetProtocol() )
                aValue = INetURLObject::decode(aURL.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
        }
        Encode( aName );
        Encode( aValue );
        aResult += aName;
        aResult += UniString('=');
        aResult += aValue;
        if (pSuccObj < aSuccObjList.end() - 1)
            aResult += UniString('&');
    }


    aSuccObjList.clear();

    return aResult;
}

//==============================================================================
// html tools
//------------------------------------------------------------------------
::rtl::OUString ODatabaseForm::GetDataTextEncoded(const Reference<XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{

    // Liste von successful Controls fuellen
    HtmlSuccessfulObjList aSuccObjList;
    FillSuccessfulList( aSuccObjList, SubmitButton, MouseEvt );
    // Liste zu ::rtl::OUString zusammensetzen
    ::rtl::OUString aResult;
    ::rtl::OUString aName;
    ::rtl::OUString aValue;

    for	(	HtmlSuccessfulObjListIterator pSuccObj = aSuccObjList.begin();
            pSuccObj < aSuccObjList.end();
            ++pSuccObj
        )
    {
        aName = pSuccObj->aName;
        aValue = pSuccObj->aValue;
        if (pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_FILE && aValue.getLength())
        {
            // Bei File-URLs wird der Dateiname und keine URL uebertragen,
            // weil Netscape dies so macht.
            INetURLObject aURL;
            aURL.SetSmartProtocol(INET_PROT_FILE);
            aURL.SetSmartURL(aValue);
            if( INET_PROT_FILE == aURL.GetProtocol() )
                aValue = INetURLObject::decode(aURL.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
        }
        Encode( aName );
        Encode( aValue );
        aResult += pSuccObj->aName;
        aResult += UniString('=');
        aResult += pSuccObj->aValue;
        if (pSuccObj < aSuccObjList.end() - 1)
            aResult += ::rtl::OUString::createFromAscii("\r\n");
    }


    // Liste loeschen
    aSuccObjList.clear();

    return aResult;
}

//------------------------------------------------------------------------
Sequence<sal_Int8> ODatabaseForm::GetDataMultiPartEncoded(const Reference<XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt, ::rtl::OUString& rContentType)
{

    // Parent erzeugen
    INetMIMEMessage aParent;
    aParent.EnableAttachChild( INETMSG_MULTIPART_FORM_DATA );


    // Liste von successful Controls fuellen
    HtmlSuccessfulObjList aSuccObjList;
    FillSuccessfulList( aSuccObjList, SubmitButton, MouseEvt );


    // Liste zu ::rtl::OUString zusammensetzen
    ::rtl::OUString aResult;
    for	(	HtmlSuccessfulObjListIterator pSuccObj = aSuccObjList.begin();
            pSuccObj < aSuccObjList.end();
            ++pSuccObj
        )
    {
        if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_TEXT )
            InsertTextPart( aParent, pSuccObj->aName, pSuccObj->aValue );
        else if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_FILE )
            InsertFilePart( aParent, pSuccObj->aName, pSuccObj->aValue );
    }


    // Liste loeschen
    aSuccObjList.clear();

    // Fuer Parent MessageStream erzeugen
    INetMIMEMessageStream aMessStream;
    aMessStream.SetSourceMessage( &aParent );
    aMessStream.GenerateHeader( sal_False );

    // MessageStream in SvStream kopieren
    SvMemoryStream aMemStream;
    char* pBuf = new char[1025];
    int nRead;
    while( (nRead = aMessStream.Read(pBuf, 1024)) > 0 )
        aMemStream.Write( pBuf, nRead );
    delete[] pBuf;

    aMemStream.Flush();
    aMemStream.Seek( 0 );
    void* pData = (void*)aMemStream.GetData();
    sal_Int32 nLen = aMemStream.Seek(STREAM_SEEK_TO_END);

    rContentType = UniString(aParent.GetContentType());
    return Sequence<sal_Int8>((sal_Int8*)pData, nLen);
}

//------------------------------------------------------------------------
void ODatabaseForm::AppendComponent(HtmlSuccessfulObjList& rList, const Reference<XPropertySet>& xComponentSet, const ::rtl::OUString& rNamePrefix,
                     const Reference<XControl>& rxSubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{
    if (!xComponentSet.is())
        return;

    // MIB 25.6.98: Geschachtelte Formulare abfangen ... oder muesste
    // man sie submitten?
    if (!hasProperty(PROPERTY_CLASSID, xComponentSet))
        return;

    // Namen ermitteln
    if (!hasProperty(PROPERTY_NAME, xComponentSet))
        return;

    sal_Int16 nClassId;
    xComponentSet->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;
    ::rtl::OUString aName;
    xComponentSet->getPropertyValue( PROPERTY_NAME ) >>= aName;
    if( !aName.getLength() && nClassId != FormComponentType::IMAGEBUTTON)
        return;
    else	// Name um den Prefix erweitern
        aName = rNamePrefix + aName;

    switch( nClassId )
    {
        // Buttons
        case FormComponentType::COMMANDBUTTON:
        {
            // Es wird nur der gedrueckte Submit-Button ausgewertet
            // MIB: Sofern ueberhaupt einer uebergeben wurde
            if( rxSubmitButton.is() )
            {
                Reference<XPropertySet>  xSubmitButtonComponent(rxSubmitButton->getModel(), UNO_QUERY);
                if (xSubmitButtonComponent == xComponentSet && hasProperty(PROPERTY_LABEL, xComponentSet))
                {
                    // <name>=<label>
                    ::rtl::OUString aLabel;
                    xComponentSet->getPropertyValue( PROPERTY_LABEL ) >>= aLabel;
                    rList.push_back( HtmlSuccessfulObj(aName, aLabel) );
                }
            }
        } break;

        // ImageButtons
        case FormComponentType::IMAGEBUTTON:
        {
            // Es wird nur der gedrueckte Submit-Button ausgewertet
            // MIB: Sofern ueberhaupt einer uebergeben wurde
            if( rxSubmitButton.is() )
            {
                Reference<XPropertySet>  xSubmitButtonComponent(rxSubmitButton->getModel(), UNO_QUERY);
                if (xSubmitButtonComponent == xComponentSet)
                {
                    // <name>.x=<pos.X>&<name>.y=<pos.Y>
                    ::rtl::OUString aLhs = aName;
                    ::rtl::OUString aRhs = ::rtl::OUString::valueOf( MouseEvt.X );

                    // nur wenn ein Name vorhanden ist, kann ein name.x
                    aLhs += aName.getLength() ? UniString::CreateFromAscii(".x") : UniString::CreateFromAscii("x");
                    rList.push_back( HtmlSuccessfulObj(aLhs, aRhs) );

                    aLhs = aName;
                    aRhs = ::rtl::OUString::valueOf( MouseEvt.Y );
                    aLhs += aName.getLength() ? UniString::CreateFromAscii(".y") : UniString::CreateFromAscii("y");
                    rList.push_back( HtmlSuccessfulObj(aLhs, aRhs) );

                }
            }
        } break;

        // CheckBoxen / RadioButtons
        case FormComponentType::CHECKBOX:
        case FormComponentType::RADIOBUTTON:
        {
            // <name>=<refValue>
            if( !hasProperty(PROPERTY_STATE, xComponentSet) )
                break;
            sal_Int16 nChecked;
            xComponentSet->getPropertyValue( PROPERTY_STATE ) >>= nChecked;
            if( nChecked != 1 )
                break;

            ::rtl::OUString aStrValue;
            if( hasProperty(PROPERTY_REFVALUE, xComponentSet) )
                xComponentSet->getPropertyValue( PROPERTY_REFVALUE ) >>= aStrValue;

            rList.push_back( HtmlSuccessfulObj(aName, aStrValue) );
        } break;

        // Edit
        case FormComponentType::TEXTFIELD:
        {
            // <name>=<text>
            if( !hasProperty(PROPERTY_TEXT, xComponentSet) )
                break;

            // MIB: Spezial-Behandlung fuer Multiline-Edit nur dann, wenn
            // es auch ein Control dazu gibt.
            Any aTmp = xComponentSet->getPropertyValue( PROPERTY_MULTILINE );
            sal_Bool bMulti =	rxSubmitButton.is()
                            && (aTmp.getValueType().getTypeClass() == TypeClass_BOOLEAN)
                            && getBOOL(aTmp);
            ::rtl::OUString sText;
            if ( bMulti )	// Bei MultiLineEdit Text am Control abholen
            {

                Reference<XControlContainer>  xControlContainer(rxSubmitButton->getContext(), UNO_QUERY);
                if( !xControlContainer.is() ) break;

                Sequence<Reference<XControl> > aControlSeq = xControlContainer->getControls();
                Reference<XControl>  xControl;
                Reference<XFormComponent>  xControlComponent;

                // Richtiges Control suchen
                sal_Int32 i;
                for( i=0; i<aControlSeq.getLength(); i++ )
                {
                    xControl = aControlSeq.getConstArray()[i];
                    Reference<XPropertySet>  xModel(xControl->getModel(), UNO_QUERY);
                    if (xModel == xComponentSet)
                    {
                        Reference<XTextComponent>  xTextComponent(xControl, UNO_QUERY);
                        if( xTextComponent.is() )
                            sText = xTextComponent->getText();
                        break;
                    }
                }
                // Control nicht gefunden oder nicht existent, (Edit im Grid)
                if (i == aControlSeq.getLength())
                    xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= sText;
            }
            else
                xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= sText;

            rList.push_back( HtmlSuccessfulObj(aName, sText) );
        } break;

        // ComboBox, Patternfield
        case FormComponentType::COMBOBOX:
        case FormComponentType::PATTERNFIELD:
        {
            // <name>=<text>
            if( hasProperty(PROPERTY_TEXT, xComponentSet) )
            {
                ::rtl::OUString aText;
                xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= aText;
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        } break;
        case FormComponentType::CURRENCYFIELD:
        case FormComponentType::NUMERICFIELD:
        {
            // <name>=<wert> // wert wird als double mit Punkt als Decimaltrenner
                             // kein Wert angegeben (NULL) -> wert leer
            if( hasProperty(PROPERTY_VALUE, xComponentSet) )
            {
                ::rtl::OUString aText;
                Any aVal  = xComponentSet->getPropertyValue( PROPERTY_VALUE );

                double aDoubleVal;
                if (aVal >>= aDoubleVal)
                {
                    sal_Int16 nScale;
                    xComponentSet->getPropertyValue( PROPERTY_DECIMAL_ACCURACY ) >>= nScale;
                    aText = ::rtl::math::doubleToUString(aDoubleVal, rtl_math_StringFormat_F, nScale, '.', sal_True);
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }	break;
        case FormComponentType::DATEFIELD:
        {
            // <name>=<wert> // Wert wird als Datum im Format (MM-DD-YYYY)
                             // kein Wert angegeben (NULL) -> wert leer
            if( hasProperty(PROPERTY_DATE, xComponentSet) )
            {
                ::rtl::OUString aText;
                Any aVal  = xComponentSet->getPropertyValue( PROPERTY_DATE );
                sal_Int32 nInt32Val;
                if (aVal >>= nInt32Val)
                {
                    ::Date aDate(nInt32Val);
                    char s[11];
                    sprintf(s,"%02d-%02d-%04d",
                    (int)aDate.GetMonth(),
                    (int)aDate.GetDay(),
                    (int)aDate.GetYear());
                    s[10] = 0;
                    aText = ::rtl::OUString::createFromAscii(s);
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }	break;
        case FormComponentType::TIMEFIELD:
        {
            // <name>=<wert> // Wert wird als Zeit im Format (HH:MM:SS) angegeben
                             // kein Wert angegeben (NULL) -> wert leer
            if( hasProperty(PROPERTY_TIME, xComponentSet) )
            {
                ::rtl::OUString aText;
                Any aVal  = xComponentSet->getPropertyValue( PROPERTY_TIME );
                sal_Int32 nInt32Val;
                if (aVal >>= nInt32Val)
                {
                    ::Time aTime(nInt32Val);
                    char s[10];
                    sprintf(s,"%02d:%02d:%02d",
                    (int)aTime.GetHour(),
                    (int)aTime.GetMin(),
                    (int)aTime.GetSec());
                    s[8] = 0;
                    aText = ::rtl::OUString::createFromAscii(s);
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }	break;

        // starform
        case FormComponentType::HIDDENCONTROL:
        {

            // <name>=<value>
            if( hasProperty(PROPERTY_HIDDEN_VALUE, xComponentSet) )
            {
                ::rtl::OUString aText;
                xComponentSet->getPropertyValue( PROPERTY_HIDDEN_VALUE ) >>= aText;
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        } break;

        // starform
        case FormComponentType::FILECONTROL:
        {
            // <name>=<text>
            if( hasProperty(PROPERTY_TEXT, xComponentSet) )
            {

                ::rtl::OUString aText;
                xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= aText;
                rList.push_back( HtmlSuccessfulObj(aName, aText, SUCCESSFUL_REPRESENT_FILE) );
            }
        } break;

        // starform
        case FormComponentType::LISTBOX:
        {

            // <name>=<Token0>&<name>=<Token1>&...&<name>=<TokenN> (Mehrfachselektion)
            if (!hasProperty(PROPERTY_SELECT_SEQ, xComponentSet) ||
                !hasProperty(PROPERTY_STRINGITEMLIST, xComponentSet))
                break;

            // angezeigte Werte
            Sequence< ::rtl::OUString > aVisibleList;
            xComponentSet->getPropertyValue( PROPERTY_STRINGITEMLIST ) >>= aVisibleList;
            sal_Int32 nStringCnt = aVisibleList.getLength();
            const ::rtl::OUString* pStrings = aVisibleList.getConstArray();

            // Werte-Liste
            Sequence< ::rtl::OUString > aValueList;
            xComponentSet->getPropertyValue( PROPERTY_VALUE_SEQ ) >>= aValueList;
            sal_Int32 nValCnt = aValueList.getLength();
            const ::rtl::OUString* pVals = aValueList.getConstArray();

            // Selektion
            Sequence<sal_Int16> aSelectList;
            xComponentSet->getPropertyValue( PROPERTY_SELECT_SEQ ) >>= aSelectList;
            sal_Int32 nSelCount = aSelectList.getLength();
            const sal_Int16* pSels = aSelectList.getConstArray();

            // Einfach- oder Mehrfach-Selektion
            // Bei Einfach-Selektionen beruecksichtigt MT nur den ersten Eintrag
            // in der Liste.
            if (nSelCount > 1 && !getBOOL(xComponentSet->getPropertyValue(PROPERTY_MULTISELECTION)))
                nSelCount = 1;

            // Die Indizes in der Selektions-Liste koennen auch ungueltig sein,
            // also muss man die gueltigen erstmal raussuchen um die Laenge
            // der neuen Liste zu bestimmen.
            sal_Int32 nCurCnt = 0;
            sal_Int32 i;
            for( i=0; i<nSelCount; ++i )
            {
                if( pSels[i] < nStringCnt )
                    ++nCurCnt;
            }

            ::rtl::OUString aSubValue;
            for(i=0; i<nCurCnt; ++i )
            {
                sal_Int16  nSelPos = pSels[i];
                // Wenn Index in WerteListe, Eintrag aus Werteliste holen
                // ansonsten angezeigten Wert nehmen. Ein
                // LISTBOX_EMPTY_VALUE entspricht einem leeren, aber
                // vorhandenem VALUE des Option-Tags.
                if (nSelPos < nValCnt && pVals[nSelPos].getLength())
                {
                    if( pVals[nSelPos] == LISTBOX_EMPTY_VALUE )
                        aSubValue = ::rtl::OUString();
                    else
                        aSubValue = pVals[nSelPos];
                }
                else
                {
                    aSubValue = pStrings[nSelPos];
                }
                rList.push_back( HtmlSuccessfulObj(aName, aSubValue) );
            }
        } break;
        case FormComponentType::GRIDCONTROL:
        {
            // Die einzelnen Spaltenwerte werden verschickt,
            // der Name wird mit dem Prefix des Names des Grids erweitert
            Reference<XIndexAccess>  xContainer(xComponentSet, UNO_QUERY);
            if (!xContainer.is())
                break;

            aName += UniString('.');

            Reference<XPropertySet>  xSet;
            sal_Int32 nCount = xContainer->getCount();
            // we know already how many objects should be appended,
            // so why not allocate the space for them
            rList.reserve( nCount + rList.capacity() ); // not size()
            for (sal_Int32 i = 0; i < nCount; ++i)
            {
                xContainer->getByIndex(i) >>= xSet;
                if (xSet.is())
                    AppendComponent(rList, xSet, aName, rxSubmitButton, MouseEvt);
            }
        }
    }
}

//------------------------------------------------------------------------
void ODatabaseForm::FillSuccessfulList( HtmlSuccessfulObjList& rList,
    const Reference<XControl>& rxSubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt )
{
    // Liste loeschen
    rList.clear();
    // Ueber Components iterieren
    Reference<XPropertySet> 		xComponentSet;
    ::rtl::OUString aPrefix;

    // we know already how many objects should be appended,
    // so why not allocate the space for them
    rList.reserve( getCount() );
    for( sal_Int32 nIndex=0; nIndex < getCount(); nIndex++ )
    {
        getByIndex( nIndex ) >>= xComponentSet;
        AppendComponent(rList, xComponentSet, aPrefix, rxSubmitButton, MouseEvt);
    }
}

//------------------------------------------------------------------------
void ODatabaseForm::Encode( ::rtl::OUString& rString ) const
{
    ::rtl::OUString aResult;

    // Immer ANSI #58641
//	rString.Convert(CHARSET_SYSTEM, CHARSET_ANSI);


    // Zeilenendezeichen werden als CR dargestellt
    UniString sConverter = rString;
    sConverter.ConvertLineEnd( LINEEND_CR );
    rString = sConverter;


    // Jeden einzelnen Character ueberpruefen
    sal_Int32 nStrLen = rString.getLength();
    sal_Unicode nCharCode;
    for( sal_Int32 nCurPos=0; nCurPos < nStrLen; ++nCurPos )
    {
        nCharCode = rString[nCurPos];

        // Behandlung fuer chars, die kein alphanumerisches Zeichen sind
        // und CharacterCodes > 127
        if( (!isalnum(nCharCode) && nCharCode != (sal_Unicode)' ') || nCharCode > 127 )
        {
            switch( nCharCode )
            {
                case 13:	// CR
                    aResult += ::rtl::OUString::createFromAscii("%0D%0A");	// Hex-Darstellung CR LF
                    break;


                // Netscape Sonderbehandlung
                case 42:	// '*'
                case 45:	// '-'
                case 46:	// '.'
                case 64:	// '@'
                case 95:	// '_'
                    aResult += UniString(nCharCode);
                    break;

                default:
                {
                    // In Hex umrechnen
                    short nHi = ((sal_Int16)nCharCode) / 16;
                    short nLo = ((sal_Int16)nCharCode) - (nHi*16);
                    if( nHi > 9 ) nHi += (int)'A'-10; else nHi += (int)'0';
                    if( nLo > 9 ) nLo += (int)'A'-10; else nLo += (int)'0';
                    aResult += UniString('%');
                    aResult += UniString((sal_Unicode)nHi);
                    aResult += UniString((sal_Unicode)nLo);
                }
            }
        }
        else
            aResult += UniString(nCharCode);
    }


    // Spaces durch	'+' ersetzen
    aResult = aResult.replace(' ', '+');

    rString = aResult;
}

//------------------------------------------------------------------------
void ODatabaseForm::InsertTextPart( INetMIMEMessage& rParent, const ::rtl::OUString& rName,
    const ::rtl::OUString& rData )
{

    // Part als Message-Child erzeugen
    INetMIMEMessage* pChild = new INetMIMEMessage();


    // Header
    ::rtl::OUString aContentDisp = ::rtl::OUString::createFromAscii("form-data; name=\"");
    aContentDisp += rName;
    aContentDisp += UniString('\"');
    pChild->SetContentDisposition( aContentDisp );
    pChild->SetContentType( UniString::CreateFromAscii("text/plain") );

    rtl_TextEncoding eSystemEncoding = gsl_getSystemTextEncoding();
    const sal_Char* pBestMatchingEncoding = rtl_getBestMimeCharsetFromTextEncoding( eSystemEncoding );
    UniString aBestMatchingEncoding = UniString::CreateFromAscii( pBestMatchingEncoding );
    pChild->SetContentTransferEncoding(aBestMatchingEncoding);

    // Body
    SvMemoryStream* pStream = new SvMemoryStream;
    pStream->WriteLine( ByteString( UniString(rData), rtl_getTextEncodingFromMimeCharset(pBestMatchingEncoding) ) );
    pStream->Flush();
    pStream->Seek( 0 );
    pChild->SetDocumentLB( new SvLockBytes(pStream, sal_True) );
    rParent.AttachChild( *pChild );
}

//------------------------------------------------------------------------
sal_Bool ODatabaseForm::InsertFilePart( INetMIMEMessage& rParent, const ::rtl::OUString& rName,
    const ::rtl::OUString& rFileName )
{
    UniString aFileName( rFileName );
    UniString aContentType(UniString::CreateFromAscii(CONTENT_TYPE_STR_TEXT_PLAIN));
    SvStream *pStream = 0;

    if( aFileName.Len() )
    {
        // Bisher koennen wir nur File-URLs verarbeiten
        INetURLObject aURL;
        aURL.SetSmartProtocol(INET_PROT_FILE);
        aURL.SetSmartURL(rFileName);
        if( INET_PROT_FILE == aURL.GetProtocol() )
        {
            aFileName = INetURLObject::decode(aURL.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
            DirEntry aDirEntry( aFileName );
            if( aDirEntry.Exists() )
            {
                pStream = ::utl::UcbStreamHelper::CreateStream(aFileName, STREAM_READ);
                if (!pStream || (pStream->GetError() != ERRCODE_NONE))
                {
                    delete pStream;
                    pStream = 0;
                }
            }
            INetContentType eContentType = INetContentTypes::GetContentType4Extension(
                                                                aDirEntry.GetExtension() );
            if (eContentType != CONTENT_TYPE_UNKNOWN)
                aContentType = INetContentTypes::GetContentType(eContentType);
        }
    }

    // Wenn irgendetwas nicht geklappt hat, legen wir einen leeren
    // MemoryStream an
    if( !pStream )
        pStream = new SvMemoryStream;


    // Part als Message-Child erzeugen
    INetMIMEMessage* pChild = new INetMIMEMessage;


    // Header
    ::rtl::OUString aContentDisp = ::rtl::OUString::createFromAscii( "form-data; name=\"" );
    aContentDisp += rName;
    aContentDisp += UniString('\"');
    aContentDisp += ::rtl::OUString::createFromAscii("; filename=\"");
    aContentDisp += aFileName;
    aContentDisp += UniString('\"');
    pChild->SetContentDisposition( aContentDisp );
    pChild->SetContentType( aContentType );
    pChild->SetContentTransferEncoding( UniString(::rtl::OUString::createFromAscii("8bit")) );


    // Body
    pChild->SetDocumentLB( new SvLockBytes(pStream, sal_True) );
    rParent.AttachChild( *pChild );

    return sal_True;
}

//==============================================================================
// internals
//------------------------------------------------------------------------------
void ODatabaseForm::onError(const SQLErrorEvent& _rEvent)
{
    m_aErrorListeners.notifyEach(&XSQLErrorListener::errorOccured, _rEvent);
}

//------------------------------------------------------------------------------
void ODatabaseForm::onError(SQLException& _rException, const ::rtl::OUString& _rContextDescription)
{
    DBG_ERROR( "ODatabaseForm::onError: dead code!" );
}

//------------------------------------------------------------------------------
void ODatabaseForm::createParameterInfo()
{
    DBG_ERROR( "ODatabaseForm::createParameterInfo: why is this still used in binfilter?!!" );
}

//------------------------------------------------------------------------------
bool ODatabaseForm::hasValidParent() const
{
    // do we have to fill the parameters again?
    if (m_bSubForm)
    {
        Reference<XResultSet>  xResultSet(m_xParent, UNO_QUERY);
        if (!xResultSet.is())
        {
            DBG_ERROR("ODatabaseForm::hasValidParent() : no parent resultset !");
            return false;
        }
        try
        {
            Reference< XPropertySet >  xSet( m_xParent, UNO_QUERY );
            Reference< XLoadable > xLoad( m_xParent, UNO_QUERY );
            if	(	xLoad->isLoaded()
                &&	(	xResultSet->isBeforeFirst()
                    ||	xResultSet->isAfterLast()
                    ||	getBOOL( xSet->getPropertyValue( PROPERTY_ISNEW ) )
                    )
                )
                // the parent form is loaded and on a "virtual" row -> not valid
                return false;
        }
        catch(Exception&)
        {
            // parent could be forwardonly?
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
bool ODatabaseForm::fillParameters(::osl::ResettableMutexGuard& _rClearForNotifies, const Reference< XInteractionHandler >& _rxCompletionHandler)
{
    Reference<XParameters>  xExecutionParams;
    if (!query_aggregation( m_xAggregate, xExecutionParams))
    {
        DBG_ERROR("ODatabaseForm::fillParameters : invalid row set (doesn't support parameters) !");
        // normally the row set should support parameters ...
        return true;
    }

    // do we have to fill the parameters again?
    if ( !m_pParameterInfo )
        createParameterInfo();

    if (!m_pParameterInfo || m_pParameterInfo->nCount == 0)
        return true;

    // is there a valid parent?
    if (m_bSubForm && !hasValidParent())
        return true;

    // do we have to fill the parent parameters?
    OParametersImpl::Parameters& rParams = m_pParameterInfo->pParameters->getParameters();

    // do we have to fill the parent parameters?
    if (m_pParameterInfo->nCount > (sal_Int32)rParams.size())
    {
        Reference<XColumnsSupplier>  xParentColsSuppl(m_xParent, UNO_QUERY);
        if (xParentColsSuppl.is())
        {
            Reference<XNameAccess>  xParentCols = xParentColsSuppl->getColumns();
            sal_Int32 nMasterLen = m_aMasterFields.getLength();
            if (xParentCols->hasElements() && (nMasterLen > 0))
            {
                const ::rtl::OUString* pMasterFields = m_aMasterFields.getConstArray();
                const ::rtl::OUString* pDetailFields = m_aDetailFields.getConstArray();

                Any aParamType,aScale,aValue;
                for (sal_Int32 i = 0; i < nMasterLen; ++i, ++pMasterFields, ++pDetailFields)
                {
                    Reference<XPropertySet>  xMasterField, xDetailField;
                    MapUString2INT32::const_iterator aFind;
                    if ( xParentCols->hasByName(*pMasterFields) &&
                        ( aFind = m_pParameterInfo->aParamMapping.find(*pDetailFields)) != m_pParameterInfo->aParamMapping.end() )
                    {
                        // parameter defined by master slave definition
                        ::cppu::extractInterface(xMasterField, xParentCols->getByName(*pMasterFields));
                        do
                        {
                            m_pParameterInfo->xParamsAsIndex->getByIndex(aFind->second) >>= xDetailField;
                            OSL_ENSURE(xDetailField.is(),"ParameterField is not valid!");

                            // get the type of the param
                            aParamType = xDetailField->getPropertyValue(PROPERTY_FIELDTYPE);
                            DBG_ASSERT(aParamType.getValueType().getTypeClass() == TypeClass_LONG, "ODatabaseForm::fillParameters : invalid parameter field !");
                            sal_Int32 nScale = 0;
                            if (hasProperty(PROPERTY_SCALE, xDetailField))
                            {
                                aScale = xDetailField->getPropertyValue(PROPERTY_SCALE);
                                DBG_ASSERT(aScale.getValueType().getTypeClass() == TypeClass_LONG, "ODatabaseForm::fillParameters : invalid parameter field !");
                                nScale = getINT32(aScale);
                            }
                            // and fill the param value
                            aValue = xMasterField->getPropertyValue(PROPERTY_VALUE);
                            // parameters are based at 1
                            try
                            {
                                xExecutionParams->setObjectWithInfo(aFind->second + 1, aValue, getINT32(aParamType), nScale);
                            }
                            catch( const Exception& )
                            {
                                OSL_ENSURE( sal_False,
                                        ::rtl::OString( "ODatabaseForm::fillParameters: master-detail parameter number " )
                                        +=	::rtl::OString::valueOf( sal_Int32(aFind->second + 1) ));
                            }
                            ++aFind;
                        }
                        while ( aFind != m_pParameterInfo->aParamMapping.end() );
                    }
                    else
                        // no column matching so leave the parameter setting
                        return true;
                }
            }
        }
    }

    // now fill the remaining params
    bool bCanceled = false;
    if (_rxCompletionHandler.is())
    {
        // ask the handler for the missing parameters

        // ensure we're connected
        if (!implEnsureConnection())
            return sal_False;

        // two continuations (Ok and Cancel)
        OInteractionAbort* pAbort = new OInteractionAbort;
        OParameterContinuation* pParams = new OParameterContinuation;

        // the request
        ParametersRequest aRequest;
        aRequest.Parameters = m_pParameterInfo->pParameters;
        aRequest.Connection = getConnection();
        OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aRequest));
        Reference< XInteractionRequest > xRequest(pRequest);

        // some knittings
        pRequest->addContinuation(pAbort);
        pRequest->addContinuation(pParams);

        // execute the request
        _rxCompletionHandler->handle(xRequest);

        if (!pParams->wasSelected())
            // canceled by the user (i.e. (s)he canceled the dialog)
            return sal_False;

        // transfer the values from the continuation object to the parameter columns
        Sequence< PropertyValue > aFinalValues = pParams->getValues();
        const PropertyValue* pFinalValues = aFinalValues.getConstArray();
        for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
        {
            Reference< XPropertySet > xParamColumn;
            ::cppu::extractInterface(xParamColumn, aRequest.Parameters->getByIndex(i));
            if (xParamColumn.is())
            {
#ifdef DBG_UTIL
                ::rtl::OUString sName;
                xParamColumn->getPropertyValue(PROPERTY_NAME) >>= sName;
                DBG_ASSERT(sName.equals(pFinalValues->Name), "ODatabaseForm::fillParameters: inconsistent parameter names!");
#endif
                xParamColumn->setPropertyValue(PROPERTY_VALUE, pFinalValues->Value);
                    // the property sets are wrapper classes, translating the Value property into a call to
                    // the appropriate XParameters interface
            }
        }
    }
    else
    {
        sal_Int32 nParamsLeft = rParams.size();
        if (nParamsLeft)
        {
            ::cppu::OInterfaceIteratorHelper aIter(m_aParameterListeners);
            DatabaseParameterEvent aEvt(static_cast<XWeak*>(this), m_pParameterInfo->pParameters);

            _rClearForNotifies.clear();
            while (aIter.hasMoreElements() && !bCanceled)
                bCanceled = !((XDatabaseParameterListener*)aIter.next())->approveParameter(aEvt);
            _rClearForNotifies.reset();
        }
    }
    return !bCanceled;
}

//------------------------------------------------------------------------------
void ODatabaseForm::saveInsertOnlyState( )
{
    OSL_ENSURE( !m_aIgnoreResult.hasValue(), "ODatabaseForm::saveInsertOnlyState: overriding old value!" );
    m_aIgnoreResult = m_xAggregateSet->getPropertyValue( PROPERTY_INSERTONLY );
}

//------------------------------------------------------------------------------
void ODatabaseForm::restoreInsertOnlyState( )
{
    if ( m_aIgnoreResult.hasValue() )
    {
        m_xAggregateSet->setPropertyValue( PROPERTY_INSERTONLY, m_aIgnoreResult );
        m_aIgnoreResult = Any();
    }
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseForm::executeRowSet(::osl::ResettableMutexGuard& _rClearForNotifies, sal_Bool bMoveToFirst, const Reference< XInteractionHandler >& _rxCompletionHandler)
{
    if (!m_xAggregateAsRowSet.is())
        return sal_False;

    if (!fillParameters(_rClearForNotifies, _rxCompletionHandler))
        return sal_False;

    restoreInsertOnlyState( );

    // ensure the aggregated row set has the correct properties
    sal_Int32 nConcurrency;
    // if we have a parent, who is not positioned on a valid row
    // we can't be updatable!
    if (m_bSubForm && !hasValidParent())
    {
        // don't use any parameters if we don't have a valid parent
        nConcurrency = ResultSetConcurrency::READ_ONLY;
        //	clearParameters();
        if(m_pParameterInfo && m_pParameterInfo->nCount > 0)
        {
            // reset all parameter values
            // (fs@openoffice.org: what is the difference to the "clearParameter" call which was removed
            // in revision 1.39, and replaced with the lines below??)
            Reference<XParameters>  xExecutionParams;
            query_aggregation( m_xAggregate, xExecutionParams);
            for (sal_Int32 nPos=1;nPos <= m_pParameterInfo->nCount; ++nPos)
                xExecutionParams->setNull(nPos,DataType::VARCHAR);

            // switch to "insert only" mode
            saveInsertOnlyState( );
            m_xAggregateSet->setPropertyValue( PROPERTY_INSERTONLY, makeAny( sal_True ) );
        }
    }
    else if (m_bAllowInsert || m_bAllowUpdate || m_bAllowDelete)
        nConcurrency = ResultSetConcurrency::UPDATABLE;
    else
        nConcurrency = ResultSetConcurrency::READ_ONLY;

    m_xAggregateSet->setPropertyValue(PROPERTY_RESULTSET_CONCURRENCY, makeAny(nConcurrency));

    sal_Int32 nResultSetType = ResultSetType::SCROLL_SENSITIVE;
    m_xAggregateSet->setPropertyValue(PROPERTY_RESULTSET_TYPE, makeAny(nResultSetType));

    sal_Bool bSuccess = sal_False;
    try
    {
        m_xAggregateAsRowSet->execute();
        bSuccess = sal_True;
    }
    catch(RowSetVetoException& eVeto)
    {
        eVeto;
    }
    catch(SQLException& eDb)
    {
        _rClearForNotifies.clear();
        if (m_sCurrentErrorContext.getLength())
            onError(eDb, m_sCurrentErrorContext);
        else
            onError(eDb, FRM_RES_STRING(RID_STR_READERROR));
        _rClearForNotifies.reset();
    }

    if (bSuccess)
    {
        // adjust the privilege property
        //	m_nPrivileges;
        m_xAggregateSet->getPropertyValue(PROPERTY_PRIVILEGES) >>= m_nPrivileges;
        if (!m_bAllowInsert)
            m_nPrivileges &= ~Privilege::INSERT;
        if (!m_bAllowUpdate)
            m_nPrivileges &= ~Privilege::UPDATE;
        if (!m_bAllowDelete)
            m_nPrivileges &= ~Privilege::DELETE;

        if (bMoveToFirst)
        {
            // the row set is positioned _before_ the first row (per definitionem), so move the set ...
            try
            {
                // if we have an insert only rowset we move to the insert row
                next();
                if (((m_nPrivileges & Privilege::INSERT) == Privilege::INSERT)
                    && isAfterLast())
                {
                    // move on the insert row of set
                    // resetting must be done later, after the load events have been posted
                    // see :moveToInsertRow and load , reload
                    Reference<XResultSetUpdate>  xUpdate;
                    if (query_aggregation( m_xAggregate, xUpdate))
                        xUpdate->moveToInsertRow();
                }
            }
            catch(SQLException& eDB)
            {
                _rClearForNotifies.clear();
                if (m_sCurrentErrorContext.getLength())
                    onError(eDB, m_sCurrentErrorContext);
                else
                    onError(eDB, FRM_RES_STRING(RID_STR_READERROR));
                _rClearForNotifies.reset();
                bSuccess = sal_False;
            }
        }
    }
    return bSuccess;
}

//------------------------------------------------------------------
void ODatabaseForm::disposing()
{
    if (m_pAggregatePropertyMultiplexer)
        m_pAggregatePropertyMultiplexer->dispose();

    if (m_bLoaded)
        unload();

    // cancel the submit/reset-thread
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (m_pThread)
        {
            m_pThread->release();
            m_pThread = NULL;
        }
    }

    EventObject aEvt(static_cast<XWeak*>(this));
    m_aLoadListeners.disposeAndClear(aEvt);
    m_aRowSetApproveListeners.disposeAndClear(aEvt);
    m_aParameterListeners.disposeAndClear(aEvt);
    m_aResetListeners.disposeAndClear(aEvt);
    m_aSubmitListeners.disposeAndClear(aEvt);
    m_aErrorListeners.disposeAndClear(aEvt);

    OFormComponents::disposing();
    OPropertySetAggregationHelper::disposing();

    // stop listening on the aggregate
    if (m_xAggregateAsRowSet.is())
        m_xAggregateAsRowSet->removeRowSetListener(this);

    // dispose the active connection
    Reference<XComponent>  xAggregationComponent;
    if (query_aggregation(m_xAggregate, xAggregationComponent))
        xAggregationComponent->dispose();
}

//------------------------------------------------------------------------------
Reference< XConnection >  ODatabaseForm::getConnection()
{
    Reference< XConnection > xConn;
    m_xAggregateSet->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConn;
    return xConn;
}

//==============================================================================
// property handling
//------------------------------------------------------------------------------
void ODatabaseForm::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    BEGIN_AGGREGATION_PROPERTY_HELPER(15, m_xAggregateSet)
        // this property is overwritten by the form
        RemoveProperty(_rAggregateProps, PROPERTY_PRIVILEGES);
        RemoveProperty(_rAggregateProps, PROPERTY_DATASOURCE);
        RemoveProperty(_rAggregateProps, PROPERTY_ACTIVE_CONNECTION);
            // we remove and re-declare the DataSourceName property, 'cause we want it to be constrained, and the
            // original property of our aggregate isn't

        DECL_IFACE_PROP3(ACTIVE_CONNECTION,			XConnection,	BOUND, TRANSIENT, MAYBEVOID);
        DECL_PROP1(NAME,			::rtl::OUString,				BOUND);
        DECL_PROP1(MASTERFIELDS,	Sequence< ::rtl::OUString >,					BOUND);
        DECL_PROP1(DETAILFIELDS,	Sequence< ::rtl::OUString >,					BOUND);
        DECL_PROP2(DATASOURCE,		::rtl::OUString,				BOUND, CONSTRAINED);
        DECL_PROP3(CYCLE,			TabulatorCycle,					BOUND, MAYBEVOID, MAYBEDEFAULT);
        DECL_PROP1(NAVIGATION,		NavigationBarMode,				BOUND);
        DECL_BOOL_PROP1(ALLOWADDITIONS,								BOUND);
        DECL_BOOL_PROP1(ALLOWEDITS,									BOUND);
        DECL_BOOL_PROP1(ALLOWDELETIONS,								BOUND);
        DECL_PROP2(PRIVILEGES,		sal_Int32,						TRANSIENT, READONLY);
        DECL_PROP1(TARGET_URL,		::rtl::OUString,				BOUND);
        DECL_PROP1(TARGET_FRAME,	::rtl::OUString,				BOUND);
        DECL_PROP1(SUBMIT_METHOD,	FormSubmitMethod,		BOUND);
        DECL_PROP1(SUBMIT_ENCODING,	FormSubmitEncoding,	BOUND);
    END_AGGREGATION_PROPERTY_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ODatabaseForm::getInfoHelper()
{
    return *const_cast<ODatabaseForm*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo>  ODatabaseForm::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xDescriptorInfo( createPropertySetInfo( getInfoHelper() ) );
    return xDescriptorInfo;
}

//------------------------------------------------------------------------------
void ODatabaseForm::fire( sal_Int32* pnHandles, const Any* pNewValues, const Any* pOldValues, sal_Int32 nCount, sal_Bool bVetoable )
{
    // same as in getFastPropertyValue(INT32) : if we're resetting currently don't fire any changes of the
    // IsModified property from FALSE to TRUE, as this is only temporary 'til the reset is done
    if (m_nResetsPending > 0)
    {
        // look for the PROPERTY_ID_ISMODIFIED
        sal_Int32 nPos = 0;
        for (nPos=0; nPos<nCount; ++nPos)
            if (pnHandles[nPos] == PROPERTY_ID_ISMODIFIED)
                break;

        if ((nPos < nCount) && (pNewValues[nPos].getValueType().getTypeClass() == TypeClass_BOOLEAN) && getBOOL(pNewValues[nPos]))
        {	// yeah, we found it, and it changed to TRUE
            if (nPos == 0)
            {	// just cut the first element
                ++pnHandles;
                ++pNewValues;
                ++pOldValues;
                --nCount;
            }
            else if (nPos == nCount - 1)
                // just cut the last element
                --nCount;
            else
            {	// split into two base class calls
                OPropertySetAggregationHelper::fire(pnHandles, pNewValues, pOldValues, nPos, bVetoable);
                ++nPos;
                OPropertySetAggregationHelper::fire(pnHandles + nPos, pNewValues + nPos, pOldValues + nPos, nCount - nPos, bVetoable);
                return;
            }
        }
    }

    OPropertySetAggregationHelper::fire(pnHandles, pNewValues, pOldValues, nCount, bVetoable);
}

//------------------------------------------------------------------------------
Any SAL_CALL ODatabaseForm::getFastPropertyValue( sal_Int32 nHandle )
       throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    if ((nHandle == PROPERTY_ID_ISMODIFIED) && (m_nResetsPending > 0))
        return ::cppu::bool2any((sal_False));
        // don't allow the aggregate which is currently reset to return a (temporary) "yes"
    else
        return OPropertySetAggregationHelper::getFastPropertyValue(nHandle);
}

//------------------------------------------------------------------------------
void ODatabaseForm::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_DATASOURCE:
        {
            rValue = makeAny(::rtl::OUString());
            try
            {
                rValue = m_xAggregateSet->getPropertyValue(PROPERTY_DATASOURCE);
            }
            catch(Exception&) { }
        }
        break;
        case PROPERTY_ID_ACTIVE_CONNECTION:
        {
            try
            {
                rValue = m_xAggregateSet->getPropertyValue(PROPERTY_ACTIVE_CONNECTION);
            }
            catch(Exception&) { }
        }
        break;
        case PROPERTY_ID_TARGET_URL:
            rValue <<= m_aTargetURL;
            break;
        case PROPERTY_ID_TARGET_FRAME:
            rValue <<= m_aTargetFrame;
            break;
        case PROPERTY_ID_SUBMIT_METHOD:
            rValue <<= m_eSubmitMethod;
            break;
        case PROPERTY_ID_SUBMIT_ENCODING:
            rValue <<= m_eSubmitEncoding;
            break;
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
        case PROPERTY_ID_MASTERFIELDS:
            rValue <<= m_aMasterFields;
            break;
        case PROPERTY_ID_DETAILFIELDS:
            rValue <<= m_aDetailFields;
            break;
        case PROPERTY_ID_CYCLE:
            rValue = m_aCycle;
            break;
        case PROPERTY_ID_NAVIGATION:
            rValue <<= m_eNavigation;
            break;
        case PROPERTY_ID_ALLOWADDITIONS:
            rValue <<= (sal_Bool)m_bAllowInsert;
            break;
        case PROPERTY_ID_ALLOWEDITS:
            rValue <<= (sal_Bool)m_bAllowUpdate;
            break;
        case PROPERTY_ID_ALLOWDELETIONS:
            rValue <<= (sal_Bool)m_bAllowDelete;
            break;
        case PROPERTY_ID_PRIVILEGES:
            rValue <<= (sal_Int32)m_nPrivileges;
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseForm::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue,
                                                sal_Int32 nHandle, const Any& rValue ) throw( IllegalArgumentException )
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_DATASOURCE:
        {
            Any aAggregateProperty;
            getFastPropertyValue(aAggregateProperty, PROPERTY_ID_DATASOURCE);
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, aAggregateProperty, ::getCppuType(static_cast<const ::rtl::OUString*>(NULL)));
        }
        break;
        case PROPERTY_ID_ACTIVE_CONNECTION:
        {
            Any aAggregateProperty;
            getFastPropertyValue(aAggregateProperty, PROPERTY_ID_ACTIVE_CONNECTION);
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, aAggregateProperty, ::getCppuType(static_cast<const Reference<XConnection>*>(NULL)));
        }
        break;
        case PROPERTY_ID_TARGET_URL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTargetURL);
            break;
        case PROPERTY_ID_TARGET_FRAME:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTargetFrame);
            break;
        case PROPERTY_ID_SUBMIT_METHOD:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_eSubmitMethod);
            break;
        case PROPERTY_ID_SUBMIT_ENCODING:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_eSubmitEncoding);
            break;
        case PROPERTY_ID_NAME:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sName);
            break;
        case PROPERTY_ID_MASTERFIELDS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aMasterFields);
            break;
        case PROPERTY_ID_DETAILFIELDS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aDetailFields);
            break;
        case PROPERTY_ID_CYCLE:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aCycle, ::getCppuType(static_cast<const TabulatorCycle*>(NULL)));
            break;
        case PROPERTY_ID_NAVIGATION:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_eNavigation);
            break;
        case PROPERTY_ID_ALLOWADDITIONS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAllowInsert);
            break;
        case PROPERTY_ID_ALLOWEDITS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAllowUpdate);
            break;
        case PROPERTY_ID_ALLOWDELETIONS:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAllowDelete);
            break;
        default:
            DBG_ERROR("ODatabaseForm::convertFastPropertyValue : unknown property !");
    }
    return bModified;
}

//------------------------------------------------------------------------------
void ODatabaseForm::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw( Exception )
{
    switch (nHandle)
    {
        case PROPERTY_ID_DATASOURCE:
            try
            {
                m_xAggregateSet->setPropertyValue(PROPERTY_DATASOURCE, rValue);
            }
            catch(Exception&) { }
            break;
        case PROPERTY_ID_ACTIVE_CONNECTION:
            try
            {
                if ( m_bSharingConnection )
                    stopSharingConnection( );

                m_bForwardingConnection = sal_True;
                m_xAggregateSet->setPropertyValue(PROPERTY_ACTIVE_CONNECTION, rValue);
                m_bForwardingConnection = sal_False;
            }
            catch(Exception&) { }
            break;
        case PROPERTY_ID_TARGET_URL:
            rValue >>= m_aTargetURL;
            break;
        case PROPERTY_ID_TARGET_FRAME:
            rValue >>= m_aTargetFrame;
            break;
        case PROPERTY_ID_SUBMIT_METHOD:
            rValue >>= m_eSubmitMethod;
            break;
        case PROPERTY_ID_SUBMIT_ENCODING:
            rValue >>= m_eSubmitEncoding;
            break;
        case PROPERTY_ID_NAME:
            rValue >>= m_sName;
            break;
        case PROPERTY_ID_MASTERFIELDS:
            rValue >>= m_aMasterFields;
            invlidateParameters();
            break;
        case PROPERTY_ID_DETAILFIELDS:
            rValue >>= m_aDetailFields;
            invlidateParameters();
            break;
        case PROPERTY_ID_CYCLE:
            m_aCycle = rValue;
            break;
        case PROPERTY_ID_NAVIGATION:
            rValue >>= m_eNavigation;
            break;
        case PROPERTY_ID_ALLOWADDITIONS:
            m_bAllowInsert = getBOOL(rValue);
            break;
        case PROPERTY_ID_ALLOWEDITS:
            m_bAllowUpdate = getBOOL(rValue);
            break;
        case PROPERTY_ID_ALLOWDELETIONS:
            m_bAllowDelete = getBOOL(rValue);
            break;
        default:
            DBG_ERROR("ODatabaseForm::setFastPropertyValue_NoBroadcast : unknown property !");
    }
}

//==============================================================================
// com::sun::star::beans::XPropertyState
//------------------------------------------------------------------
PropertyState ODatabaseForm::getPropertyStateByHandle(sal_Int32 nHandle)
{
    PropertyState eState;
    switch (nHandle)
    {
        case PROPERTY_ID_NAVIGATION:
            return (NavigationBarMode_CURRENT == m_eNavigation) ? PropertyState_DEFAULT_VALUE : PropertyState_DIRECT_VALUE;
            break;
        case PROPERTY_ID_CYCLE:
            if (!m_aCycle.hasValue())
                eState = PropertyState_DEFAULT_VALUE;
            else
                eState = PropertyState_DIRECT_VALUE;
            break;
        default:
            eState = OPropertySetAggregationHelper::getPropertyStateByHandle(nHandle);
    }
    return eState;
}

//------------------------------------------------------------------
void ODatabaseForm::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAVIGATION:
            setFastPropertyValue(nHandle, makeAny(NavigationBarMode_CURRENT));
            break;
        case PROPERTY_ID_CYCLE:
            setFastPropertyValue(nHandle, Any());
            break;
        default:
            OPropertySetAggregationHelper::setPropertyToDefaultByHandle(nHandle);
    }
}

//------------------------------------------------------------------
Any ODatabaseForm::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAVIGATION:
            return makeAny(NavigationBarMode_CURRENT);
            break;
        case PROPERTY_ID_CYCLE:
            return Any();
        default:
            return OPropertySetAggregationHelper::getPropertyDefaultByHandle(nHandle);
    }
}

//==============================================================================
// com::sun::star::form::XReset
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reset() throw( RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    if (isLoaded())
    {
        ::osl::MutexGuard aResetGuard(m_aResetSafety);
        ++m_nResetsPending;
        reset_impl(true);
        return;
    }

    if (m_aResetListeners.getLength())
    {
        ::osl::MutexGuard aResetGuard(m_aResetSafety);
        ++m_nResetsPending;
        // create an own thread if we have (approve-)reset-listeners (so the listeners can't do that much damage
        // to this thread which is probably the main one)
        if (!m_pThread)
        {
            m_pThread = new OFormSubmitResetThread(this);
            m_pThread->acquire();
            m_pThread->create();
        }
        EventObject aEvt;
        m_pThread->addEvent(&aEvt, sal_False);
    }
    else
    {
        // direct call without any approving by the listeners
        aGuard.clear();

        ::osl::MutexGuard aResetGuard(m_aResetSafety);
        ++m_nResetsPending;
        reset_impl(false);
    }
}

//-----------------------------------------------------------------------------
void ODatabaseForm::reset_impl(bool _bAproveByListeners)
{
    if (_bAproveByListeners)
    {
        bool bCanceled = false;
        ::cppu::OInterfaceIteratorHelper aIter(m_aResetListeners);
        EventObject aEvt(static_cast<XWeak*>(this));

        while (aIter.hasMoreElements() && !bCanceled)
            if (!((XResetListener*)aIter.next())->approveReset(aEvt))
                bCanceled = true;
        if (bCanceled)
            return;
    }

    ::osl::ResettableMutexGuard aResetGuard(m_aResetSafety);
    // do we have a database connected form and stay on the insert row
    sal_Bool bInsertRow = sal_False;
    if (m_xAggregateSet.is())
        bInsertRow = getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ISNEW));
    if (bInsertRow)
    {
        try
        {
            // Iterate through all columns and set the default value
            Reference< XColumnsSupplier > xColsSuppl( m_xAggregateSet, UNO_QUERY );
            Reference< XIndexAccess > xIndexCols( xColsSuppl->getColumns(), UNO_QUERY );
            for (sal_Int32 i = 0; i < xIndexCols->getCount(); ++i)
            {
                Reference< XPropertySet > xColProps;
                xIndexCols->getByIndex(i) >>= xColProps;

                Reference< XColumnUpdate > xColUpdate( xColProps, UNO_QUERY );
                if ( !xColUpdate.is() )
                    continue;

                Reference< XPropertySetInfo > xPSI;
                if ( xColProps.is() )
                    xPSI = xColProps->getPropertySetInfo( );

                static const ::rtl::OUString PROPERTY_CONTROLDEFAULT( RTL_CONSTASCII_USTRINGPARAM( "ControlDefault" ) );
                if ( xPSI.is() && xPSI->hasPropertyByName( PROPERTY_CONTROLDEFAULT ) )
                {
                    Any aDefault = xColProps->getPropertyValue( PROPERTY_CONTROLDEFAULT );

                    sal_Bool bReadOnly = sal_False;
                    if ( xPSI->hasPropertyByName( PROPERTY_ISREADONLY ) )
                        xColProps->getPropertyValue( PROPERTY_ISREADONLY ) >>= bReadOnly;

                    if ( !bReadOnly )
                    {
                        try
                        {
                            if ( aDefault.hasValue() )
                                xColUpdate->updateObject( aDefault );
                        }
                        catch(Exception&)
                        {
                        }
                    }
                }
            }
        }
        catch(Exception&)
        {
        }

        if (m_bSubForm)
        {
            // Iterate through all columns and set the default value
            Reference<XColumnsSupplier>  xColsSuppl(m_xAggregateSet, UNO_QUERY);

            // now set the values on which a subform depends
            Reference<XColumnsSupplier>  xParentColsSuppl(m_xParent, UNO_QUERY);
            Reference<XNameAccess>  xParentCols = xParentColsSuppl->getColumns();
            sal_Int32 nMasterLen = m_aMasterFields.getLength();
            if (xParentCols->hasElements() && (nMasterLen > 0))
            {
                try
                {
                    // analyze our parameters
                    if ( !m_pParameterInfo )
                        createParameterInfo();

                    if ( m_pParameterInfo && m_pParameterInfo->nCount )
                    {
                        Reference<XNameAccess>  xCols(xColsSuppl->getColumns(), UNO_QUERY);

                        const ::rtl::OUString* pMasterFields = m_aMasterFields.getConstArray();
                        const ::rtl::OUString* pDetailFields = m_aDetailFields.getConstArray();
                        const ::rtl::OUString* pDetailFieldsEnd = pDetailFields + m_aDetailFields.getLength();
                        for (pDetailFields; pDetailFields < pDetailFieldsEnd; ++pDetailFields, ++pMasterFields)
                        {
                            Reference<XPropertySet>  xMasterField, xField;
                            MapUString2INT32::const_iterator aFind;
                            if ( (aFind = m_pParameterInfo->aParamMapping.find(*pDetailFields)) != m_pParameterInfo->aParamMapping.end() )
                            {	// we really have a parameter column with this name
                                Reference< XPropertySet > xParamColumn;
                                do
                                {
                                    m_pParameterInfo->xParamsAsIndex->getByIndex(aFind->second) >>= xParamColumn;
                                    if (xParamColumn.is())
                                    {	// we really really have it :)
                                        ::rtl::OUString sParamColumnRealName;
                                        xParamColumn->getPropertyValue(PROPERTY_REALNAME) >>= sParamColumnRealName;
                                        if (xCols->hasByName(sParamColumnRealName))
                                        {	// our own columns have a column which's name equals the real name of the param column
                                            if (xParentCols->hasByName(*pMasterFields))
                                            {	// and our parent's cols know the master field which is connected to the current detail field

                                                // -> transfer the value property
                                                xParentCols->getByName(*pMasterFields) >>= xMasterField;
                                                xCols->getByName(sParamColumnRealName) >>= xField;
                                                if (xField.is() && xMasterField.is())
                                                    xField->setPropertyValue(PROPERTY_VALUE, xMasterField->getPropertyValue(PROPERTY_VALUE));
                                            }
                                        }
                                    }
                                    ++aFind;
                                }
                                while ( aFind != m_pParameterInfo->aParamMapping.end() );
                            }
                        }
                    }
                }
                catch(const Exception&)
                {
                    OSL_ENSURE(sal_False, "ODatabaseForm::reset_impl: could not initialize the master-detail-driven parameters!");
                }
            }
        }
    }

    aResetGuard.clear();
    // iterate through all components. don't use an XIndexAccess as this will cause massive
    // problems with the count.
    Reference<XEnumeration>  xIter = createEnumeration();
    while (xIter->hasMoreElements())
    {
        Reference<XReset> xReset;
        xIter->nextElement() >>= xReset;
        if (xReset.is())
        {
            // TODO : all reset-methods have to be thread-safe
            xReset->reset();
        }
    }

    aResetGuard.reset();
    // ensure that the row isn't modified
    // (do this _before_ the listeners are notified ! their reaction (maybe asynchronous) may depend
    // on the modified state of the row
    // 21.02.00 - 73265 - FS)
    if (bInsertRow)
        m_xAggregateSet->setPropertyValue(PROPERTY_ISMODIFIED, ::cppu::bool2any(sal_Bool(sal_False)));

    aResetGuard.clear();
    {
        EventObject aEvt(static_cast<XWeak*>(this));
        m_aResetListeners.notifyEach(&XResetListener::resetted, aEvt);
    }

    aResetGuard.reset();
    // and again : ensure the row isn't modified
    // we already did this after we (and maybe our dependents) resetted the values, but the listeners may have changed the row, too
    if (bInsertRow)
        m_xAggregateSet->setPropertyValue(PROPERTY_ISMODIFIED, ::cppu::bool2any((sal_False)));

    --m_nResetsPending;
}

//-----------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addResetListener(const Reference<XResetListener>& _rListener) throw( RuntimeException )
{
    m_aResetListeners.addInterface(_rListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeResetListener(const Reference<XResetListener>& _rListener) throw( RuntimeException )
{
    m_aResetListeners.removeInterface(_rListener);
}

//==============================================================================
// com::sun::star::form::XSubmit
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::submit( const Reference<XControl>& Control,
                              const ::com::sun::star::awt::MouseEvent& MouseEvt ) throw( RuntimeException )
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        // Sind Controls und eine Submit-URL vorhanden?
        if( !getCount() || !m_aTargetURL.getLength() )
            return;
    }

    ::osl::ClearableMutexGuard aGuard(m_aMutex);
    if (m_aSubmitListeners.getLength())
    {
        // create an own thread if we have (approve-)submit-listeners (so the listeners can't do that much damage
        // to this thread which is probably the main one)
        if (!m_pThread)
        {
            m_pThread = new OFormSubmitResetThread(this);
            m_pThread->acquire();
            m_pThread->create();
        }
        m_pThread->addEvent(&MouseEvt, Control, sal_True);
    }
    else
    {
        // direct call without any approving by the listeners
        aGuard.clear();
        submit_impl( Control, MouseEvt, true );
    }
}

//------------------------------------------------------------------------------
void ODatabaseForm::submit_impl(const Reference<XControl>& Control, const ::com::sun::star::awt::MouseEvent& MouseEvt, bool _bAproveByListeners)
{

    if (_bAproveByListeners)
    {
        ::cppu::OInterfaceIteratorHelper aIter(m_aSubmitListeners);
        EventObject aEvt(static_cast<XWeak*>(this));
        sal_Bool bCanceled = sal_False;
        while (aIter.hasMoreElements() && !bCanceled)
        {
            if (!((XSubmitListener*)aIter.next())->approveSubmit(aEvt))
                bCanceled = sal_True;
        }

        if (bCanceled)
            return;
    }

    FormSubmitEncoding eSubmitEncoding;
    FormSubmitMethod eSubmitMethod;
    ::rtl::OUString aURLStr;
    ::rtl::OUString aReferer;
    ::rtl::OUString aTargetName;
    Reference< XModel >  xModel;
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        // starform->Forms

        Reference<XChild>  xParent(m_xParent, UNO_QUERY);

        if (xParent.is())
            xModel = getXModel(xParent->getParent());

        if (xModel.is())
            aReferer = xModel->getURL();

        // TargetItem
        aTargetName = m_aTargetFrame;

        eSubmitEncoding = m_eSubmitEncoding;
        eSubmitMethod = m_eSubmitMethod;
        aURLStr = m_aTargetURL;
    }

    if (!xModel.is())
        return;

    Reference<XURLTransformer>
        xTransformer(m_xServiceFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")), UNO_QUERY);
    DBG_ASSERT(xTransformer.is(), "ODatabaseForm::submit_impl : could not create an URL transformer !");

    // URL-Encoding
    if( eSubmitEncoding == FormSubmitEncoding_URL )
    {
        ::rtl::OUString aData;
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            aData = GetDataURLEncoded( Control, MouseEvt );
        }

        Reference< XFrame >  xFrame = xModel->getCurrentController()->getFrame();
        if (!xFrame.is())
            return;

        URL aURL;
        // FormMethod GET
        if( eSubmitMethod == FormSubmitMethod_GET )
        {
            INetURLObject aUrlObj( aURLStr, INetURLObject::WAS_ENCODED );
            aUrlObj.SetParam( aData, INetURLObject::ENCODE_ALL );
            aURL.Complete = aUrlObj.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
            if (xTransformer.is())
                xTransformer->parseStrict(aURL);

            Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch(aURL, aTargetName,
                    FrameSearchFlag::SELF | FrameSearchFlag::PARENT | FrameSearchFlag::CHILDREN |
                    FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE | FrameSearchFlag::TASKS);

            if (xDisp.is())
            {
                Sequence<PropertyValue> aArgs(1);
                aArgs.getArray()->Name = ::rtl::OUString::createFromAscii("Referer");
                aArgs.getArray()->Value <<= aReferer;
                xDisp->dispatch(aURL, aArgs);
            }
        }
        // FormMethod POST
        else if( eSubmitMethod == FormSubmitMethod_POST )
        {
            aURL.Complete = aURLStr;
            xTransformer->parseStrict(aURL);

            Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch(aURL, aTargetName,
                FrameSearchFlag::SELF | FrameSearchFlag::PARENT | FrameSearchFlag::CHILDREN |
                FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE | FrameSearchFlag::TASKS);

            if (xDisp.is())
            {
                Sequence<PropertyValue> aArgs(2);
                aArgs.getArray()[0].Name = ::rtl::OUString::createFromAscii("Referer");
                aArgs.getArray()[0].Value <<= aReferer;

                // build a sequence from the to-be-submitted string
                ByteString a8BitData(aData.getStr(), (sal_uInt16)aData.getLength(), RTL_TEXTENCODING_MS_1252);
                    // always ANSI #58641
                Sequence< sal_Int8 > aPostData((sal_Int8*)a8BitData.GetBuffer(), a8BitData.Len());
                Reference< XInputStream > xPostData = new SequenceInputStream(aPostData);

                aArgs.getArray()[1].Name = ::rtl::OUString::createFromAscii("PostData");
                aArgs.getArray()[1].Value <<= xPostData;

                xDisp->dispatch(aURL, aArgs);
            }
        }
    }
    else if( eSubmitEncoding == FormSubmitEncoding_MULTIPART )
    {
        Reference< XFrame >  xFrame = xModel->getCurrentController()->getFrame();
        if (!xFrame.is())
            return;

        URL aURL;
        aURL.Complete = aURLStr;
        xTransformer->parseStrict(aURL);

        Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch(aURL, aTargetName,
                FrameSearchFlag::SELF | FrameSearchFlag::PARENT | FrameSearchFlag::CHILDREN |
                FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE | FrameSearchFlag::TASKS);

        if (xDisp.is())
        {
            ::rtl::OUString aContentType;
            Sequence<sal_Int8> aData;
            {
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                aData = GetDataMultiPartEncoded(Control, MouseEvt, aContentType);
            }
            if (!aData.getLength())
                return;

            Sequence<PropertyValue> aArgs(3);
            aArgs.getArray()[0].Name = ::rtl::OUString::createFromAscii("Referer");
            aArgs.getArray()[0].Value <<= aReferer;
            aArgs.getArray()[1].Name = ::rtl::OUString::createFromAscii("ContentType");
            aArgs.getArray()[1].Value <<= aContentType;

            // build a sequence from the to-be-submitted string
            Reference< XInputStream > xPostData = new SequenceInputStream(aData);

            aArgs.getArray()[2].Name = ::rtl::OUString::createFromAscii("PostData");
            aArgs.getArray()[2].Value <<= xPostData;

            xDisp->dispatch(aURL, aArgs);
        }
    }
    else if( eSubmitEncoding == FormSubmitEncoding_TEXT )
    {
        ::rtl::OUString aData;
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            aData = GetDataTextEncoded( Reference<XControl> (), MouseEvt );
        }

        Reference< XFrame >  xFrame = xModel->getCurrentController()->getFrame();
        if (!xFrame.is())
            return;

        URL aURL;

        aURL.Complete = aURLStr;
        xTransformer->parseStrict(aURL);

        Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch(aURL, aTargetName,
            FrameSearchFlag::SELF | FrameSearchFlag::PARENT | FrameSearchFlag::CHILDREN |
            FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE | FrameSearchFlag::TASKS);

        if (xDisp.is())
        {
            Sequence<PropertyValue> aArgs(2);
            aArgs.getArray()[0].Name = ::rtl::OUString::createFromAscii("Referer");
            aArgs.getArray()[0].Value <<= aReferer;

            // build a sequence from the to-be-submitted string
            ByteString aSystemEncodedData(aData.getStr(), (sal_uInt16)aData.getLength(), osl_getThreadTextEncoding());
            Sequence< sal_Int8 > aPostData((sal_Int8*)aSystemEncodedData.GetBuffer(), aSystemEncodedData.Len());
            Reference< XInputStream > xPostData = new SequenceInputStream(aPostData);

            aArgs.getArray()[1].Name = ::rtl::OUString::createFromAscii("PostData");
            aArgs.getArray()[1].Value <<= xPostData;

            xDisp->dispatch(aURL, aArgs);
        }
    }
    else
        DBG_ERROR("ODatabaseForm::submit_Impl : wrong encoding !");

}

// XSubmit
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addSubmitListener(const Reference<XSubmitListener>& _rListener) throw( RuntimeException )
{
    m_aSubmitListeners.addInterface(_rListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeSubmitListener(const Reference<XSubmitListener>& _rListener) throw( RuntimeException )
{
    m_aSubmitListeners.removeInterface(_rListener);
}

//==============================================================================
// com::sun::star::sdbc::XSQLErrorBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addSQLErrorListener(const Reference<XSQLErrorListener>& _rListener) throw( RuntimeException )
{
    m_aErrorListeners.addInterface(_rListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeSQLErrorListener(const Reference<XSQLErrorListener>& _rListener) throw( RuntimeException )
{
    m_aErrorListeners.removeInterface(_rListener);
}

//------------------------------------------------------------------------------
void ODatabaseForm::invlidateParameters()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    DELETEZ(m_pParameterInfo);
    clearParameters();
}

//==============================================================================
// OChangeListener
//------------------------------------------------------------------------------
void ODatabaseForm::_propertyChanged(const PropertyChangeEvent& evt) throw( RuntimeException )
{
    if ((0 == evt.PropertyName.compareToAscii(PROPERTY_ACTIVE_CONNECTION)) && !m_bForwardingConnection)
    {
        // the rowset changed it's active connection itself (without interaction from our side), so
        // we need to fire this event, too
        sal_Int32 nHandle = PROPERTY_ID_ACTIVE_CONNECTION;
        fire(&nHandle, &evt.NewValue, &evt.OldValue, 1, sal_False);
    }
    else	// it was one of the statement relevant props
    {
        // if the statement has changed we have to delete the parameter info
        invlidateParameters();
    }
}

//==============================================================================
// smartXChild
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setParent(const InterfaceRef& Parent) throw ( ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    Reference<XForm>  xParentForm(getParent(), UNO_QUERY);
    if (xParentForm.is())
    {
        Reference<XRowSetApproveBroadcaster>  xParentApprBroadcast(xParentForm, UNO_QUERY);
        if (xParentApprBroadcast.is())
            xParentApprBroadcast->removeRowSetApproveListener(this);
        Reference<XLoadable>  xParentLoadable(xParentForm, UNO_QUERY);
        if (xParentLoadable.is())
            xParentLoadable->removeLoadListener(this);
    }

    OFormComponents::setParent(Parent);

    xParentForm = Reference<XForm> (getParent(), UNO_QUERY);
    if (xParentForm.is())
    {
        Reference<XRowSetApproveBroadcaster>  xParentApprBroadcast(xParentForm, UNO_QUERY);
        if (xParentApprBroadcast.is())
            xParentApprBroadcast->addRowSetApproveListener(this);
        Reference<XLoadable>  xParentLoadable(xParentForm, UNO_QUERY);
        if (xParentLoadable.is())
            xParentLoadable->addLoadListener(this);
    }
}

//==============================================================================
// smartXTabControllerModel
//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::getGroupControl() throw(com::sun::star::uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    // Sollen Controls in einer TabOrder gruppe zusammengefasst werden?
    if (m_aCycle.hasValue())
    {
        sal_Int32 nCycle;
        ::cppu::enum2int(nCycle, m_aCycle);
        return nCycle != TabulatorCycle_PAGE;
    }

    if (isLoaded() && getConnection().is())
        return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setControlModels(const Sequence<Reference<XControlModel> >& rControls) throw( RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    // TabIndex in der Reihenfolge der Sequence setzen
    const Reference<XControlModel>* pControls = rControls.getConstArray();
    sal_Int16 nTabIndex = 1;
    sal_Int32 nCount = getCount();
    sal_Int32 nNewCount = rControls.getLength();

    // HiddenControls und Formulare werden nicht aufgefuehrt
    if (nNewCount <= nCount)
    {
        Any aElement;
        for (sal_Int32 i=0; i < nNewCount; ++i, ++pControls)
        {
            Reference<XFormComponent>  xComp(*pControls, UNO_QUERY);
            if (xComp.is())
            {
                // suchen der Componente in der Liste
                for (sal_Int32 j = 0; j < nCount; ++j)
                {
                    Reference<XFormComponent> xElement;
                    ::cppu::extractInterface(xElement, getByIndex(j));
                    if (xComp == xElement)
                    {
                        Reference<XPropertySet>  xSet(xComp, UNO_QUERY);
                        if (xSet.is() && hasProperty(PROPERTY_TABINDEX, xSet))
                            xSet->setPropertyValue( PROPERTY_TABINDEX, makeAny(nTabIndex++) );
                        break;
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
Sequence<Reference<XControlModel> > SAL_CALL ODatabaseForm::getControlModels() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pGroupManager->getControlModels();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setGroup( const Sequence<Reference<XControlModel> >& _rGroup, const ::rtl::OUString& Name ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // Die Controls werden gruppiert, indem ihr Name dem Namen des ersten
    // Controls der Sequenz angepasst wird
    const Reference<XControlModel>* pControls = _rGroup.getConstArray();
    Reference<XPropertySet> 	xSet;
    ::rtl::OUString sGroupName;

    for( sal_Int32 i=0; i<_rGroup.getLength(); ++i, ++pControls )
    {
        Reference< XPropertySet > xSet(*pControls, UNO_QUERY);
        if ( !xSet.is() )
        {
            // can't throw an exception other than a RuntimeException (which would not be appropriate),
            // so we ignore (and only assert) this
            OSL_ENSURE( sal_False, "ODatabaseForm::setGroup: invalid arguments!" );
            continue;
        }

        if (!sGroupName.getLength())
            xSet->getPropertyValue(PROPERTY_NAME) >>= sGroupName;
        else
            xSet->setPropertyValue(PROPERTY_NAME, makeAny(sGroupName));
    }
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseForm::getGroupCount() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pGroupManager->getGroupCount();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::getGroup( sal_Int32 nGroup, Sequence<Reference<XControlModel> >& _rGroup, ::rtl::OUString& _rName ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    _rGroup.realloc(0);
    _rName = ::rtl::OUString();

    if ((nGroup < 0) || (nGroup >= m_pGroupManager->getGroupCount()))
        return;
    m_pGroupManager->getGroup( nGroup, _rGroup, _rName  );
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::getGroupByName(const ::rtl::OUString& Name, Sequence< Reference<XControlModel>  >& _rGroup) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    _rGroup.realloc(0);
    m_pGroupManager->getGroupByName( Name, _rGroup );
}

//==============================================================================
// com::sun::star::lang::XEventListener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::disposing(const EventObject& Source) throw( RuntimeException )
{
    // does the call come from the connection which we are sharing with our parent?
    if ( isSharingConnection() )
    {
        Reference< XConnection > xConnSource( Source.Source, UNO_QUERY );
        if ( xConnSource.is() )
        {
#if OSL_DEBUG_LEVEL > 0
            Reference< XConnection > xActiveConn;
            m_xAggregateSet->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xActiveConn;
            OSL_ENSURE( xActiveConn.get() == xConnSource.get(), "ODatabaseForm::disposing: where did this come from?" );
                // there should be exactly one XConnection object we're listening at - our aggregate connection
#endif
            disposingSharedConnection( xConnSource );
        }
    }

    OInterfaceContainer::disposing(Source);

    // does the disposing come from the aggregate ?
    if (m_xAggregate.is())
    {	// no -> forward it
        com::sun::star::uno::Reference<com::sun::star::lang::XEventListener> xListener;
        if (query_aggregation(m_xAggregate, xListener))
            xListener->disposing(Source);
    }
}

//==============================================================================
// com::sun::star::form::XLoadListener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::loaded(const EventObject& aEvent) throw( RuntimeException )
{
    // now start the rowset listening to recover cursor events
    load_impl(sal_True);
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        Reference<XRowSet>  xParentRowSet(m_xParent, UNO_QUERY);
        if (xParentRowSet.is())
            xParentRowSet->addRowSetListener(this);

        m_pLoadTimer = new Timer();
        m_pLoadTimer->SetTimeout(100);
        m_pLoadTimer->SetTimeoutHdl(LINK(this,ODatabaseForm,OnTimeout));
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::unloading(const EventObject& aEvent) throw( RuntimeException )
{
    {
        // now stop the rowset listening if we are a subform
        ::osl::MutexGuard aGuard(m_aMutex);
        DELETEZ(m_pLoadTimer);

        Reference<XRowSet>  xParentRowSet(m_xParent, UNO_QUERY);
        if (xParentRowSet.is())
            xParentRowSet->removeRowSetListener(this);
    }

    unload();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::unloaded(const EventObject& aEvent) throw( RuntimeException )
{
    // nothing to do
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reloading(const EventObject& aEvent) throw( RuntimeException )
{
    // now stop the rowset listening if we are a subform
    ::osl::MutexGuard aGuard(m_aMutex);
    Reference<XRowSet>  xParentRowSet(m_xParent, UNO_QUERY);
    if (xParentRowSet.is())
        xParentRowSet->removeRowSetListener(this);

    if (m_pLoadTimer && m_pLoadTimer->IsActive())
        m_pLoadTimer->Stop();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reloaded(const EventObject& aEvent) throw( RuntimeException )
{
    reload_impl(sal_True);
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        Reference<XRowSet>  xParentRowSet(m_xParent, UNO_QUERY);
        if (xParentRowSet.is())
            xParentRowSet->addRowSetListener(this);
    }
}

//------------------------------------------------------------------------------
IMPL_LINK( ODatabaseForm, OnTimeout, void*, EMPTYARG )
{
    reload_impl(sal_True);
    return 1;
}

//==============================================================================
// com::sun::star::form::XLoadable
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::load() throw( RuntimeException )
{
    load_impl(sal_False);
}

//------------------------------------------------------------------------------
void ODatabaseForm::disposingSharedConnection( const Reference< XConnection >& _rxConn )
{
    stopSharingConnection();

    // TODO: we could think about whether or not to re-connect.
    unload( );
}

//------------------------------------------------------------------------------
void ODatabaseForm::stopSharingConnection( )
{
    OSL_ENSURE( m_bSharingConnection, "ODatabaseForm::stopSharingConnection: invalid call!" );

    if ( m_bSharingConnection )
    {
        // get the connection
        Reference< XConnection > xSharedConn;
        m_xAggregateSet->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xSharedConn;
        OSL_ENSURE( xSharedConn.is(), "ODatabaseForm::stopSharingConnection: there's no conn!" );

        // remove ourself as event listener
        Reference< XComponent > xSharedConnComp( xSharedConn, UNO_QUERY );
        if ( xSharedConnComp.is() )
            xSharedConnComp->removeEventListener( static_cast< XLoadListener* >( this ) );

        // no need to dispose the conn: we're not the owner, this is our parent
        // (in addition, this method may be called if the connection is beeing disposed while we use it)

        // reset the property
        xSharedConn.clear();
        m_bForwardingConnection = sal_True;
        m_xAggregateSet->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( xSharedConn ) );
        m_bForwardingConnection = sal_False;

        // reset the flag
        m_bSharingConnection = sal_False;
    }
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseForm::implEnsureConnection()
{
    OSL_ENSURE( sal_False, "ODatabaseForm::implEnsureConnection: this should be dead code, we're in binfilter here!" );
    return sal_False;
}

//------------------------------------------------------------------------------
void ODatabaseForm::load_impl(sal_Bool bCausedByParentForm, sal_Bool bMoveToFirst, const Reference< XInteractionHandler >& _rxCompletionHandler ) throw( RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    // are we already loaded?
    if (isLoaded())
        return;

    m_bSubForm = bCausedByParentForm;

    // if we don't have a connection, we are not intended to be a database form or the aggregate was not able
    // to establish a connection
    sal_Bool bConnected	= implEnsureConnection();

    // we don't have to execute if we do not have a command to execute
    sal_Bool bExecute = bConnected && m_xAggregateSet.is() && getString(m_xAggregateSet->getPropertyValue(PROPERTY_COMMAND)).getLength();

    // a database form always uses caching
    // we use starting fetchsize with at least 10 rows
    if (bConnected)
        m_xAggregateSet->setPropertyValue(PROPERTY_FETCHSIZE, makeAny((sal_Int32)10));

    // if we're loaded as sub form we got a "rowSetChanged" from the parent rowset _before_ we got the "loaded"
    // so we don't need to execute the statement again, this was already done
    // (and there were no relevant changes between these two listener calls, the "load" of a form is quite an
    // atomar operation.)

    sal_Bool bSuccess = sal_False;
    if (bExecute)
    {
        m_sCurrentErrorContext = FRM_RES_STRING(RID_ERR_LOADING_FORM);
        bSuccess = executeRowSet(aGuard, bMoveToFirst, _rxCompletionHandler);
    }

    if (bSuccess)
    {
        m_bLoaded = sal_True;
        aGuard.clear();
        EventObject aEvt(static_cast<XWeak*>(this));
        m_aLoadListeners.notifyEach(&XLoadListener::loaded, aEvt);

        // if we are on the insert row, we have to reset all controls
        // to set the default values
        if (bExecute && getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ISNEW)))
            reset();
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::unload() throw( RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    if (!isLoaded())
        return;

    DELETEZ(m_pLoadTimer);

    aGuard.clear();
    EventObject aEvt(static_cast<XWeak*>(this));
    m_aLoadListeners.notifyEach(&XLoadListener::unloading, aEvt);

    if (m_xAggregateAsRowSet.is())
    {
        // we may have reset the InsertOnly property on the aggregate - restore it
        restoreInsertOnlyState( );

        // clear the parameters if there are any
        invlidateParameters();

        try
        {
            // close the aggregate
            Reference<XCloseable>  xCloseable;
            query_aggregation( m_xAggregate, xCloseable);
            aGuard.clear();
            if (xCloseable.is())
                xCloseable->close();
        }
        catch(SQLException& eDB)
        {
            eDB;
        }
        aGuard.reset();
    }

    m_bLoaded = sal_False;

    // if the connection we used while we were loaded is only shared with our parent, we
    // reset it
    if ( isSharingConnection() )
        stopSharingConnection();

    aGuard.clear();
    m_aLoadListeners.notifyEach(&XLoadListener::unloaded, aEvt);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reload() throw( RuntimeException )
{
    reload_impl(sal_True);
}

//------------------------------------------------------------------------------
void ODatabaseForm::reload_impl(sal_Bool bMoveToFirst, const Reference< XInteractionHandler >& _rxCompletionHandler ) throw( RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    if (!isLoaded())
        return;

    EventObject aEvent(static_cast<XWeak*>(this));
    {
        // only if there is no approve listener we can post the event at this time
        // otherwise see approveRowsetChange
        // the aprrovement is done by the aggregate
        if (!m_aRowSetApproveListeners.getLength())
        {
            ::cppu::OInterfaceIteratorHelper aIter(m_aLoadListeners);
            aGuard.clear();

            while (aIter.hasMoreElements())
                ((XLoadListener*)aIter.next())->reloading(aEvent);

            aGuard.reset();
        }
    }

    sal_Bool bSuccess = sal_True;
    try
    {
        m_sCurrentErrorContext = FRM_RES_STRING(RID_ERR_REFRESHING_FORM);
        bSuccess = executeRowSet(aGuard, bMoveToFirst, _rxCompletionHandler);
    }
    catch(SQLException& e)
    {
        DBG_ERROR("ODatabaseForm::reload_impl : shouldn't executeRowSet catch this exception ?");
        e;
    }

    if (bSuccess)
    {
        ::cppu::OInterfaceIteratorHelper aIter(m_aLoadListeners);
        aGuard.clear();
        while (aIter.hasMoreElements())
            ((XLoadListener*)aIter.next())->reloaded(aEvent);

        // if we are on the insert row, we have to reset all controls
        // to set the default values
        if (getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ISNEW)))
            reset();
    }
    else
        m_bLoaded = sal_False;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isLoaded() throw( RuntimeException )
{
    return m_bLoaded;
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addLoadListener(const Reference<XLoadListener>& aListener) throw( RuntimeException )
{
    m_aLoadListeners.addInterface(aListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeLoadListener(const Reference<XLoadListener>& aListener) throw( RuntimeException )
{
    m_aLoadListeners.removeInterface(aListener);
}

//==============================================================================
// com::sun::star::sdbc::XCloseable
//==============================================================================
void SAL_CALL ODatabaseForm::close() throw( SQLException, RuntimeException )
{
    // unload will close the aggregate
    unload();
}

//==============================================================================
// com::sun::star::sdbc::XRowSetListener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::cursorMoved(const EventObject& event) throw( RuntimeException )
{
    // reload the subform with the new parameters of the parent
    // do this handling delayed to provide of execute too many SQL Statements
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    if (m_pLoadTimer->IsActive())
        m_pLoadTimer->Stop();

    // and start the timer again
    m_pLoadTimer->Start();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::rowChanged(const EventObject& event) throw( RuntimeException )
{
    // ignore it
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::rowSetChanged(const EventObject& event) throw( RuntimeException )
{
    // not interested in :
    // if our parent is an ODatabaseForm, too, then after this rowSetChanged we'll get a "reloaded"
    // or a "loaded" event.
    // If somebody gave us another parent which is an XRowSet but doesn't handle an execute as
    // "load" respectivly "reload" ... can't do anything ....
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::approveCursorMove(const EventObject& event) throw( RuntimeException )
{
    // is our aggregate calling?
    if (event.Source == InterfaceRef(static_cast<XWeak*>(this)))
    {
        // Our aggregate doesn't have any ApproveRowSetListeners (expect ourself), as we re-routed the queryInterface
        // for XRowSetApproveBroadcaster-interface.
        // So we have to multiplex this approve request.
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
            if (!((XRowSetApproveListener*)aIter.next())->approveCursorMove(event))
                return sal_False;
    }
    else
    {
        // this is a call from our parent ...
        // a parent's cursor move will result in a re-execute of our own row-set, so we have to
        // ask our own RowSetChangesListeners, too
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
            if (!((XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
                return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::approveRowChange(const RowChangeEvent& event) throw( RuntimeException )
{
    // is our aggregate calling?
    if (event.Source == InterfaceRef(static_cast<XWeak*>(this)))
    {
        // Our aggregate doesn't have any ApproveRowSetListeners (expect ourself), as we re-routed the queryInterface
        // for XRowSetApproveBroadcaster-interface.
        // So we have to multiplex this approve request.
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
            if (!((XRowSetApproveListener*)aIter.next())->approveRowChange(event))
                return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::approveRowSetChange(const EventObject& event) throw( RuntimeException )
{
    if (event.Source == InterfaceRef(static_cast<XWeak*>(this)))	// ignore our aggregate as we handle this approve ourself
    {
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
        if (!((XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
            return sal_False;

        if (isLoaded())
        {
            ::cppu::OInterfaceIteratorHelper aLoadIter(m_aLoadListeners);
            while (aLoadIter.hasMoreElements())
                ((XLoadListener*)aLoadIter.next())->reloading(event);
        }
    }
    else
    {
        // this is a call from our parent ...
        // a parent's cursor move will result in a re-execute of our own row-set, so we have to
        // ask our own RowSetChangesListeners, too
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        while (aIter.hasMoreElements())
            if (!((XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
                return sal_False;
    }
    return sal_True;
}

//==============================================================================
// com::sun::star::sdb::XRowSetApproveBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addRowSetApproveListener(const Reference<XRowSetApproveListener>& _rListener) throw( RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    m_aRowSetApproveListeners.addInterface(_rListener);

    // do we have to multiplex ?
    if (m_aRowSetApproveListeners.getLength() == 1)
    {
        Reference<XRowSetApproveBroadcaster>  xBroadcaster;
        if (query_aggregation( m_xAggregate, xBroadcaster))
        {
            Reference<XRowSetApproveListener>  xListener((XRowSetApproveListener*)this);
            xBroadcaster->addRowSetApproveListener(xListener);
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeRowSetApproveListener(const Reference<XRowSetApproveListener>& _rListener) throw( RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    // do we have to remove the multiplex ?
    m_aRowSetApproveListeners.removeInterface(_rListener);
    if ( m_aRowSetApproveListeners.getLength() == 0 )
    {
        Reference<XRowSetApproveBroadcaster>  xBroadcaster;
        if (query_aggregation( m_xAggregate, xBroadcaster))
        {
            Reference<XRowSetApproveListener>  xListener((XRowSetApproveListener*)this);
            xBroadcaster->removeRowSetApproveListener(xListener);
        }
    }
}

//==============================================================================
// com::sun:star::form::XDatabaseParameterBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addParameterListener(const Reference<XDatabaseParameterListener>& _rListener) throw( RuntimeException )
{
    m_aParameterListeners.addInterface(_rListener);
}
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeParameterListener(const Reference<XDatabaseParameterListener>& _rListener) throw( RuntimeException )
{
    m_aParameterListeners.removeInterface(_rListener);
}

//==============================================================================
// com::sun::star::sdb::XCompletedExecution
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::executeWithCompletion( const Reference< XInteractionHandler >& _rxHandler ) throw(SQLException, RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    // the difference between execute and load is, that we position on the first row in case of load
    // after execute we remain before the first row
    if (!isLoaded())
    {
        aGuard.clear();
        load_impl(sal_False, sal_False, _rxHandler);
    }
    else
    {
        EventObject event(static_cast< XWeak* >(this));
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        aGuard.clear();

        while (aIter.hasMoreElements())
            if (!((XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
                return;

        // we're loaded and somebody want's to execute ourself -> this means a reload
        reload_impl(sal_False, _rxHandler);
    }
}

//==============================================================================
// com::sun::star::sdbc::XRowSet
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::execute() throw( SQLException, RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    // if somebody calls an execute and we're not loaded we reroute this call to our load method.

    // the difference between execute and load is, that we position on the first row in case of load
    // after execute we remain before the first row
    if (!isLoaded())
    {
        aGuard.clear();
        load_impl(sal_False, sal_False);
    }
    else
    {
        EventObject event(static_cast< XWeak* >(this));
        ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
        aGuard.clear();

        while (aIter.hasMoreElements())
            if (!((XRowSetApproveListener*)aIter.next())->approveRowSetChange(event))
                return;

        // we're loaded and somebody want's to execute ourself -> this means a reload
        reload_impl(sal_False);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addRowSetListener(const Reference<XRowSetListener>& _rListener) throw( RuntimeException )
{
    if (m_xAggregateAsRowSet.is())
        m_xAggregateAsRowSet->addRowSetListener(_rListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeRowSetListener(const Reference<XRowSetListener>& _rListener) throw( RuntimeException )
{
    if (m_xAggregateAsRowSet.is())
        m_xAggregateAsRowSet->removeRowSetListener(_rListener);
}

//==============================================================================
// com::sun::star::sdbc::XResultSet
//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::next() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->next();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isBeforeFirst() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->isBeforeFirst();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isAfterLast() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->isAfterLast();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isFirst() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->isFirst();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::isLast() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->isLast();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::beforeFirst() throw( SQLException, RuntimeException )
{
    m_xAggregateAsRowSet->beforeFirst();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::afterLast() throw( SQLException, RuntimeException )
{
    m_xAggregateAsRowSet->afterLast();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::first() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->first();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::last() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->last();
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseForm::getRow() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->getRow();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::absolute(sal_Int32 row) throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->absolute(row);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::relative(sal_Int32 rows) throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->relative(rows);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::previous() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->previous();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::refreshRow() throw( SQLException, RuntimeException )
{
    m_xAggregateAsRowSet->refreshRow();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::rowUpdated() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->rowUpdated();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::rowInserted() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->rowInserted();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::rowDeleted() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->rowDeleted();
}

//------------------------------------------------------------------------------
InterfaceRef SAL_CALL ODatabaseForm::getStatement() throw( SQLException, RuntimeException )
{
    return m_xAggregateAsRowSet->getStatement();
}

// com::sun::star::sdbc::XResultSetUpdate
// exceptions during insert update and delete will be forwarded to the errorlistener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::insertRow() throw( SQLException, RuntimeException )
{
    try
    {
        Reference<XResultSetUpdate>  xUpdate;
        if (query_aggregation( m_xAggregate, xUpdate))
            xUpdate->insertRow();
    }
    catch(RowSetVetoException& eVeto)
    {
        eVeto;
        throw;
    }
    catch(SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_INSERTRECORD));
        throw;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::updateRow() throw( SQLException, RuntimeException )
{
    try
    {
        Reference<XResultSetUpdate>  xUpdate;
        if (query_aggregation( m_xAggregate, xUpdate))
            xUpdate->updateRow();
    }
    catch(RowSetVetoException& eVeto)
    {
        eVeto;
        throw;
    }
    catch(SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_UPDATERECORD));
        throw;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::deleteRow() throw( SQLException, RuntimeException )
{
    try
    {
        Reference<XResultSetUpdate>  xUpdate;
        if (query_aggregation( m_xAggregate, xUpdate))
            xUpdate->deleteRow();
    }
    catch(RowSetVetoException& eVeto)
    {
        eVeto;
        throw;
    }
    catch(SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_DELETERECORD));
        throw;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::cancelRowUpdates() throw( SQLException, RuntimeException )
{
    try
    {
        Reference<XResultSetUpdate>  xUpdate;
        if (query_aggregation( m_xAggregate, xUpdate))
            xUpdate->cancelRowUpdates();
    }
    catch(RowSetVetoException& eVeto)
    {
        eVeto;
        throw;
    }
    catch(SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_INSERTRECORD));
        throw;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::moveToInsertRow() throw( SQLException, RuntimeException )
{
    Reference<XResultSetUpdate>  xUpdate;
    if (query_aggregation( m_xAggregate, xUpdate))
    {
        // _always_ move to the insert row
        //
        // Formerly, the following line was conditioned with a "not is new", means we did not move the aggregate
        // to the insert row if it was already positioned there.
        //
        // This prevented the RowSet implementation from resetting it's column values. We, ourself, formerly
        // did this reset of columns in reset_impl, where we set every column to the ControlDefault, or, if this
        // was not present, to NULL. However, the problem with setting to NULL was #88888#, the problem with
        // _not_ setting to NULL (which was the original fix for #88888#) was #97955#.
        //
        // So now we
        // * move our aggregate to the insert row
        // * in reset_impl
        //   - set the control defaults into the columns if not void
        //   - do _not_ set the columns to NULL if no control default is set
        // This fixes both #88888# and #97955#
        //
        // Still, there is #72756#. During fixing this bug, DG introduced not calling the aggregate here. So
        // in theory, we re-introduced #72756#. But the bug described therein does not happen anymore, as the
        // preliminaries for it changed (no display of guessed values for new records with autoinc fields)
        //
        // BTW: the public Issuezilla bug for #97955# is #i2815#
        //
        // 16.04.2002 - 97955 - fs@openoffice.org
        xUpdate->moveToInsertRow();

        // then set the default values and the parameters given from the parent
        reset();
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::moveToCurrentRow() throw( SQLException, RuntimeException )
{
    Reference<XResultSetUpdate>  xUpdate;
    if (query_aggregation( m_xAggregate, xUpdate))
        xUpdate->moveToCurrentRow();
}

// com::sun::star::sdbcx::XDeleteRows
//------------------------------------------------------------------------------
Sequence<sal_Int32> SAL_CALL ODatabaseForm::deleteRows(const Sequence<Any>& rows) throw( SQLException, RuntimeException )
{
    try
    {
        Reference<XDeleteRows>  xDelete;
        if (query_aggregation( m_xAggregate, xDelete))
            return xDelete->deleteRows(rows);
    }
    catch(RowSetVetoException& eVeto)
    {
        eVeto; // make compiler happy
        throw;
    }
    catch(SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_DELETERECORDS));
        throw;
    }

    return Sequence< sal_Int32 >();
}

// com::sun::star::sdbc::XParameters
namespace
{
    void checkParameters(::std::vector<bool>& _rBoolVector,sal_Int32 _nParameterIndex)
    {
        if ((sal_Int32)_rBoolVector.size() < _nParameterIndex)
        {
            _rBoolVector.reserve(_rBoolVector.capacity() + _nParameterIndex);
            for (sal_Int32 i = 0; i < _nParameterIndex; i++)
                _rBoolVector.push_back(false);
        }
        _rBoolVector[_nParameterIndex - 1] = true;
    }
}
//------------------------------------------------------------------------------
#define PARAMETER_VISITED(method)										\
    ::osl::MutexGuard aGuard(m_aMutex);									\
    Reference<XParameters>  xParameters;								\
    if (query_aggregation( m_xAggregate, xParameters))					\
        xParameters->method;											\
                                                                        \
    checkParameters(m_aParameterVisited,parameterIndex)

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setNull(parameterIndex, sqlType));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setObjectNull(parameterIndex, sqlType, typeName));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setBoolean(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setByte(sal_Int32 parameterIndex, sal_Int8 x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setByte(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setShort(sal_Int32 parameterIndex, sal_Int16 x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setShort(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setInt(sal_Int32 parameterIndex, sal_Int32 x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setInt(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setLong(sal_Int32 parameterIndex, sal_Int64 x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setLong(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setFloat(sal_Int32 parameterIndex, float x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setFloat(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setDouble(sal_Int32 parameterIndex, double x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setDouble(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setString(sal_Int32 parameterIndex, const ::rtl::OUString& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setString(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBytes(sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setBytes(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setDate(sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setDate(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setTime(sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setTime(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setTimestamp(sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setTimestamp(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBinaryStream(sal_Int32 parameterIndex, const Reference<XInputStream>& x, sal_Int32 length) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setBinaryStream(parameterIndex, x, length));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setCharacterStream(sal_Int32 parameterIndex, const Reference<XInputStream>& x, sal_Int32 length) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setCharacterStream(parameterIndex, x, length));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObjectWithInfo(sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setObjectWithInfo(parameterIndex, x, targetSqlType, scale));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObject(sal_Int32 parameterIndex, const Any& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setObject(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setRef(sal_Int32 parameterIndex, const Reference<XRef>& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setRef(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBlob(sal_Int32 parameterIndex, const Reference<XBlob>& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setBlob(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setClob(sal_Int32 parameterIndex, const Reference<XClob>& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setClob(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setArray(sal_Int32 parameterIndex, const Reference<XArray>& x) throw( SQLException, RuntimeException )
{
    PARAMETER_VISITED(setArray(parameterIndex, x));
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::clearParameters() throw( SQLException, RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    Reference<XParameters>  xParameters;
    if (query_aggregation(m_xAggregate, xParameters))
        xParameters->clearParameters();
    m_aParameterVisited.clear();
}

// com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
::rtl::OUString	SAL_CALL ODatabaseForm::getImplementationName_Static()
{
    return DATABASEFORM_IMPLEMENTATION_NAME;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODatabaseForm::getCompatibleServiceNames_Static()
{
    Sequence< ::rtl::OUString > aServices( 1 );
    ::rtl::OUString* pServices = aServices.getArray();

    *pServices++ = FRM_COMPONENT_FORM;

    return aServices;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODatabaseForm::getCurrentServiceNames_Static()
{
    Sequence< ::rtl::OUString > aServices( 5 );
    ::rtl::OUString* pServices = aServices.getArray();

    *pServices++ = FRM_SUN_FORMCOMPONENT;
    *pServices++ = ::rtl::OUString::createFromAscii("com.sun.star.form.FormComponents");
    *pServices++ = FRM_SUN_COMPONENT_FORM;
    *pServices++ = FRM_SUN_COMPONENT_HTMLFORM;
    *pServices++ = FRM_SUN_COMPONENT_DATAFORM;

    return aServices;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODatabaseForm::getSupportedServiceNames_Static()
{
    return ::comphelper::concatSequences(
        getCurrentServiceNames_Static(),
        getCompatibleServiceNames_Static()
    );
}

//------------------------------------------------------------------------------
::rtl::OUString	SAL_CALL ODatabaseForm::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODatabaseForm::getSupportedServiceNames() throw( RuntimeException )
{
    // the services of our aggregate
    Sequence< ::rtl::OUString > aServices;
    Reference< XServiceInfo > xInfo;
    if (query_aggregation(m_xAggregate, xInfo))
        aServices = xInfo->getSupportedServiceNames();

    // concat with out own services
    return ::comphelper::concatSequences(
        getCurrentServiceNames_Static(),
        aServices
    );
    // use getCurrentXXX instead of getSupportedXXX, because at runtime, we do not want to have
    // the compatible names
    // This is maily to be consistent with the implementation before fixing #97083#, though the
    // better solution _may_ be to return the compatible names at runtime, too
    // 04.03.2002 - fs@openoffice.org
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::supportsService(const ::rtl::OUString& ServiceName) throw( RuntimeException )
{
    Sequence< ::rtl::OUString > aSupported( getSupportedServiceNames() );
    const ::rtl::OUString* pArray = aSupported.getConstArray();
    for( sal_Int32 i = 0; i < aSupported.getLength(); ++i, ++pArray )
        if( pArray->equals( ServiceName ) )
            return sal_True;
    return sal_False;
}

//==============================================================================
// com::sun::star::io::XPersistObject
//------------------------------------------------------------------------------

const sal_uInt16 CYCLE				= 0x0001;
const sal_uInt16 DONTAPPLYFILTER	= 0x0002;

//------------------------------------------------------------------------------
::rtl::OUString ODatabaseForm::getServiceName() throw( RuntimeException )
{
    return FRM_COMPONENT_FORM;	// old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::write(const Reference<XObjectOutputStream>& _rxOutStream) throw( IOException, RuntimeException )
{
    DBG_ASSERT(m_xAggregateSet.is(), "ODatabaseForm::write : only to be called if the aggregate exists !");

    // all children
    OFormComponents::write(_rxOutStream);

    // version
    _rxOutStream->writeShort(0x0003);

    // Name
    _rxOutStream << m_sName;

    ::rtl::OUString sDataSource;
    if (m_xAggregateSet.is())
        m_xAggregateSet->getPropertyValue(PROPERTY_DATASOURCE) >>= sDataSource;
    _rxOutStream << sDataSource;

    // former CursorSource
    ::rtl::OUString sCommand;
    if (m_xAggregateSet.is())
        m_xAggregateSet->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
    _rxOutStream << sCommand;

    // former MasterFields
    _rxOutStream << m_aMasterFields;
    // former DetailFields
    _rxOutStream << m_aDetailFields;

    // former DataSelectionType
    DataSelectionType eTranslated = DataSelectionType_TABLE;
    if (m_xAggregateSet.is())
    {
        sal_Int32 nCommandType;
        m_xAggregateSet->getPropertyValue(PROPERTY_COMMANDTYPE) >>= nCommandType;
        switch (nCommandType)
        {
            case CommandType::TABLE : eTranslated = DataSelectionType_TABLE; break;
            case CommandType::QUERY : eTranslated = DataSelectionType_QUERY; break;
            case CommandType::COMMAND:
            {
                sal_Bool bEscapeProcessing = getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_ESCAPE_PROCESSING));
                eTranslated = bEscapeProcessing ? DataSelectionType_SQL : DataSelectionType_SQLPASSTHROUGH;
            }
            break;
            default : DBG_ERROR("ODatabaseForm::write : wrong CommandType !");
        }
    }
    _rxOutStream->writeShort((sal_Int16)eTranslated);			// former DataSelectionType

    // very old versions expect a CursorType here
    _rxOutStream->writeShort(DatabaseCursorType_KEYSET);

    _rxOutStream->writeBoolean(m_eNavigation != NavigationBarMode_NONE);

    // former DataEntry
    if (m_xAggregateSet.is())
        _rxOutStream->writeBoolean(getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_INSERTONLY)));
    else
        _rxOutStream->writeBoolean(sal_False);

    _rxOutStream->writeBoolean(m_bAllowInsert);
    _rxOutStream->writeBoolean(m_bAllowUpdate);
    _rxOutStream->writeBoolean(m_bAllowDelete);

    // html form stuff
    ::rtl::OUString sTmp = INetURLObject::decode(::binfilter::StaticBaseUrl::AbsToRel( m_aTargetURL ), '%', INetURLObject::DECODE_UNAMBIGUOUS);
    _rxOutStream << sTmp;
    _rxOutStream->writeShort( (sal_Int16)m_eSubmitMethod );
    _rxOutStream->writeShort( (sal_Int16)m_eSubmitEncoding );
    _rxOutStream << m_aTargetFrame;

    // version 2 didn't know some options and the "default" state
    sal_Int32 nCycle = TabulatorCycle_RECORDS;
    if (m_aCycle.hasValue())
    {
        ::cppu::enum2int(nCycle, m_aCycle);
        if (m_aCycle == TabulatorCycle_PAGE)
                // unknown in earlier versions
            nCycle = TabulatorCycle_RECORDS;
    }
    _rxOutStream->writeShort((sal_Int16) nCycle);

    _rxOutStream->writeShort((sal_Int16)m_eNavigation);

    ::rtl::OUString sFilter;
    ::rtl::OUString sOrder;
    if (m_xAggregateSet.is())
    {
        m_xAggregateSet->getPropertyValue(PROPERTY_FILTER_CRITERIA) >>= sFilter;
        m_xAggregateSet->getPropertyValue(PROPERTY_SORT) >>= sOrder;
    }
    _rxOutStream << sFilter;
    _rxOutStream << sOrder;


    // version 3
    sal_uInt16 nAnyMask = 0;
    if (m_aCycle.hasValue())
        nAnyMask |= CYCLE;

    if (m_xAggregateSet.is() && !getBOOL(m_xAggregateSet->getPropertyValue(PROPERTY_APPLYFILTER)))
        nAnyMask |= DONTAPPLYFILTER;

    _rxOutStream->writeShort(nAnyMask);

    if (nAnyMask & CYCLE)
    {
        sal_Int32 nCycle;
        ::cppu::enum2int(nCycle, m_aCycle);
        _rxOutStream->writeShort((sal_Int16)nCycle);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::read(const Reference<XObjectInputStream>& _rxInStream) throw( IOException, RuntimeException )
{
    DBG_ASSERT(m_xAggregateSet.is(), "ODatabaseForm::read : only to be called if the aggregate exists !");

    OFormComponents::read(_rxInStream);

    // version
    sal_uInt16 nVersion = _rxInStream->readShort();

    _rxInStream >> m_sName;

    ::rtl::OUString sAggregateProp;
    _rxInStream >> sAggregateProp;
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_DATASOURCE, makeAny(sAggregateProp));
    _rxInStream >> sAggregateProp;
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_COMMAND, makeAny(sAggregateProp));

    _rxInStream >> m_aMasterFields;
    _rxInStream >> m_aDetailFields;

    sal_Int16 nCursorSourceType = _rxInStream->readShort();
    sal_Int32 nCommandType = 0;
    switch ((DataSelectionType)nCursorSourceType)
    {
        case DataSelectionType_TABLE : nCommandType = CommandType::TABLE; break;
        case DataSelectionType_QUERY : nCommandType = CommandType::QUERY; break;
        case DataSelectionType_SQL:
        case DataSelectionType_SQLPASSTHROUGH:
        {
            nCommandType = CommandType::COMMAND;
            sal_Bool bEscapeProcessing = ((DataSelectionType)nCursorSourceType) != DataSelectionType_SQLPASSTHROUGH;
            m_xAggregateSet->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, makeAny((sal_Bool)bEscapeProcessing));
        }
        break;
        default : DBG_ERROR("ODatabaseForm::read : wrong CommandType !");
    }
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_COMMANDTYPE, makeAny(nCommandType));

    // obsolete
    sal_Int16 nDummy = _rxInStream->readShort();

    // navigation mode was a boolean in version 1
    // war in der version 1 ein sal_Bool
    sal_Bool bNavigation = _rxInStream->readBoolean();
    if (nVersion == 1)
        m_eNavigation = bNavigation ? NavigationBarMode_CURRENT : NavigationBarMode_NONE;

    sal_Bool bInsertOnly = _rxInStream->readBoolean();
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_INSERTONLY, makeAny(bInsertOnly));

    m_bAllowInsert		= _rxInStream->readBoolean();
    m_bAllowUpdate		= _rxInStream->readBoolean();
    m_bAllowDelete		= _rxInStream->readBoolean();

    // html stuff
    ::rtl::OUString sTmp;
    _rxInStream >> sTmp;
    m_aTargetURL = INetURLObject::decode(::binfilter::StaticBaseUrl::RelToAbs( sTmp ), '%', INetURLObject::DECODE_UNAMBIGUOUS);
    m_eSubmitMethod		= (FormSubmitMethod)_rxInStream->readShort();
    m_eSubmitEncoding		= (FormSubmitEncoding)_rxInStream->readShort();
    _rxInStream >> m_aTargetFrame;

    if (nVersion > 1)
    {
        sal_Int32 nCycle = _rxInStream->readShort();
        m_aCycle = ::cppu::int2enum(nCycle, ::getCppuType(static_cast<const TabulatorCycle*>(NULL)));
        m_eNavigation = (NavigationBarMode)_rxInStream->readShort();

        _rxInStream >> sAggregateProp;
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_FILTER_CRITERIA, makeAny(sAggregateProp));
        _rxInStream >> sAggregateProp;
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_SORT, makeAny(sAggregateProp));
    }

    sal_uInt16 nAnyMask	= 0;
    if (nVersion > 2)
    {
        nAnyMask = _rxInStream->readShort();
        if (nAnyMask & CYCLE)
        {
            sal_Int32 nCycle = _rxInStream->readShort();
            m_aCycle = ::cppu::int2enum(nCycle, ::getCppuType(static_cast<const TabulatorCycle*>(NULL)));
        }
        else
            m_aCycle.clear();
    }
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_APPLYFILTER, makeAny((sal_Bool)((nAnyMask & DONTAPPLYFILTER) == 0)));
}

//------------------------------------------------------------------------------
void ODatabaseForm::implInserted( const ElementDescription* _pElement )
{
    OFormComponents::implInserted( _pElement );

    Reference< XSQLErrorBroadcaster >	xBroadcaster( _pElement->xInterface, UNO_QUERY );
    Reference< XForm >					xForm		( _pElement->xInterface, UNO_QUERY );

    if ( xBroadcaster.is() && !xForm.is() )
    {	// the object is an error broadcaster, but no form itself -> add ourself as listener
        xBroadcaster->addSQLErrorListener( this );
    }
}

//------------------------------------------------------------------------------
void ODatabaseForm::implRemoved(const InterfaceRef& _rxObject)
{
    OFormComponents::implRemoved( _rxObject );

    Reference<XSQLErrorBroadcaster>  xBroadcaster(_rxObject, UNO_QUERY);
    Reference<XForm>  xForm(_rxObject, UNO_QUERY);
    if (xBroadcaster.is() && !xForm.is())
    {	// the object is an error broadcaster, but no form itself -> remove ourself as listener
        xBroadcaster->removeSQLErrorListener(this);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::errorOccured(const SQLErrorEvent& _rEvent) throw( RuntimeException )
{
    // give it to my own error listener
    onError(_rEvent);
    // TODO : think about extending the chain with an SQLContext object saying
    // "this was an error of one of my children"
}

// com::sun::star::container::XNamed
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseForm::getName() throw( RuntimeException )
{
    ::rtl::OUString sReturn;
    OPropertySetHelper::getFastPropertyValue(PROPERTY_ID_NAME) >>= sReturn;
    return sReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setName(const ::rtl::OUString& aName) throw( RuntimeException )
{
    setFastPropertyValue(PROPERTY_ID_NAME, makeAny(aName));
}

//.........................................................................
}	// namespace frm
//.........................................................................

}
