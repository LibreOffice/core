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
package com.sun.star.comp.extensionoptions;

import com.sun.star.configuration.theDefaultProvider;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.Exception;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.awt.XContainerWindowEventHandler;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XControlContainer;
import com.sun.star.container.XNameAccess;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.util.XChangesBatch;

/** A handler which supports multiple options pages which all
 *  have the same controls.
 */
public class OptionsEventHandler {

    public static class _OptionsEventHandler extends WeakBase
        implements XServiceInfo, XContainerWindowEventHandler {

        static private final String __serviceName =
        "com.sun.star.comp.extensionoptions.OptionsEventHandler";

        private XComponentContext m_cmpCtx;

        private XMultiComponentFactory m_xMCF;

        private XNameAccess m_xAccessLeaves;

        /**Names of supported options pages.
         */
        private String[] m_arWindowNames = {
            "Writer1", "Writer2", "Writer3", "Calc1", "Calc2", "Calc3",
            "Draw1", "Draw2", "Draw3", "Node1_1", "Node1_2", "Node1_3",
            "Node2_1", "Node2_2", "Node2_3", "Node3_1", "Node3_2", "Node3_3"};

        /**Names of the controls which are supported by this handler. All these
         *controls must have a "Text" property.
         */
        private String[] m_arStringControls = {
            "String0", "String1", "String2", "String3", "String4"};

        public _OptionsEventHandler(XComponentContext xCompContext) {
                m_cmpCtx = xCompContext;
                m_xMCF = m_cmpCtx.getServiceManager();

            //Create the com.sun.star.configuration.ConfigurationUpdateAccess
            //for the registry node which contains the data for our option
            //pages.
            XMultiServiceFactory xConfig = theDefaultProvider.get(m_cmpCtx);

            //One argument for creating the ConfigurationUpdateAccess is the "nodepath".
            //Our nodepath point to the node of which the direct subnodes represent the
            //different options pages.
            Object[] args = new Object[1];
            args[0] = new NamedValue(
                "nodepath",
                "/org.openoffice.desktop.deployment.options.ExtensionData/Leaves");

            //We get the com.sun.star.container.XNameAccess from the instance of
            //ConfigurationUpdateAccess and save it for later use.
            try {
                m_xAccessLeaves = (XNameAccess) UnoRuntime.queryInterface(
                    XNameAccess.class, xConfig.createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationUpdateAccess", args));

            } catch (com.sun.star.uno.Exception e) {
                e.printStackTrace();
                return;
            }
        }

        /** This method returns an array of all supported service names.
         * @return Array of supported service names.
         */
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }

        /** This method is a simple helper function to used in the
         * static component initialisation functions as well as in
         * getSupportedServiceNames.
         */
        public static String[] getServiceNames() {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
        }

        /** This method returns true, if the given service will be
         * supported by the component.
         * @param sServiceName Service name.
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService( String sServiceName ) {
            return sServiceName.equals( __serviceName );
        }

        /** Return the class name of the component.
         * @return Class name of the component.
         */
        public String getImplementationName() {
            return  _OptionsEventHandler.class.getName();
        }

        //XContainerWindowEventHandler
        public boolean callHandlerMethod(com.sun.star.awt.XWindow aWindow,
                                        Object aEventObject, String sMethod)
            throws WrappedTargetException {
            if (sMethod.equals("external_event") ){
                try {
                    return handleExternalEvent(aWindow, aEventObject);
                } catch (com.sun.star.uno.RuntimeException re) {
                    throw re;
                } catch (com.sun.star.uno.Exception e) {
                    throw new WrappedTargetException(sMethod, this, e);
                }
            }

            return true;
        }

        //XContainerWindowEventHandler
        public String[] getSupportedMethodNames() {
            return new String[] {"external_event"};
        }

        private boolean handleExternalEvent(com.sun.star.awt.XWindow aWindow, Object aEventObject)
            throws com.sun.star.uno.Exception {
            try {
                String sMethod = AnyConverter.toString(aEventObject);
                if (sMethod.equals("ok")) {
                    saveData(aWindow);
                } else if (sMethod.equals("back") || sMethod.equals("initialize")) {
                    loadData(aWindow);
                }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                throw new com.sun.star.lang.IllegalArgumentException(
                    "Method external_event requires a string in the event object argument.",
                    this, (short) -1);
            }

            return true;
        }

        private void saveData(com.sun.star.awt.XWindow aWindow)
            throws com.sun.star.lang.IllegalArgumentException,
            com.sun.star.uno.Exception {

            //Determine the name of the options page. This serves two purposes. First, if this
            //options page is supported by this handler and second we use the name two locate
            //the corresponding data in the registry.
            String sWindowName = getWindowName(aWindow);
            if (sWindowName == null)
                throw new com.sun.star.lang.IllegalArgumentException(
                    "This window is not supported by this handler", this, (short) -1);

            //To access the separate controls of the window we need to obtain the
            //XControlContainer from the window implementation
            XControlContainer xContainer = (XControlContainer) UnoRuntime.queryInterface(
                XControlContainer.class, aWindow);
            if (xContainer == null)
                throw new com.sun.star.uno.Exception(
                    "Could not get XControlContainer from window.", this);

            //This is an implementation which will be used for several options pages
            //which all have the same controls. m_arStringControls is an array which
            //contains the names.
            for (int i = 0; i < m_arStringControls.length; i++) {

                //To obtain the data from the controls we need to get their model.
                //First get the respective control from the XControlContainer.
                XControl xControl = xContainer.getControl(m_arStringControls[i]);

                //This generic handler and the corresponding registry schema support
                //up to five text controls. However, if a options page does not use all
                //five controls then we will not complain here.
                if (xControl == null)
                    continue;

                //From the control we get the model, which in turn supports the
                //XPropertySet interface, which we finally use to get the data from
                //the control.
                XPropertySet xProp = (XPropertySet) UnoRuntime.queryInterface(
                    XPropertySet.class, xControl.getModel());

                if (xProp == null)
                    throw new com.sun.star.uno.Exception(
                        "Could not get XPropertySet from control.", this);
                //Get the "Text" property.
                Object aText = xProp.getPropertyValue("Text");
                String sValue = null;

                //The value is still contained in a com.sun.star.uno.Any - so convert it.
                try {
                    sValue = AnyConverter.toString(aText);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    throw new com.sun.star.lang.IllegalArgumentException(
                        "Wrong property type.", this, (short) -1);
                }

                //Now we have the actual string value of the control. What we need now is
                //the XPropertySet of the respective property in the registry, so that we
                //can store the value.
                //To access the registry we have previously created a service instance
                //of com.sun.star.configuration.ConfigurationUpdateAccess which supports
                //com.sun.star.container.XNameAccess. The XNameAccess is used to get the
                //particular registry node which represents this options page.
                //Fortunately the name of the window is the same as the registry node.
                XPropertySet xLeaf = (XPropertySet) UnoRuntime.queryInterface(
                    XPropertySet.class, m_xAccessLeaves.getByName(sWindowName));
                if (xLeaf == null)
                    throw new  com.sun.star.uno.Exception(
                        "XPropertySet not supported.", this);

                //Finally we can set the value
                xLeaf.setPropertyValue(m_arStringControls[i], sValue);
            }

            //Committing the changes will cause or changes to be written to the registry.
            XChangesBatch xUpdateCommit =
                (XChangesBatch) UnoRuntime.queryInterface(XChangesBatch.class, m_xAccessLeaves);
            xUpdateCommit.commitChanges();
        }

        private void loadData(com.sun.star.awt.XWindow aWindow)
            throws com.sun.star.uno.Exception {

            //Determine the name of the window. This serves two purposes. First, if this
            //window is supported by this handler and second we use the name two locate
            //the corresponding data in the registry.
            String sWindowName = getWindowName(aWindow);
            if (sWindowName == null)
                throw new com.sun.star.lang.IllegalArgumentException(
                    "The window is not supported by this handler", this, (short) -1);

            //To acces the separate controls of the window we need to obtain the
            //XControlContainer from window implementation
            XControlContainer xContainer = (XControlContainer) UnoRuntime.queryInterface(
                XControlContainer.class, aWindow);
            if (xContainer == null)
                throw new com.sun.star.uno.Exception(
                    "Could not get XControlContainer from window.", this);

            //This is an implementation which will be used for several options pages
            //which all have the same controls. m_arStringControls is an array which
            //contains the names.
            for (int i = 0; i < m_arStringControls.length; i++) {

                //load the values from the registry
                //To access the registry we have previously created a service instance
                //of com.sun.star.configuration.ConfigurationUpdateAccess which supports
                //com.sun.star.container.XNameAccess. We obtain now the section
                //of the registry which is assigned to this options page.
                XPropertySet xLeaf = (XPropertySet) UnoRuntime.queryInterface(
                    XPropertySet.class, m_xAccessLeaves.getByName(sWindowName));
                if (xLeaf == null)
                    throw new  com.sun.star.uno.Exception(
                        "XPropertySet not supported.", this);

                //The properties in the registry have the same name as the respective
                //controls. We use the names now to obtain the property values.
                Object aValue = xLeaf.getPropertyValue(m_arStringControls[i]);

                //Now that we have the value we need to set it at the corresponding
                //control in the window. The XControlContainer, which we obtained earlier
                //is the means to get hold of all the controls.
                XControl xControl = xContainer.getControl(m_arStringControls[i]);

                //This generic handler and the corresponding registry schema support
                //up to five text controls. However, if a options page does not use all
                //five controls then we will not complain here.
                if (xControl == null)
                    continue;

                //From the control we get the model, which in turn supports the
                //XPropertySet interface, which we finally use to set the data at the
                //control
                XPropertySet xProp = (XPropertySet) UnoRuntime.queryInterface(
                    XPropertySet.class, xControl.getModel());

                if (xProp == null)
                    throw new com.sun.star.uno.Exception(
                        "Could not get XPropertySet from control.", this);

                //This handler supports only text controls, which are named "Pattern Field"
                //in the dialog editor. We set the "Text" property.
                xProp.setPropertyValue("Text", aValue);
            }
        }

        //Checks if the name property of the window is one of the supported names and returns
        //always a valid string or null
        private String getWindowName(com.sun.star.awt.XWindow aWindow)
            throws com.sun.star.uno.Exception {

            if (aWindow == null)
                new com.sun.star.lang.IllegalArgumentException(
                    "Method external_event requires that a window is passed as argument",
                    this, (short) -1);

            //We need to get the control model of the window. Therefore the first step is
            //to query for it.
            XControl xControlDlg = (XControl) UnoRuntime.queryInterface(
                XControl.class, aWindow);

            if (xControlDlg == null)
                throw new com.sun.star.uno.Exception(
                    "Cannot obtain XControl from XWindow in method external_event.");
            //Now get model
            XControlModel xModelDlg = xControlDlg.getModel();

            if (xModelDlg == null)
                throw new com.sun.star.uno.Exception(
                    "Cannot obtain XControlModel from XWindow in method external_event.", this);
            //The model itself does not provide any information except that its
            //implementation supports XPropertySet which is used to access the data.
            XPropertySet xPropDlg = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xModelDlg);
            if (xPropDlg == null)
                throw new com.sun.star.uno.Exception(
                    "Cannot obtain XPropertySet from window in method external_event.", this);

            //Get the "Name" property of the window
            Object aWindowName = xPropDlg.getPropertyValue("Name");

            //Get the string from the returned com.sun.star.uno.Any
            String sName = null;
            try {
                sName = AnyConverter.toString(aWindowName);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                throw new com.sun.star.uno.Exception(
                    "Name - property of window is not a string.", this);
            }

            //Eventually we can check if we this handler can "handle" this options page.
            //The class has a member m_arWindowNames which contains all names of windows
            //for which it is intended
            for (int i = 0; i < m_arWindowNames.length; i++) {
                if (m_arWindowNames[i].equals(sName)) {
                    return sName;
                }
            }
            return null;
        }
    }


    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(String sImplName)
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( _OptionsEventHandler.class.getName() ) )
            xFactory = Factory.createComponentFactory(_OptionsEventHandler.class,
                                             _OptionsEventHandler.getServiceNames());

        return xFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey the registryKey
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return Factory.writeRegistryServiceInfo(_OptionsEventHandler.class.getName(),
                                                _OptionsEventHandler.getServiceNames(),
                                                regKey);
    }

    /** This method is a member of the interface for initializing an object
     * directly after its creation.
     * @param object This array of arbitrary objects will be passed to the
     * component after its creation.
     * @throws Exception Every exception will not be handled, but will be
     * passed to the caller.
     */
    public void initialize( Object[] object )
        throws com.sun.star.uno.Exception {
    }

}
