/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportImplementationHelper.java,v $
 *
 * $Revision: 1.2.36.1 $
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
