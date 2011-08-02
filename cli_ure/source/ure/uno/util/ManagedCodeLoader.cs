using System;
using System.Reflection;

using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.registry;

namespace uno.util
{
  

// loader for cs components
public class ManagedCodeLoader : uno.util.WeakBase, unoidl.com.sun.star.loader.XImplementationLoader, unoidl.com.sun.star.lang.XServiceInfo, unoidl.com.sun.star.lang.XInitialization
{
    private unoidl.com.sun.star.lang.XMultiServiceFactory multiServiceFactory;
    private String[] supportedServices = {
        "com.sun.star.loader.ManagedCodeLoader"
    };

    private unoidl.com.sun.star.util.XMacroExpander m_xMacroExpander = null;
    private String EXPAND_PROTOCOL_PREFIX = "vnd.sun.star.expand:";

    /** Expands macrofied url using the macro expander singleton.
     */
    private String expand_url( String url )
    {
            Console.WriteLine( "#1 expand_url " + url );

        if (url != null && url.StartsWith( EXPAND_PROTOCOL_PREFIX ))
        {
            try
            {
                if (m_xMacroExpander == null)
                {
                    Console.WriteLine( "#2 attempt to get macroexpander ");
                    unoidl.com.sun.star.beans.XPropertySet xProps = ( unoidl.com.sun.star.beans.XPropertySet ) multiServiceFactory;
                    if (xProps == null)
                    {
                        throw new unoidl.com.sun.star.uno.RuntimeException(
                            "service manager does not support XPropertySet!",
                            this );
                    }
                    unoidl.com.sun.star.uno.XComponentContext xContext = (unoidl.com.sun.star.uno.XComponentContext) xProps.getPropertyValue( "DefaultContext" ).Value;
                    m_xMacroExpander = ( unoidl.com.sun.star.util.XMacroExpander )xContext.getValueByName( "/singletons/com.sun.star.util.theMacroExpander" ).Value;
                    Console.WriteLine( "#3 got macroexpander ");
                }
                // decode uric class chars
                String macro = System.Uri.UnescapeDataString( url.Substring( EXPAND_PROTOCOL_PREFIX.Length ).Replace( "+", "%2B" ) );
                Console.WriteLine( "#4 decoded url " + macro);
                // expand macro string
                String ret = m_xMacroExpander.expandMacros( macro );
                Console.WriteLine( "#5 decoded & expanded url " + ret);
                return ret;
            }
            catch (unoidl.com.sun.star.uno.Exception exc)
            {
                throw new unoidl.com.sun.star.uno.RuntimeException(
                    exc.ToString(), this );
            }
            catch ( System.Exception exc)
            {
                throw new unoidl.com.sun.star.uno.RuntimeException(
                    exc.ToString(), this );
            }
        }
        return url;
    }

    // XImplementationLoader 
    public System.Object activate(String implementationName, String implementationLoaderUrl, String locationUrl, unoidl.com.sun.star.registry.XRegistryKey key )
    {
        locationUrl = expand_url( locationUrl ); 
        Console.WriteLine( "*** *** ManagedCodeLoader.activate( " +  implementationName + ", " + implementationLoaderUrl + ", " + locationUrl + ") ****" );
        // implementationName will be the class ( or Type ) name
        // locationUrl is the name of the assembly it will be in
        
        // here's a cheap and nasty facimile of what the java loader does
        Type clazz = null;

        try
        {
            clazz = RegistrationClassFinder.find( locationUrl );
        }
        catch (System.NullReferenceException e)
        {
            throw new unoidl.com.sun.star.loader.CannotActivateFactoryException( "can not activate exception because " + implementationName + "\nexc: " + e, null );
        }
        catch (System.Exception e)
        {
            throw new unoidl.com.sun.star.loader.CannotActivateFactoryException( "can not activate exception because " + implementationName + "\nexc: " + e, null );
        }

        System.Object returnObject = null;
        MethodInfo compfac_method;
        try
        {
            Type[] compParams = { typeof(String) };
            compfac_method =  clazz.GetMethod( "__getComponentFactory" , compParams );
            if ( compfac_method != null )
            {
                Object ret = compfac_method.Invoke( clazz, new Object [] { implementationName } );
                if ( ret != null )
                    returnObject = ( unoidl.com.sun.star.lang.XSingleComponentFactory )ret;
            }
    
        }
        catch ( System.Exception e )
        {
            throw new unoidl.com.sun.star.loader.CannotActivateFactoryException( "Failed to activate factory for " + implementationName + "\nexc: " + e, null );
        }
        return returnObject;
    }
    public bool writeRegistryInfo(unoidl.com.sun.star.registry.XRegistryKey key, String implementationLoaderUrl, String locationUrl)
    {
        locationUrl = expand_url( locationUrl ); 
        Console.WriteLine( "*** H E R E *** ManagedCodeLoader.writeRegistryInfo( " + implementationLoaderUrl  + ", " +  locationUrl + ") ****" );
        bool bReturn = false;
        // implementationName will be the class ( or Type ) name
        // locationUrl is the name of the assembly it will be in
        
        // here's a cheap and nasty facsimile of what the java loader does
        Type clazz = null;

        try
        {
            Console.WriteLine( "*** *** ManagedCodeLoader.writeRegistryInfo( " + implementationLoaderUrl  + ", " +  locationUrl + ") ABOUT to call find :-/ ****" );
            clazz = RegistrationClassFinder.find( locationUrl );
        }
        catch (System.NullReferenceException /*e*/ )
        {
            throw new unoidl.com.sun.star.registry.CannotRegisterImplementationException(  "Failed to find " + clazz.ToString(), null );
        }
        catch (System.Exception e )
        {
            throw new unoidl.com.sun.star.registry.CannotRegisterImplementationException(  e.ToString(), null );
        }
        MethodInfo compfac_method;
        try
        {
            Type[] regParams = { typeof(unoidl.com.sun.star.registry.XRegistryKey ) };
            compfac_method =  clazz.GetMethod( "__writeRegistryServiceInfo" , regParams );
            if ( compfac_method != null )
            {
                Object ret = compfac_method.Invoke( clazz, new Object [] { key } );
                if ( ret != null )
                    bReturn = ( bool )ret;
            }
    
        }
        catch ( System.Exception e )
        {
            throw new unoidl.com.sun.star.registry.CannotRegisterImplementationException(  e.ToString(), null );
        }
        return bReturn;

    }
    // XInitialization
    public void initialize( uno.Any[] args )
    {
        Console.WriteLine( "*** *** Entering ManagedCodeLoader.initialize() ");
        if ( args.Length == 0 )
            // probably need to change to uno.Exception
            throw new System.Exception("No arguments passed to initialize");
        multiServiceFactory = (unoidl.com.sun.star.lang.XMultiServiceFactory)args[0].Value;
        if ( multiServiceFactory == null )
            Console.WriteLine( " Bad multiservice factory " );
        Console.WriteLine( "*** *** Leaving ManagedCodeLoader.initialize() ");
        
    }
    // XServiceInfo
    public String getImplementationName() 
    {
        return GetType().ToString();
    }    
    public bool supportsService(String serviceName) 
    {
        for ( int i = 0; i < supportedServices.Length; i++ ) {
            if ( supportedServices[i] == serviceName )
                return true;
        }
        return false;
    }
    public String[] getSupportedServiceNames() 
    {
        return supportedServices;
    }    
}

}
