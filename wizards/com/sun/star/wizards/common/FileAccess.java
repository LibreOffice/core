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

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.Locale;
import com.sun.star.uno.Exception;
import com.sun.star.util.XMacroExpander;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Vector;

import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XTextInputStream;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.CommandAbortedException;
import com.sun.star.ucb.XFileIdentifierConverter;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.ucb.XSimpleFileAccess2;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.beans.PropertyValue;
import com.sun.star.document.XDocumentProperties;

/**
 * This class delivers static convenience methods
 * to use with ucb SimpleFileAccess service.
 * You can also instanciate the class, to encapsulate
 * some functionality of SimpleFileAccess. The instance
 * keeps a reference to an XSimpleFileAccess and an
 * XFileIdentifierConverter, saves the permanent
 * overhead of quering for those interfaces, and delivers
 * conveneince methods for using them.
 * These Convenince methods include mainly Exception-handling.
 */
public class FileAccess
{

    private static String deleteLastSlashfromUrl(String _sPath)
    {
        if (_sPath.endsWith("/"))
        {
            return _sPath.substring(0, _sPath.length() - 1);
        }
        else
        {
            return _sPath;
        }
    }

    /**
     * Further information on arguments value see in OO Developer Guide,
     * chapter 6.2.7
     * @return the respective path of the office application. A probable following "/" at the end is trimmed.
     */
    public static String getOfficePath(XMultiServiceFactory xMSF, String sPath)
    {
        try
        {
            String ResultPath = PropertyNames.EMPTY_STRING;
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.util.PathSettings");
            ResultPath = com.sun.star.uno.AnyConverter.toString(Helper.getUnoPropertyValue(xInterface, sPath));
            ResultPath = deleteLastSlashfromUrl(ResultPath);
            return ResultPath;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return PropertyNames.EMPTY_STRING;
        }
    }

    /**
     * Further information on arguments value see in OO Developer Guide,
     * chapter 6.2.7
     * @param sType use "share" or "user". Set to PropertyNames.EMPTY_STRING if not needed eg for the WorkPath;
     *    In the return Officepath a possible slash at the end is cut off
     */
    public static String getOfficePath(XMultiServiceFactory xMSF, String sPath, String sType, String sSearchDir) throws NoValidPathException
    {
        //This method currently only works with sPath="Template"

        String ResultPath = PropertyNames.EMPTY_STRING;

        String Template_writable = PropertyNames.EMPTY_STRING;
        String[] Template_internal;
        String[] Template_user;

        boolean bexists = false;
        try
        {
            XInterface xPathInterface = (XInterface) xMSF.createInstance("com.sun.star.util.PathSettings");
            XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xPathInterface);
            String WritePath = PropertyNames.EMPTY_STRING;
            String[] ReadPaths = null;
            XInterface xUcbInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);

            Template_writable = (String) xPropertySet.getPropertyValue(sPath + "_writable");
            Template_internal = (String[]) xPropertySet.getPropertyValue(sPath + "_internal");
            Template_user = (String[]) xPropertySet.getPropertyValue(sPath + "_user");
            int iNumEntries = Template_user.length + Template_internal.length + 1;
            ReadPaths = new String[iNumEntries];
            int t = 0;
            for (int i = 0; i < Template_internal.length; i++)
            {
                ReadPaths[t] = Template_internal[i];
                t++;
            }
            for (int i = 0; i < Template_user.length; i++)
            {
                ReadPaths[t] = Template_user[i];
                t++;
            }
            ReadPaths[t] = Template_writable;
            WritePath = Template_writable;

            if (sType.equalsIgnoreCase("user"))
            {
                ResultPath = WritePath;
                bexists = true;
            }
            else
            {
                //find right path using the search sub path
                for (int i = 0; i < ReadPaths.length; i++)
                {
                    String tmpPath = ReadPaths[i] + sSearchDir;
                    if (xSimpleFileAccess.exists(tmpPath))
                    {
                        ResultPath = ReadPaths[i];
                        bexists = true;
                        break;
                    }
                }
            }
            ResultPath = deleteLastSlashfromUrl(ResultPath);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            ResultPath = PropertyNames.EMPTY_STRING;
        }
        if (!bexists)
        {
            throw new NoValidPathException(xMSF, PropertyNames.EMPTY_STRING);
        }
        return ResultPath;
    }

    public static ArrayList<String> getOfficePaths(XMultiServiceFactory xMSF, String _sPath)
    {
        //This method currently only works with sPath="Template"

        ArrayList<String> aPathList = new ArrayList<String>();
        String Template_writable = PropertyNames.EMPTY_STRING;
        String[] Template_internal;
        String[] Template_user;

        try
        {
            XInterface xPathInterface = (XInterface) xMSF.createInstance("com.sun.star.util.PathSettings");
            XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xPathInterface);
            Template_writable = (String) xPropertySet.getPropertyValue(_sPath + "_writable");
            Template_internal = (String[]) xPropertySet.getPropertyValue(_sPath + "_internal");
            Template_user = (String[]) xPropertySet.getPropertyValue(_sPath + "_user");

            for (int i = 0; i < Template_internal.length; i++)
            {
                String sPath = Template_internal[i];
                if (sPath.startsWith("vnd."))
                {
                    String sPathToExpand = sPath.substring("vnd.sun.star.Expand:".length());

                    XMacroExpander xExpander = Helper.getMacroExpander(xMSF);
                    sPath = xExpander.expandMacros(sPathToExpand);
                }

                // if there exists a language in the directory, we try to add the right language
                sPath = checkIfLanguagePathExists(xMSF, sPath);

                aPathList.add(sPath);
            }
            aPathList.addAll(Arrays.asList(Template_user));
            aPathList.add(Template_writable);
        // There was a bug here, because we have to search through the whole list of paths
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return aPathList;
    }

    private static String checkIfLanguagePathExists(XMultiServiceFactory _xMSF, String _sPath)
    {
        try
        {
            Object defaults = _xMSF.createInstance("com.sun.star.text.Defaults");
            Locale aLocale = (Locale) Helper.getUnoStructValue(defaults, "CharLocale");
            if (aLocale == null)
            {
                aLocale = new com.sun.star.lang.Locale();
                aLocale.Country = java.util.Locale.getDefault().getCountry();
                aLocale.Language = java.util.Locale.getDefault().getLanguage();
                aLocale.Variant = java.util.Locale.getDefault().getVariant();
            }

            String sLanguage = aLocale.Language;
            String sCountry = aLocale.Country;
            String sVariant = aLocale.Variant;

            // de-DE-Bayrisch
            StringBuffer aLocaleAll = new StringBuffer();
            aLocaleAll.append(sLanguage).append('-').append(sCountry).append('-').append(sVariant);
            String sPath = _sPath + "/" + aLocaleAll.toString();

            XInterface xInterface = (XInterface) _xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
            if (xSimpleFileAccess.exists(sPath))
            {
                return sPath;
            }

            // de-DE
            StringBuffer aLocaleLang_Country = new StringBuffer();
            aLocaleLang_Country.append(sLanguage).append('-').append(sCountry);
            sPath = _sPath + "/" + aLocaleLang_Country.toString();

            if (xSimpleFileAccess.exists(sPath))
            {
                return sPath;
            }

            // de
            StringBuffer aLocaleLang = new StringBuffer();
            aLocaleLang.append(sLanguage);
            sPath = _sPath + "/" + aLocaleLang.toString();

            if (xSimpleFileAccess.exists(sPath))
            {
                return sPath;
            }

            // the absolute default is en-US or en
            sPath = _sPath + "/en-US";
            if (xSimpleFileAccess.exists(sPath))
            {
                return sPath;
            }

            sPath = _sPath + "/en";
            if (xSimpleFileAccess.exists(sPath))
            {
                return sPath;
            }

        }
        catch (com.sun.star.uno.Exception e)
        {
        }

        return _sPath;
    }

    public static void combinePaths(XMultiServiceFactory xMSF, ArrayList<String> _aFirstPath, String _sSecondPath)
    {
        for (int i = 0; i < _aFirstPath.size(); ++i)
        {
            String sOnePath = _aFirstPath.get(i);
            sOnePath = addPath(sOnePath, _sSecondPath);
            if (isPathValid(xMSF, sOnePath))
            {
                _aFirstPath.add(i, sOnePath);
                _aFirstPath.remove(i + 1);
            }
            else
            {
                _aFirstPath.remove(i);
                --i;
            }
        }
    }

    public static boolean isPathValid(XMultiServiceFactory xMSF, String _sPath)
    {
        boolean bExists = false;
        try
        {
            XInterface xUcbInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);
            bExists = xSimpleFileAccess.exists(_sPath);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return bExists;
    }

    public static String combinePaths(XMultiServiceFactory xMSF, String _sFirstPath, String _sSecondPath) throws NoValidPathException
    {
        boolean bexists = false;
        String ReturnPath = PropertyNames.EMPTY_STRING;
        try
        {
            XInterface xUcbInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);
            ReturnPath = _sFirstPath + _sSecondPath;
            bexists = xSimpleFileAccess.exists(ReturnPath);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return PropertyNames.EMPTY_STRING;
        }
        if (!bexists)
        {
            throw new NoValidPathException(xMSF, PropertyNames.EMPTY_STRING);
        }
        return ReturnPath;
    }

    /**
     * We search in all given path for a given file
     */
    private static String addPath(String _sPath, String _sPath2)
    {
        String sNewPath;
        if (!_sPath.endsWith("/"))
        {
            _sPath += "/";
        }
        if (_sPath2.startsWith("/"))
        {
            _sPath2 = _sPath2.substring(1);
        }
        sNewPath = _sPath + _sPath2;
        return sNewPath;
    }

    public static String getPathFromList(XMultiServiceFactory xMSF, ArrayList<String> _aList, String _sFile)
    {
        String sFoundFile = PropertyNames.EMPTY_STRING;
        try
        {
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            com.sun.star.ucb.XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);

            for (int i = 0; i < _aList.size(); i++)
            {
                String sPath = _aList.get(i);
                sPath = addPath(sPath, _sFile);
                if (xSimpleFileAccess.exists(sPath))
                {
                    sFoundFile = sPath;
                }
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
        }
        return sFoundFile;
    }

    public static String[][] getFolderTitles(com.sun.star.lang.XMultiServiceFactory xMSF, String _sStartFilterName, ArrayList<String> FolderNames)
            throws NoValidPathException
    {
        return getFolderTitles(xMSF, _sStartFilterName, FolderNames, PropertyNames.EMPTY_STRING);
    }

    private static String getTitle(XMultiServiceFactory xMSF, String _sFile)
    {
        String sTitle = PropertyNames.EMPTY_STRING;
        try
        {
            XInterface xDocInterface = (XInterface) xMSF.createInstance("com.sun.star.document.DocumentProperties");
            XDocumentProperties xDocProps = UnoRuntime.queryInterface(XDocumentProperties.class, xDocInterface);
            PropertyValue[] noArgs = { };
            xDocProps.loadFromMedium(_sFile, noArgs);
            sTitle = xDocProps.getTitle();
        }
        catch (Exception e)
        {
        }
        return sTitle;
    }

    public static String[][] getFolderTitles(com.sun.star.lang.XMultiServiceFactory xMSF, String _sStartFilterName, ArrayList<String> FolderName, String _sEndFilterName)
            throws NoValidPathException
    {
        String[][] LocLayoutFiles = new String[2][];
        if (FolderName.isEmpty())
        {
            throw new NoValidPathException(null, "Path not given.");
        }
        ArrayList<String> TitleVector = new ArrayList<String>();
        ArrayList<String> URLVector = new ArrayList<String>();

        com.sun.star.ucb.XSimpleFileAccess xSimpleFileAccess = null;
        try
        {
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
        }
        catch (com.sun.star.uno.Exception e)
        {
            throw new NoValidPathException(null, "Internal error.", e);
        }

        for (int j = 0; j < FolderName.size(); j++)
        {
            String sFolderName = FolderName.get(j);

            try
            {
                String[] nameList = xSimpleFileAccess.getFolderContents(sFolderName, false);
                _sStartFilterName = _sStartFilterName == null || _sStartFilterName.equals(PropertyNames.EMPTY_STRING) ? null : _sStartFilterName + "-";

                String fileName = PropertyNames.EMPTY_STRING;
                for (int i = 0; i < nameList.length; i++)
                {
                    fileName = getFilename(nameList[i]);
                    String sTitle;

                    if (_sStartFilterName == null || fileName.startsWith(_sStartFilterName))
                    {
                        if (_sEndFilterName.equals(PropertyNames.EMPTY_STRING))
                        {
                            sTitle = getTitle(xMSF, nameList[i]);
                        }
                        else if (fileName.endsWith(_sEndFilterName))
                        {
                            fileName = fileName.replaceAll(_sEndFilterName + "$", PropertyNames.EMPTY_STRING);
                            sTitle = fileName;
                        }
                        else
                        {
                            // no or wrong (start|end) filter
                            continue;
                        }
                        URLVector.add(nameList[i]);
                        TitleVector.add(sTitle);
                    }
                }
            }
            catch (com.sun.star.ucb.CommandAbortedException exception)
            {
                exception.printStackTrace(System.err);
            }
            catch (com.sun.star.uno.Exception e)
            {
            }
        }

        String[] LocNameList = new String[URLVector.size()];
        String[] LocTitleList = new String[TitleVector.size()];

        // LLA: we have to check if this works
        URLVector.toArray(LocNameList);
        TitleVector.toArray(LocTitleList);

        LocLayoutFiles[1] = LocNameList;
        LocLayoutFiles[0] = LocTitleList;

        JavaTools.bubblesortList(LocLayoutFiles);

        return LocLayoutFiles;
    }
    private XSimpleFileAccess2 fileAccess;

    public FileAccess(XMultiServiceFactory xmsf) throws com.sun.star.uno.Exception
    {
        //get a simple file access...
        Object fa = xmsf.createInstance("com.sun.star.ucb.SimpleFileAccess");
        fileAccess = UnoRuntime.queryInterface(XSimpleFileAccess2.class, fa);
        //get the file identifier converter
        Object fcv = xmsf.createInstance("com.sun.star.ucb.FileContentProvider");
        UnoRuntime.queryInterface(XFileIdentifierConverter.class, fcv);
    }

    /**
     * @return the extension of the given filename.
     */
    private static String getExtension(String filename)
    {
        int p = filename.indexOf('.');
        if (p == -1)
        {
            return PropertyNames.EMPTY_STRING;
        }
        else
        {
            do
            {
                filename = filename.substring(p + 1);
            }
            while ((p = filename.indexOf('.')) > -1);
        }
        return filename;
    }

    /**
     * @param def what to return in case of an exception
     * @return true if the given file exists or not.
     * if an exception occurs, returns the default value.
     */
    public boolean exists(String filename, boolean def)
    {
        try
        {
            return fileAccess.exists(filename);
        }
        catch (CommandAbortedException e)
        {
        }
        catch (Exception e)
        {
        }

        return def;
    }

    public static String getFilename(String path)
    {
        return getFilename(path, "/");
    }

    private static String getFilename(String path, String pathSeparator)
    {
        String[] s = JavaTools.ArrayoutofString(path, pathSeparator);
        return s[s.length - 1];
    }

    public static String getBasename(String path, String pathSeparator)
    {
        String filename = getFilename(path, pathSeparator);
        String sExtension = getExtension(filename);
        return filename.substring(0, filename.length() - (sExtension.length() + 1));
    }

    public static String[] getDataFromTextFile(XMultiServiceFactory _xMSF, String _filepath)
    {
        String[] sFileData = null;
        try
        {
            Vector<String> oDataVector = new Vector<String>();
            Object oSimpleFileAccess = _xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, oSimpleFileAccess);
            if (xSimpleFileAccess.exists(_filepath))
            {
                XInputStream xInputStream = xSimpleFileAccess.openFileRead(_filepath);
                Object oTextInputStream = _xMSF.createInstance("com.sun.star.io.TextInputStream");
                XTextInputStream xTextInputStream = UnoRuntime.queryInterface(XTextInputStream.class, oTextInputStream);
                XActiveDataSink xActiveDataSink = UnoRuntime.queryInterface(XActiveDataSink.class, oTextInputStream);
                xActiveDataSink.setInputStream(xInputStream);
                while (!xTextInputStream.isEOF())
                {
                    oDataVector.addElement( xTextInputStream.readLine());
                }
                xTextInputStream.closeInput();
                sFileData = new String[oDataVector.size()];
                oDataVector.toArray(sFileData);

            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return sFileData;
    }

}
