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
import java.io.FileWriter;

public class INIOutputter
{
    FileWriter m_aOut;
    String m_sFilename;
    String m_sNamePrefix;              // the HTML files used a suffix to build it's right name

    /**
     * ls is the current line separator (carridge return)
     */
    String ls;

    public static INIOutputter create( String _sOutputPath, String _sHTMLFilename, String _sNamePrefix, String _sTitle )
        {
            FileHelper.makeDirectories("", _sOutputPath);
            INIOutputter a = new INIOutputter();
            String fs = System.getProperty("file.separator");
            String sFilename = _sOutputPath + fs + _sHTMLFilename;

            try
            {
                File outputFile = new File(sFilename);
                a.m_aOut = new FileWriter(outputFile.toString());
                a.ls = System.getProperty("line.separator");
            }
            catch (java.io.IOException e)
            {
                e.printStackTrace();
                GlobalLogWriter.get().println("ERROR: Can't create INI Outputter");
                return null;
            }
            a.m_sFilename = sFilename;
            a.m_sNamePrefix = _sNamePrefix;

            return a;
        }
    public String getFilename() {return m_sFilename;}

    public void createHeader()
        {
            try
            {
                m_aOut.write("; This file is automatically created by a convwatch run" + ls);
                m_aOut.write("; " + ls);
                m_aOut.write("; If you see this file in a browser you may have forgotten to set the follows in the property file" + ls);
                m_aOut.write("; " + PropertyName.DOC_COMPARATOR_HTML_OUTPUT_PREFIX + "=http://lla-1.germany/gfxcmp/cw.php?inifile=" + ls);
                m_aOut.write("; Please check the documentation if you got confused." + ls);
                m_aOut.write("; " + ls);
                m_aOut.write("; " + ls);
            }
            catch (java.io.IOException e)
            {
            }
        }

    public void writeSection(String _sSectionName)
        {
            try
            {
                m_aOut.write("[" + _sSectionName + "]" + ls);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

    public void writeValue(String _sName, String _sValue)
        {
            try
            {
                m_aOut.write(_sName + "=" + _sValue + ls);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

    public void startSection(int _nNumber)
        {
            writeSection( "page" + String.valueOf(_nNumber));
        }

    public void close()
        {
            try
            {
                m_aOut.flush();
                m_aOut.close();
            }
            catch (java.io.IOException e)
            {
            }
        }

    public void checkLine(StatusHelper _aStatus, boolean _bCurrentResult)
        {
            try
            {
                m_aOut.write( "oldgfx=" + _aStatus.m_sOldGfx + ls);
                m_aOut.write( "newgfx=" + _aStatus.m_sNewGfx + ls);
                m_aOut.write( "diffgfx=" + _aStatus.m_sDiffGfx + ls);

                String sPercent = String.valueOf(_aStatus.nPercent) + "%";
                if (_aStatus.nPercent > 0 && _aStatus.nPercent < 5)
                {
                    sPercent += " (less 5% is ok)";
                }
                m_aOut.write("percent=" +  sPercent + ls);

                if (_aStatus.m_sDiff_BM_Gfx == null)
                {
                    m_aOut.write("BM=false" + ls);
                }
                else
                {
                    m_aOut.write("BM=true" + ls);
                    m_aOut.write( "old_BM_gfx=" + _aStatus.m_sOld_BM_Gfx + ls);
                    m_aOut.write( "new_BM_gfx=" + _aStatus.m_sNew_BM_Gfx + ls);
                    m_aOut.write( "diff_BM_gfx=" + _aStatus.m_sDiff_BM_Gfx + ls);

                    String sPercent2 = String.valueOf(_aStatus.nPercent2) + "%";
                    if (_aStatus.nPercent2 > 0 && _aStatus.nPercent2 < 5)
                    {
                        sPercent2 += " (less 5% is ok)";
                    }
                    m_aOut.write("percent2=" +  sPercent2 + ls);
                }

                writeResult(_bCurrentResult);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

    void writeResult(boolean _bCurrentResult) throws java.io.IOException
        {
            // is the check positiv, in a defined range
            if (_bCurrentResult)
            {
                m_aOut.write("result=YES" + ls);
            }
            else
            {
                m_aOut.write("result=NO" + ls);
            }
        }

    public void checkDiffDiffLine(StatusHelper _aStatus, boolean _bCurrentResult)
        {
            try
            {
                m_aOut.write( "oldgfx=" + _aStatus.m_sOldGfx + ls);
                m_aOut.write( "newgfx=" + _aStatus.m_sNewGfx + ls);
                m_aOut.write( "diffgfx=" + _aStatus.m_sDiffGfx + ls);

                String sPercent = String.valueOf(_aStatus.nPercent) + "%";
                m_aOut.write("percent=" + sPercent + ls);

                // is the check positiv, in a defined range
                writeResult(_bCurrentResult);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

}
