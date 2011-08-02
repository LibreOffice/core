using System;
using System.Reflection;

namespace uno.util
{
public class RegistrationClassFinder
{
    public static Type find( String url )
    {
        // LoadFrom can't handle escaped urls 
        // #FIXME how does one do this in CSharp
        Console.WriteLine( "## Find Url {0}", url );
        url =  System.Uri.UnescapeDataString( url );
        Console.WriteLine( "## Find Url escaped {0}", url );
        url = url.Replace( "%20"," " ) ;
        Console.WriteLine( "## after tweaking Url escaped {0}", url );
        Assembly assem = Assembly.LoadFrom( url );
        // we expect a component providing assembly to provide a Registration class
        // name. The name is in the static field 'name' of a class called 
        // 'component.RegistrationClass'. The 'name' is the actual name of the 
        // class that provides the following component methods
        // __getComponentFactory & __writeRegistryServiceInfo that are needed by the
        // loader
        // Of course we could use someother method, maybe bury the name in 
        // the component.dll.config ?
        FieldInfo f = assem.GetType("component.RegistrationClass").GetField( "name", BindingFlags.Public | BindingFlags.Static );
        String sTypeName = (String)f.GetValue( null );
        // Try to find the RegistrationClass
        return assem.GetType( sTypeName );
        
    }
}
}
