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

// The Service Tags team maintains the latest version of the implementation
// for system environment data collection.  JDK will include a copy of
// the most recent released version for a JDK release.  We rename
// the package to com.sun.servicetag so that the Sun Connection
// product always uses the latest version from the com.sun.scn.servicetags
// package. JDK and users of the com.sun.servicetag API
// (e.g. NetBeans and SunStudio) will use the version in JDK.

import java.io.*;
import java.util.Set;
import java.util.HashSet;

/**
 * Solaris implementation of the SystemEnvironment class.
 */
class SolarisSystemEnvironment extends SystemEnvironment {
    private static final int SN  = 1;
    private static final int SYS = 2;
    private static final int CPU = 3;
    private static final int MODEL = 4;
    private String kstatCpuInfo = null;

    SolarisSystemEnvironment() {
        setHostId(getCommandOutput("/usr/bin/hostid"));
        setSystemModel(getSolarisModel());
        setSystemManufacturer(getSolarisSystemManufacturer());
        setCpuManufacturer(getSolarisCpuManufacturer());
        setSerialNumber(getSolarisSN());
        setPhysMem(getSolarisPhysMem());
        setSockets(getSolarisSockets());
        setCores(getSolarisCores());
        setVirtCpus(getSolarisVirtCpus());
        setCpuName(getSolarisCpuName());
        setClockRate(getSolarisClockRate());
    }

    private String getSolarisClockRate() {
        String data = getSolarisKstatCpuInfo();

        String lines[] = data.split("\n");
        String token = "clock_MHz";
        for (int i=0; i<lines.length; i++) {
            String line = lines[i].trim();
            if (line.startsWith(token)) {
                return line.substring(line.indexOf(token) + token.length()).trim();
            }
        }
        return "";
    }

    private String getSolarisCpuName() {
        String data = getSolarisKstatCpuInfo();

        String lines[] = data.split("\n");
        String token = "brand";
        for (int i=0; i<lines.length; i++) {
            String line = lines[i].trim();
            if (line.startsWith(token)) {
                return line.substring(line.indexOf(token) + token.length()).trim();
            }
        }
        return "";
    }

    private String getSolarisVirtCpus() {
        String data = getSolarisKstatCpuInfo();

        int cnt = 0;
        String lines[] = data.split("\n");
        String token = " cpu_info ";
        for (int i=0; i<lines.length; i++) {
            String line = lines[i].trim();
            if (line.indexOf(token) != -1) {
                cnt++;
            }
        }
        return "" + cnt;
    }

    private String getSolarisCores() {
        String data = getSolarisKstatCpuInfo();

        Set<String> set = new HashSet<String>();
        String lines[] = data.split("\n");
        String coreIdToken = "core_id";
        String coreId = "";
        String chipIdToken = "chip_id";
        String chipId = "";
        for (int i=0; i<lines.length; i++) {
            String line = lines[i].trim();
            if (line.startsWith(chipIdToken)) {
                chipId = line.substring(line.indexOf(chipIdToken) + chipIdToken.length()).trim();
            }
            if (line.startsWith(coreIdToken)) {
                coreId = line.substring(line.indexOf(coreIdToken) + coreIdToken.length()).trim();
                set.add(chipId + "," + coreId);
            }
        }
        return "" + set.size();
    }

    private String getSolarisPhysMem() {
        String data = getCommandOutput("/usr/sbin/prtconf");

        int cnt = 0;
        String lines[] = data.split("\n");
        String token = "Memory size:";
        for (int i=0; i<lines.length; i++) {
            String line = lines[i].trim();
            if (line.startsWith(token)) {
                line = line.substring(line.indexOf(token) + token.length()).trim();
                if (line.indexOf(" ") != -1) {
                    return line.substring(0, line.indexOf(" ")).trim();
                }
            }
        }
        return "";
    }

    private String getSolarisSockets() {
        String data = getSolarisKstatCpuInfo();

        Set<String> set = new HashSet<String>();
        String lines[] = data.split("\n");
        String token = "chip_id";
        for (int i=0; i<lines.length; i++) {
            String line = lines[i].trim();
            if (line.startsWith(token)) {
                String id = line.substring(line.indexOf(token) + token.length()).trim();
                set.add(id);
            }
        }
        return "" + set.size();
    }

    private synchronized String getSolarisKstatCpuInfo() {
        // only try to get kstat cpu_info information once, after that, we can
        // reuse the output
        if (kstatCpuInfo == null) {
            Thread thread = new Thread() {
                public void run() {
                    kstatCpuInfo = getCommandOutput("/usr/bin/kstat", "cpu_info");
                }
            };
            thread.start();

            try {
                thread.join(2000);
                if (thread.isAlive()) {
                    thread.interrupt();
                    kstatCpuInfo = "";
                }
            } catch (InterruptedException ie) {
                thread.interrupt();
            }
        }
        return kstatCpuInfo;
    }

    private String getSolarisModel() {
        String tmp = getFileContent("/var/run/psn");
        if (tmp.length() > 0) {
            String[] lines = tmp.split("\n");
            if (MODEL <= lines.length) {
                return lines[MODEL-1] + "::"
                    + getCommandOutput("/usr/bin/uname", "-v");
            }
        }

        if ("sparc".equalsIgnoreCase(System.getProperty("os.arch"))) {
            return getCommandOutput("/usr/bin/uname", "-i") + "::"
                + getCommandOutput("/usr/bin/uname", "-v");
        } else {
            String model = getSmbiosData("1", "Product: ");
            if (model == null || model.trim().equals("")) {
                model = getCommandOutput("/usr/bin/uname", "-i");
            }
            if (model == null) {
                model = "";
            }
            return model.trim() + "::"
                + getCommandOutput("/usr/bin/uname", "-v");
        }
    }

    /**
     * Tries to obtain the cpu manufacturer.
     * @return The cpu manufacturer (an empty string if not found or an error occurred)
     */
    private String getSolarisCpuManufacturer() {
        String tmp = getFileContent("/var/run/psn");
        if (tmp.length() > 0) {
            String[] lines = tmp.split("\n");
            if (CPU <= lines.length) {
                return lines[CPU-1];
            }
        }

        // not fully accurate, this could be another manufacturer (fujitsu for example)
        if ("sparc".equalsIgnoreCase(System.getProperty("os.arch"))) {
            return "Sun Microsystems, Inc";
        }

        // if we're here, then we'll try smbios (type 4)
        return getSmbiosData("4", "Manufacturer: ");
    }

    /**
     * Tries to obtain the system manufacturer.
     * @return The system manufacturer (an empty string if not found or an error occurred)
     */
    private String getSolarisSystemManufacturer() {
        String tmp = getFileContent("/var/run/psn");
        if (tmp.length() > 0) {
            String[] lines = tmp.split("\n");
            if (SYS <= lines.length) {
                return lines[SYS-1];
            }
        }

        // not fully accurate, this could be another manufacturer (fujitsu for example)
        if ("sparc".equalsIgnoreCase(System.getProperty("os.arch"))) {
            if (getCommandOutput("/usr/bin/uname", "-m").equals("sun4us")) {
                return "Fujitsu";
            }
            return "Sun Microsystems, Inc";
        }

        // if we're here, then we'll try smbios (type 1)
        return getSmbiosData("1", "Manufacturer: ");
    }

    /**
     * Tries to obtain the serial number.
     * @return The serial number (empty string if not found or an error occurred)
     */
    private String getSolarisSN() {
        // try to read from the psn file if it exists
        String tmp = getFileContent("/var/run/psn");
        if (tmp.length() > 0) {
            String[] lines = tmp.split("\n");
            if (SN <= lines.length) {
                return lines[SN-1];
            }
        }

        // if we're here, then we'll try sneep
        String tmpSN = getSneepSN();
        if (tmpSN.length() > 0) {
            return tmpSN;
        }

        // if we're here, then we'll try smbios (type 1)
        tmpSN = getSmbiosData("1", "Serial Number: ");
        if (tmpSN.length() > 0) {
            return tmpSN;
        }

        // if we're here, then we'll try smbios (type 3)
        tmpSN = getSmbiosData("3", "Serial Number: ");
        if (tmpSN.length() > 0) {
            return tmpSN;
        }

        if ("sparc".equalsIgnoreCase(System.getProperty("os.arch"))) {
            tmpSN = getSNViaPrtfruX();
            if (tmpSN.length() > 0) {
                return tmpSN;
            }
            tmpSN = getSNViaPrtfru();
            if (tmpSN.length() > 0) {
                return tmpSN;
            }
        }

        // give up and return
        return "";
    }

    // Sample smbios output segment:
    // ID    SIZE TYPE
    // 1     150  SMB_TYPE_SYSTEM (system information)
    //   Manufacturer: Sun Microsystems
    //   Product: Sun Fire X4600
    //   Version: To Be Filled By O.E.M.
    //   Serial Number: 00:14:4F:45:0C:2A
    private String getSmbiosData(String type, String target) {
        String output = getCommandOutput("/usr/sbin/smbios", "-t", type);
        for (String s : output.split("\n")) {
            if (s.contains(target)) {
                int indx = s.indexOf(target) + target.length();
                if (indx < s.length()) {
                    String tmp = s.substring(indx).trim();
                    String lowerCaseStr = tmp.toLowerCase();
                    if (!lowerCaseStr.startsWith("not available")
                            && !lowerCaseStr.startsWith("to be filled by o.e.m")) {
                        return tmp;
                    }
                }
            }
        }

        return "";
    }

    private String getSneepSN() {
        String basedir = getCommandOutput("pkgparam","SUNWsneep","BASEDIR");
        File f = new File(basedir + "/bin/sneep");
        if (f.exists()) {
            String sneepSN = getCommandOutput(basedir + "/bin/sneep");
            if (sneepSN.equalsIgnoreCase("unknown")) {
                return "";
            } else {
                return sneepSN;
            }
        } else {
            return "";
        }
    }

    private String getSNViaPrtfruX() {
        String data = getCommandOutput("/usr/sbin/prtfru", "-x");

        boolean FRUTREE_FLAG = false;
        boolean FRUNAME_FLAG = false;
        boolean MB_LABEL_FLAG = false;
        boolean SYSTEM_BOARD_FLAG = false;

        String lines[] = data.split("\n");
        for (int i=0; i<lines.length; i++) {
            String line = lines[i];
            if (SYSTEM_BOARD_FLAG) {
                String tok = "<Sun_Serial_No value=\"";
                int index = line.indexOf(tok);
                if (index != -1) {
                    String val = line.substring(index+tok.length());
                    String vals[] = val.split("\"");
                    if (vals.length > 0) {
                        return vals[0].trim();
                    }
                    break;
                }
            }

            if (line.indexOf("</ContainerData>") != -1) {
                FRUTREE_FLAG = false;
                FRUNAME_FLAG = false;
                SYSTEM_BOARD_FLAG = false;
            }

            if (FRUNAME_FLAG && line.indexOf("<Container name=\"system-board\">") != -1 ) {
                SYSTEM_BOARD_FLAG = true;
            }

            if (FRUTREE_FLAG && line.indexOf("<Fru name=\"chassis\">") != -1 ) {
                FRUNAME_FLAG = true;
            }

            if (line.indexOf("<Location name=\"frutree\">") != -1) {
                FRUTREE_FLAG = true;
            }
        }

        return "";
    }

    private String getSNViaPrtfru() {
        String data = getCommandOutput("/usr/sbin/prtfru");
        boolean CHASSIS_FLAG = false;

        String lines[] = data.split("\n");
        for (int i=0; i<lines.length; i++) {
            String line = lines[i];
            if (CHASSIS_FLAG) {
                String tok = "/ManR/Sun_Serial_No:";
                int index = line.indexOf(tok);
                if (index != -1) {
                    String val = line.substring(index+tok.length());
                    return val.trim();
                }
            }

            if (line.indexOf("/frutree/chassis/system-board (container)") != -1) {
                CHASSIS_FLAG = true;
            } else if (line.indexOf("/frutree/chassis/MB?Label=MB/system-board (container)") != -1) {
                CHASSIS_FLAG = true;
            }
        }
        return "";
    }
}
