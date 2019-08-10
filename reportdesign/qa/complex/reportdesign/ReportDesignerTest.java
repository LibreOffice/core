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

package complex.reportdesign;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import helper.URLHelper;

import java.io.File;
import java.util.ArrayList;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.XReportDocumentsSupplier;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;

import convwatch.DB;

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
       Create a PropertyValue[] from an ArrayList
    */
    public static PropertyValue[] createPropertyValueArrayFormArrayList(ArrayList<PropertyValue> _aPropertyList)
        {
            // copy the whole PropertyValue List to a PropertyValue Array
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

public class ReportDesignerTest
{

    String mTestDocumentPath;

    @Before public void before()
    {
        System.out.println("before");
    }

    @After public void after()
    {
        System.out.println("after");
    }

    private XDesktop m_xDesktop = null;
    public XDesktop getXDesktop() throws com.sun.star.uno.Exception
        {

            if (m_xDesktop == null)
            {
                    XInterface xInterface = (XInterface) getMSF().createInstance( "com.sun.star.frame.Desktop" );
                    m_xDesktop = UnoRuntime.queryInterface(XDesktop.class, xInterface);
                    assertNotNull("Can't get XDesktop", m_xDesktop);
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


    private static final int WRITER = 1;
    private static final int CALC = 2;

    @Test public void firsttest() throws Exception
        {
            // -------------------- preconditions, try to find an office --------------------

            String sUser = System.getProperty("user.name");
            System.out.println("user.name='" + sUser + "'");

            String sVCSID = System.getProperty("VCSID");
            System.out.println("VCSID='" + sVCSID + "'");
            String sMailAddress = sVCSID + "@openoffice.org";
            System.out.println("Assumed mail address: " + sMailAddress);

            String sUPDMinor = System.getProperty("UPDMINOR");
            System.out.println("Current MWS: " + sUPDMinor);

            // --------------------------- Start the given Office ---------------------------

            // ------------------------------ Start a test run ------------------------------

            String sWriterDocument =  TestDocument.getUrl("RPTWriterTests.odb");
            startTestForFile(sWriterDocument, WRITER);

            String sCalcDocument =  TestDocument.getUrl("RPTCalcTests.odb");
            startTestForFile(sCalcDocument, CALC);

            // ------------------------------ Office shutdown ------------------------------
        }


    private void startTestForFile(String _sDocument, int _nType) throws Exception
        {
            FileURL aFileURL = new FileURL(_sDocument);
            assertTrue("Test File doesn't '" + _sDocument + "'exist.", aFileURL.exists());

            String sFileURL = _sDocument;
            System.out.println("File URL: " + sFileURL);

            XComponent xDocComponent = loadComponent(sFileURL, getXDesktop(), null);
            System.out.println("Load done");
            assertNotNull("Can't load document ", xDocComponent);


            XInterface x = (XInterface)getMSF().createInstance("com.sun.star.sdb.DatabaseContext");
            assertNotNull("can't create instance of com.sun.star.sdb.DatabaseContext", x);
            System.out.println("createInstance com.sun.star.sdb.DatabaseContext done");

            XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, x);
            showElements(xNameAccess);
            Object aObj = xNameAccess.getByName(sFileURL);

            XDocumentDataSource xDataSource = UnoRuntime.queryInterface(XDocumentDataSource.class, aObj);
            XOfficeDatabaseDocument xOfficeDBDoc = xDataSource.getDatabaseDocument();

            assertNotNull("can't access DatabaseDocument", xOfficeDBDoc);

            XModel xDBSource = UnoRuntime.queryInterface(XModel.class, xOfficeDBDoc);
            Object aController = xDBSource.getCurrentController();
            assertNotNull("Controller of xOfficeDatabaseDocument is empty!", aController);

            XDatabaseDocumentUI aDBDocUI = UnoRuntime.queryInterface(XDatabaseDocumentUI.class, aController);
            /* boolean isConnect = */
            // TODO: throws an exception in DEV300m78
            aDBDocUI.connect();
            Object aActiveConnectionObj = aDBDocUI.getActiveConnection();
            assertNotNull("ActiveConnection is empty", aActiveConnectionObj);

            XReportDocumentsSupplier xSupplier = UnoRuntime.queryInterface(XReportDocumentsSupplier.class, xOfficeDBDoc);
            xNameAccess = xSupplier.getReportDocuments();
            assertNotNull("xOfficeDatabaseDocument returns no Report Document", xNameAccess);

            showElements(xNameAccess);

            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();

            PropertyValue aActiveConnection = new PropertyValue();
            aActiveConnection.Name = "ActiveConnection";
            aActiveConnection.Value = aActiveConnectionObj;
            aPropertyList.add(aActiveConnection);

            loadAndStoreReports(xNameAccess, aPropertyList, _nType);
            createDBEntry();

            // Close the document
            closeComponent(xDocComponent);
        }

    private String getDocumentPoolName(int _nType)
        {
            return getFileFormat(_nType);
        }


    private void createDBEntry()
        {
            // try to connect the database
            String sDBConnection = ""; // (String)param.get( convwatch.PropertyName.DB_CONNECTION_STRING );
            System.out.println("DBConnection: " + sDBConnection);
            DB.init(sDBConnection);
//            String sDestinationVersion = m_sUPDMinor;
//            String sDestinationName = "";
//            String sDestinationCreatorType = "";
//            String sDocumentPoolDir = getOutputPath(_nType);
//            String sDocumentPoolName = getDocumentPoolName(_nType);
//            String sSpecial = "";
//            String sFixRefSubDirectory = "ReportDesign_qa_complex_" + getFileFormat(_nType);
//            DB.insertinto_documentcompare(sFixRefSubDirectory, "", "fixref",
//                                          sDestinationVersion, sDestinationName, sDestinationCreatorType,
//                                          sDocumentPoolDir, sDocumentPoolName, m_sMailAddress,
//                                          sSpecial);
        }

    private void loadAndStoreReports(XNameAccess _xNameAccess, ArrayList<PropertyValue> _aPropertyList, int _nType) throws Exception
        {
            if (_xNameAccess != null)
            {
                String[] sElementNames = _xNameAccess.getElementNames();
                for(int i=0;i<sElementNames.length; i++)
                {
                    String sReportName = sElementNames[i];
                    XComponent xDoc = loadComponent(sReportName, _xNameAccess, _aPropertyList);
                    // print? or store?
                    storeComponent(sReportName, xDoc, _nType);
                    closeComponent(xDoc);
                }
            }
        }

    private String getFormatExtension(int _nType)
        {
            String sExtension;
            switch(_nType)
            {
            case WRITER:
                sExtension = ".odt";
                break;
            case CALC:
                sExtension = ".ods";
                break;
            default:
                sExtension = ".UNKNOWN";
            }
            return sExtension;
        }
    private String getFileFormat(int _nType)
        {
            String sFileType;
            switch(_nType)
            {
            case WRITER:
                sFileType = "writer8";
                break;
            case CALC:
                sFileType = "calc8";
                break;
            default:
                sFileType = "UNKNOWN";
            }
            return sFileType;
        }

    private String getOutputPath()
        {
            String sOutputPath = util.utils.getOfficeTemp/*Dir*/(getMSF());// (String)param.get( convwatch.PropertyName.DOC_COMPARATOR_OUTPUT_PATH );

            if (!sOutputPath.endsWith("/") ||         // construct the output file name
                !sOutputPath.endsWith("\\"))
            {
                sOutputPath += System.getProperty("file.separator");
            }
            sOutputPath += "tmp_123";
            sOutputPath += System.getProperty("file.separator");

            File aOutputFile = new File(sOutputPath); // create the directory of the given output path
            aOutputFile.mkdirs();

            return sOutputPath;
        }

    /*
      store given _xComponent under the given Name in DOC_COMPARATOR_INPUTPATH
     */
    private void storeComponent(String _sName, Object _xComponent, int _nType) throws Exception
        {
            String sOutputPath = getOutputPath();

            // add DocumentPoolName
            sOutputPath += getDocumentPoolName(_nType);
            sOutputPath += System.getProperty("file.separator");

            File aOutputFile = new File(sOutputPath); // create the directory of the given output path
            aOutputFile.mkdirs();

            sOutputPath += _sName;
            sOutputPath += getFormatExtension(_nType);

            String sOutputURL = URLHelper.getFileURLFromSystemPath(sOutputPath);

            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>(); // set some properties for storeAsURL

            PropertyValue aFileFormat = new PropertyValue();
            aFileFormat.Name = "FilterName";
            aFileFormat.Value = getFileFormat(_nType);
            aPropertyList.add(aFileFormat);

            PropertyValue aOverwrite = new PropertyValue(); // always overwrite already exist files
            aOverwrite.Name = "Overwrite";
            aOverwrite.Value = Boolean.TRUE;
            aPropertyList.add(aOverwrite);

            // store the document in another directory
            XStorable aStorable = UnoRuntime.queryInterface(XStorable.class, _xComponent);
            if (aStorable != null)
            {
                System.out.println("store document as URL: '" + sOutputURL + "'");
                aStorable.storeAsURL(sOutputURL, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
            }
        }

    private XComponent loadComponent(String _sName, Object _xComponent, ArrayList<PropertyValue> _aPropertyList) throws RuntimeException, IOException
        {
            XComponent xDocComponent = null;
            XComponentLoader xComponentLoader = UnoRuntime.queryInterface(XComponentLoader.class, _xComponent);

            PropertyValue[] aLoadProperties = PropertyHelper.createPropertyValueArrayFormArrayList(_aPropertyList);
            System.out.println("Load component: '" + _sName + "'");
            xDocComponent = xComponentLoader.loadComponentFromURL(_sName, "_blank", 0, aLoadProperties);
            return xDocComponent;
        }

    private void closeComponent(XComponent _xDoc) throws CloseVetoException
        {
            // Close the document
            XCloseable xCloseable = UnoRuntime.queryInterface(XCloseable.class, _xDoc);
            xCloseable.close(true);
        }


    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
