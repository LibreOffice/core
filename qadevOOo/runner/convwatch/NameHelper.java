/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NameHelper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-19 14:20:59 $
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

/**
 * This container class should help to handle the name of the current document
 */

class NameHelper
{
    /**
     *
     */
    String m_sOutputPath;

    /**
     *
     */
    String m_sRelativeOutputPath;

    /**
     *
     */
    String m_sNameNoSuffix;
    String m_sSuffix;

    public NameHelper(String _sOutputPath, String _sRelativeOutputPath, String _sBasename)
        {
            m_sOutputPath = _sOutputPath;
            m_sRelativeOutputPath = _sRelativeOutputPath;
            String sNameNoSuffix = FileHelper.getNameNoSuffix(_sBasename);
            m_sNameNoSuffix = sNameNoSuffix;
            m_sSuffix = FileHelper.getSuffix(_sBasename);
        }
    /**
     * @return the OutputPath
     */
    public String getOutputPath() {return m_sOutputPath;}

    /**
     * @return the relative OutputPath
     */
    public String getRelativePath() {return m_sRelativeOutputPath;}

    /**
     * @return the document name without it's suffix
     */
    public String getName() {return m_sNameNoSuffix;}

    /**
     * @return the document suffix from the original document
     */
    public String getSuffix() {return m_sSuffix;}

    public void print()
        {
            GlobalLogWriter.get().println("         Outputpath: " + m_sOutputPath);
            GlobalLogWriter.get().println("relative Outputpath: " + m_sRelativeOutputPath);
            GlobalLogWriter.get().println("               Name: " + m_sNameNoSuffix);
            GlobalLogWriter.get().println("             Suffix: " + m_sSuffix);

        }

}
