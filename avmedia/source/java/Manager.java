/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
