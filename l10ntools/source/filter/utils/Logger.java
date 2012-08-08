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

package com.sun.star.filter.config.tools.utils;

import java.lang.*;
import java.util.*;

/** can be used to print out some debug messages
 *  and group it into warnings/errors or info statements.
 *
 *
 */
public class Logger
{
    /** only error message will be shown. */
    public static final int LEVEL_ERRORS = 1;

    /** only errors and warnings will be shown. */
    public static final int LEVEL_WARNINGS = 2;

    /** enable errors/warnings and some global info
     *  message. */
    public static final int LEVEL_GLOBALINFOS = 3;

    /** enable anything! */
    public static final int LEVEL_DETAILEDINFOS = 4;

    //___________________________________________
    // member

    /** enable/disable different output level.
     *  e.g. warnings/errors/infos */
    private int m_nLevel;

    //___________________________________________
    // ctor

    /** initialize new debug object with the specified
     *  debug level.
     *
     *  @param  nLevel
     *          the new debug level.
     *          See const values LEVEL_xxx too.
     */
    public Logger(int nLevel)
    {
        m_nLevel = nLevel;
    }

    //___________________________________________
    // interface

    /** initialize new debug object with a default
     *  debug level.
     */
    public Logger()
    {
        m_nLevel = LEVEL_DETAILEDINFOS;
    }

    //___________________________________________
    // interface

    /** prints out an exception ... if the right level is set.
     *
     *  @param  ex
     *          the exception object
     */
    public synchronized void setException(java.lang.Throwable ex)
    {
        if (m_nLevel >= LEVEL_ERRORS)
        {
            System.err.println("Exception:\n");
            ex.printStackTrace();
        }
    }

    //___________________________________________
    // interface

    /** prints out an error ... if the right level is set.
     *
     *  @param  sError
     *          the error message.
     */
    public synchronized void setError(java.lang.String sError)
    {
        if (m_nLevel >= LEVEL_ERRORS)
            System.err.println("Error    :\t\""+sError+"\"");
    }

    //___________________________________________
    // interface

    /** prints out a warning ... if the right level is set.
     *
     *  @param  sWarning
     *          the warning message.
     */
    public synchronized void setWarning(java.lang.String sWarning)
    {
        if (m_nLevel >= LEVEL_WARNINGS)
            System.err.println("Warning  :\t\""+sWarning+"\"");
    }

    //___________________________________________
    // interface

    /** prints out a global info message ... if the right level is set.
     *
     *  Global infos should be used to describe a complex operation.
     *  E.g.: loading of a document.
     *  But not for every sub operation like e.g. analyzing lines
     *  during loading the document!
     *
     *  @param  sInfo
     *          the info message.
     */
    public synchronized void setGlobalInfo(java.lang.String sInfo)
    {
        if (m_nLevel >= LEVEL_GLOBALINFOS)
            System.out.println("Info     :\t\""+sInfo+"\"");
    }

    //___________________________________________
    // interface

    /** prints out a mode detailed info message ... if the right level is set.
     *
     *  Such detailed message are e.g. "analyze line [n] of file ...".
     *
     *  @param  sInfo
     *          the info message.
     */
    public synchronized void setDetailedInfo(java.lang.String sInfo)
    {
        if (m_nLevel >= LEVEL_DETAILEDINFOS)
            System.out.println("Detail   :\t\""+sInfo+"\"");
    }
}
