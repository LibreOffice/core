/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.widgets;

import java.awt.Point;

import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.SmartId;

/**
 * Proxy to access a VCL window.
 *
 */
public class VclWindow extends VclControl {

    /**
     * Define a VCL window
     * @param uid the string id
     */
    public VclWindow(String uid) {
        super(uid);
    }

    public VclWindow(SmartId id) {
        super(id);
    }

    /**
     * Get the title of the window
     * @return
     */
    public String getText() {
        return getCaption();
    }

    /**
     * Presses the Help button to open the help topic for the window
     *
     */
    public void help()  {
        invoke(Constant.M_Help);
    }

    /**
     * Closes a window with the Close button.
     */
    public void close() {
        invoke(Constant.M_Close);
    }

    /**
     * Move the window by percent
     *
     * @param x
     * @param y
     */
    public void move(int x, int y)  {
        invoke(Constant.M_Move, new Object[]{new Integer(x), new Integer(y)});
    }

    /**
     * Move the window by pixel
     * @param p
     */
    public void move(Point p) {
        move(p.x, p.y);
    }



    /**
     * Returns the state of the window (maximize or minimize).
     * <p>
     *
     * @return Returns TRUE if the window is maximized, otherwise FALSE is
     *         returned.
     */
    public boolean isMax() {
        return (Boolean)invoke(Constant.M_IsMax);
    }

    /**
     * Maximizes a window so that the contents of the window are visible.
     */
    public void maximize() {
        invoke(Constant.M_Maximize);
    }

    /**
     * Minimizes a window so that only the title bar of the window is visible
     */
    public void minimize() {
        invoke(Constant.M_Minimize);
    }


    /**
     * Resize the window
     * @param x
     * @param y
     */
    public void resize(int x, int y) {
        invoke(Constant.M_Size, new Object[]{new Integer(x), new Integer(y)});
    }

    /**
     * Restore the window
     *
     */
    public void restore() {
        invoke(Constant.M_Restore);
    }

    /**
     * Activate the window
     */
    public void activate() {
        // focus it
        focus();
    }
}
