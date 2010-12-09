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
 ************************************************************************/
package com.sun.star.wizards.common;

// import java.util.Date;

// import com.sun.star.awt.XToolkit;
import com.sun.star.beans.PropertyValue;
// import com.sun.star.frame.XDesktop;
// import com.sun.star.frame.XFrame;
// import com.sun.star.frame.XFramesSupplier;

import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XNamingService;
import com.sun.star.util.XURLTransformer;
import com.sun.star.lang.Locale;
import com.sun.star.uno.XInterface;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.util.XStringSubstitution;
import com.sun.star.frame.*;
import com.sun.star.i18n.KParseType;
import com.sun.star.i18n.ParseResult;
import com.sun.star.i18n.XCharacterClassification;

public class Desktop
{

    /** Creates a new instance of Desktop */
    public Desktop()
    {
    }

    public static XDesktop getDesktop(XMultiServiceFactory xMSF)
    {
        com.sun.star.uno.XInterface xInterface = null;
        XDesktop xDesktop = null;
        if (xMSF != null)
        {
            try
            {
                xInterface = (com.sun.star.uno.XInterface) xMSF.createInstance("com.sun.star.frame.Desktop");
                xDesktop = (XDesktop) UnoRuntime.queryInterface(XDesktop.class, xInterface);
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }
        else
        {
            System.out.println("Can't create a desktop. null pointer !");
        }
        return xDesktop;
    }

    public static XFrame getActiveFrame(XMultiServiceFactory xMSF)
    {
        XDesktop xDesktop = getDesktop(xMSF);
        XFramesSupplier xFrameSuppl = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);
        XFrame xFrame = xFrameSuppl.getActiveFrame();
        return xFrame;
    }

    public static XComponent getActiveComponent(XMultiServiceFactory _xMSF)
    {
        XFrame xFrame = getActiveFrame(_xMSF);
        return (XComponent) UnoRuntime.queryInterface(XComponent.class, xFrame.getController().getModel());
    }

    public static XTextDocument getActiveTextDocument(XMultiServiceFactory _xMSF)
    {
        XComponent xComponent = getActiveComponent(_xMSF);
        return (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);
    }

    public static XSpreadsheetDocument getActiveSpreadsheetDocument(XMultiServiceFactory _xMSF)
    {
        XComponent xComponent = getActiveComponent(_xMSF);
        return (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, xComponent);
    }

    public static XDispatch getDispatcher(XMultiServiceFactory xMSF, XFrame xFrame, String _stargetframe, com.sun.star.util.URL oURL)
    {
        try
        {
            com.sun.star.util.URL[] oURLArray = new com.sun.star.util.URL[1];
            oURLArray[0] = oURL;
            XDispatchProvider xDispatchProvider = (XDispatchProvider) UnoRuntime.queryInterface(XDispatchProvider.class, xFrame);
            XDispatch xDispatch = xDispatchProvider.queryDispatch(oURLArray[0], _stargetframe, FrameSearchFlag.ALL); // "_self"
            return xDispatch;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        return null;
    }

    public static com.sun.star.util.URL getDispatchURL(XMultiServiceFactory xMSF, String _sURL)
    {
        try
        {
            Object oTransformer = xMSF.createInstance("com.sun.star.util.URLTransformer");
            XURLTransformer xTransformer = (XURLTransformer) UnoRuntime.queryInterface(XURLTransformer.class, oTransformer);
            com.sun.star.util.URL[] oURL = new com.sun.star.util.URL[1];
            oURL[0] = new com.sun.star.util.URL();
            oURL[0].Complete = _sURL;
            xTransformer.parseStrict(oURL);
            return oURL[0];
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        return null;
    }

    public static void dispatchURL(XMultiServiceFactory xMSF, String sURL, XFrame xFrame, String _stargetframe)
    {
        com.sun.star.util.URL oURL = getDispatchURL(xMSF, sURL);
        XDispatch xDispatch = getDispatcher(xMSF, xFrame, _stargetframe, oURL);
        dispatchURL(xDispatch, oURL);
    }

    public static void dispatchURL(XMultiServiceFactory xMSF, String sURL, XFrame xFrame)
    {
        dispatchURL(xMSF, sURL, xFrame, "");
    }

    public static void dispatchURL(XDispatch _xDispatch, com.sun.star.util.URL oURL)
    {
        PropertyValue[] oArg = new PropertyValue[0];
        _xDispatch.dispatch(oURL, oArg);
    }

    public static XMultiComponentFactory getMultiComponentFactory() throws com.sun.star.uno.Exception, RuntimeException, java.lang.Exception
    {
        XComponentContext xcomponentcontext = Bootstrap.createInitialComponentContext(null);
        // initial serviceManager
        return xcomponentcontext.getServiceManager();
    }

    public static XMultiServiceFactory connect(String connectStr) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException, Exception
    {
        XMultiComponentFactory componentFactory = getMultiComponentFactory();
        Object xUrlResolver = componentFactory.createInstanceWithContext( "com.sun.star.bridge.UnoUrlResolver", null );
        XUnoUrlResolver urlResolver = UnoRuntime.queryInterface(XUnoUrlResolver.class, xUrlResolver);
        XMultiServiceFactory orb = UnoRuntime.queryInterface(XMultiServiceFactory.class, urlResolver.resolve( connectStr ) );
        return orb;
    }

    public static String getIncrementSuffix(XNameAccess xElementContainer, String ElementName)
    {
        boolean bElementexists = true;
        int i = 1;
        String sIncSuffix = "";
        String BaseName = ElementName;
        while (bElementexists == true)
        {
            bElementexists = xElementContainer.hasByName(ElementName);
            if (bElementexists == true)
            {
                i += 1;
                ElementName = BaseName + Integer.toString(i);
            }
        }
        if (i > 1)
        {
            sIncSuffix = Integer.toString(i);
        }
        return sIncSuffix;
    }

    public static String getIncrementSuffix(XHierarchicalNameAccess xElementContainer, String ElementName)
    {
        boolean bElementexists = true;
        int i = 1;
        String sIncSuffix = "";
        String BaseName = ElementName;
        while (bElementexists == true)
        {
            bElementexists = xElementContainer.hasByHierarchicalName(ElementName);
            if (bElementexists == true)
            {
                i += 1;
                ElementName = BaseName + Integer.toString(i);
            }
        }
        if (i > 1)
        {
            sIncSuffix = Integer.toString(i);
        }
        return sIncSuffix;
    }

    public static int checkforfirstSpecialCharacter(XMultiServiceFactory _xMSF, String _sString, Locale _aLocale)
    {
        try
        {
            int nStartFlags = com.sun.star.i18n.KParseTokens.ANY_LETTER_OR_NUMBER + com.sun.star.i18n.KParseTokens.ASC_UNDERSCORE;
            int nContFlags = nStartFlags;
            Object ocharservice = _xMSF.createInstance("com.sun.star.i18n.CharacterClassification");
            XCharacterClassification xCharacterClassification = (XCharacterClassification) UnoRuntime.queryInterface(XCharacterClassification.class, ocharservice);
            ParseResult aResult = xCharacterClassification.parsePredefinedToken(KParseType.IDENTNAME, _sString, 0, _aLocale, nStartFlags, "", nContFlags, " ");
            return aResult.EndPos;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
            return -1;
        }
    }

    public static String removeSpecialCharacters(XMultiServiceFactory _xMSF, Locale _aLocale, String _sname)
    {
        String snewname = _sname;
        int i = 0;
        while (i < snewname.length())
        {
            i = Desktop.checkforfirstSpecialCharacter(_xMSF, snewname, _aLocale);
            if (i < snewname.length())
            {
                String sspecialchar = snewname.substring(i, i + 1);
                snewname = JavaTools.replaceSubString(snewname, "", sspecialchar);
            }
        }
        return snewname;
    }

    /**
     * Checks if the passed Element Name already exists in the  ElementContainer. If yes it appends a
     * suffix to make it unique
     * @param xElementContainer
     * @param sElementName
     * @return a unique Name ready to be added to the container.
     */
    public static String getUniqueName(XNameAccess xElementContainer, String sElementName)
    {
        String sIncSuffix = getIncrementSuffix(xElementContainer, sElementName);
        return sElementName + sIncSuffix;
    }

    /**
     * Checks if the passed Element Name already exists in the  ElementContainer. If yes it appends a
     * suffix to make it unique
     * @param xElementContainer
     * @param sElementName
     * @return a unique Name ready to be added to the container.
     */
    public static String getUniqueName(XHierarchicalNameAccess xElementContainer, String sElementName)
    {
        String sIncSuffix = getIncrementSuffix(xElementContainer, sElementName);
        return sElementName + sIncSuffix;
    }

    /**
     * Checks if the passed Element Name already exists in the list If yes it appends a
     * suffix to make it unique
     * @param _slist
     * @param _sElementName
     * @param _sSuffixSeparator
     * @return a unique Name not being in the passed list.
     */
    public static String getUniqueName(String[] _slist, String _sElementName, String _sSuffixSeparator)
    {
        int a = 2;
        String scompname = _sElementName;
        boolean bElementexists = true;
        if (_slist == null)
        {
            return _sElementName;
        }
        if (_slist.length == 0)
        {
            return _sElementName;
        }
        while (bElementexists == true)
        {
            for (int i = 0; i < _slist.length; i++)
            {
                if (JavaTools.FieldInList(_slist, scompname) == -1)
                {
                    return scompname;
                }
            }
            scompname = _sElementName + _sSuffixSeparator + a++;
        }
        return "";
    }

    /**
     * @deprecated  use Configuration.getConfigurationRoot() with the same parameters instead
     * @param xMSF
     * @param KeyName
     * @param bForUpdate
     * @return
     */
    public static XInterface getRegistryKeyContent(XMultiServiceFactory xMSF, String KeyName, boolean bForUpdate)
    {
        try
        {
            Object oConfigProvider;
            PropertyValue[] aNodePath = new PropertyValue[1];
            oConfigProvider = xMSF.createInstance("com.sun.star.configuration.ConfigurationProvider");
            aNodePath[0] = new PropertyValue();
            aNodePath[0].Name = "nodepath";
            aNodePath[0].Value = KeyName;
            XMultiServiceFactory xMSFConfig = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, oConfigProvider);
            if (bForUpdate == true)
            {
                return (XInterface) xMSFConfig.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess", aNodePath);
            }
            else
            {
                return (XInterface) xMSFConfig.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", aNodePath);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    /**
     * @deprecated used to retrieve the most common paths used in the office application
     * @author bc93774
     *
     */
    public class OfficePathRetriever
    {

        public String TemplatePath;
        public String BitmapPath;
        public String UserTemplatePath;
        public String WorkPath;

        public OfficePathRetriever(XMultiServiceFactory xMSF)
        {
            try
            {
                TemplatePath = FileAccess.getOfficePath(xMSF, "Template", "share", "/wizard");
                UserTemplatePath = FileAccess.getOfficePath(xMSF, "Template", "user", "");
                BitmapPath = FileAccess.combinePaths(xMSF, TemplatePath, "/../wizard/bitmap");
                WorkPath = FileAccess.getOfficePath(xMSF, "Work", "", "");
            }
            catch (NoValidPathException nopathexception)
            {
            }
        }
    }

    public static String getTemplatePath(XMultiServiceFactory _xMSF)
    {
        try
        {
            String sTemplatePath = FileAccess.getOfficePath(_xMSF, "Template", "share", "/wizard");
            return sTemplatePath;
        }
        catch (NoValidPathException nopathexception)
        {
        }
        return "";
    }

    public static String getUserTemplatePath(XMultiServiceFactory _xMSF)
    {
        try
        {
            String sUserTemplatePath = FileAccess.getOfficePath(_xMSF, "Template", "user", "");
            return sUserTemplatePath;
        }
        catch (NoValidPathException nopathexception)
        {
        }
        return "";
    }

    public static String getBitmapPath(XMultiServiceFactory _xMSF)
    {
        try
        {
            String sBitmapPath = FileAccess.combinePaths(_xMSF, getTemplatePath(_xMSF), "/../wizard/bitmap");
            return sBitmapPath;
        }
        catch (NoValidPathException nopathexception)
        {
        }
        return "";
    }

    public static String getWorkPath(XMultiServiceFactory _xMSF)
    {
        try
        {
            String sWorkPath = FileAccess.getOfficePath(_xMSF, "Work", "", "");
            return sWorkPath;
        }
        catch (NoValidPathException nopathexception)
        {
        }
        return "";
    }

    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF)
    {
        Object xPathSubst = null;
        try
        {
            xPathSubst = xMSF.createInstance("com.sun.star.util.PathSubstitution");
        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace();
        }
        if (xPathSubst != null)
        {
            return (XStringSubstitution) UnoRuntime.queryInterface(XStringSubstitution.class, xPathSubst);
        }
        else
        {
            return null;
        }
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
        {
            desktop = myFrame;        // we go up in the tree...
        }
        while (desktop != null && desktop.getComponentWindow() == null)
        {
            desktop = desktop.findFrame("_parent", FrameSearchFlag.PARENT);
        }
        if (desktop == null)
        {

            for (XEnumeration e = Desktop.getDesktop(xMSF).getComponents().createEnumeration(); e.hasMoreElements();)
            {

                Object comp = ((Any) e.nextElement()).getObject();
                XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, comp);
                XFrame xFrame = xModel.getCurrentController().getFrame();

                if (xFrame != null && xFrame.getComponentWindow() != null)
                {
                    return xFrame;
                }
            }
        }
        return desktop;
    }
}
