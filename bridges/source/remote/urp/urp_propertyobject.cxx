/*************************************************************************
 *
 *  $RCSfile: urp_propertyobject.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-20 16:44:05 $
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
#include <stdlib.h>

#include <osl/diagnose.h>

#include <rtl/random.h>

#include <uno/data.h>

#include <com/sun/star/bridge/XProtocolProperties.hpp>

#include "urp_propertyobject.hxx"
#include "urp_dispatch.hxx"
#include "urp_bridgeimpl.hxx"
#include "urp_job.hxx"

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::uno;

namespace bridges_urp
{
// some nice constants ....
static const sal_Int32 METHOD_QUERY_INTERFACE = 0;
static const sal_Int32 METHOD_GET_PROPERTIES = 3;
static const sal_Int32 METHOD_REQUEST_CHANGE = 4;
static const sal_Int32 METHOD_COMMIT_CHANGE = 5;

static const sal_Int32 PROPERTY_BRIDGEID = 0;
static const sal_Int32 PROPERTY_TYPECACHESIZE = 1;
static const sal_Int32 PROPERTY_OIDCACHESIZE = 2;
static const sal_Int32 PROPERTY_TIDCACHESIZE = 3;
static const sal_Int32 PROPERTY_SUPPORTEDVERSIONS = 4;
static const sal_Int32 PROPERTY_VERSION =5;
static const sal_Int32 PROPERTY_FLUSHBLOCKSIZE = 6;
static const sal_Int32 PROPERTY_ONEWAYTIMEOUT_MUSEC = 7;
static const sal_Int32 PROPERTY_SUPPORTSMUSTREPLY = 8;
static const sal_Int32 PROPERTY_SUPPERTSSYNCHRONOUS = 9;
static const sal_Int32 PROPERTY_SUPPORTSMULTIPLESYNCHRONOUS = 10;
static const sal_Int32 PROPERTY_CLEARCACHE = 11;

static const sal_Int32 MAX_PROPERTIES = PROPERTY_CLEARCACHE +1;

const sal_Char *g_aPropertyName[] =
{
    "BridgeId",
    "TypeCacheSize",
    "OidCacheSize",
    "TidCacheSize",
    "SupportedVersion",
    "Version",
    "FlushBlockSize",
    "OnewayTimeoutMUSEC",
    "SupportsMustReply",
    "SupportsSynchronous",
    "SupportsMultipleSynchronous",
    "ClearCache"
};

// nice little helper functions for conversion
template< class t >
void assignToIdl( ProtocolProperty *pIdl, sal_Int32 nIndex, const t &value )
{
    pIdl->Name = OUString::createFromAscii( g_aPropertyName[nIndex] );
    ( *(::com::sun::star::uno::Any *)&(pIdl->Value) ) <<= value;
}

template< class t >
void assignFromIdl( t * p, const ProtocolProperty & property )
{
    property.Value >>=*p;
}

static sal_Int32 getIndexFromIdl( const ProtocolProperty & property )
{
    sal_Int32 nResult = -1;
    for( sal_Int32 i = 0 ; i < MAX_PROPERTIES ; i ++ )
    {
        if( 0 == property.Name.compareToAscii( g_aPropertyName[i] ) )
        {
            nResult = i;
            break;
        }
    }
    return nResult;
}

static sal_Int32 getIndexFromString( const OUString & property )
{
    sal_Int32 nResult = -1;
    for( sal_Int32 i = 0 ; i < MAX_PROPERTIES ; i ++ )
    {
        if( 0 == property.compareToAscii( g_aPropertyName[i] ) )
        {
            nResult = i;
            break;
        }
    }
    return nResult;
}

static sal_Bool assignFromIdlToStruct( Properties *pProps, const ProtocolProperty &idl )
{
    sal_Bool bReturn = sal_True;

    sal_Int32 nIndex = getIndexFromIdl( idl );
    if( nIndex >= 0 )
    {
        switch( nIndex )
        {
        case PROPERTY_SUPPORTSMULTIPLESYNCHRONOUS:
            assignFromIdl( &(pProps->bSupportsMultipleSynchronous) , idl );
            break;
        case PROPERTY_SUPPERTSSYNCHRONOUS:
            assignFromIdl( &(pProps->bSupportsMustReply) , idl );
            break;
        case PROPERTY_SUPPORTSMUSTREPLY:
            assignFromIdl( &(pProps->bSupportsSynchronous) , idl );
            break;
        case PROPERTY_ONEWAYTIMEOUT_MUSEC:
            assignFromIdl( &(pProps->nOnewayTimeoutMUSEC) , idl );
            break;
        case PROPERTY_BRIDGEID:
            assignFromIdl( (Sequence< sal_Int8 > * )&(pProps->seqBridgeID), idl );
            break;
        case PROPERTY_TYPECACHESIZE:
            assignFromIdl( &(pProps->nTypeCacheSize) , idl );
            break;
        case PROPERTY_OIDCACHESIZE:
            assignFromIdl( &(pProps->nOidCacheSize) , idl );
                break;
        case PROPERTY_TIDCACHESIZE:
            assignFromIdl( &(pProps->nTidCacheSize), idl );
            break;
        case PROPERTY_SUPPORTEDVERSIONS:
            assignFromIdl( &(pProps->sSupportedVersions) , idl );
            break;
        case PROPERTY_VERSION:
            assignFromIdl( &(pProps->sVersion) , idl );
            break;
        case PROPERTY_FLUSHBLOCKSIZE:
            assignFromIdl( &(pProps->nFlushBlockSize) ,idl );
            break;
        case PROPERTY_CLEARCACHE:
            assignFromIdl( &(pProps->bClearCache) ,idl );
            break;
        default:
            bReturn = sal_False;
        }
    }
    else
    {
        bReturn = sal_False;
    }
    return bReturn;
}


//----------------------------------------------------------------------------------------------
// PropertyObject implementation
PropertyObject::PropertyObject(
    struct Properties *pLocalSetting , uno_Environment *pEnvRemote, urp_BridgeImpl *pImpl )
    : m_pLocalSetting( pLocalSetting )
    , m_nRefCount( 0 )
    , m_pEnvRemote( pEnvRemote )
    , m_bApplyProperties( sal_False )
    , m_pBridgeImpl( pImpl )
    , m_bClientWaitingForCommit( sal_False )
    , m_bServerWaitingForCommit( sal_False )
    , m_bRequestChangeHasBeenCalled( sal_False )
    , m_commitChangeCondition( osl_createCondition() )
{
    acquire = staticAcquire;
    release = staticRelease;
    pDispatcher = staticDispatch;
}

PropertyObject::~PropertyObject()
{
    osl_destroyCondition( m_commitChangeCondition );
}

void SAL_CALL PropertyObject::thisDispatch(
    typelib_TypeDescription * pMemberType, void * pReturn, void * ppArgs[],uno_Any ** ppException )
{
    OSL_ASSERT( pMemberType->eTypeClass == typelib_TypeClass_INTERFACE_METHOD );

    typelib_InterfaceMethodTypeDescription *pMethodType =
        ( typelib_InterfaceMethodTypeDescription * ) pMemberType;

    switch( pMethodType->aBase.nPosition )
    {
    case METHOD_QUERY_INTERFACE:
        OSL_ENSURE( 0 , "not implemented yet !" );
        break;
    case METHOD_GET_PROPERTIES:
    {
          implGetProperties( (uno_Sequence **) pReturn );
        *ppException = 0;
        break;
    }
    case METHOD_COMMIT_CHANGE:
    {
        implCommitChange( *(uno_Sequence ** ) ppArgs[0] , ppException );
        break;
    }
    case METHOD_REQUEST_CHANGE:
    {
        *(sal_Int32 *) pReturn = implRequestChange( *(sal_Int32 *)ppArgs[0], ppException );
        break;
    }
    default:
        OSL_ENSURE( 0 , "unkown method !" );
    }
}

void SAL_CALL PropertyObject::localGetPropertiesFromRemote( struct Properties *pR )
{
    OUString oid = OUString::createFromAscii( g_NameOfUrpProtocolPropertiesObject );

    typelib_TypeDescription *pInterfaceType = 0;
    getCppuType( (Reference< XProtocolProperties > *) 0 ).getDescription( &pInterfaceType );

    if( !pInterfaceType->bComplete )
    {
        typelib_typedescription_complete( &pInterfaceType );
    }

    typelib_TypeDescription *pMethodType = 0;
    typelib_typedescriptionreference_getDescription(
        &pMethodType,
        ((typelib_InterfaceTypeDescription*) pInterfaceType)->ppAllMembers[METHOD_GET_PROPERTIES] );


    uno_Sequence *pResult = 0;
    uno_Any exception;
    uno_Any *pException = &exception;
    urp_sendRequest( m_pEnvRemote,
                     pMethodType,
                     oid.pData,
                     (typelib_InterfaceTypeDescription*) pInterfaceType,
                     &pResult,
                     0,
                     &pException );

    if( pException )
    {
        OSL_ENSURE( 0 , "remote urp-bridge doesn't support property-object" );
        uno_any_destruct( pException , 0 );
        return;
    }

    ProtocolProperty *pP = (ProtocolProperty * ) pResult->elements;
    for( sal_Int32 i = 0; i < pResult->nElements ; i ++ )
    {
        if( ! assignFromIdlToStruct( pR , pP[i] ) )
        {
            OSL_ENSURE( 0 , "unknown property !!!!" );
        }
    }

    typelib_typedescription_release( pInterfaceType );
    typelib_typedescription_release( pMethodType );
}


// implementation for call from remote
void SAL_CALL PropertyObject::implGetProperties( uno_Sequence **ppReturnValue )
{
    typelib_TypeDescription *pElementType= 0;
    getCppuType( (Sequence< ProtocolProperty > *)0).getDescription( &pElementType );

    OSL_ENSURE( pElementType , "Couldn't get property type" );

    *ppReturnValue = 0;
    uno_sequence_construct( ppReturnValue , pElementType , 0, MAX_PROPERTIES , 0 );
    ProtocolProperty *pElements = (ProtocolProperty * ) ( *ppReturnValue )->elements;
    Properties *pP = m_pLocalSetting;

    assignToIdl( &(pElements[PROPERTY_BRIDGEID]),PROPERTY_BRIDGEID, *(Sequence< sal_Int8 > *)&(pP->seqBridgeID) );
    assignToIdl( &(pElements[PROPERTY_TYPECACHESIZE]),PROPERTY_TYPECACHESIZE,pP->nTypeCacheSize );
    assignToIdl( &(pElements[PROPERTY_OIDCACHESIZE]),PROPERTY_OIDCACHESIZE, pP->nOidCacheSize );
    assignToIdl( &(pElements[PROPERTY_TIDCACHESIZE]),PROPERTY_TIDCACHESIZE, pP->nTidCacheSize );
    assignToIdl( &(pElements[PROPERTY_SUPPORTEDVERSIONS]),PROPERTY_SUPPORTEDVERSIONS, pP->sSupportedVersions );
    assignToIdl( &(pElements[PROPERTY_VERSION]),PROPERTY_VERSION, pP->sVersion );
    assignToIdl( &(pElements[PROPERTY_FLUSHBLOCKSIZE]), PROPERTY_FLUSHBLOCKSIZE,pP->nFlushBlockSize );
    assignToIdl( &(pElements[PROPERTY_ONEWAYTIMEOUT_MUSEC]), PROPERTY_ONEWAYTIMEOUT_MUSEC, pP->nOnewayTimeoutMUSEC );
    assignToIdl( &(pElements[PROPERTY_SUPPORTSMUSTREPLY]), PROPERTY_SUPPORTSMUSTREPLY, pP->bSupportsMustReply );
    assignToIdl( &(pElements[PROPERTY_SUPPERTSSYNCHRONOUS]), PROPERTY_SUPPERTSSYNCHRONOUS, pP->bSupportsSynchronous );
    assignToIdl( &(pElements[PROPERTY_SUPPORTSMULTIPLESYNCHRONOUS]), PROPERTY_SUPPORTSMULTIPLESYNCHRONOUS, pP->bSupportsMultipleSynchronous );
    assignToIdl( &(pElements[PROPERTY_CLEARCACHE]), PROPERTY_CLEARCACHE, pP->bClearCache );

    typelib_typedescription_release( pElementType );
}

//----------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL PropertyObject::localRequestChange( )
{
    sal_Int32 nResult = -1; // try again is default
    sal_Bool bCall = sal_True;

    // disallow marshaling NOW !
    ClearableMutexGuard marshalingGuard( m_pBridgeImpl->m_marshalingMutex );
    {
        MutexGuard guard( m_mutex );
        if( m_bRequestChangeHasBeenCalled || m_bClientWaitingForCommit || m_bServerWaitingForCommit )
        {
            // another transaction is already underway
            // try again later !
            bCall = sal_False;
        }
        m_bRequestChangeHasBeenCalled = sal_True;

        if( bCall )
        {
            rtlRandomPool pool = rtl_random_createPool ();
            rtl_random_getBytes( pool , &m_nRandomNumberOfRequest, sizeof( m_nRandomNumberOfRequest ) );
            rtl_random_destroyPool( pool );
        }
    }

    if( bCall )
    {
        OUString oid = OUString::createFromAscii( g_NameOfUrpProtocolPropertiesObject );

        // gather types for calling
        typelib_TypeDescription *pInterfaceType = 0;
        getCppuType( (Reference< XProtocolProperties > *) 0 ).getDescription( &pInterfaceType );

        if( !pInterfaceType->bComplete )
        {
            typelib_typedescription_complete( &pInterfaceType );
        }

        typelib_TypeDescription *pMethodType = 0;
        typelib_typedescriptionreference_getDescription(
            &pMethodType,
            ((typelib_InterfaceTypeDescription*) pInterfaceType)->ppAllMembers[METHOD_REQUEST_CHANGE] );

        void *pArg1 = &m_nRandomNumberOfRequest;
        void **ppArgs = &pArg1;

        uno_Sequence *pResult = 0;
        uno_Any exception;
        uno_Any *pException = &exception;

        ClientJob job( m_pEnvRemote,
                       m_pBridgeImpl,
                       oid.pData,
                       pMethodType,
                       (typelib_InterfaceTypeDescription*) pInterfaceType,
                       &nResult,
                       ppArgs,
                       &pException );

        // put the call on the line !
        sal_Bool bSuccess = job.pack();

        // now allow writing on wire again.
        // NOTE : this has been locked, because it is inevitable to set m_bRequestChangeHasBeenCalled
        //        and call requestChange in an atomar operation. Otherwise, implRequestChange may be called
        //        inbetween and reply, before the request is put on the wire. This certainly would
        //        be confusing for the remote counterpart !
        marshalingGuard.clear();

        // wait for the reply ...
        if( bSuccess )
        {
            job.wait();

            if( pException )
            {
                // the object is unknown on the other side.
                uno_any_destruct( pException , 0 );
                nResult = 0;
            }
        }
        else
        {
            nResult = 0;
        }
        typelib_typedescription_release( pInterfaceType );
        typelib_typedescription_release( pMethodType );
    }

    {
        MutexGuard guard( m_mutex );
        m_bRequestChangeHasBeenCalled = sal_False;
        m_bClientWaitingForCommit = ( 1 == nResult );
        m_bServerWaitingForCommit = ( 0 == nResult );
    }
    return nResult;
}

// implementation for call from remote
sal_Int32 SAL_CALL PropertyObject::implRequestChange( sal_Int32 nRandomNumber, uno_Any **ppException )
{
    sal_Int32 nResult = 0;
    MutexGuard guard( m_mutex );
    if( m_bRequestChangeHasBeenCalled )
    {
        // this side has also called requestChange, now negotiate, which side is allowed
        // to commit the change !
        if( m_nRandomNumberOfRequest > nRandomNumber )
        {
            // this side may commit !!!!
            nResult = 0;
        }
        else if( m_nRandomNumberOfRequest == nRandomNumber )
        {
            // sorry, try again !
            nResult = -1;
        }
        else if( m_nRandomNumberOfRequest < nRandomNumber )
        {
            // the other side may commit !
            nResult = 1;
            // m_bServerWaitingForCommit will be set by localRequestChange
        }
    }
    else
    {
        // This side has NOT called requestChange, so the other side may commit
        nResult = 1;
        m_bServerWaitingForCommit = sal_True;
    }

    *ppException = 0;
    return nResult;
}

static void extractTokens(
    const ::rtl::OUString &sProps , ::std::list< OUString > &lst )
{
    sal_Int32 nNext = 0;
    while ( sal_True )
    {
        sal_Int32 nStart = nNext;
        nNext = sProps.indexOf( ',' , nNext );
        if( -1 == nNext )
        {
            lst.push_back( sProps.copy( nStart, sProps.getLength() - nStart ) );
            break;
        }
        lst.push_back( sProps.copy( nStart , nNext - nStart ) );
        nNext ++;
    }
}


void SAL_CALL PropertyObject::localCommitChange( const ::rtl::OUString &sProps , sal_Bool *pbExceptionThrown )
{
    // lock the bridge NOW !
    // NOTE: it is not allowed for other threads to call, when a commit change is underway.
    //       The remote counterpart cannot if the call following the commit already uses
    //       the new properties or not.
    MutexGuard guard( m_pBridgeImpl->m_marshalingMutex );

    OUString oid = OUString::createFromAscii( g_NameOfUrpProtocolPropertiesObject );

    osl_resetCondition( m_commitChangeCondition );
    m_bClientWaitingForCommit = sal_True;

    Properties props = *m_pLocalSetting;

    typelib_TypeDescription *pInterfaceType = 0;
    getCppuType( (Reference< XProtocolProperties > *) 0 ).getDescription( &pInterfaceType );

    if( !pInterfaceType->bComplete )
    {
        typelib_typedescription_complete( &pInterfaceType );
    }

    typelib_TypeDescription *pMethodType = 0;
    typelib_typedescriptionreference_getDescription(
        &pMethodType,
        ((typelib_InterfaceTypeDescription*) pInterfaceType)->ppAllMembers[METHOD_COMMIT_CHANGE] );

    typelib_TypeDescription *pSequenceType= 0;

    // extract name/value pairs
    ::std::list< OUString > lst;
    extractTokens( sProps , lst );

    getCppuType( (Sequence< ProtocolProperty > *)0).getDescription( &pSequenceType );
    uno_Sequence *pSeq = 0;
    uno_sequence_construct( &pSeq , pSequenceType , 0, lst.size() , 0 );
    ProtocolProperty *pElements = (ProtocolProperty * ) pSeq->elements;

    sal_Int32 i = 0;
    for( ::std::list< OUString >::iterator ii = lst.begin() ; ii != lst.end() ; ++ ii, i++ )
    {
        sal_Int32 nAssign = (*ii).indexOf( '=' );
        if( -1 == nAssign )
        {
            OString o = OUStringToOString( *ii, RTL_TEXTENCODING_ASCII_US );
            OSL_ENSURE( !"wrong protocol propertyt format, ignored", o.getStr() );
        }
        OUString sPropName = (*ii).copy( 0, nAssign );
        OUString sValue    = (*ii).copy( nAssign +1, (*ii).getLength() - nAssign -1 );

        sal_Int32 nIndex = getIndexFromString( sPropName );
        if( -1 == nIndex )
        {
            OString o = OUStringToOString( sPropName , RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE( !"unknown protocol property, ignored", o.getStr() );
        }
        switch( nIndex )
        {
            // bools
        case PROPERTY_CLEARCACHE:
        {
            sal_Bool bClearCache = (sal_Bool ) sValue.toInt32();
            assignToIdl( &(pElements[i]) , nIndex , bClearCache );
            break;
        }
            // ints
        case PROPERTY_TYPECACHESIZE:
        case PROPERTY_OIDCACHESIZE:
        case PROPERTY_TIDCACHESIZE:
        case PROPERTY_FLUSHBLOCKSIZE:
        case PROPERTY_ONEWAYTIMEOUT_MUSEC:
        {
            sal_Int32 nValue = sValue.toInt32();
            assignToIdl( &(pElements[i]) , nIndex , nValue );
            break;
        }

            // strings
        case PROPERTY_VERSION:
            assignToIdl( &(pElements[i]) , nIndex , sValue );
            break;
        default:
            OString o = OUStringToOString( sPropName, RTL_TEXTENCODING_ASCII_US );
            OSL_ENSURE( !"readonly protocol property, ignored" , o.getStr() );
        }
        assignFromIdlToStruct( &props, pElements[i] );
    }

    void *pArg1 = &pSeq;
    uno_Sequence *pResult = 0;
    uno_Any exception;
    uno_Any *pException = &exception;

    ClientJob job( m_pEnvRemote,
                   m_pBridgeImpl,
                   oid.pData,
                   pMethodType,
                   (typelib_InterfaceTypeDescription*) pInterfaceType,
                   0,
                   &pArg1,
                   &pException );
    job.setBridgePropertyCall();
    job.pack();
    job.wait();

    ::uno_destructData( &pSeq, pSequenceType, 0 );

    *pbExceptionThrown = pException ? sal_True : sal_False;

    if( pException )
    {
        OString o = OUStringToOString( ((com::sun::star::uno::Exception*)pException->pData)->Message,
                                       RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE( !"exception thrown during calling on PropertyObject",o.getStr() );
        uno_any_destruct( pException , 0 );
    }
    else
    {
        m_pBridgeImpl->applyProtocolChanges( props );
        m_bClientWaitingForCommit = sal_False;
        m_bServerWaitingForCommit = sal_False;
        m_bApplyProperties = sal_False;
    }

    // let the reader thread go ...
    m_bClientWaitingForCommit = sal_False;
    osl_setCondition( m_commitChangeCondition );

    typelib_typedescription_release( pSequenceType );
    typelib_typedescription_release( pMethodType );
    typelib_typedescription_release( pInterfaceType );
}

void SAL_CALL PropertyObject::implCommitChange( uno_Sequence *pSequence, uno_Any **ppException )
{
    MutexGuard guard( m_mutex );
    m_propsToBeApplied = *m_pLocalSetting;

    ProtocolProperty *pP = (ProtocolProperty * ) pSequence->elements;
    for( sal_Int32 i = 0; i < pSequence->nElements ; i ++ )
    {
        if( ! assignFromIdlToStruct( &m_propsToBeApplied , pP[i] ) )
        {
            InvalidProtocolChangeException exception;
            Type type = getCppuType( &exception );
            exception.Message = OUString::createFromAscii( "urp: unknown Property " );
            exception.Message += pP[i].Name;
            exception.invalidProperty = pP[i];
            exception.reason = 1;

            uno_type_any_construct( *ppException, &exception, type.getTypeLibType() , 0 );

            m_bApplyProperties = sal_False;
            m_bClientWaitingForCommit = sal_False;
            m_bServerWaitingForCommit = sal_False;
            return;
        }
    }

    m_bApplyProperties = sal_True;
    *ppException = 0;
}

Properties SAL_CALL PropertyObject::getCommitedChanges()
{
    MutexGuard guard( m_mutex );
    OSL_ASSERT( m_bApplyProperties );
    m_bApplyProperties = sal_False;
    m_bClientWaitingForCommit = sal_False;
    m_bServerWaitingForCommit = sal_False;
    return m_propsToBeApplied;
}

void SAL_CALL PropertyObject::waitUntilChangesAreCommitted()
{
    osl_waitCondition( m_commitChangeCondition , 0 );
}
}
