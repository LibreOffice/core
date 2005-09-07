/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FrameGrabber.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:39:37 $
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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XEventListener;
import com.sun.star.awt.*;
import com.sun.star.media.*;
import com.sun.star.graphic.*;

// -----------------
// - Player Window -
// -----------------

public class FrameGrabber implements com.sun.star.lang.XServiceInfo,
                                     com.sun.star.media.XFrameGrabber
{
    private com.sun.star.lang.XMultiServiceFactory maFactory = null;
    private javax.media.Player maPlayer = null;
    private javax.media.control.FrameGrabbingControl maFrameGrabbingControl = null;

    // -------------------------------------------------------------------------

    public FrameGrabber( com.sun.star.lang.XMultiServiceFactory aFactory, String aURL )
    {
        maFactory = aFactory;

        try
        {
            maPlayer = javax.media.Manager.createRealizedPlayer( new java.net.URL( aURL ) );
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

        if( maPlayer != null )
        {
            maFrameGrabbingControl = (javax.media.control.FrameGrabbingControl) maPlayer.getControl(
                                        "javax.media.control.FrameGrabbingControl" );
        }
    }

    // -------------------------------------------------------------------------

    public com.sun.star.graphic.XGraphic implImageToXGraphic( java.awt.Image aImage )
    {
        com.sun.star.graphic.XGraphic aRet = null;

        if( maFactory != null && aImage != null )
        {
            if( aImage instanceof java.awt.image.BufferedImage )
            {
                java.io.File aTempFile = null;

                try
                {
                    aTempFile = java.io.File.createTempFile( "sv0", ".png" );

                    if( aTempFile.canWrite() )
                    {
                        javax.imageio.ImageIO.write( (java.awt.image.BufferedImage) aImage, "png", aTempFile );

                        com.sun.star.graphic.XGraphicProvider aProvider =
                            (com.sun.star.graphic.XGraphicProvider) UnoRuntime.queryInterface(
                                com.sun.star.graphic.XGraphicProvider.class,
                                maFactory.createInstance("com.sun.star.graphic.GraphicProvider") );

                        if( aProvider != null )
                        {
                            com.sun.star.beans.PropertyValue[] aArgs = new com.sun.star.beans.PropertyValue[ 1 ];

                            aArgs[ 0 ] = new com.sun.star.beans.PropertyValue();
                            aArgs[ 0 ].Name = "URL";
                            aArgs[ 0 ].Value = "file://" + aTempFile.toString();

                            aRet = aProvider.queryGraphic( aArgs );
                        }
                    }
                }
                catch( java.lang.IllegalArgumentException aExcp )
                {
                }
                catch( java.io.IOException aExcp )
                {
                }
                catch( com.sun.star.uno.Exception aExcp )
                {
                }

                if( aTempFile != null )
                    aTempFile.delete();
            }
        }

        return aRet;
    }

    // -----------------
    // - XFrameGrabber -
    // -----------------

    public synchronized com.sun.star.graphic.XGraphic grabFrame( double fMediaTime )
    {
        com.sun.star.graphic.XGraphic aRet = null;

        if( maFrameGrabbingControl != null )
        {
            if( fMediaTime >= 0.0 && fMediaTime <= maPlayer.getDuration().getSeconds() )
            {
                maPlayer.setMediaTime( new javax.media.Time( fMediaTime ) );

                javax.media.Buffer aBuffer = maFrameGrabbingControl.grabFrame();

                if( aBuffer != null && aBuffer.getFormat() instanceof javax.media.format.VideoFormat )
                {
                    aRet = implImageToXGraphic( new javax.media.util.BufferToImage(
                                                    (javax.media.format.VideoFormat) aBuffer.getFormat() ).
                                                        createImage( aBuffer ) );
                }
            }
        }

        return aRet;
    }

    // ----------------
    // - XServiceInfo -
    // ----------------

    private static final String s_implName = "com.sun.star.comp.FrameGrabber_Java";
    private static final String s_serviceName = "com.sun.star.media.FrameGrabber_Java";

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
}
