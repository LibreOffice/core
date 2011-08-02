// MARKER(update_precomp.py): autogen include statement, do not remove
#include <comphelper/processfactory.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <cppuhelper/implbase2.hxx>
#include "uno/mapping.hxx"
#include "osl/file.hxx"
#include "glib/gtypes.h"

// for debug
#include <comphelper/anytostring.hxx>

extern "C" {
#include <mono/jit/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>
}

using namespace ::com::sun::star;
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

//static const char CLIURE_DLL[] = "$URE_LIB_DIR/cli_ure.dll";
static const char CLIURE_DLL[] = "$URE_INTERNAL_LIB_DIR/cli_ure.dll";
typedef ::cppu::WeakImplHelper2< loader::XImplementationLoader, lang::XServiceInfo > MonoLoader_BASE;

namespace mono_loader
{
    ::rtl::OUString SAL_CALL getImplementationName();

    uno::Reference< uno::XInterface > SAL_CALL create( uno::Reference< uno::XComponentContext > const & xContext ) SAL_THROW( () );

    uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();
}

uno::Reference< loader::XImplementationLoader >
create_object (MonoDomain *domain, MonoImage *image)
{
        MonoClass *klass;

        klass = mono_class_from_name (image, "uno.util", "ManagedCodeLoader");
        if (!klass) {
                OSL_TRACE ("Can't find ManagedCodeLoader in assembly %s", mono_image_get_filename (image));
                return NULL;
        }
        MonoObject* obj = mono_object_new (domain, klass);
        /* mono_object_new () only allocates the storage:
         * it doesn't run any constructor. Tell the runtime to run
         * the default argumentless constructor.
         */
        mono_runtime_object_init (obj);
        static uno::Reference< loader::XImplementationLoader > xLoader;
        // not sure if this is correct ( I'm loosely following 'to_uno'
        // method in cli_ure/source/native/native_share.h )

        loader::XImplementationLoader* pLoader = NULL;
        OSL_TRACE("About to call mapInterface for XImplementionLoader returned from Mono");
        // we are storing the object so... I guess we need to tell the gc not
        // to bother about this object
        //mono_gchandle_new( obj, false ); // where do we release that ? do we even need to do this?
        guint32 nHandle = mono_gchandle_new( obj, true ); // where do we release that ? do we even need to do this?
        uno::Mapping mapping( OUSTR( UNO_LB_CLI ), OUSTR( CPPU_CURRENT_LANGUAGE_BINDING_NAME ) );
        OSL_ASSERT( mapping.is() );
        if (! mapping.is() )
            return NULL;

        mapping.mapInterface(
            reinterpret_cast< void ** >( &pLoader ),
            reinterpret_cast< void * >( obj ), ::getCppuType( &xLoader ) );
        mono_gchandle_free ( nHandle ); // copying what cli_ure/source/native/native_share.h does for DotNet
        xLoader.set( pLoader, SAL_NO_ACQUIRE /* takeover ownership */ );
        OSL_TRACE("We appear to have got an XImplementationLoader that has a value? %s", xLoader.is() ? "yes" : "no " );
        return xLoader;
}


class MonoLoader : public MonoLoader_BASE
{
    class MonoCleanUp
    {
    public:
        MonoCleanUp() { OSL_TRACE("MonoCleanUp created "); }
        ~MonoCleanUp()
        {
            OSL_TRACE("~MonoCleanUp");
            // loader only uses the root domain
            mono_jit_cleanup (mono_get_root_domain());
        }
    };
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< loader::XImplementationLoader > mxLoader;
    uno::Reference< util::XMacroExpander > mxExpander;
    uno::Reference< loader::XImplementationLoader > getLoader( const char* file)
    {
        OSL_TRACE("** enter getLoader()");
        // init only once
        static MonoDomain* domain = mono_jit_init (file);
        // when is a good time to trigger clean up ?
        //static MonoCleanUp cleaner;
        // hmm appears we need to attach this thread to the domain
        mono_thread_attach( domain );
        MonoAssembly *assembly;

        assembly = mono_domain_assembly_open ( domain, file);
        OSL_TRACE("** open of assembly %s = 0x%x", file, assembly);
        if ( !assembly )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to open assembly " ) ) + rtl::OUString::createFromAscii( file ), NULL );
        return create_object (domain, mono_assembly_get_image (assembly));
    }


public:
    MonoLoader( const uno::Reference< uno::XComponentContext >& rxContext ) : mxContext( rxContext )
    {
        if (!(mxContext->getValueByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander"))) >>= mxExpander)
        || !mxExpander.is())
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "component context fails to supply singleton" " com.sun.star.util.theMacroExpander of type" " com.sun.star.util.XMacroExpander")), mxContext);
        }

        rtl::OUString dllUrlPath =  mxExpander->expandMacros( rtl::OUString(  RTL_CONSTASCII_USTRINGPARAM( CLIURE_DLL ) ) );
        rtl::OUString dllPath;
        if ( osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(
                dllUrlPath, dllPath ))
        {
            throw uno::RuntimeException(
                OUSTR("cannot get system path from file url ") +
                dllPath,
                uno::Reference< uno::XInterface >() );
        }

        OSL_TRACE("**** location for dll is %s", rtl::OUStringToOString( dllPath, RTL_TEXTENCODING_UTF8 ).getStr() );
        OSL_TRACE("** MonoLoader::MonoLoader() ");
        mxLoader = getLoader( rtl::OUStringToOString( dllPath, RTL_TEXTENCODING_UTF8 ).getStr() );
        if ( mxLoader.is() )
        {
            // set the service factory
            uno::Sequence< uno::Any > args(1);
            args[ 0 ] <<= rxContext->getServiceManager();
            uno::Reference< lang::XInitialization > xInitialize( mxLoader, uno::UNO_QUERY_THROW );
            OSL_TRACE("MonoLoader::MonoLoader() about to call initialise");
            xInitialize->initialize( args );
        }
        else
            OSL_TRACE("**  MonoLoader::MonoLoader(): No Mono loader found ");

    }
    ~MonoLoader()
    {
        OSL_TRACE("** MonoLoader::~MonoLoader() ");
    }
    // Methods
    virtual uno::Reference< uno::XInterface > SAL_CALL activate( const ::rtl::OUString& implementationName, const ::rtl::OUString& implementationLoaderUrl, const ::rtl::OUString& locationUrl, const uno::Reference< registry::XRegistryKey >& xKey ) throw (loader::CannotActivateFactoryException, uno::RuntimeException)
    {
        // try to instatiate a mono loader and return a reference to it
        OSL_TRACE("**** in MonoLoader::activate");
        if ( mxLoader.is() )
        {
            OSL_TRACE("*** MonoLoader::activate() about to call activate on 0x%x", mxLoader.get() );
            return mxLoader->activate( implementationName, implementationLoaderUrl, locationUrl, xKey );
        }
        return NULL;
    }

    virtual ::sal_Bool SAL_CALL writeRegistryInfo( const uno::Reference< registry::XRegistryKey >& xKey, const ::rtl::OUString& implementationLoaderUrl, const ::rtl::OUString& locationUrl ) throw (registry::CannotRegisterImplementationException, uno::RuntimeException)
    {
        if ( mxLoader.is() )
            return mxLoader->writeRegistryInfo( xKey, implementationLoaderUrl, locationUrl );
        return sal_False;
    }
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (uno::RuntimeException){ return mono_loader::getImplementationName(); }
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        sal_Bool bRes = sal_False;
        uno::Sequence< ::rtl::OUString > sServices = mono_loader::getSupportedServiceNames();
        const ::rtl::OUString* pService = sServices.getConstArray();
        const ::rtl::OUString* pEnd = sServices.getConstArray() + sServices.getLength();
        for ( ; pService != pEnd ;  ++pService )
        {
            if ( (*pService).equals( ServiceName ) )
            {
                bRes = sal_True;
                break;
            }
        }
        return bRes;
    }
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (uno::RuntimeException){ return mono_loader::getSupportedServiceNames(); }

};

namespace mono_loader
{
    ::rtl::OUString SAL_CALL getImplementationName()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.loader.MonoLoader" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    uno::Reference< uno::XInterface > SAL_CALL create(
    uno::Reference< uno::XComponentContext > const & xContext )
    SAL_THROW( () )
    {
        OSL_TRACE("** In create for monoloader");
        // mimic java loader if I read it correctly it just has a single entry
        // point ( Mono implementation loader should do the same? #TODO maybe )
        // #FIXME use whatever boiler plate static initialisatioon foo that is
        // available ( seem to recall there is some helpers for that )
        // static uno::Reference < lang::XTypeProvider  > xLoader( new MonoLoader( xContext ) );
        // hmm lets not do it for now because I'm not sure how an exiting/shutting down office/bridge co-operates
        // with the mono runtime or if it does at all :/
        uno::Reference < lang::XTypeProvider  > xLoader( new MonoLoader( xContext ) );
        return xLoader;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
    {
        const ::rtl::OUString strName( ::mono_loader::getImplementationName() );
        return uno::Sequence< ::rtl::OUString >( &strName, 1 );
    }
}
