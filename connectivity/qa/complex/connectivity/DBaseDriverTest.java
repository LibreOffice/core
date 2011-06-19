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
package complex.connectivity;

import complex.connectivity.dbase.DBaseDateFunctions;
import complex.connectivity.dbase.DBaseStringFunctions;
import complex.connectivity.dbase.DBaseSqlTests;
import complex.connectivity.dbase.DBaseNumericFunctions;
import com.sun.star.lang.XMultiServiceFactory;
import complexlib.ComplexTestCase;
import share.LogWriter;

public class DBaseDriverTest extends ComplexTestCase implements TestCase
{
    public String[] getTestMethodNames()
    {
        return new String[]
                {
                    "Functions"
                };
    }

    @Override
    public String getTestObjectName()
    {
        return "DBaseDriverTest";
    }

    @Override
    public void assure( final String i_message, final boolean i_condition )
    {
        super.assure( i_message, i_condition );
    }

    public LogWriter getLog()
    {
        return ComplexTestCase.log;
    }

    public void Functions() throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        DBaseStringFunctions aStringTest = new DBaseStringFunctions(((XMultiServiceFactory) param.getMSF()), this);
        aStringTest.testFunctions();

        DBaseNumericFunctions aNumericTest = new DBaseNumericFunctions(((XMultiServiceFactory) param.getMSF()), this);
        aNumericTest.testFunctions();

        DBaseDateFunctions aDateTest = new DBaseDateFunctions(((XMultiServiceFactory) param.getMSF()), this);
        aDateTest.testFunctions();

        DBaseSqlTests aSqlTest = new DBaseSqlTests(((XMultiServiceFactory) param.getMSF()), this);
        aSqlTest.testFunctions();
    }
}
