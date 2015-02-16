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

package com.sun.star.script.framework.log;

import java.io.StringWriter;
import java.io.PrintWriter;

public class LogUtils {

    private static boolean m_bDebugEnabled = false;

    static {
        String debugFlag =
            System.getProperties().getProperty("ScriptJavaRuntimeDebug");

        if (debugFlag != null && debugFlag.length() > 0) {
            m_bDebugEnabled = debugFlag.equalsIgnoreCase("true");
        }
    }

    // Ensure that instances of this class cannot be created
    private LogUtils() {
    }

    /**
    *  Print Debug Output
    *
    * @param  msg  message to be displayed
    */
    public static void DEBUG(String msg) {
        if (m_bDebugEnabled) {
            System.out.println(msg);
        }
    }

    public static String getTrace(Exception e) {
        StringWriter w = new StringWriter();
        e.printStackTrace(new PrintWriter(w));
        return w.toString();
    }
}
