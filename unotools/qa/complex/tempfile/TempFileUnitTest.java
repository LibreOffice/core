/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package complex.tempfile;

import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;

/* Document.
 */

public class TempFileUnitTest extends ComplexTestCase {
    private XMultiServiceFactory m_xMSF = null;
    private XSimpleFileAccess m_xSFA = null;

    public String[] getTestMethodNames() {
        return new String[] {
            "ExecuteTest01",
            "ExecuteTest02"};
    }

    public String getTestObjectName() {
        return "TempFileUnitTest";
    }

    public void before() {
        m_xMSF = (XMultiServiceFactory)param.getMSF();
        if ( m_xMSF == null ) {
            failed ( "Cannot create service factory!" );
        }
        try
        {
            Object oSFA = m_xMSF.createInstance( "com.sun.star.ucb.SimpleFileAccess" );
            m_xSFA = ( XSimpleFileAccess )UnoRuntime.queryInterface( XSimpleFileAccess.class,
                    oSFA );
        }
        catch ( Exception e )
        {
            failed ( "Cannot get simple file access! Exception: " + e);
        }
        if ( m_xSFA == null ) {
            failed ( "Cannot get simple file access!" );
        }
    }

    public void after() {
        m_xMSF = null;
        m_xSFA = null;
    }

    public void ExecuteTest01() {
        TempFileTest aTest = new Test01( m_xMSF, m_xSFA, log );
        assure( "Test01 failed!", aTest.test() );
    }

    public void ExecuteTest02() {
        TempFileTest aTest = new Test02( m_xMSF, m_xSFA, log );
        assure( "Test02 failed!", aTest.test() );
    }
};