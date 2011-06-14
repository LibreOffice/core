using System;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.bridge;
using unoidl.com.sun.star.frame;

class DynLoad
{

    static void Main(string[] args)
    {
        connect(args);
    }
    
/** Connect to a running office that is accepting connections.
        @return  The ServiceManager to instantiate office components. */
    static private XMultiServiceFactory connect( string[] args )
    {
        if (args.Length == 0)
            Console.WriteLine("You need to provide a file URL to the office" +
                              " program folder\n");
        System.Collections.Hashtable ht = new System.Collections.Hashtable();
        ht.Add( "SYSBINDIR", args[ 0 ] );
        XComponentContext xContext =
            uno.util.Bootstrap.defaultBootstrap_InitialComponentContext(
                 args[ 0 ] + "/uno.ini", ht.GetEnumerator() );
        
        if (xContext != null)
            Console.WriteLine("Successfully created XComponentContext\n");
        else
            Console.WriteLine("Could not create XComponentContext\n");
       
       return null;
    }
}
