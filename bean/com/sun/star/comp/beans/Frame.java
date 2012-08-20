/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        xDispatchProvider = UnoRuntime.queryInterface( com.sun.star.frame.XDispatchProvider.class,
            xFrame );
        xDispatchProviderInterception = UnoRuntime.queryInterface( com.sun.star.frame.XDispatchProviderInterception.class,
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

