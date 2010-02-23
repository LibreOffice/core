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

// awt
import com.sun.star.awt.*;

// media
import com.sun.star.media.*;

public class Player implements javax.media.ControllerListener,
                               com.sun.star.lang.XServiceInfo,
                               com.sun.star.media.XPlayer,
                               com.sun.star.lang.XComponent


{
    private com.sun.star.lang.XMultiServiceFactory maFactory;
    private String maURL;
    private javax.media.Player maPlayer;
    private javax.media.GainControl maGainControl;
    private boolean mbStarted = false;
    private boolean mbLooping = false;

    // -------------------------------------------------------------------------

    public Player( com.sun.star.lang.XMultiServiceFactory aFactory,
                   javax.media.Player aPlayer, String aURL )
    {
        maFactory = aFactory;
        maURL = aURL;
        maPlayer = aPlayer;
        maPlayer.addControllerListener( this );
        maGainControl = maPlayer.getGainControl();
    }

    // -------------------------------------------------------------------------

    public synchronized void controllerUpdate( javax.media.ControllerEvent aEvt )
    {
        if( aEvt instanceof javax.media.EndOfMediaEvent ||
            aEvt instanceof javax.media.StopAtTimeEvent )
        {
            mbStarted = false;

            if( mbLooping )
            {
                setMediaTime( 0.0 );
                start();
            }
            else if( aEvt instanceof javax.media.EndOfMediaEvent )
                setMediaTime( getDuration() );
        }
    }

    // -----------
    // - XPlayer -
    // -----------

    public synchronized void start()
    {
        if( !mbStarted )
        {
            maPlayer.start();
            mbStarted = true;
        }
    }

    // -------------------------------------------------------------------------

    public synchronized void stop()
    {
        if( mbStarted )
        {
            maPlayer.stop();
            mbStarted = false;
        }
    }

    // -------------------------------------------------------------------------

    public synchronized boolean isPlaying()
    {
        return mbStarted;
    }

    // -------------------------------------------------------------------------

    public synchronized double getDuration()
    {
        return maPlayer.getDuration().getSeconds();
    }

    // -------------------------------------------------------------------------

    public synchronized void setMediaTime( double fTime )
    {
        if( fTime >= 0.0 && fTime <= getDuration() )
            maPlayer.setMediaTime( new javax.media.Time( fTime ) );
    }

    // -------------------------------------------------------------------------

    public synchronized double getMediaTime()
    {
        return maPlayer.getMediaTime().getSeconds();
    }

    // -------------------------------------------------------------------------

    public synchronized void setStopTime( double fTime )
    {
        boolean bOldStarted = mbStarted;

        if( mbStarted )
            stop();

        maPlayer.setStopTime( new javax.media.Time( fTime ) );

        if( bOldStarted )
            start();
    }

    // -------------------------------------------------------------------------

    public synchronized double getStopTime()
    {
        return maPlayer.getStopTime().getSeconds();
    }

    // -------------------------------------------------------------------------

    public synchronized void setRate( double fRate )
    {
        boolean bOldStarted = mbStarted;

        if( mbStarted )
            stop();

        maPlayer.setRate( (float) fRate );

        if( bOldStarted )
            start();
    }

    // -------------------------------------------------------------------------

    public synchronized double getRate()
    {
        return (double) maPlayer.getRate();
    }

    // -------------------------------------------------------------------------

    public synchronized void setPlaybackLoop( boolean bSet )
    {
        mbLooping = bSet;
    }

    // -------------------------------------------------------------------------

    public synchronized boolean isPlaybackLoop()
    {
        return mbLooping;
    }

    // -------------------------------------------------------------------------

    public synchronized void setVolumeDB( short nVolumeDB )
    {
        if( maGainControl != null )
            maGainControl.setDB( nVolumeDB );
    }

    // -------------------------------------------------------------------------

    public synchronized short getVolumeDB()
    {
        return( maGainControl != null ? (short) maGainControl.getDB() : 0 );
    }

    // -------------------------------------------------------------------------

    public synchronized void setMute( boolean bSet )
    {
        if( maGainControl != null )
            maGainControl.setMute( bSet );
    }

    // -------------------------------------------------------------------------

    public synchronized boolean isMute()
    {
        return( maGainControl != null ? maGainControl.getMute() : false );
    }

    // -------------------------------------------------------------------------

    public synchronized com.sun.star.awt.Size getPreferredPlayerWindowSize()
    {
        java.awt.Component      aVisualComponent = maPlayer.getVisualComponent();
        com.sun.star.awt.Size   aSize = new com.sun.star.awt.Size( 0, 0 );

        if( aVisualComponent != null )
        {
            java.awt.Dimension aDim = aVisualComponent.getPreferredSize();

            aSize.Width = Math.max( aDim.width, 0 );
            aSize.Height = Math.max( aDim.height, 0 );
        }

        return aSize;
    }

    // -------------------------------------------------------------------------

    public synchronized com.sun.star.media.XPlayerWindow createPlayerWindow( java.lang.Object[] aArgs )
    {
        try
        {
            com.sun.star.media.XPlayerWindow xPlayerWindow = ( ( ( aArgs.length > 1 ) && ( AnyConverter.toInt( aArgs[ 0 ] ) > 0 ) ) ?
                                                             new PlayerWindow( maFactory, aArgs, maPlayer ) :
                                                             null );

            // check if it is a real player window (video window)
            if( xPlayerWindow != null && xPlayerWindow.getZoomLevel() == com.sun.star.media.ZoomLevel.NOT_AVAILABLE )
                xPlayerWindow = null;

            return xPlayerWindow;
        }
        catch( com.sun.star.lang.IllegalArgumentException e )
        {
            return null;
        }
    }

    // -------------------------------------------------------------------------

    public synchronized com.sun.star.media.XFrameGrabber createFrameGrabber()
    {
        return( (com.sun.star.media.XFrameGrabber) new FrameGrabber( maFactory, maURL ) );
    }

    // --------------
    // - XComponent -
    // --------------

    public synchronized void addEventListener( com.sun.star.lang.XEventListener xListener )
    {
    }

    // -------------------------------------------------------------------------

    public synchronized void removeEventListener( com.sun.star.lang.XEventListener xListener )
    {
    }

    // -------------------------------------------------------------------------

    public synchronized void dispose()
    {
        if( maPlayer != null )
        {
            maPlayer.stop();
            maPlayer.close();
            maPlayer = null;
        }
    }

    // ----------------
    // - XServiceInfo -
    // ----------------

    private static final String s_implName = "com.sun.star.comp.Player_Java";
    private static final String s_serviceName = "com.sun.star.media.Player_Java";

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
