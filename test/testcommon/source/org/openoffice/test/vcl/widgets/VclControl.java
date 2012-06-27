/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.widgets;

import java.awt.Rectangle;
import java.io.IOException;

import org.openoffice.test.common.Condition;
import org.openoffice.test.vcl.Tester;
import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.SmartId;
import org.openoffice.test.vcl.client.VclHook;

/**
 * VCL control proxy
 */
public class VclControl {

    public final static long ACTIVE = 0;

    public final static int WINDOW_BASE = 0x0100;

    public final static int WINDOW_FIRST = (WINDOW_BASE + 0x30);

    public final static int WINDOW_MESSBOX = (WINDOW_FIRST);

    public final static int WINDOW_INFOBOX = (WINDOW_FIRST + 0x01);

    public final static int WINDOW_WARNINGBOX = (WINDOW_FIRST + 0x02);

    public final static int WINDOW_ERRORBOX = (WINDOW_FIRST + 0x03);

    public final static int WINDOW_QUERYBOX = (WINDOW_FIRST + 0x04);

    public final static int WINDOW_WINDOW = (WINDOW_FIRST + 0x05);

    public final static int WINDOW_SYSWINDOW = (WINDOW_FIRST + 0x06);

    public final static int WINDOW_WORKWINDOW = (WINDOW_FIRST + 0x07);

    // public final static int WINDOW_MDIWINDOW = (WINDOW_FIRST + 0x08);
    public final static int WINDOW_FLOATINGWINDOW = (WINDOW_FIRST + 0x09);

    public final static int WINDOW_DIALOG = (WINDOW_FIRST + 0x0a);

    public final static int WINDOW_MODELESSDIALOG = (WINDOW_FIRST + 0x0b);

    public final static int WINDOW_MODALDIALOG = (WINDOW_FIRST + 0x0c);

    public final static int WINDOW_SYSTEMDIALOG = (WINDOW_FIRST + 0x0d);

    public final static int WINDOW_PATHDIALOG = (WINDOW_FIRST + 0x0e);

    public final static int WINDOW_FILEDIALOG = (WINDOW_FIRST + 0x0f);

    public final static int WINDOW_PRINTERSETUPDIALOG = (WINDOW_FIRST + 0x10);

    public final static int WINDOW_PRINTDIALOG = (WINDOW_FIRST + 0x11);

    public final static int WINDOW_COLORDIALOG = (WINDOW_FIRST + 0x12);

    public final static int WINDOW_FONTDIALOG = (WINDOW_FIRST + 0x13);

    public final static int WINDOW_CONTROL = (WINDOW_FIRST + 0x14);

    public final static int WINDOW_BUTTON = (WINDOW_FIRST + 0x15);

    public final static int WINDOW_PUSHBUTTON = (WINDOW_FIRST + 0x16);

    public final static int WINDOW_OKBUTTON = (WINDOW_FIRST + 0x17);

    public final static int WINDOW_CANCELBUTTON = (WINDOW_FIRST + 0x18);

    public final static int WINDOW_HELPBUTTON = (WINDOW_FIRST + 0x19);

    public final static int WINDOW_IMAGEBUTTON = (WINDOW_FIRST + 0x1a);

    public final static int WINDOW_MENUBUTTON = (WINDOW_FIRST + 0x1b);

    public final static int WINDOW_MOREBUTTON = (WINDOW_FIRST + 0x1c);

    public final static int WINDOW_SPINBUTTON = (WINDOW_FIRST + 0x1d);

    public final static int WINDOW_RADIOBUTTON = (WINDOW_FIRST + 0x1e);

    public final static int WINDOW_IMAGERADIOBUTTON = (WINDOW_FIRST + 0x1f);

    public final static int WINDOW_CHECKBOX = (WINDOW_FIRST + 0x20);

    public final static int WINDOW_TRISTATEBOX = (WINDOW_FIRST + 0x21);

    public final static int WINDOW_EDIT = (WINDOW_FIRST + 0x22);

    public final static int WINDOW_MULTILINEEDIT = (WINDOW_FIRST + 0x23);

    public final static int WINDOW_COMBOBOX = (WINDOW_FIRST + 0x24);

    public final static int WINDOW_LISTBOX = (WINDOW_FIRST + 0x25);

    public final static int WINDOW_MULTILISTBOX = (WINDOW_FIRST + 0x26);

    public final static int WINDOW_FIXEDTEXT = (WINDOW_FIRST + 0x27);

    public final static int WINDOW_FIXEDLINE = (WINDOW_FIRST + 0x28);

    public final static int WINDOW_FIXEDBITMAP = (WINDOW_FIRST + 0x29);

    public final static int WINDOW_FIXEDIMAGE = (WINDOW_FIRST + 0x2a);

    public final static int WINDOW_GROUPBOX = (WINDOW_FIRST + 0x2c);

    public final static int WINDOW_SCROLLBAR = (WINDOW_FIRST + 0x2d);

    public final static int WINDOW_SCROLLBARBOX = (WINDOW_FIRST + 0x2e);

    public final static int WINDOW_SPLITTER = (WINDOW_FIRST + 0x2f);

    public final static int WINDOW_SPLITWINDOW = (WINDOW_FIRST + 0x30);

    public final static int WINDOW_SPINFIELD = (WINDOW_FIRST + 0x31);

    public final static int WINDOW_PATTERNFIELD = (WINDOW_FIRST + 0x32);

    public final static int WINDOW_NUMERICFIELD = (WINDOW_FIRST + 0x33);

    public final static int WINDOW_METRICFIELD = (WINDOW_FIRST + 0x34);

    public final static int WINDOW_CURRENCYFIELD = (WINDOW_FIRST + 0x35);

    public final static int WINDOW_DATEFIELD = (WINDOW_FIRST + 0x36);

    public final static int WINDOW_TIMEFIELD = (WINDOW_FIRST + 0x37);

    public final static int WINDOW_PATTERNBOX = (WINDOW_FIRST + 0x38);

    public final static int WINDOW_NUMERICBOX = (WINDOW_FIRST + 0x39);

    public final static int WINDOW_METRICBOX = (WINDOW_FIRST + 0x3a);

    public final static int WINDOW_CURRENCYBOX = (WINDOW_FIRST + 0x3b);

    public final static int WINDOW_DATEBOX = (WINDOW_FIRST + 0x3c);

    public final static int WINDOW_TIMEBOX = (WINDOW_FIRST + 0x3d);

    public final static int WINDOW_LONGCURRENCYFIELD = (WINDOW_FIRST + 0x3e);

    public final static int WINDOW_LONGCURRENCYBOX = (WINDOW_FIRST + 0x3f);

    public final static int WINDOW_TOOLBOX = (WINDOW_FIRST + 0x41);

    public final static int WINDOW_DOCKINGWINDOW = (WINDOW_FIRST + 0x42);

    public final static int WINDOW_STATUSBAR = (WINDOW_FIRST + 0x43);

    public final static int WINDOW_TABPAGE = (WINDOW_FIRST + 0x44);

    public final static int WINDOW_TABCONTROL = (WINDOW_FIRST + 0x45);

    public final static int WINDOW_TABDIALOG = (WINDOW_FIRST + 0x46);

    public final static int WINDOW_BORDERWINDOW = (WINDOW_FIRST + 0x47);

    public final static int WINDOW_BUTTONDIALOG = (WINDOW_FIRST + 0x48);

    public final static int WINDOW_SYSTEMCHILDWINDOW = (WINDOW_FIRST + 0x49);

    public final static int WINDOW_FIXEDBORDER = (WINDOW_FIRST + 0x4a);

    public final static int WINDOW_SLIDER = (WINDOW_FIRST + 0x4b);

    public final static int WINDOW_MENUBARWINDOW = (WINDOW_FIRST + 0x4c);

    public final static int WINDOW_TREELISTBOX = (WINDOW_FIRST + 0x4d);

    public final static int WINDOW_HELPTEXTWINDOW = (WINDOW_FIRST + 0x4e);

    public final static int WINDOW_INTROWINDOW = (WINDOW_FIRST + 0x4f);

    public final static int WINDOW_LISTBOXWINDOW = (WINDOW_FIRST + 0x50);

    public final static int WINDOW_DOCKINGAREA = (WINDOW_FIRST + 0x51);

    public final static int WINDOW_VALUESETLISTBOX = (WINDOW_FIRST + 0x55);

    public final static int WINDOW_LAST = (WINDOW_DOCKINGAREA);

    protected SmartId uid = null;

    protected int type = -1;

    /**
     * Construct using smart ID
     * @param id
     */
    public VclControl(SmartId id) {
        this.uid = id;
    }

    /**
     * Construct using string ID or a
     * string indicating to dynamically find a control with the following
     * pattern: <br>
     * .find:ContainerControlID ControlType ControlIndex<br>
     * A space is needed to separate ContainerControlID ControlType and
     * ControlIndex.<br>
     * ".find:" is fixed prefix to tell automation server to dynamically find a
     * control.<br>
     * ContainerControlID is container control ID which is searched in<br>
     * ControlType gives the target control type <br>
     * ControlIndex is the index of target control<br>
     * e.g.<br>
     * new VclControl(".find:52821 326 2")
     *
     * @param uid
     */
    public VclControl(String uid) {
        this.uid = new SmartId(uid);
    }

    /**
     * Get the ID of the control
     *
     * @return
     */
    public SmartId getUID() {
        return this.uid;
    }

    public void click() {
        invoke(Constant.M_Click);
    }

    /**
     * Click
     *
     * @param locator
     * @param x
     * @param y
     */
    public void click(int x, int y) {
        Rectangle rect = getValidScreenRectangle();
        Tester.click((int) rect.x + x, (int) rect.y + y);
    }

    public void doubleClick(int x, int y) {
        Rectangle rect = getValidScreenRectangle();
        Tester.doubleClick((int) rect.x + x, (int) rect.y + y);
    }

    public void click(double xPercent, double yPercent) {
        Rectangle rect = getValidScreenRectangle();
        Tester.click((int)(rect.x + xPercent * rect.width), (int) (rect.y + yPercent * rect.height));
    }

    public void doubleClick(double xPercent, double yPercent) {
        Rectangle rect = getValidScreenRectangle();
        Tester.doubleClick((int)(rect.x + xPercent * rect.width), (int) (rect.y + yPercent * rect.height));
    }

    public void rightClick(int x, int y) {
        Rectangle rect = getValidScreenRectangle();
        Tester.rightClick((int) rect.x + x, (int) rect.y + y);
    }

    public void drag(int fromX, int fromY, int toX, int toY) {
        Rectangle rect = getValidScreenRectangle();
        Tester.drag((int) rect.x + fromX, (int) rect.y + fromY, (int) rect.x + toX, (int) rect.y + toY);
    }

    /**
     * Return the caption of control
     *
     * @return
     */
    public String getCaption() {
        return (String) invoke(Constant.M_Caption);
    }

    /**
     * Returns if the control is enabled
     *
     * @return Returns true if the control is enabled, otherwise false is
     *         returned.
     */
    public boolean isEnabled() {
        return (Boolean) invoke(Constant.M_IsEnabled);
    }

    /**
     * Returns if the control is checked
     *
     * @return Returns true if the control is enabled, otherwise false is
     *         returned.
     */
    public boolean isChecked() {
        return (Boolean) invoke(Constant.M_IsChecked);
    }

    /**
     * Return the count of fixed text in the control
     *
     * @return
     */
    protected int getFixedTextCount() {
        return (Integer) invoke(Constant.M_GetFixedTextCount);

    }

    /**
     * Return the fixed text in the control
     *
     * @param i
     *            the index of fixed text
     * @return the text of fixed text
     */
    protected String getFixedText(int i) {
        return (String) invoke(Constant.M_GetFixedText, new Object[] { new Integer(i + 1) });
    }

    /**
     * Operate the control via VclHook
     *
     * @param methodId
     * @param args
     * @return
     */
    public Object invoke(int methodId, Object... args) {
        return VclHook.invokeControl(getUID(), methodId, args);
    }

    /**
     *
     * @param methodId
     * @return
     */
    public Object invoke(int methodId) {
        return VclHook.invokeControl(getUID(), methodId, null);
    }

    /**
     * Internal use
     *
     */
    protected void useMenu() {
        invoke(Constant.M_UseMenu);
    }

    /**
     * TODO implement it. This is test tool implementation to input keyboard
     * keys.
     *
     */
    public void inputKeys(String keys) {
        invoke(Constant.M_TypeKeys, new Object[] { keys });
    }

    /**
     * Check if the control exists in a period of time
     */
    public boolean exists(double iTimeout) {
        return exists(iTimeout, 1);
    }

    /**
     * Check if the control exists in a period of time
     */
    public boolean exists(double iTimeout, double interval) {
        return new Condition() {
            @Override
            public boolean value() {
                return VclControl.this.exists();
            }
        }.test(iTimeout, interval);
    }

    /**
     * Wait for the control to exist in a period of time. If the time is out, an
     * ObjectNotFoundException will be throwed.
     *
     * @param iTimeout
     * @param interval
     */
    public void waitForExistence(double iTimeout, double interval) {
        new Condition() {
            @Override
            public boolean value() {
                return VclControl.this.exists();
            }
        }.waitForTrue( this + " is not found!", iTimeout, interval);
    }


    public void waitForEnabled(double iTimeout, double interval) {
        new Condition() {

            @Override
            public boolean value() {
                return VclControl.this.isEnabled();
            }

        }.waitForTrue("Time out to wait the control to be enabled!", iTimeout, interval);
    }

    public void waitForDisabled(double iTimeout, double interval) {
        new Condition() {

            @Override
            public boolean value() {
                return !VclControl.this.isEnabled();
            }

        }.waitForTrue("Time out to wait the control to be disabled!", iTimeout, interval);
    }

    public void waitForText(final String text, double iTimeout, double interval) {
        new Condition() {

            @Override
            public boolean value() {
                return text.equals(VclControl.this.getCaption());
            }

        }.waitForTrue("Time out to wait the control to show the text: " + text, iTimeout, interval);
    }


    public int getType() {
        if (type == -1)
            type = ((Long) invoke(Constant.M_GetRT)).intValue();
        return type;
    }

    /**
     * Returns if the control exists
     *
     * @return Returns true if the control is existed, otherwise false is
     *         returned.
     *
     */
    public boolean exists() {
        // If the communication is not established. return false
        try {
            VclHook.getCommunicationManager().connect();
        } catch (IOException e) {
            return false;
        }
        return (Boolean) invoke(Constant.M_Exists);
    }

    public void focus() {
        inputKeys("");
    }

    public Rectangle getScreenRectangle() {
        String ret = (String) invoke(Constant.M_GetScreenRectangle, new Object[] { Boolean.FALSE });
        if (ret == null)
            return null;
        String[] data = ret.split(",");
        int x = Integer.parseInt(data[0]);
        int y = Integer.parseInt(data[1]);
        int w = Integer.parseInt(data[2]);
        int h = Integer.parseInt(data[3]);
        return new Rectangle(x, y, w, h);
    }

    public Rectangle getValidScreenRectangle() {
        Rectangle rect = getScreenRectangle();
        if (rect == null)
            throw new RuntimeException(this + " - screen rectangle could not be computed! Maybe it is not showing!");
        return rect;
    }

    public VclMenuItem menuItem(String path) {
        return new VclMenuItem(new VclMenu(this), path);
    }
    /**
     * Opens the context menu of the control.
     * <p>
     * The context menu opens at the position of the mouse.
     * <p>
     * To open the context menu at a specific position in a window, you have to
     * first move the mouse to the position before using this method.
     * <p>
     *
     * @param
     */
    public void openContextMenu() {
        invoke(Constant.M_OpenContextMenu);
    }


    public String toString() {
        if (uid != null)
            return uid.toString();
        else
            return null;
    }
}
