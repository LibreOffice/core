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
