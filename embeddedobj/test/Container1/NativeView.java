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
 *************************************************************************/
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
 * @author  Andreas Schl&uuml;ns
 * @created 22.02.2002 08:47
 */

public class NativeView extends java.awt.Canvas
{
    // ____________________

    /**
     * ctor
     * Does nothing realy.
     * We can use our JNI mechanism for an already visible
     * canvas only. So we overload the method for showing ( "setVisible()" )
     * and make our intialization there. BUt we try to show an empty clean
     * window till there.
     */
    public NativeView()
    {
        maHandle = null;
        maSystem = 0;
        this.setBackground( Color.white );
    }

    // ____________________

    /**
     * Overload this method to make neccessary initializations here.
     * ( e.g. get the window handle and neccessary system informations )
     *
     * Why here?
     * Because the handle seams to be available for already visible windows
     * only. So it's the best place to get it. Special helper method
     * can be called more then ones - but call native code one times only
     * and safe the handle and the system type on our members maHandle/maSystem!
     */
    public void setVisible( boolean bState )
    {
        getHWND();
    }

    // ____________________

    /**
     * to guarantee right resize handling inside a swing container
     * ( e.g. JSplitPane ) we must provide some informations about our
     * prefered/minimum and maximum size.
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

    // ____________________

    /**
     * overload paint routine to show provide against
     * repaint errors if no office view is realy plugged
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

    // ____________________

    /**
     * JNI interface of this class
     * These two methods are implemented by using JNI mechanismen.
     * The will be used to get the platform dependent window handle
     * of a java awt canvas. This handle can be used to create an office
     * window as direct child of it. So it's possible to plug Office
     * windows in a java UI container.
     *
     * Note:
     * Native code for windows register special function pointer to handle
     * window messages ... But if it doesn't check for an already registered
     * instance of this handler it will do it twice and produce a stack overflow
     * because such method call herself in a never ending loop ...
     * So we try to use the JNI code one times only and safe already getted
     * informations inside this class.
     */
    public  native int  getNativeWindowSystemType();
    private native long getNativeWindow(); // private! => use getHWND() with cache mechanism!

    public Integer getHWND()
    {
        if( maHandle == null )
        {
            maHandle = new Integer( (int )getNativeWindow() );
            maSystem = getNativeWindowSystemType();
        }
        return maHandle;
    }

    // ____________________

    /**
     * for using of the JNI methods it's neccessary to load
     * system library which exports it.
     */
    static
    {
        System.loadLibrary( "nativeview" );
    }

    // ____________________

    /**
     * @member  maHandle    system window handle
     * @member  maSystem    info about currently used platform
     */
    public Integer maHandle ;
    public int     maSystem ;
}

