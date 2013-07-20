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


#include <list>

#include <vcl/svapp.hxx>
#include <tools/stream.hxx>
#include <svl/brdcst.hxx>
#include <tools/shl.hxx>
#include <basic/sbx.hxx>
#include "sbdiagnose.hxx"
#include "sb.hxx"
#include <sbjsmeth.hxx>
#include "sbjsmod.hxx"
#include "sbintern.hxx"
#include "image.hxx"
#include "opcodes.hxx"
#include "runtime.hxx"
#include "token.hxx"
#include "sbunoobj.hxx"

#include "sal/log.hxx"

#include <basic/basrdll.hxx>
#include <osl/mutex.hxx>
#include <basic/sbobjmod.hxx>
#include <basic/vbahelper.hxx>
#include <cppuhelper/implbase3.hxx>
#include <unotools/eventcfg.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/VBAScriptEventId.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XEventListener.hpp>

using namespace com::sun::star;

#ifdef UNX
#include <sys/resource.h>
#endif

#include <stdio.h>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <map>
#include <com/sun/star/reflection/ProxyFactory.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/script/XInvocation.hpp>

using namespace ::com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::uno;

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/awt/DialogProvider.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <comphelper/anytostring.hxx>
#include <ooo/vba/VbQueryClose.hpp>
#include "sbcomp.hxx"

typedef ::cppu::WeakImplHelper1< XInvocation > DocObjectWrapper_BASE;
typedef ::std::map< sal_Int16, Any, ::std::less< sal_Int16 > > OutParamMap;
::com::sun::star::uno::Any sbxToUnoValue( SbxVariable* pVar );
void unoToSbxValue( SbxVariable* pVar, const ::com::sun::star::uno::Any& aValue );

/*const OUString CodeCompleteDataCache::GLOB_KEY = OUString("global key");
const OUString CodeCompleteDataCache::NOT_FOUND = OUString("not found");*/

class DocObjectWrapper : public DocObjectWrapper_BASE
{
    Reference< XAggregation >  m_xAggProxy;
    Reference< XInvocation >  m_xAggInv;
    Reference< XTypeProvider > m_xAggregateTypeProv;
    Sequence< Type >           m_Types;
    SbModule*                m_pMod;
    SbMethodRef getMethod( const OUString& aName ) throw (RuntimeException);
    SbPropertyRef getProperty( const OUString& aName ) throw (RuntimeException);
    OUString mName; // for debugging

public:
    DocObjectWrapper( SbModule* pMod );
    virtual ~DocObjectWrapper();

    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (RuntimeException)
    {
        if( !m_xAggregateTypeProv.is() )
            throw RuntimeException();
        return m_xAggregateTypeProv->getImplementationId();
    }

    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw (RuntimeException);

    virtual Any SAL_CALL invoke( const OUString& aFunctionName, const Sequence< Any >& aParams, Sequence< ::sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam ) throw (IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException);
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException);
    virtual Any SAL_CALL getValue( const OUString& aPropertyName ) throw (UnknownPropertyException, RuntimeException);
    virtual ::sal_Bool SAL_CALL hasMethod( const OUString& aName ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL hasProperty( const OUString& aName ) throw (RuntimeException);
    virtual  Any SAL_CALL queryInterface( const Type& aType ) throw ( RuntimeException );

    virtual Sequence< Type > SAL_CALL getTypes() throw ( RuntimeException );
};

DocObjectWrapper::DocObjectWrapper( SbModule* pVar ) : m_pMod( pVar ), mName( pVar->GetName() )
{
    SbObjModule* pMod = PTR_CAST(SbObjModule,pVar);
    if ( pMod )
    {
        if ( pMod->GetModuleType() == ModuleType::DOCUMENT )
        {
            // Use proxy factory service to create aggregatable proxy.
            SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pMod->GetObject() );
            Reference< XInterface > xIf;
            if ( pUnoObj )
            {
                   Any aObj = pUnoObj->getUnoAny();
                   aObj >>= xIf;
                   if ( xIf.is() )
                   {
                       m_xAggregateTypeProv.set( xIf, UNO_QUERY );
                       m_xAggInv.set( xIf, UNO_QUERY );
                   }
            }
            if ( xIf.is() )
            {
                try
                {
                    Reference< XProxyFactory > xProxyFac = ProxyFactory::create( comphelper::getProcessComponentContext() );
                    m_xAggProxy = xProxyFac->createProxy( xIf );
                }
                catch(const Exception& )
                {
                    SAL_WARN( "basic", "DocObjectWrapper::DocObjectWrapper: Caught exception!" );
                }
            }

            if ( m_xAggProxy.is() )
            {
                osl_atomic_increment( &m_refCount );

                /* i35609 - Fix crash on Solaris. The setDelegator call needs
                    to be in its own block to ensure that all temporary Reference
                    instances that are acquired during the call are released
                    before m_refCount is decremented again */
                {
                    m_xAggProxy->setDelegator( static_cast< cppu::OWeakObject * >( this ) );
                }

                 osl_atomic_decrement( &m_refCount );
            }
        }
    }
}

void SAL_CALL
DocObjectWrapper::acquire() throw ()
{
    osl_atomic_increment( &m_refCount );
    SAL_INFO("basic","DocObjectWrapper::acquire("<< OUStringToOString( mName, RTL_TEXTENCODING_UTF8 ).getStr() << ") 0x" << this << " refcount is now " << m_refCount );
}
void SAL_CALL
DocObjectWrapper::release() throw ()
{
    if ( osl_atomic_decrement( &m_refCount ) == 0 )
    {
        SAL_INFO("basic","DocObjectWrapper::release("<< OUStringToOString( mName, RTL_TEXTENCODING_UTF8 ).getStr() << ") 0x" << this << " refcount is now " << m_refCount );
        delete this;
    }
    else
    {
        SAL_INFO("basic","DocObjectWrapper::release("<< OUStringToOString( mName, RTL_TEXTENCODING_UTF8 ).getStr() << ") 0x" << this << " refcount is now " << m_refCount );
    }
}

DocObjectWrapper::~DocObjectWrapper()
{
}

Sequence< Type > SAL_CALL DocObjectWrapper::getTypes()
    throw ( RuntimeException )
{
    if ( m_Types.getLength() == 0 )
    {
        Sequence< Type > sTypes;
        if ( m_xAggregateTypeProv.is() )
        {
            sTypes = m_xAggregateTypeProv->getTypes();
        }
        m_Types.realloc( sTypes.getLength() + 1 );
        Type* pPtr = m_Types.getArray();
        for ( int i=0; i<m_Types.getLength(); ++i, ++pPtr )
        {
            if ( i == 0 )
            {
                *pPtr = XInvocation::static_type( NULL );
            }
            else
            {
                *pPtr = sTypes[ i - 1 ];
            }
        }
    }
    return m_Types;
}

Reference< XIntrospectionAccess > SAL_CALL
DocObjectWrapper::getIntrospection(  ) throw (RuntimeException)
{
    return NULL;
}

Any SAL_CALL
DocObjectWrapper::invoke( const OUString& aFunctionName, const Sequence< Any >& aParams, Sequence< ::sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam ) throw (IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException)
{
    if ( m_xAggInv.is() &&  m_xAggInv->hasMethod( aFunctionName ) )
            return m_xAggInv->invoke( aFunctionName, aParams, aOutParamIndex, aOutParam );
    SbMethodRef pMethod = getMethod( aFunctionName );
    if ( !pMethod )
        throw RuntimeException();
    // check number of parameters
    sal_Int32 nParamsCount = aParams.getLength();
    SbxInfo* pInfo = pMethod->GetInfo();
    if ( pInfo )
    {
        sal_Int32 nSbxOptional = 0;
        sal_uInt16 n = 1;
        for ( const SbxParamInfo* pParamInfo = pInfo->GetParam( n ); pParamInfo; pParamInfo = pInfo->GetParam( ++n ) )
        {
            if ( ( pParamInfo->nFlags & SBX_OPTIONAL ) != 0 )
                ++nSbxOptional;
            else
                nSbxOptional = 0;
        }
        sal_Int32 nSbxCount = n - 1;
        if ( nParamsCount < nSbxCount - nSbxOptional )
        {
            throw RuntimeException( "wrong number of parameters!", Reference< XInterface >() );
        }
    }
    // set parameters
    SbxArrayRef xSbxParams;
    if ( nParamsCount > 0 )
    {
        xSbxParams = new SbxArray;
        const Any* pParams = aParams.getConstArray();
        for ( sal_Int32 i = 0; i < nParamsCount; ++i )
        {
            SbxVariableRef xSbxVar = new SbxVariable( SbxVARIANT );
            unoToSbxValue( static_cast< SbxVariable* >( xSbxVar ), pParams[i] );
            xSbxParams->Put( xSbxVar, static_cast< sal_uInt16 >( i ) + 1 );

            // Enable passing by ref
            if ( xSbxVar->GetType() != SbxVARIANT )
                xSbxVar->SetFlag( SBX_FIXED );
        }
    }
    if ( xSbxParams.Is() )
        pMethod->SetParameters( xSbxParams );

    // call method
    SbxVariableRef xReturn = new SbxVariable;

    pMethod->Call( xReturn );
    Any aReturn;
    // get output parameters
    if ( xSbxParams.Is() )
    {
        SbxInfo* pInfo_ = pMethod->GetInfo();
        if ( pInfo_ )
        {
            OutParamMap aOutParamMap;
            for ( sal_uInt16 n = 1, nCount = xSbxParams->Count(); n < nCount; ++n )
            {
                const SbxParamInfo* pParamInfo = pInfo_->GetParam( n );
                if ( pParamInfo && ( pParamInfo->eType & SbxBYREF ) != 0 )
                {
                    SbxVariable* pVar = xSbxParams->Get( n );
                    if ( pVar )
                    {
                        SbxVariableRef xVar = pVar;
                        aOutParamMap.insert( OutParamMap::value_type( n - 1, sbxToUnoValue( xVar ) ) );
                    }
                }
            }
            sal_Int32 nOutParamCount = aOutParamMap.size();
            aOutParamIndex.realloc( nOutParamCount );
            aOutParam.realloc( nOutParamCount );
            sal_Int16* pOutParamIndex = aOutParamIndex.getArray();
            Any* pOutParam = aOutParam.getArray();
            for ( OutParamMap::iterator aIt = aOutParamMap.begin(); aIt != aOutParamMap.end(); ++aIt, ++pOutParamIndex, ++pOutParam )
            {
                *pOutParamIndex = aIt->first;
                *pOutParam = aIt->second;
            }
        }
    }

    // get return value
    aReturn = sbxToUnoValue( xReturn );

    pMethod->SetParameters( NULL );

    return aReturn;
}

void SAL_CALL
DocObjectWrapper::setValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException)
{
    if ( m_xAggInv.is() &&  m_xAggInv->hasProperty( aPropertyName ) )
            return m_xAggInv->setValue( aPropertyName, aValue );

    SbPropertyRef pProperty = getProperty( aPropertyName );
    if ( !pProperty.Is() )
       throw UnknownPropertyException();
    unoToSbxValue( (SbxVariable*) pProperty, aValue );
}

Any SAL_CALL
DocObjectWrapper::getValue( const OUString& aPropertyName ) throw (UnknownPropertyException, RuntimeException)
{
    if ( m_xAggInv.is() &&  m_xAggInv->hasProperty( aPropertyName ) )
            return m_xAggInv->getValue( aPropertyName );

    SbPropertyRef pProperty = getProperty( aPropertyName );
    if ( !pProperty.Is() )
       throw UnknownPropertyException();

    SbxVariable* pProp = ( SbxVariable* ) pProperty;
    if ( pProp->GetType() == SbxEMPTY )
        pProperty->Broadcast( SBX_HINT_DATAWANTED );

    Any aRet = sbxToUnoValue( pProp );
    return aRet;
}

::sal_Bool SAL_CALL
DocObjectWrapper::hasMethod( const OUString& aName ) throw (RuntimeException)
{
    if ( m_xAggInv.is() && m_xAggInv->hasMethod( aName ) )
        return sal_True;
    return getMethod( aName ).Is();
}

::sal_Bool SAL_CALL
DocObjectWrapper::hasProperty( const OUString& aName ) throw (RuntimeException)
{
    sal_Bool bRes = sal_False;
    if ( m_xAggInv.is() && m_xAggInv->hasProperty( aName ) )
        bRes = sal_True;
    else bRes = getProperty( aName ).Is();
    return bRes;
}

Any SAL_CALL DocObjectWrapper::queryInterface( const Type& aType )
    throw ( RuntimeException )
{
    Any aRet = DocObjectWrapper_BASE::queryInterface( aType );
    if ( aRet.hasValue() )
        return aRet;
    else if ( m_xAggProxy.is() )
        aRet = m_xAggProxy->queryAggregation( aType );
    return aRet;
}

SbMethodRef DocObjectWrapper::getMethod( const OUString& aName ) throw (RuntimeException)
{
    SbMethodRef pMethod = NULL;
    if ( m_pMod )
    {
        sal_uInt16 nSaveFlgs = m_pMod->GetFlags();
        // Limit search to this module
        m_pMod->ResetFlag( SBX_GBLSEARCH );
        pMethod = (SbMethod*) m_pMod->SbModule::Find( aName,  SbxCLASS_METHOD );
        m_pMod->SetFlags( nSaveFlgs );
    }

    return pMethod;
}

SbPropertyRef DocObjectWrapper::getProperty( const OUString& aName ) throw (RuntimeException)
{
    SbPropertyRef pProperty = NULL;
    if ( m_pMod )
    {
        sal_uInt16 nSaveFlgs = m_pMod->GetFlags();
        // Limit search to this module.
        m_pMod->ResetFlag( SBX_GBLSEARCH );
        pProperty = (SbProperty*)m_pMod->SbModule::Find( aName,  SbxCLASS_PROPERTY );
        m_pMod->SetFlag( nSaveFlgs );
    }

    return pProperty;
}

TYPEINIT1(SbModule,SbxObject)
TYPEINIT1(SbMethod,SbxMethod)
TYPEINIT1(SbProperty,SbxProperty)
TYPEINIT1(SbProcedureProperty,SbxProperty)
TYPEINIT1(SbJScriptModule,SbModule)
TYPEINIT1(SbJScriptMethod,SbMethod)
TYPEINIT1(SbObjModule,SbModule)
TYPEINIT1(SbUserFormModule,SbObjModule)

uno::Reference< frame::XModel > getDocumentModel( StarBASIC* pb )
{
    uno::Reference< frame::XModel > xModel;
    if( pb && pb->IsDocBasic() )
    {
        uno::Any aDoc;
        if( pb->GetUNOConstant( "ThisComponent", aDoc ) )
            xModel.set( aDoc, uno::UNO_QUERY );
    }
    return xModel;
}

uno::Reference< vba::XVBACompatibility > getVBACompatibility( const uno::Reference< frame::XModel >& rxModel )
{
    uno::Reference< vba::XVBACompatibility > xVBACompat;
    try
    {
        uno::Reference< beans::XPropertySet > xModelProps( rxModel, uno::UNO_QUERY_THROW );
        xVBACompat.set( xModelProps->getPropertyValue( "BasicLibraries" ), uno::UNO_QUERY );
    }
    catch(const uno::Exception& )
    {
    }
    return xVBACompat;
}

bool getDefaultVBAMode( StarBASIC* pb )
{
    uno::Reference< vba::XVBACompatibility > xVBACompat = getVBACompatibility( getDocumentModel( pb ) );
    return xVBACompat.is() && xVBACompat->getVBACompatibilityMode();
}

class AsyncQuitHandler
{
    AsyncQuitHandler() {}
    AsyncQuitHandler( const AsyncQuitHandler&);
public:
    static AsyncQuitHandler& instance()
    {
        static AsyncQuitHandler dInst;
        return dInst;
    }

    void QuitApplication()
    {
        uno::Reference< frame::XDesktop2 > xDeskTop = frame::Desktop::create( comphelper::getProcessComponentContext() );
        xDeskTop->terminate();
    }
    DECL_LINK( OnAsyncQuit, void* );
};

IMPL_LINK( AsyncQuitHandler, OnAsyncQuit, void*, /*pNull*/ )
{
    QuitApplication();
    return 0L;
}

// A Basic module has set EXTSEARCH, so that the elements, that the modul contains,
// could be found from other module.

SbModule::SbModule( const OUString& rName,  sal_Bool bVBACompat )
         : SbxObject( "StarBASICModule" ),
           pImage( NULL ), pBreaks( NULL ), pClassData( NULL ), mbVBACompat( bVBACompat ),  pDocObject( NULL ), bIsProxyModule( false )
{
    SetName( rName );
    SetFlag( SBX_EXTSEARCH | SBX_GBLSEARCH );
    SetModuleType( script::ModuleType::NORMAL );

    // #i92642: Set name property to intitial name
    SbxVariable* pNameProp = pProps->Find( "Name", SbxCLASS_PROPERTY );
    if( pNameProp != NULL )
    {
        pNameProp->PutString( GetName() );
    }
}

SbModule::~SbModule()
{
    SAL_INFO("basic","Module named " << OUStringToOString( GetName(), RTL_TEXTENCODING_UTF8 ).getStr() << " is destructing");
    delete pImage;
    delete pBreaks;
    delete pClassData;
    mxWrapper = NULL;
}

uno::Reference< script::XInvocation >
SbModule::GetUnoModule()
{
    if ( !mxWrapper.is() )
        mxWrapper = new DocObjectWrapper( this );

    SAL_INFO("basic","Module named " << OUStringToOString( GetName(), RTL_TEXTENCODING_UTF8 ).getStr() << " returning wrapper mxWrapper (0x" << mxWrapper.get() <<")" );
    return mxWrapper;
}

sal_Bool SbModule::IsCompiled() const
{
    return sal_Bool( pImage != 0 );
}

const SbxObject* SbModule::FindType( OUString aTypeName ) const
{
    return pImage ? pImage->FindType( aTypeName ) : NULL;
}


// From the code generator: deletion of images and the oposite of validation for entries

void SbModule::StartDefinitions()
{
    delete pImage; pImage = NULL;
    if( pClassData )
        pClassData->clear();

    // methods and properties persist, but they are invalid;
    // at least are the information under certain conditions clogged
    sal_uInt16 i;
    for( i = 0; i < pMethods->Count(); i++ )
    {
        SbMethod* p = PTR_CAST(SbMethod,pMethods->Get( i ) );
        if( p )
            p->bInvalid = sal_True;
    }
    for( i = 0; i < pProps->Count(); )
    {
        SbProperty* p = PTR_CAST(SbProperty,pProps->Get( i ) );
        if( p )
            pProps->Remove( i );
        else
            i++;
    }
}

// request/create method

SbMethod* SbModule::GetMethod( const OUString& rName, SbxDataType t )
{
    SbxVariable* p = pMethods->Find( rName, SbxCLASS_METHOD );
    SbMethod* pMeth = p ? PTR_CAST(SbMethod,p) : NULL;
    if( p && !pMeth )
    {
        pMethods->Remove( p );
    }
    if( !pMeth )
    {
        pMeth = new SbMethod( rName, t, this );
        pMeth->SetParent( this );
        pMeth->SetFlags( SBX_READ );
        pMethods->Put( pMeth, pMethods->Count() );
        StartListening( pMeth->GetBroadcaster(), sal_True );
    }
    // The method is per default valid, because it could be
    // created from the compiler (code generator) as well.
    pMeth->bInvalid = sal_False;
    pMeth->ResetFlag( SBX_FIXED );
    pMeth->SetFlag( SBX_WRITE );
    pMeth->SetType( t );
    pMeth->ResetFlag( SBX_WRITE );
    if( t != SbxVARIANT )
    {
        pMeth->SetFlag( SBX_FIXED );
    }
    return pMeth;
}

// request/create property

SbProperty* SbModule::GetProperty( const OUString& rName, SbxDataType t )
{
    SbxVariable* p = pProps->Find( rName, SbxCLASS_PROPERTY );
    SbProperty* pProp = p ? PTR_CAST(SbProperty,p) : NULL;
    if( p && !pProp )
    {
        pProps->Remove( p );
    }
    if( !pProp )
    {
        pProp = new SbProperty( rName, t, this );
        pProp->SetFlag( SBX_READWRITE );
        pProp->SetParent( this );
        pProps->Put( pProp, pProps->Count() );
        StartListening( pProp->GetBroadcaster(), sal_True );
    }
    return pProp;
}

SbProcedureProperty* SbModule::GetProcedureProperty( const OUString& rName, SbxDataType t )
{
    SbxVariable* p = pProps->Find( rName, SbxCLASS_PROPERTY );
    SbProcedureProperty* pProp = p ? PTR_CAST(SbProcedureProperty,p) : NULL;
    if( p && !pProp )
    {
        pProps->Remove( p );
    }
    if( !pProp )
    {
        pProp = new SbProcedureProperty( rName, t );
        pProp->SetFlag( SBX_READWRITE );
        pProp->SetParent( this );
        pProps->Put( pProp, pProps->Count() );
        StartListening( pProp->GetBroadcaster(), sal_True );
    }
    return pProp;
}

SbIfaceMapperMethod* SbModule::GetIfaceMapperMethod( const OUString& rName, SbMethod* pImplMeth )
{
    SbxVariable* p = pMethods->Find( rName, SbxCLASS_METHOD );
    SbIfaceMapperMethod* pMapperMethod = p ? PTR_CAST(SbIfaceMapperMethod,p) : NULL;
    if( p && !pMapperMethod )
    {
        pMethods->Remove( p );
    }
    if( !pMapperMethod )
    {
        pMapperMethod = new SbIfaceMapperMethod( rName, pImplMeth );
        pMapperMethod->SetParent( this );
        pMapperMethod->SetFlags( SBX_READ );
        pMethods->Put( pMapperMethod, pMethods->Count() );
    }
    pMapperMethod->bInvalid = sal_False;
    return pMapperMethod;
}

SbIfaceMapperMethod::~SbIfaceMapperMethod()
{
}

TYPEINIT1(SbIfaceMapperMethod,SbMethod)


// From the code generator: remove invalid entries

void SbModule::EndDefinitions( sal_Bool bNewState )
{
    for( sal_uInt16 i = 0; i < pMethods->Count(); )
    {
        SbMethod* p = PTR_CAST(SbMethod,pMethods->Get( i ) );
        if( p )
        {
            if( p->bInvalid )
                pMethods->Remove( p );
            else
            {
                p->bInvalid = bNewState;
                i++;
            }
        }
        else
            i++;
    }
    SetModified( sal_True );
}

void SbModule::Clear()
{
    delete pImage; pImage = NULL;
    if( pClassData )
        pClassData->clear();
    SbxObject::Clear();
}


SbxVariable* SbModule::Find( const OUString& rName, SbxClassType t )
{
    // make sure a search in an uninstatiated class module will fail
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if ( bIsProxyModule && !GetSbData()->bRunInit )
    {
        return NULL;
    }
    if( !pRes && pImage )
    {
        SbiInstance* pInst = GetSbData()->pInst;
        if( pInst && pInst->IsCompatibility() )
        {
            // Put enum types as objects into module,
            // allows MyEnum.First notation
            SbxArrayRef xArray = pImage->GetEnums();
            if( xArray.Is() )
            {
                SbxVariable* pEnumVar = xArray->Find( rName, SbxCLASS_DONTCARE );
                SbxObject* pEnumObject = PTR_CAST( SbxObject, pEnumVar );
                if( pEnumObject )
                {
                    bool bPrivate = pEnumObject->IsSet( SBX_PRIVATE );
                    OUString aEnumName = pEnumObject->GetName();

                    pRes = new SbxVariable( SbxOBJECT );
                    pRes->SetName( aEnumName );
                    pRes->SetParent( this );
                    pRes->SetFlag( SBX_READ );
                    if( bPrivate )
                    {
                        pRes->SetFlag( SBX_PRIVATE );
                    }
                    pRes->PutObject( pEnumObject );
                }
            }
        }
    }
    return pRes;
}

const OUString& SbModule::GetSource32() const
{
    return aOUSource;
}

const OUString& SbModule::GetSource() const
{
    static OUString aRetStr;
    aRetStr = aOUSource;
    return aRetStr;
}

// Parent and BASIC are one!

void SbModule::SetParent( SbxObject* p )
{
    pParent = p;
}

void SbModule::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbProperty* pProp = PTR_CAST(SbProperty,pVar);
        SbMethod* pMeth = PTR_CAST(SbMethod,pVar);
        SbProcedureProperty* pProcProperty = PTR_CAST( SbProcedureProperty, pVar );
        if( pProcProperty )
        {

            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                OUString aProcName("Property Get ");
                aProcName += pProcProperty->GetName();

                SbxVariable* pMethVar = Find( aProcName, SbxCLASS_METHOD );
                if( pMethVar )
                {
                    SbxValues aVals;
                    aVals.eType = SbxVARIANT;

                    SbxArray* pArg = pVar->GetParameters();
                    sal_uInt16 nVarParCount = (pArg != NULL) ? pArg->Count() : 0;
                    if( nVarParCount > 1 )
                    {
                        SbxArrayRef xMethParameters = new SbxArray;
                        xMethParameters->Put( pMethVar, 0 );    // Method as parameter 0
                        for( sal_uInt16 i = 1 ; i < nVarParCount ; ++i )
                        {
                            SbxVariable* pPar = pArg->Get( i );
                            xMethParameters->Put( pPar, i );
                        }

                        pMethVar->SetParameters( xMethParameters );
                        pMethVar->Get( aVals );
                        pMethVar->SetParameters( NULL );
                    }
                    else
                    {
                        pMethVar->Get( aVals );
                    }

                    pVar->Put( aVals );
                }
            }
            else if( pHint->GetId() == SBX_HINT_DATACHANGED )
            {
                SbxVariable* pMethVar = NULL;

                bool bSet = pProcProperty->isSet();
                if( bSet )
                {
                    pProcProperty->setSet( false );

                    OUString aProcName("Property Set ");
                    aProcName += pProcProperty->GetName();
                    pMethVar = Find( aProcName, SbxCLASS_METHOD );
                }
                if( !pMethVar ) // Let
                {
                    OUString aProcName("Property Let " );
                    aProcName += pProcProperty->GetName();
                    pMethVar = Find( aProcName, SbxCLASS_METHOD );
                }

                if( pMethVar )
                {
                    // Setup parameters
                    SbxArrayRef xArray = new SbxArray;
                    xArray->Put( pMethVar, 0 ); // Method as parameter 0
                    xArray->Put( pVar, 1 );
                    pMethVar->SetParameters( xArray );

                    SbxValues aVals;
                    pMethVar->Get( aVals );
                    pMethVar->SetParameters( NULL );
                }
            }
        }
        if( pProp )
        {
            if( pProp->GetModule() != this )
                SetError( SbxERR_BAD_ACTION );
        }
        else if( pMeth )
        {
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                if( pMeth->bInvalid && !Compile() )
                {
                    // auto compile has not worked!
                    StarBASIC::Error( SbERR_BAD_PROP_VALUE );
                }
                else
                {
                    // Call of a subprogram
                    SbModule* pOld = GetSbData()->pMod;
                    GetSbData()->pMod = this;
                    Run( (SbMethod*) pVar );
                    GetSbData()->pMod = pOld;
                }
            }
        }
        else
        {
            // #i92642: Special handling for name property to avoid
            // side effects when using name as variable implicitely
            bool bForwardToSbxObject = true;

            sal_uIntPtr nId = pHint->GetId();
            if( (nId == SBX_HINT_DATAWANTED || nId == SBX_HINT_DATACHANGED) &&
                pVar->GetName().equalsIgnoreAsciiCase( "name" ) )
            {
                    bForwardToSbxObject = false;
            }
            if( bForwardToSbxObject )
            {
                SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
            }
        }
    }
}

// The setting of the source makes the image invalid
// and scans the method definitions newly in

void SbModule::SetSource( const OUString& r )
{
    SetSource32( r );
}

void SbModule::SetSource32( const OUString& r )
{
    // Default basic mode to library container mode, but.. allow Option VBASupport 0/1 override
    SetVBACompat( getDefaultVBAMode( static_cast< StarBASIC*>( GetParent() ) ) );
    aOUSource = r;
    StartDefinitions();
    SbiTokenizer aTok( r );
    aTok.SetCompatible( IsVBACompat() );
    while( !aTok.IsEof() )
    {
        SbiToken eEndTok = NIL;

        // Searching for SUB or FUNCTION
        SbiToken eLastTok = NIL;
        while( !aTok.IsEof() )
        {
            // #32385: not by declare
            SbiToken eCurTok = aTok.Next();
            if( eLastTok != DECLARE )
            {
                if( eCurTok == SUB )
                {
                    eEndTok = ENDSUB; break;
                }
                if( eCurTok == FUNCTION )
                {
                    eEndTok = ENDFUNC; break;
                }
                if( eCurTok == PROPERTY )
                {
                    eEndTok = ENDPROPERTY; break;
                }
                if( eCurTok == OPTION )
                {
                    eCurTok = aTok.Next();
                    if( eCurTok == COMPATIBLE )
                    {
                        aTok.SetCompatible( true );
                    }
                    else if ( ( eCurTok == VBASUPPORT ) && ( aTok.Next() == NUMBER ) )
                    {
                        sal_Bool bIsVBA = ( aTok.GetDbl()== 1 );
                        SetVBACompat( bIsVBA );
                        aTok.SetCompatible( bIsVBA );
                    }
                }
            }
            eLastTok = eCurTok;
        }
        // Definition of the method
        SbMethod* pMeth = NULL;
        if( eEndTok != NIL )
        {
            sal_uInt16 nLine1 = aTok.GetLine();
            if( aTok.Next() == SYMBOL )
            {
                OUString aName_( aTok.GetSym() );
                SbxDataType t = aTok.GetType();
                if( t == SbxVARIANT && eEndTok == ENDSUB )
                {
                    t = SbxVOID;
                }
                pMeth = GetMethod( aName_, t );
                pMeth->nLine1 = pMeth->nLine2 = nLine1;
                // The method is for a start VALID
                pMeth->bInvalid = sal_False;
            }
            else
            {
                eEndTok = NIL;
            }
        }
        // Skip up to END SUB/END FUNCTION
        if( eEndTok != NIL )
        {
            while( !aTok.IsEof() )
            {
                if( aTok.Next() == eEndTok )
                {
                    pMeth->nLine2 = aTok.GetLine();
                    break;
                }
            }
            if( aTok.IsEof() )
            {
                pMeth->nLine2 = aTok.GetLine();
            }
        }
    }
    EndDefinitions( sal_True );
}

// Broadcast of a hint to all Basics

static void _SendHint( SbxObject* pObj, sal_uIntPtr nId, SbMethod* p )
{
    // Self a BASIC?
    if( pObj->IsA( TYPE(StarBASIC) ) && pObj->IsBroadcaster() )
        pObj->GetBroadcaster().Broadcast( SbxHint( nId, p ) );
    // Then ask for the subobjects
    SbxArray* pObjs = pObj->GetObjects();
    for( sal_uInt16 i = 0; i < pObjs->Count(); i++ )
    {
        SbxVariable* pVar = pObjs->Get( i );
        if( pVar->IsA( TYPE(SbxObject) ) )
            _SendHint( PTR_CAST(SbxObject,pVar), nId, p );
    }
}

static void SendHint( SbxObject* pObj, sal_uIntPtr nId, SbMethod* p )
{
    while( pObj->GetParent() )
        pObj = pObj->GetParent();
    _SendHint( pObj, nId, p );
}

// #57841 Clear Uno-Objects, which were helt in RTL functions,
// at the end of the program, so that nothing were helt.
void ClearUnoObjectsInRTL_Impl_Rek( StarBASIC* pBasic )
{
    // delete the return value of CreateUnoService
    static OUString aName("CreateUnoService");
    SbxVariable* pVar = pBasic->GetRtl()->Find( aName, SbxCLASS_METHOD );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    // delete the return value of CreateUnoDialog
    static OUString aName2("CreateUnoDialog");
    pVar = pBasic->GetRtl()->Find( aName2, SbxCLASS_METHOD );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    // delete the return value of CDec
    static OUString aName3("CDec");
    pVar = pBasic->GetRtl()->Find( aName3, SbxCLASS_METHOD );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    // delete return value of CreateObject
    static OUString aName4("CreateObject");
    pVar = pBasic->GetRtl()->Find( aName4, SbxCLASS_METHOD );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    // Go over all Sub-Basics
    SbxArray* pObjs = pBasic->GetObjects();
    sal_uInt16 nCount = pObjs->Count();
    for( sal_uInt16 i = 0 ; i < nCount ; i++ )
    {
        SbxVariable* pObjVar = pObjs->Get( i );
        StarBASIC* pSubBasic = PTR_CAST( StarBASIC, pObjVar );
        if( pSubBasic )
        {
            ClearUnoObjectsInRTL_Impl_Rek( pSubBasic );
        }
    }
}

void ClearUnoObjectsInRTL_Impl( StarBASIC* pBasic )
{
    // #67781 Delete return values of the Uno-methods
    clearUnoMethods();
    clearUnoServiceCtors();

    ClearUnoObjectsInRTL_Impl_Rek( pBasic );

    // Search for the topmost Basic
    SbxObject* p = pBasic;
    while( p->GetParent() )
        p = p->GetParent();
    if( ((StarBASIC*)p) != pBasic )
        ClearUnoObjectsInRTL_Impl_Rek( (StarBASIC*)p );
}

bool SbModule::IsVBACompat() const
{
    return mbVBACompat;
}

void SbModule::SetVBACompat( bool bCompat )
{
    if( mbVBACompat != bCompat )
    {
        mbVBACompat = bCompat;
        // initialize VBA document API
        if( mbVBACompat ) try
        {
            StarBASIC* pBasic = static_cast< StarBASIC* >( GetParent() );
            uno::Reference< lang::XMultiServiceFactory > xFactory( getDocumentModel( pBasic ), uno::UNO_QUERY_THROW );
            xFactory->createInstance( "ooo.vba.VBAGlobals" );
        }
        catch( Exception& )
        {
        }
    }
}

// Run a Basic-subprogram
sal_uInt16 SbModule::Run( SbMethod* pMeth )
{
    SAL_INFO("basic","About to run " << OUStringToOString( pMeth->GetName(), RTL_TEXTENCODING_UTF8 ).getStr() << ", vba compatmode is " << mbVBACompat );
    static sal_uInt16 nMaxCallLevel = 0;

    sal_uInt16 nRes = 0;
    bool bDelInst = ( GetSbData()->pInst == NULL );
    bool bQuit = false;
    StarBASICRef xBasic;
    uno::Reference< frame::XModel > xModel;
    uno::Reference< script::vba::XVBACompatibility > xVBACompat;
    if( bDelInst )
    {
        // #32779: Hold Basic during the execution
        xBasic = (StarBASIC*) GetParent();

        GetSbData()->pInst = new SbiInstance( (StarBASIC*) GetParent() );

        /*  If a VBA script in a document is started, get the VBA compatibility
            interface from the document Basic library container, and notify all
            VBA script listeners about the started script. */
        if( mbVBACompat )
        {
            StarBASIC* pBasic = static_cast< StarBASIC* >( GetParent() );
            if( pBasic && pBasic->IsDocBasic() ) try
            {
                xModel.set( getDocumentModel( pBasic ), uno::UNO_SET_THROW );
                xVBACompat.set( getVBACompatibility( xModel ), uno::UNO_SET_THROW );
                xVBACompat->broadcastVBAScriptEvent( script::vba::VBAScriptEventId::SCRIPT_STARTED, GetName() );
            }
            catch(const uno::Exception& )
            {
            }
        }

        // Launcher problem
        // i80726 The Find below will genarate an error in Testtool so we reset it unless there was one before already
        sal_Bool bWasError = SbxBase::GetError() != 0;
        SbxVariable* pMSOMacroRuntimeLibVar = Find( "Launcher", SbxCLASS_OBJECT );
        if ( !bWasError && (SbxBase::GetError() == SbxERR_PROC_UNDEFINED) )
            SbxBase::ResetError();
        if( pMSOMacroRuntimeLibVar )
        {
            StarBASIC* pMSOMacroRuntimeLib = PTR_CAST(StarBASIC,pMSOMacroRuntimeLibVar);
            if( pMSOMacroRuntimeLib )
            {
                sal_uInt16 nGblFlag = pMSOMacroRuntimeLib->GetFlags() & SBX_GBLSEARCH;
                pMSOMacroRuntimeLib->ResetFlag( SBX_GBLSEARCH );
                SbxVariable* pAppSymbol = pMSOMacroRuntimeLib->Find( "Application", SbxCLASS_METHOD );
                pMSOMacroRuntimeLib->SetFlag( nGblFlag );
                if( pAppSymbol )
                {
                    pMSOMacroRuntimeLib->SetFlag( SBX_EXTSEARCH );      // Could have been disabled before
                    GetSbData()->pMSOMacroRuntimLib = pMSOMacroRuntimeLib;
                }
            }
        }

        if( nMaxCallLevel == 0 )
        {
#ifdef UNX
          struct rlimit rl;
          getrlimit ( RLIMIT_STACK, &rl );
#endif
#if defined LINUX
          // Empiric value, 900 = needed bytes/Basic call level
          // for Linux including 10% safety margin
          nMaxCallLevel = rl.rlim_cur / 900;
#elif defined SOLARIS
          // Empiric value, 1650 = needed bytes/Basic call level
          // for Solaris including 10% safety margin
          nMaxCallLevel = rl.rlim_cur / 1650;
#elif defined WIN32
          nMaxCallLevel = 5800;
#else
          nMaxCallLevel = MAXRECURSION;
#endif
        }
    }

    // Recursion to deep?
    if( ++GetSbData()->pInst->nCallLvl <= nMaxCallLevel )
    {
        // Define a globale variable in all Mods
        GlobalRunInit( /* bBasicStart = */ bDelInst );

        // Appeared a compiler error? Then we don't launch
        if( !GetSbData()->bGlobalInitErr )
        {
            if( bDelInst )
            {
                SendHint( GetParent(), SBX_HINT_BASICSTART, pMeth );

                // 1996-10-16: #31460 New concept for StepInto/Over/Out
                // For an explanation see runtime.cxx at SbiInstance::CalcBreakCallLevel()
                // Identify the BreakCallLevel
                GetSbData()->pInst->CalcBreakCallLevel( pMeth->GetDebugFlags() );
            }

            SbModule* pOldMod = GetSbData()->pMod;
            GetSbData()->pMod = this;
            SbiRuntime* pRt = new SbiRuntime( this, pMeth, pMeth->nStart );

            pRt->pNext = GetSbData()->pInst->pRun;
            if( pRt->pNext )
                pRt->pNext->block();
            GetSbData()->pInst->pRun = pRt;
            if ( mbVBACompat )
            {
                GetSbData()->pInst->EnableCompatibility( sal_True );
            }
            while( pRt->Step() ) {}
            if( pRt->pNext )
                pRt->pNext->unblock();

            // #63710 It can happen by an another thread handling at events,
            // that the show call returns to an dialog (by closing the
            // dialog per UI), before a by an event triggered further call returned,
            // which stands in Basic more top in the stack and that had been run on
            // a  Basic-Breakpoint. Then would the instance below destroyed. And if the Basic,
            // that stand still in the call, further runs, there is a GPF.
            // Thus here had to be wait until the other call comes back.
            if( bDelInst )
            {
                // Compare here with 1 instead of 0, because before nCallLvl--
                while( GetSbData()->pInst->nCallLvl != 1 )
                    GetpApp()->Yield();
            }

            nRes = sal_True;
            GetSbData()->pInst->pRun = pRt->pNext;
            GetSbData()->pInst->nCallLvl--;          // Call-Level down again

            // Exist an higher-ranking runtime instance?
            // Then take over SbDEBUG_BREAK, if set
            SbiRuntime* pRtNext = pRt->pNext;
            if( pRtNext && (pRt->GetDebugFlags() & SbDEBUG_BREAK) )
                pRtNext->SetDebugFlags( SbDEBUG_BREAK );

            delete pRt;
            GetSbData()->pMod = pOldMod;
            if( bDelInst )
            {
                // #57841 Clear Uno-Objects, which were helt in RTL functions,
                // at the end of the program, so that nothing were helt.
                ClearUnoObjectsInRTL_Impl( xBasic );

                clearNativeObjectWrapperVector();

                SAL_WARN_IF(GetSbData()->pInst->nCallLvl != 0,"basic","BASIC-Call-Level > 0");
                delete GetSbData()->pInst, GetSbData()->pInst = NULL, bDelInst = false;

                // #i30690
                SolarMutexGuard aSolarGuard;
                SendHint( GetParent(), SBX_HINT_BASICSTOP, pMeth );

                GlobalRunDeInit();

#ifdef DBG_UTIL
                ResetCapturedAssertions();
#endif

                if( xVBACompat.is() )
                {
                    // notify all VBA script listeners about the stopped script
                    try
                    {
                        xVBACompat->broadcastVBAScriptEvent( script::vba::VBAScriptEventId::SCRIPT_STOPPED, GetName() );
                    }
                    catch(const uno::Exception& )
                    {
                    }
                    // VBA always ensures screenupdating is enabled after completing
                    ::basic::vba::lockControllersOfAllDocuments( xModel, sal_False );
                    ::basic::vba::enableContainerWindowsOfAllDocuments( xModel, sal_True );
                }

#ifdef DBG_TRACE_BASIC
                dbg_DeInitTrace();
#endif
            }
        }
        else
               GetSbData()->pInst->nCallLvl--;           // Call-Level down again
    }
    else
    {
        GetSbData()->pInst->nCallLvl--;          // Call-Level down again
        StarBASIC::FatalError( SbERR_STACK_OVERFLOW );
    }

    StarBASIC* pBasic = PTR_CAST(StarBASIC,GetParent());
    if( bDelInst )
    {
       // #57841 Clear Uno-Objects, which were helt in RTL functions,
       // the end of the program, so that nothing were helt.
        ClearUnoObjectsInRTL_Impl( xBasic );

        delete GetSbData()->pInst;
        GetSbData()->pInst = NULL;
    }
    if ( pBasic && pBasic->IsDocBasic() && pBasic->IsQuitApplication() && !GetSbData()->pInst )
            bQuit = true;
        if ( bQuit )
    {
        Application::PostUserEvent( LINK( &AsyncQuitHandler::instance(), AsyncQuitHandler, OnAsyncQuit ), NULL );
    }

    return nRes;
}

// Execute of the init method of a module after the loading
// or the compilation

void SbModule::RunInit()
{
    if( pImage
     && !pImage->bInit
     && pImage->GetFlag( SBIMG_INITCODE ) )
    {
        // Set flag, so that RunInit get activ (Testtool)
        GetSbData()->bRunInit = true;

        SbModule* pOldMod = GetSbData()->pMod;
        GetSbData()->pMod = this;
        // The init code starts always here
        SbiRuntime* pRt = new SbiRuntime( this, NULL, 0 );

        pRt->pNext = GetSbData()->pInst->pRun;
        GetSbData()->pInst->pRun = pRt;
        while( pRt->Step() ) {}

        GetSbData()->pInst->pRun = pRt->pNext;
        delete pRt;
        GetSbData()->pMod = pOldMod;
        pImage->bInit = true;
        pImage->bFirstInit = false;

        // RunInit is not activ anymore
        GetSbData()->bRunInit = false;
    }
}

// Delete with private/dim declared variables

void SbModule::AddVarName( const OUString& aName )
{
    // see if the name is added already
    std::vector< OUString >::iterator it_end = mModuleVariableNames.end();
    for ( std::vector< OUString >::iterator it = mModuleVariableNames.begin(); it != it_end; ++it )
    {
        if ( aName == *it )
            return;
    }
    mModuleVariableNames.push_back( aName );
}

void SbModule::RemoveVars()
{
    std::vector< OUString >::iterator it_end = mModuleVariableNames.end();
    for ( std::vector< OUString >::iterator it = mModuleVariableNames.begin(); it != it_end; ++it )
    {
    // We don't want a Find being called in a derived class ( e.g.
    // SbUserform because it could trigger say an initialise event
    // which would cause basic to be re-run in the middle of the init ( and remember RemoveVars is called from compile and we don't want code to run as part of the compile )
    SbxVariableRef p = SbModule::Find( *it, SbxCLASS_PROPERTY );
    if( p.Is() )
        Remove (p);
    }
}

void SbModule::ClearPrivateVars()
{
    for( sal_uInt16 i = 0 ; i < pProps->Count() ; i++ )
    {
        SbProperty* p = PTR_CAST(SbProperty,pProps->Get( i ) );
        if( p )
        {
            // Delete not the arrays, only their content
            if( p->GetType() & SbxARRAY )
            {
                SbxArray* pArray = PTR_CAST(SbxArray,p->GetObject());
                if( pArray )
                {
                    for( sal_uInt16 j = 0 ; j < pArray->Count() ; j++ )
                    {
                        SbxVariable* pj = PTR_CAST(SbxVariable,pArray->Get( j ));
                        pj->SbxValue::Clear();
                    }
                }
            }
            else
            {
                p->SbxValue::Clear();
            }
        }
    }
}

void SbModule::implClearIfVarDependsOnDeletedBasic( SbxVariable* pVar, StarBASIC* pDeletedBasic )
{
    if( pVar->SbxValue::GetType() != SbxOBJECT || pVar->ISA( SbProcedureProperty ) )
        return;

    SbxObject* pObj = PTR_CAST(SbxObject,pVar->GetObject());
    if( pObj != NULL )
    {
        SbxObject* p = pObj;

        SbModule* pMod = PTR_CAST( SbModule, p );
        if( pMod != NULL )
            pMod->ClearVarsDependingOnDeletedBasic( pDeletedBasic );

        while( (p = p->GetParent()) != NULL )
        {
            StarBASIC* pBasic = PTR_CAST( StarBASIC, p );
            if( pBasic != NULL && pBasic == pDeletedBasic )
            {
                pVar->SbxValue::Clear();
                break;
            }
        }
    }
}

void SbModule::ClearVarsDependingOnDeletedBasic( StarBASIC* pDeletedBasic )
{
    (void)pDeletedBasic;

    for( sal_uInt16 i = 0 ; i < pProps->Count() ; i++ )
    {
        SbProperty* p = PTR_CAST(SbProperty,pProps->Get( i ) );
        if( p )
        {
            if( p->GetType() & SbxARRAY )
            {
                SbxArray* pArray = PTR_CAST(SbxArray,p->GetObject());
                if( pArray )
                {
                    for( sal_uInt16 j = 0 ; j < pArray->Count() ; j++ )
                    {
                        SbxVariable* pVar = PTR_CAST(SbxVariable,pArray->Get( j ));
                        implClearIfVarDependsOnDeletedBasic( pVar, pDeletedBasic );
                    }
                }
            }
            else
            {
                implClearIfVarDependsOnDeletedBasic( p, pDeletedBasic );
            }
        }
    }
}

void StarBASIC::ClearAllModuleVars( void )
{
    // Initialise the own module
    for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = (SbModule*)pModules->Get( nMod );
        // Initialise only, if the startcode was already executed
        if( pModule->pImage && pModule->pImage->bInit && !pModule->isProxyModule() && !pModule->ISA(SbObjModule) )
            pModule->ClearPrivateVars();
    }

}

// Execution of the init-code of all module
void SbModule::GlobalRunInit( bool bBasicStart )
{
    // If no Basic-Start, only initialise, if the module is not initialised
    if( !bBasicStart )
        if( !(pImage && !pImage->bInit) )
            return;

    // Initialise GlobalInitErr-Flag for Compiler-Error
    // With the help of this flags could be located in SbModule::Run() after the call of
    // GlobalRunInit, if at the intialising of the module
    // an error occurred. Then it will not be launched.
    GetSbData()->bGlobalInitErr = false;

    // Parent of the module is a Basic
    StarBASIC *pBasic = PTR_CAST(StarBASIC,GetParent());
    if( pBasic )
    {
        pBasic->InitAllModules();

        SbxObject* pParent_ = pBasic->GetParent();
        if( pParent_ )
        {
            StarBASIC * pParentBasic = PTR_CAST(StarBASIC,pParent_);
            if( pParentBasic )
            {
                pParentBasic->InitAllModules( pBasic );

                // #109018 Parent can also have a parent (library in doc)
                SbxObject* pParentParent = pParentBasic->GetParent();
                if( pParentParent )
                {
                    StarBASIC * pParentParentBasic = PTR_CAST(StarBASIC,pParentParent);
                    if( pParentParentBasic )
                        pParentParentBasic->InitAllModules( pParentBasic );
                }
            }
        }
    }
}

void SbModule::GlobalRunDeInit( void )
{
    StarBASIC *pBasic = PTR_CAST(StarBASIC,GetParent());
    if( pBasic )
    {
        pBasic->DeInitAllModules();

        SbxObject* pParent_ = pBasic->GetParent();
        if( pParent_ )
            pBasic = PTR_CAST(StarBASIC,pParent_);
        if( pBasic )
            pBasic->DeInitAllModules();
    }
}

// Search for the next STMNT-Command in the code. This was used from the STMNT-
// Opcode to set the endcolumn.

const sal_uInt8* SbModule::FindNextStmnt( const sal_uInt8* p, sal_uInt16& nLine, sal_uInt16& nCol ) const
{
    return FindNextStmnt( p, nLine, nCol, sal_False );
}

const sal_uInt8* SbModule::FindNextStmnt( const sal_uInt8* p, sal_uInt16& nLine, sal_uInt16& nCol,
    sal_Bool bFollowJumps, const SbiImage* pImg ) const
{
    sal_uInt32 nPC = (sal_uInt32) ( p - (const sal_uInt8*) pImage->GetCode() );
    while( nPC < pImage->GetCodeSize() )
    {
        SbiOpcode eOp = (SbiOpcode ) ( *p++ );
        nPC++;
        if( bFollowJumps && eOp == _JUMP && pImg )
        {
            SAL_WARN_IF( !pImg, "basic", "FindNextStmnt: pImg==NULL with FollowJumps option" );
            sal_uInt32 nOp1 = *p++; nOp1 |= *p++ << 8;
            nOp1 |= *p++ << 16; nOp1 |= *p++ << 24;
            p = (const sal_uInt8*) pImg->GetCode() + nOp1;
        }
        else if( eOp >= SbOP1_START && eOp <= SbOP1_END )
            p += 4, nPC += 4;
        else if( eOp == _STMNT )
        {
            sal_uInt32 nl, nc;
            nl = *p++; nl |= *p++ << 8;
            nl |= *p++ << 16 ; nl |= *p++ << 24;
            nc = *p++; nc |= *p++ << 8;
            nc |= *p++ << 16 ; nc |= *p++ << 24;
            nLine = (sal_uInt16)nl; nCol = (sal_uInt16)nc;
            return p;
        }
        else if( eOp >= SbOP2_START && eOp <= SbOP2_END )
            p += 8, nPC += 8;
        else if( !( eOp >= SbOP0_START && eOp <= SbOP0_END ) )
        {
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
            break;
        }
    }
    return NULL;
}

// Test, if a line contains STMNT-Opcodes

sal_Bool SbModule::IsBreakable( sal_uInt16 nLine ) const
{
    if( !pImage )
        return sal_False;
    const sal_uInt8* p = (const sal_uInt8* ) pImage->GetCode();
    sal_uInt16 nl, nc;
    while( ( p = FindNextStmnt( p, nl, nc ) ) != NULL )
        if( nl == nLine )
            return sal_True;
    return sal_False;
}

sal_Bool SbModule::IsBP( sal_uInt16 nLine ) const
{
    if( pBreaks )
    {
        for( size_t i = 0; i < pBreaks->size(); i++ )
        {
            sal_uInt16 b = pBreaks->operator[]( i );
            if( b == nLine )
                return sal_True;
            if( b < nLine )
                break;
        }
    }
    return sal_False;
}

sal_Bool SbModule::SetBP( sal_uInt16 nLine )
{
    if( !IsBreakable( nLine ) )
        return sal_False;
    if( !pBreaks )
        pBreaks = new SbiBreakpoints;
    size_t i;
    for( i = 0; i < pBreaks->size(); i++ )
    {
        sal_uInt16 b = pBreaks->operator[]( i );
        if( b == nLine )
            return sal_True;
        if( b < nLine )
            break;
    }
    pBreaks->insert( pBreaks->begin() + i, nLine );

    // #38568: Set during runtime as well here SbDEBUG_BREAK
    if( GetSbData()->pInst && GetSbData()->pInst->pRun )
        GetSbData()->pInst->pRun->SetDebugFlags( SbDEBUG_BREAK );

    return IsBreakable( nLine );
}

sal_Bool SbModule::ClearBP( sal_uInt16 nLine )
{
    sal_Bool bRes = sal_False;
    if( pBreaks )
    {
        for( size_t i = 0; i < pBreaks->size(); i++ )
        {
            sal_uInt16 b = pBreaks->operator[]( i );
            if( b == nLine )
            {
                pBreaks->erase( pBreaks->begin() + i );
                bRes = sal_True;
                break;
            }
            if( b < nLine )
                break;
        }
        if( pBreaks->empty() )
            delete pBreaks, pBreaks = NULL;
    }
    return bRes;
}

void SbModule::ClearAllBP()
{
    delete pBreaks;
    pBreaks = NULL;
}

void
SbModule::fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg ) const
{
        if ( !pImg )
            pImg = pImage;
        for( sal_uInt32 i = 0; i < pMethods->Count(); i++ )
        {
            SbMethod* pMeth = PTR_CAST(SbMethod,pMethods->Get( (sal_uInt16)i ) );
            if( pMeth )
            {
                //fixup method start positions
                if ( bCvtToLegacy )
                    pMeth->nStart = pImg->CalcLegacyOffset( pMeth->nStart );
                else
                    pMeth->nStart = pImg->CalcNewOffset( (sal_uInt16)pMeth->nStart );
            }
        }

}

sal_Bool SbModule::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    Clear();
    if( !SbxObject::LoadData( rStrm, 1 ) )
        return sal_False;
    // As a precaution...
    SetFlag( SBX_EXTSEARCH | SBX_GBLSEARCH );
    sal_uInt8 bImage;
    rStrm >> bImage;
    if( bImage )
    {
        SbiImage* p = new SbiImage;
        sal_uInt32 nImgVer = 0;

        if( !p->Load( rStrm, nImgVer ) )
        {
            delete p;
            return sal_False;
        }
        // If the image is in old format, we fix up the method start offsets
        if ( nImgVer < B_EXT_IMG_VERSION )
        {
            fixUpMethodStart( false, p );
            p->ReleaseLegacyBuffer();
        }
        aComment = p->aComment;
        SetName( p->aName );
        if( p->GetCodeSize() )
        {
            aOUSource = p->aOUSource;
            // Old version: image away
            if( nVer == 1 )
            {
                SetSource32( p->aOUSource );
                delete p;
            }
            else
                pImage = p;
        }
        else
        {
            SetSource32( p->aOUSource );
            delete p;
        }
    }
    return sal_True;
}

sal_Bool SbModule::StoreData( SvStream& rStrm ) const
{
    bool bFixup = ( pImage && !pImage->ExceedsLegacyLimits() );
    if ( bFixup )
        fixUpMethodStart( true );
    sal_Bool bRet = SbxObject::StoreData( rStrm );
    if ( !bRet )
        return sal_False;

    if( pImage )
    {
        pImage->aOUSource = aOUSource;
        pImage->aComment = aComment;
        pImage->aName = GetName();
        rStrm << (sal_uInt8) 1;
        // # PCode is saved only for legacy formats only
        // It should be noted that it probably isn't necessary
        // It would be better not to store the image ( more flexible with
        // formats )
        bool bRes = pImage->Save( rStrm, B_LEGACYVERSION );
        if ( bFixup )
            fixUpMethodStart( false ); // restore method starts
        return bRes;

    }
    else
    {
        SbiImage aImg;
        aImg.aOUSource = aOUSource;
        aImg.aComment = aComment;
        aImg.aName = GetName();
        rStrm << (sal_uInt8) 1;
        return aImg.Save( rStrm );
    }
}

sal_Bool SbModule::ExceedsLegacyModuleSize()
{
    if ( !IsCompiled() )
        Compile();
    if ( pImage && pImage->ExceedsLegacyLimits() )
        return true;
    return false;
}

class ErrorHdlResetter
{
    Link    mErrHandler;
    bool mbError;
    public:
    ErrorHdlResetter() : mbError( false )
    {
        // save error handler
        mErrHandler = StarBASIC::GetGlobalErrorHdl();
        // set new error handler
        StarBASIC::SetGlobalErrorHdl( LINK( this, ErrorHdlResetter, BasicErrorHdl ) );
    }
    ~ErrorHdlResetter()
    {
        // restore error handler
        StarBASIC::SetGlobalErrorHdl(mErrHandler);
    }
    DECL_LINK( BasicErrorHdl, StarBASIC * );
    bool HasError() { return mbError; }
};
IMPL_LINK( ErrorHdlResetter, BasicErrorHdl, StarBASIC *, /*pBasic*/)
{
    mbError = true;
    return 0;
}

CodeCompleteDataCache SbModule::GetCodeCompleteDataFromParse()
{
    CodeCompleteDataCache aCache;

    ErrorHdlResetter aErrHdl;
    SbxBase::ResetError();

    SbiParser* pParser = new SbiParser( (StarBASIC*) GetParent(), this );
    pParser->SetCodeCompleting(true);

    while( pParser->Parse() ) {}
    SbiSymPool* pPool = pParser->pPool;
    CodeCompleteVarTypes aGlobVarTypes;
    for( sal_uInt16 i = 0; i < pPool->GetSize(); ++i )
    {
        SbiSymDef* pSymDef = pPool->Get(i);
        if( pSymDef->GetType() == SbxOBJECT )
        {
            if( !pParser->aGblStrings.Find( pSymDef->GetTypeId() ).isEmpty() )
            {
                //std::cerr << "global " << pSymDef->GetName() << std::endl;
                aGlobVarTypes.insert( CodeCompleteVarTypes::value_type( pSymDef->GetName(), pParser->aGblStrings.Find( pSymDef->GetTypeId() ) ) );
            }
        }

        SbiSymPool& pChildPool = pSymDef->GetPool();
        CodeCompleteVarTypes aLocVarTypes;
        for(sal_uInt16 j = 0; j < pChildPool.GetSize(); ++j )
        {
            SbiSymDef* pChildSymDef = pChildPool.Get(j);
            if( pChildSymDef->GetType() == SbxOBJECT )
            {
                if( !pParser->aGblStrings.Find( pChildSymDef->GetTypeId() ).isEmpty() )
                {
                    //std::cerr << "local " << pChildSymDef->GetName() << std::endl;
                    aLocVarTypes.insert( CodeCompleteVarTypes::value_type( pChildSymDef->GetName(), pParser->aGblStrings.Find( pChildSymDef->GetTypeId() ) ) );
                }
            }
        }
        aCache.InsertProcedure( pSymDef->GetName(), aLocVarTypes );
    }
    aCache.InsertProcedure( aCache.GLOB_KEY, aGlobVarTypes );

    delete pParser;
    return aCache;
}

SbxArrayRef SbModule::GetMethods()
{
    return pMethods;
}

bool SbModule::HasExeCode()
{
    // And empty Image always has the Global Chain set up
    static const unsigned char pEmptyImage[] = { 0x45, 0x0 , 0x0, 0x0, 0x0 };
    // lets be stricter for the moment than VBA

    if (!IsCompiled())
    {
        ErrorHdlResetter aGblErrHdl;
        Compile();
        if (aGblErrHdl.HasError()) //assume unsafe on compile error
            return true;
    }

    bool bRes = false;
    if (pImage && !(pImage->GetCodeSize() == 5 && (memcmp(pImage->GetCode(), pEmptyImage, pImage->GetCodeSize()) == 0 )))
        bRes = true;

    return bRes;
}

// Store only image, no source
sal_Bool SbModule::StoreBinaryData( SvStream& rStrm )
{
    return StoreBinaryData( rStrm, 0 );
}

sal_Bool SbModule::StoreBinaryData( SvStream& rStrm, sal_uInt16 nVer )
{
    sal_Bool bRet = Compile();
    if( bRet )
    {
        bool bFixup = ( !nVer && !pImage->ExceedsLegacyLimits() );// save in old image format, fix up method starts

        if ( bFixup ) // save in old image format, fix up method starts
            fixUpMethodStart( true );
         bRet = SbxObject::StoreData( rStrm );
        if( bRet )
        {
            pImage->aOUSource = OUString();
            pImage->aComment = aComment;
            pImage->aName = GetName();

            rStrm << (sal_uInt8) 1;
                    if ( nVer )
                        bRet = pImage->Save( rStrm, B_EXT_IMG_VERSION );
                    else
                        bRet = pImage->Save( rStrm, B_LEGACYVERSION );
                    if ( bFixup )
                        fixUpMethodStart( false ); // restore method starts

            pImage->aOUSource = aOUSource;
        }
    }
    return bRet;
}

// Called for >= OO 1.0 passwd protected libraries only

sal_Bool SbModule::LoadBinaryData( SvStream& rStrm )
{
    OUString aKeepSource = aOUSource;
    bool bRet = LoadData( rStrm, 2 );
    LoadCompleted();
    aOUSource = aKeepSource;
    return bRet;
}

sal_Bool SbModule::LoadCompleted()
{
    SbxArray* p = GetMethods();
    sal_uInt16 i;
    for( i = 0; i < p->Count(); i++ )
    {
        SbMethod* q = PTR_CAST(SbMethod,p->Get( i ) );
        if( q )
            q->pMod = this;
    }
    p = GetProperties();
    for( i = 0; i < p->Count(); i++ )
    {
        SbProperty* q = PTR_CAST(SbProperty,p->Get( i ) );
        if( q )
            q->pMod = this;
    }
    return sal_True;
}

void SbModule::handleProcedureProperties( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    bool bDone = false;

    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbProcedureProperty* pProcProperty = PTR_CAST( SbProcedureProperty, pVar );
        if( pProcProperty )
        {
            bDone = true;

            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                OUString aProcName("Property Get ");
                aProcName += pProcProperty->GetName();

                SbxVariable* pMeth = Find( aProcName, SbxCLASS_METHOD );
                if( pMeth )
                {
                    SbxValues aVals;
                    aVals.eType = SbxVARIANT;

                    SbxArray* pArg = pVar->GetParameters();
                    sal_uInt16 nVarParCount = (pArg != NULL) ? pArg->Count() : 0;
                    if( nVarParCount > 1 )
                    {
                        SbxArrayRef xMethParameters = new SbxArray;
                        xMethParameters->Put( pMeth, 0 );   // Method as parameter 0
                        for( sal_uInt16 i = 1 ; i < nVarParCount ; ++i )
                        {
                            SbxVariable* pPar = pArg->Get( i );
                            xMethParameters->Put( pPar, i );
                        }

                        pMeth->SetParameters( xMethParameters );
                        pMeth->Get( aVals );
                        pMeth->SetParameters( NULL );
                    }
                    else
                    {
                        pMeth->Get( aVals );
                    }

                    pVar->Put( aVals );
                }
            }
            else if( pHint->GetId() == SBX_HINT_DATACHANGED )
            {
                SbxVariable* pMeth = NULL;

                bool bSet = pProcProperty->isSet();
                if( bSet )
                {
                    pProcProperty->setSet( false );

                    OUString aProcName("Property Set " );
                    aProcName += pProcProperty->GetName();
                    pMeth = Find( aProcName, SbxCLASS_METHOD );
                }
                if( !pMeth )    // Let
                {
                    OUString aProcName("Property Let " );
                    aProcName += pProcProperty->GetName();
                    pMeth = Find( aProcName, SbxCLASS_METHOD );
                }

                if( pMeth )
                {
                    // Setup parameters
                    SbxArrayRef xArray = new SbxArray;
                    xArray->Put( pMeth, 0 );    // Method as parameter 0
                    xArray->Put( pVar, 1 );
                    pMeth->SetParameters( xArray );

                    SbxValues aVals;
                    pMeth->Get( aVals );
                    pMeth->SetParameters( NULL );
                }
            }
        }
    }

    if( !bDone )
        SbModule::Notify( rBC, rHint );
}


// Implementation SbJScriptModule (Basic module for JavaScript source code)
SbJScriptModule::SbJScriptModule( const OUString& rName )
    :SbModule( rName )
{
}

sal_Bool SbJScriptModule::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    (void)nVer;

    Clear();
    if( !SbxObject::LoadData( rStrm, 1 ) )
        return sal_False;

    // Get the source string
    aOUSource = rStrm.ReadUniOrByteString( osl_getThreadTextEncoding() );
    return sal_True;
}

sal_Bool SbJScriptModule::StoreData( SvStream& rStrm ) const
{
    if( !SbxObject::StoreData( rStrm ) )
        return sal_False;

    // Write the source string
    OUString aTmp = aOUSource;
    rStrm.WriteUniOrByteString( aTmp, osl_getThreadTextEncoding() );
    return sal_True;
}


/////////////////////////////////////////////////////////////////////////

SbMethod::SbMethod( const OUString& r, SbxDataType t, SbModule* p )
        : SbxMethod( r, t ), pMod( p )
{
    bInvalid     = sal_True;
    nStart       =
    nDebugFlags  =
    nLine1       =
    nLine2       = 0;
    refStatics = new SbxArray;
    mCaller          = 0;
    // HACK due to 'Referenz could not be saved'
    SetFlag( SBX_NO_MODIFY );
}

SbMethod::SbMethod( const SbMethod& r )
    : SvRefBase( r ), SbxMethod( r )
{
    pMod         = r.pMod;
    bInvalid     = r.bInvalid;
    nStart       = r.nStart;
    nDebugFlags  = r.nDebugFlags;
    nLine1       = r.nLine1;
    nLine2       = r.nLine2;
        refStatics = r.refStatics;
    mCaller          = r.mCaller;
    SetFlag( SBX_NO_MODIFY );
}

SbMethod::~SbMethod()
{
}

void SbMethod::ClearStatics()
{
    refStatics = new SbxArray;

}
SbxArray* SbMethod::GetStatics()
{
    return refStatics;
}

sal_Bool SbMethod::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    if( !SbxMethod::LoadData( rStrm, 1 ) )
        return sal_False;
    sal_Int16 n;
    rStrm >> n;
    sal_Int16 nTempStart = (sal_Int16)nStart;
    if( nVer == 2 )
        rStrm >> nLine1 >> nLine2 >> nTempStart >> bInvalid;
    // HACK ue to 'Referenz could not be saved'
    SetFlag( SBX_NO_MODIFY );
    nStart = nTempStart;
    return sal_True;
}

sal_Bool SbMethod::StoreData( SvStream& rStrm ) const
{
    if( !SbxMethod::StoreData( rStrm ) )
        return sal_False;
    rStrm << (sal_Int16) nDebugFlags
          << (sal_Int16) nLine1
          << (sal_Int16) nLine2
          << (sal_Int16) nStart
          << (sal_uInt8)  bInvalid;
    return sal_True;
}

void SbMethod::GetLineRange( sal_uInt16& l1, sal_uInt16& l2 )
{
    l1 = nLine1; l2 = nLine2;
}

// Could later be deleted

SbxInfo* SbMethod::GetInfo()
{
    return pInfo;
}

// Interface to execute a method of the applications
// With special RefCounting, so that the Basic was not fired of by CloseDocument()
// The return value will be delivered as string.
ErrCode SbMethod::Call( SbxValue* pRet, SbxVariable* pCaller )
{
    if ( pCaller )
    {
        SAL_INFO("basic", "SbMethod::Call Have been passed a caller 0x" << pCaller );
        mCaller = pCaller;
    }
    // RefCount vom Modul hochzaehlen
    SbModule* pMod_ = (SbModule*)GetParent();
    pMod_->AddRef();

    // Increment the RefCount of the Basic
    StarBASIC* pBasic = (StarBASIC*)pMod_->GetParent();
    pBasic->AddRef();

    // Establish the values to get the return value
    SbxValues aVals;
    aVals.eType = SbxVARIANT;

    // #104083: Compile BEFORE get
    if( bInvalid && !pMod_->Compile() )
        StarBASIC::Error( SbERR_BAD_PROP_VALUE );

    Get( aVals );
    if ( pRet )
        pRet->Put( aVals );

    // Was there an error
    ErrCode nErr = SbxBase::GetError();
    SbxBase::ResetError();

    // Release objects
    pMod_->ReleaseRef();
    pBasic->ReleaseRef();
    mCaller = 0;
    return nErr;
}


// #100883 Own Broadcast for SbMethod
void SbMethod::Broadcast( sal_uIntPtr nHintId )
{
    if( pCst && !IsSet( SBX_NO_BROADCAST ) )
    {
        // Because the method could be called from outside, test here once again
        // the authorisation
        if( nHintId & SBX_HINT_DATAWANTED )
            if( !CanRead() )
                return;
        if( nHintId & SBX_HINT_DATACHANGED )
            if( !CanWrite() )
                return;

        if( pMod && !pMod->IsCompiled() )
            pMod->Compile();

        // Block broadcasts while creating new method
        SfxBroadcaster* pSave = pCst;
        pCst = NULL;
        SbMethod* pThisCopy = new SbMethod( *this );
        SbMethodRef xHolder = pThisCopy;
        if( mpPar.Is() )
        {
            // Enrigister this as element 0, but don't reset the parent!
            if( GetType() != SbxVOID )
                mpPar->PutDirect( pThisCopy, 0 );
               SetParameters( NULL );
        }

        pCst = pSave;
        pSave->Broadcast( SbxHint( nHintId, pThisCopy ) );

        sal_uInt16 nSaveFlags = GetFlags();
        SetFlag( SBX_READWRITE );
        pCst = NULL;
        Put( pThisCopy->GetValues_Impl() );
        pCst = pSave;
        SetFlags( nSaveFlags );
    }
}


// Implementation of SbJScriptMethod (method class as a wrapper for JavaScript-functions)

SbJScriptMethod::SbJScriptMethod( const OUString& r, SbxDataType t, SbModule* p )
        : SbMethod( r, t, p )
{
}

SbJScriptMethod::~SbJScriptMethod()
{}


SbObjModule::SbObjModule( const OUString& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsVbaCompatible )
    : SbModule( rName, bIsVbaCompatible )
{
    SetModuleType( mInfo.ModuleType );
    if ( mInfo.ModuleType == script::ModuleType::FORM )
    {
        SetClassName( "Form" );
    }
    else if ( mInfo.ModuleObject.is() )
    {
        SetUnoObject( uno::makeAny( mInfo.ModuleObject ) );
    }
}

SbObjModule::~SbObjModule()
{
}

void
SbObjModule::SetUnoObject( const uno::Any& aObj ) throw ( uno::RuntimeException )
{
    SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,(SbxVariable*)pDocObject);
    if ( pUnoObj && pUnoObj->getUnoAny() == aObj ) // object is equal, nothing to do
        return;
    pDocObject = new SbUnoObject( GetName(), uno::makeAny( aObj ) );

    com::sun::star::uno::Reference< com::sun::star::lang::XServiceInfo > xServiceInfo( aObj, com::sun::star::uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( "ooo.vba.excel.Worksheet" ) )
    {
        SetClassName( "Worksheet" );
    }
    else if( xServiceInfo->supportsService( "ooo.vba.excel.Workbook" ) )
    {
        SetClassName( "Workbook" );
    }
}

SbxVariable*
SbObjModule::GetObject()
{
    return pDocObject;
}
SbxVariable*
SbObjModule::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* pVar = NULL;
    if ( pDocObject)
        pVar = pDocObject->Find( rName, t );
    if ( !pVar )
        pVar = SbModule::Find( rName, t );
    return pVar;
}

void SbObjModule::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType )
{
    SbModule::handleProcedureProperties( rBC, rHint );
}


typedef ::cppu::WeakImplHelper3<
    awt::XTopWindowListener,
    awt::XWindowListener,
    document::XEventListener > FormObjEventListener_BASE;

class FormObjEventListenerImpl : public FormObjEventListener_BASE
{
    SbUserFormModule* mpUserForm;
    uno::Reference< lang::XComponent > mxComponent;
    uno::Reference< frame::XModel > mxModel;
    bool mbDisposed;
    sal_Bool mbOpened;
    sal_Bool mbActivated;
    sal_Bool mbShowing;

    FormObjEventListenerImpl(const FormObjEventListenerImpl&); // not defined
    FormObjEventListenerImpl& operator=(const FormObjEventListenerImpl&); // not defined

public:
    FormObjEventListenerImpl( SbUserFormModule* pUserForm, const uno::Reference< lang::XComponent >& xComponent, const uno::Reference< frame::XModel >& xModel ) :
        mpUserForm( pUserForm ), mxComponent( xComponent), mxModel( xModel ),
        mbDisposed( false ), mbOpened( sal_False ), mbActivated( sal_False ), mbShowing( sal_False )
    {
        if ( mxComponent.is() )
        {
            SAL_INFO("basic", "*********** Registering the listeners");
            try
            {
                uno::Reference< awt::XTopWindow >( mxComponent, uno::UNO_QUERY_THROW )->addTopWindowListener( this );
            }
            catch(const uno::Exception& ) {}
            try
            {
                uno::Reference< awt::XWindow >( mxComponent, uno::UNO_QUERY_THROW )->addWindowListener( this );
            }
            catch(const uno::Exception& ) {}
        }

        if ( mxModel.is() )
        {
            try
            {
                uno::Reference< document::XEventBroadcaster >( mxModel, uno::UNO_QUERY_THROW )->addEventListener( this );
            }
            catch(const uno::Exception& ) {}
        }
    }

    virtual ~FormObjEventListenerImpl()
    {
        removeListener();
    }

    sal_Bool isShowing() const { return mbShowing; }

    void removeListener()
    {
        if ( mxComponent.is() && !mbDisposed )
        {
            SAL_INFO("basic", "*********** Removing the listeners");
            try
            {
                uno::Reference< awt::XTopWindow >( mxComponent, uno::UNO_QUERY_THROW )->removeTopWindowListener( this );
            }
            catch(const uno::Exception& ) {}
            try
            {
                uno::Reference< awt::XWindow >( mxComponent, uno::UNO_QUERY_THROW )->removeWindowListener( this );
            }
            catch(const uno::Exception& ) {}
        }
        mxComponent.clear();

        if ( mxModel.is() && !mbDisposed )
        {
            try
            {
                uno::Reference< document::XEventBroadcaster >( mxModel, uno::UNO_QUERY_THROW )->removeEventListener( this );
            }
            catch(const uno::Exception& ) {}
        }
        mxModel.clear();
    }

    virtual void SAL_CALL windowOpened( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
        if ( mpUserForm )
        {
            mbOpened = sal_True;
            mbShowing = sal_True;
            if ( mbActivated )
            {
                mbOpened = mbActivated = sal_False;
                mpUserForm->triggerActivateEvent();
            }
        }
    }


    virtual void SAL_CALL windowClosing( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
#ifdef IN_THE_FUTURE
        uno::Reference< awt::XDialog > xDialog( e.Source, uno::UNO_QUERY );
        if ( xDialog.is() )
        {
            uno::Reference< awt::XControl > xControl( xDialog, uno::UNO_QUERY );
            if ( xControl->getPeer().is() )
            {
                uno::Reference< document::XVbaMethodParameter > xVbaMethodParameter( xControl->getPeer(), uno::UNO_QUERY );
                if ( xVbaMethodParameter.is() )
                {
                    sal_Int8 nCancel = 0;
                    sal_Int8 nCloseMode = ::ooo::vba::VbQueryClose::vbFormControlMenu;

                    Sequence< Any > aParams;
                    aParams.realloc(2);
                    aParams[0] <<= nCancel;
                    aParams[1] <<= nCloseMode;

                    mpUserForm->triggerMethod( "Userform_QueryClose", aParams);
                    return;

                }
            }
        }

        mpUserForm->triggerMethod( "Userform_QueryClose" );
#endif
    }


    virtual void SAL_CALL windowClosed( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
        mbOpened = sal_False;
        mbShowing = sal_False;
    }

    virtual void SAL_CALL windowMinimized( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL windowNormalized( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL windowActivated( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
        if ( mpUserForm )
        {
            mbActivated = sal_True;
            if ( mbOpened )
            {
                mbOpened = mbActivated = sal_False;
                mpUserForm->triggerActivateEvent();
            }
        }
    }

    virtual void SAL_CALL windowDeactivated( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
        if ( mpUserForm )
            mpUserForm->triggerDeactivateEvent();
    }

    virtual void SAL_CALL windowResized( const awt::WindowEvent& /*e*/ ) throw (uno::RuntimeException)
    {
        if ( mpUserForm )
        {
            mpUserForm->triggerResizeEvent();
            mpUserForm->triggerLayoutEvent();
        }
    }

    virtual void SAL_CALL windowMoved( const awt::WindowEvent& /*e*/ ) throw (uno::RuntimeException)
    {
        if ( mpUserForm )
            mpUserForm->triggerLayoutEvent();
    }

    virtual void SAL_CALL windowShown( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL windowHidden( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL notifyEvent( const document::EventObject& rEvent ) throw (uno::RuntimeException)
    {
        // early dosposing on document event "OnUnload", to be sure Basic still exists when calling VBA "UserForm_Terminate"
        if( rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_CLOSEDOC ) )
        {
            removeListener();
            mbDisposed = true;
            if ( mpUserForm )
                mpUserForm->ResetApiObj();   // will trigger "UserForm_Terminate"
        }
    }

    virtual void SAL_CALL disposing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException)
    {
        SAL_INFO("basic", "** Userform/Dialog disposing");
        removeListener();
        mbDisposed = true;
        if ( mpUserForm )
            mpUserForm->ResetApiObj( false );   // pass false (too late to trigger VBA events here)
    }
};

SbUserFormModule::SbUserFormModule( const OUString& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsCompat )
    : SbObjModule( rName, mInfo, bIsCompat )
    , m_mInfo( mInfo )
    , mbInit( false )
{
    m_xModel.set( mInfo.ModuleObject, uno::UNO_QUERY_THROW );
}

SbUserFormModule::~SbUserFormModule()
{
}

void SbUserFormModule::ResetApiObj(  bool bTriggerTerminateEvent )
{
    SAL_INFO("basic", " SbUserFormModule::ResetApiObj( " << (bTriggerTerminateEvent ? "true )" : "false )") );
    if ( bTriggerTerminateEvent && m_xDialog.is() ) // probably someone close the dialog window
    {
        triggerTerminateEvent();
    }
    pDocObject = NULL;
    m_xDialog = NULL;
}

void SbUserFormModule::triggerMethod( const OUString& aMethodToRun )
{
    Sequence< Any > aArguments;
    triggerMethod( aMethodToRun, aArguments );
}

void SbUserFormModule::triggerMethod( const OUString& aMethodToRun, Sequence< Any >& aArguments )
{
    SAL_INFO("basic", "*** trigger " << OUStringToOString( aMethodToRun, RTL_TEXTENCODING_UTF8 ).getStr() << " ***");
    // Search method
    SbxVariable* pMeth = SbObjModule::Find( aMethodToRun, SbxCLASS_METHOD );
    if( pMeth )
    {
        if ( aArguments.getLength() > 0 )   // Setup parameters
        {
            SbxArrayRef xArray = new SbxArray;
            xArray->Put( pMeth, 0 );    // Method as parameter 0

            for ( sal_Int32 i = 0; i < aArguments.getLength(); ++i )
            {
                SbxVariableRef xSbxVar = new SbxVariable( SbxVARIANT );
                unoToSbxValue( static_cast< SbxVariable* >( xSbxVar ), aArguments[i] );
                xArray->Put( xSbxVar, static_cast< sal_uInt16 >( i ) + 1 );

                // Enable passing by ref
                if ( xSbxVar->GetType() != SbxVARIANT )
                    xSbxVar->SetFlag( SBX_FIXED );
            }
            pMeth->SetParameters( xArray );

            SbxValues aVals;
            pMeth->Get( aVals );

            for ( sal_Int32 i = 0; i < aArguments.getLength(); ++i )
            {
                aArguments[i] = sbxToUnoValue( xArray->Get( static_cast< sal_uInt16 >(i) + 1) );
            }
            pMeth->SetParameters( NULL );
        }
        else
        {
            SbxValues aVals;
            pMeth->Get( aVals );
        }
    }
}

void SbUserFormModule::triggerActivateEvent( void )
{
    SAL_INFO("basic", "**** entering SbUserFormModule::triggerActivate");
    triggerMethod( "UserForm_Activate" );
    SAL_INFO("basic", "**** leaving SbUserFormModule::triggerActivate");
}

void SbUserFormModule::triggerDeactivateEvent( void )
{
    SAL_INFO("basic", "**** SbUserFormModule::triggerDeactivate");
    triggerMethod( "Userform_Deactivate" );
}

void SbUserFormModule::triggerInitializeEvent( void )
{
    if ( mbInit )
        return;
    SAL_INFO("basic", "**** SbUserFormModule::triggerInitializeEvent");
    static OUString aInitMethodName( "Userform_Initialize");
    triggerMethod( aInitMethodName );
    mbInit = true;
}

void SbUserFormModule::triggerTerminateEvent( void )
{
    SAL_INFO("basic", "**** SbUserFormModule::triggerTerminateEvent");
    static OUString aTermMethodName( "Userform_Terminate" );
    triggerMethod( aTermMethodName );
    mbInit=false;
}

void SbUserFormModule::triggerLayoutEvent( void )
{
    static OUString aMethodName( "Userform_Layout" );
    triggerMethod( aMethodName );
}

void SbUserFormModule::triggerResizeEvent( void )
{
    static OUString aMethodName("Userform_Resize");
    triggerMethod( aMethodName );
}

SbUserFormModuleInstance* SbUserFormModule::CreateInstance()
{
    SbUserFormModuleInstance* pInstance = new SbUserFormModuleInstance( this, GetName(), m_mInfo, IsVBACompat() );
    return pInstance;
}

SbUserFormModuleInstance::SbUserFormModuleInstance( SbUserFormModule* pParentModule,
    const OUString& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsVBACompat )
        : SbUserFormModule( rName, mInfo, bIsVBACompat )
        , m_pParentModule( pParentModule )
{
}

sal_Bool SbUserFormModuleInstance::IsClass( const OUString& rName ) const
{
    sal_Bool bParentNameMatches = m_pParentModule->GetName().equalsIgnoreAsciiCase( rName );
    sal_Bool bRet = bParentNameMatches || SbxObject::IsClass( rName );
    return bRet;
}

SbxVariable* SbUserFormModuleInstance::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* pVar = m_pParentModule->Find( rName, t );
    return pVar;
}


void SbUserFormModule::Load()
{
    SAL_INFO("basic", "** load() ");
    // forces a load
    if ( !pDocObject )
        InitObject();
}


void SbUserFormModule::Unload()
{
    SAL_INFO("basic", "** Unload() ");

    sal_Int8 nCancel = 0;
    sal_Int8 nCloseMode = ::ooo::vba::VbQueryClose::vbFormCode;

    Sequence< Any > aParams;
    aParams.realloc(2);
    aParams[0] <<= nCancel;
    aParams[1] <<= nCloseMode;

    triggerMethod( "Userform_QueryClose", aParams);

    aParams[0] >>= nCancel;
    // basic boolean ( and what the user might use ) can be ambiguous ( e.g. basic true = -1 )
    // test agains 0 ( false ) and assume anything else is true
    // ( Note: ) this used to work ( something changes somewhere )
    if (nCancel != 0)
    {
        return;
    }

    if ( m_xDialog.is() )
    {
        triggerTerminateEvent();
    }
    // Search method
    SbxVariable* pMeth = SbObjModule::Find( "UnloadObject", SbxCLASS_METHOD );
    if( pMeth )
    {
        SAL_INFO("basic", "Attempting too run the UnloadObjectMethod");
        m_xDialog.clear(); //release ref to the uno object
        SbxValues aVals;
        bool bWaitForDispose = true; // assume dialog is showing
        if ( m_DialogListener.get() )
        {
            bWaitForDispose = m_DialogListener->isShowing();
            SAL_INFO("basic", "Showing " << bWaitForDispose );
        }
        pMeth->Get( aVals);
        if ( !bWaitForDispose )
        {
            // we've either already got a dispose or we'er never going to get one
            ResetApiObj();
        } // else wait for dispose
        SAL_INFO("basic", "UnloadObject completed ( we hope )");
    }
}


void registerComponentToBeDisposedForBasic( Reference< XComponent > xComponent, StarBASIC* pBasic );

void SbUserFormModule::InitObject()
{
    try
    {
        OUString aHook("VBAGlobals");
        SbUnoObject* pGlobs = (SbUnoObject*)GetParent()->Find( aHook, SbxCLASS_DONTCARE );
        if ( m_xModel.is() && pGlobs )
        {
            // broadcast INITIALIZE_USERFORM script event before the dialog is created
            Reference< script::vba::XVBACompatibility > xVBACompat( getVBACompatibility( m_xModel ), uno::UNO_SET_THROW );
            xVBACompat->broadcastVBAScriptEvent( script::vba::VBAScriptEventId::INITIALIZE_USERFORM, GetName() );
            uno::Reference< lang::XMultiServiceFactory > xVBAFactory( pGlobs->getUnoAny(), uno::UNO_QUERY_THROW );
            uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
            OUString sDialogUrl( "vnd.sun.star.script:"  );
            OUString sProjectName( "Standard" );

            try
            {
                Reference< beans::XPropertySet > xProps( m_xModel, UNO_QUERY_THROW );
                uno::Reference< script::vba::XVBACompatibility > xVBAMode( xProps->getPropertyValue( "BasicLibraries" ), uno::UNO_QUERY_THROW );
                sProjectName = xVBAMode->getProjectName();
            }
            catch(const Exception& ) {}

            sDialogUrl = sDialogUrl + sProjectName + "." + GetName() + "?location=document";

            uno::Reference< awt::XDialogProvider > xProvider = awt::DialogProvider::createWithModel( xContext, m_xModel  );
            m_xDialog = xProvider->createDialog( sDialogUrl );

            // create vba api object
            uno::Sequence< uno::Any > aArgs(4);
            aArgs[ 0 ] = uno::Any();
            aArgs[ 1 ] <<= m_xDialog;
            aArgs[ 2 ] <<= m_xModel;
            aArgs[ 3 ] <<= OUString( GetParent()->GetName() );
            pDocObject = new SbUnoObject( GetName(), uno::makeAny( xVBAFactory->createInstanceWithArguments( "ooo.vba.msforms.UserForm", aArgs  ) ) );

            uno::Reference< lang::XComponent > xComponent( m_xDialog, uno::UNO_QUERY_THROW );

            // the dialog must be disposed at the end!
            StarBASIC* pParentBasic = NULL;
            SbxObject* pCurObject = this;
            do
            {
                SbxObject* pObjParent = pCurObject->GetParent();
                pParentBasic = PTR_CAST( StarBASIC, pObjParent );
                pCurObject = pObjParent;
            }
            while( pParentBasic == NULL && pCurObject != NULL );

            SAL_WARN_IF( pParentBasic == NULL, "basic", "pParentBasic == NULL" );
            registerComponentToBeDisposedForBasic( xComponent, pParentBasic );

            // if old listener object exists, remove it from dialog and document model
            if( m_DialogListener.is() )
                m_DialogListener->removeListener();
            m_DialogListener.set( new FormObjEventListenerImpl( this, xComponent, m_xModel ) );

            triggerInitializeEvent();
        }
    }
    catch(const uno::Exception& )
    {
    }

}

SbxVariable*
SbUserFormModule::Find( const OUString& rName, SbxClassType t )
{
    if ( !pDocObject && !GetSbData()->bRunInit && GetSbData()->pInst )
        InitObject();
    return SbObjModule::Find( rName, t );
}

SbProperty::SbProperty( const OUString& r, SbxDataType t, SbModule* p )
        : SbxProperty( r, t ), pMod( p )
{
    bInvalid = sal_False;
}

SbProperty::~SbProperty()
{}


SbProcedureProperty::~SbProcedureProperty()
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
