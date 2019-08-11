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
     * The AppExecutionCmd contains the full qualified<br>
     * command to an Application to be started.
     */
    String APP_EXECUTION_COMMAND = "AppExecutionCommand";
    /**
     * parameter name: "ConnectionString"
     */
    String CONNECTION_STRING = "ConnectionString";
    String PIPE_CONNECTION_STRING = "PipeConnectionString";
    String USE_PIPE_CONNECTION = "UsePipeConnection";

    /**
     * parameter name: "TestBase"
     * The Testbase to be executed by the runner<br>
     * default is 'java_fat'
     */
    String TEST_BASE = "TestBase";
    /**
     * parameter name: "TestDocumentPath"
     */
    String TEST_DOCUMENT_PATH = "TestDocumentPath";
    /**
     * parameter name: "LoggingIsActive"
     * 'true' is a log should be written, 'false' elsewhere <br>
     * these will be provided by the testcases<br>
     * default is true
     */
    String LOGGING_IS_ACTIVE = "LoggingIsActive";
    /**
     * parameter name: "DebugIsActive"
     */
    String DEBUG_IS_ACTIVE = "DebugIsActive";
    /**
     * parameter name: "OutProducer"
     * This parameter contains the class used<br>
     * for Logging
     */
    String OUT_PRODUCER = "OutProducer";
    /**
     * internal only, no parameter
     * The OfficeProvider contains the full qualified
     * class that provides a connection to StarOffice<br>
     * default is helper.OfficeProvider
     */
    String OFFICE_PROVIDER = "OfficeProvider";
    /**
     * internal only, no parameter
     */
    String OFFICE_WATCHER = "Watcher";
    /**
     * internal only, no parameter
     * This parameter contains the class used<br>
     * for Logging
     */
    String LOG_WRITER = "LogWriter";
    /**
     * parameter name: "TimeOut"<p>
     * time out given in milliseconds
     * This parameter contains the timeout used<br>
     * by the watcher
     */
    String TIME_OUT = "TimeOut";
    /**
     * parameter name: "ThreadTimeOut"
     * This parameter contains the timeout used<br>
     * by the complex tests
     */
    String THREAD_TIME_OUT = "ThreadTimeOut";
    /**
     * parameter name: "UnoRcName"
     */
    String UNORC_NAME = "UnoRcName";
    /**
     * parameter name: "AutoRestart"
     * If this parameter is <CODE>true</CODE> the <CODE>OfficeProvider</CODE> tries
     * to get the URL to the binary of the office and to fill the
     * <CODE>AppExecutionCommand</CODE> with useful content if needed.
     * Default is false.
     */
    String AUTO_RESTART = "AutoRestart";
    /**
     * parameter name: "NewOfficeInstance"
     */
    String NEW_OFFICE_INSTANCE = "NewOfficeInstance";

    /**
     * parameter name: "SRC_ROOT"<p>
     * path to the source root of OpenOffice.org
     */
    String SRC_ROOT = "SRC_ROOT";
}
