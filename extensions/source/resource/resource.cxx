/*************************************************************************
 *
 *  $RCSfile: resource.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pl $ $Date: 2002-05-29 14:58:01 $
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

#include <vos/mutex.hxx>
#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/factory.hxx>   // helper for factories
#include <cppuhelper/implbase3.hxx> // helper for implementations

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#include <com/sun/star/beans/XExactName.hpp>

#include <tools/resmgr.hxx>
#include <tools/rcid.h>
#include <vcl/svapp.hxx>

#include <rtl/ustring.hxx>

using namespace vos;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::script;
using namespace com::sun::star::beans;
using namespace com::sun::star::reflection;

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
    BOOL     SAL_CALL           supportsService(const OUString& ServiceName) throw();
    Sequence< OUString > SAL_CALL   getSupportedServiceNames(void) throw();

    static Sequence< OUString >  getSupportedServiceNames_Static(void) throw();
    static OUString             getImplementationName_Static() throw()
                                {
                                    return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.extensions.ResourceService"));
                                }

    // XExactName
    OUString  SAL_CALL          getExactName( const OUString & ApproximateName ) throw(RuntimeException);

    // XInvokation
    Reference< XIntrospectionAccess >  SAL_CALL getIntrospection(void)  throw(RuntimeException);
    Any  SAL_CALL               invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam) throw(IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException);
    void  SAL_CALL              setValue(const OUString& PropertyName, const Any& Value) throw(UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException);
    Any  SAL_CALL               getValue(const OUString& PropertyName) throw(UnknownPropertyException, RuntimeException);
    BOOL  SAL_CALL              hasMethod(const OUString& Name)  throw(RuntimeException);
    BOOL  SAL_CALL              hasProperty(const OUString& Name)  throw(RuntimeException);
private:
    Reference< XTypeConverter >         getTypeConverter() const;
    Reference< XInvocation >            getDefaultInvocation() const;

    Reference< XMultiServiceFactory >   xSMgr;
    Reference< XInvocation >            xDefaultInvocation;
    Reference< XTypeConverter >         xTypeConverter;
    OUString                                aFileName;
    ResMgr *                            pResMgr;
};


ResourceService::ResourceService( const Reference< XMultiServiceFactory > & rSMgr )
    : xSMgr( rSMgr )
    , pResMgr( NULL )
{
}

//-----------------------------------------------------------------------------
ResourceService::~ResourceService()
{
    delete pResMgr;
}

//-----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ResourceService_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject*)new ResourceService( rSMgr );
    return xService;
}

// XServiceInfo
OUString ResourceService::getImplementationName() throw()
{
    return getImplementationName_Static();
}

// XServiceInfo
BOOL SAL_CALL ResourceService::supportsService(const OUString& ServiceName) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;
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
    OGuard aGuard( Application::GetSolarMutex() );
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
    OGuard aGuard( Application::GetSolarMutex() );
    /* führt zur Zeit noch zu einer rekursion
    if( xSMgr.is() )
    {
        Reference< XSingleServiceFactory > xFact( xSMgr->createInstance( OUString::createFromAscii("com.sun.star.script.Invocation") ), UNO_QUERY );
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
    if( aName.equalsAscii("filename") )
        return OUString(RTL_CONSTASCII_USTRINGPARAM("FileName"));
    else if( aName.equalsAscii("getstring" ))
        return OUString(RTL_CONSTASCII_USTRINGPARAM("getString"));
    else if( aName.equalsAscii("getstrings" ))
        return OUString(RTL_CONSTASCII_USTRINGPARAM("getStrings"));
    else if( aName.equalsAscii("hasstring") )
        return OUString(RTL_CONSTASCII_USTRINGPARAM("hasString"));
    else if( aName.equalsAscii("hasstrings") )
        return OUString(RTL_CONSTASCII_USTRINGPARAM("hasStrings"));
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
    if( FunctionName.equalsAscii("getString")
        || FunctionName.equalsAscii("getStrings" )
        || FunctionName.equalsAscii("hasString" )
        || FunctionName.equalsAscii("hasStrings" )
        )
    {
        sal_Int32 nElements = Params.getLength();
        if( nElements < 1 )
            throw IllegalArgumentException();
        if( nElements > 1 && (FunctionName.equalsAscii("getString") || FunctionName.equalsAscii("hasString") ) )
            throw IllegalArgumentException();
        if( !pResMgr )
            throw IllegalArgumentException();

        Sequence< OUString > aStrings( Params.getLength() );
        Sequence< sal_Bool > aBools( Params.getLength() );
        const Any* pIn = Params.getConstArray();
        OUString* pOutString = aStrings.getArray();
        sal_Bool* pOutBool = aBools.getArray();

        Reference< XTypeConverter > xC = getTypeConverter();
        bool bGetBranch = FunctionName.equalsAscii( "getString" ) || FunctionName.equalsAscii( "getStrings" );

        OGuard aGuard( Application::GetSolarMutex() );
        for( sal_Int32 n = 0; n < nElements; n++ )
        {
            sal_Int32 nId;
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
                ResId aId( (USHORT)nId, pResMgr );
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
                    ResId aId( (USHORT)nId, pResMgr );
                    aId.SetRT( RSC_STRING );
                    bRet = pResMgr->IsAvailable( aId );
                }
                pOutBool[n] = bRet;
            }
        }
        if( FunctionName.equalsAscii("getString") )
            aRet <<= pOutString[0];
        else if( FunctionName.equalsAscii("getStrings" ) )
            aRet <<= aStrings;
        else if( FunctionName.equalsAscii("hasString" ) )
            aRet <<= pOutBool[0];
        else
            aRet <<= aBools;
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
    if( PropertyName == OUString(RTL_CONSTASCII_USTRINGPARAM("FileName")) )
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

        OGuard aGuard( Application::GetSolarMutex() );
        aName = aName + OUString( OUString::valueOf( (sal_Int32)SUPD ).getStr() );
        String aTmpString( aName );
        ByteString a( aTmpString, gsl_getSystemTextEncoding() );
        ResMgr * pRM = ResMgr::CreateResMgr( a.GetBuffer() );
        if( !pRM )
            throw InvocationTargetException();
        if( pResMgr )
            delete pResMgr;
        pResMgr = pRM;
        aFileName = aName;
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
    OGuard aGuard( Application::GetSolarMutex() );
    if( PropertyName.equalsAscii("FileName" ))
        return makeAny( aFileName );
    else
    {
        Reference< XInvocation > xI = getDefaultInvocation();
        if( xI.is() )
            return xI->getValue( PropertyName );
        else
            throw UnknownPropertyException();
    }
    return Any();
}

// XInvokation
BOOL SAL_CALL ResourceService::hasMethod(const OUString& Name)
    throw(RuntimeException)
{
    if( Name.equalsAscii("getString") || Name.equalsAscii("getStrings") || Name.equalsAscii("hasString") )
        return TRUE;
    else
    {
        Reference< XInvocation > xI = getDefaultInvocation();
        if( xI.is() )
            return xI->hasMethod( Name );
        else
            return FALSE;
    }
}

// XInvokation
BOOL SAL_CALL ResourceService::hasProperty(const OUString& Name)
    throw(RuntimeException)
{
    if( Name.equalsAscii("FileName") )
        return TRUE;
    else
    {
        Reference< XInvocation > xI = getDefaultInvocation();
        if( xI.is() )
            return xI->hasProperty( Name );
        else
            return FALSE;
    }
}


extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void * /*pServiceManager*/, XRegistryKey * pRegistryKey )
{
    try
    {
        Reference< XRegistryKey > xNewKey =
            pRegistryKey->createKey(
            OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + ResourceService::getImplementationName_Static() + OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES" )));
        Sequence< OUString > aServices = ResourceService::getSupportedServiceNames_Static();
        for( sal_Int32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i]);

        return sal_True;
    }
    catch (Exception &)
    {
        // not allowed to throw an exception over the c function.
        //OSL_ENSURE( sal_False, "Exception cannot register component!" );
        return sal_False;
    }
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, XMultiServiceFactory * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;
    if (!ResourceService::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        // create the factory
        Reference< XSingleServiceFactory > xFactory =
            cppu::createSingleFactory(
                pServiceManager, ResourceService::getImplementationName_Static(),
                ResourceService_CreateInstance,
                ResourceService::getSupportedServiceNames_Static() );
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}

