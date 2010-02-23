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
