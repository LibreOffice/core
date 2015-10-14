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
package com.sun.star.wizards.common;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XURLTransformer;
import com.sun.star.lang.Locale;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.*;
import com.sun.star.i18n.KParseType;
import com.sun.star.i18n.ParseResult;
import com.sun.star.i18n.XCharacterClassification;

public class Desktop
{

    public static XDesktop getDesktop(XMultiServiceFactory xMSF)
    {
        com.sun.star.uno.XInterface xInterface = null;
        XDesktop xDesktop = null;
        if (xMSF != null)
        {
            try
            {
                xInterface = (com.sun.star.uno.XInterface) xMSF.createInstance("com.sun.star.frame.Desktop");
                xDesktop = UnoRuntime.queryInterface(XDesktop.class, xInterface);
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.err);
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
        XFramesSupplier xFrameSuppl = UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);
        return xFrameSuppl.getActiveFrame();
    }

    private static XDispatch getDispatcher(XFrame xFrame, String _stargetframe, com.sun.star.util.URL oURL)
    {
        try
        {
            com.sun.star.util.URL[] oURLArray = new com.sun.star.util.URL[1];
            oURLArray[0] = oURL;
            XDispatchProvider xDispatchProvider = UnoRuntime.queryInterface(XDispatchProvider.class, xFrame);
            return xDispatchProvider.queryDispatch(oURLArray[0], _stargetframe, FrameSearchFlag.ALL); // "_self"
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return null;
    }

    private static com.sun.star.util.URL getDispatchURL(XMultiServiceFactory xMSF, String _sURL)
    {
        try
        {
            Object oTransformer = xMSF.createInstance("com.sun.star.util.URLTransformer");
            XURLTransformer xTransformer = UnoRuntime.queryInterface(XURLTransformer.class, oTransformer);
            com.sun.star.util.URL[] oURL = new com.sun.star.util.URL[1];
            oURL[0] = new com.sun.star.util.URL();
            oURL[0].Complete = _sURL;
            xTransformer.parseStrict(oURL);
            return oURL[0];
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return null;
    }

    private static void dispatchURL(XMultiServiceFactory xMSF, String sURL, XFrame xFrame, String _stargetframe)
    {
        com.sun.star.util.URL oURL = getDispatchURL(xMSF, sURL);
        XDispatch xDispatch = getDispatcher(xFrame, _stargetframe, oURL);
        dispatchURL(xDispatch, oURL);
    }

    public static void dispatchURL(XMultiServiceFactory xMSF, String sURL, XFrame xFrame)
    {
        dispatchURL(xMSF, sURL, xFrame, PropertyNames.EMPTY_STRING);
    }

    private static void dispatchURL(XDispatch _xDispatch, com.sun.star.util.URL oURL)
    {
        PropertyValue[] oArg = new PropertyValue[0];
        _xDispatch.dispatch(oURL, oArg);
    }

    private static XMultiComponentFactory getMultiComponentFactory() throws com.sun.star.uno.Exception, RuntimeException, java.lang.Exception
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
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, urlResolver.resolve( connectStr ) );
    }

    public static String getIncrementSuffix(XNameAccess xElementContainer, String ElementName)
    {
        boolean bElementexists = true;
        int i = 1;
        String sIncSuffix = PropertyNames.EMPTY_STRING;
        String BaseName = ElementName;
        while (bElementexists)
        {
            bElementexists = xElementContainer.hasByName(ElementName);
            if (bElementexists)
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

    private static String getIncrementSuffix(XHierarchicalNameAccess xElementContainer, String ElementName)
    {
        boolean bElementexists = true;
        int i = 1;
        String sIncSuffix = PropertyNames.EMPTY_STRING;
        String BaseName = ElementName;
        while (bElementexists)
        {
            bElementexists = xElementContainer.hasByHierarchicalName(ElementName);
            if (bElementexists)
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

    private static int checkforfirstSpecialCharacter(XMultiServiceFactory _xMSF, String _sString, Locale _aLocale)
    {
        try
        {
            int nStartFlags = com.sun.star.i18n.KParseTokens.ANY_LETTER_OR_NUMBER + com.sun.star.i18n.KParseTokens.ASC_UNDERSCORE;
            Object ocharservice = _xMSF.createInstance("com.sun.star.i18n.CharacterClassification");
            XCharacterClassification xCharacterClassification = UnoRuntime.queryInterface(XCharacterClassification.class, ocharservice);
            ParseResult aResult = xCharacterClassification.parsePredefinedToken(KParseType.IDENTNAME, _sString, 0, _aLocale, nStartFlags, PropertyNames.EMPTY_STRING, nStartFlags, PropertyNames.SPACE);
            return aResult.EndPos;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
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
                snewname = JavaTools.replaceSubString(snewname, PropertyNames.EMPTY_STRING, sspecialchar);
            }
        }
        return snewname;
    }

    /**
     * Checks if the passed Element Name already exists in the  ElementContainer. If yes it appends a
     * suffix to make it unique
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
     * @return a unique Name ready to be added to the container.
     */
    public static String getUniqueName(XHierarchicalNameAccess xElementContainer, String sElementName)
    {
        String sIncSuffix = getIncrementSuffix(xElementContainer, sElementName);
        return sElementName + sIncSuffix;
    }

    /**
     * Checks if the passed Element Name already exists in the list. If yes it appends a
     * suffix to make it unique.
     * @return a unique Name not being in the passed list.
     */
    public static String getUniqueName(String[] _slist, String _sElementName, String _sSuffixSeparator)
    {
        if (_slist == null || _slist.length == 0)
        {
            return _sElementName;
        }
        String scompname = _sElementName;
        int a = 2;
        while (true)
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
    }



    private static String getTemplatePath(XMultiServiceFactory _xMSF)
    {
        try
        {
            return FileAccess.getOfficePath(_xMSF, "Template", "share", "/wizard");
        }
        catch (NoValidPathException nopathexception)
        {
        }
        return PropertyNames.EMPTY_STRING;
    }



    public static String getBitmapPath(XMultiServiceFactory _xMSF)
    {
        try
        {
            return FileAccess.combinePaths(_xMSF, getTemplatePath(_xMSF), "/../wizard/bitmap");
        }
        catch (NoValidPathException nopathexception)
        {
        }
        return PropertyNames.EMPTY_STRING;
    }





}
