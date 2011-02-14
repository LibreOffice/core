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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include <list>

#include <vos/macros.hxx>
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
#include "sbtrace.hxx"


//#include <basic/hilight.hxx>
#include <svtools/syntaxhighlight.hxx>

#include <basic/basrdll.hxx>
#include <vos/mutex.hxx>
#include <basic/sbobjmod.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;

// for the bsearch
#ifdef WNT
#define CDECL _cdecl
#endif
#if defined(UNX) || defined(OS2)
#define CDECL
#endif
#ifdef UNX
#include <sys/resource.h>
#endif

#include <stdio.h>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <map>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <cppuhelper/implbase1.hxx>
#include <basic/sbobjmod.hxx>
#include <com/sun/star/uno/XAggregation.hpp>
#include <map>
#include <com/sun/star/script/XInvocation.hpp>

 using namespace ::com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;


#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XDialogProvider.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/anytostring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

typedef ::cppu::WeakImplHelper1< XInvocation > DocObjectWrapper_BASE;
typedef ::std::map< sal_Int16, Any, ::std::less< sal_Int16 > > OutParamMap;
::com::sun::star::uno::Any sbxToUnoValue( SbxVariable* pVar );
void unoToSbxValue( SbxVariable* pVar, const ::com::sun::star::uno::Any& aValue );

class DocObjectWrapper : public DocObjectWrapper_BASE
{
    Reference< XAggregation >  m_xAggProxy;
    Reference< XInvocation >  m_xAggInv;
    Reference< XTypeProvider > m_xAggregateTypeProv;
    Sequence< Type >           m_Types;
    SbModule*                m_pMod;
    SbMethodRef getMethod( const rtl::OUString& aName ) throw (RuntimeException);
    SbPropertyRef getProperty( const rtl::OUString& aName ) throw (RuntimeException);
    String mName; // for debugging

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

    virtual Any SAL_CALL invoke( const ::rtl::OUString& aFunctionName, const Sequence< Any >& aParams, Sequence< ::sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam ) throw (IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException);
    virtual void SAL_CALL setValue( const ::rtl::OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException);
    virtual Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName ) throw (UnknownPropertyException, RuntimeException);
    virtual ::sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName ) throw (RuntimeException);
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
            Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
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
                    Reference< XMultiComponentFactory > xMFac( xFactory, UNO_QUERY_THROW );
                    Reference< XPropertySet> xPSMPropertySet( xMFac, UNO_QUERY_THROW );
                    Reference< XComponentContext >  xCtx;
                    xPSMPropertySet->getPropertyValue(
                    String( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xCtx;
                    Reference< XProxyFactory > xProxyFac( xMFac->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.reflection.ProxyFactory" ) ), xCtx  ), UNO_QUERY_THROW );
                    m_xAggProxy = xProxyFac->createProxy( xIf );
                }
                catch(  Exception& )
                {
                    OSL_ENSURE( false, "DocObjectWrapper::DocObjectWrapper: Caught exception!" );
                }
            }

            if ( m_xAggProxy.is() )
            {
                osl_incrementInterlockedCount( &m_refCount );

                /* i35609 - Fix crash on Solaris. The setDelegator call needs
                    to be in its own block to ensure that all temporary Reference
                    instances that are acquired during the call are released
                    before m_refCount is decremented again */
                {
                    m_xAggProxy->setDelegator( static_cast< cppu::OWeakObject * >( this ) );
                }

                 osl_decrementInterlockedCount( &m_refCount );
            }
        }
    }
}

void SAL_CALL
DocObjectWrapper::acquire() throw ()
{
    osl_incrementInterlockedCount( &m_refCount );
    OSL_TRACE("DocObjectWrapper::acquire(%s) 0x%x refcount is now %d", rtl::OUStringToOString( mName, RTL_TEXTENCODING_UTF8 ).getStr(), this, m_refCount );
}
void SAL_CALL
DocObjectWrapper::release() throw ()
{
    if ( osl_decrementInterlockedCount( &m_refCount ) == 0 )
    {
        OSL_TRACE("DocObjectWrapper::release(%s) 0x%x refcount is now %d", rtl::OUStringToOString( mName, RTL_TEXTENCODING_UTF8 ).getStr(), this, m_refCount );
        delete this;
    }
    else
        OSL_TRACE("DocObjectWrapper::release(%s) 0x%x refcount is now %d", rtl::OUStringToOString( mName, RTL_TEXTENCODING_UTF8 ).getStr(), this, m_refCount );
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
            sTypes = m_xAggregateTypeProv->getTypes();
        m_Types.realloc( sTypes.getLength() + 1 );
        Type* pPtr = m_Types.getArray();
        for ( int i=0; i<m_Types.getLength(); ++i, ++pPtr )
        {
            if ( i == 0 )
                *pPtr = XInvocation::static_type( NULL );
            else
                *pPtr = sTypes[ i - 1 ];
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
DocObjectWrapper::invoke( const ::rtl::OUString& aFunctionName, const Sequence< Any >& aParams, Sequence< ::sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam ) throw (IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException)
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
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "wrong number of parameters!" ) ), Reference< XInterface >() );
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
    ErrCode nErr = SbxERR_OK;

    nErr = pMethod->Call( xReturn );
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
DocObjectWrapper::setValue( const ::rtl::OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException)
{
    if ( m_xAggInv.is() &&  m_xAggInv->hasProperty( aPropertyName ) )
            return m_xAggInv->setValue( aPropertyName, aValue );

    SbPropertyRef pProperty = getProperty( aPropertyName );
    if ( !pProperty.Is() )
       throw UnknownPropertyException();
    unoToSbxValue( (SbxVariable*) pProperty, aValue );
}

Any SAL_CALL
DocObjectWrapper::getValue( const ::rtl::OUString& aPropertyName ) throw (UnknownPropertyException, RuntimeException)
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
DocObjectWrapper::hasMethod( const ::rtl::OUString& aName ) throw (RuntimeException)
{
    if ( m_xAggInv.is() && m_xAggInv->hasMethod( aName ) )
        return sal_True;
    return getMethod( aName ).Is();
}

::sal_Bool SAL_CALL
DocObjectWrapper::hasProperty( const ::rtl::OUString& aName ) throw (RuntimeException)
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

SbMethodRef DocObjectWrapper::getMethod( const rtl::OUString& aName ) throw (RuntimeException)
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

SbPropertyRef DocObjectWrapper::getProperty( const rtl::OUString& aName ) throw (RuntimeException)
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

typedef std::vector<HighlightPortion> HighlightPortions;

bool getDefaultVBAMode( StarBASIC* pb )
{
    bool bResult = false;
    if ( pb && pb->IsDocBasic() )
    {
        uno::Any aDoc;
    if ( pb->GetUNOConstant( "ThisComponent", aDoc ) )
        {
            uno::Reference< beans::XPropertySet > xProp( aDoc, uno::UNO_QUERY );
            if ( xProp.is() )
            {
                uno::Reference< script::vba::XVBACompatibility > xVBAMode( xProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicLibraries") ) ), uno::UNO_QUERY );
                if ( xVBAMode.is() )
                    bResult = xVBAMode->getVBACompatibilityMode() == sal_True;
            }
        }
    }
    return bResult;
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
        uno::Reference< lang::XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
        if ( xFactory.is() )
    {
            uno::Reference< frame::XDesktop > xDeskTop( xFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop") ) ), uno::UNO_QUERY );
           if ( xDeskTop.is() )
               xDeskTop->terminate();
        }
    }
    DECL_LINK( OnAsyncQuit, void* );
};

IMPL_LINK( AsyncQuitHandler, OnAsyncQuit, void*, /*pNull*/ )
{
    QuitApplication();
    return 0L;
}

bool VBAUnlockControllers( StarBASIC* pBasic )
{
    bool bRes = false;
    if ( pBasic && pBasic->IsDocBasic() )
    {
        SbUnoObject* pGlobs = dynamic_cast< SbUnoObject* >( pBasic->Find( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ThisComponent" ) ), SbxCLASS_DONTCARE ) );
        if ( pGlobs ) try
        {
            uno::Reference< frame::XModel > xModel( pGlobs->getUnoAny(), uno::UNO_QUERY_THROW );
            if ( xModel->hasControllersLocked() )
                xModel->unlockControllers();
            bRes = true;
        }
        catch( uno::Exception& )
        {
        }
    }
    return bRes;
}

/////////////////////////////////////////////////////////////////////////////

// Ein BASIC-Modul hat EXTSEARCH gesetzt, damit die im Modul enthaltenen
// Elemente von anderen Modulen aus gefunden werden koennen.

SbModule::SbModule( const String& rName,  sal_Bool bVBACompat )
         : SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("StarBASICModule") ) ),
           pImage( NULL ), pBreaks( NULL ), pClassData( NULL ), mbVBACompat( bVBACompat ),  pDocObject( NULL ), bIsProxyModule( false )
{
    SetName( rName );
    SetFlag( SBX_EXTSEARCH | SBX_GBLSEARCH );
    SetModuleType( script::ModuleType::NORMAL );

    // #i92642: Set name property to intitial name
    SbxVariable* pNameProp = pProps->Find( String( RTL_CONSTASCII_USTRINGPARAM("Name") ), SbxCLASS_PROPERTY );
    if( pNameProp != NULL )
        pNameProp->PutString( GetName() );
}

SbModule::~SbModule()
{
    OSL_TRACE("Module named %s is destructing", rtl::OUStringToOString( GetName(), RTL_TEXTENCODING_UTF8 ).getStr() );
    if( pImage )
        delete pImage;
    if( pBreaks )
        delete pBreaks;
    if( pClassData )
        delete pClassData;
        mxWrapper = NULL;
}

uno::Reference< script::XInvocation >
SbModule::GetUnoModule()
{
    if ( !mxWrapper.is() )
        mxWrapper = new DocObjectWrapper( this );

    OSL_TRACE("Module named %s returning wrapper mxWrapper (0x%x)", rtl::OUStringToOString( GetName(), RTL_TEXTENCODING_UTF8 ).getStr(), mxWrapper.get() );
    return mxWrapper;
}

sal_Bool SbModule::IsCompiled() const
{
    return sal_Bool( pImage != 0 );
}

const SbxObject* SbModule::FindType( String aTypeName ) const
{
    return pImage ? pImage->FindType( aTypeName ) : NULL;
}


// Aus dem Codegenerator: Loeschen des Images und Invalidieren der Entries

void SbModule::StartDefinitions()
{
    delete pImage; pImage = NULL;
    if( pClassData )
        pClassData->clear();

    // Methoden und Properties bleiben erhalten, sind jedoch ungueltig
    // schliesslich sind ja u.U. die Infos belegt
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

// Methode anfordern/anlegen

SbMethod* SbModule::GetMethod( const String& rName, SbxDataType t )
{
    SbxVariable* p = pMethods->Find( rName, SbxCLASS_METHOD );
    SbMethod* pMeth = p ? PTR_CAST(SbMethod,p) : NULL;
    if( p && !pMeth )
        pMethods->Remove( p );
    if( !pMeth )
    {
        pMeth = new SbMethod( rName, t, this );
        pMeth->SetParent( this );
        pMeth->SetFlags( SBX_READ );
        pMethods->Put( pMeth, pMethods->Count() );
        StartListening( pMeth->GetBroadcaster(), sal_True );
    }
    // Per Default ist die Methode GUELTIG, da sie auch vom Compiler
    // (Codegenerator) erzeugt werden kann
    pMeth->bInvalid = sal_False;
    pMeth->ResetFlag( SBX_FIXED );
    pMeth->SetFlag( SBX_WRITE );
    pMeth->SetType( t );
    pMeth->ResetFlag( SBX_WRITE );
    if( t != SbxVARIANT )
        pMeth->SetFlag( SBX_FIXED );
    return pMeth;
}

// Property anfordern/anlegen

SbProperty* SbModule::GetProperty( const String& rName, SbxDataType t )
{
    SbxVariable* p = pProps->Find( rName, SbxCLASS_PROPERTY );
    SbProperty* pProp = p ? PTR_CAST(SbProperty,p) : NULL;
    if( p && !pProp )
        pProps->Remove( p );
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

SbProcedureProperty* SbModule::GetProcedureProperty
    ( const String& rName, SbxDataType t )
{
    SbxVariable* p = pProps->Find( rName, SbxCLASS_PROPERTY );
    SbProcedureProperty* pProp = p ? PTR_CAST(SbProcedureProperty,p) : NULL;
    if( p && !pProp )
        pProps->Remove( p );
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

SbIfaceMapperMethod* SbModule::GetIfaceMapperMethod
    ( const String& rName, SbMethod* pImplMeth )
{
    SbxVariable* p = pMethods->Find( rName, SbxCLASS_METHOD );
    SbIfaceMapperMethod* pMapperMethod = p ? PTR_CAST(SbIfaceMapperMethod,p) : NULL;
    if( p && !pMapperMethod )
        pMethods->Remove( p );
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


// Aus dem Codegenerator: Ungueltige Eintraege entfernen

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


SbxVariable* SbModule::Find( const XubString& rName, SbxClassType t )
{
    // make sure a search in an uninstatiated class module will fail
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if ( bIsProxyModule && !GetSbData()->bRunInit )
        return NULL;
    if( !pRes && pImage )
    {
        SbiInstance* pInst = pINST;
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
                    String aEnumName = pEnumObject->GetName();

                    pRes = new SbxVariable( SbxOBJECT );
                    pRes->SetName( aEnumName );
                    pRes->SetParent( this );
                    pRes->SetFlag( SBX_READ );
                    if( bPrivate )
                        pRes->SetFlag( SBX_PRIVATE );
                    pRes->PutObject( pEnumObject );
                }
            }
        }
    }
    return pRes;
}

const ::rtl::OUString& SbModule::GetSource32() const
{
    return aOUSource;
}

const String& SbModule::GetSource() const
{
    static String aRetStr;
    aRetStr = aOUSource;
    return aRetStr;
}

// Parent und BASIC sind eins!

void SbModule::SetParent( SbxObject* p )
{
    // #118083: Assertion is not valid any more
    // DBG_ASSERT( !p || p->IsA( TYPE(StarBASIC) ), "SbModules nur in BASIC eintragen" );
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
                    // Auto-Compile hat nicht geklappt!
                    StarBASIC::Error( SbERR_BAD_PROP_VALUE );
                else
                {
                    // Aufruf eines Unterprogramms
                    SbModule* pOld = pMOD;
                    pMOD = this;
                    Run( (SbMethod*) pVar );
                    pMOD = pOld;
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
                pVar->GetName().EqualsIgnoreCaseAscii( "name" ) )
                    bForwardToSbxObject = false;

            if( bForwardToSbxObject )
                SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
        }
    }
}

// Das Setzen der Source macht das Image ungueltig
// und scant die Methoden-Definitionen neu ein

void SbModule::SetSource( const String& r )
{
    SetSource32( r );
}

void SbModule::SetSource32( const ::rtl::OUString& r )
{
    // Default basic mode to library container mode, but.. allow Option VBASupport 0/1 override
        SetVBACompat( getDefaultVBAMode( static_cast< StarBASIC*>( GetParent() ) ) );
    aOUSource = r;
    StartDefinitions();
    SbiTokenizer aTok( r );
    while( !aTok.IsEof() )
    {
        SbiToken eEndTok = NIL;

        // Suchen nach SUB oder FUNCTION
        SbiToken eLastTok = NIL;
        while( !aTok.IsEof() )
        {
            // #32385: Nicht bei declare
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
                        aTok.SetCompatible( true );
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
        // Definition der Methode
        SbMethod* pMeth = NULL;
        if( eEndTok != NIL )
        {
            sal_uInt16 nLine1 = aTok.GetLine();
            if( aTok.Next() == SYMBOL )
            {
                String aName_( aTok.GetSym() );
                SbxDataType t = aTok.GetType();
                if( t == SbxVARIANT && eEndTok == ENDSUB )
                    t = SbxVOID;
                pMeth = GetMethod( aName_, t );
                pMeth->nLine1 = pMeth->nLine2 = nLine1;
                // Die Methode ist erst mal GUELTIG
                pMeth->bInvalid = sal_False;
            }
            else
                eEndTok = NIL;
        }
        // Skip bis END SUB/END FUNCTION
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
                pMeth->nLine2 = aTok.GetLine();
        }
    }
    EndDefinitions( sal_True );
}

void SbModule::SetComment( const String& r )
{
    aComment = r;
    SetModified( sal_True );
}

SbMethod* SbModule::GetFunctionForLine( sal_uInt16 nLine )
{
    for( sal_uInt16 i = 0; i < pMethods->Count(); i++ )
    {
        SbMethod* p = (SbMethod*) pMethods->Get( i );
        if( p->GetSbxId() == SBXID_BASICMETHOD )
        {
            if( nLine >= p->nLine1 && nLine <= p->nLine2 )
                return p;
        }
    }
    return NULL;
}

// Ausstrahlen eines Hints an alle Basics

static void _SendHint( SbxObject* pObj, sal_uIntPtr nId, SbMethod* p )
{
    // Selbst ein BASIC?
    if( pObj->IsA( TYPE(StarBASIC) ) && pObj->IsBroadcaster() )
        pObj->GetBroadcaster().Broadcast( SbxHint( nId, p ) );
    // Dann die Unterobjekte fragen
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

// #57841 Uno-Objekte, die in RTL-Funktionen gehalten werden,
// beim Programm-Ende freigeben, damit nichts gehalten wird.
void ClearUnoObjectsInRTL_Impl_Rek( StarBASIC* pBasic )
{
    // return-Wert von CreateUnoService loeschen
    static String aName( RTL_CONSTASCII_USTRINGPARAM("CreateUnoService") );
    SbxVariable* pVar = pBasic->GetRtl()->Find( aName, SbxCLASS_METHOD );
    if( pVar )
        pVar->SbxValue::Clear();

    // return-Wert von CreateUnoDialog loeschen
    static String aName2( RTL_CONSTASCII_USTRINGPARAM("CreateUnoDialog") );
    pVar = pBasic->GetRtl()->Find( aName2, SbxCLASS_METHOD );
    if( pVar )
        pVar->SbxValue::Clear();

    // return-Wert von CDec loeschen
    static String aName3( RTL_CONSTASCII_USTRINGPARAM("CDec") );
    pVar = pBasic->GetRtl()->Find( aName3, SbxCLASS_METHOD );
    if( pVar )
        pVar->SbxValue::Clear();

    // return-Wert von CreateObject loeschen
    static String aName4( RTL_CONSTASCII_USTRINGPARAM("CreateObject") );
    pVar = pBasic->GetRtl()->Find( aName4, SbxCLASS_METHOD );
    if( pVar )
        pVar->SbxValue::Clear();

    // Ueber alle Sub-Basics gehen
    SbxArray* pObjs = pBasic->GetObjects();
    sal_uInt16 nCount = pObjs->Count();
    for( sal_uInt16 i = 0 ; i < nCount ; i++ )
    {
        SbxVariable* pObjVar = pObjs->Get( i );
        StarBASIC* pSubBasic = PTR_CAST( StarBASIC, pObjVar );
        if( pSubBasic )
            ClearUnoObjectsInRTL_Impl_Rek( pSubBasic );
    }
}

void ClearUnoObjectsInRTL_Impl( StarBASIC* pBasic )
{
    // #67781 Rueckgabewerte der Uno-Methoden loeschen
    clearUnoMethods();
    clearUnoServiceCtors();

    ClearUnoObjectsInRTL_Impl_Rek( pBasic );

    // Oberstes Basic suchen
    SbxObject* p = pBasic;
    while( p->GetParent() )
        p = p->GetParent();
    if( ((StarBASIC*)p) != pBasic )
        ClearUnoObjectsInRTL_Impl_Rek( (StarBASIC*)p );
}
sal_Bool SbModule::IsVBACompat() const
{
    return mbVBACompat;
}

void SbModule::SetVBACompat( sal_Bool bCompat )
{
    mbVBACompat = bCompat;
}
// Ausfuehren eines BASIC-Unterprogramms
sal_uInt16 SbModule::Run( SbMethod* pMeth )
{
    static sal_uInt16 nMaxCallLevel = 0;
    static String aMSOMacroRuntimeLibName = String::CreateFromAscii( "Launcher" );
    static String aMSOMacroRuntimeAppSymbol = String::CreateFromAscii( "Application" );

    sal_uInt16 nRes = 0;
    sal_Bool bDelInst = sal_Bool( pINST == NULL );
    StarBASICRef xBasic;
    if( bDelInst )
    {
#ifdef DBG_TRACE_BASIC
        dbg_InitTrace();
#endif
        // #32779: Basic waehrend der Ausfuehrung festhalten
        xBasic = (StarBASIC*) GetParent();

        pINST = new SbiInstance( (StarBASIC*) GetParent() );

        // Launcher problem
        // i80726 The Find below will genarate an error in Testtool so we reset it unless there was one before already
        sal_Bool bWasError = SbxBase::GetError() != 0;
        SbxVariable* pMSOMacroRuntimeLibVar = Find( aMSOMacroRuntimeLibName, SbxCLASS_OBJECT );
        if ( !bWasError && (SbxBase::GetError() == SbxERR_PROC_UNDEFINED) )
            SbxBase::ResetError();
        if( pMSOMacroRuntimeLibVar )
        {
            StarBASIC* pMSOMacroRuntimeLib = PTR_CAST(StarBASIC,pMSOMacroRuntimeLibVar);
            if( pMSOMacroRuntimeLib )
            {
                sal_uInt16 nGblFlag = pMSOMacroRuntimeLib->GetFlags() & SBX_GBLSEARCH;
                pMSOMacroRuntimeLib->ResetFlag( SBX_GBLSEARCH );
                SbxVariable* pAppSymbol = pMSOMacroRuntimeLib->Find( aMSOMacroRuntimeAppSymbol, SbxCLASS_METHOD );
                pMSOMacroRuntimeLib->SetFlag( nGblFlag );
                if( pAppSymbol )
                {
                    pMSOMacroRuntimeLib->SetFlag( SBX_EXTSEARCH );      // Could have been disabled before
                    GetSbData()->pMSOMacroRuntimLib = pMSOMacroRuntimeLib;
                }
            }
        }

        // Error-Stack loeschen
        SbErrorStack*& rErrStack = GetSbData()->pErrStack;
        delete rErrStack;
        rErrStack = NULL;

        if( nMaxCallLevel == 0 )
        {
#ifdef UNX
          struct rlimit rl;
          getrlimit ( RLIMIT_STACK, &rl );
          // printf( "RLIMIT_STACK = %ld\n", rl.rlim_cur );
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

    // Rekursion zu tief?
    if( ++pINST->nCallLvl <= nMaxCallLevel )
    {
        // Globale Variable in allen Mods definieren
        GlobalRunInit( /* bBasicStart = */ bDelInst );

        // Trat ein Compiler-Fehler auf? Dann starten wir nicht
        if( GetSbData()->bGlobalInitErr == sal_False )
        {
            if( bDelInst )
            {
                SendHint( GetParent(), SBX_HINT_BASICSTART, pMeth );

                // 16.10.96: #31460 Neues Konzept fuer StepInto/Over/Out
                // Erklaerung siehe runtime.cxx bei SbiInstance::CalcBreakCallLevel()
                // BreakCallLevel ermitteln
                pINST->CalcBreakCallLevel( pMeth->GetDebugFlags() );
            }

            SbModule* pOldMod = pMOD;
            pMOD = this;
            SbiRuntime* pRt = new SbiRuntime( this, pMeth, pMeth->nStart );

#ifdef DBG_TRACE_BASIC
            dbg_traceNotifyCall( this, pMeth, pINST->nCallLvl );
#endif

            pRt->pNext = pINST->pRun;
            if( pRt->pNext )
                pRt->pNext->block();
            pINST->pRun = pRt;
            if ( mbVBACompat )
            {
                pINST->EnableCompatibility( sal_True );
            }
            while( pRt->Step() ) {}
            if( pRt->pNext )
                pRt->pNext->unblock();

#ifdef DBG_TRACE_BASIC
            bool bLeave = true;
            dbg_traceNotifyCall( this, pMeth, pINST->nCallLvl, bLeave );
#endif

            // #63710 Durch ein anderes Thread-Handling bei Events kann es passieren,
            // dass show-Aufruf an einem Dialog zurueckkehrt (durch schliessen des
            // Dialogs per UI), BEVOR ein per Event ausgeloester weitergehender Call,
            // der in Basic weiter oben im Stack steht und auf einen Basic-Breakpoint
            // gelaufen ist, zurueckkehrt. Dann wird unten die Instanz zerstoert und
            // wenn das noch im Call stehende Basic weiterlaeuft, gibt es einen GPF.
            // Daher muss hier gewartet werden, bis andere Call zurueckkehrt.
            if( bDelInst )
            {
                // Hier mit 1 statt 0 vergleichen, da vor nCallLvl--
                while( pINST->nCallLvl != 1 )
                    GetpApp()->Yield();
            }

            nRes = sal_True;
            pINST->pRun = pRt->pNext;
            pINST->nCallLvl--;          // Call-Level wieder runter

            // Gibt es eine uebergeordnete Runtime-Instanz?
            // Dann SbDEBUG_BREAK uebernehmen, wenn gesetzt
            SbiRuntime* pRtNext = pRt->pNext;
            if( pRtNext && (pRt->GetDebugFlags() & SbDEBUG_BREAK) )
                pRtNext->SetDebugFlags( SbDEBUG_BREAK );

            delete pRt;
            pMOD = pOldMod;
            if( bDelInst )
            {
                // #57841 Uno-Objekte, die in RTL-Funktionen gehalten werden,
                // beim Programm-Ende freigeben, damit nichts gehalten wird.
                ClearUnoObjectsInRTL_Impl( xBasic );

                clearNativeObjectWrapperVector();

                DBG_ASSERT(pINST->nCallLvl==0,"BASIC-Call-Level > 0");
                delete pINST, pINST = NULL, bDelInst = sal_False;

                // #i30690
                vos::OGuard aSolarGuard( Application::GetSolarMutex() );
                SendHint( GetParent(), SBX_HINT_BASICSTOP, pMeth );

                GlobalRunDeInit();

#ifdef DBG_UTIL
                ResetCapturedAssertions();
#endif

                // VBA always ensures screenupdating is enabled after completing
                if ( mbVBACompat )
                    VBAUnlockControllers( PTR_CAST( StarBASIC, GetParent() ) );

#ifdef DBG_TRACE_BASIC
                dbg_DeInitTrace();
#endif
            }
        }
        else
               pINST->nCallLvl--;           // Call-Level wieder runter
    }
    else
    {
        pINST->nCallLvl--;          // Call-Level wieder runter
        StarBASIC::FatalError( SbERR_STACK_OVERFLOW );
    }

    StarBASIC* pBasic = PTR_CAST(StarBASIC,GetParent());
    if( bDelInst )
    {
        // #57841 Uno-Objekte, die in RTL-Funktionen gehalten werden,
        // beim Programm-Ende freigeben, damit nichts gehalten wird.
        ClearUnoObjectsInRTL_Impl( xBasic );

        delete pINST;
        pINST = NULL;
    }
    if ( pBasic && pBasic->IsDocBasic() && pBasic->IsQuitApplication() && !pINST )
    {
        Application::PostUserEvent( LINK( &AsyncQuitHandler::instance(), AsyncQuitHandler, OnAsyncQuit ), NULL );
    }

    return nRes;
}

// Ausfuehren der Init-Methode eines Moduls nach dem Laden
// oder der Compilation

void SbModule::RunInit()
{
    if( pImage
     && !pImage->bInit
     && pImage->GetFlag( SBIMG_INITCODE ) )
    {
        // Flag setzen, dass RunInit aktiv ist (Testtool)
        GetSbData()->bRunInit = sal_True;

        // sal_Bool bDelInst = sal_Bool( pINST == NULL );
        // if( bDelInst )
            // pINST = new SbiInstance( (StarBASIC*) GetParent() );
        SbModule* pOldMod = pMOD;
        pMOD = this;
        // Der Init-Code beginnt immer hier
        SbiRuntime* pRt = new SbiRuntime( this, NULL, 0 );

#ifdef DBG_TRACE_BASIC
        dbg_traceNotifyCall( this, NULL, 0 );
#endif

        pRt->pNext = pINST->pRun;
        pINST->pRun = pRt;
        while( pRt->Step() ) {}

#ifdef DBG_TRACE_BASIC
        bool bLeave = true;
        dbg_traceNotifyCall( this, NULL, 0, bLeave );
#endif

        pINST->pRun = pRt->pNext;
        delete pRt;
        pMOD = pOldMod;
        // if( bDelInst )
            // delete pINST, pINST = NULL;
        pImage->bInit = sal_True;
        pImage->bFirstInit = sal_False;

        // RunInit ist nicht mehr aktiv
        GetSbData()->bRunInit = sal_False;
    }
}

// Mit private/dim deklarierte Variablen loeschen

void SbModule::AddVarName( const String& aName )
{
    // see if the name is added allready
    std::vector< String >::iterator it_end = mModuleVariableNames.end();
    for ( std::vector< String >::iterator it = mModuleVariableNames.begin(); it != it_end; ++it )
    {
        if ( aName == *it )
            return;
    }
    mModuleVariableNames.push_back( aName );
}

void SbModule::RemoveVars()
{
    std::vector< String >::iterator it_end = mModuleVariableNames.end();
    for ( std::vector< String >::iterator it = mModuleVariableNames.begin(); it != it_end; ++it )
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
            // Arrays nicht loeschen, sondern nur deren Inhalt
            if( p->GetType() & SbxARRAY )
            {
                SbxArray* pArray = PTR_CAST(SbxArray,p->GetObject());
                if( pArray )
                {
                    for( sal_uInt16 j = 0 ; j < pArray->Count() ; j++ )
                    {
                        SbxVariable* pj = PTR_CAST(SbxVariable,pArray->Get( j ));
                        pj->SbxValue::Clear();
                        /*
                        sal_uInt16 nFlags = pj->GetFlags();
                        pj->SetFlags( (nFlags | SBX_WRITE) & (~SBX_FIXED) );
                        pj->PutEmpty();
                        pj->SetFlags( nFlags );
                        */
                    }
                }
            }
            else
            {
                p->SbxValue::Clear();
                /*
                sal_uInt16 nFlags = p->GetFlags();
                p->SetFlags( (nFlags | SBX_WRITE) & (~SBX_FIXED) );
                p->PutEmpty();
                p->SetFlags( nFlags );
                */
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

// Zunaechst in dieses Modul, um 358-faehig zu bleiben
// (Branch in sb.cxx vermeiden)
void StarBASIC::ClearAllModuleVars( void )
{
    // Eigene Module initialisieren
    for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = (SbModule*)pModules->Get( nMod );
        // Nur initialisieren, wenn der Startcode schon ausgefuehrt wurde
        if( pModule->pImage && pModule->pImage->bInit && !pModule->isProxyModule() && !pModule->ISA(SbObjModule) )
            pModule->ClearPrivateVars();
    }

    /* #88042 This code can delete already used public vars during runtime!
    // Alle Objekte ueberpruefen, ob es sich um ein Basic handelt
    // Wenn ja, auch dort initialisieren
    for ( sal_uInt16 nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
        if( pBasic )
            pBasic->ClearAllModuleVars();
    }
    */
}

// Ausfuehren des Init-Codes aller Module
void SbModule::GlobalRunInit( sal_Bool bBasicStart )
{
    // Wenn kein Basic-Start, nur initialisieren, wenn Modul uninitialisiert
    if( !bBasicStart )
        if( !(pImage && !pImage->bInit) )
            return;

    // GlobalInitErr-Flag fuer Compiler-Error initialisieren
    // Anhand dieses Flags kann in SbModule::Run() nach dem Aufruf
    // von GlobalRunInit festgestellt werden, ob beim initialisieren
    // der Module ein Fehler auftrat. Dann wird nicht gestartet.
    GetSbData()->bGlobalInitErr = sal_False;

    // Parent vom Modul ist ein Basic
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

// Suche nach dem naechsten STMNT-Befehl im Code. Wird vom STMNT-
// Opcode verwendet, um die Endspalte zu setzen.

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
            DBG_ASSERT( pImg, "FindNextStmnt: pImg==NULL with FollowJumps option" );
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

// Testen, ob eine Zeile STMNT-Opcodes enthaelt

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

size_t SbModule::GetBPCount() const
{
    return pBreaks ? pBreaks->size() : 0;
}

sal_uInt16 SbModule::GetBP( size_t n ) const
{
    if( pBreaks && n < pBreaks->size() )
        return pBreaks->operator[]( n );
    else
        return 0;
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

    // #38568: Zur Laufzeit auch hier SbDEBUG_BREAK setzen
    if( pINST && pINST->pRun )
        pINST->pRun->SetDebugFlags( SbDEBUG_BREAK );

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
    // Precaution...
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
            // Alte Version: Image weg
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
    sal_Bool bFixup = ( pImage && !pImage->ExceedsLegacyLimits() );
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
        sal_Bool bFixup = ( !nVer && !pImage->ExceedsLegacyLimits() );// save in old image format, fix up method starts

        if ( bFixup ) // save in old image format, fix up method starts
            fixUpMethodStart( true );
         bRet = SbxObject::StoreData( rStrm );
        if( bRet )
        {
            pImage->aOUSource = ::rtl::OUString();
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
//

sal_Bool SbModule::LoadBinaryData( SvStream& rStrm )
{
    ::rtl::OUString aKeepSource = aOUSource;
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
                String aProcName;
                aProcName.AppendAscii( "Property Get " );
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

                    String aProcName;
                    aProcName.AppendAscii( "Property Set " );
                    aProcName += pProcProperty->GetName();
                    pMeth = Find( aProcName, SbxCLASS_METHOD );
                }
                if( !pMeth )    // Let
                {
                    String aProcName;
                    aProcName.AppendAscii( "Property Let " );
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


/////////////////////////////////////////////////////////////////////////
// Implementation SbJScriptModule (Basic-Modul fuer JavaScript-Sourcen)
SbJScriptModule::SbJScriptModule( const String& rName )
    :SbModule( rName )
{
}

sal_Bool SbJScriptModule::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    (void)nVer;

    Clear();
    if( !SbxObject::LoadData( rStrm, 1 ) )
        return sal_False;

    // Source-String holen
    String aTmp;
    rStrm.ReadByteString( aTmp, gsl_getSystemTextEncoding() );
    aOUSource = aTmp;
    //rStrm >> aSource;
    return sal_True;
}

sal_Bool SbJScriptModule::StoreData( SvStream& rStrm ) const
{
    if( !SbxObject::StoreData( rStrm ) )
        return sal_False;

    // Source-String schreiben
    String aTmp = aOUSource;
    rStrm.WriteByteString( aTmp, gsl_getSystemTextEncoding() );
    //rStrm << aSource;
    return sal_True;
}


/////////////////////////////////////////////////////////////////////////

SbMethod::SbMethod( const String& r, SbxDataType t, SbModule* p )
        : SbxMethod( r, t ), pMod( p )
{
    bInvalid     = sal_True;
    nStart       =
    nDebugFlags  =
    nLine1       =
    nLine2       = 0;
    refStatics = new SbxArray;
    // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
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
    SetFlag( SBX_NO_MODIFY );
}

SbMethod::~SbMethod()
{
}

SbxArray* SbMethod::GetLocals()
{
    if( pINST )
        return pINST->GetLocals( this );
    else
        return NULL;
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
    // nDebugFlags = n;     // AB 16.1.96: Nicht mehr uebernehmen
    if( nVer == 2 )
        rStrm >> nLine1 >> nLine2 >> nTempStart >> bInvalid;
    // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
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

// Kann spaeter mal weg

SbxInfo* SbMethod::GetInfo()
{
    return pInfo;
}

// Schnittstelle zum Ausfuehren einer Methode aus den Applikationen
// #34191# Mit speziellem RefCounting, damit das Basic nicht durch CloseDocument()
// abgeschossen werden kann. Rueckgabewert wird als String geliefert.
ErrCode SbMethod::Call( SbxValue* pRet )
{
    // RefCount vom Modul hochzaehlen
    SbModule* pMod_ = (SbModule*)GetParent();
    pMod_->AddRef();

    // RefCount vom Basic hochzaehlen
    StarBASIC* pBasic = (StarBASIC*)pMod_->GetParent();
    pBasic->AddRef();

    // Values anlegen, um Return-Wert zu erhalten
    SbxValues aVals;
    aVals.eType = SbxVARIANT;

    // #104083: Compile BEFORE get
    if( bInvalid && !pMod_->Compile() )
        StarBASIC::Error( SbERR_BAD_PROP_VALUE );

    Get( aVals );
    if ( pRet )
        pRet->Put( aVals );

    // Gab es einen Error
    ErrCode nErr = SbxBase::GetError();
    SbxBase::ResetError();

    // Objekte freigeben
    pMod_->ReleaseRef();
    pBasic->ReleaseRef();

    return nErr;
}


// #100883 Own Broadcast for SbMethod
void SbMethod::Broadcast( sal_uIntPtr nHintId )
{
    if( pCst && !IsSet( SBX_NO_BROADCAST ) && StaticIsEnabledBroadcasting() )
    {
        // Da die Methode von aussen aufrufbar ist, hier noch einmal
        // die Berechtigung testen
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
            // this, als Element 0 eintragen, aber den Parent nicht umsetzen!
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

/////////////////////////////////////////////////////////////////////////

// Implementation SbJScriptMethod (Method-Klasse als Wrapper fuer JavaScript-Funktionen)

SbJScriptMethod::SbJScriptMethod( const String& r, SbxDataType t, SbModule* p )
        : SbMethod( r, t, p )
{
}

SbJScriptMethod::~SbJScriptMethod()
{}


/////////////////////////////////////////////////////////////////////////
SbObjModule::SbObjModule( const String& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsVbaCompatible )
    : SbModule( rName, bIsVbaCompatible )
{
    SetModuleType( mInfo.ModuleType );
    if ( mInfo.ModuleType == script::ModuleType::FORM )
    {
        SetClassName( rtl::OUString::createFromAscii( "Form" ) );
    }
    else if ( mInfo.ModuleObject.is() )
        SetUnoObject( uno::makeAny( mInfo.ModuleObject ) );
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
    if( xServiceInfo->supportsService( rtl::OUString::createFromAscii( "ooo.vba.excel.Worksheet" ) ) )
    {
        SetClassName( rtl::OUString::createFromAscii( "Worksheet" ) );
    }
    else if( xServiceInfo->supportsService( rtl::OUString::createFromAscii( "ooo.vba.excel.Workbook" ) ) )
    {
        SetClassName( rtl::OUString::createFromAscii( "Workbook" ) );
    }
}

SbxVariable*
SbObjModule::GetObject()
{
    return pDocObject;
}
SbxVariable*
SbObjModule::Find( const XubString& rName, SbxClassType t )
{
    //OSL_TRACE("SbObjectModule find for %s", rtl::OUStringToOString(  rName, RTL_TEXTENCODING_UTF8 ).getStr() );
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


typedef ::cppu::WeakImplHelper2< awt::XTopWindowListener, awt::XWindowListener > FormObjEventListener_BASE;

class FormObjEventListenerImpl : public FormObjEventListener_BASE
{
    SbUserFormModule* mpUserForm;
    uno::Reference< lang::XComponent > mxComponent;
    bool mbDisposed;
    sal_Bool mbOpened;
    sal_Bool mbActivated;
    sal_Bool mbShowing;
    FormObjEventListenerImpl(); // not defined
    FormObjEventListenerImpl(const FormObjEventListenerImpl&); // not defined

public:
    FormObjEventListenerImpl( SbUserFormModule* pUserForm, const uno::Reference< lang::XComponent >& xComponent ) :
        mpUserForm( pUserForm ), mxComponent( xComponent) ,
        mbDisposed( false ), mbOpened( sal_False ), mbActivated( sal_False ), mbShowing( sal_False )
    {
        if ( mxComponent.is() )
        {
            OSL_TRACE("*********** Registering the listeners");
            try
            {
                uno::Reference< awt::XTopWindow >( mxComponent, uno::UNO_QUERY_THROW )->addTopWindowListener( this );
            }
            catch( uno::Exception& ) {}
            try
            {
                uno::Reference< awt::XWindow >( mxComponent, uno::UNO_QUERY_THROW )->addWindowListener( this );
            }
            catch( uno::Exception& ) {}
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
            OSL_TRACE("*********** Removing the listeners");
            try
            {
                uno::Reference< awt::XTopWindow >( mxComponent, uno::UNO_QUERY_THROW )->removeTopWindowListener( this );
            }
            catch( uno::Exception& ) {}
            try
            {
                uno::Reference< awt::XWindow >( mxComponent, uno::UNO_QUERY_THROW )->removeWindowListener( this );
            }
            catch( uno::Exception& ) {}
        }
        mxComponent.clear();
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

    //liuchen 2009-7-21, support Excel VBA Form_QueryClose event
    virtual void SAL_CALL windowClosing( const lang::EventObject& /*e*/ ) throw (uno::RuntimeException)
    {
#if IN_THE_FUTURE
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
                    sal_Int8 nCloseMode = 0;

                    Sequence< Any > aParams;
                    aParams.realloc(2);
                    aParams[0] <<= nCancel;
                    aParams[1] <<= nCloseMode;

                    mpUserForm->triggerMethod( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Userform_QueryClose") ),
                                                aParams);
                    xVbaMethodParameter->setVbaMethodParameter( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Cancel")), aParams[0]);
                    return;

                }
            }
        }

        mpUserForm->triggerMethod( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Userform_QueryClose") ) );
#endif
    }
    //liuchen 2009-7-21

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

    virtual void SAL_CALL disposing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException)
    {
        OSL_TRACE("** Userform/Dialog disposing");
        mbDisposed = true;
        mxComponent.clear();
        if ( mpUserForm )
            mpUserForm->ResetApiObj();
    }
};

SbUserFormModule::SbUserFormModule( const String& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsCompat )
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
    if ( bTriggerTerminateEvent && m_xDialog.is() ) // probably someone close the dialog window
    {
        triggerTerminateEvent();
    }
    pDocObject = NULL;
    m_xDialog = NULL;
}

void SbUserFormModule::triggerMethod( const String& aMethodToRun )
{
    Sequence< Any > aArguments;
    triggerMethod( aMethodToRun, aArguments );
}
void SbUserFormModule::triggerMethod( const String& aMethodToRun, Sequence< Any >& /*aArguments*/)
{
    OSL_TRACE("*** trigger %s ***", rtl::OUStringToOString( aMethodToRun, RTL_TEXTENCODING_UTF8 ).getStr() );
    // Search method
    SbxVariable* pMeth = SbObjModule::Find( aMethodToRun, SbxCLASS_METHOD );
    if( pMeth )
    {
#if IN_THE_FUTURE
                 //liuchen 2009-7-21, support Excel VBA UserForm_QueryClose event with parameters
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
//liuchen 2009-7-21
#endif
        {
            SbxValues aVals;
            pMeth->Get( aVals );
        }
    }
}

void SbUserFormModule::triggerActivateEvent( void )
{
    OSL_TRACE("**** entering SbUserFormModule::triggerActivate");
    triggerMethod( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("UserForm_Activate") ) );
    OSL_TRACE("**** leaving SbUserFormModule::triggerActivate");
}

void SbUserFormModule::triggerDeactivateEvent( void )
{
    OSL_TRACE("**** SbUserFormModule::triggerDeactivate");
    triggerMethod( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Userform_Deactivate") ) );
}

void SbUserFormModule::triggerInitializeEvent( void )
{
    if ( mbInit )
        return;
    OSL_TRACE("**** SbUserFormModule::triggerInitializeEvent");
    static String aInitMethodName( RTL_CONSTASCII_USTRINGPARAM("Userform_Initialize") );
    triggerMethod( aInitMethodName );
    mbInit = true;
}

void SbUserFormModule::triggerTerminateEvent( void )
{
    OSL_TRACE("**** SbUserFormModule::triggerTerminateEvent");
    static String aTermMethodName( RTL_CONSTASCII_USTRINGPARAM("Userform_Terminate") );
    triggerMethod( aTermMethodName );
    mbInit=false;
}

void SbUserFormModule::triggerLayoutEvent( void )
{
    static String aMethodName( RTL_CONSTASCII_USTRINGPARAM("Userform_Layout") );
    triggerMethod( aMethodName );
}

void SbUserFormModule::triggerResizeEvent( void )
{
    static String aMethodName( RTL_CONSTASCII_USTRINGPARAM("Userform_Resize") );
    triggerMethod( aMethodName );
}

SbUserFormModuleInstance* SbUserFormModule::CreateInstance()
{
    SbUserFormModuleInstance* pInstance = new SbUserFormModuleInstance( this, GetName(), m_mInfo, IsVBACompat() );
    return pInstance;
}

SbUserFormModuleInstance::SbUserFormModuleInstance( SbUserFormModule* pParentModule,
    const String& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsVBACompat )
        : SbUserFormModule( rName, mInfo, bIsVBACompat )
        , m_pParentModule( pParentModule )
{
}

sal_Bool SbUserFormModuleInstance::IsClass( const XubString& rName ) const
{
    sal_Bool bParentNameMatches = m_pParentModule->GetName().EqualsIgnoreCaseAscii( rName );
    sal_Bool bRet = bParentNameMatches || SbxObject::IsClass( rName );
    return bRet;
}

SbxVariable* SbUserFormModuleInstance::Find( const XubString& rName, SbxClassType t )
{
    SbxVariable* pVar = m_pParentModule->Find( rName, t );
    return pVar;
}


void SbUserFormModule::Load()
{
    OSL_TRACE("** load() ");
    // forces a load
    if ( !pDocObject )
        InitObject();
}

//liuchen 2009-7-21 change to accmordate VBA's beheavior
void SbUserFormModule::Unload()
{
    OSL_TRACE("** Unload() ");

    sal_Int8 nCancel = 0;
    sal_Int8 nCloseMode = 1;

    Sequence< Any > aParams;
    aParams.realloc(2);
    aParams[0] <<= nCancel;
    aParams[1] <<= nCloseMode;

    triggerMethod( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Userform_QueryClose") ), aParams);

    aParams[0] >>= nCancel;
    if (nCancel == 1)
    {
        return;
    }

    if ( m_xDialog.is() )
    {
        triggerTerminateEvent();
    }
    // Search method
    SbxVariable* pMeth = SbObjModule::Find( String( RTL_CONSTASCII_USTRINGPARAM( "UnloadObject" ) ), SbxCLASS_METHOD );
    if( pMeth )
    {
        OSL_TRACE("Attempting too run the UnloadObjectMethod");
        m_xDialog.clear(); //release ref to the uno object
        SbxValues aVals;
        bool bWaitForDispose = true; // assume dialog is showing
        if ( m_DialogListener.get() )
        {
            bWaitForDispose = m_DialogListener->isShowing();
            OSL_TRACE("Showing %d", bWaitForDispose );
        }
        pMeth->Get( aVals);
        if ( !bWaitForDispose )
        {
            // we've either already got a dispose or we'er never going to get one
            ResetApiObj();
        } // else wait for dispose
        OSL_TRACE("UnloadObject completed ( we hope )");
    }
}
//liuchen

void registerComponentToBeDisposedForBasic( Reference< XComponent > xComponent, StarBASIC* pBasic );

void SbUserFormModule::InitObject()
{
    try
    {
        String aHook( RTL_CONSTASCII_USTRINGPARAM( "VBAGlobals" ) );
        SbUnoObject* pGlobs = (SbUnoObject*)GetParent()->Find( aHook, SbxCLASS_DONTCARE );
        if ( m_xModel.is() && pGlobs )
        {

            uno::Reference< lang::XMultiServiceFactory > xVBAFactory( pGlobs->getUnoAny(), uno::UNO_QUERY_THROW );
            uno::Reference< lang::XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
            uno::Sequence< uno::Any > aArgs(1);
            aArgs[ 0 ] <<= m_xModel;
            rtl::OUString sDialogUrl( RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.script:" ) );
            rtl::OUString sProjectName( RTL_CONSTASCII_USTRINGPARAM("Standard") );
            if ( this->GetParent()->GetName().Len() )
                sProjectName = this->GetParent()->GetName();
            sDialogUrl = sDialogUrl.concat( sProjectName ).concat( rtl::OUString( '.') ).concat( GetName() ).concat( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("?location=document") ) );

            uno::Reference< awt::XDialogProvider > xProvider( xFactory->createInstanceWithArguments( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.DialogProvider")), aArgs  ), uno::UNO_QUERY_THROW );
            m_xDialog = xProvider->createDialog( sDialogUrl );

            // create vba api object
            aArgs.realloc( 4 );
            aArgs[ 0 ] = uno::Any();
            aArgs[ 1 ] <<= m_xDialog;
            aArgs[ 2 ] <<= m_xModel;
            aArgs[ 3 ] <<= rtl::OUString( GetParent()->GetName() );
            pDocObject = new SbUnoObject( GetName(), uno::makeAny( xVBAFactory->createInstanceWithArguments( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.UserForm")), aArgs  ) ) );
            uno::Reference< lang::XComponent > xComponent( aArgs[ 1 ], uno::UNO_QUERY_THROW );

            // the dialog must be disposed at the end!
            if( xComponent.is() )
            {
                StarBASIC* pParentBasic = NULL;
                SbxObject* pCurObject = this;
                do
                {
                    SbxObject* pObjParent = pCurObject->GetParent();
                    pParentBasic = PTR_CAST( StarBASIC, pObjParent );
                    pCurObject = pObjParent;
                }
                while( pParentBasic == NULL && pCurObject != NULL );

                OSL_ASSERT( pParentBasic != NULL );
                registerComponentToBeDisposedForBasic( xComponent, pParentBasic );
            }


            // remove old listener if it exists
            if ( m_DialogListener.get() )
                m_DialogListener->removeListener();
            m_DialogListener = new FormObjEventListenerImpl( this, xComponent );

            triggerInitializeEvent();
        }
    }
    catch( uno::Exception& )
    {
    }

}

SbxVariable*
SbUserFormModule::Find( const XubString& rName, SbxClassType t )
{
    if ( !pDocObject && !GetSbData()->bRunInit && pINST )
        InitObject();
    return SbObjModule::Find( rName, t );
}
/////////////////////////////////////////////////////////////////////////

SbProperty::SbProperty( const String& r, SbxDataType t, SbModule* p )
        : SbxProperty( r, t ), pMod( p )
{
    bInvalid = sal_False;
}

SbProperty::~SbProperty()
{}

/////////////////////////////////////////////////////////////////////////

SbProcedureProperty::~SbProcedureProperty()
{}

