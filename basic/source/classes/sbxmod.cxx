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


#include <vcl/svapp.hxx>
#include <tools/stream.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <basic/sbx.hxx>
#include <basic/sbuno.hxx>
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
#include <cppuhelper/implbase.hxx>
#include <unotools/eventcfg.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/VBAScriptEventId.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>

#ifdef UNX
#include <sys/resource.h>
#endif

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <map>
#include <com/sun/star/reflection/ProxyFactory.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/script/XInvocation.hpp>

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/awt/DialogProvider.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <comphelper/anytostring.hxx>
#include <ooo/vba/VbQueryClose.hpp>
#include <list>
#include <memory>
#include "sbxmod.hxx"
#include "parser.hxx"

#include <limits>

using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::uno;

typedef ::cppu::WeakImplHelper< XInvocation > DocObjectWrapper_BASE;
typedef ::std::map< sal_Int16, Any > OutParamMap;

class DocObjectWrapper : public DocObjectWrapper_BASE
{
    Reference< XAggregation >  m_xAggProxy;
    Reference< XInvocation >  m_xAggInv;
    Reference< XTypeProvider > m_xAggregateTypeProv;
    Sequence< Type >           m_Types;
    SbModule*                m_pMod;
    /// @throws css::uno::RuntimeException
    SbMethodRef getMethod( const OUString& aName );
    /// @throws css::uno::RuntimeException
    SbPropertyRef getProperty( const OUString& aName );

public:
    explicit DocObjectWrapper( SbModule* pMod );

    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() override
    {
        return css::uno::Sequence<sal_Int8>();
    }

    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(  ) override;

    virtual Any SAL_CALL invoke( const OUString& aFunctionName, const Sequence< Any >& aParams, Sequence< ::sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam ) override;
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const Any& aValue ) override;
    virtual Any SAL_CALL getValue( const OUString& aPropertyName ) override;
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) override;
    virtual  Any SAL_CALL queryInterface( const Type& aType ) override;

    virtual Sequence< Type > SAL_CALL getTypes() override;
};

DocObjectWrapper::DocObjectWrapper( SbModule* pVar ) : m_pMod( pVar )
{
    SbObjModule* pMod = dynamic_cast<SbObjModule*>( pVar );
    if ( pMod )
    {
        if ( pMod->GetModuleType() == ModuleType::DOCUMENT )
        {
            // Use proxy factory service to create aggregatable proxy.
            SbUnoObject* pUnoObj = dynamic_cast<SbUnoObject*>( pMod->GetObject()  );
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

Sequence< Type > SAL_CALL DocObjectWrapper::getTypes()
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
DocObjectWrapper::getIntrospection(  )
{
    return nullptr;
}

Any SAL_CALL
DocObjectWrapper::invoke( const OUString& aFunctionName, const Sequence< Any >& aParams, Sequence< ::sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
{
    if ( m_xAggInv.is() &&  m_xAggInv->hasMethod( aFunctionName ) )
            return m_xAggInv->invoke( aFunctionName, aParams, aOutParamIndex, aOutParam );
    SbMethodRef pMethod = getMethod( aFunctionName );
    if ( !pMethod.is() )
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
            if ( pParamInfo->nFlags & SbxFlagBits::Optional )
                ++nSbxOptional;
            else
                nSbxOptional = 0;
        }
        sal_Int32 nSbxCount = n - 1;
        if ( nParamsCount < nSbxCount - nSbxOptional )
        {
            throw RuntimeException( "wrong number of parameters!" );
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
            unoToSbxValue( xSbxVar.get(), pParams[i] );
            xSbxParams->Put( xSbxVar.get(), static_cast< sal_uInt16 >( i ) + 1 );

            // Enable passing by ref
            if ( xSbxVar->GetType() != SbxVARIANT )
                xSbxVar->SetFlag( SbxFlagBits::Fixed );
        }
    }
    if ( xSbxParams.is() )
        pMethod->SetParameters( xSbxParams.get() );

    // call method
    SbxVariableRef xReturn = new SbxVariable;

    pMethod->Call( xReturn.get() );
    Any aReturn;
    // get output parameters
    if ( xSbxParams.is() )
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
                        aOutParamMap.insert( OutParamMap::value_type( n - 1, sbxToUnoValue( xVar.get() ) ) );
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
    aReturn = sbxToUnoValue( xReturn.get() );

    pMethod->SetParameters( nullptr );

    return aReturn;
}

void SAL_CALL
DocObjectWrapper::setValue( const OUString& aPropertyName, const Any& aValue )
{
    if ( m_xAggInv.is() &&  m_xAggInv->hasProperty( aPropertyName ) )
            return m_xAggInv->setValue( aPropertyName, aValue );

    SbPropertyRef pProperty = getProperty( aPropertyName );
    if ( !pProperty.is() )
       throw UnknownPropertyException();
    unoToSbxValue( pProperty.get(), aValue );
}

Any SAL_CALL
DocObjectWrapper::getValue( const OUString& aPropertyName )
{
    if ( m_xAggInv.is() &&  m_xAggInv->hasProperty( aPropertyName ) )
            return m_xAggInv->getValue( aPropertyName );

    SbPropertyRef pProperty = getProperty( aPropertyName );
    if ( !pProperty.is() )
       throw UnknownPropertyException();

    SbxVariable* pProp = pProperty.get();
    if ( pProp->GetType() == SbxEMPTY )
        pProperty->Broadcast( SfxHintId::BasicDataWanted );

    Any aRet = sbxToUnoValue( pProp );
    return aRet;
}

sal_Bool SAL_CALL
DocObjectWrapper::hasMethod( const OUString& aName )
{
    if ( m_xAggInv.is() && m_xAggInv->hasMethod( aName ) )
        return true;
    return getMethod( aName ).is();
}

sal_Bool SAL_CALL
DocObjectWrapper::hasProperty( const OUString& aName )
{
    bool bRes = false;
    if ( m_xAggInv.is() && m_xAggInv->hasProperty( aName ) )
        bRes = true;
    else bRes = getProperty( aName ).is();
    return bRes;
}

Any SAL_CALL DocObjectWrapper::queryInterface( const Type& aType )
{
    Any aRet = DocObjectWrapper_BASE::queryInterface( aType );
    if ( aRet.hasValue() )
        return aRet;
    else if ( m_xAggProxy.is() )
        aRet = m_xAggProxy->queryAggregation( aType );
    return aRet;
}

SbMethodRef DocObjectWrapper::getMethod( const OUString& aName )
{
    SbMethodRef pMethod = nullptr;
    if ( m_pMod )
    {
        SbxFlagBits nSaveFlgs = m_pMod->GetFlags();
        // Limit search to this module
        m_pMod->ResetFlag( SbxFlagBits::GlobalSearch );
        pMethod = dynamic_cast<SbMethod*>(m_pMod->SbModule::Find(aName,  SbxClassType::Method));
        m_pMod->SetFlags( nSaveFlgs );
    }

    return pMethod;
}

SbPropertyRef DocObjectWrapper::getProperty( const OUString& aName )
{
    SbPropertyRef pProperty = nullptr;
    if ( m_pMod )
    {
        SbxFlagBits nSaveFlgs = m_pMod->GetFlags();
        // Limit search to this module.
        m_pMod->ResetFlag( SbxFlagBits::GlobalSearch );
        pProperty = dynamic_cast<SbProperty*>(m_pMod->SbModule::Find(aName,  SbxClassType::Property));
        m_pMod->SetFlag( nSaveFlgs );
    }

    return pProperty;
}


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

public:
    AsyncQuitHandler(const AsyncQuitHandler&) = delete;
    const AsyncQuitHandler& operator=(const AsyncQuitHandler&) = delete;
    static AsyncQuitHandler& instance()
    {
        static AsyncQuitHandler dInst;
        return dInst;
    }

    static void QuitApplication()
    {
        uno::Reference< frame::XDesktop2 > xDeskTop = frame::Desktop::create( comphelper::getProcessComponentContext() );
        xDeskTop->terminate();
    }
    DECL_STATIC_LINK( AsyncQuitHandler, OnAsyncQuit, void*, void );
};

IMPL_STATIC_LINK_NOARG( AsyncQuitHandler, OnAsyncQuit, void*, void )
{
    QuitApplication();
}

// A Basic module has set EXTSEARCH, so that the elements, that the modul contains,
// could be found from other module.

SbModule::SbModule( const OUString& rName, bool bVBACompat )
         : SbxObject( "StarBASICModule" ),
           pImage( nullptr ), pBreaks( nullptr ), pClassData( nullptr ), mbVBACompat( bVBACompat ),  pDocObject( nullptr ), bIsProxyModule( false )
{
    SetName( rName );
    SetFlag( SbxFlagBits::ExtSearch | SbxFlagBits::GlobalSearch );
    SetModuleType( script::ModuleType::NORMAL );

    // #i92642: Set name property to intitial name
    SbxVariable* pNameProp = pProps->Find( "Name", SbxClassType::Property );
    if( pNameProp != nullptr )
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
    mxWrapper = nullptr;
}

uno::Reference< script::XInvocation > const &
SbModule::GetUnoModule()
{
    if ( !mxWrapper.is() )
        mxWrapper = new DocObjectWrapper( this );

    SAL_INFO("basic","Module named " << OUStringToOString( GetName(), RTL_TEXTENCODING_UTF8 ).getStr() << " returning wrapper mxWrapper (0x" << mxWrapper.get() <<")" );
    return mxWrapper;
}

bool SbModule::IsCompiled() const
{
    return pImage != nullptr;
}

const SbxObject* SbModule::FindType( const OUString& aTypeName ) const
{
    return pImage ? pImage->FindType( aTypeName ) : nullptr;
}


// From the code generator: deletion of images and the opposite of validation for entries

void SbModule::StartDefinitions()
{
    delete pImage; pImage = nullptr;
    if( pClassData )
        pClassData->clear();

    // methods and properties persist, but they are invalid;
    // at least are the information under certain conditions clogged
    sal_uInt16 i;
    for( i = 0; i < pMethods->Count(); i++ )
    {
        SbMethod* p = dynamic_cast<SbMethod*>( pMethods->Get( i )  );
        if( p )
            p->bInvalid = true;
    }
    for( i = 0; i < pProps->Count(); )
    {
        SbProperty* p = dynamic_cast<SbProperty*>( pProps->Get( i )  );
        if( p )
            pProps->Remove( i );
        else
            i++;
    }
}

// request/create method

SbMethod* SbModule::GetMethod( const OUString& rName, SbxDataType t )
{
    SbxVariable* p = pMethods->Find( rName, SbxClassType::Method );
    SbMethod* pMeth = p ? dynamic_cast<SbMethod*>( p ) : nullptr;
    if( p && !pMeth )
    {
        pMethods->Remove( p );
    }
    if( !pMeth )
    {
        pMeth = new SbMethod( rName, t, this );
        pMeth->SetParent( this );
        pMeth->SetFlags( SbxFlagBits::Read );
        pMethods->Put( pMeth, pMethods->Count() );
        StartListening( pMeth->GetBroadcaster(), true );
    }
    // The method is per default valid, because it could be
    // created from the compiler (code generator) as well.
    pMeth->bInvalid = false;
    pMeth->ResetFlag( SbxFlagBits::Fixed );
    pMeth->SetFlag( SbxFlagBits::Write );
    pMeth->SetType( t );
    pMeth->ResetFlag( SbxFlagBits::Write );
    if( t != SbxVARIANT )
    {
        pMeth->SetFlag( SbxFlagBits::Fixed );
    }
    return pMeth;
}

SbMethod* SbModule::FindMethod( const OUString& rName, SbxClassType t )
{
    return dynamic_cast<SbMethod*> (pMethods->Find( rName, t ));
}


// request/create property

SbProperty* SbModule::GetProperty( const OUString& rName, SbxDataType t )
{
    SbxVariable* p = pProps->Find( rName, SbxClassType::Property );
    SbProperty* pProp = p ? dynamic_cast<SbProperty*>( p ) : nullptr;
    if( p && !pProp )
    {
        pProps->Remove( p );
    }
    if( !pProp )
    {
        pProp = new SbProperty( rName, t, this );
        pProp->SetFlag( SbxFlagBits::ReadWrite );
        pProp->SetParent( this );
        pProps->Put( pProp, pProps->Count() );
        StartListening( pProp->GetBroadcaster(), true );
    }
    return pProp;
}

void SbModule::GetProcedureProperty( const OUString& rName, SbxDataType t )
{
    SbxVariable* p = pProps->Find( rName, SbxClassType::Property );
    SbProcedureProperty* pProp = p ? dynamic_cast<SbProcedureProperty*>( p ) : nullptr;
    if( p && !pProp )
    {
        pProps->Remove( p );
    }
    if( !pProp )
    {
        pProp = new SbProcedureProperty( rName, t );
        pProp->SetFlag( SbxFlagBits::ReadWrite );
        pProp->SetParent( this );
        pProps->Put( pProp, pProps->Count() );
        StartListening( pProp->GetBroadcaster(), true );
    }
}

void SbModule::GetIfaceMapperMethod( const OUString& rName, SbMethod* pImplMeth )
{
    SbxVariable* p = pMethods->Find( rName, SbxClassType::Method );
    SbIfaceMapperMethod* pMapperMethod = p ? dynamic_cast<SbIfaceMapperMethod*>( p ) : nullptr;
    if( p && !pMapperMethod )
    {
        pMethods->Remove( p );
    }
    if( !pMapperMethod )
    {
        pMapperMethod = new SbIfaceMapperMethod( rName, pImplMeth );
        pMapperMethod->SetParent( this );
        pMapperMethod->SetFlags( SbxFlagBits::Read );
        pMethods->Put( pMapperMethod, pMethods->Count() );
    }
    pMapperMethod->bInvalid = false;
}

SbIfaceMapperMethod::~SbIfaceMapperMethod()
{
}


// From the code generator: remove invalid entries

void SbModule::EndDefinitions( bool bNewState )
{
    for( sal_uInt16 i = 0; i < pMethods->Count(); )
    {
        SbMethod* p = dynamic_cast<SbMethod*>( pMethods->Get( i )  );
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
    SetModified( true );
}

void SbModule::Clear()
{
    delete pImage; pImage = nullptr;
    if( pClassData )
        pClassData->clear();
    SbxObject::Clear();
}


SbxVariable* SbModule::Find( const OUString& rName, SbxClassType t )
{
    // make sure a search in an uninstantiated class module will fail
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if ( bIsProxyModule && !GetSbData()->bRunInit )
    {
        return nullptr;
    }
    if( !pRes && pImage )
    {
        SbiInstance* pInst = GetSbData()->pInst;
        if( pInst && pInst->IsCompatibility() )
        {
            // Put enum types as objects into module,
            // allows MyEnum.First notation
            SbxArrayRef xArray = pImage->GetEnums();
            if( xArray.is() )
            {
                SbxVariable* pEnumVar = xArray->Find( rName, SbxClassType::DontCare );
                SbxObject* pEnumObject = dynamic_cast<SbxObject*>( pEnumVar  );
                if( pEnumObject )
                {
                    bool bPrivate = pEnumObject->IsSet( SbxFlagBits::Private );
                    OUString aEnumName = pEnumObject->GetName();

                    pRes = new SbxVariable( SbxOBJECT );
                    pRes->SetName( aEnumName );
                    pRes->SetParent( this );
                    pRes->SetFlag( SbxFlagBits::Read );
                    if( bPrivate )
                    {
                        pRes->SetFlag( SbxFlagBits::Private );
                    }
                    pRes->PutObject( pEnumObject );
                }
            }
        }
    }
    return pRes;
}


const OUString& SbModule::GetSource() const
{
    return aOUSource;
}

// Parent and BASIC are one!

void SbModule::SetParent( SbxObject* p )
{
    pParent = p;
}

void SbModule::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbProperty* pProp = dynamic_cast<SbProperty*>( pVar );
        SbMethod* pMeth = dynamic_cast<SbMethod*>( pVar );
        SbProcedureProperty* pProcProperty = dynamic_cast<SbProcedureProperty*>( pVar  );
        if( pProcProperty )
        {

            if( pHint->GetId() == SfxHintId::BasicDataWanted )
            {
                OUString aProcName = "Property Get "
                                   + pProcProperty->GetName();

                SbxVariable* pMethVar = Find( aProcName, SbxClassType::Method );
                if( pMethVar )
                {
                    SbxValues aVals;
                    aVals.eType = SbxVARIANT;

                    SbxArray* pArg = pVar->GetParameters();
                    sal_uInt16 nVarParCount = (pArg != nullptr) ? pArg->Count() : 0;
                    if( nVarParCount > 1 )
                    {
                        auto xMethParameters = tools::make_ref<SbxArray>();
                        xMethParameters->Put( pMethVar, 0 );    // Method as parameter 0
                        for( sal_uInt16 i = 1 ; i < nVarParCount ; ++i )
                        {
                            SbxVariable* pPar = pArg->Get( i );
                            xMethParameters->Put( pPar, i );
                        }

                        pMethVar->SetParameters( xMethParameters.get() );
                        pMethVar->Get( aVals );
                        pMethVar->SetParameters( nullptr );
                    }
                    else
                    {
                        pMethVar->Get( aVals );
                    }

                    pVar->Put( aVals );
                }
            }
            else if( pHint->GetId() == SfxHintId::BasicDataChanged )
            {
                SbxVariable* pMethVar = nullptr;

                bool bSet = pProcProperty->isSet();
                if( bSet )
                {
                    pProcProperty->setSet( false );

                    OUString aProcName = "Property Set "
                                       + pProcProperty->GetName();
                    pMethVar = Find( aProcName, SbxClassType::Method );
                }
                if( !pMethVar ) // Let
                {
                    OUString aProcName = "Property Let "
                                       + pProcProperty->GetName();
                    pMethVar = Find( aProcName, SbxClassType::Method );
                }

                if( pMethVar )
                {
                    // Setup parameters
                    SbxArrayRef xArray = new SbxArray;
                    xArray->Put( pMethVar, 0 ); // Method as parameter 0
                    xArray->Put( pVar, 1 );
                    pMethVar->SetParameters( xArray.get() );

                    SbxValues aVals;
                    pMethVar->Get( aVals );
                    pMethVar->SetParameters( nullptr );
                }
            }
        }
        if( pProp )
        {
            if( pProp->GetModule() != this )
                SetError( ERRCODE_SBX_BAD_ACTION );
        }
        else if( pMeth )
        {
            if( pHint->GetId() == SfxHintId::BasicDataWanted )
            {
                if( pMeth->bInvalid && !Compile() )
                {
                    // auto compile has not worked!
                    StarBASIC::Error( ERRCODE_BASIC_BAD_PROP_VALUE );
                }
                else
                {
                    // Call of a subprogram
                    SbModule* pOld = GetSbData()->pMod;
                    GetSbData()->pMod = this;
                    Run( static_cast<SbMethod*>(pVar) );
                    GetSbData()->pMod = pOld;
                }
            }
        }
        else
        {
            // #i92642: Special handling for name property to avoid
            // side effects when using name as variable implicitly
            bool bForwardToSbxObject = true;

            const SfxHintId nId = pHint->GetId();
            if( (nId == SfxHintId::BasicDataWanted || nId == SfxHintId::BasicDataChanged) &&
                pVar->GetName().equalsIgnoreAsciiCase( "name" ) )
            {
                    bForwardToSbxObject = false;
            }
            if( bForwardToSbxObject )
            {
                SbxObject::Notify( rBC, rHint );
            }
        }
    }
}

// The setting of the source makes the image invalid
// and scans the method definitions newly in

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
                        bool bIsVBA = ( aTok.GetDbl()== 1 );
                        SetVBACompat( bIsVBA );
                        aTok.SetCompatible( bIsVBA );
                    }
                }
            }
            eLastTok = eCurTok;
        }
        // Definition of the method
        SbMethod* pMeth = nullptr;
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
                pMeth->bInvalid = false;
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
    EndDefinitions( true );
}

// Broadcast of a hint to all Basics

static void SendHint_( SbxObject* pObj, SfxHintId nId, SbMethod* p )
{
    // Self a BASIC?
    if( dynamic_cast<const StarBASIC *>(pObj) != nullptr && pObj->IsBroadcaster() )
        pObj->GetBroadcaster().Broadcast( SbxHint( nId, p ) );
    // Then ask for the subobjects
    SbxArray* pObjs = pObj->GetObjects();
    for( sal_uInt16 i = 0; i < pObjs->Count(); i++ )
    {
        SbxVariable* pVar = pObjs->Get( i );
        if( dynamic_cast<const SbxObject *>(pVar) != nullptr )
            SendHint_( dynamic_cast<SbxObject*>( pVar), nId, p  );
    }
}

static void SendHint( SbxObject* pObj, SfxHintId nId, SbMethod* p )
{
    while( pObj->GetParent() )
        pObj = pObj->GetParent();
    SendHint_( pObj, nId, p );
}

// #57841 Clear Uno-Objects, which were helt in RTL functions,
// at the end of the program, so that nothing were helt.
void ClearUnoObjectsInRTL_Impl_Rek( StarBASIC* pBasic )
{
    // delete the return value of CreateUnoService
    SbxVariable* pVar = pBasic->GetRtl()->Find( "CreateUnoService", SbxClassType::Method );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    // delete the return value of CreateUnoDialog
    pVar = pBasic->GetRtl()->Find( "CreateUnoDialog", SbxClassType::Method );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    // delete the return value of CDec
    pVar = pBasic->GetRtl()->Find( "CDec", SbxClassType::Method );
    if( pVar )
    {
        pVar->SbxValue::Clear();
    }
    // delete return value of CreateObject
    pVar = pBasic->GetRtl()->Find( "CreateObject", SbxClassType::Method );
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
        StarBASIC* pSubBasic = dynamic_cast<StarBASIC*>( pObjVar  );
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
    if( static_cast<StarBASIC*>(p) != pBasic )
        ClearUnoObjectsInRTL_Impl_Rek( static_cast<StarBASIC*>(p) );
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
void SbModule::Run( SbMethod* pMeth )
{
    SAL_INFO("basic","About to run " << OUStringToOString( pMeth->GetName(), RTL_TEXTENCODING_UTF8 ).getStr() << ", vba compatmode is " << mbVBACompat );

    static sal_uInt16 nMaxCallLevel = 0;

    bool bDelInst = ( GetSbData()->pInst == nullptr );
    bool bQuit = false;
    StarBASICRef xBasic;
    uno::Reference< frame::XModel > xModel;
    uno::Reference< script::vba::XVBACompatibility > xVBACompat;
    if( bDelInst )
    {
        // #32779: Hold Basic during the execution
        xBasic = static_cast<StarBASIC*>( GetParent() );

        GetSbData()->pInst = new SbiInstance( static_cast<StarBASIC*>(GetParent()) );

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
        // i80726 The Find below will generate an error in Testtool so we reset it unless there was one before already
        bool bWasError = SbxBase::GetError() != 0;
        SbxVariable* pMSOMacroRuntimeLibVar = Find( "Launcher", SbxClassType::Object );
        if ( !bWasError && (SbxBase::GetError() == ERRCODE_SBX_PROC_UNDEFINED) )
            SbxBase::ResetError();
        if( pMSOMacroRuntimeLibVar )
        {
            StarBASIC* pMSOMacroRuntimeLib = dynamic_cast<StarBASIC*>( pMSOMacroRuntimeLibVar );
            if( pMSOMacroRuntimeLib )
            {
                SbxFlagBits nGblFlag = pMSOMacroRuntimeLib->GetFlags() & SbxFlagBits::GlobalSearch;
                pMSOMacroRuntimeLib->ResetFlag( SbxFlagBits::GlobalSearch );
                SbxVariable* pAppSymbol = pMSOMacroRuntimeLib->Find( "Application", SbxClassType::Method );
                pMSOMacroRuntimeLib->SetFlag( nGblFlag );
                if( pAppSymbol )
                {
                    pMSOMacroRuntimeLib->SetFlag( SbxFlagBits::ExtSearch );      // Could have been disabled before
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
                SendHint( GetParent(), SfxHintId::BasicStart, pMeth );

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
                GetSbData()->pInst->EnableCompatibility( true );
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
                    Application::Yield();
            }

            GetSbData()->pInst->pRun = pRt->pNext;
            GetSbData()->pInst->nCallLvl--;          // Call-Level down again

            // Exist an higher-ranking runtime instance?
            // Then take over BasicDebugFlags::Break, if set
            SbiRuntime* pRtNext = pRt->pNext;
            if( pRtNext && (pRt->GetDebugFlags() & BasicDebugFlags::Break) )
                pRtNext->SetDebugFlags( BasicDebugFlags::Break );

            delete pRt;
            GetSbData()->pMod = pOldMod;
            if( bDelInst )
            {
                // #57841 Clear Uno-Objects, which were helt in RTL functions,
                // at the end of the program, so that nothing were helt.
                ClearUnoObjectsInRTL_Impl( xBasic.get() );

                clearNativeObjectWrapperVector();

                SAL_WARN_IF(GetSbData()->pInst->nCallLvl != 0,"basic","BASIC-Call-Level > 0");
                delete GetSbData()->pInst;
                GetSbData()->pInst = nullptr;
                bDelInst = false;

                // #i30690
                SolarMutexGuard aSolarGuard;
                SendHint( GetParent(), SfxHintId::BasicStop, pMeth );

                GlobalRunDeInit();

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
                    ::basic::vba::lockControllersOfAllDocuments( xModel, false );
                    ::basic::vba::enableContainerWindowsOfAllDocuments( xModel, true );
                }
            }
        }
        else
               GetSbData()->pInst->nCallLvl--;           // Call-Level down again
    }
    else
    {
        GetSbData()->pInst->nCallLvl--;          // Call-Level down again
        StarBASIC::FatalError( ERRCODE_BASIC_STACK_OVERFLOW );
    }

    StarBASIC* pBasic = dynamic_cast<StarBASIC*>( GetParent() );
    if( bDelInst )
    {
       // #57841 Clear Uno-Objects, which were helt in RTL functions,
       // the end of the program, so that nothing were helt.
        ClearUnoObjectsInRTL_Impl( xBasic.get() );

        delete GetSbData()->pInst;
        GetSbData()->pInst = nullptr;
    }
    if ( pBasic && pBasic->IsDocBasic() && pBasic->IsQuitApplication() && !GetSbData()->pInst )
            bQuit = true;
        if ( bQuit )
    {
        Application::PostUserEvent( LINK( &AsyncQuitHandler::instance(), AsyncQuitHandler, OnAsyncQuit ) );
    }
}

// Execute of the init method of a module after the loading
// or the compilation

void SbModule::RunInit()
{
    if( pImage
     && !pImage->bInit
     && pImage->IsFlag( SbiImageFlags::INITCODE ) )
    {
        // Set flag, so that RunInit get activ (Testtool)
        GetSbData()->bRunInit = true;

        SbModule* pOldMod = GetSbData()->pMod;
        GetSbData()->pMod = this;
        // The init code starts always here
        SbiRuntime* pRt = new SbiRuntime( this, nullptr, 0 );

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
    for ( const auto& rModuleVariableName: mModuleVariableNames )
    {
        if ( aName == rModuleVariableName )
            return;
    }
    mModuleVariableNames.push_back( aName );
}

void SbModule::RemoveVars()
{
    for ( const auto& rModuleVariableName: mModuleVariableNames )
    {
    // We don't want a Find being called in a derived class ( e.g.
    // SbUserform because it could trigger say an initialise event
    // which would cause basic to be re-run in the middle of the init ( and remember RemoveVars is called from compile and we don't want code to run as part of the compile )
    SbxVariableRef p = SbModule::Find( rModuleVariableName, SbxClassType::Property );
    if( p.is() )
        Remove( p.get() );
    }
}

void SbModule::ClearPrivateVars()
{
    for( sal_uInt16 i = 0 ; i < pProps->Count() ; i++ )
    {
        SbProperty* p = dynamic_cast<SbProperty*>( pProps->Get( i )  );
        if( p )
        {
            // Delete not the arrays, only their content
            if( p->GetType() & SbxARRAY )
            {
                SbxArray* pArray = dynamic_cast<SbxArray*>( p->GetObject() );
                if( pArray )
                {
                    for( sal_uInt16 j = 0 ; j < pArray->Count() ; j++ )
                    {
                        SbxVariable* pj = dynamic_cast<SbxVariable*>( pArray->Get( j ) );
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
    if( pVar->SbxValue::GetType() != SbxOBJECT || nullptr != dynamic_cast<const SbProcedureProperty*>( pVar) )
        return;

    SbxObject* pObj = dynamic_cast<SbxObject*>( pVar->GetObject() );
    if( pObj != nullptr )
    {
        SbxObject* p = pObj;

        SbModule* pMod = dynamic_cast<SbModule*>( p  );
        if( pMod != nullptr )
            pMod->ClearVarsDependingOnDeletedBasic( pDeletedBasic );

        while( (p = p->GetParent()) != nullptr )
        {
            StarBASIC* pBasic = dynamic_cast<StarBASIC*>( p  );
            if( pBasic != nullptr && pBasic == pDeletedBasic )
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
        SbProperty* p = dynamic_cast<SbProperty*>( pProps->Get( i )  );
        if( p )
        {
            if( p->GetType() & SbxARRAY )
            {
                SbxArray* pArray = dynamic_cast<SbxArray*>( p->GetObject() );
                if( pArray )
                {
                    for( sal_uInt16 j = 0 ; j < pArray->Count() ; j++ )
                    {
                        SbxVariable* pVar = dynamic_cast<SbxVariable*>( pArray->Get( j ) );
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

void StarBASIC::ClearAllModuleVars()
{
    // Initialise the own module
    for (const auto& rModule: pModules)
    {
        // Initialise only, if the startcode was already executed
        if( rModule->pImage && rModule->pImage->bInit && !rModule->isProxyModule() && nullptr == dynamic_cast<const SbObjModule*>( rModule.get()) )
            rModule->ClearPrivateVars();
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
    // GlobalRunInit, if at the initialising of the module
    // an error occurred. Then it will not be launched.
    GetSbData()->bGlobalInitErr = false;

    // Parent of the module is a Basic
    StarBASIC *pBasic = dynamic_cast<StarBASIC*>( GetParent() );
    if( pBasic )
    {
        pBasic->InitAllModules();

        SbxObject* pParent_ = pBasic->GetParent();
        if( pParent_ )
        {
            StarBASIC * pParentBasic = dynamic_cast<StarBASIC*>( pParent_ );
            if( pParentBasic )
            {
                pParentBasic->InitAllModules( pBasic );

                // #109018 Parent can also have a parent (library in doc)
                SbxObject* pParentParent = pParentBasic->GetParent();
                if( pParentParent )
                {
                    StarBASIC * pParentParentBasic = dynamic_cast<StarBASIC*>( pParentParent );
                    if( pParentParentBasic )
                        pParentParentBasic->InitAllModules( pParentBasic );
                }
            }
        }
    }
}

void SbModule::GlobalRunDeInit()
{
    StarBASIC *pBasic = dynamic_cast<StarBASIC*>( GetParent() );
    if( pBasic )
    {
        pBasic->DeInitAllModules();

        SbxObject* pParent_ = pBasic->GetParent();
        if( pParent_ )
            pBasic = dynamic_cast<StarBASIC*>( pParent_ );
        if( pBasic )
            pBasic->DeInitAllModules();
    }
}

// Search for the next STMNT-Command in the code. This was used from the STMNT-
// Opcode to set the endcolumn.

const sal_uInt8* SbModule::FindNextStmnt( const sal_uInt8* p, sal_uInt16& nLine, sal_uInt16& nCol ) const
{
    return FindNextStmnt( p, nLine, nCol, false );
}

const sal_uInt8* SbModule::FindNextStmnt( const sal_uInt8* p, sal_uInt16& nLine, sal_uInt16& nCol,
    bool bFollowJumps, const SbiImage* pImg ) const
{
    sal_uInt32 nPC = (sal_uInt32) ( p - reinterpret_cast<const sal_uInt8*>(pImage->GetCode()) );
    while( nPC < pImage->GetCodeSize() )
    {
        SbiOpcode eOp = (SbiOpcode ) ( *p++ );
        nPC++;
        if( bFollowJumps && eOp == SbiOpcode::JUMP_ && pImg )
        {
            SAL_WARN_IF( !pImg, "basic", "FindNextStmnt: pImg==NULL with FollowJumps option" );
            sal_uInt32 nOp1 = *p++; nOp1 |= *p++ << 8;
            nOp1 |= *p++ << 16; nOp1 |= *p++ << 24;
            p = reinterpret_cast<const sal_uInt8*>(pImg->GetCode()) + nOp1;
        }
        else if( eOp >= SbiOpcode::SbOP1_START && eOp <= SbiOpcode::SbOP1_END )
        {
            p += 4;
            nPC += 4;
        }
        else if( eOp == SbiOpcode::STMNT_ )
        {
            sal_uInt32 nl, nc;
            nl = *p++; nl |= *p++ << 8;
            nl |= *p++ << 16 ; nl |= *p++ << 24;
            nc = *p++; nc |= *p++ << 8;
            nc |= *p++ << 16 ; nc |= *p++ << 24;
            nLine = (sal_uInt16)nl; nCol = (sal_uInt16)nc;
            return p;
        }
        else if( eOp >= SbiOpcode::SbOP2_START && eOp <= SbiOpcode::SbOP2_END )
        {
            p += 8;
            nPC += 8;
        }
        else if( !( eOp >= SbiOpcode::SbOP0_START && eOp <= SbiOpcode::SbOP0_END ) )
        {
            StarBASIC::FatalError( ERRCODE_BASIC_INTERNAL_ERROR );
            break;
        }
    }
    return nullptr;
}

// Test, if a line contains STMNT-Opcodes

bool SbModule::IsBreakable( sal_uInt16 nLine ) const
{
    if( !pImage )
        return false;
    const sal_uInt8* p = reinterpret_cast<const sal_uInt8*>(pImage->GetCode());
    sal_uInt16 nl, nc;
    while( ( p = FindNextStmnt( p, nl, nc ) ) != nullptr )
        if( nl == nLine )
            return true;
    return false;
}

bool SbModule::IsBP( sal_uInt16 nLine ) const
{
    if( pBreaks )
    {
        for( size_t i = 0; i < pBreaks->size(); i++ )
        {
            sal_uInt16 b = pBreaks->operator[]( i );
            if( b == nLine )
                return true;
            if( b < nLine )
                break;
        }
    }
    return false;
}

bool SbModule::SetBP( sal_uInt16 nLine )
{
    if( !IsBreakable( nLine ) )
        return false;
    if( !pBreaks )
        pBreaks = new SbiBreakpoints;
    size_t i;
    for( i = 0; i < pBreaks->size(); i++ )
    {
        sal_uInt16 b = pBreaks->operator[]( i );
        if( b == nLine )
            return true;
        if( b < nLine )
            break;
    }
    pBreaks->insert( pBreaks->begin() + i, nLine );

    // #38568: Set during runtime as well here BasicDebugFlags::Break
    if( GetSbData()->pInst && GetSbData()->pInst->pRun )
        GetSbData()->pInst->pRun->SetDebugFlags( BasicDebugFlags::Break );

    return IsBreakable( nLine );
}

bool SbModule::ClearBP( sal_uInt16 nLine )
{
    bool bRes = false;
    if( pBreaks )
    {
        for( size_t i = 0; i < pBreaks->size(); i++ )
        {
            sal_uInt16 b = pBreaks->operator[]( i );
            if( b == nLine )
            {
                pBreaks->erase( pBreaks->begin() + i );
                bRes = true;
                break;
            }
            if( b < nLine )
                break;
        }
        if( pBreaks->empty() )
        {
            delete pBreaks;
            pBreaks = nullptr;
        }
    }
    return bRes;
}

void SbModule::ClearAllBP()
{
    delete pBreaks;
    pBreaks = nullptr;
}

void
SbModule::fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg ) const
{
        if ( !pImg )
            pImg = pImage;
        for( sal_uInt32 i = 0; i < pMethods->Count(); i++ )
        {
            SbMethod* pMeth = dynamic_cast<SbMethod*>( pMethods->Get( (sal_uInt16)i )  );
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

bool SbModule::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    Clear();
    if( !SbxObject::LoadData( rStrm, 1 ) )
        return false;
    // As a precaution...
    SetFlag( SbxFlagBits::ExtSearch | SbxFlagBits::GlobalSearch );
    sal_uInt8 bImage;
    rStrm.ReadUChar( bImage );
    if( bImage )
    {
        SbiImage* p = new SbiImage;
        sal_uInt32 nImgVer = 0;

        if( !p->Load( rStrm, nImgVer ) )
        {
            delete p;
            return false;
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
    return true;
}

bool SbModule::StoreData( SvStream& rStrm ) const
{
    bool bFixup = ( pImage && !pImage->ExceedsLegacyLimits() );
    if ( bFixup )
        fixUpMethodStart( true );
    bool bRet = SbxObject::StoreData( rStrm );
    if ( !bRet )
        return false;

    if( pImage )
    {
        pImage->aOUSource = aOUSource;
        pImage->aComment = aComment;
        pImage->aName = GetName();
        rStrm.WriteUChar( 1 );
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
        rStrm.WriteUChar( 1 );
        return aImg.Save( rStrm );
    }
}

bool SbModule::ExceedsLegacyModuleSize()
{
    if ( !IsCompiled() )
        Compile();
    if ( pImage && pImage->ExceedsLegacyLimits() )
        return true;
    return false;
}

class ErrorHdlResetter
{
    Link<StarBASIC*,bool> mErrHandler;
    bool    mbError;
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
    DECL_LINK( BasicErrorHdl, StarBASIC *, bool );
    bool HasError() { return mbError; }
};

IMPL_LINK( ErrorHdlResetter, BasicErrorHdl, StarBASIC *, /*pBasic*/, bool)
{
    mbError = true;
    return false;
}

void SbModule::GetCodeCompleteDataFromParse(CodeCompleteDataCache& aCache)
{
    ErrorHdlResetter aErrHdl;
    SbxBase::ResetError();

    std::unique_ptr<SbiParser> pParser(new SbiParser( static_cast<StarBASIC*>(GetParent()), this ));
    pParser->SetCodeCompleting(true);

    while( pParser->Parse() ) {}
    SbiSymPool* pPool = pParser->pPool;
    aCache.Clear();
    for( sal_uInt16 i = 0; i < pPool->GetSize(); ++i )
    {
        SbiSymDef* pSymDef = pPool->Get(i);
        //std::cerr << "i: " << i << ", type: " << pSymDef->GetType() << "; name:" << pSymDef->GetName() << std::endl;
        if( (pSymDef->GetType() != SbxEMPTY) && (pSymDef->GetType() != SbxNULL) )
            aCache.InsertGlobalVar( pSymDef->GetName(), pParser->aGblStrings.Find(pSymDef->GetTypeId()) );

        SbiSymPool& rChildPool = pSymDef->GetPool();
        for(sal_uInt16 j = 0; j < rChildPool.GetSize(); ++j )
        {
            SbiSymDef* pChildSymDef = rChildPool.Get(j);
            //std::cerr << "j: " << j << ", type: " << pChildSymDef->GetType() << "; name:" << pChildSymDef->GetName() << std::endl;
            if( (pChildSymDef->GetType() != SbxEMPTY) && (pChildSymDef->GetType() != SbxNULL) )
                aCache.InsertLocalVar( pSymDef->GetName(), pChildSymDef->GetName(), pParser->aGblStrings.Find(pChildSymDef->GetTypeId()) );
        }
    }
}


OUString SbModule::GetKeywordCase( const OUString& sKeyword )
{
    return SbiParser::GetKeywordCase( sKeyword );
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
void SbModule::StoreBinaryData( SvStream& rStrm, sal_uInt16 nVer )
{
    if (!Compile())
        return;
    bool bFixup = ( !nVer && !pImage->ExceedsLegacyLimits() );// save in old image format, fix up method starts

    if ( bFixup ) // save in old image format, fix up method starts
        fixUpMethodStart( true );

    if (!SbxObject::StoreData(rStrm))
        return;

    pImage->aOUSource.clear();
    pImage->aComment = aComment;
    pImage->aName = GetName();

    rStrm.WriteUChar(1);
    if (nVer)
        pImage->Save(rStrm);
    else
        pImage->Save(rStrm, B_LEGACYVERSION);
    if (bFixup)
        fixUpMethodStart(false); // restore method starts

    pImage->aOUSource = aOUSource;
}

// Called for >= OO 1.0 passwd protected libraries only

void SbModule::LoadBinaryData( SvStream& rStrm )
{
    OUString aKeepSource = aOUSource;
    LoadData( rStrm, 2 );
    LoadCompleted();
    aOUSource = aKeepSource;
}

bool SbModule::LoadCompleted()
{
    SbxArray* p = GetMethods().get();
    sal_uInt16 i;
    for( i = 0; i < p->Count(); i++ )
    {
        SbMethod* q = dynamic_cast<SbMethod*>( p->Get( i )  );
        if( q )
            q->pMod = this;
    }
    p = GetProperties();
    for( i = 0; i < p->Count(); i++ )
    {
        SbProperty* q = dynamic_cast<SbProperty*>( p->Get( i )  );
        if( q )
            q->pMod = this;
    }
    return true;
}

void SbModule::handleProcedureProperties( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    bool bDone = false;

    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbProcedureProperty* pProcProperty = dynamic_cast<SbProcedureProperty*>( pVar  );
        if( pProcProperty )
        {
            bDone = true;

            if( pHint->GetId() == SfxHintId::BasicDataWanted )
            {
                OUString aProcName = "Property Get "
                                   + pProcProperty->GetName();

                SbxVariable* pMeth = Find( aProcName, SbxClassType::Method );
                if( pMeth )
                {
                    SbxValues aVals;
                    aVals.eType = SbxVARIANT;

                    SbxArray* pArg = pVar->GetParameters();
                    sal_uInt16 nVarParCount = (pArg != nullptr) ? pArg->Count() : 0;
                    if( nVarParCount > 1 )
                    {
                        SbxArrayRef xMethParameters = new SbxArray;
                        xMethParameters->Put( pMeth, 0 );   // Method as parameter 0
                        for( sal_uInt16 i = 1 ; i < nVarParCount ; ++i )
                        {
                            SbxVariable* pPar = pArg->Get( i );
                            xMethParameters->Put( pPar, i );
                        }

                        pMeth->SetParameters( xMethParameters.get() );
                        pMeth->Get( aVals );
                        pMeth->SetParameters( nullptr );
                    }
                    else
                    {
                        pMeth->Get( aVals );
                    }

                    pVar->Put( aVals );
                }
            }
            else if( pHint->GetId() == SfxHintId::BasicDataChanged )
            {
                SbxVariable* pMeth = nullptr;

                bool bSet = pProcProperty->isSet();
                if( bSet )
                {
                    pProcProperty->setSet( false );

                    OUString aProcName = "Property Set "
                                       + pProcProperty->GetName();
                    pMeth = Find( aProcName, SbxClassType::Method );
                }
                if( !pMeth )    // Let
                {
                    OUString aProcName = "Property Let "
                                       + pProcProperty->GetName();
                    pMeth = Find( aProcName, SbxClassType::Method );
                }

                if( pMeth )
                {
                    // Setup parameters
                    SbxArrayRef xArray = new SbxArray;
                    xArray->Put( pMeth, 0 );    // Method as parameter 0
                    xArray->Put( pVar, 1 );
                    pMeth->SetParameters( xArray.get() );

                    SbxValues aVals;
                    pMeth->Get( aVals );
                    pMeth->SetParameters( nullptr );
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

bool SbJScriptModule::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    (void)nVer;

    Clear();
    if( !SbxObject::LoadData( rStrm, 1 ) )
        return false;

    // Get the source string
    aOUSource = rStrm.ReadUniOrByteString( osl_getThreadTextEncoding() );
    return true;
}

bool SbJScriptModule::StoreData( SvStream& rStrm ) const
{
    if( !SbxObject::StoreData( rStrm ) )
        return false;

    // Write the source string
    OUString aTmp = aOUSource;
    rStrm.WriteUniOrByteString( aTmp, osl_getThreadTextEncoding() );
    return true;
}


SbMethod::SbMethod( const OUString& r, SbxDataType t, SbModule* p )
        : SbxMethod( r, t ), pMod( p )
{
    bInvalid     = true;
    nStart       = 0;
    nDebugFlags  = BasicDebugFlags::NONE;
    nLine1       = 0;
    nLine2       = 0;
    refStatics   = new SbxArray;
    mCaller      = nullptr;
    // HACK due to 'Referenz could not be saved'
    SetFlag( SbxFlagBits::NoModify );
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
    SetFlag( SbxFlagBits::NoModify );
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
    return refStatics.get();
}

bool SbMethod::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    if( !SbxMethod::LoadData( rStrm, 1 ) )
        return false;

    sal_uInt16 nFlag;
    rStrm.ReadUInt16( nFlag );

    sal_Int16 nTempStart = (sal_Int16)nStart;

    if( nVer == 2 )
    {
        rStrm.ReadUInt16( nLine1 ).ReadUInt16( nLine2 ).ReadInt16( nTempStart ).ReadCharAsBool( bInvalid );
        //tdf#94617
        if (nFlag & 0x8000)
        {
            sal_uInt16 nMult = nFlag & 0x7FFF;
            sal_Int16 nMax = std::numeric_limits<sal_Int16>::max();
            nStart = nMult * nMax + nTempStart;
        }
        else
        {
            nStart = nTempStart;
        }
    }
    else
    {
        nStart = nTempStart;
    }

    // HACK ue to 'Referenz could not be saved'
    SetFlag( SbxFlagBits::NoModify );

    return true;
}

bool SbMethod::StoreData( SvStream& rStrm ) const
{
    if( !SbxMethod::StoreData( rStrm ) )
        return false;

    //tdf#94617
    sal_Int16 nMax = std::numeric_limits<sal_Int16>::max();
    sal_Int16 nStartTemp = nStart % nMax;
    sal_uInt16 nDebugFlagsTemp = nStart / nMax;
    nDebugFlagsTemp |= 0x8000;

    rStrm.WriteUInt16( nDebugFlagsTemp )
         .WriteInt16( nLine1 )
         .WriteInt16( nLine2 )
         .WriteInt16( nStartTemp )
         .WriteBool( bInvalid );

    return true;
}

void SbMethod::GetLineRange( sal_uInt16& l1, sal_uInt16& l2 )
{
    l1 = nLine1; l2 = nLine2;
}

// Could later be deleted

SbxInfo* SbMethod::GetInfo()
{
    return pInfo.get();
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
    tools::SvRef<SbModule> pMod_ = static_cast<SbModule*>(GetParent());

    tools::SvRef<StarBASIC> pBasic = static_cast<StarBASIC*>(pMod_->GetParent());

    // Establish the values to get the return value
    SbxValues aVals;
    aVals.eType = SbxVARIANT;

    // #104083: Compile BEFORE get
    if( bInvalid && !pMod_->Compile() )
        StarBASIC::Error( ERRCODE_BASIC_BAD_PROP_VALUE );

    Get( aVals );
    if ( pRet )
        pRet->Put( aVals );

    // Was there an error
    ErrCode nErr = SbxBase::GetError();
    SbxBase::ResetError();

    mCaller = nullptr;
    return nErr;
}


// #100883 Own Broadcast for SbMethod
void SbMethod::Broadcast( SfxHintId nHintId )
{
    if( pCst && !IsSet( SbxFlagBits::NoBroadcast ) )
    {
        // Because the method could be called from outside, test here once again
        // the authorisation
        if( nHintId == SfxHintId::BasicDataWanted )
            if( !CanRead() )
                return;
        if( nHintId == SfxHintId::BasicDataChanged )
            if( !CanWrite() )
                return;

        if( pMod && !pMod->IsCompiled() )
            pMod->Compile();

        // Block broadcasts while creating new method
        SfxBroadcaster* pSave = pCst;
        pCst = nullptr;
        SbMethod* pThisCopy = new SbMethod( *this );
        SbMethodRef xHolder = pThisCopy;
        if( mpPar.is() )
        {
            // Enregister this as element 0, but don't reset the parent!
            if( GetType() != SbxVOID ) {
                mpPar->PutDirect( pThisCopy, 0 );
            }
            SetParameters( nullptr );
        }

        pCst = pSave;
        pSave->Broadcast( SbxHint( nHintId, pThisCopy ) );

        SbxFlagBits nSaveFlags = GetFlags();
        SetFlag( SbxFlagBits::ReadWrite );
        pCst = nullptr;
        Put( pThisCopy->GetValues_Impl() );
        pCst = pSave;
        SetFlags( nSaveFlags );
    }
}


// Implementation of SbJScriptMethod (method class as a wrapper for JavaScript-functions)

SbJScriptMethod::SbJScriptMethod( const OUString& r, SbxDataType t )
        : SbMethod( r, t, nullptr )
{
}

SbJScriptMethod::~SbJScriptMethod()
{}


SbObjModule::SbObjModule( const OUString& rName, const css::script::ModuleInfo& mInfo, bool bIsVbaCompatible )
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
SbObjModule::SetUnoObject( const uno::Any& aObj )
{
    SbUnoObject* pUnoObj = dynamic_cast<SbUnoObject*>( pDocObject.get() );
    if ( pUnoObj && pUnoObj->getUnoAny() == aObj ) // object is equal, nothing to do
        return;
    pDocObject = new SbUnoObject( GetName(), aObj );

    css::uno::Reference< css::lang::XServiceInfo > xServiceInfo( aObj, css::uno::UNO_QUERY_THROW );
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
    return pDocObject.get();
}
SbxVariable*
SbObjModule::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* pVar = nullptr;
    if ( pDocObject.get() )
        pVar = pDocObject->Find( rName, t );
    if ( !pVar )
        pVar = SbModule::Find( rName, t );
    return pVar;
}

void SbObjModule::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SbModule::handleProcedureProperties( rBC, rHint );
}


typedef ::cppu::WeakImplHelper<
    awt::XTopWindowListener,
    awt::XWindowListener,
    document::XDocumentEventListener > FormObjEventListener_BASE;

class FormObjEventListenerImpl:
    public FormObjEventListener_BASE
{
    SbUserFormModule* mpUserForm;
    uno::Reference< lang::XComponent > mxComponent;
    uno::Reference< frame::XModel > mxModel;
    bool mbDisposed;
    bool mbOpened;
    bool mbActivated;
    bool mbShowing;

public:
    FormObjEventListenerImpl(const FormObjEventListenerImpl&) = delete;
    const FormObjEventListenerImpl& operator=(const FormObjEventListenerImpl&) = delete;
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
                uno::Reference< document::XDocumentEventBroadcaster >( mxModel, uno::UNO_QUERY_THROW )->addDocumentEventListener( this );
            }
            catch(const uno::Exception& ) {}
        }
    }

    virtual ~FormObjEventListenerImpl() override
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
                uno::Reference< document::XDocumentEventBroadcaster >( mxModel, uno::UNO_QUERY_THROW )->removeDocumentEventListener( this );
            }
            catch(const uno::Exception& ) {}
        }
        mxModel.clear();
    }

    virtual void SAL_CALL windowOpened( const lang::EventObject& /*e*/ ) override
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


    virtual void SAL_CALL windowClosing( const lang::EventObject& /*e*/ ) override
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


    virtual void SAL_CALL windowClosed( const lang::EventObject& /*e*/ ) override
    {
        mbOpened = false;
        mbShowing = false;
    }

    virtual void SAL_CALL windowMinimized( const lang::EventObject& /*e*/ ) override
    {
    }

    virtual void SAL_CALL windowNormalized( const lang::EventObject& /*e*/ ) override
    {
    }

    virtual void SAL_CALL windowActivated( const lang::EventObject& /*e*/ ) override
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

    virtual void SAL_CALL windowDeactivated( const lang::EventObject& /*e*/ ) override
    {
        if ( mpUserForm )
            mpUserForm->triggerDeactivateEvent();
    }

    virtual void SAL_CALL windowResized( const awt::WindowEvent& /*e*/ ) override
    {
        if ( mpUserForm )
        {
            mpUserForm->triggerResizeEvent();
            mpUserForm->triggerLayoutEvent();
        }
    }

    virtual void SAL_CALL windowMoved( const awt::WindowEvent& /*e*/ ) override
    {
        if ( mpUserForm )
            mpUserForm->triggerLayoutEvent();
    }

    virtual void SAL_CALL windowShown( const lang::EventObject& /*e*/ ) override
    {
    }

    virtual void SAL_CALL windowHidden( const lang::EventObject& /*e*/ ) override
    {
    }

    virtual void SAL_CALL documentEventOccured( const document::DocumentEvent& rEvent ) override
    {
        // early disposing on document event "OnUnload", to be sure Basic still exists when calling VBA "UserForm_Terminate"
        if( rEvent.EventName == GlobalEventConfig::GetEventName( GlobalEventId::CLOSEDOC ) )
        {
            removeListener();
            mbDisposed = true;
            if ( mpUserForm )
                mpUserForm->ResetApiObj();   // will trigger "UserForm_Terminate"
        }
    }

    virtual void SAL_CALL disposing( const lang::EventObject& /*Source*/ ) override
    {
        SAL_INFO("basic", "** Userform/Dialog disposing");
        removeListener();
        mbDisposed = true;
        if ( mpUserForm )
            mpUserForm->ResetApiObj( false );   // pass false (too late to trigger VBA events here)
    }
};

SbUserFormModule::SbUserFormModule( const OUString& rName, const css::script::ModuleInfo& mInfo, bool bIsCompat )
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
    pDocObject = nullptr;
    m_xDialog = nullptr;
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
    SbxVariable* pMeth = SbObjModule::Find( aMethodToRun, SbxClassType::Method );
    if( pMeth )
    {
        if ( aArguments.getLength() > 0 )   // Setup parameters
        {
            auto xArray = tools::make_ref<SbxArray>();
            xArray->Put( pMeth, 0 );    // Method as parameter 0

            for ( sal_Int32 i = 0; i < aArguments.getLength(); ++i )
            {
                auto xSbxVar = tools::make_ref<SbxVariable>( SbxVARIANT );
                unoToSbxValue( xSbxVar.get(), aArguments[i] );
                xArray->Put( xSbxVar.get(), static_cast< sal_uInt16 >( i ) + 1 );

                // Enable passing by ref
                if ( xSbxVar->GetType() != SbxVARIANT )
                    xSbxVar->SetFlag( SbxFlagBits::Fixed );
            }
            pMeth->SetParameters( xArray.get() );

            SbxValues aVals;
            pMeth->Get( aVals );

            for ( sal_Int32 i = 0; i < aArguments.getLength(); ++i )
            {
                aArguments[i] = sbxToUnoValue( xArray->Get( static_cast< sal_uInt16 >(i) + 1) );
            }
            pMeth->SetParameters( nullptr );
        }
        else
        {
            SbxValues aVals;
            pMeth->Get( aVals );
        }
    }
}

void SbUserFormModule::triggerActivateEvent()
{
    SAL_INFO("basic", "**** entering SbUserFormModule::triggerActivate");
    triggerMethod( "UserForm_Activate" );
    SAL_INFO("basic", "**** leaving SbUserFormModule::triggerActivate");
}

void SbUserFormModule::triggerDeactivateEvent()
{
    SAL_INFO("basic", "**** SbUserFormModule::triggerDeactivate");
    triggerMethod( "Userform_Deactivate" );
}

void SbUserFormModule::triggerInitializeEvent()
{
    if ( mbInit )
        return;
    SAL_INFO("basic", "**** SbUserFormModule::triggerInitializeEvent");
    triggerMethod("Userform_Initialize");
    mbInit = true;
}

void SbUserFormModule::triggerTerminateEvent()
{
    SAL_INFO("basic", "**** SbUserFormModule::triggerTerminateEvent");
    triggerMethod("Userform_Terminate");
    mbInit=false;
}

void SbUserFormModule::triggerLayoutEvent()
{
    triggerMethod("Userform_Layout");
}

void SbUserFormModule::triggerResizeEvent()
{
    triggerMethod("Userform_Resize");
}

SbUserFormModuleInstance* SbUserFormModule::CreateInstance()
{
    SbUserFormModuleInstance* pInstance = new SbUserFormModuleInstance( this, GetName(), m_mInfo, IsVBACompat() );
    return pInstance;
}

SbUserFormModuleInstance::SbUserFormModuleInstance( SbUserFormModule* pParentModule,
    const OUString& rName, const css::script::ModuleInfo& mInfo, bool bIsVBACompat )
        : SbUserFormModule( rName, mInfo, bIsVBACompat )
        , m_pParentModule( pParentModule )
{
}

bool SbUserFormModuleInstance::IsClass( const OUString& rName ) const
{
    bool bParentNameMatches = m_pParentModule->GetName().equalsIgnoreAsciiCase( rName );
    bool bRet = bParentNameMatches || SbxObject::IsClass( rName );
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
    if ( !pDocObject.is() )
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
    // test against 0 ( false ) and assume anything else is true
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
    SbxVariable* pMeth = SbObjModule::Find( "UnloadObject", SbxClassType::Method );
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


void SbUserFormModule::InitObject()
{
    try
    {
        OUString aHook("VBAGlobals");
        SbUnoObject* pGlobs = static_cast<SbUnoObject*>(GetParent()->Find( aHook, SbxClassType::DontCare ));
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
            StarBASIC* pParentBasic = nullptr;
            SbxObject* pCurObject = this;
            do
            {
                SbxObject* pObjParent = pCurObject->GetParent();
                pParentBasic = dynamic_cast<StarBASIC*>( pObjParent  );
                pCurObject = pObjParent;
            }
            while( pParentBasic == nullptr && pCurObject != nullptr );

            SAL_WARN_IF( pParentBasic == nullptr, "basic", "pParentBasic == NULL" );
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
    if ( !pDocObject.is() && !GetSbData()->bRunInit && GetSbData()->pInst )
        InitObject();
    return SbObjModule::Find( rName, t );
}

SbProperty::SbProperty( const OUString& r, SbxDataType t, SbModule* p )
        : SbxProperty( r, t ), pMod( p )
{
}

SbProperty::~SbProperty()
{}


SbProcedureProperty::~SbProcedureProperty()
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
