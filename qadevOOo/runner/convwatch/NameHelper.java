/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NameHelper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:14:43 $
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
            System.out.println("         Outputpath: " + m_sOutputPath);
            System.out.println("relative Outputpath: " + m_sRelativeOutputPath);
            System.out.println("               Name: " + m_sNameNoSuffix);
            System.out.println("             Suffix: " + m_sSuffix);

        }

}
