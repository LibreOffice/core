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

package complex.tdoc;

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.CommandAbortedException;
import com.sun.star.ucb.CommandInfo;
import com.sun.star.ucb.GlobalTransferCommandArgument;
import com.sun.star.ucb.NameClash;
import com.sun.star.ucb.TransferCommandOperation;
import com.sun.star.ucb.XCommandInfo;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

/**
* Tests <code>XCommandProcessor</code>. The TestCase can pass (but doesn't have
* to) "XCommandProcessor.AbortCommand" relation, to specify command to abort in
* <code>abort()</code> test.
*
* Testing <code>com.sun.star.ucb.XCommandProcessor</code>
* interface methods :
* <ul>
*  <li><code> createCommandIdentifier()</code></li>
*  <li><code> execute()</code></li>
*  <li><code> abort()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XCommandProcessor.AbortCommand'</code> <b>optional</b>
*   (of type <code>com.sun.star.ucb.Command</code>):
*   specify command to abort in <code>abort()</code> test.
*   If the relation is not specified the 'GlobalTransfer'
*   command is used.</li>
* <ul> <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>poliball.gif</code> : this file is required in case
*   if the relation <code>'XCommandProcessor.AbortCommand'</code>
*   is not specified. This file is used by 'GlobalTransfer'
*   command as a source file for copying.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XCommandProcessor
*/
public class _XCommandProcessor {

    /**
     * Contains the tested object.
     */
    public XCommandProcessor oObj;
    private final LogWriter log = null;
    private XMultiServiceFactory xMSF = null;

    /**
     * Contains the command id returned by <code>createCommandIdentifier()
     * </code>. It is used in <code>abort()</code> test.
     */
    private int cmdId;

    public void before(XMultiServiceFactory _xMSF) {
        xMSF = _xMSF;
    }

    /**
     * Tests <code>createCommandIdentifier()</code>. Calls it for two times
     * and checks returned values. <p>
     * Has <b>OK</b> status if values are unique correct identifiers: not 0.
     */
    public boolean _createCommandIdentifier() {
        log.println("creating a command line identifier");

        int testCmdId = oObj.createCommandIdentifier();
        cmdId = oObj.createCommandIdentifier();

        if (cmdId == 0 || testCmdId == 0) {
            log.println("createCommandLineIdentifier() returned 0 - FAILED");
        }

        if (cmdId == testCmdId) {
            log.println("the command identifier is not unique");
        }

        return testCmdId != 0 && cmdId != 0 && cmdId != testCmdId;
    }

    /**
     * First executes 'geCommandInfo' command and examines returned
     * command info information. Second tries to execute in proper
     * command. <p>
     * Has <b> OK </b> status if in the first case returned information
     * contains info about 'getCommandInfo' command and in the second
     * case an exception is thrown. <p>
     */
    public boolean _execute() throws Exception {
        String[]commands = new String[] {"getCommandInfo", "getPropertySetInfo"};
        boolean returnVal = true;
        for (int j=0; j<commands.length; j++) {
            String commandName = commands[j];
            Command command = new Command(commandName, -1, null);

            Object result;

            log.println("executing command " + commandName);
            result = oObj.execute(command, 0, null);

            boolean found = false;

            XCommandInfo xCmdInfo = UnoRuntime.queryInterface(
                    XCommandInfo.class, result);
            if (xCmdInfo != null) {
                CommandInfo[] cmdInfo = xCmdInfo.getCommands();
                for (int i = 0; i < cmdInfo.length; i++) {
                    log.println("\t##### " + cmdInfo[i].Name + " - " + cmdInfo[i].Handle + " - " + cmdInfo[i].ArgType.getTypeName());
                    if (cmdInfo[i].Name.equals(commandName)) {
                        found = true;
//                      break;
                    }
                }
            }

            XPropertySetInfo xPropInfo = UnoRuntime.queryInterface(
                    XPropertySetInfo.class, result);
            if (xPropInfo != null) {
                Property[] props = xPropInfo.getProperties();
                String[] defProps = new String[] {"ContentType", "IsDocument", "IsFolder", "Title"};
                int propCount = defProps.length;
                for (int i = 0; i < props.length; i++) {
                    for (int k=0; k<defProps.length; k++) {
                        if (props[i].Name.equals(defProps[k])) {
                            propCount--;
                            log.println("Property '" + defProps[k] + "' has been found.");
                        }
                    }
                }
                found = propCount == 0;
            }

            returnVal &= found;

            if (!found) {
                log.println("Command '" + commandName + "' was not executed correctly.");
            }
        }
        return returnVal;
    }

    /**
     * First a separate thread where <code>abort</code> method
     * is called permanently. Then a "long" command (for example,
     * "transfer") is started. I case if relation is not
     * specified 'GlobalTransfer' command starts to
     * copy a file to temporary directory (if the relation is present
     * then the its command starts to work). <p>
     * Has <b> OK </b> status if the command execution is aborted, i.e.
     * <code>CommandAbortedException</code> is thrown. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> createCommandIdentifier() </code> : to have a unique
     *  identifier which is used to abort started command. </li>
     * </ul>
     */
    public boolean _abort() {

        Command command = new Command("getCommandInfo", -1, null);

        String commandName = "globalTransfer";

        String srcURL = util.utils.getFullTestURL("solibrary.jar") ;
        String tmpURL = util.utils.getOfficeTemp(xMSF) ;
        log.println("Copying '" + srcURL + "' to '" + tmpURL) ;

        GlobalTransferCommandArgument arg = new
            GlobalTransferCommandArgument(
                TransferCommandOperation.COPY, srcURL,
                    tmpURL, "", NameClash.OVERWRITE);

        command = new Command(commandName, -1, arg);

        Thread aborter = new Thread() {
            @Override
            public void run() {
                for (int i = 0; i < 10; i++) {
                    log.println("try to abort command");
                    oObj.abort(cmdId);
                    util.utils.pause(10);
                }
            }
        };

        aborter.start();

        util.utils.pause(15);

        log.println("executing command");
        try {
            oObj.execute(command, cmdId, null);
            log.println("Command execution completed");
            log.println("CommandAbortedException is not thrown");
            log.println("This is OK since there is no command implemented "+
                "that can be aborted");
        } catch (CommandAbortedException e) {
            return true;
        } catch (Exception e) {
            log.println("Unexpected exception " + e.getMessage());
            e.printStackTrace((java.io.PrintWriter)log);
            return false;
        }

        try {
            aborter.join(5000);
            aborter.interrupt();
        } catch(InterruptedException e) {
        }
        return true;
    }
}
