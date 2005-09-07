/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SystemWindowAdapter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:41:51 $
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

import java.awt.*;
import java.lang.reflect.*;

public class SystemWindowAdapter
{
    static public java.awt.Frame createFrame( int windowHandle )
    {
        String          aOS = (String) System.getProperty( "os.name" );
        java.awt.Frame  aFrame = null;

        if( aOS.startsWith( "SunOS" ) )
        {
            try
            {
                Class aClass = Class.forName( "sun.awt.motif.MEmbeddedFrame" );

                if( aClass != null )
                {
                    try
                    {
                        Constructor aCtor = aClass.getConstructor( new Class[] { long.class, boolean.class } );

                        if( aCtor != null )
                        {
                            aFrame = (java.awt.Frame) aCtor.newInstance( new Object[] { new Long( windowHandle ),
                                                                                    new Boolean( false ) } );
                        }
                    }
                    catch( Exception e )
                    {
                    }

                    if( aFrame == null )
                    {
                        try
                        {
                            Constructor aCtor = aClass.getConstructor( new Class[] { long.class } );

                            if( aCtor != null )
                            {
                                 aFrame = (java.awt.Frame) aCtor.newInstance( new Object[] { new Long( windowHandle ) } );
                            }
                        }
                        catch( Exception e )
                        {
                        }
                    }
                }
            }
            catch( Exception e )
            {
            }
        }
        else
        {
            try
            {
                Class aClass = Class.forName( "sun.awt.motif.MEmbeddedFrame" );

                if( aClass != null )
                {
                    Constructor aCtor = aClass.getConstructor( new Class[] { long.class } );

                    if( aCtor != null )
                    {
                        aFrame = (java.awt.Frame) aCtor.newInstance( new Object[] { new Long( windowHandle ) } );
                    }
                }
            }
            catch( Exception e )
            {
            }

            if( aFrame == null )
            {
                try
                {
                    Class aClass = Class.forName( "sun.awt.X11.XEmbeddedFrame" );

                    if( aClass != null )
                    {
                        Constructor aCtor = aClass.getConstructor( new Class[] { long.class } );

                        if( aCtor != null )
                            aFrame = (java.awt.Frame) aCtor.newInstance( new Object[] { new Long( windowHandle ) } );
                    }
                }
                catch( Exception e )
                {
                }
            }
        }

        return aFrame;
    }
}
