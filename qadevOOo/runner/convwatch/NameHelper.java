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

/**
 * This container class should help to handle the name of the current document
 */

public class NameHelper
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
