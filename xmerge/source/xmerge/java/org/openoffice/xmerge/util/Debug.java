/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge.util;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.FileWriter;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.Writer;
import java.text.DateFormat;
import java.util.Date;
import java.util.Calendar;
import java.util.Properties;

/**
 *  This class is used for logging debug messages.
 *  Currently, there are three types of logging:  {@link #INFO},
 *  {@link #TRACE} & {@link #ERROR}.  Use the Debug.properties
 *  file to set or unset each type.  Also use Debug.properties
 *  to set the writer to either <code>System.out</code>,
 *  <code>System.err</code>, or to a file.
 *
 */
public final class Debug {

    /**  Informational messages. */
    public final static int INFO = 0x0001;
    /**  Error messages. */
    public final static int ERROR = 0x0002;
    /**  Trace messages. */
    public final static int TRACE = 0x0004;

    /**  To set a flag. */
    public final static boolean SET = true;
    /**  To unset a flag. */
    public final static boolean UNSET = false;

    private static int flags = 0;
    private static PrintWriter writer = null;

    static {

        try {

            Class<? extends Debug> c = new Debug().getClass();
            InputStream is = c.getResourceAsStream("Debug.properties");
            Properties props = new Properties();
            props.load(is);

            String info = props.getProperty("debug.info", "false");
            info = info.toLowerCase();

            if (info.equals("true")) {
                setFlags(Debug.INFO, Debug.SET);
            }

            String trace = props.getProperty("debug.trace", "false");
            trace = trace.toLowerCase();

            if (trace.equals("true")) {
                setFlags(Debug.TRACE, Debug.SET);
            }

            String error = props.getProperty("debug.error", "false");
            error = error.toLowerCase();

            if (error.equals("true")) {
                setFlags(Debug.ERROR, Debug.SET);
            }

            String w = props.getProperty("debug.output", "System.out");
            setOutput(w);

        } catch (Throwable ex) {

            ex.printStackTrace(System.err);
        }
    }


    /**
     *  Private constructor so as not to allow any instances
     *  of this class.  This serves as a singleton class.
     */
    private Debug() {
    }


    /**
     *  Set the output to the specified argument.
     *  This method is only used internally to prevent
     *  invalid string parameters.
     *
     *  @param  str  Output specifier.
     */
    private static void setOutput(String str) {

        if (writer == null) {

            if (str.equals("System.out")) {

                setOutput(System.out);

            } else if (str.equals("System.err")) {

                setOutput(System.err);

            } else {

                try {

                    setOutput(new FileWriter(str));

                } catch (IOException e) {

                    e.printStackTrace(System.err);
                }
            }
        }
    }


    /**
     *  Set the output to an <code>OutputStream</code> object.
     *
     *  @param  stream  OutputStream object.
     */
    private static void setOutput(OutputStream stream) {

        setOutput(new OutputStreamWriter(stream));
    }


    /**
     *  Set the <code>Writer</code> object to manage the output.
     *
     *  @param  w  <code>Writer</code> object to write out.
     */
    private static void setOutput(Writer w) {

        if (writer != null) {

            writer.close();
        }

        writer = new PrintWriter(new BufferedWriter(w), true);
    }


    /**
     *  <p>
     *  This method sets the levels for debugging logs.
     *  Example calls:
     *  </p>
     *
     *  <blockquote><pre><code>
     *  Debug.setFlags( Debug.INFO, Debug.SET )
     *  Debug.setFlags( Debug.TRACE, Debug.SET )
     *  Debug.setFlags( Debug.INFO | Debug.TRACE, Debug.SET )
     *  Debug.setFlags( Debug.ERROR, Debug.UNSET )
     *  </code></pre></blockquote>
     *
     *  @param   f     Debug flag
     *  @param   set   Use Debug.SET to set, and Debug.UNSET to unset
     *                 the given flag.
     */
    private static void setFlags(int f, boolean set) {

        if (set) {
            flags |= f;
        } else {
            flags &= ~f;
        }
    }


    /**
     *  Prints out information regarding platform.
     */
    public static void logSystemInfo() {

        if (writer != null) {

            writer.println();
            writer.println("Platform Information:");
            writer.println("OS          : " + System.getProperty("os.name"));
            writer.println("Version     : " + System.getProperty("os.version"));
            writer.println("Platform    : " + System.getProperty("os.arch"));
            writer.println("JDK Version : " + System.getProperty("java.version"));
            writer.println("JDK Vendor  : " + System.getProperty("java.vendor"));
            writer.println();
        }
    }


    /**
     *  Prints out timestamp.
     */
    public static void logTime() {

        if (writer != null) {

            Date time = Calendar.getInstance().getTime();
            DateFormat dt = DateFormat.getDateTimeInstance(DateFormat.FULL, DateFormat.FULL);
            writer.println(dt.format(time));
        }
    }


    /**
     *  Checks if flag is set.
     *
     *  @return  true if info logging is on, otherwise false
     */
    public static boolean isFlagSet(int f) {

        return ((flags & f) != 0);
    }


    /**
     *  <p>Log message based on the flag type.</p>
     *
     *  <p>Example 1:</p>
     *
     *  <blockquote><pre><code>
     *  Debug.log(Debug.INFO, "info string here");
     *  </code></pre></blockquote>
     *
     *  <p>This logs the message during runtime if
     *  <code>debug.info</code> in the properties file is
     *  set to true.</p>
     *
     *  <p>Example 2:</p>
     *
     *  <blockquote><pre><code>
     *  Debug.log(Debug.INFO | Debug.TRACE, "info string here");
     *  </code></pre></blockquote>
     *
     *  <p>This logs the message during runtime if debug.info or debug.trace
     *  in the properties file is set to true.</p>
     *
     *  @param   flag  Log type, one of the Debug constants
     *                 {@link #INFO}, {@link #TRACE}, {@link #ERROR}
     *                 or a combination of which or'ed together.
     *  @param   msg   The message.
     */
    public static void log(int flag, String msg) {

        if (isFlagSet(flag)) {

            if (writer != null) {

                writer.println(msg);
            }
        }
    }


    /**
     *  Log message based on flag type plus print out stack trace
     *  of the exception passed in.  Refer to the other log method
     *  for description.
     *
     *  @param   flag  Log type, one of the Debug constants
     *                 {@link #INFO}, {@link #TRACE}, {@link #ERROR}
     *                 or a combination of which or'ed together.
     *  @param   msg   The message.
     *  @param   e     Throwable object.
     */
    public static void log(int flag, String msg, Throwable e) {

        if (isFlagSet(flag)) {

            if (writer != null) {

                writer.println(msg);
                if (e != null)
                    e.printStackTrace(writer);
            }
        }
    }


    /**
     *  Converts the given bytes to a <code>String</code> of
     *  Hex digits.
     *
     *  @param  bytes  <code>byte</code> array.
     *
     *  @return  Hex representation in a <code>String</code>.
     */
    public static String byteArrayToHexString(byte bytes[]) {

        StringBuffer buff = new StringBuffer();

        for (int i = 0; i < bytes.length; i++) {

            int ch = (bytes[i] & 0xff);
            String str = Integer.toHexString(ch);
            if (str.length() < 2)
                buff.append('0');
            buff.append(str);
            buff.append(' ');
        }

        return buff.toString();
    }
}

