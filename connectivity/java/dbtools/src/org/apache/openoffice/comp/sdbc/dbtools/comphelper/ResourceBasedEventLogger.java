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
package org.apache.openoffice.comp.sdbc.dbtools.comphelper;

import com.sun.star.lang.NullPointerException;
import com.sun.star.uno.XComponentContext;

public class ResourceBasedEventLogger extends EventLogger {
    private String resourceBundleBaseName;
    private OfficeResourceBundle resourceBundle;

    public ResourceBasedEventLogger(XComponentContext context, String resourceBundleBaseName, String loggerName) {
        super(context, loggerName);
        this.resourceBundleBaseName = resourceBundleBaseName;
        try {
            resourceBundle = new OfficeResourceBundle(context, resourceBundleBaseName);
        } catch (NullPointerException nullPointerException) {
            throw new RuntimeException(nullPointerException);
        }
    }

    public ResourceBasedEventLogger(ResourceBasedEventLogger logger) {
        super(logger.context, logger.getName());
        this.resourceBundleBaseName = logger.resourceBundleBaseName;
        try {
            resourceBundle = new OfficeResourceBundle(logger.context, logger.resourceBundleBaseName);
        } catch (NullPointerException nullPointerException) {
            throw new RuntimeException(nullPointerException);
        }
    }

    /**
     * Logs a given message with its arguments, without the caller's class and method.
     * @param logLevel the log level
     * @param messageResID the resource ID of the message to log
     * @param arguments the arguments to log, which are converted to strings and replace $1$, $2$, up to $n$ in the message
     * @return whether logging succeeded
     */
    public boolean log(int logLevel, int messageResID, Object... arguments) {
        if (isLoggable(logLevel))
            return impl_log(logLevel, null, null, loadStringMessage(messageResID), arguments);
        return false;
    }

    /**
     * Logs a given message with its arguments, with the caller's class and method
     * taken from a (relatively costly!) stack trace.
     * @param logLevel the log level
     * @param messageResID the resource ID of the message to log
     * @param arguments the arguments to log, which are converted to strings and replace $1$, $2$, up to $n$ in the message
     * @return whether logging succeeded
     */
    public boolean logp(int logLevel, int messageResID, Object... arguments) {
        if (isLoggable(logLevel)) {
            StackTraceElement caller = Thread.currentThread().getStackTrace()[2];
            return impl_log(logLevel, caller.getClassName(), caller.getMethodName(), loadStringMessage(messageResID), arguments);
        }
        return false;
    }

    private String loadStringMessage(int messageResID) {
        String message = resourceBundle.loadString(messageResID);
        if (message.isEmpty()) {
            message = String.format("<invalid event resource: '%s:%d'>", resourceBundleBaseName, messageResID);
        }
        return message;
    }
}
