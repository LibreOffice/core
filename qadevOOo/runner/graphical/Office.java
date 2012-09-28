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

import java.util.ArrayList;

public class Office implements IOffice
{
    private ParameterHelper m_aParameterHelper;
    private String m_sDocumentName;
    private String m_sResult;
    private IOffice m_aOffice = null;

    public Office(ParameterHelper _aParam, String _sResult)
    {
        m_aParameterHelper = _aParam;
        m_sResult = _sResult;

        if (_aParam.getReferenceType().toLowerCase().equals("ooo") ||
            _aParam.getReferenceType().toLowerCase().equals("o3") ||
            _aParam.getReferenceType().toLowerCase().equals("ps") ||
            _aParam.getReferenceType().toLowerCase().equals("pdf"))
        {
            m_aOffice = new OpenOfficePostscriptCreator(_aParam, m_sResult);
        }
        else if (_aParam.getReferenceType().toLowerCase().equals("msoffice"))
        {
            m_aOffice = new MSOfficePostscriptCreator(_aParam, m_sResult);
        }
    }


    /**
     * Load a document with an already started Office.
     * @param _sDocumentName
     * @throws graphical.OfficeException
     */
    public void load(String _sDocumentName) throws OfficeException
    {
        m_sDocumentName = _sDocumentName;
        // check if given file is a picture, then do nothing
        String sDocumentSuffix = FileHelper.getSuffix(m_sDocumentName);
        if (sDocumentSuffix.toLowerCase().endsWith(".png") ||
            sDocumentSuffix.toLowerCase().endsWith(".gif") ||
            sDocumentSuffix.toLowerCase().endsWith(".jpg") ||
            sDocumentSuffix.toLowerCase().endsWith(".bmp"))
        {
            throw new OfficeException("The given document is not a document type.");
        }

        // TODO: we should start the office after we know if we really need an Office.
        if (m_aOffice != null)
        {
            if (sDocumentSuffix.toLowerCase().endsWith(".odb"))
            {
                if (m_aParameterHelper.getReferenceType().toLowerCase().equals("msoffice"))
                {
                    // we can't handle .odb with msoffice
                    return;
                }
                // TODO: run through all documents which exists as reports in odb files
                OpenOfficeDatabaseReportExtractor aExtractor = new OpenOfficeDatabaseReportExtractor(m_aParameterHelper);
                ArrayList<String> aList = aExtractor.load(m_sDocumentName);
                if (aList != null)
                {
                    // remove the whole section about the 'name'.odb there are no information we need
                    // we will create a new one.
                    String sIniFile = FileHelper.appendPath(m_sResult, "index.ini");
                    IniFile aIniFile2 = new IniFile(sIniFile);
                    String sSection = FileHelper.getBasename(_sDocumentName); // name of the odb file
                    aIniFile2.removeSection(sSection);
                    aIniFile2.close();

                    for (int i=0; i<aList.size();i++)
                    {
                        String sDocumentName = aList.get(i);
                        m_aOffice.load(sDocumentName);
                        m_aOffice.storeAsPostscript();


                        // foreach Report found in the .odb file, create an entry 'report'<number> in the original <name>.odb Section
                        // so it is possible to run through all reports by the given .odb name
                        IniFile aIniFile = new IniFile(sIniFile);
                        int nFileCount = aIniFile.getIntValue(sSection, "reportcount", 0);
                        String sValue = FileHelper.getBasename(sDocumentName);    // name of the corresponding report
                        aIniFile.insertValue(sSection, "report" + nFileCount, sValue);
                        aIniFile.insertValue(sSection, "reportcount", nFileCount + 1);
                        aIniFile.close();
                    }
                }
                else
                {
                    throw new OfficeException("Can't open the document " + m_sDocumentName);
                }
            }
            else
            {
                m_aOffice.load(_sDocumentName);
            }
        }
    }

    public void storeAsPostscript() throws OfficeException
    {
        if (m_aOffice != null)
        {
            if (m_sDocumentName.endsWith(".odb"))
            {
                // this has already be done by load() for odb files.
            }
            else
            {
                m_aOffice.storeAsPostscript();
            }
        }
    }

    public void start() throws OfficeException
    {
        if (m_aOffice != null)
        {
            m_aOffice.start();
        }
    }

    public void close() throws OfficeException
    {
        if (m_aOffice != null)
        {
            m_aOffice.close();
        }
    }

}
