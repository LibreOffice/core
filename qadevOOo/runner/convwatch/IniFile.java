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
import java.io.RandomAccessFile;
import java.util.ArrayList;

/**
   Helper class to give a simple API to read/write windows like ini files
 */
/* public */ // is only need, if we need this class outside package convwatch
class IniFile
{
    /**
     * internal representation of the ini file content.
     * Problem, if ini file changed why other write something difference, we don't realise this.
     */
    private String m_sFilename;
    private ArrayList<String> m_aList;
    private boolean m_bListContainUnsavedChanges = false;

    /**
       open a ini file by its name
       @param _sFilename string a filename, if the file doesn't exist, a new empty ini file will create.
       write back to disk only if there are really changes.
     */
    public IniFile(String _sFilename)
        {
            m_sFilename = _sFilename;
            m_aList = loadLines();
        }

    private ArrayList<String> loadLines()
        {
            File aFile = new File(m_sFilename);
            ArrayList<String> aLines = new ArrayList<String>();
            if (! aFile.exists())
            {
                GlobalLogWriter.get().println("couldn't find file " + m_sFilename);
                return aLines;
            }
            RandomAccessFile aReader = null;
            try
            {
                aReader = new RandomAccessFile(aFile,"r");
                String aLine = "";
                while (aLine != null)
                {
                    aLine = aReader.readLine();
                    if (aLine != null)
                    {
                        aLines.add(aLine);
                        }
                    }
                }
            catch (java.io.FileNotFoundException fne)
            {
                GlobalLogWriter.get().println("couldn't open file " + m_sFilename);
                GlobalLogWriter.get().println("Message: " + fne.getMessage());
            }
            catch (java.io.IOException ie)
            {
                GlobalLogWriter.get().println("Exception occurs while reading from file " + m_sFilename);
                GlobalLogWriter.get().println("Message: " + ie.getMessage());
            }
            try
            {
                aReader.close();
            }
            catch (java.io.IOException ie)
            {
                GlobalLogWriter.get().println("Couldn't close file " + m_sFilename);
                GlobalLogWriter.get().println("Message: " + ie.getMessage());
            }
            return aLines;
        }

    /**
     * @return true, if the ini file contain some readable data
     */
    public boolean is()
        {
            return m_aList.size() > 1;
        }



    private boolean isRemark(String _sLine)
        {
            if ( ((_sLine.length() < 2) ) ||
                 ( _sLine.startsWith("#")) ||
                 ( _sLine.startsWith(";")) )
            {
                return true;
            }
            return false;
        }

    private String getItem(int i)
        {
            return m_aList.get(i);
        }

    private String buildSectionName(String _sSectionName)
        {
            String sFindSection = "[" + _sSectionName + "]";
            return sFindSection;
        }
    private String toLowerIfNeed(String _sName)
        {
            return _sName.toLowerCase();
        }

    // return the number where this section starts
    private int findSection(String _sSection)
        {
            String sFindSection = toLowerIfNeed(buildSectionName(_sSection));
            // ----------- find _sSection ---------------
            int i;
            for (i=0; i<m_aList.size();i++)
            {
                String sLine = toLowerIfNeed(getItem(i).trim());
                if (isRemark(sLine))
            {
                    continue;
                }
                if (sFindSection.equals("[]"))
                {
                    // special case, empty Section.
                    return i - 1;
                }
                if (sLine.startsWith(sFindSection))
                {
                    return i;
                }
            }
            return -1;
        }

    // return the line number, where the key is found.
    private int findKey(String _sSection, String _sKey)
        {
            int i = findSection(_sSection);
            if (i == -1)
            {
                // Section not found, therefore the value can't exist
                return -1;
            }
            return findKeyFromKnownSection(i, _sKey);
        }

    // i must be the index in the list, where the well known section starts
    private int findKeyFromKnownSection(int _nSectionIndex, String _sKey)
        {
            _sKey = toLowerIfNeed(_sKey);
            for (int j=_nSectionIndex + 1; j<m_aList.size();j++)
            {
                String sLine = getItem(j).trim();

                if (isRemark(sLine))
                {
                    continue;
                }

                if (sLine.startsWith("["))
                {
                    // found end.
                    break;
                }

                int nEqual = sLine.indexOf("=");
                if (nEqual >= 0)
                {
                    String sKey = toLowerIfNeed(sLine.substring(0, nEqual).trim());
                    if (sKey.equals(_sKey))
                    {
                        return j;
                    }
            }
            }
            return -1;
        }

    private String getValue(int _nIndex)
        {
            String sLine = getItem(_nIndex).trim();
            if (isRemark(sLine))
            {
                return "";
            }
            int nEqual = sLine.indexOf("=");
            if (nEqual >= 0)
            {
                String sValue = sLine.substring(nEqual + 1).trim();
                return sValue;
                    }
            return "";
                }

    /**
       @param _sSection string
       @param _sKey string
       @return the value found in the inifile which is given by the section and key parameter
    */
    public String getValue(String _sSection, String _sKey)
        {
            String sValue = "";
            int i = findKey(_sSection, _sKey);
            if (i == -1)
            {
                // Section not found, therefore the value can't exist
                return "";
            }

            sValue = getValue(i);

            return sValue;
        }







}
