/*************************************************************************
 *
 *  $RCSfile: _XCommandProcessor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:23:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.ucb;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.CommandAbortedException;
import com.sun.star.ucb.CommandInfo;
import com.sun.star.ucb.GlobalTransferCommandArgument;
import com.sun.star.ucb.NameClash;
import com.sun.star.ucb.TransferCommandOperation;
import com.sun.star.ucb.UnsupportedCommandException;
import com.sun.star.ucb.XCommandInfo;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;

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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XCommandProcessor
*/
public class _XCommandProcessor extends MultiMethodTest {

    /**
     * Conatins the tested object.
     */
    public XCommandProcessor oObj;

    /**
     * Contains the command id returned by <code>createCommandIdentifier()
     * </code>. It is used in <code>abort()</code> test.
     */
    int cmdId;

    /**
     * Tests <code>createCommandIdentifier()</code>. Calls it for two times
     * and checks returned values. <p>
     * Has <b>OK</b> status if values are unique correct idenifiers: not 0.
     */
    public void _createCommandIdentifier() {
        log.println("creating a command line identifier");

        int testCmdId = oObj.createCommandIdentifier();
        cmdId = oObj.createCommandIdentifier();

        if (cmdId == 0 || testCmdId == 0) {
            log.println("createCommandLineIdentifier() returned 0 - FAILED");
        }

        if (cmdId == testCmdId) {
            log.println("the command identifier is not unique");
        }

        tRes.tested("createCommandIdentifier()",
                testCmdId != 0 && cmdId != 0 && cmdId != testCmdId);
    }

    /**
     * First executes 'geCommandInfo' command and examines returned
     * command info information. Second tries to execute inproper
     * command. <p>
     * Has <b> OK </b> status if in the first case returned information
     * contains info about 'getCommandInfo' command and in the second
     * case an exception is thrown. <p>
     */
    public void _execute() {
        String commandName = "getCommandInfo";
        Command command = new Command(commandName, -1, null);

        Object result;

        log.println("executing command " + commandName);
        try {
            result = oObj.execute(command, 0, null);
        } catch (CommandAbortedException e) {
            log.println("The command aborted " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        } catch (Exception e) {
            log.println("Unexpected exception " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        XCommandInfo xCmdInfo = (XCommandInfo)UnoRuntime.queryInterface(
                XCommandInfo.class, result);

        CommandInfo[] cmdInfo = xCmdInfo.getCommands();

        boolean found = false;

        for (int i = 0; i < cmdInfo.length; i++) {
            if (cmdInfo[i].Name.equals(commandName)) {
                found = true;
                break;
            }
        }

        log.println("testing execute with wrong command");

        Command badCommand = new Command("bad command", -1, null);

        try {
            oObj.execute(badCommand, 0, null);
        } catch (CommandAbortedException e) {
            log.println("CommandAbortedException thrown - OK");
        } catch (UnsupportedCommandException e) {
            log.println("UnsupportedCommandException thrown - OK");
        } catch (Exception e) {
            log.println("Wrong exception thrown " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        tRes.tested("execute()", found);
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
     *  identifier which is used to abourt started command. </li>
     * </ul>
     */
    public void _abort() {
        executeMethod("createCommandIdentifier()");

        Command command = (Command)tEnv.getObjRelation(
                "XCommandProcessor.AbortCommand");

        if (command == null) {
            String commandName = "globalTransfer";

            String srcURL = util.utils.getFullTestURL("solibrary.jar") ;
            String tmpURL = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            log.println("Copying '" + srcURL + "' to '" + tmpURL) ;

            GlobalTransferCommandArgument arg = new
                GlobalTransferCommandArgument(
                    TransferCommandOperation.COPY, srcURL,
                        tmpURL, "", NameClash.OVERWRITE);

            command = new Command(commandName, -1, arg);
        }

        Thread aborter = new Thread() {
            public void run() {
                for (int i = 0; i < 10; i++) {
                    log.println("try to abort command");
                    oObj.abort(cmdId);
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                    }
                }
            }
        };

        aborter.start();

        try {
            Thread.sleep(15);
        } catch (InterruptedException e) {
        }

        log.println("executing command");
        try {
            oObj.execute(command, cmdId, null);
            log.println("Command execution completed");
            log.println("CommandAbortedException is not thrown");
            log.println("This is OK since there is no command implemented "+
                "that can be aborted");
            tRes.tested("abort()", true);
        } catch (CommandAbortedException e) {
            tRes.tested("abort()", true);
        } catch (Exception e) {
            log.println("Unexpected exception " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        try {
            aborter.join(5000);
            aborter.interrupt();
        } catch(java.lang.InterruptedException e) {
        }
    }
}
