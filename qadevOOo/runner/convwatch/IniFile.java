/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IniFile.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:14:12 $
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
 ************************************************************************/

package convwatch;

import java.io.File;
import java.io.RandomAccessFile;
import java.util.ArrayList;

/**
 * Helper class to get really simple and direct access to an ini file
 */
/* public */ // is only need, if we need this class outside package convwatch
class IniFile
{
    /**
     * internal representation of the ini file content.
     * Problem, if ini file changed why other write something difference, we don't realise this.
     */
    ArrayList m_aList;

    /**
     * @param sFilename is the whole system path name to a ini file. Which will read by this class.
     */
    public IniFile(String sFilename)
        {
            m_aList = getLines(sFilename);
        }

    ArrayList getLines(String filename)
        {
            File the_file = new File(filename);
            ArrayList the_lines = new ArrayList();
            if (! the_file.exists())
            {
                System.out.println(getClass().getName() + " couldn't find file " + filename);
                return the_lines;
            }
            RandomAccessFile the_reader = null;
            try
            {
                the_reader = new RandomAccessFile(the_file,"r");
                String aLine = "";
                while (aLine != null)
                {
                    aLine = the_reader.readLine();
                    if (aLine != null)
                    {
                        aLine = aLine.trim();
                        if ( (! (aLine.length() < 2) ) &&
                             (! aLine.startsWith("#")) &&
                             (! aLine.startsWith(";")) )
                        {
                            the_lines.add(aLine);
                        }
                    }
                }

            }
            catch (java.io.FileNotFoundException fne)
            {
                System.out.println(getClass().getName() + " couldn't open file " + filename);
                System.out.println("Message: " + fne.getMessage());
            }
            catch (java.io.IOException ie)
            {
                System.out.println(getClass().getName() + "Exception while reading file " + filename);
                System.out.println("Message: " + ie.getMessage());
            }
            try
            {
                the_reader.close();
            }
            catch (java.io.IOException ie)
            {
                System.out.println("Couldn't close file " + filename);
                System.out.println("Message: " + ie.getMessage());
            }
            return the_lines;
        }

    /**
     * @return true, if the ini file contain some readable data
     */
    public boolean is()
        {
            return m_aList.size() > 0 ? true : false;
        }

    // -----------------------------------------------------------------------------

    /**
     * @param _sSection is a string to a section which is first search in the ini file
     * @param _sKey     is a string to a key in the given section
     * @return the value of the ini file like: [_sSection] _sKey =Value, the value is trimmed.
     *
     * If nothing is found, empty string is returned. No exception.
     */
    public String getKey(String _sSection, String _sKey)
        {
            String sValue = "";
            // ----------- find _sSection ---------------
            int i = 0;
            if (_sSection.length() > 0)
            {
            String sFindSection = "[" + _sSection + "]";
            for (i=0; i<m_aList.size();i++)
            {
                String sLine = (String) m_aList.get(i);
                if (sLine.startsWith(sFindSection))
                {
                        i++;
                    break;
                }
            }
            }

            // ------------ find key --------------------
            for (int j=i; j<m_aList.size();j++)
            {
                String sLine = (String) m_aList.get(j);

                if (sLine.startsWith("[") && sLine.endsWith("]"))
                {
                    // found end.
                    break;
                }

                int nEqual = sLine.indexOf("=");
                if (nEqual >= 0)
                {
                    String sKey = sLine.substring(0, nEqual).trim();
                    if (sKey.equals(_sKey))
                    {
                        sValue = sLine.substring(nEqual + 1).trim();
                        break;
                    }
                }
            }
            return sValue;
        }
}
