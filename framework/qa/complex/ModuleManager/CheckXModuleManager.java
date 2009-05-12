/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CheckXModuleManager.java,v $
 * $Revision: 1.3 $
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

package complex.ModuleManager;

import com.sun.star.beans.*;
import com.sun.star.frame.*;
import com.sun.star.lang.*;
import com.sun.star.uno.*;
import com.sun.star.util.*;
import com.sun.star.container.*;

import complexlib.ComplexTestCase;

import helper.URLHelper;

import java.lang.*;
import java.util.*;

//-----------------------------------------------
/** @short  todo document me
 */
public class CheckXModuleManager extends ComplexTestCase
{
    //-------------------------------------------
    // some const

    //-------------------------------------------
    // member

    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xSmgr = null;

    /** the module manager for testing. */
    private XModuleManager m_xMM = null;

    /** a special frame used to load documents there. */
    private XComponentLoader m_xLoader = null;

    //-------------------------------------------
    // test environment

    //-------------------------------------------
    /** @short  A function to tell the framework,
                which test functions are available.

        @return All test methods.
        @todo   Think about selection of tests from outside ...
     */
    public String[] getTestMethodNames()
    {
        return new String[]
        {
            "checkModuleIdentification"        ,
            "checkModuleConfigurationReadable" ,
            "checkModuleConfigurationWriteable",
            "checkModuleConfigurationQueries"
        };
    }

    //-------------------------------------------
    /** @short  Create the environment for following tests.

        @descr  Use either a component loader from desktop or
                from frame
     */
    public void before()
        throws java.lang.Exception
    {
        // get uno service manager from global test environment
        m_xSmgr = (XMultiServiceFactory)param.getMSF();

        // create module manager
        m_xMM = (XModuleManager)UnoRuntime.queryInterface(
                    XModuleManager.class,
                    m_xSmgr.createInstance("com.sun.star.frame.ModuleManager"));

        // create desktop instance to create a special frame to load documents there.
        XFrame xDesktop = (XFrame)UnoRuntime.queryInterface(
                                XFrame.class,
                                m_xSmgr.createInstance("com.sun.star.frame.Desktop"));

        m_xLoader = (XComponentLoader)UnoRuntime.queryInterface(
                                XComponentLoader.class,
                                xDesktop.findFrame("_blank", 0));
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    public void after()
        throws java.lang.Exception
    {
        XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(
                                XCloseable.class,
                                m_xLoader);
        xClose.close(false);

        m_xLoader = null;
        m_xMM     = null;
        m_xSmgr   = null;
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkModuleIdentification()
        throws java.lang.Exception
    {
        impl_identifyModulesBasedOnDocs("com.sun.star.text.TextDocument"                );
        impl_identifyModulesBasedOnDocs("com.sun.star.text.WebDocument"                 );
        impl_identifyModulesBasedOnDocs("com.sun.star.text.GlobalDocument"              );
        impl_identifyModulesBasedOnDocs("com.sun.star.formula.FormulaProperties"        );
        impl_identifyModulesBasedOnDocs("com.sun.star.sheet.SpreadsheetDocument"        );
        impl_identifyModulesBasedOnDocs("com.sun.star.drawing.DrawingDocument"          );
        impl_identifyModulesBasedOnDocs("com.sun.star.presentation.PresentationDocument");
        impl_identifyModulesBasedOnDocs("com.sun.star.sdb.OfficeDatabaseDocument"       );
        impl_identifyModulesBasedOnDocs("com.sun.star.chart.ChartDocument"              );
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkModuleConfigurationReadable()
        throws java.lang.Exception
    {
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkModuleConfigurationWriteable()
        throws java.lang.Exception
    {
        // modules supporting real documents
        impl_checkReadOnlyPropsOfModule("com.sun.star.text.TextDocument"                );
        impl_checkReadOnlyPropsOfModule("com.sun.star.text.WebDocument"                 );
        impl_checkReadOnlyPropsOfModule("com.sun.star.text.GlobalDocument"              );
        impl_checkReadOnlyPropsOfModule("com.sun.star.formula.FormulaProperties"        );
        impl_checkReadOnlyPropsOfModule("com.sun.star.sheet.SpreadsheetDocument"        );
        impl_checkReadOnlyPropsOfModule("com.sun.star.drawing.DrawingDocument"          );
        impl_checkReadOnlyPropsOfModule("com.sun.star.presentation.PresentationDocument");
        impl_checkReadOnlyPropsOfModule("com.sun.star.sdb.OfficeDatabaseDocument"       );
        impl_checkReadOnlyPropsOfModule("com.sun.star.chart.ChartDocument"              );

        // other modules
        impl_checkReadOnlyPropsOfModule("com.sun.star.sdb.FormDesign"       );
        impl_checkReadOnlyPropsOfModule("com.sun.star.sdb.TextReportDesign" );
        impl_checkReadOnlyPropsOfModule("com.sun.star.sdb.RelationDesign"   );
        impl_checkReadOnlyPropsOfModule("com.sun.star.sdb.QueryDesign"      );
        impl_checkReadOnlyPropsOfModule("com.sun.star.sdb.TableDesign"      );
        impl_checkReadOnlyPropsOfModule("com.sun.star.sdb.DataSourceBrowser");
        impl_checkReadOnlyPropsOfModule("com.sun.star.frame.Bibliography"   );
        impl_checkReadOnlyPropsOfModule("com.sun.star.script.BasicIDE"      );
        impl_checkReadOnlyPropsOfModule("com.sun.star.frame.StartModule"    );
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkModuleConfigurationQueries()
        throws java.lang.Exception
    {
        impl_searchModulesByDocumentService("com.sun.star.text.TextDocument"                );
        impl_searchModulesByDocumentService("com.sun.star.text.WebDocument"                 );
        impl_searchModulesByDocumentService("com.sun.star.text.GlobalDocument"              );
        impl_searchModulesByDocumentService("com.sun.star.formula.FormulaProperties"        );
        impl_searchModulesByDocumentService("com.sun.star.sheet.SpreadsheetDocument"        );
        impl_searchModulesByDocumentService("com.sun.star.drawing.DrawingDocument"          );
        impl_searchModulesByDocumentService("com.sun.star.presentation.PresentationDocument");
        impl_searchModulesByDocumentService("com.sun.star.sdb.OfficeDatabaseDocument"       );
        impl_searchModulesByDocumentService("com.sun.star.chart.ChartDocument"              );
    }

    //-------------------------------------------
    /** @todo document me
     */
    private void impl_searchModulesByDocumentService(String sDocumentService)
        throws java.lang.Exception
    {
        log.println("search modules matching document service '"+sDocumentService+"' ...");

        NamedValue[] lProps          = new NamedValue[1];
                     lProps[0]       = new NamedValue();
                     lProps[0].Name  = "ooSetupFactoryDocumentService";
                     lProps[0].Value = sDocumentService;

        XContainerQuery xMM     = (XContainerQuery)UnoRuntime.queryInterface(XContainerQuery.class, m_xMM);
        XEnumeration    xResult = xMM.createSubSetEnumerationByProperties(lProps);
        while(xResult.hasMoreElements())
        {
            PropertyValue[] lModuleProps      = (PropertyValue[])AnyConverter.toArray(xResult.nextElement());
            int             c                 = lModuleProps.length;
            int             i                 = 0;
            String          sFoundModule      = "";
            String          sFoundDocService  = "";
            for (i=0; i<c; ++i)
            {
                if (lModuleProps[i].Name.equals("ooSetupFactoryModuleIdentifier"))
                    sFoundModule = AnyConverter.toString(lModuleProps[i].Value);
                if (lModuleProps[i].Name.equals("ooSetupFactoryDocumentService"))
                    sFoundDocService = AnyConverter.toString(lModuleProps[i].Value);
            }

            if (sFoundModule.length() < 1)
                failed("Miss module identifier in result set. Returned data are incomplete.");

            if ( ! sFoundDocService.equals(sDocumentService))
                failed("Query returned wrong module '"+sFoundModule+"' with DocumentService='"+sFoundDocService+"'.");

            log.println("Found module '"+sFoundModule+"'.");
        }
    }

    //-------------------------------------------
    /** @todo document me
     */
    private void impl_identifyModulesBasedOnDocs(String sModule)
        throws java.lang.Exception
    {
        log.println("check identification of module '"+sModule+"' ...");

        XNameAccess     xMM          = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, m_xMM);
        PropertyValue[] lModuleProps = (PropertyValue[])AnyConverter.toArray(xMM.getByName(sModule));
        int             c            = lModuleProps.length;
        int             i            = 0;
        String          sFactoryURL  = "";

        for (i=0; i<c; ++i)
        {
            if (lModuleProps[i].Name.equals("ooSetupFactoryEmptyDocumentURL"))
            {
                sFactoryURL = AnyConverter.toString(lModuleProps[i].Value);
                break;
            }
        }

        PropertyValue[] lArgs             = new PropertyValue[1];
                        lArgs[0]          = new PropertyValue();
                        lArgs[0].Name     = "Hidden";
                        lArgs[0].Value    = Boolean.TRUE;

        XComponent      xModel            = m_xLoader.loadComponentFromURL(sFactoryURL, "_self", 0, lArgs);
        XFrame          xFrame            = (XFrame)UnoRuntime.queryInterface(XFrame.class, m_xLoader);
        XController     xController       = xFrame.getController();

        String          sModuleFrame      = m_xMM.identify(xFrame     );
        String          sModuleController = m_xMM.identify(xController);
        String          sModuleModel      = m_xMM.identify(xModel     );

        if ( ! sModuleFrame.equals(sModule))
            failed("Identification of module '"+sModule+"' failed if frame was used as entry point.");
        if ( ! sModuleController.equals(sModule))
            failed("Identification of module '"+sModule+"' failed if controller was used as entry point.");
        if ( ! sModuleModel.equals(sModule))
            failed("Identification of module '"+sModule+"' failed if model was used as entry point.");
    }

    //-------------------------------------------
    /** @todo document me
     */
    private void impl_checkReadOnlyPropsOfModule(String sModule)
        throws java.lang.Exception
    {
        XNameReplace xWrite = (XNameReplace)UnoRuntime.queryInterface(XNameReplace.class, m_xMM);

        impl_checkReadOnlyPropOfModule(xWrite, sModule, "ooSetupFactoryDocumentService"     , "test");
        impl_checkReadOnlyPropOfModule(xWrite, sModule, "ooSetupFactoryActualFilter"        , "test");
        impl_checkReadOnlyPropOfModule(xWrite, sModule, "ooSetupFactoryActualTemplateFilter", "test");
        impl_checkReadOnlyPropOfModule(xWrite, sModule, "ooSetupFactoryEmptyDocumentURL"    , "test");
    }

    //-------------------------------------------
    /** @todo document me
     */
    private void impl_checkReadOnlyPropOfModule(XNameReplace xMM        ,
                                                String       sModule    ,
                                                String       sPropName  ,
                                                Object       aPropValue )
        throws java.lang.Exception
    {
        PropertyValue[] lChanges          = new PropertyValue[1];
                        lChanges[0]       = new PropertyValue();
                        lChanges[0].Name  = sPropName;
                        lChanges[0].Value = aPropValue;

        // Note: Exception is expected !
        log.println("check readonly state of module '"+sModule+"' for property '"+sPropName+"' ...");
        try
        {
            xMM.replaceByName(sModule, lChanges);
            failed("Was able to write READONLY property '"+sPropName+"' of module '"+sModule+"' configuration.");
        }
        catch(Throwable ex)
            {}
    }
}
