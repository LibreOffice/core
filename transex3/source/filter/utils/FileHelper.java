/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FileHelper.java,v $
 * $Revision: 1.13 $
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

package com.sun.star.filter.config.tools.utils;

// __________ Imports __________

import java.io.*;
import java.lang.*;
import java.net.*;
import java.util.*;

// __________ Implementation __________

/**
 * It collects some static helper functons to handle file system specific problems.
 * Sometimes it's neccessary to convert URL from/to system pathes;
 * or from string notation to structural versions (e.g. com.sun.star.util.URL).
 * And sometimes java had another notation then the office it has.
 * Further it provides functionality to work easiear with the java.io.File class of java.
 *
 *
 */
public class FileHelper
{
    // ____________________

    /**
     * Because the office need URLs for loading/saving documents
     * we must convert used system pathes.
     * And java use another notation for file URLs ... correct it.
     *
     * @param aSystemPath
     *          represent the file in system notation
     *
     * @return [String]
     *          a file url which represent the given system path
     */
    public static java.lang.String getFileURLFromSystemPath(java.io.File aSystemPath)
    {
        System.out.println("TODO: must be adapted to java 1.3 :-(");
        System.exit(-1);
/*TODO_JAVA
        try
        {
            sFileURL = aSystemPath.toURI().toURL().toString();
        }
        catch( MalformedURLException exWrong )
        {
            sFileURL = null;
        }
*/
        java.lang.String sFileURL = null;

        // problem of java: file URL's are coded with 1 slash instead of 2 or 3 ones!
        // => correct this problem first, otherwise office can't use these URL's
        if(
            (sFileURL                       != null ) &&
            (sFileURL.startsWith("file:/")  == true ) &&
            (sFileURL.startsWith("file://") == false)
          )
        {
            java.lang.StringBuffer sWorkBuffer = new java.lang.StringBuffer(sFileURL);
            sWorkBuffer.insert(6,"//");
            sFileURL = sWorkBuffer.toString();
        }

        return sFileURL;
    }

    // ____________________

    /**
     * The same as getFileURLFromSystemPath() before but uses string parameter instead
     * of a java.io.File type. It exist to supress converting of neccessary parameters in the
     * outside code. But of course getFileURLFromSystemPath(File) will be a little bit faster
     * then this method ...
     *
     * @param sSystemPath
     *          represent the file in system notation
     *
     * @return [String]
     *          a file url which represent the given system path
     */
    public static java.lang.String getFileURLFromSystemPath(java.lang.String sSystemPath)
    {
        return getFileURLFromSystemPath(new java.io.File(sSystemPath));
    }

    // ____________________

    /**
     * Does the same as getFileURLFromSystemPath() before ... but uses
     * the given protocol string (e.g."http://") insted of "file:///".
     *
     * @param aSystemPath
     *          represent the file in system notation
     *
     * @param aBasePath
     *          define the base path of the aSystemPath value,
     *          which must be replaced with the value of "sServerPath".
     *
     * @param sServerURL
     *          Will be used to replace sBasePath.
     *
     * @example
     *          System Path = "d:\test\file.txt"
     *          Base Path   = "d:\test"
     *          Server Path = "http://alaska:8000"
     *          => "http://alaska:8000/file.txt"
     *
     * @return [String]
     *          an url which represent the given system path
     *          and uses the given protocol
     */
    public static java.lang.String getURLWithProtocolFromSystemPath(java.io.File     aSystemPath,
                                                                    java.io.File     aBasePath  ,
                                                                    java.lang.String sServerURL )
    {
        System.out.println("TODO: must be adapted to java 1.3 :-(");
        System.exit(-1);

        java.lang.String sFileURL = FileHelper.getFileURLFromSystemPath(aSystemPath);
        java.lang.String sBaseURL = FileHelper.getFileURLFromSystemPath(aBasePath  );

        // cut last '/'!
        if (sBaseURL.lastIndexOf('/')==(sBaseURL.length()-1))
            sBaseURL = sBaseURL.substring(0,sBaseURL.length()-1);

        // cut last '/'!
        if (sServerURL.lastIndexOf('/')==(sServerURL.length()-1))
            sServerURL = sServerURL.substring(0,sServerURL.length()-1);

//TODO_JAVA        java.lang.String sURL = sFileURL.replaceFirst(sBaseURL,sServerURL);
        java.lang.String sURL = null;
        return sURL;
    }

    // ____________________

    /**
     * The same as getURLWithProtocolFromSystemPath() before but uses string parameter instead
     * of a java.io.File types. It exist to supress converting of neccessary parameters in the
     * outside code. But of course getURLWithProtocolFromSystemPath(File,File,String) will be
     * a little bit faster then this method ...
     *
     * @param sSystemPath
     *          represent the file in system notation
     *
     * @param sBasePath
     *          define the base path of the aSystemPath value,
     *          which must be replaced with the value of "sServerPath".
     *
     * @param sServerPath
     *          Will be used to replace sBasePath.
     *
     * @example
     *          System Path = "d:\test\file.txt"
     *          Base Path   = "d:\test"
     *          Server Path = "http://alaska:8000"
     *          => "http://alaska:8000/file.txt"
     *
     * @return [String]
     *          an url which represent the given system path
     *          and uses the given protocol
     */
    public static java.lang.String getURLWithProtocolFromSystemPath(java.lang.String sSystemPath,
                                                                    java.lang.String sBasePath  ,
                                                                    java.lang.String sServerPath)
    {
        return getURLWithProtocolFromSystemPath(new java.io.File(sSystemPath), new java.io.File(sBasePath), sServerPath);
    }

    //_________________________________

    /**
     * Return a list of all available files of a directory.
     * We filter sub directories. All other files
     * are returned. So they can be used for further purposes.
     * One parameter define the start directory,
     * another one enable/disable recursive search into sub directories.
     *
     * @param   aRoot
     *          the start directory, which should be analyzed.
     *
     * @param   bRecursive
     *          enable/disable search in sub directories.
     *
     * @return  [Vector]
     *          a filtered list of java java.io.File objects of all available files
     *          of the start dir (and may of its sub directories).
     */
    public static java.util.Vector getSystemFilesFromDir(java.io.File aRoot     ,
                                                         boolean      bRecursive)
    {
        java.io.File[] lAllFiles = aRoot.listFiles();
        if (lAllFiles == null)
            return null;

        int              c              = lAllFiles.length;
        java.util.Vector lFilteredFiles = new java.util.Vector(c);
        for (int i=0; i<c; ++i)
        {
            // simple files!
            if (lAllFiles[i].isFile())
                lFilteredFiles.add(lAllFiles[i]);
            else
            // recursion?
            if (bRecursive && lAllFiles[i].isDirectory())
            {
                java.util.Vector lSubFiles = FileHelper.getSystemFilesFromDir(lAllFiles[i],bRecursive);
                if (lSubFiles != null)
                {
                    java.util.Enumeration aSnapshot = lSubFiles.elements();
                    while (aSnapshot.hasMoreElements())
                        lFilteredFiles.add(aSnapshot.nextElement());
                }
            }
        }

        return lFilteredFiles;
    }

    //_________________________________
    /** it converts the given name (e.g. an internal type name) to
     *  an usable system file name.
     *
     *  Do so some special characters (e.g. "/") must be replaced with other ones.
     *
     *  @param  sName
     *              the name, which should be analyzed and converted.
     *
     *  @return A valid system file name, which should be similar to the
     *          given name, but does not contain special characters any longer.
     */
    public static java.lang.String convertName2FileName(String sName)
    {
        int    i       = 0;
        int    nLength = sName.length();
        char[] lBuffer = sName.toCharArray();

        java.lang.StringBuffer sNewName = new java.lang.StringBuffer(nLength);
        for (i=0; i<nLength; ++i)
        {
            char c = lBuffer[i];
            if (
                c>=48 && c<=57      // 0-9
                &&
                c>=97 && c<=122     // a-z
                &&
                c>=65 && c<=90      // A-Z
               )
            {
                sNewName.append(c);
            }
            else
            {
                sNewName.append("_");
            }
        }

        return sNewName.toString();
    }

    //___________________________________________

    /** it removes all child nodes of a file system directory.
     *
     *  @param  aDirectory
     *          points to the directory, which should be made empty.
     *
     *  @param  bFilesOnly
     *          force deletion of files only. If its set to TRUE,
     *          no subdirectory will be removed.
     *
     *  @throw  [java.io.IOException]
     *          if some of the child nodes couldn't be removed.
     */
    public static void makeDirectoryEmpty(java.io.File aDirectory,
                                          boolean      bFilesOnly)
        throws java.io.IOException
    {
        if (!aDirectory.isDirectory())
            throw new java.io.FileNotFoundException("\""+aDirectory.toString()+"\" is not a directory.");

        java.io.File[] lChilds = aDirectory.listFiles();
        for (int f=0; f<lChilds.length; ++f)
        {
            if (lChilds[f].isDirectory())
            {
                FileHelper.makeDirectoryEmpty(lChilds[f], bFilesOnly);
                if (!bFilesOnly)
                {
                    if (!lChilds[f].delete())
                        throw new java.io.IOException("\""+lChilds[f].toString()+"\" could not be deleted.");
                }
            }
            else
            {
                if (!lChilds[f].delete())
                    throw new java.io.IOException("\""+lChilds[f].toString()+"\" could not be deleted.");
            }
        }
    }

    //___________________________________________

    /** it try to generate a new file with a unique ID
     *  inside given directory.
     *
     *  Call this method with a directory and a base name for
     *  a file. It will be used to generate a new files inside
     *  the directory. Existing files will be checked and new file
     *  name will be tested till a non existing file name would be found.
     *
     *  @param  aBaseDir
     *          must be a system path
     *          e.g.: "c:\temp"
     *
     *  @param  sBaseName
     *          must be a system file name without extensions.
     *          e.g.: "myfile_"
     *
     *  @param  sExtension
     *          the whished extension.
     *          e.g.: "dat"
     *
     *  @return A valid file object, if an unique file could be created -
     *          Null otherwhise.
     *          e.g.: "c:\temp\myfile_1.dat"
     */
    public static java.io.File createUniqueFile(java.io.File     aBaseDir  ,
                                                java.lang.String sBaseName ,
                                                java.lang.String sExtension)
    {
        java.io.File aBaseFile = new java.io.File(aBaseDir, sBaseName);
        java.io.File aFile     = null;
        long         nr        = 0;
        while (aFile == null && nr < java.lang.Long.MAX_VALUE)
        {
            java.lang.String sFileName = aBaseFile.getPath() + java.lang.String.valueOf(nr) + "." + sExtension;
            aFile = new java.io.File(sFileName);
            if (aFile.exists())
                aFile=null;
            ++nr;
        }
        return aFile;
    }

    //___________________________________________

    /** reads the complete file, using the right encoding,
     *  into the given string buffer.
     *
     *  @param  aFile
     *          must point to a system file, which must exist.
     *          e.g.: "c:\temp\test.txt"
     *                "/tmp/test.txt"
     *
     *  @param  sEncoding
     *          will be used to encode the string content
     *          inside the file.
     *          e.g.: "UTF8"
     *
     *  @param  sBuffer
     *          used to return the file content.
     *
     *  @throw  [IOException]
     *          - if the file couldnt be opened
     *          - if the file does not use the right encoding
     */
    public static void readEncodedBufferFromFile(java.io.File           aFile    ,
                                                 java.lang.String       sEncoding,
                                                 java.lang.StringBuffer sBuffer  )
        throws java.io.IOException
    {
        if (sEncoding.equals("UTF-8Special"))
        {
            FileHelper.readAndCheckUTF8File(aFile,sBuffer);
            return;
        }

        java.io.FileInputStream   aByteStream    = new java.io.FileInputStream(aFile.getAbsolutePath());
        java.io.InputStreamReader aEncodedReader = new java.io.InputStreamReader(aByteStream, sEncoding);
        char[]                    aEncodedBuffer = new char[4096];
        int                       nReadCount     = 0;

        while((nReadCount=aEncodedReader.read(aEncodedBuffer))>0)
            sBuffer.append(aEncodedBuffer, 0, nReadCount);

        aEncodedReader.close();
    }

    //___________________________________________
    private static void logEncodingData(java.lang.StringBuffer sLog         ,
                                        int                    nUTF8        ,
                                        int                    nByteOrg1    ,
                                        int                    nByteOrg2    ,
                                        int                    nByteOrg3    ,
                                        int                    nByteOrg4    ,
                                        int                    nByte1       ,
                                        int                    nByte2       ,
                                        int                    nByte3       ,
                                        int                    nByte4       ,
                                        int                    nEncodingType)
    {
        sLog.append("["+nEncodingType+"]\t");
        sLog.append((int)nUTF8+"\t=");
        sLog.append("\t"+nByteOrg1+"/"+nByte1);
        sLog.append("\t"+nByteOrg2+"/"+nByte2);
        sLog.append("\t"+nByteOrg3+"/"+nByte3);
        sLog.append("\t"+nByteOrg4+"/"+nByte4);
        sLog.append("\n");
    }

    //___________________________________________
    private static char impl_convertBytesToChar(int nByte1, int nByte2, int nByte3, int nByte4)
    {
        return (char)((nByte1*0x40000)+(nByte2*0x1000)+(nByte3*0x40)+nByte4);
    }

    //___________________________________________
    private static int impl_readAndCheckNextByte(byte[] aBuffer    ,
                                                 int    nBufPos    ,
                                                 int    nBufLength ,
                                                 int    nMinRange  ,
                                                 int    nMaxRange  )
        throws java.lang.Exception
    {
        if (nBufPos>=nBufLength)
            throw new java.lang.Exception("impl_readAndCheckNextByte()\nEnd of buffer reached.");

        int nByte = aBuffer[nBufPos] & 0xFF;
        if (
            (nByte < nMinRange) ||
            (nByte > nMaxRange)
           )
        {
            throw new java.lang.Exception("impl_readAndCheckNextByte()\nByte does not fit the specified range.");
        }

        return nByte;
    }

    //___________________________________________
    public static void readAndCheckUTF8File(java.io.File           aFile  ,
                                            java.lang.StringBuffer sBuffer)
        throws java.io.IOException
    {
        java.io.FileInputStream aByteStream     = new java.io.FileInputStream(aFile.getAbsolutePath());
        byte[]                  aBuffer         = new byte[4096];
        int                     nReadCount      = 0;
        int                     nByteOrg_1      = 0;
        int                     nByteOrg_2      = 0;
        int                     nByteOrg_3      = 0;
        int                     nByteOrg_4      = 0;
        int                     nByte_1         = 0;
        int                     nByte_2         = 0;
        int                     nByte_3         = 0;
        int                     nByte_4         = 0;
        char                    nUTF8           = 0;
        int                     i               = 0;
        int                     nEncodingType   = 0;
        java.lang.StringBuffer  sLog            = new java.lang.StringBuffer();

        try
        {

        while((nReadCount=aByteStream.read(aBuffer))>0)
        {
            i=0;
            while (i<nReadCount)
            {
                nByteOrg_1    = 0;
                nByteOrg_2    = 0;
                nByteOrg_3    = 0;
                nByteOrg_4    = 0;
                nByte_1       = 0;
                nByte_2       = 0;
                nByte_3       = 0;
                nByte_4       = 0;
                nUTF8         = 0;
                nEncodingType = 0;

                nByteOrg_1 = aBuffer[i++] & 0xFF;
                nByte_1    = nByteOrg_1;
                /*
                    Table 3-6. Well-Formed UTF-8 Byte Sequences

                    ============================================================================
                    Nr.     Code Points             1st Byte    2nd Byte    3rd Byte    4th Byte
                    ============================================================================
                    01      U+     0..U+    7F      00..7F
                    02      U+    80..U+   7FF      C2..DF      80..BF
                    03      U+   800..U+   FFF      E0          A0..BF      80..BF
                    04      U+  1000..U+  CFFF      E1..EC      80..BF      80..BF
                    05      U+  D000..U+  D7FF      ED          80..9F      80..BF
                    06      U+  E000..U+  FFFF      EE..EF      80..BF      80..BF
                    07      U+ 10000..U+ 3FFFF      F0          90..BF      80..BF      80..BF
                    08      U+ 40000..U+ FFFFF      F1..F3      80..BF      80..BF      80..BF
                    09      U+100000..U+10FFFF      F4          80..8F      80..BF      80..BF
                */
                // ------------------------------------------------------------
                // 01
                // 1 byte: 0xxxxxxx
                // ------------------------------------------------------------
                if (
                    (nByteOrg_1 >= 0x00) &&
                    (nByteOrg_1 <= 0x7F)
                   )
                {
                    nEncodingType = 1;
                    nUTF8         = (char)nByte_1;
                }
                // ------------------------------------------------------------
                // 02
                // 1 byte: 110xxxxx
                // 2 byte: 101xxxxx
                // ------------------------------------------------------------
                else
                if (
                    (nByteOrg_1 >= 0xC2) &&
                    (nByteOrg_1 <= 0xDF)
                   )
                {
                    nEncodingType = 2;
                    nByteOrg_2    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByte_1       = nByteOrg_1-0xC2;
                    nByte_2       = nByteOrg_2-0x80;
                    nUTF8         = FileHelper.impl_convertBytesToChar(0,0,nByte_1, nByte_2);
                }
                // ------------------------------------------------------------
                // 03
                // 1 byte: 11100000
                // 2 byte: 101xxxxx
                // 3 byte: 10xxxxxx
                // ------------------------------------------------------------
                else
                if (nByteOrg_1 == 0xE0)
                {
                    nEncodingType = 3;
                    nByteOrg_2    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0xA0, 0xBF);
                    nByteOrg_3    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByte_2       = nByteOrg_2-0xA0;
                    nByte_3       = nByteOrg_3-0x80;
                    nUTF8         = FileHelper.impl_convertBytesToChar(0,0,nByte_2, nByte_3);
                }
                // ------------------------------------------------------------
                // 04
                // 1 byte: 111xxxxx
                // 2 byte: 10xxxxxx
                // 3 byte: 10xxxxxx
                // ------------------------------------------------------------
                else
                if (
                    (nByteOrg_1 >= 0xE1) &&
                    (nByteOrg_1 <= 0xEC)
                   )
                {
                    nEncodingType = 4;
                    nByteOrg_2    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByteOrg_3    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByte_1       = nByteOrg_1-0xE1;
                    nByte_2       = nByteOrg_2-0x80;
                    nByte_3       = nByteOrg_3-0x80;
                    nUTF8         = FileHelper.impl_convertBytesToChar(0,nByte_1, nByte_2, nByte_3);
                }
                // ------------------------------------------------------------
                // 05
                // 1 byte: 11101101
                // 2 byte: 10xxxxxx
                // 3 byte: 10xxxxxx
                // ------------------------------------------------------------
                else
                if (nByteOrg_1 == 0xED)
                {
                    nEncodingType = 5;
                    nByteOrg_2    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0x9F);
                    nByteOrg_3    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByte_2       = nByteOrg_2-0x80;
                    nByte_3       = nByteOrg_3-0x80;
                    nUTF8         = FileHelper.impl_convertBytesToChar(0,0, nByte_2, nByte_3);
                }
                // ------------------------------------------------------------
                // 06
                // 1 byte: 1110111x
                // 2 byte: 10xxxxxx
                // 3 byte: 10xxxxxx
                // ------------------------------------------------------------
                else
                if (
                    (nByteOrg_1 >= 0xEE) &&
                    (nByteOrg_1 <= 0xEF)
                   )
                {
                    nEncodingType = 6;
                    nByteOrg_2    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByteOrg_3    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByte_1       = nByteOrg_1-0xEE;
                    nByte_2       = nByteOrg_2-0x80;
                    nByte_3       = nByteOrg_3-0x80;
                    nUTF8         = FileHelper.impl_convertBytesToChar(0,nByte_1, nByte_2, nByte_3);
                }
                // ------------------------------------------------------------
                // 07
                // 1 byte: 11110000
                // 2 byte: 1001xxxx
                // 3 byte: 10xxxxxx
                // 4 byte: 10xxxxxx
                // ------------------------------------------------------------
                else
                if (nByteOrg_1 == 0xF0)
                {
                    nEncodingType = 7;
                    nByteOrg_2    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x90, 0xBF);
                    nByteOrg_3    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByteOrg_4    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByte_2       = nByteOrg_2-0x90;
                    nByte_3       = nByteOrg_3-0x80;
                    nByte_4       = nByteOrg_4-0x80;
                    nUTF8         = FileHelper.impl_convertBytesToChar(0, nByte_2, nByte_3, nByte_4);
                }
                // ------------------------------------------------------------
                // 08
                // 1 byte: 111100xx
                // 2 byte: 10xxxxxx
                // 3 byte: 10xxxxxx
                // 3 byte: 10xxxxxx
                // ------------------------------------------------------------
                else
                if (
                    (nByteOrg_1 >= 0xF1) &&
                    (nByteOrg_1 <= 0xF3)
                   )
                {
                    nEncodingType = 8;
                    nByteOrg_2    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByteOrg_3    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByteOrg_4    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByte_1       = nByteOrg_1-0xF1;
                    nByte_2       = nByteOrg_2-0x80;
                    nByte_3       = nByteOrg_3-0x80;
                    nByte_4       = nByteOrg_4-0x80;
                    nUTF8         = FileHelper.impl_convertBytesToChar(nByte_1, nByte_2, nByte_3, nByte_4);
                }
                // ------------------------------------------------------------
                // 09
                // 1 byte: 11110100
                // 2 byte: 10xxxxxx
                // 3 byte: 10xxxxxx
                // 4 byte: 10xxxxxx
                // ------------------------------------------------------------
                else
                if (nByteOrg_1 == 0xF0)
                {
                    nEncodingType = 9;
                    nByteOrg_2    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByteOrg_3    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByteOrg_4    = FileHelper.impl_readAndCheckNextByte(aBuffer, i++, nReadCount, 0x80, 0xBF);
                    nByte_2       = nByteOrg_2-0x80;
                    nByte_3       = nByteOrg_3-0x80;
                    nByte_4       = nByteOrg_4-0x80;
                    nUTF8         = FileHelper.impl_convertBytesToChar(0, nByte_2, nByte_3, nByte_4);
                }
                // wrong encoding ?
                else
                {
                    throw new java.lang.Exception("Non well formed UTF-8 encoding.");
                }

                sBuffer.append(nUTF8);
                // -> DEBUG !
                FileHelper.logEncodingData(sLog, nUTF8, nByteOrg_1, nByteOrg_2, nByteOrg_3, nByteOrg_4, nByte_1, nByte_2, nByte_3, nByte_4, nEncodingType);
                // <- DEBUG !
            }
        }

        }
        catch(java.lang.Throwable ex)
        {
            // -> DEBUG !
            FileHelper.logEncodingData(sLog, nUTF8, nByteOrg_1, nByteOrg_2, nByteOrg_3, nByteOrg_4, nByte_1, nByte_2, nByte_3, nByte_4, nEncodingType);

            java.io.File     aDir  = new java.io.File(aFile.getParent());
            java.lang.String sDump = aFile.getName();
            java.io.File     aDump = FileHelper.createUniqueFile(aDir, sDump, "dump");
            FileHelper.writeEncodedBufferToFile(aDump, "UTF-8", false, sLog);
            // <- DEBUG !

            java.lang.String sMsg = "File '"+aFile.getPath()+"' is not encoded right as UTF-8.";
            throw new java.io.IOException(sMsg);
        }

        aByteStream.close();
    }

    //___________________________________________

    /** writes the given string buffer into the specified file
     *  using the specified encoding.
     *
     *  Further it can be set, if the file should be expanded
     *  or replaced by this new string buffer.
     *
     *  @param  aFile
     *          must point to a system file. It can already exist!
     *          e.g.: "c:\temp\test.txt"
     *                "/tmp/test.txt"
     *
     *  @param  sEncoding
     *          will be used to encode the string content inside the file.
     *          e.g.: "UTF8"
     *
     *  @param  bAppend
     *          specify if an already existing file will be
     *          expanded or replaced.
     *
     *  @param  sBuffer
     *          the new string content for this file.
     */
    public static void writeEncodedBufferToFile(java.io.File           aFile    ,
                                                java.lang.String       sEncoding,
                                                boolean                bAppend  ,
                                                java.lang.StringBuffer sBuffer  )
        throws java.io.IOException
    {
        java.io.FileOutputStream   aByteStream    = new java.io.FileOutputStream(aFile.getAbsolutePath(), bAppend);
        java.io.OutputStreamWriter aEncodedWriter = new java.io.OutputStreamWriter(aByteStream, sEncoding);

        java.lang.String sTemp = sBuffer.toString();
        aEncodedWriter.write(sTemp, 0, sTemp.length());

        aEncodedWriter.flush();
        aEncodedWriter.close();

        if (!aFile.exists())
            throw new java.io.IOException("File \""+aFile.getAbsolutePath()+"\" not written correctly.");
    }
}
