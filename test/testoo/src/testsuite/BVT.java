/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testsuite;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import testcase.BVTFileType;
import testcase.BVTFunction;
import testcase.BVTFunction2;
import testcase.SmokeTest;

@RunWith(Suite.class)
@SuiteClasses({SmokeTest.class, BVTFileType.class, BVTFunction.class, BVTFunction2.class })
public class BVT {

}
