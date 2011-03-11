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

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.Locale;
import com.sun.star.uno.Exception;
import com.sun.star.util.XMacroExpander;
// import com.sun.star.wizards.common.NoValidPathException;
import java.io.File;
import java.util.ArrayList;
import java.util.Vector;

import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
// import com.sun.star.io.XStream;
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
        //  sAddPath.replace(null, (char) 47);
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
        return;
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
            exception.printStackTrace(System.out);
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
            XPropertySet xPropertySet = (XPropertySet) com.sun.star.uno.UnoRuntime.queryInterface(XPropertySet.class, xPathInterface);
            String WritePath = PropertyNames.EMPTY_STRING;
            String[] ReadPaths = null;
            XInterface xUcbInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);

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
            exception.printStackTrace(System.out);
            ResultPath = PropertyNames.EMPTY_STRING;
        }
        if (bexists == false)
        {
            throw new NoValidPathException(xMSF, PropertyNames.EMPTY_STRING);
        }
        return ResultPath;
    }

    public static ArrayList<String> getOfficePaths(XMultiServiceFactory xMSF, String _sPath, String sType, String sSearchDir) throws NoValidPathException
    {
        //This method currently only works with sPath="Template"

        // String ResultPath = PropertyNames.EMPTY_STRING;
        ArrayList<String> aPathList = new ArrayList<String>();
        String Template_writable = PropertyNames.EMPTY_STRING;
        String[] Template_internal;
        String[] Template_user;

        // String [] ReadPaths = null;

        // boolean bexists = false;
        try
        {
            XInterface xPathInterface = (XInterface) xMSF.createInstance("com.sun.star.util.PathSettings");
            XPropertySet xPropertySet = (XPropertySet) com.sun.star.uno.UnoRuntime.queryInterface(XPropertySet.class, xPathInterface);
            // String WritePath = PropertyNames.EMPTY_STRING;
            // XInterface xUcbInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            // XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);

            Template_writable = (String) xPropertySet.getPropertyValue(_sPath + "_writable");
            Template_internal = (String[]) xPropertySet.getPropertyValue(_sPath + "_internal");
            Template_user = (String[]) xPropertySet.getPropertyValue(_sPath + "_user");

            // int iNumEntries = Template_user.length + Template_internal.length + 1;
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
            for (int i = 0; i < Template_user.length; i++)
            {
                aPathList.add(Template_user[i]);
            }
            aPathList.add(Template_writable);
        // WritePath = Template_writable;

//            if (sType.equalsIgnoreCase("user"))
//            {
//                ResultPath = WritePath;
//                bexists = true;
//            }

        // There was a bug here, because we have to search through the whole list of paths
//            else
//            {
//                //find right path using the search sub path
//                for (int i = 0; i<ReadPaths.length; i++)
//                {
//                    String tmpPath = ReadPaths[i]+sSearchDir;
//                    if (xSimpleFileAccess.exists(tmpPath))
//                    {
//                        ResultPath = ReadPaths[i];
//                        bexists = true;
//                        break;
//                    }
//                }
//            }
//            ResultPath = deleteLastSlashfromUrl(ResultPath);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        // ResultPath = PropertyNames.EMPTY_STRING;
        }
//        if (bexists == false)
//        {
//            throw new NoValidPathException(xMSF, PropertyNames.EMPTY_STRING);
//        }
//        return ResultPath;
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
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
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

        // java.util.Locale jl = new java.util.Locale(
        //        l.Language , l.Country, l.Variant );
        }
        catch (com.sun.star.uno.Exception e)
        {
        }

        return _sPath;
    }

    /*
    public static String getOfficePath(XMultiServiceFactory xMSF, String sPath, String sType) throws NoValidPathException {
    String ResultPath = PropertyNames.EMPTY_STRING;
    Object oPathSettings;
    int iPathCount;
    String[] PathList;
    boolean bexists = false;
    try {
    XInterface xUcbInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
    XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);
    ResultPath = getOfficePath(xMSF, sPath, xSimpleFileAccess);
    PathList = JavaTools.ArrayoutofString(ResultPath, PropertyNames.SEMI_COLON);
    if (!sType.equals(PropertyNames.EMPTY_STRING)) {
    ResultPath = PropertyNames.EMPTY_STRING;
    String CurPath = PropertyNames.EMPTY_STRING;
    String EndString = "/" + sType;
    int EndLength = EndString.length();
    sType = "/" + sType + "/";
    int MaxIndex = PathList.length - 1;
    int iPos;
    for (int i = 0; i <= MaxIndex; i++) {
    CurPath = PathList[i];
    iPos = CurPath.length() - EndLength;
    if ((CurPath.indexOf(sType) > 0) || (CurPath.indexOf(EndString) == iPos)) {
    ResultPath = deleteLastSlashfromUrl(CurPath);
    if (xSimpleFileAccess.exists(ResultPath))
    break;
    }
    }
    } else
    ResultPath = PathList[0];
    if (ResultPath.equals(PropertyNames.EMPTY_STRING) == false)
    bexists = xSimpleFileAccess.exists(ResultPath);
    } catch (Exception exception) {
    exception.printStackTrace(System.out);
    ResultPath = PropertyNames.EMPTY_STRING;
    }
    if (bexists == false)
    throw new NoValidPathException(xMSF);
    return ResultPath;
    }
     **/
    public static void combinePaths(XMultiServiceFactory xMSF, ArrayList _aFirstPath, String _sSecondPath) throws NoValidPathException
    {
        for (int i = 0; i < _aFirstPath.size(); ++i)
        {
            String sOnePath = (String) _aFirstPath.get(i);
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
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);
            bExists = xSimpleFileAccess.exists(_sPath);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
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
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);
            ReturnPath = _sFirstPath + _sSecondPath;
            bexists = xSimpleFileAccess.exists(ReturnPath);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            return PropertyNames.EMPTY_STRING;
        }
        if (bexists == false)
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
            if (oResource != null)
            {
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
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
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
                            bexists = (((LowerCasePath.equals("file:///")) || (LowerCasePath.equals("file://")) || (LowerCasePath.equals("file:/")) || (LowerCasePath.equals("file:"))) == false);
                        }
                        if (bexists)
                        {
                            if (bSubDirexists == false)
                            {
                                boolean bSubDiriscreated = createSubDirectory(xMSF, xSimpleFileAccess, SubDirPath);
                                return bSubDiriscreated;
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
            exception.printStackTrace(System.out);
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
        String[][] LocLayoutFiles = new String[2][]; //{PropertyNames.EMPTY_STRING,PropertyNames.EMPTY_STRING}{PropertyNames.EMPTY_STRING};
        try
        {
            java.util.Vector<String> TitleVector = null;
            java.util.Vector<String> NameVector = null;

            XInterface xDocInterface = (XInterface) xMSF.createInstance("com.sun.star.document.DocumentProperties");
            XDocumentProperties xDocProps = (XDocumentProperties) UnoRuntime.queryInterface(XDocumentProperties.class, xDocInterface);

            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            com.sun.star.ucb.XSimpleFileAccess xSimpleFileAccess = (com.sun.star.ucb.XSimpleFileAccess) UnoRuntime.queryInterface(com.sun.star.ucb.XSimpleFileAccess.class, xInterface);

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
            exception.printStackTrace(System.out);
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

    public static String getPathFromList(XMultiServiceFactory xMSF, ArrayList _aList, String _sFile)
    {
        String sFoundFile = PropertyNames.EMPTY_STRING;
        try
        {
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            com.sun.star.ucb.XSimpleFileAccess xSimpleFileAccess = (com.sun.star.ucb.XSimpleFileAccess) UnoRuntime.queryInterface(com.sun.star.ucb.XSimpleFileAccess.class, xInterface);

            for (int i = 0; i < _aList.size(); i++)
            {
                String sPath = (String) _aList.get(i);
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
    public static String[][] getFolderTitles(com.sun.star.lang.XMultiServiceFactory xMSF, String _sStartFilterName, ArrayList FolderNames)
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
            XDocumentProperties xDocProps = (XDocumentProperties) UnoRuntime.queryInterface(XDocumentProperties.class, xDocInterface);
            PropertyValue[] noArgs = { };
            xDocProps.loadFromMedium(_sFile, noArgs);
            sTitle = xDocProps.getTitle();
        }
        catch (Exception e)
        {
        }
        return sTitle;
    }

    public static String[][] getFolderTitles(com.sun.star.lang.XMultiServiceFactory xMSF, String _sStartFilterName, ArrayList FolderName, String _sEndFilterName)
            throws NoValidPathException
    {
        String[][] LocLayoutFiles = new String[2][]; //{PropertyNames.EMPTY_STRING,PropertyNames.EMPTY_STRING}{PropertyNames.EMPTY_STRING};
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
            xSimpleFileAccess = (com.sun.star.ucb.XSimpleFileAccess) UnoRuntime.queryInterface(com.sun.star.ucb.XSimpleFileAccess.class, xInterface);
        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace();
            throw new NoValidPathException(null, "Internal error.");
        }

        for (int j = 0; j < FolderName.size(); j++)
        {
            String sFolderName = (String) FolderName.get(j);

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
                exception.printStackTrace(System.out);
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
        fileAccess = (XSimpleFileAccess2) UnoRuntime.queryInterface(XSimpleFileAccess2.class, fa);
        //get the file identifier converter
        Object fcv = xmsf.createInstance("com.sun.star.ucb.FileContentProvider");
        filenameConverter = (XFileIdentifierConverter) UnoRuntime.queryInterface(XFileIdentifierConverter.class, fcv);
    }

    public String getURL(String parentPath, String childPath)
    {
        String parent = filenameConverter.getSystemPathFromFileURL(parentPath);
        File f = new File(parent, childPath);
        String r = filenameConverter.getFileURLFromSystemPath(parentPath, f.getAbsolutePath());
        return r;
    }

    public String getURL(String path)
    {
        File f = new File(path);
        String r = filenameConverter.getFileURLFromSystemPath(
                path, f.getAbsolutePath());
        return r;
    }

    public String getPath(String parentURL, String childURL)
    {
        return filenameConverter.getSystemPathFromFileURL(parentURL + (((childURL == null || childURL.equals(PropertyNames.EMPTY_STRING)) ? PropertyNames.EMPTY_STRING : "/" + childURL)));
    }

    /**
     * @author rpiterman
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
     * @author rpiterman
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
     * @author rpiterman
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
     * @author rpiterman
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
     * @author rpiterman
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
     * @author rpiterman
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
            e.printStackTrace(System.out);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }

        return false;
    }

    /**
     * @author rpiterman
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
     * @author rpiterman
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
        String basename = filename.substring(0, filename.length() - (sExtension.length() + 1));
        return basename;
    }

    /**
     * @author rpiterman
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
     *
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
            String u = getURL(parentDir, filename);
            url = u;
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
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, oSimpleFileAccess);
            if (xSimpleFileAccess.exists(_filepath))
            {
                XInputStream xInputStream = xSimpleFileAccess.openFileRead(_filepath);
                Object oTextInputStream = _xMSF.createInstance("com.sun.star.io.TextInputStream");
                XTextInputStream xTextInputStream = (XTextInputStream) UnoRuntime.queryInterface(XTextInputStream.class, oTextInputStream);
                XActiveDataSink xActiveDataSink = (XActiveDataSink) UnoRuntime.queryInterface(XActiveDataSink.class, oTextInputStream);
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
            e.printStackTrace(System.out);
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
