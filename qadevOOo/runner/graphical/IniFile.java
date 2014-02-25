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

// import java.io.BufferedReader;
import java.io.File;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.Enumeration;

/**
   Helper class to give a simple API to read/write windows like ini files
*/
/* public */ // is only need, if we need this class outside package convwatch
public class IniFile implements Enumeration<String>
{

    /**
     * internal representation of the ini file content.
     * Problem, if ini file changed why other write something difference, we don't realise this.
     */
    private String m_sFilename;
    private ArrayList<String> m_aList;
    boolean m_bListContainUnsavedChanges = false;
    private int m_aEnumerationPos = 0;

    /**
       open a ini file by it's name
       @param _sFilename string a filename, if the file doesn't exist, a new empty ini file will create.
       write back to disk only if there are really changes.
    */
    public IniFile(String _sFilename)
        {
            m_sFilename = _sFilename;
            m_aList = loadLines();
            m_aEnumerationPos = findNextSection(0);
//            if (_sFilename.endsWith(".odb.ps.ini"))
//            {
//                int dummy = 0;
//            }
        }

    public void insertFirstComment(String[] _aList)
        {
            if (m_aList.size() == 0)
            {
                // can only insert if there is nothing else already in the ini file
                for (int i = 0; i < _aList.length; i++)
                {
                    m_aList.add(_aList[i]);
                }
            }
        }

    private ArrayList<String> loadLines()
        {
            File aFile = new File(m_sFilename);
            ArrayList<String> aLines = new ArrayList<String>();
            if (!aFile.exists())
            {
                // GlobalLogWriter.println("couldn't find file '" + m_sFilename + "', will be created.");
                // DebugHelper.exception(BasicErrorCode.SbERR_FILE_NOT_FOUND, "");
                // m_bListContainUnsavedChanges = false;
                return aLines;
            }
            RandomAccessFile aReader = null;
            // BufferedReader aReader;
            try
            {
                aReader = new RandomAccessFile(aFile, "r");
                String aLine = "";
                while (aLine != null)
                {
                    aLine = aReader.readLine();
                    if (aLine != null && aLine.length() > 0)
                    {
                        aLines.add(aLine);
                    }
                }
            }
            catch (java.io.FileNotFoundException fne)
            {
                GlobalLogWriter.println("couldn't open file " + m_sFilename);
                GlobalLogWriter.println("Message: " + fne.getMessage());
                // DebugHelper.exception(BasicErrorCode.SbERR_FILE_NOT_FOUND, "");
            }
            catch (java.io.IOException ie)
            {
                GlobalLogWriter.println("Exception occurs while reading from file " + m_sFilename);
                GlobalLogWriter.println("Message: " + ie.getMessage());
                // DebugHelper.exception(BasicErrorCode.SbERR_INTERNAL_ERROR, ie.getMessage());
            }
            try
            {
                aReader.close();
            }
            catch (java.io.IOException ie)
            {
                GlobalLogWriter.println("Couldn't close file " + m_sFilename);
                GlobalLogWriter.println("Message: " + ie.getMessage());
                // DebugHelper.exception(BasicErrorCode.SbERR_INTERNAL_ERROR, ie.getMessage());
            }
            return aLines;
        }

    /**
     * @return true, if the ini file contain some readable data
     */
    public boolean is()
        {
            return m_aList.size() > 1 ? true : false;
        }

    /**
     * Check if a given Section and Key exists in the ini file
     * @param _sSectionName
     * @param _sKey
     * @return true if the given Section, Key exists, now you can get the value
     */
    public boolean hasValue(String _sSectionName, String _sKey)
        {
            int n = findKey(_sSectionName, _sKey);
            if (n > 0)
            {
                return true;
            }
            return false;
        }


    private boolean isRemark(String _sLine)
        {
            if (((_sLine.length() < 2)) ||
                (_sLine.startsWith("#")) ||
                (_sLine.startsWith(";")))
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

    private String sectionToString(String _sSectionName)
        {
            String sKeyName = _sSectionName;
            if (sKeyName.startsWith("[") &&
                sKeyName.endsWith("]"))
            {
                sKeyName = sKeyName.substring(1, sKeyName.length() - 1);
            }
            return sKeyName;
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
            for (i = 0; i < m_aList.size(); i++)
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

    /**
     * Checks if a given section exists in the ini file
     * @param _sSection
     * @return true if the given _sSection was found
     */
    public boolean hasSection(String _sSection)
        {
            int i = findSection(_sSection);
            if (i == -1)
            {
                return false;
            }
            return true;
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
            for (int j = _nSectionIndex + 1; j < m_aList.size(); j++)
            {
                String sLine = getItem(j).trim();

                if (isRemark(sLine))
                {
                    continue;
                }
                if (sLine.startsWith("[") /* && sLine.endsWith("]") */)
                {
                    // TODO: due to the fact we would like to insert an empty line before new sections
                    // TODO: we should check if we are in an empty line and if, go back one line.

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

    // i must be the index in the list, where the well known section starts
    private int findLastKnownKeyIndex(int _nSectionIndex, String _sKey)
        {
            _sKey = toLowerIfNeed(_sKey);
            int i = _nSectionIndex + 1;
            for (int j = i; j < m_aList.size(); j++)
            {
                String sLine = getItem(j).trim();

                if (isRemark(sLine))
                {
                    continue;
                }

                if (sLine.startsWith("[") /* && sLine.endsWith("]") */)
                {
                    // found end.
                    return j;
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
            return i;
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
                sLine.substring(0, nEqual).trim();
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
    // private int m_nCurrentPosition;
    // private String m_sOldKey;
    public String getValue(String _sSection, String _sKey)
        {
            String sValue = "";
            int m_nCurrentPosition = findKey(_sSection, _sKey);
            if (m_nCurrentPosition == -1)
            {
                // Section not found, therefore the value can't exist
                return "";
            }

            // m_sOldKey = _sKey;
            sValue = getValue(m_nCurrentPosition);

            return sValue;
        }

//    private String getNextValue()
//    {
//        if (m_nCurrentPosition >= 0)
//        {
//            ++m_nCurrentPosition;
//            String sValue = getValue(m_nCurrentPosition);
//            return sValue;
//        }
//        return "";
//    }
    /**
     * Returns the value at Section, Key converted to an integer
     * Check with hasValue(Section, Key) to check before you get into trouble.
     * @param _sSection
     * @param _sKey
     * @param _nDefault if there is a problem, key not found... this value will return
     * @return
     */
    public int getIntValue(String _sSection, String _sKey, int _nDefault)
        {
            String sValue = getValue(_sSection, _sKey);
            int nValue = _nDefault;
            if (sValue.length() > 0)
            {
                try
                {
                    nValue = Integer.valueOf(sValue).intValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    GlobalLogWriter.println("IniFile.getIntValue(): Caught a number format exception, return the default value.");
                }
            }
            return nValue;
        }

    public void close()
        {
            store();
        }

    /**
       write back the ini file to the disk, only if there exist changes
       * @deprecated use close() instead!
       */

    // TODO: make private
    public void store()
        {
            if (m_bListContainUnsavedChanges == false)
            {
                // nothing has changed, so no need to store
                return;
            }

            File aFile = new File(m_sFilename);
            if (aFile.exists())
            {
                // System.out.println("couldn't find file " + m_sFilename);
                // TODO: little bit unsafe here, first rename, after write is complete, delete the old.
                aFile.delete();
                if (aFile.exists())
                {
                    GlobalLogWriter.println("Couldn't delete the file " + m_sFilename);
                    return;
                    // DebugHelper.exception(BasicErrorCode.SbERR_INTERNAL_ERROR, "Couldn't delete the file " + m_sFilename);
                }
            }
            // if (! aFile.canWrite())
            // {
            //    System.out.println("Couldn't write to file " + m_sFilename);
            //    DebugHelper.exception(BasicErrorCode.SbERR_INTERNAL_ERROR, "");
            // }
            try
            {
                RandomAccessFile aWriter = new RandomAccessFile(aFile, "rw");
                for (int i = 0; i < m_aList.size(); i++)
                {
                    String sLine = getItem(i);
                    if (sLine.startsWith("["))
                    {
                        // write an extra empty line before next section.
                        aWriter.writeByte('\n');
                    }
                    aWriter.writeBytes(sLine);
                    aWriter.writeByte('\n');
                }
                aWriter.close();
            }
            catch (java.io.FileNotFoundException fne)
            {
                GlobalLogWriter.println("couldn't open file for writing " + m_sFilename);
                GlobalLogWriter.println("Message: " + fne.getMessage());
                // DebugHelper.exception(BasicErrorCode.SbERR_FILE_NOT_FOUND, "");
            }
            catch (java.io.IOException ie)
            {
                GlobalLogWriter.println("Exception occurs while writing to file " + m_sFilename);
                GlobalLogWriter.println("Message: " + ie.getMessage());
                // DebugHelper.exception(BasicErrorCode.SbERR_INTERNAL_ERROR, ie.getMessage());
            }
        }

    public void insertValue(String _sSection, String _sKey, int _nValue)
        {
            insertValue(_sSection, _sKey, String.valueOf(_nValue));
        }

    public void insertValue(String _sSection, String _sKey, long _nValue)
        {
            insertValue(_sSection, _sKey, String.valueOf(_nValue));
        }

    /**
       insert a value
       there are 3 cases
       1. section doesn't exist, goto end and insert a new section, insert a new key value pair
       2. section exist but key not, search section, search key, if key is -1 get last known key position and insert new key value pair there
       3. section exist and key exist, remove the old key and insert the key value pair at the same position
     * @param _sSection
     * @param _sKey
     * @param _sValue
     */
    public void insertValue(String _sSection, String _sKey, String _sValue)
        {
            int i = findSection(_sSection);
            if (i == -1)
            {
                // case 1: section doesn't exist
                String sFindSection = buildSectionName(_sSection);

                // TODO: before create a new Section, insert a empty line
                m_aList.add(sFindSection);
                if (_sKey.length() > 0)
                {
                    String sKeyValuePair = _sKey + "=" + _sValue;
                    m_aList.add(sKeyValuePair);
                }
                m_bListContainUnsavedChanges = true;
                return;
            }
            int j = findKeyFromKnownSection(i, _sKey);
            if (j == -1)
            {
                // case 2: section exist, but not the key
                j = findLastKnownKeyIndex(i, _sKey);
                if (_sKey.length() > 0)
                {
                    String sKeyValuePair = _sKey + "=" + _sValue;
                    m_aList.add(j, sKeyValuePair);
                    m_bListContainUnsavedChanges = true;
                }
                return;
            }
            else
            {
                // case 3: section exist, and also the key
                String sKeyValuePair = _sKey + "=" + _sValue;
                m_aList.set(j, sKeyValuePair);
                m_bListContainUnsavedChanges = true;
            }
        }

    // String replaceEvaluatedValue(String _sSection, String _sValue)
    //     {
    //         String sValue = _sValue;
    //         int nIndex = 0;
    //         while (( nIndex = sValue.indexOf("$(", nIndex)) >= 0)
    //         {
    //             int nNextIndex = sValue.indexOf(")", nIndex);
    //             if (nNextIndex >= 0)
    //             {
    //                 String sKey = sValue.substring(nIndex + 2, nNextIndex);
    //                 String sNewValue = getValue(_sSection, sKey);
    //                 if (sNewValue != null && sNewValue.length() > 0)
    //                 {
    //                     String sRegexpKey = "\\$\\(" + sKey + "\\)";
    //                     sValue = sValue.replaceAll(sRegexpKey, sNewValue);
    //                 }
    //                 nIndex = nNextIndex;
    //             }
    //             else
    //             {
    //                 nIndex += 2;
    //             }
    //         }
    //         return sValue;
    //     }


    // public String getLocalEvaluatedValue(String _sSection, String _sKey)
    //     {
    //         String sValue = getValue(_sSection, _sKey);
    //         sValue = replaceEvaluatedValue(_sSection, sValue);
    //         return sValue;
    //     }



    // this is a special behaviour.
    // public String getGlobalLocalEvaluatedValue(String _sSection, String _sKey)
    //     {
    //         String sGlobalValue = getKey("global", _sKey);
    //         String sLocalValue = getKey(_sSection, _sKey);
    //         if (sLocalValue.length() == 0)
    //         {
    //             sGlobalValue = replaceEvaluatedKey(_sSection, sGlobalValue);
    //             sGlobalValue = replaceEvaluatedKey("global", sGlobalValue);
    //             return sGlobalValue;
    //         }
    //         sLocalValue = replaceEvaluatedKey(_sSection, sLocalValue);
    //         sLocalValue = replaceEvaluatedKey("global", sLocalValue);

    //         return sLocalValue;
    //     }
    public void removeSection(String _sSectionToRemove)
        {
            // first, search for the name
            int i = findSection(_sSectionToRemove);
            if (i == -1)
            {
                // Section to remove not found, do nothing.
                return;
            }
            // second, find the next section
            int j = findNextSection(i + 1);
            if (j == -1)
            {
                // if we are at the end, use size() as second section
                j = m_aList.size();
            }
            // remove all between first and second section
            for (int k = i; k < j; k++)
            {
                m_aList.remove(i);
            }
            // mark the list as changed
            m_bListContainUnsavedChanges = true;
        }

    /**
     * some tests for this class
     */
//    public static void main(String[] args)
//        {
//            String sTempFile = System.getProperty("java.io.tmpdir");
//            sTempFile += "inifile";


//            IniFile aIniFile = new IniFile(sTempFile);
//            String sValue = aIniFile.getValue("Section", "Key");
//            // insert a new value to a already exist section
//            aIniFile.insertValue("Section", "Key2", "a new value in a existing section");
//            // replace a value
//            aIniFile.insertValue("Section", "Key", "replaced value");
//            // create a new value
//            aIniFile.insertValue("New Section", "Key", "a new key value pair");
//            aIniFile.insertValue("New Section", "Key2", "a new second key value pair");

//            String sValue2 = aIniFile.getValue("Section2", "Key");

//            aIniFile.removeSection("Section");
//            aIniFile.removeSection("New Section");

//            aIniFile.close();
//        }

    /**
     * Enumeration Interface
     * @return true, if there are more Key values
     */
    public boolean hasMoreElements()
        {
            if (m_aEnumerationPos >= 0 &&
                m_aEnumerationPos < m_aList.size())
            {
                return true;
            }
            return false;
        }

    /**
     * Find the next line, which starts with '['
     * @param i start position
     * @return the line where '[' found or -1
     */
    private int findNextSection(int i)
        {
            if (i >= 0)
            {
                while (i < m_aList.size())
                {
                    String sLine =  m_aList.get(i);
                    if (sLine.startsWith("["))
                    {
                        return i;
                    }
                    i++;
                }
            }
            return -1;
        }

    /**
     * Enumeration Interface
     * @return a key without the enveloped '[' ']'
     */
    public String nextElement()
        {
            int nLineWithSection = findNextSection(m_aEnumerationPos);
            if (nLineWithSection != -1)
            {
                String sSection =  m_aList.get(nLineWithSection);
                m_aEnumerationPos = findNextSection(nLineWithSection + 1);
                sSection = sectionToString(sSection);
                return sSection;
            }
            else
            {
                m_aEnumerationPos = m_aList.size();
            }
            return null;
        }

    /**
     * Helper to count the occurrence of Sections
     * @return returns the count of '^['.*']$' Elements
     */
    public int getElementCount()
        {
            int nCount = 0;
            int nPosition = 0;
            while ((nPosition = findNextSection(nPosition)) != -1)
            {
                nCount++;
                nPosition++;
            }
            return nCount;
        }
}

