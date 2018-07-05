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
package helper;

import java.io.InputStream;
import java.io.File;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.PrintWriter;
import java.io.PrintStream;
import java.io.LineNumberReader;
import java.io.InputStreamReader;
import lib.TestParameters;
import util.PropertyName;
import util.utils;

/*
 * Class collect information from input stream in
 * background (separate thread) and outputs it to
 * some log stream. I helps to avoid buffer overflow
 * when output stream has small buffer size (e.g.
 * in case when handling stdout from external
 * <code>Process</code>)
 *
 * This class is currently used by ProcessHandler
 * internally only.
 */
class Pump extends Thread
{

    private final LineNumberReader reader;
    private final String pref;
    private final StringBuffer buf = new StringBuffer(256);
    private final PrintWriter log;
    private final boolean bOutput;

    /**
     * Creates Pump for specified <code>InputStream</code>.
     * This Pump also synchronously output text read to
     * log by prefixed lines. Constructor immediately
     * starts reading in a separate thread.
     *
     * @param is Stream which requires permanent reading.
     * @param log Writer where prefixed text lines to be output
     * @param outPrefix A prefix which is printed at the
     *   beginning of each output line.
     */
    public Pump(InputStream is, PrintWriter log, String outPrefix, boolean _bOutput)
    {
        this.pref = (outPrefix == null) ? "" : outPrefix;
        reader = new LineNumberReader(new InputStreamReader(is));
        this.log = log;
        this.bOutput = _bOutput;
        start();
    }

    @Override
    public void run()
    {
        try
        {
            String line = reader.readLine();
            while (line != null)
            {
                if (bOutput)
                {
                    log.println(pref + line);
                    log.flush();
                }
                buf.append(line).append('\n');
                line = reader.readLine();
            }
        }
        catch (java.io.IOException e)
        {
            log.println(pref + "Exception occurred: " + e);
        }
    }

    /**
     * Returns the text collected from input stream.
     */
    public String getStringBuffer()
    {
        return buf.toString();
    }
}

/**
 * Class provides convenient way for running external program
 * handle its standard streams, control execution and check results.
 * Instance of this class must be created only for a single
 * execution. If you need to execute the same command again you
 * should create a new instance for this.
 */
public class ProcessHandler
{
    private String cmdLine;
    private String[] envVars = null;
    private File workDir = null;
    private PrintWriter log;
    private int exitValue = -1;
    private boolean isFinished = false;
    private boolean isStarted = false;
    private PrintStream stdIn = null;
    private Process m_aProcess = null;
    private boolean debug = false;
    private boolean bUseOutput = true;

    private int m_nProcessTimeout = 0;
    private ProcessWatcher m_aWatcher;

    /**
     * Creates instance with specified external command and
     * log stream where debug info and output
     * of external command is printed out.
     */
    public ProcessHandler(String cmdLine, PrintWriter log) throws UnsupportedEncodingException
    {
        this(cmdLine, log, null, null);
    }

    /**
     * Creates instance with specified external command which
     * will be executed in the same work directory and
     *
     * @param cmdLine       the command to be executed
     * @param log           log stream where debug info and output
     *                      of external command is printed .
     * @param workDir       The working directory of the new process
     * @param envVars       The specified environment variables are
     *                      set for the new process.
     *                      If log stream is null, logging is printed to stdout.
     * @param  timeOut      When started synchronously, the maximum time the
     *                      process will live. When the process being destroyed
     *                      a log will be written out. It can be asked on
     *                      <code>isTimedOut()</code> if it has been terminated.
     *
     *                      timeOut > 0
     *                      Waits specified time in milliSeconds for
     *                      process to exit and return its status.
     *
     *                      timeOut = 0
     *                      Waits for the process to end regulary
     *
     */
    private ProcessHandler(String cmdLine, PrintWriter log, File workDir, String[] envVars) throws UnsupportedEncodingException
    {
        this.cmdLine = cmdLine;
        this.workDir = workDir;
        this.log = log;
        this.envVars = envVars;
        if (log == null)
        {
            this.log = new PrintWriter(new OutputStreamWriter(System.out, "UTF-8"));
        }
        else
        {
            this.log = log;
        }
    }

    /**
     * Executes the command immediately returns. The process
     * remains in running state.
     *
     * @return <code>true</code> if process was successfully
     * started.
     */
    public boolean executeAsynchronously()
    {
        execute();
        return isStarted();
    }

    public synchronized void kill()
    {
        if (!isStarted())
        {
            return;
        }
        boolean exit = false;
        int counter = 1;
        while (counter < 3 && !exit)
        {
            m_aProcess.destroy();

            util.utils.pause(1000 * counter);
            try
            {
                final int exit_Value = m_aProcess.exitValue();
                if (exit_Value < 1)
                {
                    exit = true;
                }
                else
                {
                    counter++;
                }
                dbg("kill: process closed with exit code " + exit_Value);
            }
            catch (java.lang.IllegalThreadStateException e)
            {
                if (counter < 3)
                {
                    dbg("kill: Couldn't close process after " + counter + " attempts, trying again");
                }
                counter++;
            }
        }
        isStarted = false;
    }

    private void showEnvVars()
    {
        if (envVars != null)
        {
            for (int i = 0; i < envVars.length; i++)
            {
                log.println("env: " + envVars[i]);
            }
        }
        else
        {
            log.println("env: null");
        }
    }

    private void execute()
    {
        if (isStarted())
        {
            throw new RuntimeException(
                    "The process handler has already been executed.");
        }
        final Runtime runtime = Runtime.getRuntime();
        try
        {
            if (workDir != null)
            {
                log.println(utils.getDateTime() + "execute: Starting command: ");
                log.println(cmdLine + " path=" + workDir.getAbsolutePath());
                showEnvVars();
                m_aProcess = runtime.exec(cmdLine, envVars, workDir);
            }
            else
            {
                log.println(utils.getDateTime() + "execute: Starting command: ");
                log.println(cmdLine);
                showEnvVars();
                m_aProcess = runtime.exec(cmdLine, envVars);
            }
            isStarted = true;
        }
        catch (java.io.IOException e)
        {
            log.println(utils.getDateTime() + "execute: The command " + cmdLine + " can't be started: " + e);
            return;
        }
        dbg("execute: pump io-streams");
        new Pump(m_aProcess.getInputStream(), log, "out > ", bUseOutput);
        new Pump(m_aProcess.getErrorStream(), log, "err > ", bUseOutput);
        stdIn = new PrintStream(m_aProcess.getOutputStream());

        dbg("execute: flush io-streams");

        flushInput();
    }

    private void flushInput()
    {
        if (stdIn == null)
        {
            return;
        }

        synchronized(this)
        {
            stdIn.flush();
        }
    }

    /**
     * Returns information about was the command started or
     * not.
     *
     * @return <code>true</code> if the external command was
     * found and successfully started.
     */
    private boolean isStarted()
    {
        return isStarted;
    }

    /**
     * Returns the information about the final state of command
     * execution.
     *
     * @return <code>true</code> if the command correctly starts,
     * exits and was not interrupted due to timeout.
     */
    private boolean isFinished()
    {
        return isFinished;
    }

    /**
     * Returns exit code of the external command.
     *
     * @return exit code of command if it was finished,
     * -1 if not.
     */
    public int getExitCode()
    {
        try
        {
            exitValue = m_aProcess.exitValue();
        }
        catch (Exception e)
        {
        }

        return exitValue;
    }

    private void dbg(String message)
    {
        if (debug)
        {
            log.println(utils.getDateTime() + "PH." + message);
        }
    }

    private static class ProcessWatcher extends Thread
    {

        private int m_nTimeoutInSec;
        private final boolean m_bInterrupt;

        private ProcessWatcher(int _nTimeOut)
        {
            m_nTimeoutInSec = _nTimeOut;
            m_bInterrupt = false;
        }

        /**
         * returns true, if the thread should hold on
         */
        public synchronized boolean isInHoldOn()
        {
            return m_bInterrupt;
        }
        @Override
        public void run()
        {
            while (m_nTimeoutInSec > 0)
            {
                m_nTimeoutInSec--;
                util.utils.pause(1000);
                if (isInHoldOn())
                {
                    break;
                }
            }
        }

    }

    /**
     *  If the timeout only given by setProcessTimeout(int seconds) function is != 0,
     *  a extra thread is created and after time has run out, the ProcessKiller string
     *  given by function setProcessKiller(string) will execute.
     *  So it is possible to kill a running office after a given time of seconds.
     */
    private void initializeProcessKiller()
    {
        if (m_nProcessTimeout != 0)
        {
            m_aWatcher = new ProcessWatcher(m_nProcessTimeout);
            m_aWatcher.start();
        }
    }


}
