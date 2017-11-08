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

import java.io.PrintWriter;
import java.io.StringWriter;

import com.sun.star.logging.LogLevel;
import com.sun.star.logging.XLogHandler;
import com.sun.star.logging.XLogger;
import com.sun.star.logging.XLoggerPool;
import com.sun.star.uno.DeploymentException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class EventLogger {
    protected XComponentContext context;
    private String loggerName;
    private XLogger logger;

    public static XLoggerPool getLoggerPool(XComponentContext context) {
        Object loggerPoolObject = context.getValueByName("/singletons/com.sun.star.logging.LoggerPool");
        XLoggerPool loggerPool = UnoRuntime.queryInterface(XLoggerPool.class, loggerPoolObject);
        if (loggerPool == null) {
            throw new DeploymentException(
                    "component context fails to supply singleton com.sun.star.logging.LoggerPool of type com.sun.star.logging.XLoggerPool",
                    context);
        }
        return loggerPool;
    }

    public EventLogger(XComponentContext context) {
        this(context, "");
    }

    /**
     * Creates an <code>EventLogger</code> instance working with a css.logging.XLogger
     * instance given by name.
     *
     * @param context
     *    the component context to create services.
     * @param loggerName
     *    the name of the logger to create. If empty, the office-wide default logger will be used.
     */
    public EventLogger(XComponentContext context, String loggerName) {
        this.context = context;
        this.loggerName = loggerName;

        try {
            XLoggerPool loggerPool = getLoggerPool(context);
            if (!loggerName.isEmpty()) {
                logger = loggerPool.getNamedLogger(loggerName);
            } else {
                logger = loggerPool.getDefaultLogger();
            }
        } catch (com.sun.star.uno.RuntimeException exception) {
        }
    }

    /**
     * Returns the name of the logger
     */
    public String getName() {
        return loggerName;
    }

    /// Returns the current log level threshold of the logger.
    public int getLogLevel() {
        try {
            if (logger != null) {
                return logger.getLevel();
            }
        } catch (com.sun.star.uno.RuntimeException exception) {
        }
        return LogLevel.OFF;
    }

    /// Sets a new log level threshold of the logger.
    void setLogLevel(int logLevel) {
        try {
            if (logger != null) {
                logger.setLevel(logLevel);
            }
        } catch (com.sun.star.uno.RuntimeException exception) {
        }
    }

    /// Determines whether an event with the given level would be logged.
    public boolean isLoggable(int logLevel) {
        if (logger == null) {
            return false;
        }

        try {
            return logger.isLoggable(logLevel);
        } catch (com.sun.star.uno.RuntimeException exception) {
        }

        return false;
    }

    /**
     * Adds the given log handler to the logger's set of handlers.
     *
     * Note that normally, you would not use this method: The logger implementations
     * initialize themselves from the configuration, where usually, a default log handler
     * is specified. In this case, the logger will create and use this handler.
     *
     * @return
     *   true if and only if the addition was successful (as far as this can be detected
     *   from outside the <code>XLogger</code>'s implementation.
     */
    public boolean addLogHandler(XLogHandler logHandler) {
        try {
            if (logger != null) {
                logger.addLogHandler(logHandler);
                return true;
            }
        } catch (com.sun.star.uno.RuntimeException exception) {
        }
        return false;
    }

    /** removes the given log handler from the logger's set of handlers.
     *
     * @return
     *   true if and only if the addition was successful (as far as this can be detected
     *   from outside the <code>XLogger</code>'s implementation.
     */
    public boolean removeLogHandler(XLogHandler logHandler) {
        try {
            if (logger != null) {
                logger.removeLogHandler(logHandler);
                return true;
            }
        } catch (com.sun.star.uno.RuntimeException exception) {
        }
        return false;
    }

    /**
     * Logs a given message with its arguments, without the caller's class and method.
     * @param logLevel the log level
     * @param message the message to log
     * @param arguments the arguments to log, which are converted to strings and replace $1$, $2$, up to $n$ in the message
     * @return whether logging succeeded
     */
    public boolean log(int logLevel, String message, Object... arguments) {
        if (isLoggable(logLevel))
            return impl_log(logLevel, null, null, message, arguments);
        return false;
    }

    /**
     * Logs the given exception.
     * @param logLevel the log level
     * @param exception the exception
     * @return whether logging succeeded
     */
    public boolean log(int logLevel, Throwable exception) {
        return log(logLevel, "", exception);
    }

    /**
     * Logs the given message and exception.
     * @param logLevel the log level
     * @param message the message
     * @param exception the exception
     * @return whether logging succeeded
     */
    public boolean log(int logLevel, String message, Throwable exception) {
        if (isLoggable(logLevel)) {
            StringWriter stringWriter = new StringWriter();
            PrintWriter printerWriter = new PrintWriter(stringWriter);
            exception.printStackTrace(printerWriter);
            message += "\n" + stringWriter.getBuffer().toString();
            return impl_log(logLevel, null, null, message);
        }
        return true;
    }

    /**
     * Logs a given message with its arguments, with the caller's class and method
     * taken from a (relatively costly!) stack trace.
     * @param logLevel the log level
     * @param message the message to log
     * @param arguments the arguments to log, which are converted to strings and replace $1$, $2$, up to $n$ in the message
     * @return whether logging succeeded
     */
    public boolean logp(int logLevel, String message, Object...arguments) {
        if (isLoggable(logLevel)) {
            StackTraceElement caller = Thread.currentThread().getStackTrace()[2];
            return impl_log(logLevel, caller.getClassName(), caller.getMethodName(), message, arguments);
        }
        return false;
    }

    protected boolean impl_log(int logLevel, String sourceClass, String sourceMethod, String message, Object... arguments) {
        if (logger == null) {
            return false;
        }
        try {
            for (int i = 0; i < arguments.length; i++) {
                String placeholder = "$" + (i+1) + "$";
                int position = message.indexOf(placeholder);
                if (position >= 0) {
                    message = message.substring(0, position) + arguments[i].toString() +
                            message.substring(position + placeholder.length());
                }
            }

            if (sourceClass != null && sourceMethod != null) {
                logger.logp(logLevel, sourceClass, sourceMethod, message);
            } else {
                logger.log(logLevel, message);
            }
        } catch (com.sun.star.uno.RuntimeException exception) {
        }
        return true;
    }
}
