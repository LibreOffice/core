/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <list>

#include <vcl/svapp.hxx>
#include <tools/stream.hxx>
#include <svl/brdcst.hxx>
#include <tools/shl.hxx>
#include <basic/sbx.hxx>
#include <basic/sbuno.hxx>
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

#include <sal/log.hxx>

#include <basic/basrdll.hxx>
#include <osl/mutex.hxx>
#include "sbobjmod.hxx"
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

class DocObjectWrapper : public DocObjectWrapper_BASE
{
    Reference< XAggregation >  m_xAggProxy;
    Reference< XInvocation >  m_xAggInv;
    Reference< XTypeProvider > m_xAggregateTypeProv;
    Sequence< Type >           m_Types;
    SbModule*                m_pMod;
    SbMethodRef getMethod( const OUString& aName ) throw (RuntimeException);
    SbPropertyRef getProperty( const OUString& aName ) throw (RuntimeException);
    OUString mName; 

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
                *pPtr = cppu::UnoType<XInvocation>::get();
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

            
            if ( xSbxVar->GetType() != SbxVARIANT )
                xSbxVar->SetFlag( SBX_FIXED );
        }
    }
    if ( xSbxParams.Is() )
        pMethod->SetParameters( xSbxParams );

    
    SbxVariableRef xReturn = new SbxVariable;

    pMethod->Call( xReturn );
    Any aReturn;
    
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
        
        m_pMod->ResetFlag( SBX_GBLSEARCH );
        pMethod = dynamic_cast<SbMethod*>(m_pMod->SbModule::Find(aName,  SbxCLASS_METHOD));
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
        
        m_pMod->ResetFlag( SBX_GBLSEARCH );
        pProperty = dynamic_cast<SbProperty*>(m_pMod->SbModule::Find(aName,  SbxCLASS_PROPERTY));
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




SbModule::SbModule( const OUString& rName,  sal_Bool bVBACompat )
         : SbxObject( "StarBASICModule" ),
           pImage( NULL ), pBreaks( NULL ), pClassData( NULL ), mbVBACompat( bVBACompat ),  pDocObject( NULL ), bIsProxyModule( false )
{
    SetName( rName );
    SetFlag( SBX_EXTSEARCH | SBX_GBLSEARCH );
    SetModuleType( script::ModuleType::NORMAL );

    
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




void SbModule::StartDefinitions()
{
    delete pImage; pImage = NULL;
    if( pClassData )
        pClassData->clear();

    
    
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
        StartListening( pMeth->GetBroadcaster(), true );
    }
    
    
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
        StartListening( pProp->GetBroadcaster(), true );
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
        StartListening( pProp->GetBroadcaster(), true );
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




void SbModule::EndDefinitions( sal_Bool bNewState )
{
    for( sal_uInt16 i = 0; i < pMethods->Count(); )
    {
        SbMethod* p = PTR_CAST(SbMethod,pMethods->Get( i ) );
        if( p )
        {
            if( p->bInvalid )
            {
                pMethods->Remove( p );
            }
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
                        xMethParameters->Put( pMethVar, 0 );    
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
                if( !pMethVar ) 
                {
                    OUString aProcName("Property Let " );
                    aProcName += pProcProperty->GetName();
                    pMethVar = Find( aProcName, SbxCLASS_METHOD );
                }

                if( pMethVar )
                {
                    
                    SbxArrayRef xArray = new SbxArray;
                    xArray->Put( pMethVar, 0 ); 
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
                    
                    StarBASIC::Error( SbERR_BAD_PROP_VALUE );
                }
                else
                {
                    
                    SbModule* pOld = GetSbData()->pMod;
                    GetSbData()->pMod = this;
                    Run( (SbMethod*) pVar );
                    GetSbData()->pMod = pOld;
                }
            }
        }
        else
        {
            
            
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




void SbModule::SetSource( const OUString& r )
{
    SetSource32( r );
}

void SbModule::SetSource32( const OUString& r )
{
    
    SetVBACompat( getDefaultVBAMode( static_cast< StarBASIC*>( GetParent() ) ) );
    aOUSource = r;
    StartDefinitions();
    SbiTokenizer aTok( r );
    aTok.SetCompatible( IsVBACompat() );

    while( !aTok.IsEof() )
    {
        SbiToken eEndTok = NIL;

        
        SbiToken eLastTok = NIL;
        while( !aTok.IsEof() )
        {
            
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
                
                pMeth->bInvalid = sal_False;
            }
            else
            {
                eEndTok = NIL;
            }
        }
        
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



static void _SendHint( SbxObject* pObj, sal_uIntPtr nId, SbMethod* p )
{
    
    if( pObj->IsA( TYPE(StarBASIC) ) && pObj->IsBroadcaster() )
        pObj->GetBroadcaster().Broadcast( SbxHint( nId, p ) );
    
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



void ClearUnoObjectsInRTL_Impl_Rek( StarBASIC* pBasic )
{
    
    static OUString aName("CreateUnoService");
    SbxVariable* pVar = pBasic->GetRtl()->Find( aName, SbxCLASS_METHOD );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    
    static OUString aName2("CreateUnoDialog");
    pVar = pBasic->GetRtl()->Find( aName2, SbxCLASS_METHOD );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    
    static OUString aName3("CDec");
    pVar = pBasic->GetRtl()->Find( aName3, SbxCLASS_METHOD );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    
    static OUString aName4("CreateObject");
    pVar = pBasic->GetRtl()->Find( aName4, SbxCLASS_METHOD );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    
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
    
    clearUnoMethods();
    clearUnoServiceCtors();

    ClearUnoObjectsInRTL_Impl_Rek( pBasic );

    
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

        
        
        bool bWasError = SbxBase::GetError() != 0;
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
                    pMSOMacroRuntimeLib->SetFlag( SBX_EXTSEARCH );      
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
          
          
          nMaxCallLevel = rl.rlim_cur / 900;
#elif defined SOLARIS
          
          
          nMaxCallLevel = rl.rlim_cur / 1650;
#elif defined WIN32
          nMaxCallLevel = 5800;
#else
          nMaxCallLevel = MAXRECURSION;
#endif
        }
    }

    
    if( ++GetSbData()->pInst->nCallLvl <= nMaxCallLevel )
    {
        
        GlobalRunInit( /* bBasicStart = */ bDelInst );

        
        if( !GetSbData()->bGlobalInitErr )
        {
            if( bDelInst )
            {
                SendHint( GetParent(), SBX_HINT_BASICSTART, pMeth );

                
                
                
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

            
            
            
            
            
            
            
            if( bDelInst )
            {
                
                while( GetSbData()->pInst->nCallLvl != 1 )
                    GetpApp()->Yield();
            }

            nRes = sal_True;
            GetSbData()->pInst->pRun = pRt->pNext;
            GetSbData()->pInst->nCallLvl--;          

            
            
            SbiRuntime* pRtNext = pRt->pNext;
            if( pRtNext && (pRt->GetDebugFlags() & SbDEBUG_BREAK) )
                pRtNext->SetDebugFlags( SbDEBUG_BREAK );

            delete pRt;
            GetSbData()->pMod = pOldMod;
            if( bDelInst )
            {
                
                
                ClearUnoObjectsInRTL_Impl( xBasic );

                clearNativeObjectWrapperVector();

                SAL_WARN_IF(GetSbData()->pInst->nCallLvl != 0,"basic","BASIC-Call-Level > 0");
                delete GetSbData()->pInst, GetSbData()->pInst = NULL, bDelInst = false;

                
                SolarMutexGuard aSolarGuard;
                SendHint( GetParent(), SBX_HINT_BASICSTOP, pMeth );

                GlobalRunDeInit();

#ifdef DBG_UTIL
                ResetCapturedAssertions();
#endif

                if( xVBACompat.is() )
                {
                    
                    try
                    {
                        xVBACompat->broadcastVBAScriptEvent( script::vba::VBAScriptEventId::SCRIPT_STOPPED, GetName() );
                    }
                    catch(const uno::Exception& )
                    {
                    }
                    
                    ::basic::vba::lockControllersOfAllDocuments( xModel, sal_False );
                    ::basic::vba::enableContainerWindowsOfAllDocuments( xModel, sal_True );
                }

#ifdef DBG_TRACE_BASIC
                dbg_DeInitTrace();
#endif
            }
        }
        else
               GetSbData()->pInst->nCallLvl--;           
    }
    else
    {
        GetSbData()->pInst->nCallLvl--;          
        StarBASIC::FatalError( SbERR_STACK_OVERFLOW );
    }

    StarBASIC* pBasic = PTR_CAST(StarBASIC,GetParent());
    if( bDelInst )
    {
       
       
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




void SbModule::RunInit()
{
    if( pImage
     && !pImage->bInit
     && pImage->GetFlag( SBIMG_INITCODE ) )
    {
        
        GetSbData()->bRunInit = true;

        SbModule* pOldMod = GetSbData()->pMod;
        GetSbData()->pMod = this;
        
        SbiRuntime* pRt = new SbiRuntime( this, NULL, 0 );

        pRt->pNext = GetSbData()->pInst->pRun;
        GetSbData()->pInst->pRun = pRt;
        while( pRt->Step() ) {}

        GetSbData()->pInst->pRun = pRt->pNext;
        delete pRt;
        GetSbData()->pMod = pOldMod;
        pImage->bInit = true;
        pImage->bFirstInit = false;

        
        GetSbData()->bRunInit = false;
    }
}



void SbModule::AddVarName( const OUString& aName )
{
    
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
    
    for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = (SbModule*)pModules->Get( nMod );
        
        if( pModule->pImage && pModule->pImage->bInit && !pModule->isProxyModule() && !pModule->ISA(SbObjModule) )
            pModule->ClearPrivateVars();
    }

}


void SbModule::GlobalRunInit( bool bBasicStart )
{
    
    if( !bBasicStart )
        if( !(pImage && !pImage->bInit) )
            return;

    
    
    
    
    GetSbData()->bGlobalInitErr = false;

    
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
    
    SetFlag( SBX_EXTSEARCH | SBX_GBLSEARCH );
    sal_uInt8 bImage;
    rStrm.ReadUChar( bImage );
    if( bImage )
    {
        SbiImage* p = new SbiImage;
        sal_uInt32 nImgVer = 0;

        if( !p->Load( rStrm, nImgVer ) )
        {
            delete p;
            return sal_False;
        }
        
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
        rStrm.WriteUChar( (sal_uInt8) 1 );
        
        
        
        
        bool bRes = pImage->Save( rStrm, B_LEGACYVERSION );
        if ( bFixup )
            fixUpMethodStart( false ); 
        return bRes;

    }
    else
    {
        SbiImage aImg;
        aImg.aOUSource = aOUSource;
        aImg.aComment = aComment;
        aImg.aName = GetName();
        rStrm.WriteUChar( (sal_uInt8) 1 );
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
        
        mErrHandler = StarBASIC::GetGlobalErrorHdl();
        
        StarBASIC::SetGlobalErrorHdl( LINK( this, ErrorHdlResetter, BasicErrorHdl ) );
    }
    ~ErrorHdlResetter()
    {
        
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

void SbModule::GetCodeCompleteDataFromParse(CodeCompleteDataCache& aCache)
{
    ErrorHdlResetter aErrHdl;
    SbxBase::ResetError();

    SbiParser* pParser = new SbiParser( (StarBASIC*) GetParent(), this );
    pParser->SetCodeCompleting(true);

    while( pParser->Parse() ) {}
    SbiSymPool* pPool = pParser->pPool;
    aCache.Clear();
    for( sal_uInt16 i = 0; i < pPool->GetSize(); ++i )
    {
        SbiSymDef* pSymDef = pPool->Get(i);
        
        if( (pSymDef->GetType() != SbxEMPTY) || (pSymDef->GetType() != SbxNULL) )
            aCache.InsertGlobalVar( pSymDef->GetName(), pParser->aGblStrings.Find(pSymDef->GetTypeId()) );

        SbiSymPool& pChildPool = pSymDef->GetPool();
        for(sal_uInt16 j = 0; j < pChildPool.GetSize(); ++j )
        {
            SbiSymDef* pChildSymDef = pChildPool.Get(j);
            
            if( (pChildSymDef->GetType() != SbxEMPTY) || (pChildSymDef->GetType() != SbxNULL) )
                aCache.InsertLocalVar( pSymDef->GetName(), pChildSymDef->GetName(), pParser->aGblStrings.Find(pChildSymDef->GetTypeId()) );
        }
    }
    delete pParser;
}

SbxArrayRef SbModule::GetMethods()
{
    return pMethods;
}

OUString SbModule::GetKeywordCase( const OUString& sKeyword ) const
{
    return SbiParser::GetKeywordCase( sKeyword );
}

bool SbModule::HasExeCode()
{
    
    static const unsigned char pEmptyImage[] = { 0x45, 0x0 , 0x0, 0x0, 0x0 };
    

    if (!IsCompiled())
    {
        ErrorHdlResetter aGblErrHdl;
        Compile();
        if (aGblErrHdl.HasError()) 
            return true;
    }

    bool bRes = false;
    if (pImage && !(pImage->GetCodeSize() == 5 && (memcmp(pImage->GetCode(), pEmptyImage, pImage->GetCodeSize()) == 0 )))
        bRes = true;

    return bRes;
}


sal_Bool SbModule::StoreBinaryData( SvStream& rStrm )
{
    return StoreBinaryData( rStrm, 0 );
}

sal_Bool SbModule::StoreBinaryData( SvStream& rStrm, sal_uInt16 nVer )
{
    sal_Bool bRet = Compile();
    if( bRet )
    {
        bool bFixup = ( !nVer && !pImage->ExceedsLegacyLimits() );

        if ( bFixup ) 
            fixUpMethodStart( true );
         bRet = SbxObject::StoreData( rStrm );
        if( bRet )
        {
            pImage->aOUSource = OUString();
            pImage->aComment = aComment;
            pImage->aName = GetName();

            rStrm.WriteUChar( (sal_uInt8) 1 );
                    if ( nVer )
                        bRet = pImage->Save( rStrm, B_EXT_IMG_VERSION );
                    else
                        bRet = pImage->Save( rStrm, B_LEGACYVERSION );
                    if ( bFixup )
                        fixUpMethodStart( false ); 

            pImage->aOUSource = aOUSource;
        }
    }
    return bRet;
}



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
                        xMethParameters->Put( pMeth, 0 );   
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
                if( !pMeth )    
                {
                    OUString aProcName("Property Let " );
                    aProcName += pProcProperty->GetName();
                    pMeth = Find( aProcName, SbxCLASS_METHOD );
                }

                if( pMeth )
                {
                    
                    SbxArrayRef xArray = new SbxArray;
                    xArray->Put( pMeth, 0 );    
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

    
    aOUSource = rStrm.ReadUniOrByteString( osl_getThreadTextEncoding() );
    return sal_True;
}

sal_Bool SbJScriptModule::StoreData( SvStream& rStrm ) const
{
    if( !SbxObject::StoreData( rStrm ) )
        return sal_False;

    
    OUString aTmp = aOUSource;
    rStrm.WriteUniOrByteString( aTmp, osl_getThreadTextEncoding() );
    return sal_True;
}




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
    rStrm.ReadInt16( n );
    sal_Int16 nTempStart = (sal_Int16)nStart;
    if( nVer == 2 )
        rStrm.ReadUInt16( nLine1 ).ReadUInt16( nLine2 ).ReadInt16( nTempStart ).ReadUChar( bInvalid );
    
    SetFlag( SBX_NO_MODIFY );
    nStart = nTempStart;
    return sal_True;
}

sal_Bool SbMethod::StoreData( SvStream& rStrm ) const
{
    if( !SbxMethod::StoreData( rStrm ) )
        return sal_False;
    rStrm.WriteInt16( (sal_Int16) nDebugFlags )
         .WriteInt16( (sal_Int16) nLine1 )
         .WriteInt16( (sal_Int16) nLine2 )
         .WriteInt16( (sal_Int16) nStart )
         .WriteUChar( (sal_uInt8)  bInvalid );
    return sal_True;
}

void SbMethod::GetLineRange( sal_uInt16& l1, sal_uInt16& l2 )
{
    l1 = nLine1; l2 = nLine2;
}



SbxInfo* SbMethod::GetInfo()
{
    return pInfo;
}




ErrCode SbMethod::Call( SbxValue* pRet, SbxVariable* pCaller )
{
    if ( pCaller )
    {
        SAL_INFO("basic", "SbMethod::Call Have been passed a caller 0x" << pCaller );
        mCaller = pCaller;
    }
    
    SbModule* pMod_ = (SbModule*)GetParent();
    pMod_->AddRef();

    
    StarBASIC* pBasic = (StarBASIC*)pMod_->GetParent();
    pBasic->AddRef();

    
    SbxValues aVals;
    aVals.eType = SbxVARIANT;

    
    if( bInvalid && !pMod_->Compile() )
        StarBASIC::Error( SbERR_BAD_PROP_VALUE );

    Get( aVals );
    if ( pRet )
        pRet->Put( aVals );

    
    ErrCode nErr = SbxBase::GetError();
    SbxBase::ResetError();

    
    pMod_->ReleaseRef();
    pBasic->ReleaseRef();
    mCaller = 0;
    return nErr;
}



void SbMethod::Broadcast( sal_uIntPtr nHintId )
{
    if( pCst && !IsSet( SBX_NO_BROADCAST ) )
    {
        
        
        if( nHintId & SBX_HINT_DATAWANTED )
            if( !CanRead() )
                return;
        if( nHintId & SBX_HINT_DATACHANGED )
            if( !CanWrite() )
                return;

        if( pMod && !pMod->IsCompiled() )
            pMod->Compile();

        
        SfxBroadcaster* pSave = pCst;
        pCst = NULL;
        SbMethod* pThisCopy = new SbMethod( *this );
        SbMethodRef xHolder = pThisCopy;
        if( mpPar.Is() )
        {
            
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
    if ( pUnoObj && pUnoObj->getUnoAny() == aObj ) 
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
    bool mbOpened;
    bool mbActivated;
    bool mbShowing;

    FormObjEventListenerImpl(const FormObjEventListenerImpl&); 
    FormObjEventListenerImpl& operator=(const FormObjEventListenerImpl&); 

public:
    FormObjEventListenerImpl( SbUserFormModule* pUserForm, const uno::Reference< lang::XComponent >& xComponent, const uno::Reference< frame::XModel >& xModel ) :
        mpUserForm( pUserForm ), mxComponent( xComponent), mxModel( xModel ),
        mbDisposed( false ), mbOpened( false ), mbActivated( false ), mbShowing( false )
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

    bool isShowing() const { return mbShowing; }

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
            mbOpened = true;
            mbShowing = true;
            if ( mbActivated )
            {
                mbOpened = mbActivated = false;
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
        mbOpened = false;
        mbShowing = false;
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
            mbActivated = true;
            if ( mbOpened )
            {
                mbOpened = mbActivated = false;
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
        
        if( rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_CLOSEDOC ) )
        {
            removeListener();
            mbDisposed = true;
            if ( mpUserForm )
                mpUserForm->ResetApiObj();   
        }
    }

    virtual void SAL_CALL disposing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException)
    {
        SAL_INFO("basic", "** Userform/Dialog disposing");
        removeListener();
        mbDisposed = true;
        if ( mpUserForm )
            mpUserForm->ResetApiObj( false );   
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
    if ( bTriggerTerminateEvent && m_xDialog.is() ) 
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
    
    SbxVariable* pMeth = SbObjModule::Find( aMethodToRun, SbxCLASS_METHOD );
    if( pMeth )
    {
        if ( aArguments.getLength() > 0 )   
        {
            SbxArrayRef xArray = new SbxArray;
            xArray->Put( pMeth, 0 );    

            for ( sal_Int32 i = 0; i < aArguments.getLength(); ++i )
            {
                SbxVariableRef xSbxVar = new SbxVariable( SbxVARIANT );
                unoToSbxValue( static_cast< SbxVariable* >( xSbxVar ), aArguments[i] );
                xArray->Put( xSbxVar, static_cast< sal_uInt16 >( i ) + 1 );

                
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
    
    
    
    if (nCancel != 0)
    {
        return;
    }

    if ( m_xDialog.is() )
    {
        triggerTerminateEvent();
    }
    
    SbxVariable* pMeth = SbObjModule::Find( "UnloadObject", SbxCLASS_METHOD );
    if( pMeth )
    {
        SAL_INFO("basic", "Attempting too run the UnloadObjectMethod");
        m_xDialog.clear(); 
        SbxValues aVals;
        bool bWaitForDispose = true; 
        if ( m_DialogListener.get() )
        {
            bWaitForDispose = m_DialogListener->isShowing();
            SAL_INFO("basic", "Showing " << bWaitForDispose );
        }
        pMeth->Get( aVals);
        if ( !bWaitForDispose )
        {
            
            ResetApiObj();
        } 
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

            
            uno::Sequence< uno::Any > aArgs(4);
            aArgs[ 0 ] = uno::Any();
            aArgs[ 1 ] <<= m_xDialog;
            aArgs[ 2 ] <<= m_xModel;
            aArgs[ 3 ] <<= OUString( GetParent()->GetName() );
            pDocObject = new SbUnoObject( GetName(), uno::makeAny( xVBAFactory->createInstanceWithArguments( "ooo.vba.msforms.UserForm", aArgs  ) ) );

            uno::Reference< lang::XComponent > xComponent( m_xDialog, uno::UNO_QUERY_THROW );

            
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
