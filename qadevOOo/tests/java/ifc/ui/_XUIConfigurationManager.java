/*************************************************************************
 *
 *  $RCSfile: _XUIConfigurationManager.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2004-07-23 10:46:35 $
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
 *  Source License Version 1.1 (the  {License {); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an  {AS IS { basis,
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

package ifc.ui;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XIndexContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import drafts.com.sun.star.ui.UIElementType;
import drafts.com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import drafts.com.sun.star.ui.XUIConfigurationManager;
import java.io.PrintWriter;
import lib.MultiMethodTest;

public class _XUIConfigurationManager extends MultiMethodTest {

    public XUIConfigurationManager oObj;
    private String msResourceUrl = "private:resource/menubar/menubar";
    private String msMyResourceUrl = "private:resource/menubar/mymenubar";
    private XIndexContainer mxSettings = null;
    private XIndexAccess mxMenuBarSettings = null;
    private XMultiServiceFactory mxMSF = null;


    /**
     * Some stuff before the tests:
     * extract the multi service factory.
     */
    protected void before() {
        mxMSF = (XMultiServiceFactory)tParam.getMSF();

    }

    /**
     * reset all changes: do at the end.
     */
    public void _reset() {
        requiredMethod("removeSettings()");
        oObj.reset();
        tRes.tested("reset()", true);
    }

    public void _getUIElementsInfo() {
        boolean result = true;
        try {
            PropertyValue[][]props = oObj.getUIElementsInfo(UIElementType.UNKNOWN);
            for (int i=0; i<props.length; i++)
                for(int j=0; j<props[i].length; j++)
                    log.println("Prop["+i+"]["+j+"]: " + props[i][j].Name + "   " + props[i][j].Value.toString());
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("getUIElementsInfo()", result);
    }

    public void _createSettings() {
        mxSettings = oObj.createSettings();
        util.dbg.printInterfaces(mxSettings);
        tRes.tested("createSettings()", mxSettings != null);
    }

    public void _hasSettings() {
        boolean result = false;
        try {
            result = oObj.hasSettings(msResourceUrl);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println(e);
            result = false;
        }
        tRes.tested("hasSettings()", result);
    }

    public void _getSettings() {
        requiredMethod("hasSettings()");
        boolean result = true;
        try {
            mxMenuBarSettings = oObj.getSettings(msResourceUrl, true);
            result = mxMenuBarSettings != null;
            for (int i=0; i<mxMenuBarSettings.getCount(); i++) {
                Object[] o = (Object[])mxMenuBarSettings.getByIndex(i);
                System.out.println("+++++++++ i = " + i);
                for (int j=0; j<o.length; j++) {
                    PropertyValue prop = (PropertyValue)o[j];
                    System.out.println("Property" + j + ": " + prop.Name + "   " + prop.Value.toString());
                }
            }
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("getSettings()", result);
    }

    public void _replaceSettings() {
        requiredMethod("getSettings()");
        boolean result = true;
        PropertyValue[] prop = createMenuBarEntry("My Entry", mxMenuBarSettings, mxMSF, log);
        if (prop == null) {
            tRes.tested("replaceSettings()", false);
            return;
        }

        createMenuBarItem("Click for Macro", (XIndexContainer)UnoRuntime.queryInterface(
                                        XIndexContainer.class, prop[3].Value), log);

        XIndexContainer x = (XIndexContainer)UnoRuntime.queryInterface(XIndexContainer.class, mxMenuBarSettings);
        try {
            x.insertByIndex(x.getCount(), prop);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            result = false;
            e.printStackTrace(log);
        }

        try {
            oObj.replaceSettings(msResourceUrl, mxMenuBarSettings);
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalAccessException e) {
            result = false;
            e.printStackTrace(log);
        }
        _getSettings();
        tRes.tested("replaceSettings()", result);
    }

    public void _removeSettings() {
        requiredMethod("insertSettings()");
        boolean result = true;
        try {
            oObj.removeSettings(msMyResourceUrl);
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalAccessException e) {
            e.printStackTrace(log);
            result = false;
        }
        tRes.tested("removeSettings()", result);
    }

    public void _insertSettings() {
        requiredMethod("createSettings()");
        requiredMethod("replaceSettings()");
        boolean result = true;
        util.dbg.printInterfaces(mxSettings);
        PropertyValue[] prop = createMenuBarEntry("A new entry", mxSettings, mxMSF, log);
        if (prop == null) {
            tRes.tested("replaceSettings()", false);
            return;
        }

        createMenuBarItem("A new sub entry", (XIndexContainer)UnoRuntime.queryInterface(
                                        XIndexContainer.class, prop[3].Value), log);

        XIndexContainer x = (XIndexContainer)UnoRuntime.queryInterface(XIndexContainer.class,mxSettings);
        try {
            int count = x.getCount();
            x.insertByIndex(count, prop);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            result = false;
            e.printStackTrace(log);
        }

        try {
            oObj.insertSettings(msMyResourceUrl, mxSettings);
        }
        catch(com.sun.star.container.ElementExistException e) {
            e.printStackTrace(log);
            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            result = false;
        }
        catch(com.sun.star.lang.IllegalAccessException e) {
            e.printStackTrace(log);
            result = false;
        }
        tRes.tested("insertSettings()", result);
    }

    public void _getImageManager() {
        Object o = oObj.getImageManager();
        tRes.tested("getImageManager()", o != null);
    }

    public void _getShortCutManager() {
        Object o = oObj.getShortCutManager();
        tRes.tested("getShortCutManager()", o == null);
    }

    public void _getEventsManager() {
        Object o = oObj.getEventsManager();
        tRes.tested("getEventsManager()", o == null);
    }

    /**
     * Create  a menu bar entry for adding to the menu bar of the Office.
     * @param sLabelName The name of the new entry.
     * @param xMenuBarSettings The existing menu bar settings, used for creating the new entry.
     * @return An array of properties of the new entry.
     */
    public static PropertyValue[] createMenuBarEntry(String sLabelName, XIndexAccess xMenuBarSettings, XMultiServiceFactory xMSF, PrintWriter log) {
        PropertyValue[] prop = new PropertyValue[4];
        prop[0] = new PropertyValue();
        prop[0].Name = "CommandURL";
        prop[0].Value = "vnd.openoffice.org:MyMenu";
        prop[1] = new PropertyValue();
        prop[1].Name = "Label";
        prop[1].Value = sLabelName;
        prop[2] = new PropertyValue();
        prop[2].Name = "Type";
        prop[2].Value = new Short((short)0);
        prop[3] = new PropertyValue();
        prop[3].Name = "ItemDescriptorContainer";

        XSingleComponentFactory xFactory = (XSingleComponentFactory)UnoRuntime.queryInterface(
                                XSingleComponentFactory.class, xMenuBarSettings);
        try {
            XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xMSF);
            XComponentContext xContext = (XComponentContext)UnoRuntime.queryInterface(
                    XComponentContext.class, xProp.getPropertyValue("DefaultContext"));
            prop[3].Value = xFactory.createInstanceWithContext(xContext);
        }
        catch(com.sun.star.uno.Exception e) {
            log.println("Could not create an instance for ItemDescriptorContainer property.");
            e.printStackTrace(log);
            return null;
        }
        return prop;
    }

    /**
     * Create a sub entry to the menu bar.
     * @param sLabelName The name of the entry in the UI.
     * @param xDescriptionContainer The parent entry in the menu bar where
     *      this entry is added.
     */
    public static void createMenuBarItem(String sLabelName, XIndexContainer xDescriptionContainer, PrintWriter log) {
        PropertyValue[]aMenuItem = new PropertyValue[3];
        // create a menu item
        aMenuItem[0] = new PropertyValue();
        aMenuItem[0].Name = "CommandURL";
        aMenuItem[0].Value = "macro:///Standard.Module1.Test()";
        aMenuItem[1] = new PropertyValue();
        aMenuItem[1].Name = "Label";
        aMenuItem[1].Value = sLabelName;
        aMenuItem[2] = new PropertyValue();
        aMenuItem[2].Name = "Type";
        aMenuItem[2].Value = new Short((short)0);

        try {
            xDescriptionContainer.insertByIndex(0, aMenuItem);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        }
    }
}
