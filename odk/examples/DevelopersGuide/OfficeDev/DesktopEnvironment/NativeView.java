/*************************************************************************
 *
 *  $RCSfile: NativeView.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:30:56 $
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

package OfficeDev.samples.DesktopEnvironment;

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
