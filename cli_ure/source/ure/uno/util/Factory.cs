using System;
using System.Reflection;

using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.registry;
namespace uno.util {

public class Factory : WeakComponentBase , unoidl.com.sun.star.lang.XSingleComponentFactory, unoidl.com.sun.star.lang.XServiceInfo
{
    public static XSingleComponentFactory createComponentFactory(
        Type impl_class, String[] supported_services )
    {
        return new Factory( impl_class, supported_services );
    }

    public static bool writeRegistryServiceInfo(
        String impl_name, String[] supported_services, XRegistryKey xKey )
    {
        Console.WriteLine( "#####  HERE ##### ");
        try
        {
            Console.WriteLine( "##### " + typeof( Factory ).ToString() + ".writeRegistryServiceInfo creating new key for SERVICES" );

            unoidl.com.sun.star.registry.XRegistryKey xNewKey = xKey.createKey( "/" + impl_name + "/UNO/SERVICES" );
            for ( int nPos = 0; nPos < supported_services.Length; ++nPos )
            {
                xNewKey.createKey( supported_services[ nPos ] );
                Console.WriteLine( "##### " + typeof( Factory ).ToString() + ".writeRegistryServiceInfo created new key fo " + supported_services[ nPos ] );
            }
            return true;
        }
        catch ( unoidl.com.sun.star.registry.InvalidRegistryException exc)
        {
            Console.WriteLine( "##### " + typeof( Factory ).ToString() + ".writeRegistryServiceInfo - exc: " + exc );
        }
        return false;
    }

    private String m_impl_name;
    private String [] m_supported_services;
    private Type m_impl_class;
    private MethodInfo m_method;
    private ConstructorInfo m_ctor;

    // ctor
    private Factory( Type impl_class, String[] supported_services )
    {
        m_impl_name = impl_class.ToString();
        m_supported_services = supported_services;
        m_impl_class = impl_class;
        m_method = null;
        m_ctor = null;

        Type[] mparams = { typeof ( unoidl.com.sun.star.uno.XComponentContext ) };

        try
        {
            // seeking for "public static Object __create( XComponentContext )"
            m_method = m_impl_class.GetMethod("__create", BindingFlags.Public | BindingFlags.Static, null, CallingConventions.Any, mparams, null );
            if ( m_method.ReturnType != typeof ( Object ) )
                m_method = null;
        }
        catch (System.Exception /*exc*/)
        {
        }

        if (null == m_method)
        {
            try
            {
                Console.WriteLine( "searching for ctor with unoidl.com.sun.star.uno.XComponentContext ");
                // ctor with context
                m_ctor = m_impl_class.GetConstructor( new Type[] { typeof ( unoidl.com.sun.star.uno.XComponentContext ) }  );
                Console.WriteLine( "found ctor ? " + ( m_ctor != null ).ToString() );

            }
            catch (System.Exception /*exc*/)
            {

                // else take default ctor
                m_ctor = m_impl_class.GetConstructor(null);
            }
        }
    }

    //______________________________________________________________________________________________
    private Object instantiate( XComponentContext xContext )
//        throws com.sun.star.uno.Exception
    {
        try
        {
            Console.WriteLine( "instantiating " + m_impl_class.ToString() + " using " );
            if (null != m_method)
            {
                Console.WriteLine( "\t__create( XComponentContext )..." );
                return m_method.Invoke( null, new Object [] { xContext } );
            }
            if (null != m_ctor)
            {
                Console.WriteLine( "\tctor( XComponentContext )..." );
                return m_ctor.Invoke( new Object[] { xContext } );
            }
            Console.WriteLine( "\tdefault ctor ..." );
            // #FIXME check this
            return m_impl_class.GetConstructor(null).Invoke(null); // default ctor
        }
        catch ( System.Exception e )
        {
            Console.WriteLine( "\tcontructing component " + m_impl_class.ToString() + " failed exc: " + e );
            throw new unoidl.com.sun.star.uno.RuntimeException( e.ToString(), null );
        }
        // #FIXME sort out the exception foo below
/*
        catch (java.lang.reflect.InvocationTargetException exc)
        {
            Throwable targetException = exc.getTargetException();
            if (targetException instanceof java.lang.RuntimeException)
                throw (java.lang.RuntimeException)targetException;
            else if (targetException instanceof com.sun.star.uno.RuntimeException)
                throw (com.sun.star.uno.RuntimeException)targetException;
            else if (targetException instanceof com.sun.star.uno.Exception)
                throw (com.sun.star.uno.Exception)targetException;
            else
                throw new com.sun.star.uno.Exception( targetException.toString(), this );
        }
        catch (IllegalAccessException exc)
        {
            throw new com.sun.star.uno.RuntimeException( exc.toString(), this );
        }
        catch (InstantiationException exc)
        {
            throw new com.sun.star.uno.RuntimeException( exc.toString(), this );
        }
*/
    }
    // XSingleComponentFactory impl
    //______________________________________________________________________________________________
    public Object createInstanceWithContext(
        unoidl.com.sun.star.uno.XComponentContext xContext )
//        throws com.sun.star.uno.Exception
    {
        return instantiate( xContext );
    }
    //______________________________________________________________________________________________
    public Object createInstanceWithArgumentsAndContext(
        uno.Any[] arguments, unoidl.com.sun.star.uno.XComponentContext xContext )
//        throws com.sun.star.uno.Exception
    {
        Object inst = instantiate( xContext );
        unoidl.com.sun.star.lang.XInitialization xInit = ( unoidl.com.sun.star.lang.XInitialization ) inst;
        xInit.initialize( arguments );
        return inst;
    }

    // XServiceInfo impl
    //______________________________________________________________________________________________
    public String getImplementationName()
    {
        return m_impl_name;
    }
    //______________________________________________________________________________________________
    public bool supportsService( String service_name )
    {
        for ( int nPos = 0; nPos < m_supported_services.Length; ++nPos )
        {
            if (m_supported_services[ nPos ] == service_name )
                return true;
        }
        return false;
    }
    //______________________________________________________________________________________________
    public String [] getSupportedServiceNames()
    {
        return m_supported_services;
    }
}

}

