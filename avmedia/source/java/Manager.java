/*************************************************************************
 *
 *  $RCSfile: Manager.java,v $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
