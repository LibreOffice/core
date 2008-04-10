/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: NameHelper.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
