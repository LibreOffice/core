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

package com.sun.star.servicetag;

import java.io.*;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import static com.sun.star.servicetag.Util.*;
import static com.sun.star.servicetag.RegistrationDocument.*;

/**
 * Class containing additional methods that are not yet
 * in the JDK Registry class.  Note that all methods in this class
 * will be superceeded by the JDK classes.
 */
public class SysnetRegistryHelper {

    private static final String STCLIENT_SOLARIS = "/usr/bin/stclient";
    private static final String STCLIENT_LINUX = "/opt/sun/servicetag/bin/stclient";
    // stclient exit value (see sthelper.h)
    private static final int ST_ERR_NOT_AUTH = 245;
    private static final int ST_ERR_REC_NOT_FOUND = 225;

    // The stclient output has to be an exported interface
    private static final String INSTANCE_URN_OPEN_ELEMENT = "<instance_urn>";
    private static final String INSTANCE_URN_CLOSE_ELEMENT = "</instance_urn>";
    private static final String REGISTRY_URN = "<registry urn=\"";
    private static final String INSTANCE_URN_DESC = "Product instance URN=";
    private static boolean initialized = false;
    private static boolean supportsHelperClass = true; // default
    private static File stclient = null;
    private static String stclientPath = null;

    // System properties for testing
    private static String SVCTAG_STCLIENT_CMD = "servicetag.stclient.cmd";
    private static String SVCTAG_STHELPER_SUPPORTED = "servicetag.sthelper.supported";

    private synchronized static String getSTclient() {
        if (!initialized) {
            // the system property always overrides the default setting
            if (System.getProperty(SVCTAG_STHELPER_SUPPORTED) != null) {
                supportsHelperClass = Boolean.getBoolean(SVCTAG_STHELPER_SUPPORTED);
            }

            // This is only used for testing
            stclientPath = System.getProperty(SVCTAG_STCLIENT_CMD);
            if (stclientPath != null) {
                return stclientPath;
            }

            // Initialization to determine the platform's stclient pathname
            String os = System.getProperty("os.name");
            if (os.equals("SunOS")) {
                stclient = new File(STCLIENT_SOLARIS);
            } else if (os.equals("Linux")) {
                stclient = new File(STCLIENT_LINUX);
            } else if (os.startsWith("Windows")) {
        stclient = getWindowsStClientFile();
            } else {
                if (isVerbose()) {
                    System.out.println("Running on non-Sun JDK");
                }
            }
            initialized = true;
        }

        // com.sun.servicetag package has to be compiled with JDK 5 as well
        // JDK 5 doesn't support the File.canExecute() method.
        // Risk not checking isExecute() for the stclient command is very low.

        if (stclientPath == null && stclient != null && stclient.exists()) {
            stclientPath = stclient.getAbsolutePath();
        }
        return stclientPath;
    }

    private static List<String> getCommandList() {
        // Set up the arguments to call stclient
        List<String> command = new ArrayList<String>();
        if (System.getProperty(SVCTAG_STCLIENT_CMD) != null) {
            // This is for jtreg testing use. This will be set to something
            // like:
            // $JAVA_HOME/bin/java -cp $TEST_DIR \
            //    -Dstclient.registry.path=$TEST_DIR/registry.xml \
            //    SvcTagClient
            //
            // On Windows, the JAVA_HOME and TEST_DIR path could contain
            // space e.g. c:\Program Files\Java\jdk1.6.0_05\bin\java.
            // The SVCTAG_STCLIENT_CMD must be set with a list of
            // space-separated parameters.  If a parameter contains spaces,
            // it must be quoted with '"'.

            String cmd = getSTclient();
            int len = cmd.length();
            int i = 0;
            while (i < len) {
                char separator = ' ';
                if (cmd.charAt(i) == '"') {
                    separator = '"';
                    i++;
                }
                // look for the separator or matched the closing '"'
                int j;
                for (j = i+1; j < len; j++) {
                    if (cmd.charAt(j) == separator) {
                        break;
                    }
                }

                if (i == j-1) {
                    // add an empty parameter
                    command.add("\"\"");
                } else {
                    // double quotes and space are not included
                    command.add(cmd.substring(i,j));
                }

                // skip spaces
                for (i = j+1; i < len; i++) {
                    if (!Character.isSpaceChar(cmd.charAt(i))) {
                        break;
                    }
                }
            }
            if (isVerbose()) {
                System.out.println("Command list:");
                for (String s : command) {
                    System.out.println(s);
                }
            }
        } else {
            command.add(getSTclient());
        }
        return command;
    }

    // Returns null if the service tag record not found;
    // or throw UnauthorizedAccessException or IOException
    // based on the exitValue.
    private static ServiceTag checkReturnError(int exitValue,
                                               String output,
                                               ServiceTag st) throws IOException {
        switch (exitValue) {
            case ST_ERR_REC_NOT_FOUND:
                return null;
            case ST_ERR_NOT_AUTH:
                if (st != null) {
                    throw new UnauthorizedAccessException(
                        "Not authorized to access " + st.getInstanceURN() +
                        " installer_uid=" + st.getInstallerUID());
                } else  {
                    throw new UnauthorizedAccessException(
                        "Not authorized:" + output);
                }
            default:
                throw new IOException("stclient exits with error" +
                     " (" + exitValue + ")\n" + output);
        }
    }

    /**
     * Returns a {@code ServiceTag} object of the given  <tt>instance_urn</tt>
     * in this registry.
     *
     * @param instanceURN the  <tt>instance_urn</tt> of the service tag
     * @return a {@code ServiceTag} object of the given <tt>instance_urn</tt>
     * in this registry; or {@code null} if not found.
     *
     * @throws java.io.IOException if an I/O error occurs in this operation.
     */
    private static ServiceTag getServiceTag(String instanceURN) throws IOException {
        if (instanceURN == null) {
            throw new NullPointerException("instanceURN is null");
        }

        List<String> command = getCommandList();
        command.add("-g");
        command.add("-i");
        command.add(instanceURN);

        ProcessBuilder pb = new ProcessBuilder(command);
        Process p = pb.start();
        String output = commandOutput(p);
        if (isVerbose()) {
            System.out.println("Output from stclient -g command:");
            System.out.println(output);
        }
        if (p.exitValue() == 0) {
            return parseServiceTag(output);
        } else {
            return checkReturnError(p.exitValue(), output, null);
        }
    }

    private static ServiceTag parseServiceTag(String output) throws IOException {
        BufferedReader in = null;
        try {
            Properties props = new Properties();
            // parse the service tag output from stclient
            in = new BufferedReader(new StringReader(output));
            String line = null;
            while ((line = in.readLine()) != null) {
                if ((line = line.trim()).length() > 0) {
                    String[] ss = line.trim().split("=", 2);
                    if (ss.length == 2) {
                        props.setProperty(ss[0].trim(), ss[1].trim());
                    } else {
                        props.setProperty(ss[0].trim(), "");
                    }
                }
            }

            String urn = props.getProperty(ST_NODE_INSTANCE_URN);
            String productName = props.getProperty(ST_NODE_PRODUCT_NAME);
            String productVersion = props.getProperty(ST_NODE_PRODUCT_VERSION);
            String productURN = props.getProperty(ST_NODE_PRODUCT_URN);
            String productParent = props.getProperty(ST_NODE_PRODUCT_PARENT);
            String productParentURN = props.getProperty(ST_NODE_PRODUCT_PARENT_URN);
            String productDefinedInstanceID =
                props.getProperty(ST_NODE_PRODUCT_DEFINED_INST_ID);
            String productVendor = props.getProperty(ST_NODE_PRODUCT_VENDOR);
            String platformArch = props.getProperty(ST_NODE_PLATFORM_ARCH);
            String container = props.getProperty(ST_NODE_CONTAINER);
            String source = props.getProperty(ST_NODE_SOURCE);
            int installerUID =
                Util.getIntValue(props.getProperty(ST_NODE_INSTALLER_UID));
            Date timestamp =
                Util.parseTimestamp(props.getProperty(ST_NODE_TIMESTAMP));

            return new ServiceTag(urn,
                                  productName,
                                  productVersion,
                                  productURN,
                                  productParent,
                                  productParentURN,
                                  productDefinedInstanceID,
                                  productVendor,
                                  platformArch,
                                  container,
                                  source,
                                  installerUID,
                                  timestamp);
        } finally {
            if (in != null) {
                in.close();
            }
        }

    }

    /**
     * Returns the urn of this registry.
     *
     * @return a {@code String} for the urn of this registry.
     *
     * @throws java.io.IOException if an I/O error occurs in this operation.
     */
    // Once JDK makes this method available, we'll deprecate this method
    // @deprecated Use the JDK version when available.
    public static String getRegistryURN() throws IOException {
        List<String> command = getCommandList();
        command.add("-x");

        BufferedReader in = null;
        try {
            ProcessBuilder pb = new ProcessBuilder(command);
            Process p = pb.start();
            String output = commandOutput(p);

            String registryURN = null;
            if (p.exitValue() == 0) {
                // parse the service tag output from stclient
                in = new BufferedReader(new StringReader(output));
                String line = null;
                while ((line = in.readLine()) != null) {
                    String s = line.trim();
                    if (s.indexOf(REGISTRY_URN) != -1) {
                        s = s.substring(s.indexOf(REGISTRY_URN)
                            + REGISTRY_URN.length());
                        if (s.indexOf("\"") != -1) {
                            s = s.substring(0, s.indexOf("\""));
                            registryURN = s;
                            break;
                        }
                    }
                }
            } else {
                checkReturnError(p.exitValue(), output, null);
            }
            return registryURN;
        } finally {
            if (in != null) {
                in.close();
            }
        }
    }

    /**
     * Returns all the service tags in this registry.
     *
     * @return a {@code Set} of {@code ServiceTag} objects
     * in this registry.
     *
     * @throws java.io.IOException if an I/O error occurs in this operation.
     */
    // Once JDK makes this method available, we'll deprecate this method
    // @deprecated Use the JDK version when available.
    public static Set<ServiceTag> getServiceTags() throws IOException {
        List<String> command = getCommandList();
        command.add("-x");

        BufferedReader in = null;
        try {
            ProcessBuilder pb = new ProcessBuilder(command);
            Process p = pb.start();
            String output = commandOutput(p);

            Set<ServiceTag> instances = new HashSet<ServiceTag>();
            if (p.exitValue() == 0) {
                // parse the service tag output from stclient
                in = new BufferedReader(new StringReader(output));
                String line = null;
                while ((line = in.readLine()) != null) {
                    String s = line.trim();
                    if (s.indexOf(INSTANCE_URN_OPEN_ELEMENT) != -1
                            && s.indexOf(INSTANCE_URN_CLOSE_ELEMENT) != -1) {
                        s = s.substring(s.indexOf(INSTANCE_URN_OPEN_ELEMENT)
                            + INSTANCE_URN_OPEN_ELEMENT.length(),
                            s.indexOf(INSTANCE_URN_CLOSE_ELEMENT));
                        try {
                            instances.add(getServiceTag(s));
                        } catch (Exception e) {
                        }
                    }
                }
            } else {
                checkReturnError(p.exitValue(), output, null);
            }
            return instances;
        } finally {
            if (in != null) {
                in.close();
            }
        }
    }
}
