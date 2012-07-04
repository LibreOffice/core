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



package org.openoffice.test.common;

import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.StringSelection;
import java.awt.datatransfer.Transferable;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.StringReader;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;
import java.util.logging.Logger;

/**
 * Utilities related to system
 *
 */
public class SystemUtil {

    private static Logger LOG = Logger.getLogger(SystemUtil.class.getName());

    private static Clipboard sysClipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

    private static final String OSNAME = System.getProperty("os.name");

    public static final File SCRIPT_TEMP_DIR = new File(System.getProperty("user.home"), ".ootest");

    /**
     * Play beep sound! The method doesn't work, if the code is executed on
     * Eclipse IDE.
     *
     */
    public static void beep() {
        System.out.print("\007\007\007");
        System.out.flush();
    }

    public static boolean isWindows() {
        return OSNAME.startsWith("Windows");
    }

    public static boolean isLinux() {
        return OSNAME.startsWith("Linux");
    }

    public static boolean isMac() {
        return OSNAME.startsWith("Mac");
    }

    public static String getEnv(String name, String defaultValue) {
        String value = System.getenv(name);
        return value == null ? defaultValue : value;
    }

    /**
     * Set the contents of the clipboard to the provided text
     */
    public static void setClipboardText(String s) {
        StringSelection ss = new StringSelection(s);

        // if (OS.get() == OS.MACOSX) {
        // // workaround MAC OS X has a bug. After setting a text into
        // clipboard, the java program will not
        // // receive the data written by other apllications.
        // File file = null;
        // try {
        // file = File.createTempFile("SystemUtil", "SystemUtil");
        // FileUtil.writeStringToFile(file.getAbsolutePath(), s);
        // if (exec("pbcopy < \""+ file.getAbsolutePath() + "\"", false) == 0)
        // return;
        // } catch (IOException e) {
        // // TODO Auto-generated catch block
        // e.printStackTrace();
        // } finally {
        // if (file != null)
        // file.delete();
        // }
        //
        // }
        //
        sysClipboard.setContents(ss, ss);
    }

    /**
     * Get plain text from clipboard
     *
     * @return
     */
    public static String getClipboardText() {
        Transferable contents = getTransferable();
        if (contents == null
                || !contents.isDataFlavorSupported(DataFlavor.stringFlavor))
            return "";
        try {
            return (String) contents.getTransferData(DataFlavor.stringFlavor);
        } catch (Exception ex) {
            return "";
        }
    }

    private static Transferable getTransferable() {
        // To avoid IllegalStateException, we try 25 times to access clipboard.
        for (int i = 0; i < 25; i++) {
            try {
                return sysClipboard.getContents(null);
            } catch (IllegalStateException e) {
                try {
                    Thread.sleep(200);
                } catch (InterruptedException e1) {
                }
            }
        }
        throw new RuntimeException("System Clipboard is not ready");
    }

    /**
     * Execute a script and waiting it for finishing
     * @param content
     * @return
     */
    public static int execScript(String content) {
        return execScript(content, false);
    }

    /**
     * Execute a script. bat on Windows, bash on Linux
     * @param content
     * @param spawn
     * @return
     */
    public static int execScript(String content, boolean spawn) {
        File file = null;
        try {
            file = FileUtil
                    .getUniqueFile(SCRIPT_TEMP_DIR, "tempscript", ".bat");
            FileUtil.writeStringToFile(file.getAbsolutePath(), content);
            String[] cmd;
            if (isWindows())
                cmd = new String[] { file.getAbsolutePath() };
            else
                cmd = new String[] { "sh", file.getAbsolutePath() };
            StringBuffer output = new StringBuffer();
            int code = exec(cmd, null, spawn, output, output);
            LOG.info(content + "\n" + "Exit Code: " + code + "\n" + output);
            return code;
        } catch (Exception e) {
            return -1;
        } finally {
            if (file != null) {
                try {
                    file.deleteOnExit();
                } catch (Exception e) {
                    // ignore
                }
            }
        }
    }

    public static int exec(String[] command, String workingDir, boolean spawn,
            StringBuffer output, StringBuffer error) {
        Process process = null;
        File dir = workingDir == null ? null : new File(workingDir);
        int code = 0;
        try {
            process = Runtime.getRuntime().exec(command, null, dir);
        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }
        StreamPump inputPump = new StreamPump(output, process.getInputStream());
        StreamPump errorPump = new StreamPump(error, process.getErrorStream());
        inputPump.start();
        errorPump.start();
        try {
            if (!spawn) {
                code = process.waitFor();
                inputPump.join();
                errorPump.join();
            }
            return code;
        } catch (InterruptedException e) {
            e.printStackTrace();
            return -1;
        }
    }

    /**
     * Make the current thread sleep some seconds.
     * @param second
     */
    public static void sleep(double second) {
        try {
            Thread.sleep((long) (second * 1000));
        } catch (InterruptedException e) {
        }
    }

    /**
     * Get the commands of all running processes
     *
     * @return
     */
    public static List<String> getProcesses() {
        List<String> ret = new ArrayList<String>();
        try {
            StringBuffer output = new StringBuffer();
            if (isWindows()) {
                File file = File.createTempFile("ssssss", ".js");
                String contents = "var e=new Enumerator(GetObject(\"winmgmts:\").InstancesOf(\"Win32_process\"));\n\r";
                contents += "for (;!e.atEnd();e.moveNext()) {\n\r";
                contents += "WScript.Echo(e.item ().CommandLine);}";
                FileUtil.writeStringToFile(file.getAbsolutePath(), contents);
                // exec("cscript //Nologo \"" + file.getAbsolutePath() + "\"",
                // output, output);
            } else {
                // exec("ps -x -eo command", output, output);
            }
            BufferedReader reader = new BufferedReader(new StringReader(
                    output.toString()));
            String line = null;
            while ((line = reader.readLine()) != null) {
                ret.add(line);
            }
        } catch (IOException e) {

        }

        return ret;
    }

    /**
     * parse a string to arguments array.
     *
     * @param line
     * @return
     */
    public static String[] parseCommandLine(String line) {
        ArrayList<String> arguments = new ArrayList<String>();
        StringTokenizer tokenizer = new StringTokenizer(line, "\"\' ", true);
        int state = 0;
        StringBuffer current = new StringBuffer();
        while (tokenizer.hasMoreTokens()) {
            String token = tokenizer.nextToken();
            switch (state) {
            case 1:
                if ("\'".equals(token)) {
                    state = 3;
                } else {
                    current.append(token);
                }
                break;
            case 2:
                if ("\"".equals(token)) {
                    state = 3;
                } else {
                    current.append(token);
                }
                break;
            default:
                if ("\'".equals(token)) {
                    state = 1;
                } else if ("\"".equals(token)) {
                    state = 2;
                } else if (" ".equals(token)) {
                    if (current.length() > 0) {
                        arguments.add(current.toString());
                        current = new StringBuffer();
                    }
                } else {
                    current.append(token);
                }
                break;
            }
        }
        if (current.length() > 0)
            arguments.add(current.toString());
        return arguments.toArray(new String[arguments.size()]);
    }

    /**
     * Get local machine ip address
     * */

    public static String getIPAddress() {
        InetAddress addr = null;
        try {
            addr = InetAddress.getLocalHost();
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }
        return addr.getHostAddress().toString();
    }
}
