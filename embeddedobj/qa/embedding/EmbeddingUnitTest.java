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


package complex.embedding;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.lang.*;

import complexlib.ComplexTestCase;

import complex.embedding.*;

import util.utils;
import java.util.*;
import java.io.*;

/* This unit test for storage objects is designed to
 * test most important statements from storage service
 * specification.
 *
 * Regression tests are added to extend the tested
 * functionalities.
 */
public class EmbeddingUnitTest  extends ComplexTestCase
{
    private XMultiServiceFactory m_xMSF = null;

    public String[] getTestMethodNames()
    {
        return new String[] {
                                "ExecuteTest01" };
    }

    public String getTestObjectName()
    {
        return "EmbeddingUnitTest";
    }

    public void before()
    {
        m_xMSF = (XMultiServiceFactory)param.getMSF();
        if ( m_xMSF == null )
        {
            failed( "Can't create service factory!" );
            return;
        }
    }

    public void ExecuteTest01()
    {
        EmbeddingTest aTest = new Test01( m_xMSF, log );
        assure( "Test01 failed!", aTest.test() );
    }

}

