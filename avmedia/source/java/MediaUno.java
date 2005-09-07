/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MediaUno.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:40:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// UNO
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.lang.XInitialization;

public class MediaUno
{
    private static final String s_implName = "com.sun.star.comp.media.Manager_Java";
    private static final String s_serviceName = "com.sun.star.media.Manager_Java";

    // -------------------------------------------------------------------------

    public MediaUno()
    {
    }

    // -------------------------------------------------------------------------

    public static com.sun.star.lang.XSingleServiceFactory __getServiceFactory(
        String implName,
        com.sun.star.lang.XMultiServiceFactory multiFactory,
        com.sun.star.registry.XRegistryKey regKey )
    {
        if (implName.equals( s_implName ))
        {
            try
            {
                return com.sun.star.comp.loader.FactoryHelper.getServiceFactory(
                    Class.forName( "Manager" ), s_serviceName, multiFactory, regKey );
            }
            catch( java.lang.ClassNotFoundException exception )
            {
            }
        }

        return null;
    }

    // -------------------------------------------------------------------------

    public static boolean __writeRegistryServiceInfo(
        com.sun.star.registry.XRegistryKey regKey )
    {
        return com.sun.star.comp.loader.FactoryHelper.writeRegistryServiceInfo(
            s_implName, s_serviceName, regKey );
    }
}
