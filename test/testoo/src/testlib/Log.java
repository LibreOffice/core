/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testlib;

import java.io.File;
import java.io.IOException;
import java.util.logging.ConsoleHandler;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;

import org.junit.rules.TestWatcher;
import org.junit.runner.Description;
import org.openoffice.test.common.GraphicsUtil;
import org.openoffice.test.vcl.client.CommunicationException;


/**
 * The class is used to capture extra information during the testing, including
 * 1. Take a screenshot when testing is failed.
 * 2. Collect data as the clue when oo crashes.
 *
 */
public class Log extends TestWatcher {
    static File screenshotOutput = null;
    static File logOutput = null;
    static Logger logger;
    static {
        screenshotOutput =  Testspace.getFile("output/screenshot");
        screenshotOutput.mkdirs();

        logOutput = Testspace.getFile("output/logs");
        logOutput.mkdirs();
        try {
            logger = Logger.getLogger("vclauto");
            FileHandler fh = new FileHandler(logOutput.getAbsolutePath()+ "/" + "%u.log", true);
            logger.addHandler(fh);
            logger.setLevel(Level.ALL);
            SimpleFormatter sf = new SimpleFormatter();
            fh.setFormatter(sf);
//          ConsoleHandler ch = new ConsoleHandler();
//          logger.addHandler(ch);
//          logger.setLevel(Level.ALL);
//          ch.setFormatter(sf);
        } catch (IOException e) {
            //ignore;
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


        File file = new File(screenshotOutput, description.getClassName()+"."+description.getMethodName()+"." + screenshotType + ".png");
        GraphicsUtil.screenShot(file.getAbsolutePath());
        logger.log(Level.SEVERE, "Testing is failed. Screenshot: " + file.getAbsolutePath(), e);
        // Check if crash occurs!
        if (e instanceof CommunicationException) {
            logger.severe("Pay attention! OpenOffice maybe crashed or freezed. ");
            // If testcase is failed, kill AOO to avoid impacting the following test cases.
            UIMap.app.kill();
        }

    }

    public void info(String message) {
        logger.info(message);
    }
}
