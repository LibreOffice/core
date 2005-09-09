/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentHandle.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:38:50 $
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
package complex.persistent_window_states;


import com.sun.star.awt.Rectangle;
import com.sun.star.awt.PosSize;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XComponent;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;
import com.sun.star.frame.XController;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XFramesSupplier;
import helper.WindowListener;

/**
 * Load and resize a document.
 *
 */
public class DocumentHandle {
    // the component loader to load a document
    XComponentLoader xCompLoader = null;
    // the document
    XComponent xComp = null;
    // the current window
    XWindow xWin = null;
    // a own window listener
    WindowListener wl = null;

    /**
     * Constructor
     * @param xComponentLoader A loader to load a document
     */
    public DocumentHandle(XComponentLoader xCompLoader) {
        this.xCompLoader = xCompLoader;
        wl = new WindowListener();
    }

    /**
     * Load/Create a document.
     * @param docName The name of a document as file URL
     * @param hidden If true, the document is loaded hidden.
     * @return The size of the opened/created document.
     */
    public Rectangle loadDocument(String docName, boolean hidden)
                                                            throws Exception{
        wl.resetTrigger();
        try {
            PropertyValue [] szArgs = null;
            if (hidden) {
                szArgs = new PropertyValue [1];
                PropertyValue Arg = new PropertyValue();
                Arg.Name = "Hidden";
                Arg.Value = hidden?"True":"False";
                Arg.Handle = -1;
                Arg.State = PropertyState.DEFAULT_VALUE;
                szArgs[0] = Arg;
            }
            else {
                szArgs = new PropertyValue [0];
            }

            // get the current active window
            XFrame xCurFrame = (XFrame)UnoRuntime.queryInterface(XFrame.class, xCompLoader);

            // create a new frame
            XFrame xFrame = xCurFrame.findFrame("_blank", FrameSearchFlag.CREATE);

            // load document in this frame
            XComponentLoader xFrameLoader = (XComponentLoader)UnoRuntime.queryInterface(XComponentLoader.class, xFrame);
            xComp = xFrameLoader.loadComponentFromURL(
                                                docName, "_self", 0, szArgs);
            // wait for the document to load.
            try {
                Thread.sleep(10000);
            }
            catch(java.lang.InterruptedException e) {}

            xWin = xFrame.getContainerWindow();
            xWin.addWindowListener(wl);
        }
        catch(com.sun.star.io.IOException e) {
            e.printStackTrace();
            return null;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            return null;
        }
        catch(java.lang.Exception e) {
            System.out.println("DH3");
            e.printStackTrace();
            throw e;
        }
        return xWin.getPosSize();

    }

    /**
     * Get the size of the current window.
     * @return The size of the window as Rectangle.
     */
    public Rectangle getDocumentPosSize() {
        return xWin.getPosSize();
    }

    /**
     * Resize the window in defined steps:
     * width -10 pixel;
     * height -10 pixel;
     * X-Position +10 pixel;
     * Y-Position +10 pixel
     * @return True if resize worked.
     */
    public boolean resizeDocument() {
        Rectangle newPosSize = xWin.getPosSize();
        newPosSize.Width = newPosSize.Width - 20;
        newPosSize.Height = newPosSize.Height - 20;
        newPosSize.X = newPosSize.X + 80;
        newPosSize.Y = newPosSize.Y + 80;
        return resizeDocument(newPosSize);
    }

    /**
     * Resize window to the given Rectangle
     * @param newPosSize The new position and size of the window.
     * @return True if resize worked.
     */
    public boolean resizeDocument(Rectangle newPosSize){
        wl.resetTrigger();
        xWin.setPosSize(newPosSize.X, newPosSize.Y, newPosSize.Width,
                                    newPosSize.Height, PosSize.POSSIZE);
        try {
            Thread.sleep(3000);
        }
        catch(java.lang.InterruptedException e) {}
        return wl.resizedTrigger;
    }
}
