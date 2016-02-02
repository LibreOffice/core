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
package embeddedobj.test;

// __________ Imports __________

import java.awt.*;
import java.lang.*;
import java.awt.event.*;

// __________ Implementation __________

/**
 * Class to pass the system window handle to the OpenOffice.org toolkit.
 * It use special JNI methods to get the system handle of used java window.
 *
 * Attention!
 * Use JNI functions on already visible canvas objects only!
 * Otherwise they can make some trouble.
 *
 */

public class NativeView extends java.awt.Canvas
{


    /**
     * ctor
     * Does nothing really.
     * We can use our JNI mechanism for an already visible
     * canvas only. So we override the method for showing ( "setVisible()" )
     * and make our initialization there. BUt we try to show an empty clean
     * window till there.
     */
    public NativeView()
    {
        maHandle = null;
        maSystem = 0;
        this.setBackground( Color.white );
    }



    /**
     * Override this method to make necessary initializations here.
     * ( e.g. get the window handle and necessary system information )
     *
     * Why here?
     * Because the handle seems to be available for already-visible windows
     * only. So it's the best place to get it. The special helper method
     * can be called more than once - but call native code one time only
     * and save the handle and the system type on our members maHandle/maSystem!
     */
    public void setVisible( boolean bState )
    {
        getHWND();
    }



    /**
     * to guarantee right resize handling inside a swing container
     * ( e.g. JSplitPane ) we must provide some information about our
     * preferred/minimum and maximum size.
     */
    public Dimension getPreferredSize()
    {
        return new Dimension( 800, 600 );
    }

    public Dimension getMaximumSize()
    {
        return new Dimension( 1024, 768 );
    }

    public Dimension getMinimumSize()
    {
        return new Dimension( 300, 300 );
    }



    /**
     * override paint routine to show provide against
     * repaint errors if no office view is really plugged
     * into this canvas.
     * If handle is present - we shouldn't paint anything further.
     * May the remote window is already plugged. In such case we
     * shouldn't paint it over.
     */
    public void paint( Graphics aGraphic )
    {
        if( maHandle == null )
        {
            Dimension aSize = getSize();
            aGraphic.clearRect( 0, 0, aSize.width, aSize.height );
        }
    }



    /**
     * JNI interface of this class
     * These two methods are implemented by using JNI mechanismen.
     * The will be used to get the platform dependent window handle
     * of a java awt canvas. This handle can be used to create an office
     * window as direct child of it. So it's possible to plug Office
     * windows in a java UI container.
     *
     * Note:
     * Native code for Windows registers a special function pointer to handle
     * window messages... But if it doesn't check for an already-registered
     * instance of this handler it will do it twice and produce a stack overflow
     * because such method calls itself in a never-ending loop...
     * So we try to use the JNI code one time only and save already-obtained
     * information inside this class.
     */
    public  native int  getNativeWindowSystemType();
    private native long getNativeWindow(); // private! => use getHWND() with cache mechanism!

    public Integer getHWND()
    {
        if( maHandle == null )
        {
            maHandle = Integer.valueOf( (int )getNativeWindow() );
            maSystem = getNativeWindowSystemType();
        }
        return maHandle;
    }



    /**
     * for using of the JNI methods it's necessary to load
     * system library which exports it.
     */
    static
    {
        System.loadLibrary( "nativeview" );
    }



    /**
     * @member  maHandle    system window handle
     * @member  maSystem    info about currently used platform
     */
    public Integer maHandle ;
    public int     maSystem ;
}

