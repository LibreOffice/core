/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase2.hxx>

#include "../tools/fastserializer.hxx"
#include "fastparser.hxx"

using namespace sax_fastparser;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using ::rtl::OUString;
using namespace ::com::sun::star::lang;

namespace sax_fastparser
{

//--------------------------------------
// the extern interface
//---------------------------------------
Reference< XInterface > SAL_CALL FastSaxParser_CreateInstance( const Reference< XMultiServiceFactory  >  & ) throw(Exception)
{
    FastSaxParser *p = new FastSaxParser;
    return Reference< XInterface > ( (OWeakObject * ) p );
}

Reference< XInterface > SAL_CALL FastSaxSerializer_CreateInstance( const Reference< XMultiServiceFactory  >  & ) throw(Exception)
{
    FastSaxSerializer *p = new FastSaxSerializer;
    return Reference< XInterface > ( (OWeakObject * ) p );
}
}

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

            Reference< XRegistryKey > xNewKey( xKey->createKey(
                OUString::createFromAscii( "/" PARSER_IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );
            xNewKey->createKey( OUString::createFromAscii( PARSER_SERVICE_NAME ) );

            Reference< XRegistryKey > xNewKey1( xKey->createKey(
                OUString::createFromAscii( "/" SERIALIZER_IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );
            xNewKey1->createKey( OUString::createFromAscii( SERIALIZER_SERVICE_NAME ) );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    if (pServiceManager )
    {
        Reference< XSingleServiceFactory > xRet;
        Reference< XMultiServiceFactory > xSMgr( reinterpret_cast< XMultiServiceFactory * > ( pServiceManager ) );

        OUString aImplementationName( OUString::createFromAscii( pImplName ) );

        if (aImplementationName == OUString( RTL_CONSTASCII_USTRINGPARAM( PARSER_IMPLEMENTATION_NAME  ) ) )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        FastSaxParser_CreateInstance,
                                        FastSaxParser::getSupportedServiceNames_Static() );
        }
        else if (aImplementationName == OUString( RTL_CONSTASCII_USTRINGPARAM( SERIALIZER_IMPLEMENTATION_NAME  ) ) )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        FastSaxSerializer_CreateInstance,
                                        FastSaxSerializer::getSupportedServiceNames_Static() );
        }

        if (xRet.is())
        {
            xRet->acquire();
            pRet = xRet.get();
        }
    }

    return pRet;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
