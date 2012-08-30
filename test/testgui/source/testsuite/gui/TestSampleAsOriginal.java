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

package testsuite.gui;

import java.util.ArrayList;
import java.util.Collection;

import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import testcase.gui.TestSample;
import testsuite.gui.TestSampleAsOriginal.SetParams;

@RunWith(Suite.class)
@SuiteClasses({ SetParams.class, TestSample.class })
public class TestSampleAsOriginal {

    @RunWith(Parameterized.class)
    public static class SetParams {
        @Parameters
        public static Collection<Object[]> data() {
            TestSample.repos = "../samples";
            TestSample.params = new String[][] { { ".*\\.((odt)|(ott)|(sxw)|(stw)|(doc)|(dot))$", null, "writer" },
                    { ".*\\.((ods)|(ots)|(sxc)|(stc)|(xls)|(xlt))$", null, "calc" }, { ".*\\.((odp)|(otp)|(sxi)|(sti)|(ppt)|(pot))$", null, "impress" } };
            return new ArrayList<Object[]>();
        }
    }
}
