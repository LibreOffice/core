/*************************************************************************
 *
 *  $RCSfile: CanvasGraphicDevice.java,v $
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
import com.sun.star.beans.XPropertySet;
import com.sun.star.lib.uno.helper.WeakBase;

// OOo AWT
import com.sun.star.awt.*;

// Canvas
import drafts.com.sun.star.rendering.*;
import drafts.com.sun.star.geometry.*;

// Java AWT
import java.awt.*;
import java.awt.geom.*;

// system-dependent stuff
import sun.awt.*;

// TEMP
import com.sun.star.uno.Type;


public class CanvasGraphicDevice
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.lang.XServiceInfo,
               com.sun.star.beans.XPropertySet,
               drafts.com.sun.star.rendering.XGraphicDevice
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

    public synchronized drafts.com.sun.star.geometry.RealSize2D getPhysicalResolution()
    {
        CanvasUtils.printLog( "CanvasGraphicDevice.getPhysicalResolution!" );
        // TODO: getDefaultTransform + getNormalizingTransform
        return new drafts.com.sun.star.geometry.RealSize2D(100,100);
    }

    public synchronized drafts.com.sun.star.geometry.RealSize2D getSize()
    {
        CanvasUtils.printLog( "CanvasGraphicDevice.getSize!" );
        java.awt.Rectangle bounds = graphicsConfig.getBounds();

        return new drafts.com.sun.star.geometry.RealSize2D(bounds.width, bounds.height);
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

    public synchronized drafts.com.sun.star.rendering.XBitmap createCompatibleBitmap( IntegerSize2D size )
    {
        CanvasUtils.printLog( "createCompatibleBitmap called with size (" + size.Width + ", " + size.Height + ")" );
        return new CanvasBitmap( graphicsConfig.createCompatibleImage( size.Width,
                                                                       size.Height,
                                                                       Transparency.OPAQUE ) );
    }

    public synchronized drafts.com.sun.star.rendering.XVolatileBitmap createVolatileBitmap( IntegerSize2D size )
    {
        CanvasUtils.printLog( "createVolatileBitmap called with size (" + size.Width + ", " + size.Height + ")" );
        //return new CanvasBitmap( graphicsConfig.createCompatibleVolatileImage( size.Width, size.Height ) );
        return null;
    }

    public synchronized drafts.com.sun.star.rendering.XBitmap createCompatibleAlphaBitmap( IntegerSize2D size )
    {
        CanvasUtils.printLog( "createCompatibleBitmap called with size (" + size.Width + ", " + size.Height + ")" );
        return new CanvasBitmap( graphicsConfig.createCompatibleImage( size.Width,
                                                                       size.Height,
                                                                        Transparency.TRANSLUCENT ) );
    }

    public synchronized drafts.com.sun.star.rendering.XVolatileBitmap createVolatileAlphaBitmap( IntegerSize2D size )
    {
        CanvasUtils.printLog( "createVolatileBitmap called with size (" + size.Width + ", " + size.Height + ")" );
        //return new CanvasBitmap( graphicsConfig.createCompatibleVolatileImage( size.Width, size.Height ) );
        return null;
    }

    public synchronized drafts.com.sun.star.rendering.XParametricPolyPolygon2DFactory getParametricPolyPolygonFactory()
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
    private static final String s_serviceName = "drafts.com.sun.star.rendering.GraphicsDevice";

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
