
#include <stdio.h>

#include <cppuhelper/factory.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>


#include <com/sun/star/bridge/XUnoUrlResolver.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>


using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::loader;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;


using namespace ::cppu;
using namespace ::rtl;

namespace dynamic_loader {

    class SingleServiceFactory : public WeakImplHelper2<XServiceInfo, XSingleServiceFactory> {
        Reference<XMultiServiceFactory> _xServiceManager;
        OUString _serviceName;
        OUString _link;
        OUString _resolver;

        Reference<XSingleServiceFactory> getRemoteFactory() throw(Exception, RuntimeException);

    public:

        SingleServiceFactory(const Reference<XMultiServiceFactory > & xServiceManager,
                             const OUString & serviceName,
                             const OUString & link,
                             const OUString & resolver)
            : _xServiceManager(xServiceManager),
              _serviceName(serviceName),
              _link(link),
              _resolver(resolver)
        {}

        // XSingleServiceFactory
        Reference<XInterface> SAL_CALL createInstance() throw(Exception, RuntimeException);
        Reference<XInterface> SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
            throw(::com::sun::star::uno::Exception,
                  ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        OUString           SAL_CALL getImplementationName()                      throw(RuntimeException);
        sal_Bool           SAL_CALL supportsService(const OUString& ServiceName) throw(RuntimeException);
        Sequence<OUString> SAL_CALL getSupportedServiceNames(void)               throw(RuntimeException);
    };


    Reference<XSingleServiceFactory> SingleServiceFactory::getRemoteFactory() throw(Exception, RuntimeException) {
        Reference<XUnoUrlResolver> xResolver(_xServiceManager->createInstance(_resolver), UNO_QUERY);
        if(!xResolver.is()) {
            OUString message(RTL_CONSTASCII_USTRINGPARAM("dynamic_loader::singleServiceFactory.createInstance - couldn't create resolver: "));
            message += _resolver;

            throw Exception(message, Reference<XInterface>());
        }

        Reference<XInterface> remoteObject = xResolver->resolve(_link);
        if(!remoteObject.is()) {
            OUString message(RTL_CONSTASCII_USTRINGPARAM("dynamic_loader::singleServiceFactory.createInstance - couldn't resolve link: "));
            message += _link;

            throw Exception(message, Reference<XInterface>());
        }

        Reference<XSingleServiceFactory> remoteFactory(remoteObject, UNO_QUERY);
        if(!remoteFactory.is()) {
            OUString message(RTL_CONSTASCII_USTRINGPARAM("dynamic_loader::singleServiceFactory.createInstance - couldn't get XSingleServiceFactory from: "));
            message += _link;

            throw Exception(message, Reference<XInterface>());
        }

        return remoteFactory;
    }

    // XSingleServiceFactory
    Reference<XInterface> SAL_CALL SingleServiceFactory::createInstance() throw(Exception, RuntimeException) {
        OSL_TRACE("dynamic_loader::singleServiceFactory::createInstance");

        return getRemoteFactory()->createInstance();
    }

    Reference<XInterface> SAL_CALL SingleServiceFactory::createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(Exception, RuntimeException)
    {
        OSL_TRACE("dynamic_loader::singleServiceFactory::createInstanceWithArguments");

        return getRemoteFactory()->createInstanceWithArguments(Arguments);
    }

    // XServiceInfo
    OUString SAL_CALL SingleServiceFactory::getImplementationName() throw(RuntimeException) {
        return _link;
    }

    sal_Bool SAL_CALL SingleServiceFactory::supportsService(const OUString & ServiceName) throw(RuntimeException) {
        return _serviceName.equals(ServiceName);
    }

    Sequence<OUString> SAL_CALL SingleServiceFactory::getSupportedServiceNames(void) throw(RuntimeException) {
        return Sequence<OUString>(&_serviceName, 1);
    }



    class DynamicLoader : public WeakImplHelper2<XImplementationLoader, XServiceInfo> {
        Reference<XMultiServiceFactory>  _xSMgr;

    protected:
        DynamicLoader(const Reference<XMultiServiceFactory> & rXSMgr) throw(RuntimeException);
        ~DynamicLoader();

    public:
        static const OUString implname;
        static const OUString servname;

        static Reference<XInterface> SAL_CALL createInstance(const Reference<XMultiServiceFactory> & rSMgr) throw(Exception);
        static Sequence<OUString>    SAL_CALL getSupportedServiceNames_Static() throw();

        static void parseUrl(const OUString & url, OUString * serviceName, OUString * link, OUString * resolver) throw(RuntimeException);

        // XServiceInfo
        virtual OUString           SAL_CALL getImplementationName()                      throw(RuntimeException);
        virtual sal_Bool           SAL_CALL supportsService(const OUString& ServiceName) throw(RuntimeException);
        virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()                   throw(RuntimeException);

        // XImplementationLoader
        virtual Reference<XInterface> SAL_CALL activate(const OUString & implementationName,
                                                        const OUString & implementationLoaderUrl,
                                                        const OUString& locationUrl,
                                                        const Reference<XRegistryKey>& xKey)       throw(CannotActivateFactoryException, RuntimeException);
        virtual sal_Bool              SAL_CALL writeRegistryInfo(const Reference<XRegistryKey>& xKey,
                                                                 const OUString& implementationLoaderUrl,
                                                                 const OUString& locationUrl)      throw(CannotRegisterImplementationException, RuntimeException);
    };

    const OUString DynamicLoader::implname = OUString::createFromAscii("com.sun.star.comp.stoc.DynamicLoader");
    const OUString DynamicLoader::servname = OUString::createFromAscii("com.sun.star.loader.Dynamic");

    Sequence<OUString> SAL_CALL DynamicLoader::getSupportedServiceNames_Static() throw() {
        return Sequence<OUString>(&servname, 1);
    }

    Reference<XInterface> SAL_CALL DynamicLoader::createInstance(const Reference<XMultiServiceFactory> & rSMgr) throw(Exception) {
        Reference<XInterface> xRet;

        try {
            XImplementationLoader *pXLoader = (XImplementationLoader *)new DynamicLoader(rSMgr);

            xRet = Reference<XInterface>::query(pXLoader);
        }
        catch(RuntimeException & runtimeException) {
            OString message = OUStringToOString(runtimeException.Message, RTL_TEXTENCODING_ASCII_US);
            osl_trace("dynamic loader - could not init cause of %s", message.getStr());
        }

        OSL_TRACE("DynamicLoader - createInstance: %d", xRet.is());


        return xRet;
    }

    DynamicLoader::DynamicLoader(const Reference<XMultiServiceFactory> & xSMgr) throw(RuntimeException)
        : _xSMgr(xSMgr)
    {
    }


    DynamicLoader::~DynamicLoader() throw() {
    }

    // XServiceInfo
    OUString SAL_CALL DynamicLoader::getImplementationName() throw(RuntimeException) {
        return implname;
    }

    sal_Bool SAL_CALL DynamicLoader::supportsService(const OUString & ServiceName) throw(RuntimeException)  {
        sal_Bool bSupport = sal_False;

        Sequence<OUString> aSNL = getSupportedServiceNames();
        const OUString * pArray = aSNL.getArray();
        for(sal_Int32 i = 0; i < aSNL.getLength() && !bSupport; ++ i)
            bSupport = pArray[i] == ServiceName;

        return bSupport;
    }

    Sequence<OUString> SAL_CALL DynamicLoader::getSupportedServiceNames() throw(RuntimeException) {
        return getSupportedServiceNames_Static();
    }


    void DynamicLoader::parseUrl(const OUString & locationUrl, OUString * serviceName, OUString * link, OUString * resolver) throw(RuntimeException) {
#ifdef DEBUG
        OString tmp = OUStringToOString(locationUrl, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("DynamicLoader - locationUrl %s", tmp.getStr());
#endif

        // This is the default resolver
        *resolver = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.UnoUrlResolver"));

        const OUString bSlash(OUString(RTL_CONSTASCII_USTRINGPARAM("\\")));
        const OUString tuedle(OUString(RTL_CONSTASCII_USTRINGPARAM("\"")));
        const OUString separator(OUString(RTL_CONSTASCII_USTRINGPARAM(",")));
        const OUString emptyString(OUString(RTL_CONSTASCII_USTRINGPARAM("")));
        const OUString equalSign(OUString(RTL_CONSTASCII_USTRINGPARAM("=")));

        sal_Int32 index = 0;
        sal_Bool  left = sal_True;
        sal_Bool  quote = sal_False;
        sal_Bool  inString = sal_False;

        const sal_Unicode * raw_url = locationUrl.getStr();
        OUString token;
        OUString attribute;

        while(index <= locationUrl.getLength()) {
            if(index >= locationUrl.getLength() || (raw_url[index] == separator.getStr()[0] && !quote && !inString)) { // a separator or end?
                OUString value;

                if(left)
                    attribute = token.trim();

                else
                    value = token.trim();

#ifdef DEBUG
                OString attribute_tmp = OUStringToOString(attribute, RTL_TEXTENCODING_ASCII_US);
                OSL_TRACE("DynamicLoader - attribute %s", attribute_tmp.getStr());
                OString value_tmp = OUStringToOString(value, RTL_TEXTENCODING_ASCII_US);
                OSL_TRACE("DynamicLoader - value %s", value_tmp.getStr());
#endif

                if(attribute.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("servicename"))))
                    *serviceName = value;

                else if(attribute.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("link"))))
                    *link = value;

                else if(attribute.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("resolver"))))
                    *resolver = value;

                else  {
                    OUString message(RTL_CONSTASCII_USTRINGPARAM("help called"));

                    if(!attribute.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("help")))) {
                        message = OUString(RTL_CONSTASCII_USTRINGPARAM("DynamicLoader - unknown attribute: "));
                        message += attribute;
                    }

                    fprintf(stdout, "DynamicLoader - help\n");
                    fprintf(stdout, "attributes:\n");
                    fprintf(stdout, "\tservicename:  service name of dynamic component\n");
                    fprintf(stdout, "\tlink:         link to a single service factory for dynamic component\n");
                    fprintf(stdout, "\tresolver:     the service which resolves the link\n");
                    fprintf(stdout, "\thelp:         this help\n");

                    throw RuntimeException(message, Reference<XInterface>());
                }

                left = sal_True; // reset state to be left
                token = emptyString;
            }
            else if(raw_url[index] == bSlash.getStr()[0] && !quote) // a back slash?
                quote = sal_True;

            else if(raw_url[index] == equalSign.getStr()[0] && !quote && !inString) { // equalSign (switch from left to right)?
                left = sal_False;

                attribute = token.trim();
                token = emptyString;
            }
            else if(raw_url[index] == tuedle.getStr()[0] && !quote) // begin or end of string?
                inString = !inString;

            else { // no special handling
                token += OUString(raw_url + index, 1);
                quote = sal_False;
            }

            ++ index;
        }

        // enshure, that attributes are set properly
        if(!(*serviceName).getLength())
            throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("dynamic_loader::DynamicLoader.parseUrl - missing or empty attribute: servicename")),
                                   Reference<XInterface>());

        if(!(*link).getLength())
            throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("dynamic_loader::DynamicLoader.parseUrl - missing or empty attribute: link")),
                                   Reference<XInterface>());

        if(!(*resolver).getLength())
            throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("dynamic_loader::DynamicLoader.parseUrl - missing or empty attribute: resolver")),
                                   Reference<XInterface>());
    }


    // XImplementationLoader
    sal_Bool SAL_CALL DynamicLoader::writeRegistryInfo(const Reference<XRegistryKey> & xKey,
                                                       const OUString & implementationLoaderUrl,
                                                       const OUString & locationUrl)
        throw(CannotRegisterImplementationException, RuntimeException)
    {
        OSL_TRACE("DynamicLoader::writeRegistryInfo");

        OUString serviceName;
        OUString link;
        OUString resolver;

        try {
            parseUrl(locationUrl, &serviceName, &link, &resolver);
        }
        catch(RuntimeException & runtimeException) {
            throw CannotRegisterImplementationException(runtimeException.Message, Reference<XInterface>());
        }

        // create the keys
        OUString keyName = OUString::createFromAscii("/");
        keyName += implementationLoaderUrl;
        keyName += OUString(RTL_CONSTASCII_USTRINGPARAM("_"));
        keyName += serviceName;
        keyName += OUString::createFromAscii("/UNO/SERVICES");

        Reference<XRegistryKey> xNewKey(xKey->createKey(keyName));
        xNewKey->createKey(serviceName);

        sal_Bool bSuccess = sal_True;

        return bSuccess;
    }

    Reference<XInterface> SAL_CALL DynamicLoader::activate(const OUString & rImplName,
                                                           const OUString & loaderUrl,
                                                           const OUString & locationUrl,
                                                           const Reference<XRegistryKey> & xKey)
        throw(CannotActivateFactoryException, RuntimeException)
    {
        OSL_TRACE("DynamicLoader::activate");

        OUString serviceName;
        OUString link;
        OUString resolver;

        parseUrl(locationUrl, &serviceName, &link, &resolver);

        XSingleServiceFactory * xFactory = (XSingleServiceFactory *)new SingleServiceFactory(_xSMgr,
                                                                                             serviceName,
                                                                                             link,
                                                                                             resolver);

          Reference<XInterface> xReturn;

        if(xFactory)
            xReturn = Reference<XInterface>::query(xFactory);

        return xReturn;
    }
}




extern "C" {
    SAL_DLLEXPORT void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)   {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    SAL_DLLEXPORT sal_Bool SAL_CALL component_writeInfo(XMultiServiceFactory * pServiceManager, XRegistryKey * pRegistryKey) {
        sal_Bool bRes = sal_False;

        if (pRegistryKey) {
            try {
                OUString x = OUString::createFromAscii("/");
                x += ::dynamic_loader::DynamicLoader::implname;
                x += OUString::createFromAscii("/UNO/SERVICES");


                Reference<XRegistryKey> xNewKey(pRegistryKey->createKey(x));

                const Sequence<OUString> rSNL = ::dynamic_loader::DynamicLoader::getSupportedServiceNames_Static();
                const OUString * pArray = rSNL.getConstArray();
                for (sal_Int32 nPos = rSNL.getLength(); nPos--;)
                    xNewKey->createKey(pArray[nPos]);


                bRes = sal_True;
            }
            catch (InvalidRegistryException &) {
                  OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
            }
        }

        return bRes;
    }

    SAL_DLLEXPORT void * SAL_CALL component_getFactory(const sal_Char * pImplName, XMultiServiceFactory * pServiceManager, XRegistryKey * pRegistryKey) {
        void * pRet = 0;

        if (pServiceManager && OUString::createFromAscii(pImplName).equals(::dynamic_loader::DynamicLoader::implname)) {
            Reference<XSingleServiceFactory> xFactory(createOneInstanceFactory(pServiceManager,
                                                                               ::dynamic_loader::DynamicLoader::implname,
                                                                               ::dynamic_loader::DynamicLoader::createInstance,
                                                                               ::dynamic_loader::DynamicLoader::getSupportedServiceNames_Static()));

            if (xFactory.is()) {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        else
            OSL_TRACE("DynamicLoader - warning - given wrong implName: %s", pImplName);

        return pRet;
    }
}
