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

/**
 * Helper class to hold a Filename or a FileURL
 * Something like File in Java,
 * with some more extensions direct to ConvWatch and it's name conventions
 *
 *
 */
package convwatch;

import helper.URLHelper;
import convwatch.FileHelper;
import helper.StringHelper;
import util.utils;

interface Filenamer
{
    public String getSuffix();
    public String getFileURL();
    public String getAbsoluteSystemFilename();
    public String getFilename();
    public String getSystemPath();
}



abstract class FilenameHelper_impl implements Filenamer
{
    String fs;                                   // file separator like '/'
    String m_sPath;
    String m_sFilename;
    String m_sSuffix;
    int    m_nNumber = 0;

    public String getNumber()
        {
            return StringHelper.createValueString(m_nNumber, 4);
        }
    public void setNumber(int _n)
        {
            m_nNumber = _n;
        }
    void initMember()
        {
            fs = System.getProperty("file.separator");
        }

        /**
         * initialise a FilenameHelper_impl with a complete filename.
         * if the filename starts with 'file:///' it is interpret as a file URL
         *
         */
    public FilenameHelper_impl()
        {
            initMember();
        }

    public void setCompleteFilename(String _sFilename)
        {
            if (_sFilename.startsWith("file:///"))
            {
                _sFilename = FileHelper.getSystemPathFromFileURL(_sFilename);
            }
            _sFilename = utils.replaceAll13(_sFilename, "\\\\", "/");

            String sPath = checkPath(FileHelper.getPath(_sFilename));
            String sFilenameWithSuffix = checkFilename(FileHelper.getBasename(_sFilename));
            String sSuffix = splitSuffix(sFilenameWithSuffix);

            m_sPath = sPath;
            m_sFilename = FileHelper.getNameNoSuffix(sFilenameWithSuffix);
            m_sSuffix = sSuffix;
        }

        /**
         * initialise a FilenameHelper_impl with a path a name and a suffix separately
         */
    public FilenameHelper_impl(String _sPath, String _sName, String _sSuffix)
        {
            initMember();
            _sPath = utils.replaceAll13(_sPath, "\\\\", "/");

            String sPath = checkPath(_sPath);
            String sFilename = checkFilename(_sName);
            String sSuffix = checkSuffix(_sSuffix);

            m_sPath = sPath;
            m_sFilename = sFilename;
            m_sSuffix = sSuffix;
        }

    /**
     * @return the current path as a OOo path URL
     */
    public String getFileURL()
        {
            String sSystemPath = createAbsoluteFilename();
            String sFileURL = URLHelper.getFileURLFromSystemPath(sSystemPath);
            return sFileURL;
        }


    /**
     * @return the current path as a system path
     */
    public String getAbsoluteSystemFilename()
        {
            String sSystemFilename = createAbsoluteFilename();
            sSystemFilename = utils.replaceAll13(sSystemFilename, "/", fs);
            return sSystemFilename;
        }

    /**
     * @return the filename without it's suffix
     */
    public String getName()
    {
        return m_sFilename;
    }
    /**
     * set only the filename, maybe it's is only a directory.
     */
    public void setName(String _sName)
        {
            m_sFilename = _sName;
        }
    public void setPath(String _sName)
        {
            m_sPath = _sName;
        }

    /**
     * @return a created name
     */
    abstract public String buildName();
        // {
        //    return getName();
        // }

    /**
     * @return the complete filename with it's suffix
     */
    public String getFilename()
        {
            return buildName() + "." + getSuffix();
        }

     /**
      * @return the path as system path
      */
    public String getSystemPath()
    {
        String sSystemPath = m_sPath;
        sSystemPath = utils.replaceAll13(sSystemPath, "/", fs);
        return sSystemPath;
    }
    /**
     * @return true, if current SystemPath is a directory
     */
    public boolean isDirectory()
        {
            return FileHelper.isDir(getSystemPath());
        }

    /**
     * @return true, if the file really exist.
     */
    public boolean exists()
        {
            return FileHelper.exists(createAbsoluteFilename());
        }

    /**
     * @return the current suffix
     */
    public String getSuffix()
        {
            return m_sSuffix;
        }
    /**
     * @return the complete name. Without convert the path separator!
     */
    String createAbsoluteFilename()
        {
            return m_sPath + fs + getFilename();
        }

    /*
     * remove follows 'file separators'
     */
    String checkPath(String _sPath)
        {
            String sPath;
            if (_sPath.endsWith("/") || _sPath.endsWith("\\"))
            {
                sPath = _sPath.substring(0, _sPath.length() - 1);
            }
            else
            {
               sPath = _sPath;
            }
            return sPath;
        }

    String checkFilename(String _sFilename)
        {
            String sFilename;
            if (_sFilename.startsWith("/") || _sFilename.startsWith("\\"))
            {
                sFilename = _sFilename.substring(1);
            }
            else
            {
               sFilename = _sFilename;
            }
            return sFilename;
        }

    String checkSuffix(String _sSuffix)
        {
            String sSuffix;
            if (_sSuffix.startsWith("."))
            {
                sSuffix = _sSuffix.substring(1);
            }
            else
            {
                sSuffix = _sSuffix;
            }
            return sSuffix;
        }

    String splitSuffix(String _sName)
        {
            String sSuffix = FileHelper.getSuffix(_sName);
            return checkSuffix(sSuffix);
        }

    public boolean equals(FilenameHelper_impl _aOtherFN)
        {
            String sPath = createAbsoluteFilename();
            String sPathOther = _aOtherFN.createAbsoluteFilename();
            if (sPath.equals(sPathOther))
            {
                return true;
            }
            return false;
        }

}

/**
 * Original filename
 */
class OriginalFilename  extends FilenameHelper_impl
{
    public String buildName()
        {
            return getName();
        }

    public OriginalFilename(){}
    public OriginalFilename(String _path, String _filename, String _ext) { super(_path, _filename, _ext);}
}

/**
 * Reference from original
 */
class OriginalReferenceFilename extends FilenameHelper_impl
{
    public String getSuffix()
        {
            return "prn";
        }
    public String buildName()
        {
            return getName();
        }
    public OriginalReferenceFilename(){}
    public OriginalReferenceFilename(String _path, String _filename, String _ext) { super(_path, _filename, _ext);}
}

/**
 * picture from reference from original
 */
class OriginalReferencePictureFilename extends FilenameHelper_impl
{
    public String getSuffix()
        {
            return "jpg";
        }
    public String buildName()
        {
            return getName() + "-" + getNumber() + "-ref";
        }
    public String getBuildString()
        {
            return getName() + "-" + "%04d" + "-ref";
        }

    public OriginalReferencePictureFilename(){}
    public OriginalReferencePictureFilename(String _path, String _filename, String _ext) { super(_path, _filename, _ext);}
}

/**
 * Reference from OpenOffice.org
 */
class CurrentReferenceFilename extends FilenameHelper_impl
{
    public String getSuffix()
        {
            return "ps";
        }
    public String buildName()
        {
            return getName();
        }

    public CurrentReferenceFilename(){}
    public CurrentReferenceFilename(String _path, String _filename, String _ext) { super(_path, _filename, _ext);}
}

/**
 * picture from reference from OpenOffice.org
 */
class CurrentReferencePictureFilename extends FilenameHelper_impl
{
    public String getSuffix()
        {
            return "jpg";
        }
    public String buildName()
        {
            return getName() + "-" + getNumber() + "-new-ref";
        }
    public String getBuildString()
        {
            return getName() + "-" + "%04d" + "-new-ref";
        }

    public CurrentReferencePictureFilename(){}
    public CurrentReferencePictureFilename(String _path, String _filename, String _ext) { super(_path, _filename, _ext);}
}


public class FilenameHelper
{

    public static void main(String[] args)
        {
            OriginalReferenceFilename d = new OriginalReferenceFilename();
            d.setCompleteFilename("c:\\dir1\\dir2\\name.ext");
            System.out.println("Suffix: " + d.getSuffix());
            System.out.println("Path: " + d.getSystemPath());
            System.out.println("Absolute system path filename: " + d.getAbsoluteSystemFilename());
            System.out.println("URL: " + d.getFileURL());
            System.out.println("Filename: " + d.getFilename());

            new OriginalReferenceFilename("/dir1/dir2/", "name",".ext");
            new OriginalReferenceFilename("/dir1/dir2","name.ext","");
            new OriginalReferenceFilename("/dir1/dir2","/name.ext","");
            new OriginalReferenceFilename("/dir1/dir2","/name",".ext");
            new OriginalReferenceFilename("/dir1/dir2","name","ext");


            // OriginalReferenceFilename b = new OriginalReferenceFilename("c:/dir1/dir2/name.ext");
            // OriginalReferenceFilename c = new OriginalReferenceFilename("file:///dir1/dir2/name.ext");
            // OriginalReferenceFilename e = new OriginalReferenceFilename("c:\\dir1\\dir2\\name");
            // OriginalReferenceFilename f = new OriginalReferenceFilename("c:\\dir1\\dir2");
            // OriginalReferenceFilename g = new OriginalReferenceFilename("c:\\dir1\\dir2\\");
        }
}
