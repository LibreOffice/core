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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "res_services.hxx"

#include <osl/mutex.hxx>
#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/factory.hxx>   // helper for factories
#include <cppuhelper/implbase3.hxx> // helper for implementations

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>

#include <tools/resmgr.hxx>
#include <tools/rcid.h>
#include <tools/resary.hxx>
#include <vcl/svapp.hxx>

#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::script;
using namespace com::sun::star::beans;
using namespace com::sun::star::reflection;

using ::rtl::OUString;
using ::rtl::OStringBuffer;
using ::rtl::OUStringToOString;
using ::rtl::OStringToOUString;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class ResourceService : public cppu::WeakImplHelper3< XInvocation, XExactName, XServiceInfo >
{
public:
                                ResourceService( const Reference< XMultiServiceFactory > & );
                                ~ResourceService();

    // XServiceInfo
    OUString SAL_CALL           getImplementationName() throw();
    sal_Bool     SAL_CALL           supportsService(const OUString& ServiceName) throw();
    Sequence< OUString > SAL_CALL   getSupportedServiceNames(void) throw();

    static Sequence< OUString > getSupportedServiceNames_Static(void) throw();
    static OUString             getImplementationName_Static() throw()
                                {
                                    return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.extensions.ResourceService"));
                                }
    static Reference< XInterface > Create( const Reference< XComponentContext >& _rxContext );

    // XExactName
    OUString  SAL_CALL          getExactName( const OUString & ApproximateName ) throw(RuntimeException);

    // XInvokation
    Reference< XIntrospectionAccess >  SAL_CALL getIntrospection(void)  throw(RuntimeException);
    Any  SAL_CALL               invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam) throw(IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException);
    void  SAL_CALL              setValue(const OUString& PropertyName, const Any& Value) throw(UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException);
    Any  SAL_CALL               getValue(const OUString& PropertyName) throw(UnknownPropertyException, RuntimeException);
    sal_Bool  SAL_CALL              hasMethod(const OUString& Name)  throw(RuntimeException);
    sal_Bool  SAL_CALL              hasProperty(const OUString& Name)  throw(RuntimeException);
private:
    Reference< XTypeConverter >         getTypeConverter() const;
    Reference< XInvocation >            getDefaultInvocation() const;

    Reference< XMultiServiceFactory >   xSMgr;
    Reference< XInvocation >            xDefaultInvocation;
    Reference< XTypeConverter >         xTypeConverter;
    OUString                                aFileName;
    ResMgr *                            pResMgr;
};


//-----------------------------------------------------------------------------
ResourceService::ResourceService( const Reference< XMultiServiceFactory > & rSMgr )
    : xSMgr( rSMgr )
    , pResMgr( NULL )
{
}

//-----------------------------------------------------------------------------
Reference< XInterface > ResourceService::Create( const Reference< XComponentContext >& _rxContext )
{
    Reference< XMultiServiceFactory > xFactory( _rxContext->getServiceManager(), UNO_QUERY_THROW );
    return *( new ResourceService( xFactory ) );
}

//-----------------------------------------------------------------------------
ResourceService::~ResourceService()
{
    delete pResMgr;
}

// XServiceInfo
OUString ResourceService::getImplementationName() throw()
{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool SAL_CALL ResourceService::supportsService(const OUString& ServiceName) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// XServiceInfo
Sequence< OUString > SAL_CALL ResourceService::getSupportedServiceNames(void) throw()
{
    return getSupportedServiceNames_Static();
}

// ResourceService
Sequence< OUString > ResourceService::getSupportedServiceNames_Static(void) throw()
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.resource.VclStringResourceLoader"));
    return aSNS;
}

// ResourceService
Reference< XTypeConverter > ResourceService::getTypeConverter() const
{
    SolarMutexGuard aGuard;
    if( xSMgr.is() )
    {
        Reference< XTypeConverter > xConv( xSMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter" ))), UNO_QUERY );
        ((ResourceService*)this)->xTypeConverter = xConv;
    }
    return xTypeConverter;
}

// ResourceService
Reference< XInvocation > ResourceService::getDefaultInvocation() const
{
    SolarMutexGuard aGuard;
    /* f�hrt zur Zeit noch zu einer rekursion
    if( xSMgr.is() )
    {
        Reference< XSingleServiceFactory > xFact( xSMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Invocation")) ), UNO_QUERY );
        if( xFact.is() )
        {
            Sequence< Any > aArgs( 1 );
            Reference< XInterface > xThis( *this );
            aArgs.getArray()[0].set( &xThis, XInterface_Reference< get >lection() );
            Reference< XInvokation > xI( xFact->createInstanceWithArguments( aArgs ), UNO_QUERY );
            ((ResourceService*)this)->xDefaultInvocation = xI;
        }
    }
    */
    return xDefaultInvocation;
}

// XExactName
OUString    SAL_CALL ResourceService::getExactName( const OUString & ApproximateName ) throw(RuntimeException)
{
    OUString aName( ApproximateName );
    aName = aName.toAsciiLowerCase();
    if( aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("filename")) )
        return OUString(RTL_CONSTASCII_USTRINGPARAM("FileName"));
    else if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("getstring" ) ))
        return OUString(RTL_CONSTASCII_USTRINGPARAM("getString"));
    else if( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("getstrings" ) ))
        return OUString(RTL_CONSTASCII_USTRINGPARAM("getStrings"));
    else if( aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("hasstring")) )
        return OUString(RTL_CONSTASCII_USTRINGPARAM("hasString"));
    else if( aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("hasstrings")) )
        return OUString(RTL_CONSTASCII_USTRINGPARAM("hasStrings"));
    else if( aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getstringlist")) )
        return OUString(RTL_CONSTASCII_USTRINGPARAM("getStringList"));
    else if( aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("hasStringList")) )
        return OUString(RTL_CONSTASCII_USTRINGPARAM("hasStringList"));
    Reference< XExactName > xEN( getDefaultInvocation(), UNO_QUERY );
    if( xEN.is() )
        return xEN->getExactName( ApproximateName );
    return OUString();
}

// XInvokation
Reference< XIntrospectionAccess > SAL_CALL ResourceService::getIntrospection(void)
    throw(RuntimeException)
{
    Reference< XInvocation > xI = getDefaultInvocation();
    if( xI.is() )
        return xI->getIntrospection();
    return Reference< XIntrospectionAccess >();
}

// XInvokation
Any SAL_CALL ResourceService::invoke
(
    const OUString& FunctionName,
    const Sequence< Any >& Params,
    Sequence< sal_Int16 >& OutParamIndex,
    Sequence< Any >& OutParam
)
    throw(IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException)
{
    Any aRet;
    if( FunctionName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getString"))
        || FunctionName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("getStrings" ) )
        || FunctionName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("hasString" ) )
        || FunctionName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("hasStrings" ) )
        )
    {
        sal_Int32 nElements = Params.getLength();
        if( nElements < 1 )
            throw IllegalArgumentException();
        if( nElements > 1 && (FunctionName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getString")) || FunctionName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("hasString")) ) )
            throw IllegalArgumentException();
        if( !pResMgr )
            throw IllegalArgumentException();

        Sequence< OUString > aStrings( Params.getLength() );
        Sequence< sal_Bool > aBools( Params.getLength() );
        const Any* pIn = Params.getConstArray();
        OUString* pOutString = aStrings.getArray();
        sal_Bool* pOutBool = aBools.getArray();

        Reference< XTypeConverter > xC = getTypeConverter();
        bool bGetBranch = FunctionName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getString" ) ) || FunctionName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getStrings" ) );

        SolarMutexGuard aGuard;
        for( sal_Int32 n = 0; n < nElements; n++ )
        {
            sal_Int32 nId = 0;
            if( !(pIn[n] >>= nId) )
            {
                if( xC.is() )
                {
                    xC->convertToSimpleType( pIn[n], TypeClass_LONG ) >>= nId;
                }
                else
                    throw CannotConvertException();
            }
            if( nId > 0xFFFF || nId < 0 )
                throw IllegalArgumentException();

            if( bGetBranch )
            {
                ResId aId( (sal_uInt16)nId, *pResMgr );
                aId.SetRT( RSC_STRING );
                if( pResMgr->IsAvailable( aId ) )
                {
                    String aStr( aId );
                    pOutString[n] = aStr;
                }
                else
                    throw IllegalArgumentException();
            }
            else // hasString(s)
            {
                sal_Bool bRet = sal_False;
                if( pResMgr )
                {
                    ResId aId( (sal_uInt16)nId, *pResMgr );
                    aId.SetRT( RSC_STRING );
                    bRet = pResMgr->IsAvailable( aId );
                }
                pOutBool[n] = bRet;
            }
        }
        if( FunctionName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getString")) )
            aRet <<= pOutString[0];
        else if( FunctionName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("getStrings" ) ) )
            aRet <<= aStrings;
        else if( FunctionName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("hasString" ) ) )
            aRet <<= pOutBool[0];
        else
            aRet <<= aBools;
    }
    else if( FunctionName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getStringList")) || FunctionName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("hasStringList" ) ) )
    {
        if( Params.getLength() != 1 )
            throw IllegalArgumentException();
        Reference< XTypeConverter > xC = getTypeConverter();
        SolarMutexGuard aGuard;

        sal_Int32 nId = 0;
        if( !(Params.getConstArray()[0] >>= nId) )
        {
            if( xC.is() )
            {
                xC->convertToSimpleType( Params.getConstArray()[0], TypeClass_LONG ) >>= nId;
            }
            else
                throw CannotConvertException();
        }

        if( FunctionName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getStringList")) )
        {
            ResId aId( (sal_uInt16)nId, *pResMgr );
            aId.SetRT( RSC_STRINGARRAY );
            if( pResMgr->IsAvailable( aId ) )
            {
                ResStringArray aStr( aId );
                int nEntries = aStr.Count();
                Sequence< PropertyValue > aPropSeq( nEntries );
                PropertyValue* pOut = aPropSeq.getArray();
                for( int i = 0; i < nEntries; i++ )
                {
                    pOut[i].Name        = aStr.GetString( i );
                    pOut[i].Handle      = -1;
                    pOut[i].Value     <<= aStr.GetValue( i );
                    pOut[i].State       = PropertyState_DIRECT_VALUE;
                }
                aRet <<= aPropSeq;
            }
            else
                throw IllegalArgumentException();
        }
        else // hasStringList
        {
            sal_Bool bRet = sal_False;
            if( pResMgr )
            {
                ResId aId( (sal_uInt16)nId, *pResMgr );
                aId.SetRT( RSC_STRINGARRAY );
                bRet = pResMgr->IsAvailable( aId );
            }
            aRet <<= bRet;
        }
    }
    else
    {
        Reference< XInvocation > xI = getDefaultInvocation();
        if( xI.is() )
            return xI->invoke( FunctionName, Params, OutParamIndex, OutParam );
        else
            throw IllegalArgumentException();
    }
    return aRet;
}

// XInvokation
void SAL_CALL ResourceService::setValue(const OUString& PropertyName, const Any& Value)
    throw(UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException)
{
    if( PropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("FileName")) )
    {
        OUString aName;
        if( !(Value >>= aName) )
        {
            Reference< XTypeConverter > xC = getTypeConverter();
            if( xC.is() )
                xC->convertToSimpleType( Value, TypeClass_STRING ) >>= aName;
            else
                throw CannotConvertException();
        }

        SolarMutexGuard aGuard;
        OStringBuffer aBuf( aName.getLength()+8 );
        aBuf.append( OUStringToOString( aName, osl_getThreadTextEncoding() ) );
        ResMgr * pRM = ResMgr::CreateResMgr( aBuf.getStr() );
        if( !pRM )
            throw InvocationTargetException();
        if( pResMgr )
            delete pResMgr;
        pResMgr = pRM;
        aFileName = OStringToOUString( aBuf.makeStringAndClear(), osl_getThreadTextEncoding() );
    }
    else
    {
        Reference< XInvocation > xI = getDefaultInvocation();
        if( xI.is() )
            xI->setValue( PropertyName, Value );
        else
            throw UnknownPropertyException();
    }
}

// XInvokation
Any SAL_CALL ResourceService::getValue(const OUString& PropertyName)
    throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("FileName" ) ))
        return makeAny( aFileName );

    Reference< XInvocation > xI = getDefaultInvocation();
    if( xI.is() )
        return xI->getValue( PropertyName );

    throw UnknownPropertyException();
}

// XInvokation
sal_Bool SAL_CALL ResourceService::hasMethod(const OUString& Name)
    throw(RuntimeException)
{
    if( Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getString"))     ||
        Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getStrings"))    ||
        Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("hasString"))     ||
        Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("hasStrings"))    ||
        Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getStringList")) ||
        Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("hasStringList"))
        )
        return sal_True;
    else
    {
        Reference< XInvocation > xI = getDefaultInvocation();
        if( xI.is() )
            return xI->hasMethod( Name );
        else
            return sal_False;
    }
}

// XInvokation
sal_Bool SAL_CALL ResourceService::hasProperty(const OUString& Name)
    throw(RuntimeException)
{
    if( Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("FileName")) )
        return sal_True;
    else
    {
        Reference< XInvocation > xI = getDefaultInvocation();
        if( xI.is() )
            return xI->hasProperty( Name );
        else
            return sal_False;
    }
}

namespace res
{
    ComponentInfo getComponentInfo_VclStringResourceLoader()
    {
        ComponentInfo aInfo;
        aInfo.aSupportedServices = ResourceService::getSupportedServiceNames_Static();
        aInfo.sImplementationName = ResourceService::getImplementationName_Static();
        aInfo.pFactory = &ResourceService::Create;
        return aInfo;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
