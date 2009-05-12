/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SequenceOutputStreamUnitTest.java,v $
 * $Revision: 1.3 $
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
package complex.sequenceoutputstream;

import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/* Document.
 */

public class SequenceOutputStreamUnitTest extends ComplexTestCase {
    private XMultiServiceFactory m_xMSF = null;

    public String[] getTestMethodNames() {
        return new String[] {
            "ExecuteTest01"};
    }

    public String getTestObjectName () {
        return "SequenceOutputStreamUnitTest";
    }

    public void before() {
        try {
            m_xMSF = (XMultiServiceFactory)param.getMSF ();
        } catch (Exception e) {
            failed ("Cannot create service factory!");
        }
        if (m_xMSF==null) {
            failed ("Cannot create service factory!");
        }
    }

    public void after() {
        m_xMSF = null;
    }

    public void ExecuteTest01() {
        SequenceOutputStreamTest aTest = new Test01 (m_xMSF, log);
        assure ( "Test01 failed!", aTest.test() );
    }
}