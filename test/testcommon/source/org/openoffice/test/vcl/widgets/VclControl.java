/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.test.vcl.widgets;

import java.awt.Rectangle;

import org.openoffice.test.common.Condition;
import org.openoffice.test.vcl.Tester;
import org.openoffice.test.vcl.client.Constant;

/**
 * VCL control proxy
 */
public class VclControl extends VclWidget {

    protected String id = null;

    protected int type = -1;

    /**
     * Construct an instance with id in the default VclApp
     * @param id
     */
    public VclControl(String id) {
        super();
        this.id = id;
    }

    /**
     * Construct an instance with the given VclApp and ID
     * @param app
     * @param id
     */
    public VclControl(VclApp app, String id) {
        super(app);
        this.id = id;
    }

    /**
     * Get the ID of the control
     *
     * @return
     */
    public String getId() {
        return this.id;
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
        return app.caller.callControl(getId(), methodId, args);
    }

    /**
     *
     * @param methodId
     * @return
     */
    public Object invoke(int methodId) {
        return app.caller.callControl(getId(), methodId);
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
        if (!app.exists())
            return false;

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
        return id;
    }
}
