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

public class LISTOutputter
{
    FileWriter m_aOut;
    String m_sFilename;

    /**
     * ls is the current line separator (carridge return)
     */
    String ls;

    public static LISTOutputter create( String _sOutputPath, String _sFilename)
        {
            FileHelper.makeDirectories("", _sOutputPath);
            LISTOutputter a = new LISTOutputter();
            String fs = System.getProperty("file.separator");
            String sFilename = _sOutputPath + fs + _sFilename;

            try
            {
                File outputFile = new File(sFilename);
                a.m_aOut = new FileWriter(outputFile.toString());
                a.ls = System.getProperty("line.separator");
            }
            catch (java.io.IOException e)
            {
                e.printStackTrace();
                GlobalLogWriter.get().println("ERROR: Can't create LIST Outputter");
                return null;
            }
            a.m_sFilename = sFilename;

            return a;
        }
    public String getFilename() {return m_sFilename;}

    public void createHeader()
        {
            try
            {
                m_aOut.write("# This file is automatically created by a convwatch run" + ls);
                m_aOut.write("# " + ls);
            }
            catch (java.io.IOException e)
            {
            }
        }

    public void writeValue(String _sValue)
        {
            try
            {
                m_aOut.write(_sValue + ls);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
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

}
