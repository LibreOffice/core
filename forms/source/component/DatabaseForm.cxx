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


#include "componenttools.hxx"
#include "DatabaseForm.hxx"
#include "EventThread.hxx"
#include "frm_module.hxx"
#include "frm_resource.hrc"
#include "frm_resource.hxx"
#include "GroupManager.hxx"
#include "property.hrc"
#include "property.hxx"
#include "services.hxx"

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/form/DataSelectionType.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/TabulatorCycle.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/RowSetVetoException.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XModifiable2.hpp>

#include <comphelper/basicio.hxx>
#include <comphelper/container.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase2.hxx>
#include <rtl/math.hxx>
#include <rtl/tencinfo.h>
#include <svl/inettype.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/inetmsg.hxx>
#include <tools/inetstrm.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucblockbytes.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <osl/mutex.hxx>

#include <ctype.h>
#include <boost/unordered_map.hpp>

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
    static ::frm::OMultiInstanceAutoRegistration< ::frm::ODatabaseForm > aAutoRegistration;
}

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
//= DocumentModifyGuard
//==================================================================
class DocumentModifyGuard
{
public:
    DocumentModifyGuard( const Reference< XInterface >& _rxFormComponent )
        :m_xDocumentModify( getXModel( _rxFormComponent ), UNO_QUERY )
    {
        impl_changeModifiableFlag_nothrow( false );
    }
    ~DocumentModifyGuard()
    {
        impl_changeModifiableFlag_nothrow( true );
    }

private:
    void    impl_changeModifiableFlag_nothrow( const bool _enable )
    {
        try
        {
            if ( m_xDocumentModify.is() )
                _enable ? m_xDocumentModify->enableSetModified() : m_xDocumentModify->disableSetModified();
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

private:
    Reference< XModifiable2 >   m_xDocumentModify;
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
        ((ODatabaseForm *)pCompImpl)->submit_impl(_rControl, *static_cast<const ::com::sun::star::awt::MouseEvent*>(_pEvt), true);
    else
        ((ODatabaseForm *)pCompImpl)->reset_impl(true);
}

//==================================================================
//= ODatabaseForm
//==================================================================

//------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseForm::Create( const Reference< XMultiServiceFactory >& _rxFactory )
{
    return *( new ODatabaseForm( comphelper::getComponentContext(_rxFactory) ) );
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

    Sequence< Type > aRet = concatSequences(
        aAggregateTypes, ODatabaseForm_BASE1::getTypes(), OFormComponents::getTypes()
    );
    aRet = concatSequences( aRet, ODatabaseForm_BASE2::getTypes(), ODatabaseForm_BASE3::getTypes() );
    return concatSequences( aRet, OPropertySetAggregationHelper::getTypes() );
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

DBG_NAME(ODatabaseForm);
//------------------------------------------------------------------
ODatabaseForm::ODatabaseForm(const Reference<XComponentContext>& _rxContext)
    :OFormComponents(_rxContext)
    ,OPropertySetAggregationHelper(OComponentHelper::rBHelper)
    ,OPropertyChangeListener(m_aMutex)
    ,m_aLoadListeners(m_aMutex)
    ,m_aRowSetApproveListeners(m_aMutex)
    ,m_aRowSetListeners(m_aMutex)
    ,m_aSubmitListeners(m_aMutex)
    ,m_aErrorListeners(m_aMutex)
    ,m_aResetListeners( *this, m_aMutex )
    ,m_aPropertyBagHelper( *this )
    ,m_pAggregatePropertyMultiplexer(NULL)
    ,m_pGroupManager( NULL )
    ,m_aParameterManager( m_aMutex, _rxContext )
    ,m_aFilterManager()
    ,m_pLoadTimer(NULL)
    ,m_pThread(NULL)
    ,m_nResetsPending(0)
    ,m_nPrivileges(0)
    ,m_bInsertOnly( sal_False )
    ,m_eSubmitMethod(FormSubmitMethod_GET)
    ,m_eSubmitEncoding(FormSubmitEncoding_URL)
    ,m_eNavigation(NavigationBarMode_CURRENT)
    ,m_bAllowInsert(sal_True)
    ,m_bAllowUpdate(sal_True)
    ,m_bAllowDelete(sal_True)
    ,m_bLoaded(sal_False)
    ,m_bSubForm(sal_False)
    ,m_bForwardingConnection(sal_False)
    ,m_bSharingConnection( sal_False )
{
    DBG_CTOR( ODatabaseForm, NULL );
    impl_construct();
}

//------------------------------------------------------------------
ODatabaseForm::ODatabaseForm( const ODatabaseForm& _cloneSource )
    :OFormComponents( _cloneSource )
    ,OPropertySetAggregationHelper( OComponentHelper::rBHelper )
    ,OPropertyChangeListener( m_aMutex )
    ,ODatabaseForm_BASE1()
    ,ODatabaseForm_BASE2()
    ,ODatabaseForm_BASE3()
    ,IPropertyBagHelperContext()
    ,m_aLoadListeners( m_aMutex )
    ,m_aRowSetApproveListeners( m_aMutex )
    ,m_aRowSetListeners( m_aMutex )
    ,m_aSubmitListeners( m_aMutex )
    ,m_aErrorListeners( m_aMutex )
    ,m_aResetListeners( *this, m_aMutex )
    ,m_aPropertyBagHelper( *this )
    ,m_pAggregatePropertyMultiplexer( NULL )
    ,m_pGroupManager( NULL )
    ,m_aParameterManager( m_aMutex, _cloneSource.m_xContext )
    ,m_aFilterManager()
    ,m_pLoadTimer( NULL )
    ,m_pThread( NULL )
    ,m_nResetsPending( 0 )
    ,m_nPrivileges( 0 )
    ,m_bInsertOnly( _cloneSource.m_bInsertOnly )
    ,m_aControlBorderColorFocus( _cloneSource.m_aControlBorderColorFocus )
    ,m_aControlBorderColorMouse( _cloneSource.m_aControlBorderColorMouse )
    ,m_aControlBorderColorInvalid( _cloneSource.m_aControlBorderColorInvalid )
    ,m_aDynamicControlBorder( _cloneSource.m_aDynamicControlBorder )
    ,m_sName( _cloneSource.m_sName )
    ,m_aTargetURL( _cloneSource.m_aTargetURL )
    ,m_aTargetFrame( _cloneSource.m_aTargetFrame )
    ,m_eSubmitMethod( _cloneSource.m_eSubmitMethod )
    ,m_eSubmitEncoding( _cloneSource.m_eSubmitEncoding )
    ,m_eNavigation( _cloneSource.m_eNavigation )
    ,m_bAllowInsert( _cloneSource.m_bAllowInsert )
    ,m_bAllowUpdate( _cloneSource.m_bAllowUpdate )
    ,m_bAllowDelete( _cloneSource.m_bAllowDelete )
    ,m_bLoaded( sal_False )
    ,m_bSubForm( sal_False )
    ,m_bForwardingConnection( sal_False )
    ,m_bSharingConnection( sal_False )
{
    DBG_CTOR( ODatabaseForm, NULL );

    impl_construct();

    osl_atomic_increment( &m_refCount );
    {
        // our aggregated rowset itself is not cloneable, so simply copy the properties
        ::comphelper::copyProperties( _cloneSource.m_xAggregateSet, m_xAggregateSet );

        // also care for the dynamic properties: If the clone source has properties which we do not have,
        // then add them
        try
        {
            Reference< XPropertySet > xSourceProps( const_cast< ODatabaseForm& >( _cloneSource ).queryAggregation(
                XPropertySet::static_type() ), UNO_QUERY_THROW );
            Reference< XPropertySetInfo > xSourcePSI( xSourceProps->getPropertySetInfo(), UNO_SET_THROW );
            Reference< XPropertyState > xSourcePropState( xSourceProps, UNO_QUERY );

            Reference< XPropertySetInfo > xDestPSI( getPropertySetInfo(), UNO_QUERY_THROW );

            Sequence< Property > aSourceProperties( xSourcePSI->getProperties() );
            for (   const Property* pSourceProperty = aSourceProperties.getConstArray();
                    pSourceProperty != aSourceProperties.getConstArray() + aSourceProperties.getLength();
                    ++pSourceProperty
                )
            {
                if ( xDestPSI->hasPropertyByName( pSourceProperty->Name ) )
                    continue;

                // the initial value passed to XPropertyContainer is also used as default, usually. So, try
                // to retrieve the default of the source property
                Any aInitialValue;
                if ( xSourcePropState.is() )
                {
                    aInitialValue = xSourcePropState->getPropertyDefault( pSourceProperty->Name );
                }
                else
                {
                    aInitialValue = xSourceProps->getPropertyValue( pSourceProperty->Name );
                }
                addProperty( pSourceProperty->Name, pSourceProperty->Attributes, aInitialValue );
                setPropertyValue( pSourceProperty->Name, xSourceProps->getPropertyValue( pSourceProperty->Name ) );
            }
        }
        catch(const Exception&)
        {
            throw WrappedTargetException(
                OUString( "Could not clone the given database form." ),
                *const_cast< ODatabaseForm* >( &_cloneSource ),
                ::cppu::getCaughtException()
            );
        }
    }
    osl_atomic_decrement( &m_refCount );
}

//------------------------------------------------------------------
void ODatabaseForm::impl_construct()
{
    // aggregate a row set
    increment(m_refCount);
    {
        m_xAggregate = Reference< XAggregation >( m_xContext->getServiceManager()->createInstanceWithContext(SRV_SDB_ROWSET, m_xContext), UNO_QUERY_THROW );
        m_xAggregateAsRowSet.set( m_xAggregate, UNO_QUERY_THROW );
        setAggregation( m_xAggregate );
    }

    // listen for the properties, important for Parameters
    if ( m_xAggregateSet.is() )
    {
        m_pAggregatePropertyMultiplexer = new OPropertyChangeMultiplexer(this, m_xAggregateSet, sal_False);
        m_pAggregatePropertyMultiplexer->acquire();
        m_pAggregatePropertyMultiplexer->addProperty(PROPERTY_COMMAND);
        m_pAggregatePropertyMultiplexer->addProperty(PROPERTY_ACTIVE_CONNECTION);
    }

    {
        Reference< XWarningsSupplier > xRowSetWarnings( m_xAggregate, UNO_QUERY );
        m_aWarnings.setExternalWarnings( xRowSetWarnings );
    }

    if ( m_xAggregate.is() )
    {
        m_xAggregate->setDelegator( static_cast< XWeak* >( this ) );
    }

    {
        m_aFilterManager.initialize( m_xAggregateSet );
        m_aParameterManager.initialize( this, m_xAggregate );

        declareForwardedProperty( PROPERTY_ID_ACTIVE_CONNECTION );
    }
    decrement( m_refCount );

    m_pGroupManager = new OGroupManager( this );
    m_pGroupManager->acquire();
}

//------------------------------------------------------------------
ODatabaseForm::~ODatabaseForm()
{
    DBG_DTOR(ODatabaseForm,NULL);

    m_pGroupManager->release();
    m_pGroupManager = NULL;

    if (m_xAggregate.is())
        m_xAggregate->setDelegator( NULL );

    m_aWarnings.setExternalWarnings( NULL );

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
OUString ODatabaseForm::GetDataURLEncoded(const Reference<XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{
    return GetDataEncoded(true,SubmitButton,MouseEvt);
}
// -----------------------------------------------------------------------------
OUString ODatabaseForm::GetDataEncoded(bool _bURLEncoded,const Reference<XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{
    // Fill List of successful Controls
    HtmlSuccessfulObjList aSuccObjList;
    FillSuccessfulList( aSuccObjList, SubmitButton, MouseEvt );


    // Aggregate Liste to OUString
    OUStringBuffer aResult;
    OUString aName;
    OUString aValue;

    for (   HtmlSuccessfulObjListIterator pSuccObj = aSuccObjList.begin();
            pSuccObj < aSuccObjList.end();
            ++pSuccObj
        )
    {
        aName = pSuccObj->aName;
        aValue = pSuccObj->aValue;
        if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_FILE && !aValue.isEmpty() )
        {
            // For File URLs we transfer the file name and not a URL, because Netscape does it like that
            INetURLObject aURL;
            aURL.SetSmartProtocol(INET_PROT_FILE);
            aURL.SetSmartURL(aValue);
            if( INET_PROT_FILE == aURL.GetProtocol() )
                aValue = INetURLObject::decode(aURL.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
        }
        Encode( aName );
        Encode( aValue );

        aResult.append(aName);
        aResult.append(sal_Unicode('='));
        aResult.append(aValue);

        if (pSuccObj < aSuccObjList.end() - 1)
        {
            if ( _bURLEncoded )
                aResult.append(sal_Unicode('&'));
            else
                aResult.appendAscii("\r\n");
        }
    }


    aSuccObjList.clear();

    return aResult.makeStringAndClear();
}

//==============================================================================
// html tools
//------------------------------------------------------------------------
OUString ODatabaseForm::GetDataTextEncoded(const Reference<XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{
    return GetDataEncoded(false,SubmitButton,MouseEvt);
}

//------------------------------------------------------------------------
Sequence<sal_Int8> ODatabaseForm::GetDataMultiPartEncoded(const Reference<XControl>& SubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt, OUString& rContentType)
{

    // Create Parent
    INetMIMEMessage aParent;
    aParent.EnableAttachChild( INETMSG_MULTIPART_FORM_DATA );


    // Fill List of successful Controls
    HtmlSuccessfulObjList aSuccObjList;
    FillSuccessfulList( aSuccObjList, SubmitButton, MouseEvt );


    // Aggregate Liste to OUString
    for (   HtmlSuccessfulObjListIterator pSuccObj = aSuccObjList.begin();
            pSuccObj < aSuccObjList.end();
            ++pSuccObj
        )
    {
        if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_TEXT )
            InsertTextPart( aParent, pSuccObj->aName, pSuccObj->aValue );
        else if( pSuccObj->nRepresentation == SUCCESSFUL_REPRESENT_FILE )
            InsertFilePart( aParent, pSuccObj->aName, pSuccObj->aValue );
    }


    // Delete List
    aSuccObjList.clear();

    // Create MessageStream for parent
    INetMIMEMessageStream aMessStream;
    aMessStream.SetSourceMessage( &aParent );
    aMessStream.GenerateHeader( sal_False );

    // Copy MessageStream to SvStream
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

    rContentType = aParent.GetContentType();
    return Sequence<sal_Int8>((sal_Int8*)pData, nLen);
}

//------------------------------------------------------------------------
namespace
{
    static void appendDigits( sal_Int32 _nNumber, sal_Int8 nDigits, OUStringBuffer& _rOut )
    {
        sal_Int32 nCurLen = _rOut.getLength();
        _rOut.append( _nNumber );
        while ( _rOut.getLength() - nCurLen < nDigits )
            _rOut.insert( nCurLen, (sal_Unicode)'0' );
    }
}

//------------------------------------------------------------------------
void ODatabaseForm::AppendComponent(HtmlSuccessfulObjList& rList, const Reference<XPropertySet>& xComponentSet, const OUString& rNamePrefix,
                     const Reference<XControl>& rxSubmitButton, const ::com::sun::star::awt::MouseEvent& MouseEvt)
{
    if (!xComponentSet.is())
        return;

    // MIB 25.6.98: Catch nested Forms; or would we need to submit them?
    if (!hasProperty(PROPERTY_CLASSID, xComponentSet))
        return;

    // Get names
    if (!hasProperty(PROPERTY_NAME, xComponentSet))
        return;

    sal_Int16 nClassId = 0;
    xComponentSet->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;
    OUString aName;
    xComponentSet->getPropertyValue( PROPERTY_NAME ) >>= aName;
    if( aName.isEmpty() && nClassId != FormComponentType::IMAGEBUTTON)
        return;
    else    // Extend name with the prefix
        aName = rNamePrefix + aName;

    switch( nClassId )
    {
        // Buttons
        case FormComponentType::COMMANDBUTTON:
        {
            // We only evaluate the pressed Submit button
            // MIB: If one is passed at all
            if( rxSubmitButton.is() )
            {
                Reference<XPropertySet>  xSubmitButtonComponent(rxSubmitButton->getModel(), UNO_QUERY);
                if (xSubmitButtonComponent == xComponentSet && hasProperty(PROPERTY_LABEL, xComponentSet))
                {
                    // <name>=<label>
                    OUString aLabel;
                    xComponentSet->getPropertyValue( PROPERTY_LABEL ) >>= aLabel;
                    rList.push_back( HtmlSuccessfulObj(aName, aLabel) );
                }
            }
        } break;

        // ImageButtons
        case FormComponentType::IMAGEBUTTON:
        {
            // We only evaluate the pressed Submit button
            // MIB: If one is passed at all
            if( rxSubmitButton.is() )
            {
                Reference<XPropertySet>  xSubmitButtonComponent(rxSubmitButton->getModel(), UNO_QUERY);
                if (xSubmitButtonComponent == xComponentSet)
                {
                    // <name>.x=<pos.X>&<name>.y=<pos.Y>
                    OUString aRhs = OUString::number( MouseEvt.X );

                    // Only if a name is available we have a name.x
                    OUStringBuffer aLhs(aName);
                    if (!aName.isEmpty())
                        aLhs.append(".x");
                    else
                        aLhs.append("x");
                    rList.push_back( HtmlSuccessfulObj(aLhs.makeStringAndClear(), aRhs) );

                    aLhs.append(aName);
                    aRhs = OUString::number( MouseEvt.Y );
                    if (!aName.isEmpty())
                        aLhs.append(".y");
                    else
                        aLhs.append("y");
                    rList.push_back( HtmlSuccessfulObj(aLhs.makeStringAndClear(), aRhs) );
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
            sal_Int16 nChecked = 0;
            xComponentSet->getPropertyValue( PROPERTY_STATE ) >>= nChecked;
            if( nChecked != 1 )
                break;

            OUString aStrValue;
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

            // MIB: Special treatment for multiline edit only if we have a control for it
            Any aTmp = xComponentSet->getPropertyValue( PROPERTY_MULTILINE );
            sal_Bool bMulti =   rxSubmitButton.is()
                            && (aTmp.getValueType().getTypeClass() == TypeClass_BOOLEAN)
                            && getBOOL(aTmp);
            OUString sText;
            if ( bMulti )   // For multiline edit, get the text at the control
            {

                Reference<XControlContainer>  xControlContainer(rxSubmitButton->getContext(), UNO_QUERY);
                if( !xControlContainer.is() ) break;

                Sequence<Reference<XControl> > aControlSeq = xControlContainer->getControls();
                Reference<XControl>  xControl;
                Reference<XFormComponent>  xControlComponent;

                // Find the right control
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
                // Couldn't find control or it does not exist (edit in the grid)
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
                OUString aText;
                xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= aText;
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        } break;
        case FormComponentType::CURRENCYFIELD:
        case FormComponentType::NUMERICFIELD:
        {
            // <name>=<wert> // Value is a double with dot as decimal delimiter
                             // no value (NULL) means empty value
            if( hasProperty(PROPERTY_VALUE, xComponentSet) )
            {
                OUString aText;
                Any aVal  = xComponentSet->getPropertyValue( PROPERTY_VALUE );

                double aDoubleVal = 0;
                if (aVal >>= aDoubleVal)
                {
                    sal_Int16 nScale = 0;
                    xComponentSet->getPropertyValue( PROPERTY_DECIMAL_ACCURACY ) >>= nScale;
                    aText = ::rtl::math::doubleToUString(aDoubleVal, rtl_math_StringFormat_F, nScale, '.', sal_True);
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }   break;
        case FormComponentType::DATEFIELD:
        {
            // <name>=<wert> // Value is a Date with the format MM-DD-YYYY
                             // no value (NULL) means empty value
            if( hasProperty(PROPERTY_DATE, xComponentSet) )
            {
                OUString aText;
                Any aVal  = xComponentSet->getPropertyValue( PROPERTY_DATE );
                sal_Int32 nInt32Val = 0;
                if (aVal >>= nInt32Val)
                {
                    ::Date aDate( nInt32Val );
                    OUStringBuffer aBuffer;
                    appendDigits( aDate.GetMonth(), 2, aBuffer );
                    aBuffer.append( (sal_Unicode)'-' );
                    appendDigits( aDate.GetDay(), 2, aBuffer );
                    aBuffer.append( (sal_Unicode)'-' );
                    appendDigits( aDate.GetYear(), 4, aBuffer );
                    aText = aBuffer.makeStringAndClear();
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }   break;
        case FormComponentType::TIMEFIELD:
        {
            // <name>=<wert> // Value is a Time with the format HH:MM:SS
                             // no value (NULL) means empty value
            if( hasProperty(PROPERTY_TIME, xComponentSet) )
            {
                OUString aText;
                Any aVal  = xComponentSet->getPropertyValue( PROPERTY_TIME );
                sal_Int32 nInt32Val = 0;
                if (aVal >>= nInt32Val)
                {
                    ::Time aTime(nInt32Val);
                    OUStringBuffer aBuffer;
                    appendDigits( aTime.GetHour(), 2, aBuffer );
                    aBuffer.append( (sal_Unicode)'-' );
                    appendDigits( aTime.GetMin(), 2, aBuffer );
                    aBuffer.append( (sal_Unicode)'-' );
                    appendDigits( aTime.GetSec(), 2, aBuffer );
                    aText = aBuffer.makeStringAndClear();
                }
                rList.push_back( HtmlSuccessfulObj(aName, aText) );
            }
        }   break;

        // starform
        case FormComponentType::HIDDENCONTROL:
        {

            // <name>=<value>
            if( hasProperty(PROPERTY_HIDDEN_VALUE, xComponentSet) )
            {
                OUString aText;
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

                OUString aText;
                xComponentSet->getPropertyValue( PROPERTY_TEXT ) >>= aText;
                rList.push_back( HtmlSuccessfulObj(aName, aText, SUCCESSFUL_REPRESENT_FILE) );
            }
        } break;

        // starform
        case FormComponentType::LISTBOX:
        {

            // <name>=<Token0>&<name>=<Token1>&...&<name>=<TokenN> (multiple selection)
            if (!hasProperty(PROPERTY_SELECT_SEQ, xComponentSet) ||
                !hasProperty(PROPERTY_STRINGITEMLIST, xComponentSet))
                break;

            // Displayed values
            Sequence< OUString > aVisibleList;
            xComponentSet->getPropertyValue( PROPERTY_STRINGITEMLIST ) >>= aVisibleList;
            sal_Int32 nStringCnt = aVisibleList.getLength();
            const OUString* pStrings = aVisibleList.getConstArray();

            // Value list
            Sequence< OUString > aValueList;
            xComponentSet->getPropertyValue( PROPERTY_VALUE_SEQ ) >>= aValueList;
            sal_Int32 nValCnt = aValueList.getLength();
            const OUString* pVals = aValueList.getConstArray();

            // Selection
            Sequence<sal_Int16> aSelectList;
            xComponentSet->getPropertyValue( PROPERTY_SELECT_SEQ ) >>= aSelectList;
            sal_Int32 nSelCount = aSelectList.getLength();
            const sal_Int16* pSels = aSelectList.getConstArray();

            // Simple or multiple selection
            // For simple selections MT only accounts for the list's first entry.
            if (nSelCount > 1 && !getBOOL(xComponentSet->getPropertyValue(PROPERTY_MULTISELECTION)))
                nSelCount = 1;

            // The indices in the selection list can also be invalid, so we first have to
            // find the valid ones to determine the length of the new list.
            sal_Int32 nCurCnt = 0;
            sal_Int32 i;
            for( i=0; i<nSelCount; ++i )
            {
                if( pSels[i] < nStringCnt )
                    ++nCurCnt;
            }

            OUString aSubValue;
            for(i=0; i<nCurCnt; ++i )
            {
                sal_Int16  nSelPos = pSels[i];
                if (nSelPos < nValCnt && !pVals[nSelPos].isEmpty())
                {
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
            // Each of the column values is sent;
            // the name is extended by the grid's prefix.
            Reference<XIndexAccess>  xContainer(xComponentSet, UNO_QUERY);
            if (!xContainer.is())
                break;

            aName += OUString(static_cast<sal_Unicode>('.'));

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
    // Delete list
    rList.clear();
    // Iterate over Components
    Reference<XPropertySet>         xComponentSet;
    OUString aPrefix;

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
void ODatabaseForm::Encode( OUString& rString ) const
{
    OUStringBuffer aResult;

    // Line endings are represented as CR
    rString = convertLineEnd(rString, LINEEND_CR);

    // Check each character
    sal_Int32 nStrLen = rString.getLength();
    sal_Unicode nCharCode;
    for( sal_Int32 nCurPos=0; nCurPos < nStrLen; ++nCurPos )
    {
        nCharCode = rString[nCurPos];

        // Handle chars, which are not an alphanumeric character and character codes > 127
        if( (!isalnum(nCharCode) && nCharCode != (sal_Unicode)' ') || nCharCode > 127 )
        {
            switch( nCharCode )
            {
                case 13:    // CR
                    aResult.append("%0D%0A"); // CR LF in hex
                    break;


                // Special treatment for Netscape
                case 42:    // '*'
                case 45:    // '-'
                case 46:    // '.'
                case 64:    // '@'
                case 95:    // '_'
                    aResult.append(nCharCode);
                    break;

                default:
                {
                    // Convert to hex
                    short nHi = ((sal_Int16)nCharCode) / 16;
                    short nLo = ((sal_Int16)nCharCode) - (nHi*16);
                    if( nHi > 9 ) nHi += (int)'A'-10; else nHi += (int)'0';
                    if( nLo > 9 ) nLo += (int)'A'-10; else nLo += (int)'0';
                    aResult.append('%');
                    aResult.append((sal_Unicode)nHi);
                    aResult.append((sal_Unicode)nLo);
                }
            }
        }
        else
            aResult.append(nCharCode);
    }

    // Replace spaces with '+'
    rString = aResult.makeStringAndClear().replace(' ', '+');
}

//------------------------------------------------------------------------
void ODatabaseForm::InsertTextPart( INetMIMEMessage& rParent, const OUString& rName,
    const OUString& rData )
{

    // Create part as MessageChild
    INetMIMEMessage* pChild = new INetMIMEMessage();


    // Header
    OUStringBuffer aContentDisp;
    aContentDisp.append("form-data; name=\"");
    aContentDisp.append(rName);
    aContentDisp.append('\"');
    pChild->SetContentDisposition(aContentDisp.makeStringAndClear());
    pChild->SetContentType(OUString("text/plain"));

    rtl_TextEncoding eSystemEncoding = osl_getThreadTextEncoding();
    const sal_Char* pBestMatchingEncoding = rtl_getBestMimeCharsetFromTextEncoding( eSystemEncoding );
    OUString aBestMatchingEncoding = OUString::createFromAscii(pBestMatchingEncoding);
    pChild->SetContentTransferEncoding(aBestMatchingEncoding);

    // Body
    SvMemoryStream* pStream = new SvMemoryStream;
    pStream->WriteLine( OUStringToOString(rData, rtl_getTextEncodingFromMimeCharset(pBestMatchingEncoding)) );
    pStream->Flush();
    pStream->Seek( 0 );
    pChild->SetDocumentLB( new SvLockBytes(pStream, sal_True) );
    rParent.AttachChild( *pChild );
}

//------------------------------------------------------------------------
sal_Bool ODatabaseForm::InsertFilePart( INetMIMEMessage& rParent, const OUString& rName,
    const OUString& rFileName )
{
    OUString aFileName(rFileName);
    OUString aContentType(CONTENT_TYPE_STR_TEXT_PLAIN);
    SvStream *pStream = 0;

    if (!aFileName.isEmpty())
    {
        // We can only process File URLs yet
        INetURLObject aURL;
        aURL.SetSmartProtocol(INET_PROT_FILE);
        aURL.SetSmartURL(rFileName);
        if( INET_PROT_FILE == aURL.GetProtocol() )
        {
            aFileName = INetURLObject::decode(aURL.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
            pStream = ::utl::UcbStreamHelper::CreateStream(aFileName, STREAM_READ);
            if (!pStream || (pStream->GetError() != ERRCODE_NONE))
            {
                delete pStream;
                pStream = 0;
            }
            sal_Int32 nSepInd = aFileName.lastIndexOf('.');
            OUString aExtension = aFileName.copy( nSepInd + 1, aFileName.getLength() - nSepInd - 1 );
            INetContentType eContentType = INetContentTypes::GetContentType4Extension( aExtension );
            if (eContentType != CONTENT_TYPE_UNKNOWN)
                aContentType = INetContentTypes::GetContentType(eContentType);
        }
    }

    // If something didn't work, we create an empty MemoryStream
    if( !pStream )
        pStream = new SvMemoryStream;


    // Create part as MessageChild
    INetMIMEMessage* pChild = new INetMIMEMessage;


    // Header
    OUStringBuffer aContentDisp;
    aContentDisp.append("form-data; name=\"");
    aContentDisp.append(rName);
    aContentDisp.append('\"');
    aContentDisp.append("; filename=\"");
    aContentDisp.append(aFileName);
    aContentDisp.append('\"');
    pChild->SetContentDisposition(aContentDisp.makeStringAndClear());
    pChild->SetContentType( aContentType );
    pChild->SetContentTransferEncoding(OUString("8bit"));


    // Body
    pChild->SetDocumentLB( new SvLockBytes(pStream, sal_True) );
    rParent.AttachChild( *pChild );

    return sal_True;
}

//==============================================================================
// internals
//------------------------------------------------------------------------------
void ODatabaseForm::onError( const SQLErrorEvent& _rEvent )
{
    m_aErrorListeners.notifyEach( &XSQLErrorListener::errorOccured, _rEvent );
}

//------------------------------------------------------------------------------
void ODatabaseForm::onError( const SQLException& _rException, const OUString& _rContextDescription )
{
    if ( !m_aErrorListeners.getLength() )
        return;

    SQLErrorEvent aEvent( *this, makeAny( prependErrorInfo( _rException, *this, _rContextDescription ) ) );
    onError( aEvent );
}

//------------------------------------------------------------------------------
void ODatabaseForm::updateParameterInfo()
{
    m_aParameterManager.updateParameterInfo( m_aFilterManager );
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
            OSL_FAIL("ODatabaseForm::hasValidParent() : no parent resultset !");
            return false;
        }
        try
        {
            Reference< XPropertySet >  xSet( m_xParent, UNO_QUERY );
            Reference< XLoadable > xLoad( m_xParent, UNO_QUERY );
            if  (   xLoad->isLoaded()
                &&  (   xResultSet->isBeforeFirst()
                    ||  xResultSet->isAfterLast()
                    ||  getBOOL( xSet->getPropertyValue( PROPERTY_ISNEW ) )
                    )
                )
                // the parent form is loaded and on a "virtual" row -> not valid
                return false;
        }
        catch(const Exception&)
        {
            // parent could be forwardonly?
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
bool ODatabaseForm::fillParameters( ::osl::ResettableMutexGuard& _rClearForNotifies, const Reference< XInteractionHandler >& _rxCompletionHandler )
{
    // do we have to fill the parameters again?
    if ( !m_aParameterManager.isUpToDate() )
        updateParameterInfo();

    // is there a valid parent?
    if ( m_bSubForm && !hasValidParent() )
        return true;

    // ensure we're connected
    if ( !implEnsureConnection() )
        return false;

    if ( m_aParameterManager.isUpToDate() )
        return m_aParameterManager.fillParameterValues( _rxCompletionHandler, _rClearForNotifies );

    return true;
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
    sal_Int32 nConcurrency = ResultSetConcurrency::READ_ONLY;

    // if we have a parent, who is not positioned on a valid row
    // we can't be updatable!
    if (m_bSubForm && !hasValidParent())
    {
        nConcurrency = ResultSetConcurrency::READ_ONLY;

        // don't use any parameters if we don't have a valid parent
        m_aParameterManager.setAllParametersNull();

        // switch to "insert only" mode
        saveInsertOnlyState( );
        m_xAggregateSet->setPropertyValue( PROPERTY_INSERTONLY, makeAny( sal_True ) );
    }
    else if (m_bAllowInsert || m_bAllowUpdate || m_bAllowDelete)
        nConcurrency = ResultSetConcurrency::UPDATABLE;
    else
        nConcurrency = ResultSetConcurrency::READ_ONLY;

    m_xAggregateSet->setPropertyValue( PROPERTY_RESULTSET_CONCURRENCY, makeAny( (sal_Int32)nConcurrency ) );
    m_xAggregateSet->setPropertyValue( PROPERTY_RESULTSET_TYPE, makeAny( (sal_Int32)ResultSetType::SCROLL_SENSITIVE ) );

    sal_Bool bSuccess = sal_False;
    try
    {
        m_xAggregateAsRowSet->execute();
        bSuccess = sal_True;
    }
    catch(const RowSetVetoException&)
    {
    }
    catch(const SQLException& eDb)
    {
        _rClearForNotifies.clear();
        if (!m_sCurrentErrorContext.isEmpty())
            onError(eDb, m_sCurrentErrorContext);
        else
            onError(eDb, FRM_RES_STRING(RID_STR_READERROR));
        _rClearForNotifies.reset();

        restoreInsertOnlyState( );
    }

    if (bSuccess)
    {
        // adjust the privilege property
        //  m_nPrivileges;
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
            catch(const SQLException& eDB)
            {
                _rClearForNotifies.clear();
                if (!m_sCurrentErrorContext.isEmpty())
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
    m_aParameterManager.disposing( aEvt );
    m_aResetListeners.disposing();
    m_aSubmitListeners.disposeAndClear(aEvt);
    m_aErrorListeners.disposeAndClear(aEvt);

    m_aParameterManager.dispose();   // (to free any references it may have to me)
    m_aFilterManager.dispose();      // (dito)

    OFormComponents::disposing();
    OPropertySetAggregationHelper::disposing();

    // stop listening on the aggregate
    if (m_xAggregateAsRowSet.is())
        m_xAggregateAsRowSet->removeRowSetListener(this);

    // dispose the active connection
    Reference<XComponent>  xAggregationComponent;
    if (query_aggregation(m_xAggregate, xAggregationComponent))
        xAggregationComponent->dispose();

    m_aPropertyBagHelper.dispose();
}

//------------------------------------------------------------------------------
Reference< XConnection > ODatabaseForm::getConnection()
{
    Reference< XConnection > xConn;
    m_xAggregateSet->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConn;
    return xConn;
}

//------------------------------------------------------------------------------
::osl::Mutex& ODatabaseForm::getMutex()
{
    return m_aMutex;
}

//==============================================================================
// property handling
//------------------------------------------------------------------------------
void ODatabaseForm::describeFixedAndAggregateProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    BEGIN_DESCRIBE_AGGREGATION_PROPERTIES(22, m_xAggregateSet)
        // we want to "override" the privileges, since we have additional "AllowInsert" etc. properties
        RemoveProperty( _rAggregateProps, PROPERTY_PRIVILEGES );

        // InsertOnly is also to be overridden, since we sometimes change it ourself
        RemoveProperty( _rAggregateProps, PROPERTY_INSERTONLY );

        // we remove and re-declare the DataSourceName property, 'cause we want it to be constrained, and the
        // original property of our aggregate isn't
        RemoveProperty( _rAggregateProps, PROPERTY_DATASOURCE );

        // for connection sharing, we need to override the ActiveConnection property, too
        RemoveProperty( _rAggregateProps, PROPERTY_ACTIVE_CONNECTION );

        // the Filter property is also overwritten, since we have some implicit filters
        // (e.g. the ones which result from linking master fields to detail fields
        // via column names instead of parameters)
        RemoveProperty( _rAggregateProps, PROPERTY_FILTER );
        RemoveProperty( _rAggregateProps, PROPERTY_APPLYFILTER );

        DECL_IFACE_PROP4(ACTIVE_CONNECTION, XConnection,                    BOUND, TRANSIENT, MAYBEVOID, CONSTRAINED);
        DECL_BOOL_PROP2 ( APPLYFILTER,                                      BOUND, MAYBEDEFAULT            );
        DECL_PROP1      ( NAME,             OUString,                BOUND                          );
        DECL_PROP1      ( MASTERFIELDS,     Sequence< OUString >,    BOUND                          );
        DECL_PROP1      ( DETAILFIELDS,     Sequence< OUString >,    BOUND                          );
        DECL_PROP2      ( DATASOURCE,       OUString,                BOUND, CONSTRAINED             );
        DECL_PROP3      ( CYCLE,            TabulatorCycle,                 BOUND, MAYBEVOID, MAYBEDEFAULT );
        DECL_PROP2      ( FILTER,           OUString,                BOUND, MAYBEDEFAULT            );
        DECL_BOOL_PROP2 ( INSERTONLY,                                       BOUND, MAYBEDEFAULT            );
        DECL_PROP1      ( NAVIGATION,       NavigationBarMode,              BOUND                          );
        DECL_BOOL_PROP1 ( ALLOWADDITIONS,                                   BOUND                          );
        DECL_BOOL_PROP1 ( ALLOWEDITS,                                       BOUND                          );
        DECL_BOOL_PROP1 ( ALLOWDELETIONS,                                   BOUND                          );
        DECL_PROP2      ( PRIVILEGES,       sal_Int32,                      TRANSIENT, READONLY            );
        DECL_PROP1      ( TARGET_URL,       OUString,                BOUND                          );
        DECL_PROP1      ( TARGET_FRAME,     OUString,                BOUND                          );
        DECL_PROP1      ( SUBMIT_METHOD,    FormSubmitMethod,               BOUND                          );
        DECL_PROP1      ( SUBMIT_ENCODING,  FormSubmitEncoding,             BOUND                          );
        DECL_BOOL_PROP3 ( DYNAMIC_CONTROL_BORDER,                           BOUND, MAYBEVOID, MAYBEDEFAULT );
        DECL_PROP3      ( CONTROL_BORDER_COLOR_FOCUS,   sal_Int32,          BOUND, MAYBEVOID, MAYBEDEFAULT );
        DECL_PROP3      ( CONTROL_BORDER_COLOR_MOUSE,   sal_Int32,          BOUND, MAYBEVOID, MAYBEDEFAULT );
        DECL_PROP3      ( CONTROL_BORDER_COLOR_INVALID, sal_Int32,          BOUND, MAYBEVOID, MAYBEDEFAULT );
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
Reference< XMultiPropertySet > ODatabaseForm::getPropertiesInterface()
{
    return Reference< XMultiPropertySet >( *this, UNO_QUERY );
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ODatabaseForm::getInfoHelper()
{
    return m_aPropertyBagHelper.getInfoHelper();
}

//------------------------------------------------------------------------------
Reference< XPropertySetInfo > ODatabaseForm::getPropertySetInfo() throw( RuntimeException )
{
    return createPropertySetInfo( getInfoHelper() );
}

//--------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addProperty( const OUString& _rName, ::sal_Int16 _nAttributes, const Any& _rInitialValue ) throw (PropertyExistException, IllegalTypeException, IllegalArgumentException, RuntimeException)
{
    m_aPropertyBagHelper.addProperty( _rName, _nAttributes, _rInitialValue );
}

//--------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeProperty( const OUString& _rName ) throw (UnknownPropertyException, NotRemoveableException, RuntimeException)
{
    m_aPropertyBagHelper.removeProperty( _rName );
}

//--------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL ODatabaseForm::getPropertyValues() throw (RuntimeException)
{
    return m_aPropertyBagHelper.getPropertyValues();
}

//--------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setPropertyValues( const Sequence< PropertyValue >& _rProps ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    m_aPropertyBagHelper.setPropertyValues( _rProps );
}

//------------------------------------------------------------------------------
Any SAL_CALL ODatabaseForm::getWarnings(  ) throw (SQLException, RuntimeException)
{
    return m_aWarnings.getWarnings();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::clearWarnings(  ) throw (SQLException, RuntimeException)
{
    m_aWarnings.clearWarnings();
}

//------------------------------------------------------------------------------
Reference< XCloneable > SAL_CALL ODatabaseForm::createClone(  ) throw (RuntimeException)
{
    ODatabaseForm* pClone = new ODatabaseForm( *this );
    osl_atomic_increment( &pClone->m_refCount );
    pClone->clonedFrom( *this );
    osl_atomic_decrement( &pClone->m_refCount );
    return pClone;
}

//------------------------------------------------------------------------------
void ODatabaseForm::fire( sal_Int32* pnHandles, const Any* pNewValues, const Any* pOldValues, sal_Int32 nCount, sal_Bool bVetoable )
{
    // same as in getFastPropertyValue(sal_Int32) : if we're resetting currently don't fire any changes of the
    // IsModified property from sal_False to sal_True, as this is only temporary 'til the reset is done
    if (m_nResetsPending > 0)
    {
        // look for the PROPERTY_ID_ISMODIFIED
        sal_Int32 nPos = 0;
        for (nPos=0; nPos<nCount; ++nPos)
            if (pnHandles[nPos] == PROPERTY_ID_ISMODIFIED)
                break;

        if ((nPos < nCount) && (pNewValues[nPos].getValueType().getTypeClass() == TypeClass_BOOLEAN) && getBOOL(pNewValues[nPos]))
        {   // yeah, we found it, and it changed to TRUE
            if (nPos == 0)
            {   // just cut the first element
                ++pnHandles;
                ++pNewValues;
                ++pOldValues;
                --nCount;
            }
            else if (nPos == nCount - 1)
                // just cut the last element
                --nCount;
            else
            {   // split into two base class calls
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
        // don't allow the aggregate which is currently being reset to return a (temporary) "yes"
    else
        return OPropertySetAggregationHelper::getFastPropertyValue(nHandle);
}

//------------------------------------------------------------------------------
void ODatabaseForm::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_INSERTONLY:
            rValue <<= m_bInsertOnly;
            break;

        case PROPERTY_ID_FILTER:
            rValue <<= m_aFilterManager.getFilterComponent( FilterManager::fcPublicFilter );
            break;

        case PROPERTY_ID_APPLYFILTER:
            rValue <<= m_aFilterManager.isApplyPublicFilter();
            break;

        case PROPERTY_ID_DATASOURCE:
            rValue = m_xAggregateSet->getPropertyValue( PROPERTY_DATASOURCE );
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
        case PROPERTY_ID_DYNAMIC_CONTROL_BORDER:
            rValue = m_aDynamicControlBorder;
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_FOCUS:
            rValue = m_aControlBorderColorFocus;
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_MOUSE:
            rValue = m_aControlBorderColorMouse;
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_INVALID:
            rValue = m_aControlBorderColorInvalid;
            break;
        default:
            if ( m_aPropertyBagHelper.hasDynamicPropertyByHandle( nHandle ) )
                m_aPropertyBagHelper.getDynamicFastPropertyValue( nHandle, rValue );
            else
                OPropertySetAggregationHelper::getFastPropertyValue( rValue, nHandle );
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
        case PROPERTY_ID_INSERTONLY:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_bInsertOnly );
            break;

        case PROPERTY_ID_FILTER:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_aFilterManager.getFilterComponent( FilterManager::fcPublicFilter ) );
            break;

        case PROPERTY_ID_APPLYFILTER:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_aFilterManager.isApplyPublicFilter() );
            break;

        case PROPERTY_ID_DATASOURCE:
        {
            Any aAggregateProperty;
            getFastPropertyValue(aAggregateProperty, PROPERTY_ID_DATASOURCE);
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, aAggregateProperty, ::getCppuType(static_cast<const OUString*>(NULL)));
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
        case PROPERTY_ID_DYNAMIC_CONTROL_BORDER:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_aDynamicControlBorder, ::getBooleanCppuType() );
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_FOCUS:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_aControlBorderColorFocus, getCppuType( static_cast< sal_Int32* >( NULL ) ) );
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_MOUSE:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_aControlBorderColorMouse, getCppuType( static_cast< sal_Int32* >( NULL ) ) );
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_INVALID:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_aControlBorderColorInvalid, getCppuType( static_cast< sal_Int32* >( NULL ) ) );
            break;
        default:
            if ( m_aPropertyBagHelper.hasDynamicPropertyByHandle ( nHandle ) )
                bModified = m_aPropertyBagHelper.convertDynamicFastPropertyValue( nHandle, rValue, rConvertedValue, rOldValue );
            else
                bModified = OPropertySetAggregationHelper::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void ODatabaseForm::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw( Exception )
{
    switch (nHandle)
    {
        case PROPERTY_ID_INSERTONLY:
            rValue >>= m_bInsertOnly;
            if ( m_aIgnoreResult.hasValue() )
                m_aIgnoreResult <<= m_bInsertOnly;
            else
                m_xAggregateSet->setPropertyValue( PROPERTY_INSERTONLY, makeAny( m_bInsertOnly ) );
            break;

        case PROPERTY_ID_FILTER:
        {
            OUString sNewFilter;
            rValue >>= sNewFilter;
            m_aFilterManager.setFilterComponent( FilterManager::fcPublicFilter, sNewFilter );
        }
        break;

        case PROPERTY_ID_APPLYFILTER:
        {
            sal_Bool bApply = sal_True;
            rValue >>= bApply;
            m_aFilterManager.setApplyPublicFilter( bApply );
        }
        break;

        case PROPERTY_ID_DATASOURCE:
        {
            Reference< XConnection > xSomeConnection;
            if ( ::dbtools::isEmbeddedInDatabase( getParent(), xSomeConnection ) )
                throw PropertyVetoException();

            try
            {
                m_xAggregateSet->setPropertyValue(PROPERTY_DATASOURCE, rValue);
            }
            catch(const Exception&)
            {
            }
        }
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
        case PROPERTY_ID_DYNAMIC_CONTROL_BORDER:
            m_aDynamicControlBorder = rValue;
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_FOCUS:
            m_aControlBorderColorFocus = rValue;
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_MOUSE:
            m_aControlBorderColorMouse = rValue;
            break;
        case PROPERTY_ID_CONTROL_BORDER_COLOR_INVALID:
            m_aControlBorderColorInvalid = rValue;
            break;

        case PROPERTY_ID_ACTIVE_CONNECTION:
        {
            Reference< XConnection > xOuterConnection;
            if ( ::dbtools::isEmbeddedInDatabase( getParent(), xOuterConnection ) )
            {
                if ( xOuterConnection != Reference< XConnection >( rValue, UNO_QUERY ) )
                    // somebody's trying to set a connection which is not equal the connection
                    // implied by the database we're embedded in
                    throw PropertyVetoException();
            }
            OPropertySetAggregationHelper::setFastPropertyValue_NoBroadcast( nHandle, rValue );
            break;
        }

        default:
            if ( m_aPropertyBagHelper.hasDynamicPropertyByHandle( nHandle ) )
                m_aPropertyBagHelper.setDynamicFastPropertyValue( nHandle, rValue );
            else
                OPropertySetAggregationHelper::setFastPropertyValue_NoBroadcast( nHandle, rValue );
            break;
    }
}

//------------------------------------------------------------------
void SAL_CALL ODatabaseForm::forwardingPropertyValue( sal_Int32 _nHandle )
{
    OSL_ENSURE( _nHandle == PROPERTY_ID_ACTIVE_CONNECTION, "ODatabaseForm::forwardingPropertyValue: unexpected property!" );
    if ( _nHandle == PROPERTY_ID_ACTIVE_CONNECTION )
    {
        if ( m_bSharingConnection )
            stopSharingConnection( );
        m_bForwardingConnection = sal_True;
    }
}

//------------------------------------------------------------------
void SAL_CALL ODatabaseForm::forwardedPropertyValue( sal_Int32 _nHandle, bool /*_bSuccess*/ )
{
    OSL_ENSURE( _nHandle == PROPERTY_ID_ACTIVE_CONNECTION, "ODatabaseForm::forwardedPropertyValue: unexpected property!" );
    if ( _nHandle == PROPERTY_ID_ACTIVE_CONNECTION )
    {
        m_bForwardingConnection = sal_False;
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

        case PROPERTY_ID_CYCLE:
            eState = m_aCycle.hasValue() ? PropertyState_DIRECT_VALUE : PropertyState_DEFAULT_VALUE;
            break;

        case PROPERTY_ID_INSERTONLY:
            eState = m_bInsertOnly ? PropertyState_DIRECT_VALUE : PropertyState_DEFAULT_VALUE;
            break;

        case PROPERTY_ID_FILTER:
            if ( m_aFilterManager.getFilterComponent( FilterManager::fcPublicFilter ).isEmpty() )
                eState = PropertyState_DEFAULT_VALUE;
            else
                eState = PropertyState_DIRECT_VALUE;
            break;

        case PROPERTY_ID_APPLYFILTER:
            eState = m_aFilterManager.isApplyPublicFilter() ? PropertyState_DEFAULT_VALUE : PropertyState_DIRECT_VALUE;
            break;

        case PROPERTY_ID_DYNAMIC_CONTROL_BORDER:
            eState = m_aDynamicControlBorder.hasValue() ? PropertyState_DIRECT_VALUE : PropertyState_DEFAULT_VALUE;
            break;

        case PROPERTY_ID_CONTROL_BORDER_COLOR_FOCUS:
            eState = m_aControlBorderColorFocus.hasValue() ? PropertyState_DIRECT_VALUE : PropertyState_DEFAULT_VALUE;
            break;

        case PROPERTY_ID_CONTROL_BORDER_COLOR_MOUSE:
            eState = m_aControlBorderColorMouse.hasValue() ? PropertyState_DIRECT_VALUE : PropertyState_DEFAULT_VALUE;
            break;

        case PROPERTY_ID_CONTROL_BORDER_COLOR_INVALID:
            eState = m_aControlBorderColorInvalid.hasValue() ? PropertyState_DIRECT_VALUE : PropertyState_DEFAULT_VALUE;
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
        case PROPERTY_ID_INSERTONLY:
        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_APPLYFILTER:
        case PROPERTY_ID_NAVIGATION:
        case PROPERTY_ID_CYCLE:
        case PROPERTY_ID_DYNAMIC_CONTROL_BORDER:
        case PROPERTY_ID_CONTROL_BORDER_COLOR_FOCUS:
        case PROPERTY_ID_CONTROL_BORDER_COLOR_MOUSE:
        case PROPERTY_ID_CONTROL_BORDER_COLOR_INVALID:
            setFastPropertyValue( nHandle, getPropertyDefaultByHandle( nHandle ) );
            break;

        default:
            OPropertySetAggregationHelper::setPropertyToDefaultByHandle(nHandle);
    }
}

//------------------------------------------------------------------
Any ODatabaseForm::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    Any aReturn;
    switch (nHandle)
    {
        case PROPERTY_ID_INSERTONLY:
        case PROPERTY_ID_DYNAMIC_CONTROL_BORDER:
            aReturn <<= sal_False;
            break;

        case PROPERTY_ID_FILTER:
            aReturn <<= OUString();
            break;

        case PROPERTY_ID_APPLYFILTER:
            aReturn <<= sal_True;
            break;

        case PROPERTY_ID_NAVIGATION:
            aReturn = makeAny(NavigationBarMode_CURRENT);
            break;

        case PROPERTY_ID_CYCLE:
        case PROPERTY_ID_CONTROL_BORDER_COLOR_FOCUS:
        case PROPERTY_ID_CONTROL_BORDER_COLOR_MOUSE:
        case PROPERTY_ID_CONTROL_BORDER_COLOR_INVALID:
            break;

        default:
            if ( m_aPropertyBagHelper.hasDynamicPropertyByHandle( nHandle ) )
                m_aPropertyBagHelper.getDynamicPropertyDefaultByHandle( nHandle, aReturn );
            else
                aReturn = OPropertySetAggregationHelper::getPropertyDefaultByHandle( nHandle );
            break;
    }
    return aReturn;
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

    if ( !m_aResetListeners.empty() )
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
    if ( _bAproveByListeners )
        if ( !m_aResetListeners.approveReset() )
            return;

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

                static const OUString PROPERTY_CONTROLDEFAULT( "ControlDefault" );
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
                        catch(const Exception&)
                        {
                            DBG_UNHANDLED_EXCEPTION();
                        }
                    }
                }
            }
        }
        catch(const Exception&)
        {
        }

        if (m_bSubForm)
        {
            Reference< XColumnsSupplier > xParentColSupp( m_xParent, UNO_QUERY );
            Reference< XNameAccess >      xParentCols;
            if ( xParentColSupp.is() )
                xParentCols = xParentColSupp->getColumns();

            if ( xParentCols.is() && xParentCols->hasElements() && m_aMasterFields.getLength() )
            {
                try
                {
                    // analyze our parameters
                    if ( !m_aParameterManager.isUpToDate() )
                        updateParameterInfo();

                    m_aParameterManager.resetParameterValues( );
                }
                catch(const Exception&)
                {
                    OSL_FAIL("ODatabaseForm::reset_impl: could not initialize the master-detail-driven parameters!");
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
    // on the modified state of the row)
    if (bInsertRow)
        m_xAggregateSet->setPropertyValue(PROPERTY_ISMODIFIED, ::cppu::bool2any(sal_Bool(sal_False)));

    aResetGuard.clear();
    {
        m_aResetListeners.resetted();
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
    m_aResetListeners.addTypedListener( _rListener );
}

//-----------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeResetListener(const Reference<XResetListener>& _rListener) throw( RuntimeException )
{
    m_aResetListeners.removeTypedListener( _rListener );
}

//==============================================================================
// com::sun::star::form::XSubmit
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::submit( const Reference<XControl>& Control,
                              const ::com::sun::star::awt::MouseEvent& MouseEvt ) throw( RuntimeException )
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        // Do we have controls and a Submit URL?
        if( !getCount() || m_aTargetURL.isEmpty() )
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
// -----------------------------------------------------------------------------
void lcl_dispatch(const Reference< XFrame >& xFrame,const Reference<XURLTransformer>& xTransformer,const OUString& aURLStr,const OUString& aReferer,const OUString& aTargetName
                  ,const OUString& aData,rtl_TextEncoding _eEncoding)
{
    URL aURL;
    aURL.Complete = aURLStr;
    xTransformer->parseStrict(aURL);

    Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch(aURL, aTargetName,
        FrameSearchFlag::SELF | FrameSearchFlag::PARENT | FrameSearchFlag::CHILDREN |
        FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE | FrameSearchFlag::TASKS);

    if (xDisp.is())
    {
        Sequence<PropertyValue> aArgs(2);
        aArgs.getArray()[0].Name = OUString("Referer");
        aArgs.getArray()[0].Value <<= aReferer;

        // build a sequence from the to-be-submitted string
        OString a8BitData(OUStringToOString(aData, _eEncoding));
        // always ANSI #58641
        Sequence< sal_Int8 > aPostData((const sal_Int8*)a8BitData.getStr(), a8BitData.getLength());
        Reference< XInputStream > xPostData = new SequenceInputStream(aPostData);

        aArgs.getArray()[1].Name = OUString("PostData");
        aArgs.getArray()[1].Value <<= xPostData;

        xDisp->dispatch(aURL, aArgs);
    } // if (xDisp.is())
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
    OUString aURLStr;
    OUString aReferer;
    OUString aTargetName;
    Reference< XModel >  xModel;
    {
        SolarMutexGuard aGuard;
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
    Reference< XFrame >  xFrame = xModel->getCurrentController()->getFrame();
    if (!xFrame.is())
        return;

    Reference<XURLTransformer> xTransformer(URLTransformer::create(m_xContext));

    // URL encoding
    if( eSubmitEncoding == FormSubmitEncoding_URL )
    {
        OUString aData;
        {
            SolarMutexGuard aGuard;
            aData = GetDataURLEncoded( Control, MouseEvt );
        }

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
                aArgs.getArray()->Name = OUString("Referer");
                aArgs.getArray()->Value <<= aReferer;
                xDisp->dispatch(aURL, aArgs);
            }
        }
        // FormMethod POST
        else if( eSubmitMethod == FormSubmitMethod_POST )
        {
            lcl_dispatch(xFrame,xTransformer,aURLStr,aReferer,aTargetName,aData,RTL_TEXTENCODING_MS_1252);
        }
    }
    else if( eSubmitEncoding == FormSubmitEncoding_MULTIPART )
    {
        URL aURL;
        aURL.Complete = aURLStr;
        xTransformer->parseStrict(aURL);

        Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch(aURL, aTargetName,
                FrameSearchFlag::SELF | FrameSearchFlag::PARENT | FrameSearchFlag::CHILDREN |
                FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE | FrameSearchFlag::TASKS);

        if (xDisp.is())
        {
            OUString aContentType;
            Sequence<sal_Int8> aData;
            {
                SolarMutexGuard aGuard;
                aData = GetDataMultiPartEncoded(Control, MouseEvt, aContentType);
            }
            if (!aData.getLength())
                return;

            Sequence<PropertyValue> aArgs(3);
            aArgs.getArray()[0].Name = OUString("Referer");
            aArgs.getArray()[0].Value <<= aReferer;
            aArgs.getArray()[1].Name = OUString("ContentType");
            aArgs.getArray()[1].Value <<= aContentType;

            // build a sequence from the to-be-submitted string
            Reference< XInputStream > xPostData = new SequenceInputStream(aData);

            aArgs.getArray()[2].Name = OUString("PostData");
            aArgs.getArray()[2].Value <<= xPostData;

            xDisp->dispatch(aURL, aArgs);
        }
    }
    else if( eSubmitEncoding == FormSubmitEncoding_TEXT )
    {
        OUString aData;
        {
            SolarMutexGuard aGuard;
            aData = GetDataTextEncoded( Reference<XControl> (), MouseEvt );
        }

        lcl_dispatch(xFrame,xTransformer,aURLStr,aReferer,aTargetName,aData,osl_getThreadTextEncoding());
    }
    else {
        OSL_FAIL("ODatabaseForm::submit_Impl : wrong encoding !");
    }

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
    m_aParameterManager.clearAllParameterInformation();
}

//==============================================================================
// OChangeListener
//------------------------------------------------------------------------------
void ODatabaseForm::_propertyChanged(const PropertyChangeEvent& evt) throw( RuntimeException )
{
    if (evt.PropertyName == PROPERTY_ACTIVE_CONNECTION && !m_bForwardingConnection)
    {
        // the rowset changed its active connection itself (without interaction from our side), so
        // we need to fire this event, too
        sal_Int32 nHandle = PROPERTY_ID_ACTIVE_CONNECTION;
        fire(&nHandle, &evt.NewValue, &evt.OldValue, 1, sal_False);
    }
    else    // it was one of the statement relevant props
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
    // SYNCHRONIZED ----->
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    Reference<XForm>  xParentForm(getParent(), UNO_QUERY);
    if (xParentForm.is())
    {
        try
        {
            Reference< XRowSetApproveBroadcaster > xParentApprBroadcast( xParentForm, UNO_QUERY_THROW );
            xParentApprBroadcast->removeRowSetApproveListener( this );

            Reference< XLoadable > xParentLoadable( xParentForm, UNO_QUERY_THROW );
            xParentLoadable->removeLoadListener( this );

            Reference< XPropertySet > xParentProperties( xParentForm, UNO_QUERY_THROW );
            xParentProperties->removePropertyChangeListener( PROPERTY_ISNEW, this );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    OFormComponents::setParent(Parent);

    xParentForm.set(getParent(), UNO_QUERY);
    if ( xParentForm.is() )
    {
        try
        {
            Reference< XRowSetApproveBroadcaster > xParentApprBroadcast( xParentForm, UNO_QUERY_THROW );
            xParentApprBroadcast->addRowSetApproveListener( this );

            Reference< XLoadable > xParentLoadable( xParentForm, UNO_QUERY_THROW );
            xParentLoadable->addLoadListener( this );

            Reference< XPropertySet > xParentProperties( xParentForm, UNO_QUERY_THROW );
            xParentProperties->addPropertyChangeListener( PROPERTY_ISNEW, this );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    Reference< XPropertySet > xAggregateProperties( m_xAggregateSet );
    aGuard.clear();
    // <----- SYNCHRONIZED

    Reference< XConnection > xOuterConnection;
    sal_Bool bIsEmbedded = ::dbtools::isEmbeddedInDatabase( Parent, xOuterConnection );

    if ( bIsEmbedded )
        xAggregateProperties->setPropertyValue( PROPERTY_DATASOURCE, makeAny( OUString() ) );
}

//==============================================================================
// smartXTabControllerModel
//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::getGroupControl() throw(com::sun::star::uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    // Should controls be combined into a TabOrder group?
    if (m_aCycle.hasValue())
    {
        sal_Int32 nCycle = 0;
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

    // Set TabIndex in the order of the sequence
    const Reference<XControlModel>* pControls = rControls.getConstArray();
    sal_Int32 nCount = getCount();
    sal_Int32 nNewCount = rControls.getLength();

    // HiddenControls and forms are not listed
    if (nNewCount <= nCount)
    {
        Any aElement;
        sal_Int16 nTabIndex = 1;
        for (sal_Int32 i=0; i < nNewCount; ++i, ++pControls)
        {
            Reference<XFormComponent>  xComp(*pControls, UNO_QUERY);
            if (xComp.is())
            {
                // Find component in the list
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
void SAL_CALL ODatabaseForm::setGroup( const Sequence<Reference<XControlModel> >& _rGroup, const OUString& Name ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // The controls are grouped by adjusting their names to the name of the
    // first control of the sequence
    const Reference<XControlModel>* pControls = _rGroup.getConstArray();
    Reference< XPropertySet > xSet;
    OUString sGroupName( Name );

    for( sal_Int32 i=0; i<_rGroup.getLength(); ++i, ++pControls )
    {
        xSet = xSet.query( *pControls );
        if ( !xSet.is() )
        {
            // can't throw an exception other than a RuntimeException (which would not be appropriate),
            // so we ignore (and only assert) this
            OSL_FAIL( "ODatabaseForm::setGroup: invalid arguments!" );
            continue;
        }

        if (sGroupName.isEmpty())
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
void SAL_CALL ODatabaseForm::getGroup( sal_Int32 nGroup, Sequence<Reference<XControlModel> >& _rGroup, OUString& _rName ) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    _rGroup.realloc(0);
    _rName = OUString();

    if ((nGroup < 0) || (nGroup >= m_pGroupManager->getGroupCount()))
        return;
    m_pGroupManager->getGroup( nGroup, _rGroup, _rName  );
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::getGroupByName(const OUString& Name, Sequence< Reference<XControlModel>  >& _rGroup) throw( RuntimeException )
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
    {   // no -> forward it
        com::sun::star::uno::Reference<com::sun::star::lang::XEventListener> xListener;
        if (query_aggregation(m_xAggregate, xListener))
            xListener->disposing(Source);
    }
}

//------------------------------------------------------------------------------
void ODatabaseForm::impl_createLoadTimer()
{
    OSL_PRECOND( m_pLoadTimer == NULL, "ODatabaseForm::impl_createLoadTimer: timer already exists!" );
    m_pLoadTimer = new Timer();
    m_pLoadTimer->SetTimeout(100);
    m_pLoadTimer->SetTimeoutHdl(LINK(this,ODatabaseForm,OnTimeout));
}

//==============================================================================
// com::sun::star::form::XLoadListener
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::loaded(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Reference< XRowSet > xParentRowSet( m_xParent, UNO_QUERY_THROW );
        xParentRowSet->addRowSetListener( this );

        impl_createLoadTimer();
    }

    load_impl( sal_True );
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::unloading(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    {
        // now stop the rowset listening if we are a subform
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_pLoadTimer && m_pLoadTimer->IsActive() )
            m_pLoadTimer->Stop();
        DELETEZ( m_pLoadTimer );

        Reference< XRowSet > xParentRowSet( m_xParent, UNO_QUERY_THROW );
        xParentRowSet->removeRowSetListener( this );
    }

    unload();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::unloaded(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    // nothing to do
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::reloading(const EventObject& /*aEvent*/) throw( RuntimeException )
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
void SAL_CALL ODatabaseForm::reloaded(const EventObject& /*aEvent*/) throw( RuntimeException )
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
IMPL_LINK_NOARG(ODatabaseForm, OnTimeout)
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
sal_Bool ODatabaseForm::canShareConnection( const Reference< XPropertySet >& _rxParentProps )
{
    // our own data source
    OUString sOwnDatasource;
    m_xAggregateSet->getPropertyValue( PROPERTY_DATASOURCE ) >>= sOwnDatasource;

    // our parents data source
    OUString sParentDataSource;
    OSL_ENSURE( _rxParentProps.is() && _rxParentProps->getPropertySetInfo().is() && _rxParentProps->getPropertySetInfo()->hasPropertyByName( PROPERTY_DATASOURCE ),
        "ODatabaseForm::doShareConnection: invalid parent form!" );
    if ( _rxParentProps.is() )
        _rxParentProps->getPropertyValue( PROPERTY_DATASOURCE ) >>= sParentDataSource;

    sal_Bool bCanShareConnection = sal_False;

    // both rowsets share are connected to the same data source
    if ( sParentDataSource == sOwnDatasource )
    {
        if ( !sParentDataSource.isEmpty() )
            // and it's really a data source name (not empty)
            bCanShareConnection = sal_True;
        else
        {   // the data source name is empty
            // -> ook for the URL
            OUString sParentURL;
            OUString sMyURL;
            _rxParentProps->getPropertyValue( PROPERTY_URL ) >>= sParentURL;
            m_xAggregateSet->getPropertyValue( PROPERTY_URL ) >>= sMyURL;

            bCanShareConnection = (sParentURL == sMyURL);
        }
    }

    if ( bCanShareConnection )
    {
        // check for the user/password

        // take the user property on the rowset (if any) into account
        OUString sParentUser, sParentPwd;
        _rxParentProps->getPropertyValue( PROPERTY_USER ) >>= sParentUser;
        _rxParentProps->getPropertyValue( PROPERTY_PASSWORD ) >>= sParentPwd;

        OUString sMyUser, sMyPwd;
        m_xAggregateSet->getPropertyValue( PROPERTY_USER ) >>= sMyUser;
        m_xAggregateSet->getPropertyValue( PROPERTY_PASSWORD ) >>= sMyPwd;

        bCanShareConnection =
                ( sParentUser == sMyUser )
            &&  ( sParentPwd == sMyPwd );
    }

    return bCanShareConnection;
}

//------------------------------------------------------------------------------
void ODatabaseForm::doShareConnection( const Reference< XPropertySet >& _rxParentProps )
{
    // get the conneciton of the parent
    Reference< XConnection > xParentConn;
    _rxParentProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xParentConn;
    OSL_ENSURE( xParentConn.is(), "ODatabaseForm::doShareConnection: we're a valid sub-form, but the parent has no connection?!" );

    if ( xParentConn.is() )
    {
        // add as dispose listener to the connection
        Reference< XComponent > xParentConnComp( xParentConn, UNO_QUERY );
        OSL_ENSURE( xParentConnComp.is(), "ODatabaseForm::doShareConnection: invalid connection!" );
        xParentConnComp->addEventListener( static_cast< XLoadListener* >( this ) );

        // forward the connection to our own aggreagte
        m_bForwardingConnection = sal_True;
        m_xAggregateSet->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( xParentConn ) );
        m_bForwardingConnection = sal_False;

        m_bSharingConnection = sal_True;
    }
    else
        m_bSharingConnection = sal_False;
}

//------------------------------------------------------------------------------
void ODatabaseForm::disposingSharedConnection( const Reference< XConnection >& /*_rxConn*/ )
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
    try
    {
        if ( getConnection( ).is() )
            // if our aggregate already has a connection, nothing needs to be done about it
            return sal_True;

        // see whether we're an embedded form
        Reference< XConnection > xOuterConnection;
        if ( ::dbtools::isEmbeddedInDatabase( getParent(), xOuterConnection ) )
        {
            m_xAggregateSet->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( xOuterConnection ) );
            return xOuterConnection.is();
        }

        m_bSharingConnection = sal_False;

        // if we're a sub form, we try to re-use the connection of our parent
        if (m_bSubForm)
        {
            OSL_ENSURE( Reference< XForm >( getParent(), UNO_QUERY ).is(),
                "ODatabaseForm::implEnsureConnection: m_bSubForm is TRUE, but the parent is no form?" );

            Reference< XPropertySet > xParentProps( getParent(), UNO_QUERY );

            // can we re-use (aka share) the connection of the parent?
            if ( canShareConnection( xParentProps ) )
            {
                // yep -> do it
                doShareConnection( xParentProps );
                // success?
                if ( m_bSharingConnection )
                    // yes -> outta here
                    return sal_True;
            }
        }

        if (m_xAggregateSet.is())
        {
            Reference< XConnection >  xConnection = connectRowset(
                Reference<XRowSet> (m_xAggregate, UNO_QUERY),
                m_xContext,
                sal_True    // set a calculated connection as ActiveConnection
            );
            return xConnection.is();
        }
    }
    catch(const SQLException& eDB)
    {
        onError(eDB, FRM_RES_STRING(RID_STR_CONNECTERROR));
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

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
    sal_Bool bConnected = implEnsureConnection();

    // we don't have to execute if we do not have a command to execute
    sal_Bool bExecute = bConnected && m_xAggregateSet.is() && !getString(m_xAggregateSet->getPropertyValue(PROPERTY_COMMAND)).isEmpty();

    // a database form always uses caching
    // we use starting fetchsize with at least 10 rows
    if (bConnected)
        m_xAggregateSet->setPropertyValue(PROPERTY_FETCHSIZE, makeAny((sal_Int32)40));

    // if we're loaded as sub form we got a "rowSetChanged" from the parent rowset _before_ we got the "loaded"
    // so we don't need to execute the statement again, this was already done
    // (and there were no relevant changes between these two listener calls, the "load" of a form is quite an
    // atomic operation.)

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
        m_aLoadListeners.notifyEach( &XLoadListener::loaded, aEvt );

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
    m_aLoadListeners.notifyEach( &XLoadListener::unloading, aEvt );

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
        catch(const SQLException&)
        {
        }
        aGuard.reset();
    }

    m_bLoaded = sal_False;

    // if the connection we used while we were loaded is only shared with our parent, we
    // reset it
    if ( isSharingConnection() )
        stopSharingConnection();

    aGuard.clear();
    m_aLoadListeners.notifyEach( &XLoadListener::unloaded, aEvt );
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

    DocumentModifyGuard aModifyGuard( *this );
        // ensures the document is not marked as "modified" just because we change some control's content during
        // reloading ...

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
    catch(const SQLException&)
    {
        OSL_FAIL("ODatabaseForm::reload_impl : shouldn't executeRowSet catch this exception?");
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
void SAL_CALL ODatabaseForm::cursorMoved(const EventObject& /*event*/) throw( RuntimeException )
{
    // reload the subform with the new parameters of the parent
    // do this handling delayed to provide of execute too many SQL Statements
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    DBG_ASSERT( m_pLoadTimer, "ODatabaseForm::cursorMoved: how can this happen?!" );
    if ( !m_pLoadTimer )
        impl_createLoadTimer();

    if ( m_pLoadTimer->IsActive() )
        m_pLoadTimer->Stop();

    // and start the timer again
    m_pLoadTimer->Start();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::rowChanged(const EventObject& /*event*/) throw( RuntimeException )
{
    // ignore it
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::rowSetChanged(const EventObject& /*event*/) throw( RuntimeException )
{
    // not interested in :
    // if our parent is an ODatabaseForm, too, then after this rowSetChanged we'll get a "reloaded"
    // or a "loaded" event.
    // If somebody gave us another parent which is an XRowSet but doesn't handle an execute as
    // "load" respectivly "reload" ... can't do anything ....
}

//------------------------------------------------------------------------------
bool ODatabaseForm::impl_approveRowChange_throw( const EventObject& _rEvent, const bool _bAllowSQLException,
    ::osl::ClearableMutexGuard& _rGuard )
{
    ::cppu::OInterfaceIteratorHelper aIter( m_aRowSetApproveListeners );
    _rGuard.clear();
    while ( aIter.hasMoreElements() )
    {
        Reference< XRowSetApproveListener > xListener( static_cast< XRowSetApproveListener* >( aIter.next() ) );
        if ( !xListener.is() )
            continue;

        try
        {
            if ( !xListener->approveRowSetChange( _rEvent ) )
                return false;
        }
        catch (const DisposedException& e)
        {
            if ( e.Context == xListener )
                aIter.remove();
        }
        catch (const RuntimeException&)
        {
            throw;
        }
        catch (const SQLException&)
        {
            if ( _bAllowSQLException )
                throw;
            DBG_UNHANDLED_EXCEPTION();
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return true;
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
        ::cppu::OInterfaceIteratorHelper aIter( m_aRowSetApproveListeners );
        while ( aIter.hasMoreElements() )
        {
            Reference< XRowSetApproveListener > xListener( static_cast< XRowSetApproveListener* >( aIter.next() ) );
            if ( !xListener.is() )
                continue;

            try
            {
                if ( !xListener->approveCursorMove( event ) )
                    return sal_False;
            }
            catch (const DisposedException& e)
            {
                if ( e.Context == xListener )
                    aIter.remove();
            }
            catch (const RuntimeException&)
            {
                throw;
            }
            catch (const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return true;
    }
    else
    {
        // this is a call from our parent ...
        // a parent's cursor move will result in a re-execute of our own row-set, so we have to
        // ask our own RowSetChangesListeners, too
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( !impl_approveRowChange_throw( event, false, aGuard ) )
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
        ::cppu::OInterfaceIteratorHelper aIter( m_aRowSetApproveListeners );
        while ( aIter.hasMoreElements() )
        {
            Reference< XRowSetApproveListener > xListener( static_cast< XRowSetApproveListener* >( aIter.next() ) );
            if ( !xListener.is() )
                continue;

            try
            {
                if ( !xListener->approveRowChange( event ) )
                    return false;
            }
            catch (const DisposedException& e)
            {
                if ( e.Context == xListener )
                    aIter.remove();
            }
            catch (const RuntimeException&)
            {
                throw;
            }
            catch (const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return true;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::approveRowSetChange(const EventObject& event) throw( RuntimeException )
{
    if (event.Source == InterfaceRef(static_cast<XWeak*>(this)))    // ignore our aggregate as we handle this approve ourself
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        bool bWasLoaded = isLoaded();
        if ( !impl_approveRowChange_throw( event, false, aGuard ) )
            return sal_False;

        if ( bWasLoaded )
        {
            m_aLoadListeners.notifyEach( &XLoadListener::reloading, event );
        }
    }
    else
    {
        // this is a call from our parent ...
        // a parent's cursor move will result in a re-execute of our own row-set, so we have to
        // ask our own RowSetChangesListeners, too
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( !impl_approveRowChange_throw( event, false, aGuard ) )
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
void SAL_CALL ODatabaseForm::addDatabaseParameterListener(const Reference<XDatabaseParameterListener>& _rListener) throw( RuntimeException )
{
    m_aParameterManager.addParameterListener( _rListener );
}
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeDatabaseParameterListener(const Reference<XDatabaseParameterListener>& _rListener) throw( RuntimeException )
{
    m_aParameterManager.removeParameterListener( _rListener );
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::addParameterListener(const Reference<XDatabaseParameterListener>& _rListener) throw( RuntimeException )
{
    ODatabaseForm::addDatabaseParameterListener( _rListener );
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::removeParameterListener(const Reference<XDatabaseParameterListener>& _rListener) throw( RuntimeException )
{
    ODatabaseForm::removeDatabaseParameterListener( _rListener );
}

//==============================================================================
// com::sun::star::sdb::XCompletedExecution
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::executeWithCompletion( const Reference< XInteractionHandler >& _rxHandler ) throw(SQLException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard(m_aMutex);
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
        if ( !impl_approveRowChange_throw( event, true, aGuard ) )
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
        if ( !impl_approveRowChange_throw( event, true, aGuard ) )
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
    catch(const RowSetVetoException&)
    {
        throw;
    }
    catch(const SQLException& eDb)
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
    catch(const RowSetVetoException&)
    {
        throw;
    }
    catch(const SQLException& eDb)
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
    catch(const RowSetVetoException&)
    {
        throw;
    }
    catch(const SQLException& eDb)
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
    catch(const RowSetVetoException&)
    {
        throw;
    }
    catch(const SQLException& eDb)
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
        //
        // Still, there is #72756#. During fixing this bug, DG introduced not calling the aggregate here. So
        // in theory, we re-introduced #72756#. But the bug described therein does not happen anymore, as the
        // preliminaries for it changed (no display of guessed values for new records with autoinc fields)
        //
        // BTW: the public Issuezilla bug is #i2815#
        //
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
    catch(const RowSetVetoException&)
    {
        throw;
    }
    catch(const SQLException& eDb)
    {
        onError(eDb, FRM_RES_STRING(RID_STR_ERR_DELETERECORDS));
        throw;
    }

    return Sequence< sal_Int32 >();
}

// com::sun::star::sdbc::XParameters
//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setNull(parameterIndex, sqlType);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setObjectNull(parameterIndex, sqlType, typeName);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setBoolean(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setByte(sal_Int32 parameterIndex, sal_Int8 x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setByte(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setShort(sal_Int32 parameterIndex, sal_Int16 x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setShort(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setInt(sal_Int32 parameterIndex, sal_Int32 x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setInt(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setLong(sal_Int32 parameterIndex, sal_Int64 x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setLong(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setFloat(sal_Int32 parameterIndex, float x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setFloat(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setDouble(sal_Int32 parameterIndex, double x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setDouble(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setString(sal_Int32 parameterIndex, const OUString& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setString(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBytes(sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setBytes(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setDate(sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setDate(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setTime(sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setTime(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setTimestamp(sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setTimestamp(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBinaryStream(sal_Int32 parameterIndex, const Reference<XInputStream>& x, sal_Int32 length) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setBinaryStream(parameterIndex, x, length);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setCharacterStream(sal_Int32 parameterIndex, const Reference<XInputStream>& x, sal_Int32 length) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setCharacterStream(parameterIndex, x, length);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObjectWithInfo(sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setObjectWithInfo(parameterIndex, x, targetSqlType, scale);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setObject(sal_Int32 parameterIndex, const Any& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setObject(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setRef(sal_Int32 parameterIndex, const Reference<XRef>& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setRef(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setBlob(sal_Int32 parameterIndex, const Reference<XBlob>& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setBlob(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setClob(sal_Int32 parameterIndex, const Reference<XClob>& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setClob(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setArray(sal_Int32 parameterIndex, const Reference<XArray>& x) throw( SQLException, RuntimeException )
{
    m_aParameterManager.setArray(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::clearParameters() throw( SQLException, RuntimeException )
{
    m_aParameterManager.clearParameters();
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::propertyChange( const PropertyChangeEvent& evt ) throw (RuntimeException)
{
    if ( evt.Source == m_xParent )
    {
        if ( evt.PropertyName == PROPERTY_ISNEW )
        {
            sal_Bool bCurrentIsNew( sal_False );
            OSL_VERIFY( evt.NewValue >>= bCurrentIsNew );
            if ( !bCurrentIsNew )
                reload_impl( sal_True );
        }
        return;
    }
    OFormComponents::propertyChange( evt );
}

// com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
OUString SAL_CALL ODatabaseForm::getImplementationName_Static()
{
    return OUString( "com.sun.star.comp.forms.ODatabaseForm" );
}

//------------------------------------------------------------------------------
Sequence< OUString > SAL_CALL ODatabaseForm::getCompatibleServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    OUString* pServices = aServices.getArray();

    *pServices++ = FRM_COMPONENT_FORM;

    return aServices;
}

//------------------------------------------------------------------------------
Sequence< OUString > SAL_CALL ODatabaseForm::getCurrentServiceNames_Static()
{
    Sequence< OUString > aServices( 5 );
    OUString* pServices = aServices.getArray();

    *pServices++ = FRM_SUN_FORMCOMPONENT;
    *pServices++ = OUString("com.sun.star.form.FormComponents");
    *pServices++ = FRM_SUN_COMPONENT_FORM;
    *pServices++ = FRM_SUN_COMPONENT_HTMLFORM;
    *pServices++ = FRM_SUN_COMPONENT_DATAFORM;

    return aServices;
}

//------------------------------------------------------------------------------
Sequence< OUString > SAL_CALL ODatabaseForm::getSupportedServiceNames_Static()
{
    return ::comphelper::concatSequences(
        getCurrentServiceNames_Static(),
        getCompatibleServiceNames_Static()
    );
}

//------------------------------------------------------------------------------
OUString SAL_CALL ODatabaseForm::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
Sequence< OUString > SAL_CALL ODatabaseForm::getSupportedServiceNames() throw( RuntimeException )
{
    // the services of our aggregate
    Sequence< OUString > aServices;
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
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseForm::supportsService(const OUString& ServiceName) throw( RuntimeException )
{
    Sequence< OUString > aSupported( getSupportedServiceNames() );
    const OUString* pArray = aSupported.getConstArray();
    for( sal_Int32 i = 0; i < aSupported.getLength(); ++i, ++pArray )
        if( pArray->equals( ServiceName ) )
            return sal_True;
    return sal_False;
}

//==============================================================================
// com::sun::star::io::XPersistObject
//------------------------------------------------------------------------------

const sal_uInt16 CYCLE              = 0x0001;
const sal_uInt16 DONTAPPLYFILTER    = 0x0002;

//------------------------------------------------------------------------------
OUString ODatabaseForm::getServiceName() throw( RuntimeException )
{
    return OUString(FRM_COMPONENT_FORM);  // old (non-sun) name for compatibility !
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

    OUString sDataSource;
    if (m_xAggregateSet.is())
        m_xAggregateSet->getPropertyValue(PROPERTY_DATASOURCE) >>= sDataSource;
    _rxOutStream << sDataSource;

    // former CursorSource
    OUString sCommand;
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
        sal_Int32 nCommandType = 0;
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
            default : OSL_FAIL("ODatabaseForm::write : wrong CommandType !");
        }
    }
    _rxOutStream->writeShort((sal_Int16)eTranslated);           // former DataSelectionType

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
    OUString sTmp = INetURLObject::decode( m_aTargetURL, '%', INetURLObject::DECODE_UNAMBIGUOUS);
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

    OUString sFilter;
    OUString sOrder;
    if (m_xAggregateSet.is())
    {
        m_xAggregateSet->getPropertyValue(PROPERTY_FILTER) >>= sFilter;
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
        sal_Int32 nRealCycle = 0;
        ::cppu::enum2int(nRealCycle, m_aCycle);
        _rxOutStream->writeShort((sal_Int16)nRealCycle);
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

    OUString sAggregateProp;
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
        default : OSL_FAIL("ODatabaseForm::read : wrong CommandType !");
    }
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_COMMANDTYPE, makeAny(nCommandType));

    // obsolete
    _rxInStream->readShort();

    // navigation mode was a boolean in version 1
    // was a sal_Bool in version 1
    sal_Bool bNavigation = _rxInStream->readBoolean();
    if (nVersion == 1)
        m_eNavigation = bNavigation ? NavigationBarMode_CURRENT : NavigationBarMode_NONE;

    sal_Bool bInsertOnly = _rxInStream->readBoolean();
    if (m_xAggregateSet.is())
        m_xAggregateSet->setPropertyValue(PROPERTY_INSERTONLY, makeAny(bInsertOnly));

    m_bAllowInsert      = _rxInStream->readBoolean();
    m_bAllowUpdate      = _rxInStream->readBoolean();
    m_bAllowDelete      = _rxInStream->readBoolean();

    // html stuff
    OUString sTmp;
    _rxInStream >> sTmp;
    m_aTargetURL = INetURLObject::decode( sTmp, '%', INetURLObject::DECODE_UNAMBIGUOUS);
    m_eSubmitMethod     = (FormSubmitMethod)_rxInStream->readShort();
    m_eSubmitEncoding       = (FormSubmitEncoding)_rxInStream->readShort();
    _rxInStream >> m_aTargetFrame;

    if (nVersion > 1)
    {
        sal_Int32 nCycle = _rxInStream->readShort();
        m_aCycle = ::cppu::int2enum(nCycle, ::getCppuType(static_cast<const TabulatorCycle*>(NULL)));
        m_eNavigation = (NavigationBarMode)_rxInStream->readShort();

        _rxInStream >> sAggregateProp;
        setPropertyValue(PROPERTY_FILTER, makeAny(sAggregateProp));

        _rxInStream >> sAggregateProp;
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_SORT, makeAny(sAggregateProp));
    }

    sal_uInt16 nAnyMask = 0;
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

    Reference< XSQLErrorBroadcaster >   xBroadcaster( _pElement->xInterface, UNO_QUERY );
    Reference< XForm >                  xForm       ( _pElement->xInterface, UNO_QUERY );

    if ( xBroadcaster.is() && !xForm.is() )
    {   // the object is an error broadcaster, but no form itself -> add ourself as listener
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
    {   // the object is an error broadcaster, but no form itself -> remove ourself as listener
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
OUString SAL_CALL ODatabaseForm::getName() throw( RuntimeException )
{
    OUString sReturn;
    OPropertySetHelper::getFastPropertyValue(PROPERTY_ID_NAME) >>= sReturn;
    return sReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseForm::setName(const OUString& aName) throw( RuntimeException )
{
    setFastPropertyValue(PROPERTY_ID_NAME, makeAny(aName));
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
