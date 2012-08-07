/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

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
 */

public class NativeView extends java.awt.Canvas
{
    // ____________________

    /**
     * ctor
     * Does nothing realy.
     * We can use our JNI mechanism for an already visible
     * canvas only. So we overload the method for showing ("setVisible()")
     * and make our intialization there. BUt we try to show an empty clean
     * window till there.
     */
    public NativeView()
    {
        maHandle = null;
        maSystem = 0;
        this.setBackground(Color.white);
    }

    // ____________________

    /**
     * Overload this method to make neccessary initializations here.
     * (e.g. get the window handle and neccessary system informations)
     *
     * Why here?
     * Because the handle seams to be available for already visible windows
     * only. So it's the best place to get it. Special helper method
     * can be called more then ones - but call native code one times only
     * and safe the handle and the system type on our members maHandle/maSystem!
     */
    public void setVisible(boolean bState)
    {
        getHWND();
    }

    // ____________________

    /**
     * to guarantee right resize handling inside a swing container
     * (e.g. JSplitPane) we must provide some informations about our
     * prefered/minimum and maximum size.
     */
    public Dimension getPreferredSize()
    {
        return new Dimension(500,300);
    }

    public Dimension getMaximumSize()
    {
        return new Dimension(1024,768);
    }

    public Dimension getMinimumSize()
    {
        return new Dimension(100,100);
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
    public void paint(Graphics aGraphic)
    {
        if(maHandle==null)
        {
            Dimension aSize = getSize();
            aGraphic.clearRect(0,0,aSize.width,aSize.height);
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
        if(maHandle==null)
        {
            maHandle = new Integer((int)getNativeWindow());
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
        System.loadLibrary("nativeview");
    }

    // ____________________

    /**
     * @member  maHandle    system window handle
     * @member  maSystem    info about currently used platform
     */
    public Integer maHandle ;
    public int     maSystem ;
}
