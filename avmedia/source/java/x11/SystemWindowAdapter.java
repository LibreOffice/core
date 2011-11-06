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
