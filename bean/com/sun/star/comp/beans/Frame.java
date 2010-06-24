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

package com.sun.star.comp.beans;

import com.sun.star.uno.UnoRuntime;

/** Wrapper class for a com.sun.star.frame.XFrame.
 *
 * @since OOo 2.0.0
 */
public class Frame
    extends Wrapper
    implements
        com.sun.star.frame.XFrame,
        com.sun.star.frame.XDispatchProvider,
        com.sun.star.frame.XDispatchProviderInterception
{
    private com.sun.star.frame.XFrame xFrame;
    private com.sun.star.frame.XDispatchProvider xDispatchProvider;
    private com.sun.star.frame.XDispatchProviderInterception xDispatchProviderInterception;

    public Frame( com.sun.star.frame.XFrame xFrame )
    {
        super( xFrame );
        this.xFrame = xFrame;
        xDispatchProvider = (com.sun.star.frame.XDispatchProvider)
            UnoRuntime.queryInterface( com.sun.star.frame.XDispatchProvider.class,
                xFrame );
        xDispatchProviderInterception = (com.sun.star.frame.XDispatchProviderInterception)
            UnoRuntime.queryInterface( com.sun.star.frame.XDispatchProviderInterception.class,
                xFrame );
    }

    //==============================================================
    // com.sun.star.frame.XFrame
    //--------------------------------------------------------------

    public void initialize( /*IN*/com.sun.star.awt.XWindow xWindow )
    {
        xFrame.initialize( xWindow );
    }

    public com.sun.star.awt.XWindow getContainerWindow(  )
    {
        return xFrame.getContainerWindow();
    }

    public void setCreator( /*IN*/ com.sun.star.frame.XFramesSupplier xCreator )
    {
        xFrame.setCreator( xCreator );
    }

    public com.sun.star.frame.XFramesSupplier getCreator(  )
    {
        return xFrame.getCreator();
    }

    public String getName(  )
    {
        return xFrame.getName();
    }

    public void setName( /*IN*/ String aName )
    {
        xFrame.setName( aName );
    }

    public com.sun.star.frame.XFrame findFrame( /*IN*/String aTargetFrameName, /*IN*/int nSearchFlags )
    {
        return xFrame.findFrame( aTargetFrameName, nSearchFlags );
    }

    public boolean isTop(  )
    {
        return xFrame.isTop();
    }

    public void activate(  )
    {
        xFrame.activate();
    }

    public void deactivate(  )
    {
        xFrame.deactivate();
    }

    public boolean isActive(  )
    {
        return xFrame.isActive();
    }

    public boolean setComponent( /*IN*/com.sun.star.awt.XWindow xComponentWindow, /*IN*/ com.sun.star.frame.XController xController )
    {
        return xFrame.setComponent( xComponentWindow, xController );
    }

    public com.sun.star.awt.XWindow getComponentWindow(  )
    {
        return xFrame.getComponentWindow();
    }

    public com.sun.star.frame.XController getController(  )
    {
        return xFrame.getController();
    }

    public void contextChanged(  )
    {
        xFrame.contextChanged();
    }

    public void addFrameActionListener( /*IN*/ com.sun.star.frame.XFrameActionListener xListener )
    {
        xFrame.addFrameActionListener( xListener );
    }

    public void removeFrameActionListener( /*IN*/ com.sun.star.frame.XFrameActionListener xListener )
    {
        xFrame.removeFrameActionListener( xListener );
    }

    //==============================================================
    // com.sun.star.frame.XDispatchProvider
    //--------------------------------------------------------------

    public com.sun.star.frame.XDispatch queryDispatch(
            /*IN*/ com.sun.star.util.URL aURL,
            /*IN*/ String aTargetFrameName,
            /*IN*/ int nSearchFlags )
    {
        return xDispatchProvider.queryDispatch( aURL, aTargetFrameName, nSearchFlags );
    }

    public com.sun.star.frame.XDispatch[] queryDispatches(
            /*IN*/ com.sun.star.frame.DispatchDescriptor[] aRequests )
    {
        return xDispatchProvider.queryDispatches( aRequests );
    }

    //==============================================================
    // com.sun.star.frame.XDispatchProviderInterception
    //--------------------------------------------------------------

    public void registerDispatchProviderInterceptor(
            /*IN*/ com.sun.star.frame.XDispatchProviderInterceptor xInterceptor )
    {
        xDispatchProviderInterception.registerDispatchProviderInterceptor( xInterceptor );
    }

    public void releaseDispatchProviderInterceptor(
            /*IN*/ com.sun.star.frame.XDispatchProviderInterceptor xInterceptor )
    {
        xDispatchProviderInterception.releaseDispatchProviderInterceptor( xInterceptor );
    }
};

