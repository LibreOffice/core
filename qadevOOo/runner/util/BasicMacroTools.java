/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BasicMacroTools.java,v $
 * $Revision: 1.7 $
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
package util;

import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.XLibraryContainer;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.*;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

import java.util.Vector;



public class BasicMacroTools {
    private final XDispatchProvider mDispProv;
    private final XMultiServiceFactory mMSF;
    private final XURLTransformer mParser;
    private final XNameAccess mLCxNA; //LibraryContainer::XNameAccess
    private final XLibraryContainer mLCxLC; //LibraryContainer::XLibraryContainer

    /*
     *While initializing the Basic Libraries will be appendend to the Document
     */
    public BasicMacroTools(XMultiServiceFactory msf, XModel xModel,
                           XComponent xDoc) throws java.lang.Exception {
        try {
            mMSF = msf;
            mDispProv = makeDispatchProvider(mMSF, xModel);
            mParser = makeParser(mMSF);

            Object DocLibCont = null;

            try {
                XPropertySet xDocProps = (XPropertySet) UnoRuntime.queryInterface(
                                                 XPropertySet.class, xDoc);
                DocLibCont = xDocProps.getPropertyValue("BasicLibraries");
            } catch (com.sun.star.uno.Exception e) {
                throw new Exception(
                        "Couldn't get BasicLibraries-Container from document: " + e.toString());
            }

            mLCxNA = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                             DocLibCont);

            mLCxLC = (XLibraryContainer) UnoRuntime.queryInterface(
                             XLibraryContainer.class, DocLibCont);

        } catch (Exception e) {
            throw new Exception("could not initialize BasicMacros " +
                                e.toString());
        }
    }

    /*
     * While initializing the Basic Libraries will be appendend to the Office
    */
    public BasicMacroTools(XMultiServiceFactory msf, XModel xModel)
                    throws java.lang.Exception {
        try {
            mMSF = msf;
            mDispProv = makeDispatchProvider(mMSF, xModel);
            mParser = makeParser(mMSF);

            Object ASLC = null;

            try {
                ASLC = mMSF.createInstance(
                               "com.sun.star.script.ApplicationScriptLibraryContainer");
            } catch (com.sun.star.uno.Exception e) {
                throw new Exception(
                        "Couldn't create ApplicationScriptLibraryContainer" + e.toString());
            }

            mLCxNA = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                             ASLC);

            mLCxLC = (XLibraryContainer) UnoRuntime.queryInterface(
                             XLibraryContainer.class, ASLC);

        } catch (Exception e) {
            throw new Exception("could not initialize BasicMacros " +
                                e.toString());
        }
    }

    private static XDispatchProvider makeDispatchProvider(XMultiServiceFactory mMSF,
                                                          XModel aModel)
                                                   throws java.lang.Exception {
        XController xController = aModel.getCurrentController();
        XFrame xFrame = xController.getFrame();

        if (xFrame == null) {
            throw new Exception("Could not create DispatchProvider");
        }

        return (XDispatchProvider) UnoRuntime.queryInterface(
                       XDispatchProvider.class, xFrame);
    }

    private static XURLTransformer makeParser(XMultiServiceFactory mMSF)
                                       throws java.lang.Exception {
        try {
            return (com.sun.star.util.XURLTransformer) UnoRuntime.queryInterface(
                           XURLTransformer.class, mMSF.createInstance(
                                   "com.sun.star.util.URLTransformer"));
        } catch (Exception e) {
            throw new Exception("could not create UTL-Transformer " +
                                e.toString());
        }
    }

    public void loadLibrary(String LibraryName, String LibraryURL)
                     throws java.lang.Exception {
        try {
            appendLibrary(LibraryName, LibraryURL);
        } catch (java.lang.Exception e) {
            e.printStackTrace();
            throw new Exception("ERROR: Could not append Library " +
                                LibraryName + e.toString());
        }

        try {
            mLCxLC.loadLibrary(LibraryName);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace();
            throw new Exception("ERROR: Could not load Library " +
                                LibraryName + e.toString());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new Exception("ERROR: Could not load Library " +
                                LibraryName + e.toString());
        }
    }

    private void appendLibrary(String LibraryName, String LibraryURL)
                        throws java.lang.Exception {
        try {
            removeLibrary(LibraryName);
        } catch (java.lang.Exception e) {
        }

        try {
            mLCxLC.createLibraryLink(LibraryName, LibraryURL, false);
        } catch (com.sun.star.container.ElementExistException e) {
            e.printStackTrace();
            throw new Exception("ERROR: Library " + LibraryName +
                                "already exist." + e.toString());
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
            throw new Exception("Could not link Basic library:" +
                                LibraryName + e.toString());
        }
    }

    public void removeLibrary(String LibraryName) throws java.lang.Exception {
        if (mLCxNA.hasByName(LibraryName)) {
            try {
                mLCxLC.removeLibrary(LibraryName);
            } catch (com.sun.star.container.NoSuchElementException e) {
                e.printStackTrace();
                throw new Exception("Could not remove Basic library:" +
                                    LibraryName + ": Library does not exist" + e.toString());
            } catch (com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace();
                throw new Exception("Could not remove Basic library:" +
                                    LibraryName + e.toString());
            }
        }
    }

    public void runMarco(String MacroName) throws java.lang.Exception {
        URL[] aParseURL = new URL[1];
        aParseURL[0] = new URL();
        aParseURL[0].Complete = "macro://./" + MacroName; //Standard.Stock.GetSymbol('micro','')";
        mParser.parseStrict(aParseURL);

        URL aURL = aParseURL[0];
        XDispatch xDispatcher = mDispProv.queryDispatch(aURL, "", 0);

        if (xDispatcher != null) {
            xDispatcher.dispatch(aURL, null);
        } else {
            throw new Exception("Could not run Macro " + MacroName);
        }
    }

    /**
     * Set the given <CODE>secureURL</CODE> as secure URL for marco execution.
     * The macros of documents located in <CODE>secureURL</CODE> will be executed
     * automatically.
     * @param xMSF the XMultiServiceFactory
     * @param secureURL the URL the documet is located
     * @throws java.lang.Exception throws this exception on any error
     */
    public static void addSecureBasicMarcosURL(XMultiServiceFactory xMSF, String secureURL)
        throws Exception {

        secureURL = utils.getFullURL(secureURL);

        // configure Office to allow to execute macos
        PropertyValue[] ProvArgs = new PropertyValue [1];
        PropertyValue Arg = new PropertyValue();
        Arg.Name = "nodepath";
        Arg.Value = "/org.openoffice.Office.Common/Security";
        ProvArgs[0] = Arg;

        Object oProvider = xMSF.createInstance("com.sun.star.configuration.ConfigurationProvider");


        XMultiServiceFactory oProviderMSF = (XMultiServiceFactory)
                        UnoRuntime.queryInterface(XMultiServiceFactory.class, oProvider);

        Object oSecure = oProviderMSF.createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess",
            ProvArgs);

        XPropertySet oSecureProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oSecure);

        Object oScripting = oSecureProps.getPropertyValue("Scripting");
        XPropertySet oScriptingSettings = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oScripting);

        oScriptingSettings.setPropertyValue("SecureURL", new String[]{secureURL});
        oScriptingSettings.setPropertyValue("OfficeBasic", new Integer(2));

        XChangesBatch oSecureChange = (XChangesBatch) UnoRuntime.queryInterface(XChangesBatch.class, oSecure);
        oSecureChange.commitChanges();
    }
}