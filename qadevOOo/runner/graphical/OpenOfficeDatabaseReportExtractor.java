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
import helper.PropertyHelper;
import helper.URLHelper;
import java.io.File;
import java.util.ArrayList;

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
                    m_xDesktop =  UnoRuntime.queryInterface(XDesktop.class, xInterface);
                }
                catch (com.sun.star.uno.Exception e)
                {
                    GlobalLogWriter.println("ERROR: uno.Exception caught");
                    GlobalLogWriter.println("Message: " + e.getMessage());
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


    private XMultiServiceFactory m_xMultiServiceFactory = null;
    private XMultiServiceFactory getMultiServiceFactory()
    {
        if (m_xMultiServiceFactory == null)
        {
            m_xMultiServiceFactory = m_aParameterHelper.getMultiServiceFactory();
        }
        return m_xMultiServiceFactory;
    }

    /**
     * This is the main test Function of current ReportDesignerTest
     * @param _sDocument
     * @return
     */

    public ArrayList<String> load(String _sDocument /*, int _nType*/)
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
            GlobalLogWriter.println("File URL: " + sFileURL);

            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();

            XComponent xDocComponent = loadComponent(sFileURL, getXDesktop(), aPropertyList);

            GlobalLogWriter.println("Load done");

            ArrayList<String> aList = null;
            try
            {
                XOfficeDatabaseDocument xOfficeDBDoc = UnoRuntime.queryInterface(XOfficeDatabaseDocument.class, xDocComponent);

                assure("can't access DatabaseDocument", xOfficeDBDoc != null);

                XModel xDBSource = UnoRuntime.queryInterface(XModel.class, xOfficeDBDoc);
                Object aController = xDBSource.getCurrentController();
                assure("Controller of xOfficeDatabaseDocument is empty!", aController != null);

                XDatabaseDocumentUI aDBDocUI = UnoRuntime.queryInterface(XDatabaseDocumentUI.class, aController);
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

                XReportDocumentsSupplier xSupplier = UnoRuntime.queryInterface(XReportDocumentsSupplier.class, xOfficeDBDoc);
                XNameAccess xNameAccess = xSupplier.getReportDocuments();
                assure("xOfficeDatabaseDocument returns no Report Document", xNameAccess != null);

                showElements(xNameAccess);

                Object aActiveConnectionObj = aDBDocUI.getActiveConnection();
                assure("ActiveConnection is empty", aActiveConnectionObj != null);

                ArrayList<PropertyValue> aPropertyList2 = new ArrayList<PropertyValue>();

                PropertyValue aActiveConnection = new PropertyValue();
                aActiveConnection.Name = "ActiveConnection";
                aActiveConnection.Value = aActiveConnectionObj;
                aPropertyList2.add(aActiveConnection);

                aList = loadAndStoreReports(xNameAccess, aPropertyList2);
                createDBEntry();
            }
            catch(Exception e)
            {
                GlobalLogWriter.println("ERROR: Exception caught");
                GlobalLogWriter.println("Message: " + e.getMessage());
            }

            closeComponent(xDocComponent);
            return aList;
        }

    private String getDocumentPoolName()
        {
            return "AutogenReportDesignTest";
        }

    private void createDBEntry()
        {
            // try to connect the database
            String sDBConnection = (String)m_aParameterHelper.getTestParameters().get( convwatch.PropertyName.DB_CONNECTION_STRING );
            if (sDBConnection != null && sDBConnection.length() > 0)
            {
                GlobalLogWriter.println("DBConnection: " + sDBConnection);

                getOutputPath();
                getDocumentPoolName();
                TimeHelper.waitInSeconds(1, "wait for DB.");
            }
        }

    private ArrayList<String> loadAndStoreReports(XNameAccess _xNameAccess, ArrayList<PropertyValue> _aPropertyList)
        {
            ArrayList<String> aList = new ArrayList<String>();
            if (_xNameAccess != null)
            {
                String[] sElementNames = _xNameAccess.getElementNames();
                for(int i=0;i<sElementNames.length; i++)
                {
                    String sReportName = sElementNames[i];
                    XComponent xDoc = loadComponent(sReportName, _xNameAccess, _aPropertyList);
                    if (xDoc != null)
                    {
                        String sDocumentPathName = storeComponent(sReportName, xDoc);
                        aList.add(sDocumentPathName);
                        closeComponent(xDoc);
                    }
                    else
                    {
                        System.out.println("Leave out maybe due to errors.");
                    }
                    // sBackPath contains the path where to find the extracted ODB Document
                }
            }
            return aList;
        }

    private String getFormatExtension(Object _xComponent)
         {
             String sExtension;
             XServiceInfo xServiceInfo =  UnoRuntime.queryInterface( XServiceInfo.class, _xComponent );
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

    private String m_sOutputPath = null;

    private String getOutputPath()
        {
            if (m_sOutputPath == null)
            {
                String sOutputPath = (String)m_aParameterHelper.getTestParameters().get( convwatch.PropertyName.DOC_COMPARATOR_OUTPUT_PATH );
                sOutputPath = helper.StringHelper.removeQuoteIfExists(sOutputPath);

                sOutputPath = FileHelper.appendPath(sOutputPath, DateHelper.getDateTimeForFilename());

                File aOutputFile = new File(sOutputPath); // create the directory of the given output path
                aOutputFile.mkdirs();
                m_sOutputPath = sOutputPath;
            }
            return m_sOutputPath;
        }

    /*
      store given _xComponent under the given Name in DOC_COMPARATOR_INPUTPATH
     */
    private String storeComponent(String _sName, Object _xComponent)
        {
            String sOutputPath = getOutputPath();

            String sName = _sName + getFormatExtension(_xComponent);
            sOutputPath = FileHelper.appendPath(sOutputPath, sName);

            // we need the name and path
            String sBackPathName = sOutputPath;

            String sOutputURL = URLHelper.getFileURLFromSystemPath(sOutputPath);

            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>(); // set some properties for storeAsURL

            PropertyValue aOverwrite = new PropertyValue(); // always overwrite already exist files
            aOverwrite.Name = "Overwrite";
            aOverwrite.Value = Boolean.TRUE;
            aPropertyList.add(aOverwrite);

            // store the document in an other directory
            XStorable aStorable = UnoRuntime.queryInterface( XStorable.class, _xComponent);
            if (aStorable != null)
            {
                GlobalLogWriter.println("store document as URL: '" + sOutputURL + "'");
                try
                {
                    aStorable.storeAsURL(sOutputURL, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
                }
                catch (com.sun.star.io.IOException e)
                {
                    GlobalLogWriter.println("ERROR: Exception caught");
                    GlobalLogWriter.println("Can't write document URL: '" + sOutputURL + "'");
                    GlobalLogWriter.println("Message: " + e.getMessage());
                }
            }
            return sBackPathName;
        }

    private XComponent loadComponent(String _sName, Object _xComponent, ArrayList<PropertyValue> _aPropertyList)
        {
            XComponent xDocComponent = null;
            XComponentLoader xComponentLoader =  UnoRuntime.queryInterface( XComponentLoader.class, _xComponent );

            try
            {
                PropertyValue[] aLoadProperties = PropertyHelper.createPropertyValueArrayFormArrayList(_aPropertyList);
                GlobalLogWriter.println("Load component: '" + _sName + "'");
                xDocComponent = xComponentLoader.loadComponentFromURL(_sName, "_blank", FrameSearchFlag.ALL, aLoadProperties);
                GlobalLogWriter.println("Load component: '" + _sName + "' done");
            }
            catch (com.sun.star.io.IOException e)
            {
                GlobalLogWriter.println("ERROR: Exception caught");
                GlobalLogWriter.println("Can't load document '" + _sName + "'");
                GlobalLogWriter.println("Message: " + e.getMessage());
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                GlobalLogWriter.println("ERROR: Exception caught");
                GlobalLogWriter.println("Illegal Arguments given to loadComponentFromURL.");
                GlobalLogWriter.println("Message: " + e.getMessage());
            }
            return xDocComponent;
        }

    private void closeComponent(XComponent _xDoc)
        {
            // Close the document
            XCloseable xCloseable =  UnoRuntime.queryInterface(XCloseable.class, _xDoc);
            try
            {
                xCloseable.close(true);
            }
            catch (com.sun.star.util.CloseVetoException e)
            {
                GlobalLogWriter.println("ERROR: CloseVetoException caught");
                GlobalLogWriter.println("CloseVetoException occurred Can't close document.");
                GlobalLogWriter.println("Message: " + e.getMessage());
            }
        }

}
