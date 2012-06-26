/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testlib;

import java.io.File;

/**
 *
 *
 */
public class Testspace {

    private static final File testspace = new File(System.getProperty("testspace", "../testspace"));

    public static String getPath() {
        return testspace.getAbsolutePath();
    }

    public static String getPath(String file) {
        return getFile(file).getAbsolutePath();
    }

    public static File getFile() {
        return testspace;
    }


    public static File getFile(String file) {
        return new File(testspace, file);
    }
}
