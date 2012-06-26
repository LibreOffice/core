/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testsuite;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Suite;
import org.junit.runners.Parameterized.Parameters;
import org.junit.runners.Suite.SuiteClasses;

import testcase.TestSample;
import testsuite.TestSampleAsMS2003.SetParams;

@RunWith(Suite.class)
@SuiteClasses({SetParams.class, TestSample.class })
public class TestSampleAsMS2003 {

    @RunWith(Parameterized.class)
    public static class SetParams {
        @Parameters
        public static Collection<Object[]>  data() {
            TestSample.repos = "../samples";
            TestSample.params = new String[][]{
                {".*\\.((docx)|(docm)|(dotx)|(dotm))$", "doc", "writer"},
                {".*\\.((xlsx)|(xltx)|(xlsm)|(xltm))$", "xls", "calc"},
                {".*\\.((pptx)|(pptm)|(potm)|(potx))$", "ppt", "impress"}
            };
            return new ArrayList<Object[]>();
        }
    }
}
