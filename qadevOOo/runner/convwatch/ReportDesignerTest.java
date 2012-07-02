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

package convwatch;

import java.io.File;
import java.util.ArrayList;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.XReportDocumentsSupplier;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import com.sun.star.lang.XServiceInfo;

// import util.BasicMacroTools;
// import util.DesktopTools;
// import util.dbg;
import complexlib.ComplexTestCase;
import helper.OfficeProvider;
import helper.URLHelper;
import helper.OfficeWatcher;

// import convwatch.DB;

// import java.util.Date;
// import java.text.SimpleDateFormat;
// import java.text.ParsePosition;
// import java.sql.Time;
//
// import java.io.BufferedReader;
// import java.io.File;
// import java.io.FileReader;
// import java.io.IOException;
// import java.io.FilenameFilter;
//
// import java.util.Vector;
//
// import helper.AppProvider;
// import java.text.DecimalFormat;
// import util.DynamicClassLoader;
// import java.util.StringTokenizer;




class PropertySetHelper
{
    XPropertySet m_xPropertySet;
    public PropertySetHelper(Object _aObj)
        {
            m_xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, _aObj);
        }

    /**
       get a property and don't convert it
       @param _sName the string name of the property
       @return the object value of the property without any conversion
    */
    public Object getPropertyValueAsObject(String _sName)
        {
            Object aObject = null;

            if (m_xPropertySet != null)
            {
                try
                {
                    aObject = m_xPropertySet.getPropertyValue(_sName);
                }
                catch (com.sun.star.beans.UnknownPropertyException e)
                {
                    System.out.println("ERROR: UnknownPropertyException caught. '" + _sName + "'");
                    System.out.println("Message: " + e.getMessage());
                }
                catch (com.sun.star.lang.WrappedTargetException e)
                {
                    System.out.println("ERROR: WrappedTargetException caught.");
                    System.out.println("Message: " + e.getMessage());
                }
            }
            return aObject;
        }
}

class PropertyHelper
{
    /**
       Create a PropertyValue[] from a ArrayList
       @param _aPropertyList
       @return a PropertyValue[]
    */
    public static PropertyValue[] createPropertyValueArrayFormArrayList(ArrayList<PropertyValue> _aPropertyList)
        {
            // copy the whole PropertyValue List to an PropertyValue Array
            PropertyValue[] aSaveProperties = null;

            if (_aPropertyList == null)
            {
                aSaveProperties = new PropertyValue[0];
            }
            else
            {
                if (_aPropertyList.size() > 0)
                {
                    aSaveProperties = new PropertyValue[_aPropertyList.size()];
                    for (int i = 0;i<_aPropertyList.size(); i++)
                    {
                        aSaveProperties[i] = _aPropertyList.get(i);
                    }
                }
                else
                {
                    aSaveProperties = new PropertyValue[0];
                }
            }
            return aSaveProperties;
        }
}

public class ReportDesignerTest extends ComplexTestCase {

    String mTestDocumentPath;

    /**
     * This method returns a list of Strings, each string must be a function name in this class.
     * @return
     */
    public String[] getTestMethodNames()
        {
            return new String[] {"ReportDesignTest"}; // MAIN
        }

    private void checkIfOfficeExists(String _sOfficePathWithTrash)
        {
            String sOfficePath = "";
            int nIndex = _sOfficePathWithTrash.indexOf("soffice.exe");
            if (nIndex > 0)
            {
                sOfficePath = _sOfficePathWithTrash.substring(0, nIndex + 11);
            }
            else
            {
                nIndex = _sOfficePathWithTrash.indexOf("soffice");
                if (nIndex > 0)
                {
                    sOfficePath = _sOfficePathWithTrash.substring(0, nIndex + 7);
                }
            }

            // if (sOfficePath.startsWith("\"") ||
            //     sOfficePath.startsWith("'"))
            // {
            //     sOfficePath = sOfficePath.substring(1);
            // }
            sOfficePath = helper.StringHelper.removeQuoteIfExists(sOfficePath);

            log.println(sOfficePath);
            File sOffice = new File(sOfficePath);
            if (! sOffice.exists())
            {
                log.println("ERROR: There exists no office installation at given path: '" + sOfficePath + "'");
                System.exit(0);
            }
        }


    private static XDesktop m_xDesktop = null;
    public static XDesktop getXDesktop()
        {

            if (m_xDesktop == null)
            {
                try
                {
                    XInterface xInterface = (XInterface) m_xXMultiServiceFactory.createInstance( "com.sun.star.frame.Desktop" );
                    m_xDesktop = UnoRuntime.queryInterface(XDesktop.class, xInterface);
                }
                catch (com.sun.star.uno.Exception e)
                {
                    log.println("ERROR: uno.Exception caught");
                    log.println("Message: " + e.getMessage());
                }
            }
            return m_xDesktop;
        }

    private void showElements(XNameAccess _xNameAccess)
        {
            if (_xNameAccess != null)
            {
                String[] sElementNames = _xNameAccess.getElementNames();
                for(int i=0;i<sElementNames.length; i++)
                {
                    System.out.println("Value: [" + i + "] := " + sElementNames[i]);
                }
            }
            else
            {
                System.out.println("Warning: Given object is null.");
            }
        }


    private OfficeProvider m_aProvider = null;
    private static XMultiServiceFactory m_xXMultiServiceFactory = null;
    private void startOffice()
        {
            // int tempTime = param.getInt("SingleTimeOut");
            param.put("TimeOut", new Integer(300000));
            System.out.println("TimeOut: " + param.getInt("TimeOut"));
            System.out.println("ThreadTimeOut: " + param.getInt("ThreadTimeOut"));

            // OfficeProvider aProvider = null;
            m_aProvider = new OfficeProvider();
            m_xXMultiServiceFactory = (XMultiServiceFactory) m_aProvider.getManager(param);
            param.put("ServiceFactory", m_xXMultiServiceFactory);
        }

    private void stopOffice()
        {
            if (m_aProvider != null)
            {
                m_aProvider.closeExistingOffice(param, true);
                m_aProvider = null;
            }
            TimeHelper.waitInSeconds(2, "Give close Office some time.");
        }

    private String m_sMailAddress = null;
    private String m_sParentDistinct = null;

    // private String m_sUPDMinor;
    // private String m_sCWS_WORK_STAMP;

    private static String m_sSourceVersion;
    private static String m_sDestinationVersion;
    private static String m_sSourceName;
    private static String m_sDestinationName;

    /**
     * This is the main test Function of current ReportDesignerTest
     */
    public void ReportDesignTest()
        {
            convwatch.GlobalLogWriter.set(log);

            GlobalLogWriter.get().println("Set office watcher");
            OfficeWatcher aWatcher = (OfficeWatcher)param.get("Watcher");
            GlobalLogWriter.get().setWatcher(aWatcher);

            try
            {

                // -------------------- preconditions, try to find an office --------------------

                String sAppExecutionCommand = (String) param.get("AppExecutionCommand");
                log.println("sAppExecutionCommand='" + sAppExecutionCommand + "'");

                String sUser = System.getProperty("user.name");
                log.println("user.name='" + sUser + "'");

                // String sVCSID = System.getProperty("VCSID");
                // log.println("VCSID='" + sVCSID + "'");
                // m_sMailAddress = sVCSID + "@openoffice.org";
                m_sMailAddress = System.getProperty("MailAddress");
                log.println("Assumed mail address: " + m_sMailAddress);

                m_sParentDistinct = System.getProperty("ParentDistinct");

                m_sSourceVersion = System.getProperty("SourceVersion");
                m_sSourceName = System.getProperty("SourceName");
                m_sDestinationVersion = System.getProperty("DestinationVersion");
                m_sDestinationName = System.getProperty("DestinationName");
                // createDBEntry();
                // log.println("Current CWS: " + m_sCWS_WORK_STAMP);
                // log.println("Current MWS: " + m_sUPDMinor);

                if (m_sSourceVersion == null)
                {
                    System.out.println("Error, Sourceversion is null.");
                    System.exit(1);
                }

                sAppExecutionCommand = sAppExecutionCommand.replaceAll( "\\$\\{USERNAME\\}", sUser);
                log.println("sAppExecutionCommand='" + sAppExecutionCommand + "'");

                // an other way to replace strings
                // sAppExecutionCommand = utils.replaceAll13(sAppExecutionCommand, "${USERNAME}", sUser);

                checkIfOfficeExists(sAppExecutionCommand);
                param.put("AppExecutionCommand", new String(sAppExecutionCommand));

                // System.exit(1);

                // --------------------------- Start the given Office ---------------------------

                startOffice();

                // ------------------------------ Start a test run ------------------------------

                // String sCurrentDirectory = System.getProperty("user.dir");
                // log.println("Current Dir: " + sCurrentDirectory);
                String sDocument = (String) param.get(convwatch.PropertyName.DOC_COMPARATOR_INPUT_PATH);
                sDocument = helper.StringHelper.removeQuoteIfExists( sDocument );
                startTestForFile(sDocument);
                // if (sDocument.toLowerCase().indexOf("writer") >= 0)
                // {
                //     startTestForFile(sDocument, WRITER);
                // }
                // else if (sDocument.toLowerCase().indexOf("calc") >= 0)
                // {
                //     startTestForFile(sDocument, CALC);
                // }
                // else
                // {
                //     assure("Can't identify the document no 'writer' nor 'calc' in it's name given.", false);
                // }
            }
            catch (AssureException e)
            {
                stopOffice();
                throw new AssureException(e.getMessage());
            }

            // ------------------------------ Office shutdown ------------------------------
            stopOffice();
        }

// -----------------------------------------------------------------------------
    private void startTestForFile(String _sDocument /*, int _nType*/)
        {
            File aFile = new File(_sDocument);
            assure("Test File '" + _sDocument + "' doesn't exist.", aFile.exists());

            String sFileURL = URLHelper.getFileURLFromSystemPath(_sDocument);
            log.println("File URL: " + sFileURL);

            XComponent xDocComponent = loadComponent(sFileURL, getXDesktop(), null);
            log.println("Load done");
//  context = createUnoService("com.sun.star.sdb.DatabaseContext")
//     oDataBase = context.getByName("hh")
//     oDBDoc = oDataBase.DatabaseDocument
//
//  dim args(1) as new com.sun.star.beans.PropertyValue
//  args(0).Name = "ActiveConnection"
//  args(0).Value = oDBDoc.getCurrentController().getPropertyValue("ActiveConnection")
//  reportContainer = oDBDoc.getReportDocuments()
//     report = reportContainer.loadComponentFromURL("Report40","",0,args)

            try
            {
                XInterface x = (XInterface)m_xXMultiServiceFactory.createInstance("com.sun.star.sdb.DatabaseContext");
                assure("can't create instance of com.sun.star.sdb.DatabaseContext", x != null);
                log.println("createInstance com.sun.star.sdb.DatabaseContext done");

                XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, x);
                showElements(xNameAccess);
                Object aObj = xNameAccess.getByName(sFileURL);
//                    log.println("1");

                    // PropertySetHelper aHelper = new PropertySetHelper(aObj);
                XDocumentDataSource xDataSource = UnoRuntime.queryInterface(XDocumentDataSource.class, aObj);
//                    Object aDatabaseDocmuent = aHelper.getPropertyValueAsObject("DatabaseDocument");
                XOfficeDatabaseDocument xOfficeDBDoc = xDataSource.getDatabaseDocument();

                // XOfficeDatabaseDocument xOfficeDBDoc = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(XOfficeDatabaseDocument.class, aDatabaseDocument);
                assure("can't access DatabaseDocument", xOfficeDBDoc != null);
//                    log.println("2");

                XModel xDBSource = UnoRuntime.queryInterface(XModel.class, xOfficeDBDoc);
                Object aController = xDBSource.getCurrentController();
                assure("Controller of xOfficeDatabaseDocument is empty!", aController != null);
//                     log.println("3");

                XDatabaseDocumentUI aDBDocUI = UnoRuntime.queryInterface(XDatabaseDocumentUI.class, aController);
                aDBDocUI.connect();
//                     if (aDBDocUI.isConnected())
//                     {
//                         System.out.println("true");
//                     }
//                     else
//                     {
//                         System.out.println("false");
//                     }
//                     log.println("4");

                // aHelper = new PropertySetHelper(aController);

                // Object aActiveConnectionObj = aHelper.getPropertyValueAsObject("ActiveConnection");
                Object aActiveConnectionObj = aDBDocUI.getActiveConnection();
                assure("ActiveConnection is empty", aActiveConnectionObj != null);
//                     log.println("5");

                XReportDocumentsSupplier xSupplier = UnoRuntime.queryInterface(XReportDocumentsSupplier.class, xOfficeDBDoc);
                xNameAccess = xSupplier.getReportDocuments();
                assure("xOfficeDatabaseDocument returns no Report Document", xNameAccess != null);
//                     log.println("5");

                showElements(xNameAccess);

                ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();

                PropertyValue aActiveConnection = new PropertyValue();
                aActiveConnection.Name = "ActiveConnection";
                aActiveConnection.Value = aActiveConnectionObj;
                aPropertyList.add(aActiveConnection);

                loadAndStoreReports(xNameAccess, aPropertyList /*, _nType*/ );
                createDBEntry(/*_nType*/);
            }
            catch(com.sun.star.uno.Exception e)
            {
                log.println("ERROR: Exception caught");
                log.println("Message: " + e.getMessage());
            }

            // String mTestDocumentPath = (String) param.get("TestDocumentPath");
            // System.out.println("mTestDocumentPath: '" + mTestDocumentPath + "'");
            // // workaround for issue using deprecated "DOCPTH" prop
            // System.setProperty("DOCPTH", mTestDocumentPath);

            // Close the document
            closeComponent(xDocComponent);
        }

    private String getDocumentPoolName(/*int _nType*/)
        {
            return "AutogenReportDesignTest";
            // return getFileFormat(_nType);
        }

// -----------------------------------------------------------------------------
    private void createDBEntry(/*int _nType*/)
        {
            // try to connect the database
            String sDBConnection = (String)param.get( convwatch.PropertyName.DB_CONNECTION_STRING );
            log.println("DBConnection: " + sDBConnection);
            DB.init(sDBConnection);

            String sSourceVersion = m_sSourceVersion;
            // String sSourceVersion = sFixRefSubDirectory;
            String sSourceName = m_sSourceName;
            // String sSourceCreatorType = "fixref";
            String sSourceCreatorType = "";
            // if (sDestinationVersion.length() == 0)
            // {
            //     sDestinationVersion = m_sUPDMinor;
            // }
            String sDestinationName = m_sDestinationName;
            String sDestinationCreatorType = "";
            String sDocumentPoolDir = getOutputPath(/*_nType*/);
            String sDocumentPoolName = getDocumentPoolName(/*_nType*/);
            String sSpecial = "";

            DB.insertinto_documentcompare(sSourceVersion, sSourceName, sSourceCreatorType,
                                          m_sDestinationVersion, sDestinationName, sDestinationCreatorType,
                                          sDocumentPoolDir, sDocumentPoolName, m_sMailAddress,
                                          sSpecial, m_sParentDistinct);
            TimeHelper.waitInSeconds(1, "wait for DB.");
            // DB.test();
            // System.exit(1);
        }

    private void loadAndStoreReports(XNameAccess _xNameAccess, ArrayList<PropertyValue> _aPropertyList /*, int _nType*/ )
        {
            if (_xNameAccess != null)
            {
                String[] sElementNames = _xNameAccess.getElementNames();
                for(int i=0;i<sElementNames.length; i++)
                {
                    String sReportName = sElementNames[i];
                    XComponent xDoc = loadComponent(sReportName, _xNameAccess, _aPropertyList);
                    // print? or store?
                    storeComponent(sReportName, xDoc /*, _nType*/);
                    closeComponent(xDoc);
                }
            }
        }

    private String getFormatExtension(Object _xComponent /* int _nType*/ )
         {
             String sExtension;
             XServiceInfo xServiceInfo = UnoRuntime.queryInterface( XServiceInfo.class, _xComponent );
             if ( xServiceInfo.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
             {
                 // calc
                 sExtension = ".ods";
             }
             else if (xServiceInfo.supportsService("com.sun.star.text.TextDocument"))
             {
                 //writer
                 sExtension = ".odt";
             }
             else
             {
                 sExtension = ".UNKNOWN";
             }
             return sExtension;
         }

    //         switch(_nType)
    //         {
    //         case WRITER:
    //             sExtension = ".odt";
    //             break;
    //         case CALC:
    //             sExtension = ".ods";
    //             break;
    //         default:
    //             sExtension = ".UNKNOWN";
    //         }
    //         return sExtension;
    //     }
    // private String getFileFormat(int _nType)
    //     {
    //         String sFileType;
    //         switch(_nType)
    //         {
    //         case WRITER:
    //             sFileType = "writer8";
    //             break;
    //         case CALC:
    //             sFileType = "calc8";
    //             break;
    //         default:
    //             sFileType = "UNKNOWN";
    //         }
    //         return sFileType;
    //     }

    private String m_sOutputPath = null;

    private String getOutputPath(/*int _nType*/)
        {
            if (m_sOutputPath == null)
            {
                String sOutputPath = (String)param.get( convwatch.PropertyName.DOC_COMPARATOR_OUTPUT_PATH );
                sOutputPath = helper.StringHelper.removeQuoteIfExists(sOutputPath);

                if (!sOutputPath.endsWith("/") ||         // construct the output file name
                    !sOutputPath.endsWith("\\"))
                {
                    sOutputPath += System.getProperty("file.separator");
                }
                // sOutputPath += "tmp_123";
                sOutputPath += DateHelper.getDateTimeForFilename();
                sOutputPath += System.getProperty("file.separator");

                // sOutputPath += getFileFormat(_nType);
                // sOutputPath += System.getProperty("file.separator");

                File aOutputFile = new File(sOutputPath); // create the directory of the given output path
                aOutputFile.mkdirs();
                m_sOutputPath = sOutputPath;
            }
            return m_sOutputPath;
        }

    /*
      store given _xComponent under the given Name in DOC_COMPARATOR_INPUTPATH
     */
    private void storeComponent(String _sName, Object _xComponent /*, int _nType*/ )
        {
            String sOutputPath = getOutputPath(/*_nType*/);

            // add DocumentPoolName
            sOutputPath += getDocumentPoolName(/*_nType*/);
            sOutputPath += System.getProperty("file.separator");

            File aOutputFile = new File(sOutputPath); // create the directory of the given output path
            aOutputFile.mkdirs();

            sOutputPath += _sName;
            sOutputPath += getFormatExtension(_xComponent /*_nType*/);

            String sOutputURL = URLHelper.getFileURLFromSystemPath(sOutputPath);

            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>(); // set some properties for storeAsURL

            // PropertyValue aFileFormat = new PropertyValue();
            // aFileFormat.Name = "FilterName";
            // aFileFormat.Value = getFileFormat(_nType);
            // aPropertyList.add(aFileFormat);

            PropertyValue aOverwrite = new PropertyValue(); // always overwrite already exist files
            aOverwrite.Name = "Overwrite";
            aOverwrite.Value = Boolean.TRUE;
            aPropertyList.add(aOverwrite);

            // store the document in an other directory
            XStorable aStorable = UnoRuntime.queryInterface( XStorable.class, _xComponent);
            if (aStorable != null)
            {
                log.println("store document as URL: '" + sOutputURL + "'");
                try
                {
                    aStorable.storeAsURL(sOutputURL, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
                }
                catch (com.sun.star.io.IOException e)
                {
                    log.println("ERROR: Exception caught");
                    log.println("Can't write document URL: '" + sOutputURL + "'");
                    log.println("Message: " + e.getMessage());
                }
            }
        }

    private XComponent loadComponent(String _sName, Object _xComponent, ArrayList<PropertyValue> _aPropertyList)
        {
            XComponent xDocComponent = null;
            XComponentLoader xComponentLoader = UnoRuntime.queryInterface( XComponentLoader.class, _xComponent );

            try
            {
                PropertyValue[] aLoadProperties = PropertyHelper.createPropertyValueArrayFormArrayList(_aPropertyList);
                log.println("Load component: '" + _sName + "'");
                xDocComponent = xComponentLoader.loadComponentFromURL(_sName, "_blank", 0, aLoadProperties);
                log.println("Load component: '" + _sName + "' done");
            }
            catch (com.sun.star.io.IOException e)
            {
                log.println("ERROR: Exception caught");
                log.println("Can't load document '" + _sName + "'");
                log.println("Message: " + e.getMessage());
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                log.println("ERROR: Exception caught");
                log.println("Illegal Arguments given to loadComponentFromURL.");
                log.println("Message: " + e.getMessage());
            }
            return xDocComponent;
        }

    private void closeComponent(XComponent _xDoc)
        {
            // Close the document
            XCloseable xCloseable = UnoRuntime.queryInterface(XCloseable.class, _xDoc);
            try
            {
                xCloseable.close(true);
            }
            catch (com.sun.star.util.CloseVetoException e)
            {
                log.println("ERROR: CloseVetoException caught");
                log.println("CloseVetoException occurred Can't close document.");
                log.println("Message: " + e.getMessage());
            }
        }

}
