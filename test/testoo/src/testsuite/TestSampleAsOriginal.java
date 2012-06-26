/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testsuite;

import java.util.ArrayList;
import java.util.Collection;

import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import testcase.TestSample;
import testsuite.TestSampleAsOriginal.SetParams;

@RunWith(Suite.class)
@SuiteClasses({SetParams.class, TestSample.class })
public class TestSampleAsOriginal {

    @RunWith(Parameterized.class)
    public static class SetParams {
        @Parameters
        public static Collection<Object[]>  data() {
            TestSample.repos = "../samples";
            TestSample.params = new String[][]{
                {".*\\.((odt)|(ott)|(sxw)|(stw)|(doc)|(dot))$", null, "writer"},
                {".*\\.((ods)|(ots)|(sxc)|(stc)|(xls)|(xlt))$", null, "calc"},
                {".*\\.((odp)|(otp)|(sxi)|(sti)|(ppt)|(pot))$", null, "impress"}
            };
            return new ArrayList<Object[]>();
        }
    }
}
