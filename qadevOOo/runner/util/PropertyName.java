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
package util;

/**
 * This interfaces describes some key names which are used in <CODE>lib.TestParameters</CODE>.
 */

public interface PropertyName {
    /**
     * parameter name: "AppExecutionCommand"
     */
    final public static String APP_EXECUTION_COMMAND = "AppExecutionCommand";
    /**
     * parameter name: "AppKillCommand"
     */
    final public static String APP_KILL_COMMAND = "AppKillCommand";
    /**
     * parameter name: "ConnectionString"
     */
    final public static String CONNECTION_STRING = "ConnectionString";
    final public static String PIPE_CONNECTION_STRING = "PipeConnectionString";
    final public static String USE_PIPE_CONNECTION = "UsePipeConnection";

    /**
     * parameter name: "TestBase"
     */
    final public static String TEST_BASE = "TestBase";
    /**
     * parameter name: "TestDocumentPath"
     */
    final public static String TEST_DOCUMENT_PATH = "TestDocumentPath";
    /**
     * parameter name: "LoggingIsActive"
     */
    final public static String LOGGING_IS_ACTIVE = "LoggingIsActive";
    /**
     * parameter name: "DebugIsActive"
     */
    final public static String DEBUG_IS_ACTIVE = "DebugIsActive";
    /**
     * parameter name: "OutProduceer"
     */
    final public static String OUT_PRODUCER = "OutProducer";
    /**
     * parameter name: "ShortWait"
     */
    final public static String SHORT_WAIT= "ShortWait";
    /**
     * internal only, no parameter
     */
    final public static String OFFICE_PROVIDER = "OfficeProvider";
    /**
     * internal only, no parameter
     */
    final public static String OFFICE_WATCHER = "Watcher";
    /**
     * internal only, no parameter
     */
    final public static String LOG_WRITER = "LogWriter";
    /**
     * parameter name: "TimeOut"<p>
     * time out given in milli seconds
     */
    final public static String TIME_OUT = "TimeOut";
    /**
     * parameter name: "ThreadTimeOut"
     */
    final public static String THREAD_TIME_OUT = "ThreadTimeOut";
    /**
     * parameter name: "OfficeCloseTimeOut"
     */
    final public static String OFFICE_CLOSE_TIME_OUT = "OfficeCloseTimeOut";
    /**
     * parameter name: "OperatingSystem"
     */
    final public static String OPERATING_SYSTEM = "OperatingSystem";
    /**
     * parameter name: "AutoRestart"
     */
    final public static String AUTO_RESTART = "AutoRestart";
    /**
     * parameter name: "NewOfficeInstance"
     */
    final public static String NEW_OFFICE_INSTANCE = "NewOfficeInstance";
    /**
     * parameter name: "KeepDocument"
     */
    final public static String KEEP_DOCUMENT = "KeepDocument";
    /**
     * parameter name: "SRC_ROOT"<p>
     * path to the source root of OpenOffice.org
     */
    final public static String SRC_ROOT = "SRC_ROOT";
    /**
     * parameter name: "Version"<p>
     * the name of the version to test
     */
    final public static String VERSION = "Version";

    /**
     * parameter name "Shell"<p>
     * Path to a shell.
     * This shell is used to run some commands outside of Java
     * example: /bin/tcsh c:\\myShell\\myShell.exe
     */
    final public static String SHELL = "Shell";
    /**
     * parameter name "Cygwin"<p>
     * If Cygwin is set to TRUE it indicates if the runner runs in a Cygwin
     * environment
     */
    final public static String CYGWIN = "Cygwin";
    /**
     * parameter name: "NoCwsAttach"<p>
     * If this parameter is set to "true" , a status of CWS-UnoAPI-Tests was not attached to EIS<p>
     * @see tests.complex.unoapi.CheckModuleAPI
     */
    final public static String NO_CWS_ATTACH = "NoCwsAttach";
    /**
     * internal only, no parameter
     */
    final public static String WNTMSCI = "wntmsci";
    /**
     * internal only, no parameter
     */
    final public static String UNXLNGI = "unxlngi";
    /**
     * internal only, no parameter
     */
    final public static String UNXSOLS = "unxsols";
    /**
     * internal only, no parameter
     */
    final public static String UNXSOLI = "unxsoli";
    /**
     * internal only, no parameter
     */
    final public static String UNXMACXI = "unxmacxi";

    /**
     * can be used to dont backup the user layer, faster office start/stop but less secure default is to backup the user layer
     */
    final public static String DONT_BACKUP_USERLAYER = "DontBackupUserLayer";
}
