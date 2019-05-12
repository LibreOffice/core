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


package helper;

import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import lib.TestParameters;
import share.CwsDataExchange;
import share.LogWriter;
import util.PropertyName;
import util.utils;

/**
 * Implementaion of the interface CwsDataExchange
 * @see share.CwsDataExchange
 */
public class CwsDataExchangeImpl implements CwsDataExchange
{

    private final String cwsName;
    private final TestParameters param;
    private final LogWriter log;
    private final BuildEnvTools bet;
    private final boolean mDebug;

    public CwsDataExchangeImpl(String cwsName, TestParameters param, LogWriter log) throws ParameterNotFoundException
    {
        this.cwsName = cwsName;
        this.param = param;
        this.log = log;
        this.bet = new BuildEnvTools(param, log);
        mDebug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);
    }

    public ArrayList getModules()
    {
        // the cwstouched command send its version information to StdErr.
        // A piping from StdErr to SdtOut the tcsh does not support.
        // To find the output easily the echo command is used
        final String[] commands =
        {
            "echo cwstouched starts here",
            "cwstouched",
            "echo cwstouched ends here"
        };

        final ProcessHandler procHdl = bet.runCommandsInEnvironmentShell(commands, null, 20000);

        if (mDebug)
        {
            log.println("---> Output of getModules:");
            log.println(procHdl.getOutputText());
            log.println("<--- Output of getModules");
            log.println("---> Error output of getModules");
            log.println(procHdl.getErrorText());
            log.println("<--- Error output of getModules");
        }

        final String[] outs = procHdl.getOutputText().split("\n");

        final ArrayList<String> moduleNames = new ArrayList<String>();
        boolean bStart = false;
        for (int i = 0; i < outs.length; i++)
        {
            final String line = outs[i];
            if (line.startsWith("cwstouched starts here"))
            {
                bStart = true;
                continue;
            }
            if (line.startsWith("cwstouched ends here"))
            {
                bStart = false;
                continue;
            }
            if (bStart && line.length() > 1)
            {
                moduleNames.add(line);
            }
        }

        return moduleNames;
    }

    public void setUnoApiCwsStatus(boolean status)
    {

        FileWriter out = null;
        String statusFile = null;
        try
        {

            final String stat = status ? ".PASSED.OK" : ".PASSED.FAILED";

            statusFile = utils.getUsersTempDir() +
                    System.getProperty("file.separator") +
                    "UnoApiCwsStatus." +
                    (String) param.get(PropertyName.VERSION) +
                    "_" + param.get(PropertyName.OPERATING_SYSTEM) + stat + ".txt";

            out = new FileWriter(statusFile);

            out.write(stat);
            out.flush();
            out.close();

            final String[] commands =
            {
                "cwsattach " + statusFile
            };

            bet.runCommandsInEnvironmentShell(commands, null, 5000);

        }
        catch (IOException ex)
        {
            System.out.println("ERROR: could not attach file '" + statusFile + "' to cws\n" + ex.toString());
        }
        finally
        {
            try
            {
                out.close();
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
            }
        }
    }
}
