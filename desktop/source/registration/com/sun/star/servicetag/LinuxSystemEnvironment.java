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




package com.sun.star.servicetag;

// The Service Tags team maintains the latest version of the implementation
// for system environment data collection.  JDK will include a copy of
// the most recent released version for a JDK release.  We rename
// the package to com.sun.servicetag so that the Sun Connection
// product always uses the latest version from the com.sun.scn.servicetags
// package. JDK and users of the com.sun.servicetag API
// (e.g. NetBeans and SunStudio) will use the version in JDK.

import java.io.*;
import java.util.*;

/**
 * Linux implementation of the SystemEnvironment class.
 */
class LinuxSystemEnvironment extends SystemEnvironment {
    LinuxSystemEnvironment() {
        setHostId(getLinuxHostId());

        setSystemModel(getLinuxModel());
        setSystemManufacturer(getLinuxSystemManufacturer());
        setCpuManufacturer(getLinuxCpuManufacturer());
        setSerialNumber(getLinuxSN());
        setPhysMem(getLinuxPhysMem());
        setSockets(getLinuxSockets());
        setCores(getLinuxCores());
        setVirtCpus(getLinuxVirtCpus());
        setCpuName(getLinuxCpuName());
        setClockRate(getLinuxClockRate());
    }
    private String dmiInfo = null;
    private String kstatCpuInfo = null;

    private static final int SN  = 1;
    private static final int SYS = 2;
    private static final int CPU = 3;
    private static final int MODEL = 4;

    private String getLinuxHostId() {
        String output = getCommandOutput("/usr/bin/hostid");
        // trim off the leading 0x
        if (output.startsWith("0x")) {
            output = output.substring(2);
        }
        return output;
    }

    /**
     * Tries to obtain and return the cpu manufacturer.
     * @return The cpu manufacturer (an empty string if not found or an error occurred)
     */
    private String getLinuxCpuManufacturer() {
        String tmp = getLinuxPSNInfo(CPU);
        if (tmp.length() > 0) {
            return tmp;
        }

        String contents = getFileContent("/proc/cpuinfo");
        for (String line : contents.split("\n")) {
            if (line.contains("vendor_id")) {
                String[] ss = line.split(":", 2);
                if (ss.length > 1) {
                    return ss[1].trim();
                }
            }
        }

        // returns an empty string if it can't be found or an error happened
        return getLinuxDMIInfo("dmi type 4", "manufacturer");
    }

    private String getLinuxModel() {
        String tmp = getLinuxPSNInfo(MODEL);
        if (tmp.length() > 0) {
            return tmp + "::" + getCommandOutput("/bin/uname","-v");
        }

        tmp = getLinuxDMIInfo("dmi type 1", "product name");
        if (tmp.length() > 0) {
            return tmp + "::" + getCommandOutput("/bin/uname","-v");
        }

        return getCommandOutput("/bin/uname","-i")
            + "::" + getCommandOutput("/bin/uname","-v");
    }


    /**
     * Tries to obtain and return the system manufacturer.
     * @return The system manufacturer (an empty string if not found or an error occurred)
     */
    private String getLinuxSystemManufacturer() {
        String tmp = getLinuxPSNInfo(SYS);
        if (tmp.length() > 0) {
            return tmp;
        }

        // returns an empty string if it can't be found or an error happened
        return getLinuxDMIInfo("dmi type 1", "manufacturer");
    }

    /**
     * Tries to obtain and return the serial number of the system.
     * @return The serial number (an empty string if not found or an error occurred)
     */
    private String getLinuxSN() {
        String tmp = getLinuxPSNInfo(SN);
        if (tmp.length() > 0) {
            return tmp;
        }

        // returns an empty string if it can't be found or an error happened
        return getLinuxDMIInfo("dmi type 1", "serial number");
    }

    private String getLinuxPSNInfo(int target) {
        // try to read from the psn file if it exists
        String contents = getFileContent("/var/run/psn");
        String[] ss = contents.split("\n");
        if (target <= ss.length) {
            return ss[target-1];
        }

        // default case is to return ""
        return "";
    }

    // reads from dmidecode with the given type and target
    // returns an empty string if nothing was found or an error occurred
    //
    // Sample output segment:
    // Handle 0x0001
    //         DMI type 1, 25 bytes.
    //         System Information
    //                 Manufacturer: System manufacturer
    //                 Product Name: System Product Name
    //                 Version: System Version
    //                 Serial Number: System Serial Number
    //                 UUID: 3091D719-B25B-D911-959D-6D1B12C7686E
    //                 Wake-up Type: Power Switch

    private synchronized String getLinuxDMIInfo(String dmiType, String target) {
        // only try to get dmidecode information once, after that, we can
        // reuse the output
        if (dmiInfo == null) {
            Thread dmidecodeThread = new Thread() {
                public void run() {
                    dmiInfo = getCommandOutput("/usr/sbin/dmidecode");
                }
            };
            dmidecodeThread.start();

            try {
                dmidecodeThread.join(3000);
                if (dmidecodeThread.isAlive()) {
                    dmidecodeThread.interrupt();
                    dmiInfo = "";
                }
            } catch (InterruptedException ie) {
                dmidecodeThread.interrupt();
            }
        }

        if (dmiInfo.length() == 0) {
            return "";
        }
        boolean dmiFlag = false;
        for (String s : dmiInfo.split("\n")) {
            String line = s.toLowerCase();
            if (dmiFlag) {
                if (line.contains(target)) {
                    String key = target + ":";
                    int indx = line.indexOf(key) + key.length();
                    if (line.contains(key) && indx < line.length()) {
                        return line.substring(indx).trim();
                    }
                    String[] ss = line.split(":");
                    return ss[ss.length-1];
                }
            } else if (line.contains(dmiType)) {
                dmiFlag = true;
            }
        }
        return "";
    }

    private String getLinuxClockRate() {
        String contents = getFileContent("/proc/cpuinfo");
        String token = "cpu MHz";
        for (String line : contents.split("\n")) {
            if (line.contains(token)) {
                String[] key = line.split(":", 2);
                if (key.length > 1) {
                    return key[1].trim();
                }
            }
        }
        return "";
    }

    private String getLinuxCpuName() {
        String contents = getFileContent("/proc/cpuinfo");
        String token = "model name";
        for (String line : contents.split("\n")) {
            if (line.contains(token)) {
                String[] key = line.split(":", 2);
                if (key.length > 1) {
                    return key[1].trim();
                }
            }
        }
        return "";
    }

    private String getLinuxVirtCpus() {
        Set<String> set = new HashSet<String>();
        String contents = getFileContent("/proc/cpuinfo");
        String token = "processor";
        for (String line : contents.split("\n")) {
            if (line.contains(token)) {
                String[] key = line.split(":", 2);
                if (key.length > 1) {
                    set.add(key[1].trim());
                }
            }
        }
        return "" + set.size();
    }

    private String getLinuxCores() {
        Set<String> set = new HashSet<String>();
        String contents = getFileContent("/proc/cpuinfo");
        String token = "core id";
        for (String line : contents.split("\n")) {
            if (line.contains(token)) {
                String[] key = line.split(":", 2);
                if (key.length > 1) {
                    set.add(key[1].trim());
                }
            }
        }
        if (set.size() == 0) {
            return "1";
        }
        return "" + set.size();
    }

    private String getLinuxPhysMem() {
        String contents = getFileContent("/proc/meminfo");
        for (String line : contents.split("\n")) {
            if (line.contains("MemTotal")) {
                String[] total = line.split(":", 2);
                if (total.length > 1) {
                    String[] mem = total[1].trim().split(" ");
                    if (mem.length >= 1) {
                        return mem[0].trim();
                    } else {
                        return total[1].trim();
                    }
                }
            }
        }

        return "";
    }

    private String getLinuxSockets() {
        Set<String> physIdSet = new HashSet<String>();
        Set<String> procSet = new HashSet<String>();
        String contents = getFileContent("/proc/cpuinfo");
        String physIdToken = "physical id";
        String procToken = "processor";

        for (String line : contents.split("\n")) {
            if (line.contains(physIdToken)) {
                String[] key = line.split(":", 2);
                if (key.length > 1) {
                    physIdSet.add(key[1].trim());
                }
            }

            if (line.contains(procToken)) {
                String[] key = line.split(":", 2);
                if (key.length > 1) {
                    procSet.add(key[1].trim());
                }
            }
        }
        if (physIdSet.size() != 0) {
            return "" + physIdSet.size();
        }
        return "" + procSet.size();
    }
}
