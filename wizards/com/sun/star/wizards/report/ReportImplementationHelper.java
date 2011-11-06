/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.wizards.report;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.db.RecordParser;

/**
 *
 * @author ll93751
 */
public class ReportImplementationHelper
{

    private RecordParser m_aCurDBMetaData;
    private XMultiServiceFactory m_xMSF;
    protected int m_nDefaultPageOrientation;

    /**
     *
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
