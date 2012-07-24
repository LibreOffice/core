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

package testlib;

import java.io.File;
import java.io.IOException;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;

import org.junit.rules.TestWatcher;
import org.junit.runner.Description;
import org.openoffice.test.OpenOffice;
import org.openoffice.test.common.GraphicsUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.vcl.client.CommunicationException;

/**
 * The class is used to capture extra information during the testing, including
 * 1. Take a screenshot when testing is failed. 2. Collect data as the clue when
 * oo crashes.
 *
 */
public class Log extends TestWatcher {
    static File screenshotOutput = null;
    static File logOutput = null;
    static Logger logger;
    static {
        screenshotOutput = Testspace.getFile("output/screenshot");
        screenshotOutput.mkdirs();

        logOutput = Testspace.getFile("output/logs");
        logOutput.mkdirs();
        try {
            logger = Logger.getLogger("vclauto");
            FileHandler fh = new FileHandler(logOutput.getAbsolutePath() + "/" + "%u.log", true);
            logger.addHandler(fh);
            logger.setLevel(Level.ALL);
            SimpleFormatter sf = new SimpleFormatter();
            fh.setFormatter(sf);
            // ConsoleHandler ch = new ConsoleHandler();
            // logger.addHandler(ch);
            // logger.setLevel(Level.ALL);
            // ch.setFormatter(sf);
        } catch (IOException e) {
            // ignore;
        }
    }

    @Override
    protected void starting(Description description) {
        logger.info("Start " + description);
        super.starting(description);
    }

    @Override
    protected void failed(Throwable e, Description description) {
        super.failed(e, description);
        String screenshotType = null;
        if (e instanceof AssertionError) {
            screenshotType = "failure";
        } else {
            screenshotType = "error";
        }

        File file = new File(screenshotOutput, description.getClassName() + "." + description.getMethodName() + "." + screenshotType + ".png");
        GraphicsUtil.screenShot(file.getAbsolutePath());
        logger.log(Level.SEVERE, "Testing is failed. Screenshot: " + file.getAbsolutePath(), e);
        // Check if crash occurs!
        if (e instanceof CommunicationException) {
            logger.severe("Pay attention! OpenOffice maybe crashed or freezed. ");
            // If testcase is failed, kill AOO to avoid impacting the following
            // test cases.
            OpenOffice.killAll();
        }

    }

    public void info(String message) {
        logger.info(message);
    }
}
