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
package com.sun.star.wizards.report;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.db.RecordParser;

public class ReportImplementationHelper
{

    private RecordParser m_aCurDBMetaData;
    private XMultiServiceFactory m_xMSF;
    protected int m_nDefaultPageOrientation;

    /**
     * @param _aMSF
     * @param _nOrientation 
     */
    public ReportImplementationHelper(XMultiServiceFactory _aMSF, int _nOrientation)
    {
        m_xMSF = _aMSF;
        m_nDefaultPageOrientation = _nOrientation;
    }

    public RecordParser getRecordParser()
    {
        if (m_aCurDBMetaData == null)
        {
            m_aCurDBMetaData = new RecordParser(getMSF());
        }
        return m_aCurDBMetaData;
    }

    public XMultiServiceFactory getMSF()
    {
        return m_xMSF;
    }
}
