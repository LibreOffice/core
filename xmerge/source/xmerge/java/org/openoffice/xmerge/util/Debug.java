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
import java.util.Properties;

/**
 * This class is used for logging debug messages.
 *
 * <p>Currently, there are three types of logging: {@link #INFO}, {@link #TRACE}
 * {@literal &} {@link #ERROR}.  Use the {@code Debug.properties} file to set or
 * unset each type.  Also use {@code Debug.properties} to set the writer to
 * either {@code System.out}, {@code System.err}, or to a file.</p>
 */
public final class Debug {

    /** Informational messages. */
    public static final int INFO = 0x0001;
    /** Error messages. */
    public static final int ERROR = 0x0002;
    /** Trace messages. */
    public static final int TRACE = 0x0004;

    /** To set a flag. */
    private static final boolean SET = true;

    private static int flags = 0;
    private static PrintWriter writer = null;

    static {

        InputStream is = null;
        try {
            try {
                is = Debug.class.getResourceAsStream("Debug.properties");
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

            } finally {
                if (is !=null)
                    is.close();
            }
        } catch (Throwable ex) {
            ex.printStackTrace(System.err);
        }
    }

    /**
     * Private constructor so as not to allow any instances  of this class.
     *
     * <p>This serves as a singleton class.</p>
     */
    private Debug() {
    }

    /**
     * Set the output to the specified argument.
     *
     * <p>This method is only used internally to prevent invalid string
     * parameters.</p>
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
     * Set the output to an {@code OutputStream} object.
     *
     * @param  stream  {@code OutputStream} object.
     */
    private static void setOutput(OutputStream stream) {
        setOutput(new OutputStreamWriter(stream));
    }

    /**
     * Set the {@code Writer} object to manage the output.
     *
     * @param  w  {@code Writer} object to write out.
     */
    private static void setOutput(Writer w) {
        if (writer != null) {
            writer.close();
        }
        writer = new PrintWriter(new BufferedWriter(w), true);
    }

    /**
     * This method sets the levels for debugging logs.
     *
     * <p>Example calls:</p>
     *
     * <blockquote><pre>{@code Debug.setFlags( Debug.INFO, Debug.SET )
     * Debug.setFlags( Debug.TRACE, Debug.SET )
     * Debug.setFlags( Debug.INFO | Debug.TRACE, Debug.SET )
     * Debug.setFlags( Debug.ERROR, Debug.UNSET )}</pre></blockquote>
     *
     * @param   f     Debug flag
     * @param   set   Use {@code Debug.SET} to set, and {@code Debug.UNSET} to
     *                unset the given flag.
     */
    private static void setFlags(int f, boolean set) {
        if (set) {
            flags |= f;
        } else {
            flags &= ~f;
        }
    }

    /**
     * Checks if flag is set.
     *
     * @return  {@code true} if info logging is on, otherwise {@code false}.
     */
    public static boolean isFlagSet(int f) {
        return ((flags & f) != 0);
    }

    /**
     * Log message based on the flag type.
     *
     * <p>Example 1:</p>
     *
     * <blockquote><pre>{@code Debug.log(Debug.INFO, "info string here");}</pre>
     * </blockquote>
     *
     * <p>This logs the message during runtime if {@code debug.info} in the
     * properties file is set to true.</p>
     *
     * <p>Example 2:</p>
     *
     * <blockquote>
     * <pre>{@code Debug.log(Debug.INFO | Debug.TRACE, "info string here");}</pre>
     * </blockquote>
     *
     * <p>This logs the message during runtime if debug.info or debug.trace in
     * the properties file is set to true.</p>
     *
     * @param  flag  Log type, one of the Debug constants {@link #INFO},
     *               {@link #TRACE}, {@link #ERROR} or a combination of which
     *               or'ed together.
     * @param  msg   The message.
     */
    public static void log(int flag, String msg) {
        if (isFlagSet(flag) && writer != null) {
            writer.println(msg);
        }
    }

    /**
     * Log message based on flag type plus print out stack trace of the
     * exception passed in.
     *
     * <p>Refer to the other log method for description.</p>
     *
     * @param   flag  Log type, one of the Debug constants {@link #INFO},
     *                {@link #TRACE}, {@link #ERROR} or a combination of which
     *                or'ed together.
     * @param   msg   The message.
     * @param   e     Throwable object.
     */
    public static void log(int flag, String msg, Throwable e) {
        if (isFlagSet(flag) && writer != null) {
            writer.println(msg);
            if (e != null)
                e.printStackTrace(writer);
        }
    }
}