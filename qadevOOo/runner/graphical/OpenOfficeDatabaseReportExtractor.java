/*
 * ************************************************************************
 *
 *  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright 2008 by Sun Microsystems, Inc.
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OpenOfficeDatabaseReportExtractor.java,v $
 *  $Revision: 1.1.2.3 $
 *
 *  This file is part of OpenOffice.org.
 *
 *  OpenOffice.org is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version 3
 *  only, as published by the Free Software Foundation.
 *
 *  OpenOffice.org is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License version 3 for more details
 *  (a copy is included in the LICENSE file that accompanied this code).
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  version 3 along with OpenOffice.org.  If not, see
 *  <http://www.openoffice.org/license.html>
 *  for a copy of the LGPLv3 License.
 *
 * ***********************************************************************
 */

package graphical;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.XReportDocumentsSupplier;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import complexlib.Assurance;
// import complexlib.ComplexTestCase.AssureException;
import helper.OfficeProvider;
import helper.PropertyHelper;
import helper.URLHelper;
import java.io.File;
import java.util.ArrayList;

/**
 *
 * @author ll93751
 */

class PropertySetHelper
{
    XPropertySet m_xPropertySet;
    public PropertySetHelper(Object _aObj)
        {
            m_xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, _aObj);
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

public class OpenOfficeDatabaseReportExtractor extends Assurance
{
    private ParameterHelper m_aParameterHelper;

    public OpenOfficeDatabaseReportExtractor(ParameterHelper _aParameter)
    {
        m_aParameterHelper = _aParameter;
    }

    private XDesktop m_xDesktop = null;
    private XDesktop getXDesktop()
        {

            if (m_xDesktop == null)
            {
                try
                {
                    XInterface xInterface = (XInterface) getMultiServiceFactory().createInstance( "com.sun.star.frame.Desktop" );
                    m_xDesktop = (XDesktop) UnoRuntime.queryInterface(XDesktop.class, xInterface);
                }
                catch (com.sun.star.uno.Exception e)
                {
                    GlobalLogWriter.get().println("ERROR: uno.Exception caught");
                    GlobalLogWriter.get().println("Message: " + e.getMessage());
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
    private XMultiServiceFactory m_xMultiServiceFactory = null;
    private XMultiServiceFactory getMultiServiceFactory()
    {
        if (m_xMultiServiceFactory == null)
        {
            m_xMultiServiceFactory = (XMultiServiceFactory)m_aParameterHelper.getMultiServiceFactory();
        }
        return m_xMultiServiceFactory;
    }
//    private void startOffice()
//        {
//            // int tempTime = m_aParameterHelper.getTestParameters().getInt("SingleTimeOut");
//            param.put("TimeOut", new Integer(300000));
//            System.out.println("TimeOut: " + param.getInt("TimeOut"));
//            System.out.println("ThreadTimeOut: " + param.getInt("ThreadTimeOut"));
//
//            // OfficeProvider aProvider = null;
//            m_aProvider = new OfficeProvider();
//            m_xXMultiServiceFactory = (XMultiServiceFactory) m_aProvider.getManager(param);
//            param.put("ServiceFactory", m_xXMultiServiceFactory);
//        }
//
//    private void stopOffice()
//        {
//            if (m_aProvider != null)
//            {
//                m_aProvider.closeExistingOffice(param, true);
//                m_aProvider = null;
//            }
//            TimeHelper.waitInSeconds(2, "Give close Office some time.");
//        }

    private String m_sMailAddress = null;
    private String m_sParentDistinct = null;

    // private String m_sUPDMinor;
    // private String m_sCWS_WORK_STAMP;

    private static String m_sSourceVersion;
    private static String m_sDestinationVersion;
    private static String m_sSourceName;
    private static String m_sDestinationName;

    private static final int WRITER = 1;
    private static final int CALC = 2;

    /**
     * This is the main test Function of current ReportDesignerTest
     */
//    public void load(String _sDocumentName)
//        {
//            // convwatch.GlobalLogWriter.set(log);
//
//            // GlobalLogWriter.get().println("Set office watcher");
//            // OfficeWatcher aWatcher = (OfficeWatcher)m_aParameterHelper.getTestParameters().get("Watcher");
//            // GlobalLogWriter.get().setWatcher(aWatcher);
//
//            try
//            {
//
//                // -------------------- preconditions, try to find an office --------------------
//
////                String sAppExecutionCommand = (String) m_aParameterHelper.getTestParameters().get("AppExecutionCommand");
////                GlobalLogWriter.get().println("sAppExecutionCommand='" + sAppExecutionCommand + "'");
////
////                String sUser = System.getProperty("user.name");
////                GlobalLogWriter.get().println("user.name='" + sUser + "'");
////
////                // String sVCSID = System.getProperty("VCSID");
////                // GlobalLogWriter.get().println("VCSID='" + sVCSID + "'");
////                // m_sMailAddress = sVCSID + "@openoffice.org";
////                m_sMailAddress = System.getProperty("MailAddress");
////                GlobalLogWriter.get().println("Assumed mail address: " + m_sMailAddress);
////
////                m_sParentDistinct = System.getProperty("ParentDistinct");
////
////                m_sSourceVersion = System.getProperty("SourceVersion");
////                m_sSourceName = System.getProperty("SourceName");
////                m_sDestinationVersion = System.getProperty("DestinationVersion");
////                m_sDestinationName = System.getProperty("DestinationName");
////                // createDBEntry();
////                // GlobalLogWriter.get().println("Current CWS: " + m_sCWS_WORK_STAMP);
////                // GlobalLogWriter.get().println("Current MWS: " + m_sUPDMinor);
////
////                if (m_sSourceVersion == null)
////                {
////                    System.out.println("Error, Sourceversion is null.");
////                    System.exit(1);
////                }
////
////                sAppExecutionCommand = sAppExecutionCommand.replaceAll( "\\$\\{USERNAME\\}", sUser);
////                GlobalLogWriter.get().println("sAppExecutionCommand='" + sAppExecutionCommand + "'");
//
//                // an other way to replace strings
//                // sAppExecutionCommand = utils.replaceAll13(sAppExecutionCommand, "${USERNAME}", sUser);
//
//                // checkIfOfficeExists(sAppExecutionCommand);
//                // param.put("AppExecutionCommand", new String(sAppExecutionCommand));
//
//                // System.exit(1);
//
//                // --------------------------- Start the given Office ---------------------------
//
//                // startOffice();
//
//                // ------------------------------ Start a test run ------------------------------
//
//                // String sCurrentDirectory = System.getProperty("user.dir");
//                // GlobalLogWriter.get().println("Current Dir: " + sCurrentDirectory);
////                 String sDocument = (String) m_aParameterHelper.getTestParameters().get(convwatch.PropertyName.DOC_COMPARATOR_INPUT_PATH);
////                sDocument = helper.StringHelper.removeQuoteIfExists( sDocument );
//                startTestForFile(_sDocumentName);
//                // if (sDocument.toLowerCase().indexOf("writer") >= 0)
//                // {
//                //     startTestForFile(sDocument, WRITER);
//                // }
//                // else if (sDocument.toLowerCase().indexOf("calc") >= 0)
//                // {
//                //     startTestForFile(sDocument, CALC);
//                // }
//                // else
//                // {
//                //     assure("Can't identify the document no 'writer' nor 'calc' in it's name given.", false);
//                // }
//            }
//            catch (AssureException e)
//            {
//                // stopOffice();
//                // throw new AssureException(e.getMessage());
//            }
//
//            // ------------------------------ Office shutdown ------------------------------
//            // stopOffice();
//        }

// -----------------------------------------------------------------------------
    public ArrayList load(String _sDocument /*, int _nType*/)
        {
            // We need to copy the database file to a place where we have write access, NEVER use the docpool for this
            String sOutputPath = m_aParameterHelper.getOutputPath();
            File aOutputPath = new File(sOutputPath);
            aOutputPath.mkdirs();

            String sFilename = FileHelper.getBasename(_sDocument);
            String sDestinationFile = FileHelper.appendPath(sOutputPath, sFilename);
            FileHelper.copy(_sDocument, sDestinationFile);

            // now the fix reference of the AbsoluteReferenceFile should exist.
            assure("There exists no file: " + sDestinationFile, FileHelper.exists(sDestinationFile));

            String sFileURL = URLHelper.getFileURLFromSystemPath(sDestinationFile);
            GlobalLogWriter.get().println("File URL: " + sFileURL);

            ArrayList aPropertyList = new ArrayList();

            // FYI: it is not allowed to open the document read only
//            PropertyValue aReadOnly = new PropertyValue(); // always overwrite already exist files
//            aReadOnly.Name = "ReadOnly";
//            aReadOnly.Value = Boolean.TRUE;
//            aPropertyList.add(aReadOnly);

            XComponent xDocComponent = loadComponent(sFileURL, getXDesktop(), aPropertyList);

            GlobalLogWriter.get().println("Load done");
//  context = createUnoService("com.sun.star.sdb.DatabaseContext")
//     oDataBase = context.getByName("hh")
//     oDBDoc = oDataBase.DatabaseDocument
//
//  dim args(1) as new com.sun.star.beans.PropertyValue
//  args(0).Name = "ActiveConnection"
//  args(0).Value = oDBDoc.getCurrentController().getPropertyValue("ActiveConnection")
//  reportContainer = oDBDoc.getReportDocuments()
//     report = reportContainer.loadComponentFromURL("Report40","",0,args)

            ArrayList aList = null;
            try
            {
//                XInterface x = (XInterface)getMultiServiceFactory().createInstance("com.sun.star.sdb.DatabaseContext");
//                assure("can't create instance of com.sun.star.sdb.DatabaseContext", x != null);
//                GlobalLogWriter.get().println("createInstance com.sun.star.sdb.DatabaseContext done");

//                XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, x);
//                showElements(xNameAccess);
//                Object aObj = xNameAccess.getByName(sFileURL);
//                    GlobalLogWriter.get().println("1");

//                PropertySetHelper aHelper = new PropertySetHelper(aObj);
//                XDocumentDataSource xDataSource = (XDocumentDataSource)UnoRuntime.queryInterface(XDocumentDataSource.class, aObj);
//                Object aDatabaseDocument = aHelper.getPropertyValueAsObject("DatabaseDocument");
//                XOfficeDatabaseDocument xOfficeDBDoc = xDataSource.getDatabaseDocument();

                XOfficeDatabaseDocument xOfficeDBDoc = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(XOfficeDatabaseDocument.class, xDocComponent);

//                XOfficeDatabaseDocument xOfficeDBDoc = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(XOfficeDatabaseDocument.class, xDataSource);
                assure("can't access DatabaseDocument", xOfficeDBDoc != null);
//                GlobalLogWriter.get().println("2");

                XModel xDBSource = (XModel)UnoRuntime.queryInterface(XModel.class, xOfficeDBDoc);
                Object aController = xDBSource.getCurrentController();
                assure("Controller of xOfficeDatabaseDocument is empty!", aController != null);
//                GlobalLogWriter.get().println("3");

                XDatabaseDocumentUI aDBDocUI = (XDatabaseDocumentUI)UnoRuntime.queryInterface(XDatabaseDocumentUI.class, aController);
                aDBDocUI.connect();
                boolean isConnect = aDBDocUI.isConnected();
                if (isConnect)
                {
                    GlobalLogWriter.println("Connection is true");
                }
                else
                {
                    GlobalLogWriter.println("Connection is false");
                }

                // aHelper = new PropertySetHelper(aController);

                XReportDocumentsSupplier xSupplier = (XReportDocumentsSupplier)UnoRuntime.queryInterface(XReportDocumentsSupplier.class, xOfficeDBDoc);
                XNameAccess xNameAccess = xSupplier.getReportDocuments();
                assure("xOfficeDatabaseDocument returns no Report Document", xNameAccess != null);
//                     GlobalLogWriter.get().println("5");

                showElements(xNameAccess);

                // Object aActiveConnectionObj = aHelper.getPropertyValueAsObject("ActiveConnection");
                Object aActiveConnectionObj = aDBDocUI.getActiveConnection();
                assure("ActiveConnection is empty", aActiveConnectionObj != null);
//                     GlobalLogWriter.get().println("5");

                ArrayList aPropertyList2 = new ArrayList();

                PropertyValue aActiveConnection = new PropertyValue();
                aActiveConnection.Name = "ActiveConnection";
                aActiveConnection.Value = aActiveConnectionObj;
                aPropertyList2.add(aActiveConnection);

                aList = loadAndStoreReports(xNameAccess, aPropertyList2 /*, _nType*/ );
                createDBEntry(/*_nType*/);
            }
            catch(Exception/*com.sun.star.uno.Exception*/ e)
            {
                GlobalLogWriter.get().println("ERROR: Exception caught");
                GlobalLogWriter.get().println("Message: " + e.getMessage());
            }

            // String mTestDocumentPath = (String) param.get("TestDocumentPath");
            // System.out.println("mTestDocumentPath: '" + mTestDocumentPath + "'");
            // // workaround for issue using deprecated "DOCPTH" prop
            // System.setProperty("DOCPTH", mTestDocumentPath);

            // Close the document
            // util.utils.shortWait(2000);

            closeComponent(xDocComponent);
            return aList;
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
            String sDBConnection = (String)m_aParameterHelper.getTestParameters().get( convwatch.PropertyName.DB_CONNECTION_STRING );
            if (sDBConnection != null && sDBConnection.length() > 0)
            {
                GlobalLogWriter.get().println("DBConnection: " + sDBConnection);
// TODO: DB
//                DB.init(sDBConnection);

                // String sFixRefSubDirectory = "ReportDesign_qa_complex_" + getFileFormat(_nType);
                String sFixRefSubDirectory = "ReportDesignFixRef";

                String sSourceVersion = m_sSourceVersion;
                // String sSourceVersion = sFixRefSubDirectory;
                String sSourceName = m_sSourceName;
                // String sSourceCreatorType = "fixref";
                String sSourceCreatorType = "";
                String sDestinationVersion = m_sDestinationVersion;
                // if (sDestinationVersion.length() == 0)
                // {
                //     sDestinationVersion = m_sUPDMinor;
                // }
                String sDestinationName = m_sDestinationName;
                String sDestinationCreatorType = "";
                String sDocumentPoolDir = getOutputPath(/*_nType*/);
                String sDocumentPoolName = getDocumentPoolName(/*_nType*/);
                String sSpecial = "";

// TODO: DB
//                DB.insertinto_documentcompare(sSourceVersion, sSourceName, sSourceCreatorType,
//                                              m_sDestinationVersion, sDestinationName, sDestinationCreatorType,
//                                              sDocumentPoolDir, sDocumentPoolName, m_sMailAddress,
//                                              sSpecial, m_sParentDistinct);
                TimeHelper.waitInSeconds(1, "wait for DB.");
                // DB.test();
                // System.exit(1);
            }
        }

    private ArrayList loadAndStoreReports(XNameAccess _xNameAccess, ArrayList _aPropertyList /*, int _nType*/ )
        {
            ArrayList aList = new ArrayList();
            if (_xNameAccess != null)
            {
                String[] sElementNames = _xNameAccess.getElementNames();
                for(int i=0;i<sElementNames.length; i++)
                {
                    String sReportName = sElementNames[i];
                    XComponent xDoc = loadComponent(sReportName, _xNameAccess, _aPropertyList);
//                     util.utils.shortWait(1000);
                    // print? or store?
                    String sDocumentPathName = storeComponent(sReportName, xDoc /*, _nType*/);
                    aList.add(sDocumentPathName);
//                    util.utils.shortWait(1000);
                    closeComponent(xDoc);
//                    util.utils.shortWait(1000);
                    // sBackPath contains the path where to find the extracted ODB Document
                }
            }
            return aList;
        }

    private String getFormatExtension(Object _xComponent /* int _nType*/ )
         {
             String sExtension;
             XServiceInfo xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface( XServiceInfo.class, _xComponent );
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
                String sOutputPath = (String)m_aParameterHelper.getTestParameters().get( convwatch.PropertyName.DOC_COMPARATOR_OUTPUT_PATH );
                sOutputPath = helper.StringHelper.removeQuoteIfExists(sOutputPath);

                // sOutputPath += "tmp_123";
                sOutputPath = FileHelper.appendPath(sOutputPath, DateHelper.getDateTimeForFilename());
                // sOutputPath += System.getProperty("file.separator");

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
    private String storeComponent(String _sName, Object _xComponent /*, int _nType*/ )
        {
            String sOutputPath = getOutputPath(/*_nType*/);

//            // add DocumentPoolName
//            sOutputPath = FileHelper.appendPath(sOutputPath, getDocumentPoolName(/*_nType*/));
//            // sOutputPath += System.getProperty("file.separator");
//
//            File aOutputFile = new File(sOutputPath); // create the directory of the given output path
//            aOutputFile.mkdirs();


            String sName = _sName + getFormatExtension(_xComponent /*_nType*/);
            sOutputPath = FileHelper.appendPath(sOutputPath, sName);

            // we need the name and path
            String sBackPathName = sOutputPath;

            String sOutputURL = URLHelper.getFileURLFromSystemPath(sOutputPath);

            ArrayList aPropertyList = new ArrayList(); // set some properties for storeAsURL

            // PropertyValue aFileFormat = new PropertyValue();
            // aFileFormat.Name = "FilterName";
            // aFileFormat.Value = getFileFormat(_nType);
            // aPropertyList.add(aFileFormat);

            PropertyValue aOverwrite = new PropertyValue(); // always overwrite already exist files
            aOverwrite.Name = "Overwrite";
            aOverwrite.Value = Boolean.TRUE;
            aPropertyList.add(aOverwrite);

            // store the document in an other directory
            XStorable aStorable = (XStorable) UnoRuntime.queryInterface( XStorable.class, _xComponent);
            if (aStorable != null)
            {
                GlobalLogWriter.get().println("store document as URL: '" + sOutputURL + "'");
                try
                {
                    aStorable.storeAsURL(sOutputURL, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
                }
                catch (com.sun.star.io.IOException e)
                {
                    GlobalLogWriter.get().println("ERROR: Exception caught");
                    GlobalLogWriter.get().println("Can't write document URL: '" + sOutputURL + "'");
                    GlobalLogWriter.get().println("Message: " + e.getMessage());
                }
            }
            return sBackPathName;
        }

    private XComponent loadComponent(String _sName, Object _xComponent, ArrayList _aPropertyList)
        {
            XComponent xDocComponent = null;
            XComponentLoader xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface( XComponentLoader.class, _xComponent );

            try
            {
                PropertyValue[] aLoadProperties = PropertyHelper.createPropertyValueArrayFormArrayList(_aPropertyList);
                GlobalLogWriter.get().println("Load component: '" + _sName + "'");
                xDocComponent = xComponentLoader.loadComponentFromURL(_sName, "_blank", FrameSearchFlag.ALL, aLoadProperties);
                GlobalLogWriter.get().println("Load component: '" + _sName + "' done");
            }
            catch (com.sun.star.io.IOException e)
            {
                GlobalLogWriter.get().println("ERROR: Exception caught");
                GlobalLogWriter.get().println("Can't load document '" + _sName + "'");
                GlobalLogWriter.get().println("Message: " + e.getMessage());
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                GlobalLogWriter.get().println("ERROR: Exception caught");
                GlobalLogWriter.get().println("Illegal Arguments given to loadComponentFromURL.");
                GlobalLogWriter.get().println("Message: " + e.getMessage());
            }
            return xDocComponent;
        }

    private void closeComponent(XComponent _xDoc)
        {
            // Close the document
            XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, _xDoc);
            try
            {
                xCloseable.close(true);
            }
            catch (com.sun.star.util.CloseVetoException e)
            {
                GlobalLogWriter.get().println("ERROR: CloseVetoException caught");
                GlobalLogWriter.get().println("CloseVetoException occured Can't close document.");
                GlobalLogWriter.get().println("Message: " + e.getMessage());
            }
        }

}
