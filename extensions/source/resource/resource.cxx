/*************************************************************************
 *
 *  $RCSfile: resource.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
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
//#include <usr/ustring.hxx>
//#include <usr/factoryhlp.hxx>
//#include <usr/weak.hxx>

//#include <smart/com/sun/star/lang/XServiceInfo.hxx>
//#include <smart/com/sun/star/script/XInvocation.hxx>
//#include <smart/com/sun/star/script/XTypeConverter.hxx>
//#include <smart/com/sun/star/beans/XExactName.hxx>

// Changes from ...Ref to Reference< ... >, USR_QUERY -> UNO_QUERY, THROWS( ... ) -> throw( ... ),
// UsrAny -> Any, (BOOL -> sal_BOOL, ...), OUString -> OUString, put SAL_CALL infront of each interface method,
// UsrSystemException -> RuntimeException, const is removed from each interface method
// are done without comment
/**** NEW ****/
#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/factory.hxx>   // helper for factories
#include <cppuhelper/implbase3.hxx> // helper for implementations

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#include <com/sun/star/beans/XExactName.hpp>

/**** END NEW ****/

#include <tools/resmgr.hxx>
#include <tools/rcid.h>
#include <vcl/svapp.hxx>

#include <rtl/ustring.hxx>

#ifndef _VOS_NO_NAMESPACE
using namespace vos;
//using namespace usr;
using namespace rtl;
/**** NEW ****/
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::script;
using namespace com::sun::star::beans;
using namespace com::sun::star::reflection;
/**** END NEW ****/
#endif


//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//class ResourceService : public XInvocation
//                      , public XExactName
//                      , public XServiceInfo
//                      , public OWeakObject

/**** NEW ****/
class ResourceService : public cppu::WeakImplHelper3< XInvocation, XExactName, XServiceInfo >
/**** END NEW ****/
{
public:
                                ResourceService( const Reference< XMultiServiceFactory > & );
                                ~ResourceService();

    // XInterface
    //BOOL                      queryInterface( Uik aUik, Reference< XInterface > & rOut );
    //void                      acquire()                        { OWeakObject::acquire(); }
    //void                      release()                        { OWeakObject::release(); }
    //void*                         getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

    // XIdlClassProvider
    //Sequence< Reference< XIdlClass > >    getIdlClasses(void);

    // XServiceInfo
    OUString SAL_CALL           getImplementationName() throw();
    BOOL     SAL_CALL           supportsService(const OUString& ServiceName) throw();
    Sequence< OUString > SAL_CALL   getSupportedServiceNames(void) throw();

    static Sequence< OUString >  getSupportedServiceNames_Static(void) throw();
    static OUString             getImplementationName_Static() throw()
                                {
                                    return OUString::createFromAscii("com.sun.star.comp.extensions.ResourceService");
                                }

    // XExactName
    OUString  SAL_CALL          getExactName( const OUString & ApproximateName );

    // XInvokation
    Reference< XIntrospectionAccess >  SAL_CALL getIntrospection(void)  throw(RuntimeException);
    Any  SAL_CALL               invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< INT16 >& OutParamIndex, Sequence< Any >& OutParam) throw(IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException);
    void  SAL_CALL              setValue(const OUString& PropertyName, const Any& Value) throw(UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException);
    Any  SAL_CALL               getValue(const OUString& PropertyName) throw(UnknownPropertyException, RuntimeException);
    BOOL  SAL_CALL              hasMethod(const OUString& Name)  throw(RuntimeException);
    BOOL  SAL_CALL              hasProperty(const OUString& Name)  throw(RuntimeException);
private:
    Reference< XTypeConverter >         getTypeConverter() const;
    Reference< XInvocation >            getDefaultInvocation() const;
    //Reference< XIdlClass >            getStaticIdlClass();

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

//*************************************************************************
// DLLImplementationLoader::getStaticIdlClass()
//
//Reference< XIdlClass > ResourceService::getStaticIdlClass()
//{
//  // Global Method, must be guarded (multithreading)
//  OGuard aGuard( OMutex::getGlobalMutex() );
//  {
//      // use the standard class implementation of the usr library
//      static Reference< XIdlClass > xClass =
//      createStandardClass( getImplementationName_Static(),
//                      OWeakObject::getStaticIdlClass(), 2,
//                      XInvocation_Reference< get >lection(),
//                      XExactName_getReflection(),
//                      XServiceInfo_getReflection()
//                      );
//      return xClass;
//  }
//}

// Methoden von XInterface
//BOOL ResourceService::queryInterface( Uik aUik, Reference< XInterface > & rOut )
//{
//  if( aUik == XInvocation::getSmartUik() )
//      rOut = (XInvocation*)this;
//  else if( aUik == XExactName::getSmartUik() )
//      rOut = (XExactName*)this;
//  else if( aUik == XServiceInfo::getSmartUik() )
//      rOut = (XServiceInfo *)this;
//  else
//      OWeakObject::queryInterface( aUik, rOut );
//  return rOut.is();
//}
//
// XIdlClassProvider
//Sequence< XIdlClassRef > ResourceService::getIdlClasses()
//{
//  XIdlClassRef x = getStaticIdlClass();
//  return Sequence< XIdlClassRef >( &x, 1 );
//}


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
    for( INT32 i = 0; i < aSNL.getLength(); i++ )
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
    aSNS.getArray()[0] = OUString::createFromAscii("com.sun.star.resource.VclStringResourceLoader");
    return aSNS;
}

// ResourceService
Reference< XTypeConverter > ResourceService::getTypeConverter() const
{
    OGuard aGuard( Application::GetSolarMutex() );
    if( xSMgr.is() )
    {
        Reference< XTypeConverter > xConv( xSMgr->createInstance( OUString::createFromAscii("com.sun.star.script.Converter" )), UNO_QUERY );
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
OUString    SAL_CALL ResourceService::getExactName( const OUString & ApproximateName )
{
    OUString aName( ApproximateName );
    aName = aName.toLowerCase();
    if( aName == OUString::createFromAscii("filename") )
        return OUString::createFromAscii("FileName");
    else if( aName == OUString::createFromAscii("getstring" ))
        return OUString::createFromAscii("getString");
    else if( aName == OUString::createFromAscii("hasstring") )
        return OUString::createFromAscii("hasString");
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
    Sequence< INT16 >& OutParamIndex,
    Sequence< Any >& OutParam
)
    throw(IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException)
{
    if( FunctionName == OUString::createFromAscii("getString") || FunctionName == OUString::createFromAscii("hasString" ))
    {
        if( Params.getLength() != 1 )
            throw IllegalArgumentException();
        INT32 nId;
        /**** NEW ****/
        if( !(Params.getConstArray()[0] >>= nId) )
        /**** END NEW ****/
        //if( Params.getConstArray()[0].getReflection()->getTypeClass() == TypeClass_LONG )
        //  nId = Params.getConstArray()[0].getINT32();
        //else
        {
            Reference< XTypeConverter > xC = getTypeConverter();
            if( xC.is() )
            {
                /**** NEW ****/
                xC->convertToSimpleType( Params.getConstArray()[0], TypeClass_LONG ) >>= nId;
                /**** END NEW ****/
                //nId = xC->convertToSimpleType( Params.getConstArray()[0], TypeClass_LONG ).getINT32();
            }
            else
                throw CannotConvertException();
        }
        if( nId > 0xFFFF || nId < 0 )
            throw IllegalArgumentException();

        OGuard aGuard( Application::GetSolarMutex() );
        if( FunctionName == OUString::createFromAscii("getString" ))
        {
            if( !pResMgr )
                throw IllegalArgumentException();
            ResId aId( (USHORT)nId, pResMgr );
            aId.SetRT( RSC_STRING );
            if( pResMgr->IsAvailable( aId ) )
            {
                String aStr( aId );
        /**** NEW ****/
//              return makeAny( StringToOUString( aStr, CHARSET_SYSTEM ) );
                return makeAny( OUString( aStr ) );
        /**** END NEW ****/
                //return Any( StringToOWString( aStr, CHARSET_SYSTEM ) );
            }
            else
                throw IllegalArgumentException();
        }
        else //if( FunctionName == OUString::createFromAscii("hasString") )
        {
            sal_Bool bRet = sal_False;
            if( pResMgr )
            {
                ResId aId( (USHORT)nId, pResMgr );
                aId.SetRT( RSC_STRING );
                bRet = pResMgr->IsAvailable( aId );
            }
            //return UsrAny( bRet );
            /**** NEW ****/
            return Any( &bRet, getBooleanCppuType() );
            /**** END NEW ****/
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
    return Any();
}

// XInvokation
void SAL_CALL ResourceService::setValue(const OUString& PropertyName, const Any& Value)
    throw(UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException)
{
    if( PropertyName == OUString::createFromAscii("FileName") )
    {
        OUString aName;
        //if( Value.getReflection()->getTypeClass() == TypeClass_STRING )
        //  aName = Value.getString();
        //else
        /**** NEW ****/
        if( !(Value >>= aName) )
        /**** END NEW ****/
        {
            Reference< XTypeConverter > xC = getTypeConverter();
            if( xC.is() )
                xC->convertToSimpleType( Value, TypeClass_STRING ) >>= aName;
                //aName = xC->convertToSimpleType( Value, TypeClass_STRING ).getString();
            else
                throw CannotConvertException();
        }

        OGuard aGuard( Application::GetSolarMutex() );
        aName = aName + OUString( OUString::valueOf( (INT32)SUPD ).getStr() );
//      ResMgr * pRM = ResMgr::CreateResMgr( OUStringToString( aName, CHARSET_SYSTEM ) );
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
    if( PropertyName == OUString::createFromAscii("FileName" ))
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
    if( Name == OUString::createFromAscii("getString") || Name == OUString::createFromAscii("hasString") )
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
    if( Name == OUString::createFromAscii("FileName") )
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

// NEW: return the environment type name of the c++ compiler
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//BOOL EXTERN_SERVICE_CALLTYPE exService_writeRegEntry(
//        const UNO_INTERFACE(XRegistryKey)* xUnoKey)
sal_Bool SAL_CALL component_writeInfo( void * /*pServiceManager*/, XRegistryKey * pRegistryKey )
{
    //XRegistryKeyRef   pRegistryKey;
    //uno2smart(pRegistryKey, *xUnoKey);
    /**** NEW ****/
    try
    {
        Reference< XRegistryKey > xNewKey =
    /**** END NEW ****/
            pRegistryKey->createKey(
            OUString::createFromAscii( "/" ) + ResourceService::getImplementationName_Static() + OUString::createFromAscii( "/UNO/SERVICES" ) );
        Sequence< OUString > aServices = ResourceService::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i]);

        return sal_True;
    /**** NEW ****/
    }
    catch (Exception &)
    {
        // not allowed to throw an exception over the c function.
        //OSL_ENSHURE( sal_False, "Exception cannot register component!" );
        return sal_False;
    }
    /**** END NEW ****/

}

//UNO_INTERFACE(XInterface) EXTERN_SERVICE_CALLTYPE exService_getFactory
//(
//  const sal_Unicode* pImplName,
//  const UNO_INTERFACE(XMultiServiceFactory)* pServiceManager,
//  const UNO_INTERFACE(XRegistryKey)*
//)
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, XMultiServiceFactory * pServiceManager, void * /*pRegistryKey*/ )
{
    //UNO_INTERFACE(XInterface) xUnoRet = {0, 0};
    //XInterfaceRef             xRet;
    //XMultiServiceFactoryRef xSMgr;
    //OUString                  aImplementationName(pImplName);
    //uno2smart(xSMgr, *xUnoFact);
    //if (OUString(pImplName) == ResourceService::getpImplName_Static() )
    //{
    //  xRet = createSingleFactory( pServiceManager, pImplName,
    //                              ResourceService_CreateInstance,
    //                              ResourceService::getSupportedServiceNames_Static() );
    //}
    //
    //if (xRet.is())
    //{
    //  smart2uno(xRet, xUnoRet);
    //}
    //return xUnoRet;

    /**** NEW ****/
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
    /**** END NEW ****/
}

}

