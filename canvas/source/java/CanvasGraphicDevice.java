/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CanvasGraphicDevice.java,v $
 * $Revision: 1.8 $
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
import com.sun.star.beans.XPropertySet;
import com.sun.star.lib.uno.helper.WeakBase;

// OOo AWT
import com.sun.star.awt.*;

// Canvas
import com.sun.star.rendering.*;
import com.sun.star.geometry.*;

// Java AWT
import java.awt.*;
import java.awt.geom.*;

// TEMP
import com.sun.star.uno.Type;


public class CanvasGraphicDevice
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.lang.XServiceInfo,
               com.sun.star.beans.XPropertySet,
               com.sun.star.rendering.XGraphicDevice
{
    private java.awt.Graphics2D             graphics;
    private java.awt.GraphicsConfiguration  graphicsConfig;

    //----------------------------------------------------------------------------------

    public CanvasGraphicDevice( java.awt.Graphics2D _graphics )
    {
        graphics = _graphics;
        graphicsConfig = graphics.getDeviceConfiguration();
    }

    //----------------------------------------------------------------------------------

    //
    // GraphicsDevice implementation
    // =============================
    //
    public synchronized XBufferController getBufferController()
    {
        // Use java.awt.image.BufferStrategy to implement XBufferController
        CanvasUtils.printLog( "CanvasGraphicDevice.getBufferController!" );
        return null;
    }

    public synchronized XColorSpace getDeviceColorSpace()
    {
        CanvasUtils.printLog( "CanvasGraphicDevice.getDeviceColorSpace!" );
        return null;
    }

    public synchronized com.sun.star.geometry.RealSize2D getPhysicalResolution()
    {
        CanvasUtils.printLog( "CanvasGraphicDevice.getPhysicalResolution!" );
        // TODO: getDefaultTransform + getNormalizingTransform
        return new com.sun.star.geometry.RealSize2D(100,100);
    }

    public synchronized com.sun.star.geometry.RealSize2D getPhysicalSize()
    {
        CanvasUtils.printLog( "CanvasGraphicDevice.getSize!" );
        java.awt.Rectangle bounds = graphicsConfig.getBounds();

        return new com.sun.star.geometry.RealSize2D(bounds.width, bounds.height);
    }

    public synchronized XLinePolyPolygon2D createCompatibleLinePolyPolygon( RealPoint2D[][] points )
    {
        CanvasUtils.printLog( "createCompatibleLinePolyPolygon" );
        return new LinePolyPolygon( points );
    }

    public synchronized XBezierPolyPolygon2D createCompatibleBezierPolyPolygon( RealBezierSegment2D[][] points )
    {
        CanvasUtils.printLog( "createCompatibleBezierPolyPolygon" );
        return new BezierPolyPolygon( points );
    }

    public synchronized com.sun.star.rendering.XBitmap createCompatibleBitmap( IntegerSize2D size )
    {
        CanvasUtils.printLog( "createCompatibleBitmap called with size (" + size.Width + ", " + size.Height + ")" );
        return new CanvasBitmap( graphicsConfig.createCompatibleImage( size.Width,
                                                                       size.Height,
                                                                       Transparency.OPAQUE ) );
    }

    public synchronized com.sun.star.rendering.XVolatileBitmap createVolatileBitmap( IntegerSize2D size )
    {
        CanvasUtils.printLog( "createVolatileBitmap called with size (" + size.Width + ", " + size.Height + ")" );
        //return new CanvasBitmap( graphicsConfig.createCompatibleVolatileImage( size.Width, size.Height ) );
        return null;
    }

    public synchronized com.sun.star.rendering.XBitmap createCompatibleAlphaBitmap( IntegerSize2D size )
    {
        CanvasUtils.printLog( "createCompatibleBitmap called with size (" + size.Width + ", " + size.Height + ")" );
        return new CanvasBitmap( graphicsConfig.createCompatibleImage( size.Width,
                                                                       size.Height,
                                                                        Transparency.TRANSLUCENT ) );
    }

    public synchronized com.sun.star.rendering.XVolatileBitmap createVolatileAlphaBitmap( IntegerSize2D size )
    {
        CanvasUtils.printLog( "createVolatileBitmap called with size (" + size.Width + ", " + size.Height + ")" );
        //return new CanvasBitmap( graphicsConfig.createCompatibleVolatileImage( size.Width, size.Height ) );
        return null;
    }

    public synchronized com.sun.star.rendering.XParametricPolyPolygon2DFactory getParametricPolyPolygonFactory()
    {
        // TODO
        return null;
    }

    public synchronized com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        // This is a stealth property set
        return null;
    }

    public synchronized void setPropertyValue( String aPropertyName, java.lang.Object aValue ) throws com.sun.star.beans.PropertyVetoException
    {
        // all our properties are read-only
        throw new com.sun.star.beans.PropertyVetoException();
    }

    public synchronized java.lang.Object getPropertyValue( String PropertyName ) throws com.sun.star.beans.UnknownPropertyException
    {
        if( PropertyName == "DeviceHandle" )
            return graphics;

        throw new com.sun.star.beans.UnknownPropertyException();
    }

    public synchronized void addPropertyChangeListener( String aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener ) throws com.sun.star.beans.UnknownPropertyException
    {
        if( aPropertyName == "DeviceHandle" )
            return;

        throw new com.sun.star.beans.UnknownPropertyException();
    }

    public synchronized void removePropertyChangeListener( String aPropertyName, com.sun.star.beans.XPropertyChangeListener aListener ) throws com.sun.star.beans.UnknownPropertyException
    {
        if( aPropertyName == "DeviceHandle" )
            return;

        throw new com.sun.star.beans.UnknownPropertyException();
    }

    public synchronized void addVetoableChangeListener( String PropertyName, com.sun.star.beans.XVetoableChangeListener aListener ) throws com.sun.star.beans.UnknownPropertyException
    {
        if( PropertyName == "DeviceHandle" )
            return;

        throw new com.sun.star.beans.UnknownPropertyException();
    }

    public synchronized void removeVetoableChangeListener( String PropertyName, com.sun.star.beans.XVetoableChangeListener aListener ) throws com.sun.star.beans.UnknownPropertyException
    {
        if( PropertyName == "DeviceHandle" )
            return;

        throw new com.sun.star.beans.UnknownPropertyException();
    }


    public synchronized boolean hasFullScreenMode()
    {
        return graphicsConfig.getDevice().isFullScreenSupported();
    }

    public synchronized boolean enterFullScreenMode( boolean bEnter )
    {
        return false;
    }

    //----------------------------------------------------------------------------------

    //
    // XServiceInfo impl
    // =================
    //

    private static final String s_implName = "XGraphicsDevice.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.GraphicsDevice";

    public String getImplementationName()
    {
        return s_implName;
    }

    public String [] getSupportedServiceNames()
    {
        return new String [] { s_serviceName };
    }

    public boolean supportsService( String serviceName )
    {
        return serviceName.equals( s_serviceName );
    }

}
