/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ResourceTest.java,v $
 * $Revision: 1.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
