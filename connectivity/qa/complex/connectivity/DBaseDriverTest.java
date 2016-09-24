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

package complex.connectivity;

import complex.connectivity.dbase.DBaseDateFunctions;
import complex.connectivity.dbase.DBaseStringFunctions;
import complex.connectivity.dbase.DBaseSqlTests;
import complex.connectivity.dbase.DBaseNumericFunctions;
import complexlib.ComplexTestCase;
import share.LogWriter;

public class DBaseDriverTest extends ComplexTestCase implements TestCase
{
    @Override
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
        return log;
    }

    public void Functions() throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        DBaseStringFunctions aStringTest = new DBaseStringFunctions(param.getMSF(), this);
        aStringTest.testFunctions();

        DBaseNumericFunctions aNumericTest = new DBaseNumericFunctions(param.getMSF(), this);
        aNumericTest.testFunctions();

        DBaseDateFunctions aDateTest = new DBaseDateFunctions(param.getMSF(), this);
        aDateTest.testFunctions();

        DBaseSqlTests aSqlTest = new DBaseSqlTests(param.getMSF(), this);
        aSqlTest.testFunctions();
    }
}
