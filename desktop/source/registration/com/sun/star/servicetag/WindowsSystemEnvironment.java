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
// the most recent released version for a JDK release.        We rename
// the package to com.sun.servicetag so that the Sun Connection
// product always uses the latest version from the com.sun.scn.servicetags
// package. JDK and users of the com.sun.servicetag API
// (e.g. NetBeans and SunStudio) will use the version in JDK.

import java.io.*;
import java.util.ArrayList;
import java.util.List;
import java.util.HashSet;
import java.util.Set;

/**
 * Windows implementation of the SystemEnvironment class.
 */
class WindowsSystemEnvironment extends SystemEnvironment {
    WindowsSystemEnvironment() {
        super();

        // run a call to make sure things are initialized
        // ignore the first call result as the system may
        // give inconsistent data on the first invocation ever
        getWmicResult("computersystem", "get", "model");

        setSystemModel(getWmicResult("computersystem", "get", "model"));
        setSystemManufacturer(getWmicResult("computersystem", "get", "manufacturer"));
        setSerialNumber(getWmicResult("bios", "get", "serialnumber"));

        String cpuMfr = getWmicResult("cpu", "get", "manufacturer");
        // this isn't as good an option, but if we couldn't get anything
        // from wmic, try the processor_identifier
        if (cpuMfr.length() == 0) {
            String procId = System.getenv("processor_identifer");
            if (procId != null) {
                String[] s = procId.split(",");
                cpuMfr = s[s.length - 1].trim();
            }
        }
        setCpuManufacturer(cpuMfr);

        setSockets(getWindowsSockets());
        setCores(getWindowsCores());
        setVirtCpus(getWindowsVirtCpus());
        setPhysMem(getWindowsPhysMem());
        setCpuName(getWmicResult("cpu", "get", "Name"));
        setClockRate(getWmicResult("cpu", "get", "MaxClockSpeed"));

        // try to remove the temp file that gets created from running wmic cmds
        try {
            // look in the current working directory
            File f = new File("TempWmicBatchFile.bat");
            if (f.exists()) {
                f.delete();
            }
        } catch (Exception e) {
            // ignore the exception
        }
    }

    private String getWindowsVirtCpus() {
        String res = getWmicResult("cpu", "get", "NumberOfLogicalProcessors");
        if (res == null || res.equals("")) {
            res = "1";
        }
        return res;
    }

    private String getWindowsCores() {
        String res = getWmicResult("cpu", "get", "NumberOfCores");
        if (res == null || res.equals("")) {
            res = "1";
        }
        return res;
    }

    private String getWindowsSockets() {
        String res = getFullWmicResult("cpu", "get", "DeviceID");
        Set<String> set = new HashSet<String>();
        for (String line : res.split("\n")) {
            line = line.trim();
            if (line.equals("")) {
                continue;
            }
            set.add(line);
        }
        if (set.size() == 0) {
            return "1";
        }
        return "" + set.size();
    }

    private String getWindowsPhysMem() {
        String mem = getWmicResult("computersystem", "get", "TotalPhysicalMemory");
        long l = Long.parseLong(mem);
        return "" + ((long) (l / (1024*1024)));
    }


    /**
     * This method invokes wmic outside of the normal environment
     * collection routines.
     *
     * An initial call to wmic can be costly in terms of time.
     *
     * <code>
     * Details of why the first call is costly can be found at:
     *
     * http://support.microsoft.com/kb/290216/en-us
     *
     * "When you run the Wmic.exe utility for the first time, the utility
     * compiles its .mof files into the repository. To save time during
     * Windows installation, this operation takes place as necessary."
     * </code>
     */
    private String getWmicResult(String alias, String verb, String property) {
        String res = "";
        BufferedReader in = null;
        try {
            ProcessBuilder pb = new ProcessBuilder("cmd", "/C", "WMIC", alias, verb, property);
            Process p = pb.start();
            // need this for executing windows commands (at least
            // needed for executing wmic command)
            BufferedWriter bw = new BufferedWriter(
                new OutputStreamWriter(p.getOutputStream()));
            bw.write(13);
            bw.flush();
            bw.close();

            p.waitFor();
            if (p.exitValue() == 0) {
                in = new BufferedReader(new InputStreamReader(p.getInputStream()));
                String line = null;
                while ((line = in.readLine()) != null) {
                    line = line.trim();
                    if (line.length() == 0) {
                        continue;
                    }
                    res = line;
                }
                // return the *last* line read
                return res;
            }

        } catch (Exception e) {
            // ignore the exception
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    // ignore
                }
            }
        }
        return res.trim();
    }

    private String getFullWmicResult(String alias, String verb, String property) {
        String res = "";
        BufferedReader in = null;
        try {
            ProcessBuilder pb = new ProcessBuilder("cmd", "/C", "WMIC", alias, verb, property);
            Process p = pb.start();
            // need this for executing windows commands (at least
            // needed for executing wmic command)
            BufferedWriter bw = new BufferedWriter(
                new OutputStreamWriter(p.getOutputStream()));
            bw.write(13);
            bw.flush();
            bw.close();

            p.waitFor();
            if (p.exitValue() == 0) {
                in = new BufferedReader(new InputStreamReader(p.getInputStream()));
                String line = null;
                while ((line = in.readLine()) != null) {
                    line = line.trim();
                    if (line.length() == 0) {
                        continue;
                    }
                    if (line.toLowerCase().indexOf(property.toLowerCase()) != -1) {
                        continue;
                    }
                    res += line + "\n";
                }
            }

        } catch (Exception e) {
            // ignore the exception
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    // ignore
                }
            }
        }
        return res;
    }
}
