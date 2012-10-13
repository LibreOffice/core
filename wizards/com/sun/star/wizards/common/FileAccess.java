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
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Vector;

import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XTextInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.*;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.DateTime;
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

    /**
     * 
     * @param xMSF
     * @param sPath
     * @param sAddPath
     */
    public static void addOfficePath(XMultiServiceFactory xMSF, String sPath, String sAddPath)
    {
        XSimpleFileAccess xSimpleFileAccess = null;
        String ResultPath = getOfficePath(xMSF, sPath, xSimpleFileAccess);
        // As there are several conventions about the look of Url  (e.g. with " " or with "%20") you cannot make a
        // simple String comparison to find out, if a path is already in "ResultPath"
        String[] PathList = JavaTools.ArrayoutofString(ResultPath, PropertyNames.SEMI_COLON);
        int MaxIndex = PathList.length - 1;
        String CompCurPath;
        String CompAddPath = JavaTools.replaceSubString(sAddPath, PropertyNames.EMPTY_STRING, "/");
        String CurPath;
        for (int i = 0; i <= MaxIndex; i++)
        {
            CurPath = JavaTools.convertfromURLNotation(PathList[i]);
            CompCurPath = JavaTools.replaceSubString(CurPath, PropertyNames.EMPTY_STRING, "/");
            if (CompCurPath.equals(CompAddPath))
            {
                return;
            }
        }
        ResultPath += PropertyNames.SEMI_COLON + sAddPath;
        }

    public static String deleteLastSlashfromUrl(String _sPath)
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
     * @param xMSF
     * @param sPath
     * @param xSimpleFileAccess
     * @return the respective path of the office application. A probable following "/" at the end is trimmed.
     */
    public static String getOfficePath(XMultiServiceFactory xMSF, String sPath, XSimpleFileAccess xSimpleFileAccess)
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
     * @param xMSF
     * @param sPath
     * @param sType use "share" or "user". Set to PropertyNames.EMPTY_STRING if not needed eg for the WorkPath;
     * In the return Officepath a possible slash at the end is cut off
     * @param sSearchDir
     * @return
     * @throws NoValidPathException
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

    public static ArrayList<String> getOfficePaths(XMultiServiceFactory xMSF, String _sPath, String sType, String sSearchDir) throws NoValidPathException
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
                java.util.Locale.getDefault();
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

    public static void combinePaths(XMultiServiceFactory xMSF, ArrayList<String> _aFirstPath, String _sSecondPath) throws NoValidPathException
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

    public static boolean createSubDirectory(XMultiServiceFactory xMSF, XSimpleFileAccess xSimpleFileAccess, String Path)
    {
        String sNoDirCreation = PropertyNames.EMPTY_STRING;
        try
        {
            Resource oResource = new Resource(xMSF, "ImportWizard", "imp");
            sNoDirCreation = oResource.getResText(1050);
            String sMsgDirNotThere = oResource.getResText(1051);
            String sQueryForNewCreation = oResource.getResText(1052);
            String OSPath = JavaTools.convertfromURLNotation(Path);
            String sQueryMessage = JavaTools.replaceSubString(sMsgDirNotThere, OSPath, "%1");
            sQueryMessage = sQueryMessage + (char) 13 + sQueryForNewCreation;
            int icreate = SystemDialog.showMessageBox(xMSF, "QueryBox", VclWindowPeerAttribute.YES_NO, sQueryMessage);
            if (icreate == 2)
            {
                xSimpleFileAccess.createFolder(Path);
                return true;
            }
            return false;
        }
        catch (com.sun.star.ucb.CommandAbortedException exception)
        {
            String sMsgNoDir = JavaTools.replaceSubString(sNoDirCreation, Path, "%1");
            SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, sMsgNoDir);
            return false;
        }
        catch (com.sun.star.uno.Exception unoexception)
        {
            String sMsgNoDir = JavaTools.replaceSubString(sNoDirCreation, Path, "%1");
            SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, sMsgNoDir);
            return false;
        }
    }

    // checks if the root of a path exists. if the parameter xWindowPeer is not null then also the directory is
    // created when it does not exists and the user
    public static boolean PathisValid(XMultiServiceFactory xMSF, String Path, String sMsgFilePathInvalid, boolean baskbeforeOverwrite)
    {
        try
        {
            String SubDir;
            String SubDirPath = PropertyNames.EMPTY_STRING;
            int SubLen;
            int NewLen;
            int RestLen;
            boolean bexists;
            boolean bSubDirexists = true;
            String LowerCasePath;
            String NewPath = Path;
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
            if (baskbeforeOverwrite)
            {
                if (xSimpleFileAccess.exists(Path))
                {
                    Resource oResource = new Resource(xMSF, "ImportWizard", "imp");
                    String sFileexists = oResource.getResText(1053);
                    String NewString = JavaTools.convertfromURLNotation(Path);
                    sFileexists = JavaTools.replaceSubString(sFileexists, NewString, "<1>");
                    sFileexists = JavaTools.replaceSubString(sFileexists, String.valueOf((char) 13), "<CR>");
                    int iLeave = SystemDialog.showMessageBox(xMSF, "QueryBox", VclWindowPeerAttribute.YES_NO, sFileexists);
                    if (iLeave == 3)
                    {
                        return false;
                    }
                }
            }
            String[] DirArray = JavaTools.ArrayoutofString(Path, "/");
            int MaxIndex = DirArray.length - 1;
            if (MaxIndex > 0)
            {
                for (int i = MaxIndex; i >= 0; i--)
                {
                    SubDir = DirArray[i];
                    SubLen = SubDir.length();
                    NewLen = NewPath.length();
                    RestLen = NewLen - SubLen;
                    if (RestLen > 0)
                    {
                        NewPath = NewPath.substring(0, NewLen - SubLen - 1);
                        if (i == MaxIndex)
                        {
                            SubDirPath = NewPath;
                        }
                        bexists = xSimpleFileAccess.exists(NewPath);
                        if (bexists)
                        {
                            LowerCasePath = NewPath.toLowerCase();
                            bexists = (!((LowerCasePath.equals("file:///")) || (LowerCasePath.equals("file://")) || (LowerCasePath.equals("file:/")) || (LowerCasePath.equals("file:"))));
                        }
                        if (bexists)
                        {
                            if (!bSubDirexists)
                            {
                                return createSubDirectory(xMSF, xSimpleFileAccess, SubDirPath);
                            }
                            return true;
                        }
                        else
                        {
                            bSubDirexists = false;
                        }
                    }
                }
            }
            SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, sMsgFilePathInvalid);
            return false;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, sMsgFilePathInvalid);
            return false;
        }
    }

    /**
     * searches a directory for files which start with a certain
     * prefix, and returns their URLs and document-titles.
     * @param xMSF
     * @param FilterName the prefix of the filename. a "-" is added to the prefix !
     * @param FolderName the folder (URL) to look for files...
     * @return an array with two array members. The first one, with document titles, 
     * the second with the corresponding URLs.
     * @deprecated please use the getFolderTitles() with ArrayList
     */
    public static String[][] getFolderTitles(com.sun.star.lang.XMultiServiceFactory xMSF, String FilterName, String FolderName)
    {
        String[][] LocLayoutFiles = new String[2][];
        try
        {
            java.util.Vector<String> TitleVector = null;
            java.util.Vector<String> NameVector = null;

            XInterface xDocInterface = (XInterface) xMSF.createInstance("com.sun.star.document.DocumentProperties");
            XDocumentProperties xDocProps = UnoRuntime.queryInterface(XDocumentProperties.class, xDocInterface);

            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            com.sun.star.ucb.XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);

            String[] nameList = xSimpleFileAccess.getFolderContents(FolderName, false);

            TitleVector = new java.util.Vector<String>(/*nameList.length*/);
            NameVector = new java.util.Vector<String>(nameList.length);

            FilterName = FilterName == null || FilterName.equals(PropertyNames.EMPTY_STRING) ? null : FilterName + "-";

            String fileName = PropertyNames.EMPTY_STRING;
            PropertyValue[] noArgs = { };
            for (int i = 0; i < nameList.length; i++)
            {
                fileName = getFilename(nameList[i]);

                if (FilterName == null || fileName.startsWith(FilterName))
                {
                    xDocProps.loadFromMedium(nameList[i], noArgs);
                    NameVector.addElement(nameList[i]);
                    TitleVector.addElement(xDocProps.getTitle());
                }
            }
            String[] LocNameList = new String[NameVector.size()];
            String[] LocTitleList = new String[TitleVector.size()];

            NameVector.copyInto(LocNameList);
            TitleVector.copyInto(LocTitleList);
            LocLayoutFiles[1] = LocNameList;
            LocLayoutFiles[0] = LocTitleList;

            JavaTools.bubblesortList(LocLayoutFiles);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return LocLayoutFiles;
    }

    /**
     * We search in all given path for a given file
     * @param _sPath
     * @param _sPath2
     * @return
     */
    public static String addPath(String _sPath, String _sPath2)
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

    /**
     * 
     * @param xMSF
     * @param _sStartFilterName
     * @param FolderNames
     * @return
     * @throws com.sun.star.wizards.common.NoValidPathException
     */
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
        if (FolderName.size() == 0)
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
            e.printStackTrace();
            throw new NoValidPathException(null, "Internal error.");
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
    public XSimpleFileAccess2 fileAccess;
    public XFileIdentifierConverter filenameConverter;

    public FileAccess(XMultiServiceFactory xmsf) throws com.sun.star.uno.Exception
    {
        //get a simple file access...
        Object fa = xmsf.createInstance("com.sun.star.ucb.SimpleFileAccess");
        fileAccess = UnoRuntime.queryInterface(XSimpleFileAccess2.class, fa);
        //get the file identifier converter
        Object fcv = xmsf.createInstance("com.sun.star.ucb.FileContentProvider");
        filenameConverter = UnoRuntime.queryInterface(XFileIdentifierConverter.class, fcv);
    }

    public String getURL(String parentPath, String childPath)
    {
        String parent = filenameConverter.getSystemPathFromFileURL(parentPath);
        File f = new File(parent, childPath);
        return filenameConverter.getFileURLFromSystemPath(parentPath, f.getAbsolutePath());
    }

    public String getURL(String path)
    {
        File f = new File(path);
        return filenameConverter.getFileURLFromSystemPath(path, f.getAbsolutePath());
    }

    public String getPath(String parentURL, String childURL)
    {
        return filenameConverter.getSystemPathFromFileURL(parentURL + (((childURL == null || childURL.equals(PropertyNames.EMPTY_STRING)) ? PropertyNames.EMPTY_STRING : "/" + childURL)));
    }

    /**
     * @param filename
     * @return the extension of the given filename.
     */
    public static String getExtension(String filename)
    {
        int p = filename.indexOf(".");
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
            while ((p = filename.indexOf(".")) > -1);
        }
        return filename;
    }

    /**
     * @param s
     * @return
     */
    public boolean mkdir(String s)
    {
        try
        {
            fileAccess.createFolder(s);
            return true;
        }
        catch (CommandAbortedException cax)
        {
            cax.printStackTrace();
        }
        catch (com.sun.star.uno.Exception ex)
        {
            ex.printStackTrace();
        }
        return false;
    }

    /**
     * @param filename
     * @param def what to return in case of an exception
     * @return true if the given file exists or not.
     * if an exception accures, returns the def value.
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

    /**
     * @param filename
     * @return
     */
    public boolean isDirectory(String filename)
    {
        try
        {
            return fileAccess.isFolder(filename);
        }
        catch (CommandAbortedException e)
        {
        }
        catch (Exception e)
        {
        }

        return false;
    }

    /**
     * lists the files in a given directory
     * @param dir
     * @param includeFolders
     * @return
     */
    public String[] listFiles(String dir, boolean includeFolders)
    {
        try
        {
            return fileAccess.getFolderContents(dir, includeFolders);
        }
        catch (CommandAbortedException e)
        {
        }
        catch (Exception e)
        {
        }

        return new String[0];
    }

    /**
     * @param file
     * @return
     */
    public boolean delete(String file)
    {
        try
        {
            fileAccess.kill(file);
            return true;
        }
        catch (CommandAbortedException e)
        {
            e.printStackTrace(System.err);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }

        return false;
    }

    /**
     * @param path
     * @return
     */
    public static String getFilename(String path)
    {
        return getFilename(path, "/");
    }

    /**
     * return the filename out of a system-dependent path
     * @param path
     * @return
     */
    public static String getPathFilename(String path)
    {
        return getFilename(path, File.separator);
    }

    /**
     * @param path 
     * @param pathSeparator
     * @return
     */
    public static String getFilename(String path, String pathSeparator)
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

    /**
     * @param source
     * @param target
     * @return
     */
    public boolean copy(String source, String target)
    {
        try
        {
            fileAccess.copy(source, target);
            return true;
        }
        catch (CommandAbortedException e)
        {
        }
        catch (Exception e)
        {
        }

        return false;
    }

    public DateTime getLastModified(String url)
    {
        try
        {
            return fileAccess.getDateTimeModified(url);
        }
        catch (CommandAbortedException e)
        {
        }
        catch (Exception e)
        {
        }
        return null;
    }

    /**
     * @param url 
     * @return the parent dir of the given url.
     * if the path points to file, gives the directory in which the file is.
     */
    public static String getParentDir(String url)
    {
        if (url.endsWith("/"))
        {
            return getParentDir(url.substring(0, url.length() - 1));
        }
        int pos = -1;
        int lastPos = 0;
        while ((pos = url.indexOf("/", pos + 1)) > -1)
        {
            lastPos = pos;
        }
        return url.substring(0, lastPos);
    }

    public String createNewDir(String parentDir, String name)
    {
        String s = getNewFile(parentDir, name, PropertyNames.EMPTY_STRING);
        if (mkdir(s))
        {
            return s;
        }
        else
        {
            return null;
        }
    }

    public String getNewFile(String parentDir, String name, String extension)
    {

        int i = 0;
        String url;
        do
        {
            String filename = filename(name, extension, i++);
            url = getURL(parentDir, filename);
        }
        while (exists(url, true));

        return url;
    }

    private static String filename(String name, String ext, int i)
    {
        return name + (i == 0 ? PropertyNames.EMPTY_STRING : String.valueOf(i)) + (ext.equals(PropertyNames.EMPTY_STRING) ? PropertyNames.EMPTY_STRING : "." + ext);
    }

    public int getSize(String url)
    {
        try
        {
            return fileAccess.getSize(url);
        }
        catch (Exception ex)
        {
            return -1;
        }
    }

    public static String connectURLs(String urlFolder, String urlFilename)
    {
        return urlFolder + (urlFolder.endsWith("/") ? PropertyNames.EMPTY_STRING : "/") +
                (urlFilename.startsWith("/") ? urlFilename.substring(1) : urlFilename);
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

    /**
     * shortens a filename to a user displayable representation. 
     * @param path
     * @param maxLength
     * @return
     */
    public static String getShortFilename(String path, int maxLength)
    {
        int firstPart = 0;

        if (path.length() > maxLength)
        {
            if (path.startsWith("/"))
            { // unix
                int nextSlash = path.indexOf("/", 1) + 1;
                firstPart = Math.min(nextSlash, (maxLength - 3) / 2);
            }
            else
            { //windows
                firstPart = Math.min(10, (maxLength - 3) / 2);
            }

            String s1 = path.substring(0, firstPart);
            String s2 = path.substring(path.length() - (maxLength - (3 + firstPart)));

            return s1 + "..." + s2;
        }
        else
        {
            return path;
        }
    }
}
