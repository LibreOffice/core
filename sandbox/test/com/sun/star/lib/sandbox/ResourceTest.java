package com.sun.star.lib.sandbox;

import java.net.URL;
import com.sun.star.lib.sandbox.ClassContextProxy;
import com.sun.star.lib.sandbox.ClassContext;
import com.sun.star.lib.sandbox.Resource;
import com.sun.star.lib.sandbox.ResourceProxy;

public class ResourceTest
{
    public static void main( String [] args ) throws java.lang.Exception
    {
        if( args.length != 2 )
        {
            System.out.println( "usage : ResourceTest file-url-to-jarfile class-name-in-jarfile" );
            System.exit(1);
        }

        URL u2 = new URL( args[0] );

        ClassContext context = ClassContextProxy.create(u2, null, null, true);

        Resource resource = ResourceProxy.load(u2, null);
        resource.loadJar(u2);
        context.addCargo( resource );
        resource = null;

        for( int i = 0 ; i < 1000000 ; i ++ )
        {
            new Object();
        }
        System.gc();
        System.gc();
        System.gc();

        context.loadClass( args[1] );
        System.out.println( "Test succeeded" );
    }
}
