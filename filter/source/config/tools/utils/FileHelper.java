/*************************************************************************
 *
 *  $RCSfile: FileHelper.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-21 13:24:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
                                        int                    nByte1       ,
                                        int                    nByte2       ,
                                        int                    nByte3       ,
                                        int                    nByte4       ,
                                        int                    nEncodingType)
    {
        sLog.append("'"+nUTF8+"'\t:"+(int)nUTF8+"\t=\t"+nByte1+"\t"+nByte2+"\t"+nByte3+"\t"+nByte4+"\t");
        switch(nEncodingType)
        {
            case 1:
                sLog.append("7-bit ASCII");
                break;
            case 2:
                sLog.append("2 Byte Encoding");
                break;
            case 3:
                sLog.append("3 Byte Encoding");
                break;
            case 4:
                sLog.append("4 Byte Encoding");
                break;
        }
        sLog.append("\n");
    }

    //___________________________________________
    public static void readAndCheckUTF8File(java.io.File           aFile  ,
                                            java.lang.StringBuffer sBuffer)
        throws java.io.IOException
    {
        java.io.FileInputStream aByteStream     = new java.io.FileInputStream(aFile.getAbsolutePath());
        byte[]                  aBuffer         = new byte[4096];
        int                     nReadCount      = 0;
        int                     nByte_1         = 0;
        int                     nByte_2         = 0;
        int                     nByte_3         = 0;
        int                     nByte_4         = 0;
        int                     nByte_5         = 0;
        int                     nByte_6         = 0;
        char                    nUTF8           = 0;
        int                     i               = 0;
        boolean                 bError          = false;
        int                     nEncodingType   = 0;
        java.lang.StringBuffer  sLog            = new java.lang.StringBuffer();

        while((nReadCount=aByteStream.read(aBuffer))>0)
        {
            i=0;
            while (i<nReadCount)
            {
                nByte_1       = 0;
                nByte_2       = 0;
                nByte_3       = 0;
                nByte_4       = 0;
                nByte_5       = 0;
                nByte_6       = 0;
                nUTF8         = 0;
                nEncodingType = 0;
                bError        = false;

                nByte_1 = aBuffer[i++] & 0xFF;

                // range: U0000 - U007F / 0xxxxxxx = 1 Byte Encoding = 7-Bit ASCII !
                if (nByte_1 < 0x80)
                {
                    nEncodingType = 1;
                    nUTF8         = (char)nByte_1;
                }

                // range: U10000 - U10FFFF / 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx = 4 Byte Encoding
                else
                if ((nByte_1 & 0xF0) == 0xF0)
                {
                    nEncodingType = 4;
                    if (i+2>=nReadCount)
                    {
                        bError = true;
                        break;
                    }
                    nByte_2  = aBuffer[i++] & 0xFF;
                    nByte_3  = aBuffer[i++] & 0xFF;
                    nByte_4  = aBuffer[i++] & 0xFF;
                    if (
                        ((nByte_2 & 0x80) != 0x80) ||
                        ((nByte_3 & 0x80) != 0x80) ||
                        ((nByte_4 & 0x80) != 0x80)
                       )
                    {
                        bError = true;
                        break;
                    }
                    nByte_1 &= 0xF;
                    nByte_2 &= 0x3F;
                    nByte_3 &= 0x3F;
                    nByte_4 &= 0x3F;
                    nUTF8    = (char)((nByte_1*0x40000)+(nByte_2*0x1000)+(nByte_3*0x40)+nByte_4);
                }

                // range: U0800 - UFFFF / 1110xxxx 10xxxxxx 10xxxxxx = 3 Byte Encoding
                else
                if ((nByte_1 & 0xE0) == 0xE0)
                {
                    nEncodingType = 3;
                    if (i+1>=nReadCount)
                    {
                        bError = true;
                        break;
                    }
                    nByte_2 = aBuffer[i++] & 0xFF;
                    nByte_3 = aBuffer[i++] & 0xFF;
                    if (
                        ((nByte_2 & 0x80) != 0x80) ||
                        ((nByte_3 & 0x80) != 0x80)
                       )
                    {
                        bError = true;
                        break;
                    }
                    nByte_1 &= 0xF;
                    nByte_2 &= 0x3F;
                    nByte_3 &= 0x3F;
                    nUTF8    = (char)((nByte_1*0x1000)+(nByte_2*0x40)+nByte_3);
                }

                // range: U0080 - U07FF / 110xxxxx 10xxxxxx = 2 Byte Encoding
                else
                if ((nByte_1 & 0xC0) == 0xC0)
                {
                    nEncodingType = 2;
                    if (i>=nReadCount)
                    {
                        bError = true;
                        break;
                    }
                    nByte_2 = aBuffer[i++] & 0xFF;
                    if ((nByte_2 & 0x80) != 0x80)
                    {
                        bError = true;
                        break;
                    }
                    nByte_1 &= 0xF;
                    nByte_2 &= 0x3F;
                    nUTF8    = (char)((nByte_1*0x40)+nByte_2);
                }

                // wrong encoding ?
                else
                {
                    bError = true;
                    break;
                }

                sBuffer.append(nUTF8);
                // -> DEBUG !
                FileHelper.logEncodingData(sLog, nUTF8, nByte_1, nByte_2, nByte_3, nByte_4, nEncodingType);
                // <- DEBUG !
            }

            if (bError)
            {
                // -> DEBUG !
                FileHelper.logEncodingData(sLog, nUTF8, nByte_1, nByte_2, nByte_3, nByte_4, nEncodingType);

                java.io.File     aDir  = new java.io.File(aFile.getParent());
                java.lang.String sDump = aFile.getName();
                java.io.File     aDump = FileHelper.createUniqueFile(aDir, sDump, "dump");
                FileHelper.writeEncodedBufferToFile(aDump, "UTF-8", false, sLog);
                // <- DEBUG !

                java.lang.String sMsg = "";
                sMsg += "\n================================================================================================================\n";
                sMsg += "Wrong encoding detected in file '"+aFile.getPath()+"'!\n";
                sMsg += "It's not UTF8 ... Please analyse dump file \""+aDump.getPath()+"\".\n";

                throw new java.io.IOException(sMsg);
            }
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
