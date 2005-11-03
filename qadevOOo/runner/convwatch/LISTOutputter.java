/*************************************************************************
 *
 *  $RCSfile: LISTOutputter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2005-11-03 14:23:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *
 ************************************************************************/

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
                System.out.println("ERROR: Can't create LIST Outputter");
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
