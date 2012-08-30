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

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;
import java.util.logging.Level;
import java.util.logging.LogManager;

import org.junit.internal.AssumptionViolatedException;
import org.junit.rules.TestRule;
import org.junit.runner.Description;
import org.junit.runners.model.Statement;


public class Logger extends java.util.logging.Logger implements TestRule {
    private static final String SCREENSHOT_DIR = "output/screenshot";

    private static String curClassName;

    private boolean screenshotEnabled = true;

    static {
        File logDir = Testspace.getFile("log");
        logDir.mkdirs();
        String loggingProperties =
                "java.util.logging.ConsoleHandler.level=INFO\n"
                + "java.util.logging.ConsoleHandler.formatter=java.util.logging.SimpleFormatter\n"
                + "java.util.logging.FileHandler.level=INFO\n"
                + "java.util.logging.FileHandler.formatter=java.util.logging.SimpleFormatter\n"
                + "java.util.logging.FileHandler.limit=1024000\n"
                + "java.util.logging.FileHandler.count=1\n"
                + "java.util.logging.FileHandler.pattern=" + logDir.getAbsolutePath().replace("\\", "/") + "/%u.log\n"
                + "java.util.logging.FileHandler.append=true\n"
                + "handlers= java.util.logging.ConsoleHandler,java.util.logging.FileHandler ";
        InputStream inputStream = new ByteArrayInputStream(loggingProperties.getBytes());
        LogManager logManager = LogManager.getLogManager();
        try {
            logManager.readConfiguration(inputStream);
        } catch (Exception e) {
            // ignore
        }

    }

    private Logger(String name) {
        super(name, null);
    }

    public static Logger getLogger(Object obj, boolean screenshotEnabled) {
        Logger logger = getLogger(obj);
        logger.setScreenshotEnabled(screenshotEnabled);
        return logger;
    }

    public static Logger getLogger(String name, boolean screenshotEnabled) {
        Logger logger = getLogger(name);
        logger.setScreenshotEnabled(screenshotEnabled);
        return logger;
    }

    @SuppressWarnings("rawtypes")
    public static Logger getLogger(Object obj) {
        if (obj == null)
            return getLogger("global");
        return getLogger(obj instanceof Class ? ((Class) obj).getName() : obj.getClass().getName());
    }

    public static synchronized Logger getLogger(String name) {
        LogManager manager = LogManager.getLogManager();
        Logger result = (Logger) manager.getLogger(name);
        if (result == null) {
            result = new Logger(name);
            manager.addLogger(result);
            result = (Logger) manager.getLogger(name);
        }
        return result;
    }

    protected void start(Description description) {
        if (!description.getClassName().equals(curClassName)) {
            curClassName = description.getClassName();
            log(Level.INFO, "Start running test class [" + curClassName + "]");
            File temp = Testspace.getFile("classtemp");
            FileUtil.deleteFile(temp);
            log(Level.INFO, "Clean up temp directory for test class [" + temp.getAbsolutePath() + "]");
            temp.mkdirs();
        }
        log(Level.INFO, "Start running test method [" + description.getMethodName() + "]");
        File temp = Testspace.getFile("temp");
        FileUtil.deleteFile(temp);
        log(Level.INFO, "Clean up temp directory for test method [" + temp.getAbsolutePath() + "]");
        temp.mkdirs();
    }

    protected void fail(Throwable e, Description description) {
        String screenshotType = e instanceof AssertionError ? "failure" : "error";
        File screenshotOutput = Testspace.getFile(SCREENSHOT_DIR);
        screenshotOutput.mkdirs();
        File file = new File(screenshotOutput, description.getClassName() + "." + description.getMethodName() + "." + screenshotType + ".png");
        GraphicsUtil.screenShot(file.getAbsolutePath());
        log(Level.SEVERE, "[" + description.getMethodName() + "] is failed. Screenshot [" + file.getAbsolutePath() + "]. Cause: ", e);
        // Check if crash occurs!
        // if (e instanceof CommunicationException) {
        // logger.severe("Pay attention! OpenOffice maybe crashed or freezed. ");
        // // If testcase is failed, kill AOO to avoid impacting the following
        // // test cases.
        // OpenOffice.killAll();
        // }

    }

    public boolean isScreenshotEnabled() {
        return screenshotEnabled;
    }

    public void setScreenshotEnabled(boolean screenshotEnabled) {
        this.screenshotEnabled = screenshotEnabled;
    }

    private void finish(Description description) {
        log(Level.INFO, "Finish running test method [" + description.getMethodName() + "]");
    }

    @Override
    public Statement apply(final Statement base, final Description description) {
        return new Statement() {
            @Override
            public void evaluate() throws Throwable {
                start(description);
                try {
                    base.evaluate();
                    // succeeded(description);
                } catch (AssumptionViolatedException e) {
                    throw e;
                } catch (Throwable t) {
                    fail(t, description);
                    throw t;
                } finally {
                    finish(description);
                }
            }
        };

    }
}
