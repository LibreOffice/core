/*************************************************************************
*
*  $RCSfile: Desktop.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:36:03 $
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
*/

package com.sun.star.wizards.common;

import com.sun.star.awt.XToolkit;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFramesSupplier;

import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XNamingService;
import com.sun.star.util.MalformedNumberFormatException;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XNumberFormatter;
import com.sun.star.util.XURLTransformer;
import com.sun.star.lang.Locale;
import com.sun.star.uno.XInterface;
import com.sun.star.beans.XPropertySet;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XNameAccess;
import com.sun.star.util.XStringSubstitution;
import com.sun.star.frame.*;

public class Desktop {

    /** Creates a new instance of Desktop */
    public Desktop() {
    }

    public static XDesktop getDesktop(XMultiServiceFactory xMSF) {
        com.sun.star.uno.XInterface xInterface = null;
        XDesktop xDesktop = null;
        if (xMSF != null) {
            try {
                xInterface = (com.sun.star.uno.XInterface) xMSF.createInstance("com.sun.star.frame.Desktop");
                xDesktop = (XDesktop) UnoRuntime.queryInterface(XDesktop.class, xInterface);
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
        } else
            System.out.println("Can't create a desktop. null pointer !");
        return xDesktop;
    }

    public static XFrame getActiveFrame(XMultiServiceFactory xMSF) {
        XDesktop xDesktop = getDesktop(xMSF);
        XFramesSupplier xFrameSuppl = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);
        XFrame xFrame = xFrameSuppl.getActiveFrame();
        return xFrame;
    }



    public static XComponent getActiveComponent(XMultiServiceFactory _xMSF){
        XFrame xFrame = getActiveFrame(_xMSF);
        return (XComponent) UnoRuntime.queryInterface(XComponent.class, xFrame.getController().getModel());
    }


    public static XTextDocument getActiveTextDocument(XMultiServiceFactory _xMSF){
        XComponent xComponent = getActiveComponent(_xMSF);
        return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);
    }


    public static XSpreadsheetDocument getActiveSpreadsheetDocument(XMultiServiceFactory _xMSF){
        XComponent xComponent = getActiveComponent(_xMSF);
        return (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, xComponent);
    }


    public static void dispatchURL(XMultiServiceFactory xMSF, String sURL, com.sun.star.frame.XFrame xFrame) {
        try {
            PropertyValue[] oArg = new PropertyValue[0];
            com.sun.star.util.URL[] oUrl = new com.sun.star.util.URL[1];
            oUrl[0] = new com.sun.star.util.URL();
            oUrl[0].Complete = sURL;
            Object oTransformer = xMSF.createInstance("com.sun.star.util.URLTransformer");
            XURLTransformer xTransformer = (XURLTransformer) UnoRuntime.queryInterface(XURLTransformer.class, oTransformer);
            xTransformer.parseStrict(oUrl);
            XDispatchProvider xDispatchProvider = (XDispatchProvider) UnoRuntime.queryInterface(XDispatchProvider.class, xFrame);
            XDispatch xDispatch = xDispatchProvider.queryDispatch(oUrl[0], "", 0); // "_self"
            xDispatch.dispatch(oUrl[0], oArg);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public static XMultiComponentFactory getMultiComponentFactory() throws com.sun.star.uno.Exception, RuntimeException, java.lang.Exception{
        XComponentContext xcomponentcontext = Bootstrap.createInitialComponentContext(null);
        // initial serviceManager
        return xcomponentcontext.getServiceManager();
    }


    public static XMultiServiceFactory connect(String connectStr) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException, Exception {
        XComponentContext xcomponentcontext = null;
        XMultiComponentFactory xMultiComponentFactory = getMultiComponentFactory();
        // create a connector, so that it can contact the office
        Object xUrlResolver = xMultiComponentFactory.createInstanceWithContext("com.sun.star.bridge.UnoUrlResolver", xcomponentcontext);
        XUnoUrlResolver urlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(XUnoUrlResolver.class, xUrlResolver);
        Object rInitialObject = urlResolver.resolve(connectStr);
        XNamingService rName = (XNamingService) UnoRuntime.queryInterface(XNamingService.class, rInitialObject);
        XMultiServiceFactory xMSF = null;
        if (rName != null) {
            System.err.println("got the remote naming service !");
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager");
            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, rXsmgr);
        }
        return (xMSF);
    }

    public static String getIncrementSuffix(XNameAccess xElementContainer, String ElementName) {
        boolean bElementexists = true;
        int i = 1;
        String sIncSuffix;
        String[] sControlNames = xElementContainer.getElementNames();
        String BaseName = ElementName;
        while (bElementexists == true) {
            bElementexists = xElementContainer.hasByName(ElementName);
            if (bElementexists == true) {
                i += 1;
                ElementName = BaseName + "_" + Integer.toString(i);
            }
        }
        if (i == 1)
            sIncSuffix = "";
        else
            sIncSuffix = "_" + Integer.toString(i);
        return sIncSuffix;
    }

    /**
     * Checks if the passed Element Name already exists in the  ElementContainer. If yes it appends a
     * suffix to make it unique
     * @param xElementContainer
     * @param sElementName
     * @return a unique Name ready to be added to the container.
     */

    public static String getUniqueName(XNameAccess xElementContainer, String sElementName) {
        String sIncSuffix = getIncrementSuffix(xElementContainer, sElementName);
        if (sIncSuffix == "")
            return sElementName;
        else
            return sElementName + sIncSuffix;
    }


    /**
     * @deprecated  use Configuration.getConfigurationRoot() with the same parameters instead
     * @param xMSF
     * @param KeyName
     * @param bForUpdate
     * @return
     */
    public static XInterface getRegistryKeyContent(XMultiServiceFactory xMSF, String KeyName, boolean bForUpdate) {
        try {
            Object oConfigProvider;
            PropertyValue[] aNodePath = new PropertyValue[1];
            oConfigProvider = xMSF.createInstance("com.sun.star.configuration.ConfigurationProvider");
            aNodePath[0] = new PropertyValue();
            aNodePath[0].Name = "nodepath";
            aNodePath[0].Value = KeyName;
            XMultiServiceFactory xMSFConfig = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, oConfigProvider);
            if (bForUpdate == true)
                return (XInterface) xMSFConfig.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess", aNodePath);
            else
                return (XInterface) xMSFConfig.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", aNodePath);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }


    /**
     * @deprecated use Configuration.getProductName instead
     * @param xMSF
     * @return
     */
    public static String getProductName(XMultiServiceFactory xMSF) {
    try {
        Object oProdNameAccess = Configuration.getConfigurationRoot(xMSF, "org.openoffice.Setup/Product", false);
        String ProductName = (String) Helper.getUnoObjectbyName(oProdNameAccess, "ooName");
        return ProductName;
    } catch (Exception exception) {
        exception.printStackTrace(System.out);
        return null;
    }}


    public static XNumberFormatter createNumberFormatter(XMultiServiceFactory _xMSF, XNumberFormatsSupplier _xNumberFormatsSupplier) throws Exception{
        Object oNumberFormatter = _xMSF.createInstance("com.sun.star.util.NumberFormatter");
        XNumberFormatter xNumberFormatter = (XNumberFormatter) UnoRuntime.queryInterface(XNumberFormatter.class, oNumberFormatter);
        xNumberFormatter.attachNumberFormatsSupplier(_xNumberFormatsSupplier);
        return xNumberFormatter;
    }


    public static String convertNumberToString(XNumberFormatter _xNumberFormatter, int _nkey, double _dblValue){
        return _xNumberFormatter.convertNumberToString(_nkey, _dblValue);
    }


    public static double convertStringToNumber(XNumberFormatter _xNumberFormatter, int _nkey, String _sString)throws Exception{
        return _xNumberFormatter.convertStringToNumber(_nkey, _sString);
    }


    /**
     * returns a numberformat for a FormatString. As Locale the american english locale is assumed
     * @param _xFormatObject
     * @param _xNumberFormats
     * @param FormatString
     * @return
     */
    public static int defineNumberFormat(XNumberFormats _xNumberFormats, String _FormatString){
        Locale oLocale = new Locale();
        oLocale.Country = "US";
        oLocale.Language = "en";
        return (defineNumberFormat(_xNumberFormats, _FormatString, oLocale));
    }


    /**
     * returns a numberformat for a FormatString.
     * @param _xFormatObject
     * @param _xNumberFormats
     * @param FormatString
     * @return
     */
    public static int defineNumberFormat(XNumberFormats _xNumberFormats, String FormatString, Locale _oLocale){
    try {
        int NewFormatKey = _xNumberFormats.queryKey(FormatString, _oLocale, true);
        if (NewFormatKey == -1)
            NewFormatKey = _xNumberFormats.addNew(FormatString, _oLocale);
        return NewFormatKey;
    } catch (MalformedNumberFormatException e) {
        e.printStackTrace(System.out);
        return -1;
    }}



    public static void setNumberFormat(XInterface _xFormatObject, XNumberFormats _xNumberFormats, int _FormatKey) {
        try {
            XPropertySet xNumberFormat = _xNumberFormats.getByKey(_FormatKey); //CurDBField.DBFormatKey);
            String FormatString = AnyConverter.toString(Helper.getUnoPropertyValue(xNumberFormat, "FormatString"));
            Locale oLocale = (Locale) Helper.getUnoPropertyValue(xNumberFormat, "Locale");
            int NewFormatKey = defineNumberFormat(_xNumberFormats, FormatString, oLocale);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, _xFormatObject);
            if (xPSet.getPropertySetInfo().hasPropertyByName("NumberFormat"))
                xPSet.setPropertyValue("NumberFormat", new Integer(NewFormatKey));
            else if (xPSet.getPropertySetInfo().hasPropertyByName("FormatKey"))
                xPSet.setPropertyValue("FormatKey", new Integer(NewFormatKey));
            else
                throw new Exception();
        } catch (Exception exception){
            exception.printStackTrace(System.out);
        }
    }


    /**
     * used to retrieve the most common paths used in the office application
     * @author bc93774
     *
     */
    public class OfficePathRetriever {
        public String TemplatePath;
        public String BitmapPath;
        public String UserTemplatePath;
        public String WorkPath;

        public OfficePathRetriever(XMultiServiceFactory xMSF) {
            try {
                TemplatePath = FileAccess.getOfficePath(xMSF, "Template", "share");
                UserTemplatePath = FileAccess.getOfficePath(xMSF, "Template", "user");
                BitmapPath = FileAccess.combinePaths(xMSF, TemplatePath, "/wizard/bitmap");
                WorkPath = FileAccess.getOfficePath(xMSF, "Work", "");
            } catch (NoValidPathException nopathexception) {
            }
        }
    }

    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF) {
        Object xPathSubst = null;
        try {
            xPathSubst = xMSF.createInstance("com.sun.star.util.PathSubstitution");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        if (xPathSubst != null)
            return (XStringSubstitution) UnoRuntime.queryInterface(XStringSubstitution.class, xPathSubst);
        else
            return null;
    }

    /**
     * This method searches (and hopefully finds...) a frame
     * with a componentWindow.
     * It does it in three phases:
     * 1. Check if the given desktop argument has a componentWindow.
     * If it is null, the myFrame argument is taken.
     * 2. Go up the tree of frames and search a frame with a component window.
     * 3. Get from the desktop all the components, and give the first one
     * which has a frame.
     * @param xMSF
     * @param myFrame
     * @param desktop
     * @return
     * @throws NoSuchElementException
     * @throws WrappedTargetException
     */
    public static XFrame findAFrame(XMultiServiceFactory xMSF, XFrame myFrame, XFrame desktop)
        throws NoSuchElementException,
                WrappedTargetException
    {
        if (desktop == null)
            desktop = myFrame;

        // we go up in the tree...
        while (desktop != null && desktop.getComponentWindow() == null)
            desktop = desktop.findFrame("_parent", FrameSearchFlag.PARENT);

        if (desktop == null) {

            for (XEnumeration e = Desktop.getDesktop(xMSF).getComponents().createEnumeration(); e.hasMoreElements();) {

                Object comp = ((Any) e.nextElement()).getObject();
                XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, comp);
                XFrame xFrame = xModel.getCurrentController().getFrame();

                if (xFrame != null && xFrame.getComponentWindow() != null)
                    return xFrame;
            }
        }
        return desktop;
    }
}
