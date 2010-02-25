/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// UNO
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.lang.XInitialization;

// media
import com.sun.star.media.*;

public class Manager implements com.sun.star.lang.XServiceInfo,
                                com.sun.star.lang.XTypeProvider,
                                com.sun.star.media.XManager

{
    private com.sun.star.lang.XMultiServiceFactory maFactory;

    // -------------------------------------------------------------------------

    public Manager( com.sun.star.lang.XMultiServiceFactory aFactory )
    {
        maFactory = aFactory;
    }

    // ------------
    // - XManager -
    // ------------

    public com.sun.star.media.XPlayer createPlayer( String aURL )
    {
        javax.media.Player aPlayer = null;

        try
        {
            aPlayer = javax.media.Manager.createRealizedPlayer( new java.net.URL( aURL ) );
        }
        catch( java.net.MalformedURLException e )
        {
        }
        catch( java.io.IOException e )
        {
        }
        catch( javax.media.NoPlayerException e )
        {
        }
        catch( javax.media.CannotRealizeException e )
        {
        }
        catch( java.lang.Exception e )
        {
        }

        if( aPlayer != null )
        {
            return new Player( maFactory, aPlayer, aURL );
        }
        else
            return null;
    }

    // ----------------
    // - XServiceInfo -
    // ----------------

    private static final String s_implName = "com.sun.star.comp.media.Manager_Java";
    private static final String s_serviceName = "com.sun.star.media.Manager_Java";

    public synchronized String getImplementationName()
    {
        return s_implName;
    }

    // -------------------------------------------------------------------------

    public synchronized String [] getSupportedServiceNames()
    {
        return new String [] { s_serviceName };
    }

    // -------------------------------------------------------------------------

    public synchronized boolean supportsService( String serviceName )
    {
        return serviceName.equals( s_serviceName );
    }

    // -----------------
    // - XTypeProvider -
    // -----------------
    protected byte[] maImplementationId;

    public com.sun.star.uno.Type[] getTypes()
    {
        com.sun.star.uno.Type[] retValue = new com.sun.star.uno.Type[ 3 ];

        retValue[ 0 ]= new com.sun.star.uno.Type( com.sun.star.lang.XServiceInfo.class );
        retValue[ 1 ]= new com.sun.star.uno.Type( com.sun.star.lang.XTypeProvider.class );
        retValue[ 2 ]= new com.sun.star.uno.Type( com.sun.star.media.XManager.class );

        return retValue;
    }

    // -------------------------------------------------------------------------

    synchronized public byte[] getImplementationId()
    {
        if( maImplementationId == null)
        {
            maImplementationId = new byte[ 16 ];

            int hash = hashCode();

            maImplementationId[ 0 ] = (byte)(hash & 0xff);
            maImplementationId[ 1 ] = (byte)((hash >>> 8) & 0xff);
            maImplementationId[ 2 ] = (byte)((hash >>> 16) & 0xff);
            maImplementationId[ 3 ] = (byte)((hash >>>24) & 0xff);
        }

        return maImplementationId;
    }
}
