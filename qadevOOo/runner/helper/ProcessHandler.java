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

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.File;
import java.io.PrintWriter;
import java.io.PrintStream;
import java.io.LineNumberReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import lib.TestParameters;
import util.PropertyName;
import util.utils;

/**
 * Class collect information from input stream in
 * background (sparate thread) and outputs it to
 * some log stream. I helps to avoid buffer overflow
 * when output stream has small buffer size (e.g.
 * in case when handling stdout from external
 * <code>Process</code>)
 *
 * This class is currently used by ProcesHandler
 * internally only.
 */
class Pump extends Thread
{

    private LineNumberReader reader;
    private String pref;
    private StringBuffer buf = new StringBuffer(256);
    private PrintWriter log;
    private boolean bOutput;

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
    private String[] cmdLineArray;
    private String[] envVars = null;
    private File workDir = null;
    private PrintWriter log;
    private int exitValue = -1;
    private boolean isFinished = false;
    private boolean isStarted = false;
    private boolean mbTimedOut = false;
    private long mTimeOut = 0;
    private String stdInBuff = "";
    private Pump stdout = null;
    private Pump stderr = null;
    private PrintStream stdIn = null;
    private Process m_aProcess = null;
    private TestParameters param = null;
    private boolean debug = false;
    private boolean bUseOutput = true;

    private int m_nProcessTimeout = 0;
    private String m_sProcessKiller;
    private ProcessWatcher m_aWatcher;

    /**
     * Creates instance with specified external command.
     * Debug info and output
     * of external command is printed to stdout.
     * @param cmdLine
     */
    public ProcessHandler(String cmdLine)
    {
        this(cmdLine, null, null, null, 0);
    }

    /**
     * Creates instance with specified external command
     * including parameters as an array.
     * Debug info and output
     * of external command is printed to stdout.
     * @param cmdLines
     */
    public ProcessHandler(String[] cmdLines)
    {
        this(null, null, null, null, 0);
        cmdLineArray = cmdLines;
    }

    /**
     * Creates instance with specified external command
     * including parameters as an array, with environment
     * variables.
     * Debug info and output
     * of external command is printed to stdout.
     * @param cmdLines
     * @param envVars
     * @see java.lang.Runtime exec(String[], String[])
     */
    public ProcessHandler(String[] cmdLines, String[] envVars)
    {
        this(null, null, null, envVars, 0);
        cmdLineArray = cmdLines;
    }

    /**
     * Creates instance with specified external command
     * including parameters as an array, with environment
     * variables. The command will be started in workDir.
     * Debug info and output
     * of external command is printed to stdout.
     * @param cmdLines
     * @param workDir
     */
    public ProcessHandler(String[] cmdLines, File workDir)
    {
        this(null, null, workDir, null, 0);
        cmdLineArray = cmdLines;

    }

    /**
     * Creates instance with specified external command and
     * log stream where debug info and output
     * of external command is printed out.  The command will be started in workDir.
     * @param cmdLines
     * @param log
     * @param workDir
     */
    public ProcessHandler(String[] cmdLines, PrintWriter log, File workDir)
    {
        this(null, log, workDir, null, 0);
        cmdLineArray = cmdLines;
    }

    /**
     * Creates instance with specified external command and
     * log stream where debug info and output
     * of external command is printed out.
     * @param cmdLine
     * @param log
     */
    public ProcessHandler(String cmdLine, PrintWriter log)
    {
        this(cmdLine, log, null, null, 0);
    }

    /**
     * Creates instance with specified external command and set the time out for the command.
     * @param cmdLine
     * @param timeOut
     */
    public ProcessHandler(String cmdLine, int timeOut)
    {
        this(cmdLine, null, null, null, timeOut);
    }

    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory.
     * Debug info and output
     * of external commandis printed to stdout.
     * @param cmdLine
     * @param workDir
     */
    public ProcessHandler(String cmdLine, File workDir)
    {
        this(cmdLine, null, workDir, null, 0);
    }

    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory.
     * Debug info and output printed in log stream.
     * @param cmdLine
     * @param log
     * @param workDir
     */
    public ProcessHandler(String cmdLine, PrintWriter log, File workDir)
    {
        this(cmdLine, log, workDir, null, 0);
    }

    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory  and
     * log stream where debug info and output
     * of external command is printed .
     * The specified environment variables are set for the new process.
     * If log stream is null, logging is printed to stdout.
     * @param cmdLine
     * @param log
     * @param workDir
     * @param envVars
     */
    public ProcessHandler(String cmdLine, PrintWriter log, File workDir, String[] envVars)
    {
        this(cmdLine, log, workDir, envVars, 0);
    }

    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory  and
     *
     * @param cmdLine       the command to be executed
     * @param log           log stream where debug info and output
     *                      of external command is printed .
     * @param workDir       The working directory of the new process
     * @param envVars       The specified environment variables are
     *                      set for the new process.
     *                      If log stream is null, logging is printed to stdout.
     * @param  timeOut      When started sychronisly, the maximum time the
     *                      process will live. When the process being destroyed
     *                      a log will be written out. It can be asked on
     *                      <code>isTimedOut()</code> if it has been terminated.
     *
     *                      timeOut > 0
     *                      Waits specified time in miliSeconds for
     *                      process to exit and return its status.
     *
     *                      timeOut = 0
     *                      Waits for the process to end regulary
     *
     *                      timeOut < 0
     *                      Kills the process immediately
     *
     *
     */
    public ProcessHandler(String cmdLine, PrintWriter log, File workDir, String[] envVars, long timeOut)
    {
        this.cmdLine = cmdLine;
        this.workDir = workDir;
        this.log = log;
        this.cmdLine = cmdLine;
        this.envVars = envVars;
        if (log == null)
        {
            this.log = new PrintWriter(new OutputStreamWriter(System.out));
        }
        else
        {
            this.log = log;
        }
        this.mTimeOut = timeOut;
    }

    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory  and
     * log stream where debug info and output of external command is printed.
     * If log stream is null, logging is printed to stdout.
     * From the <CODE>TestParameters</CODE> the <CODE>OfficeWachter</CODE> get a ping.
     * @param commands
     * @param log
     * @param workDir
     * @param shortWait If this parameter is ture the <CODE>mTimeOut</CODE> is set to 5000 ms, else it is set to
     *        half of time out from parameter timeout.
     * @param param the TestParameters
     * @see lib.TestParameters
     * @see helper.OfficeWatcher
     */
    public ProcessHandler(String[] commands, PrintWriter log, File workDir, int shortWait, TestParameters param)
    {
        this(null, log, workDir, null, 0);
        this.cmdLineArray = commands;
        this.param = param;
        if (shortWait != 0)
        {
            this.mTimeOut = shortWait;
        }
        else
        {
            this.mTimeOut = (long) (param.getInt(PropertyName.TIME_OUT) / 1.3);
        }
        debug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);

    }

    /**
     * If not equal 0, the time to maximal wait.
     * @param _n
     */
    public void setProcessTimeout(int _n)
    {
        m_nProcessTimeout = _n;
    }

    /**
     * This command will call after ProcessTimeout is arrived.
     * @param _s
     */
    public void setProcessKiller(String _s)
    {
        m_sProcessKiller = _s;
    }

    /**
     * This method do an asynchronous execution of the commands. To avoid a interruption on long running processes
     * caused by <CODE>OfficeWatcher</CODE>, the OfficeWatcher get frequently a ping.
     * @see helper.OfficeWatcher
     */
    public void runCommand()
    {

        boolean changedText = true;
        String memText = "";

        this.executeAsynchronously();

        OfficeWatcher ow = null;
        if (param != null)
        {
            ow = (OfficeWatcher) param.get(PropertyName.OFFICE_WATCHER);
        }
        if (ow != null)
        {
            ow.ping();
        }

        int hangcheck = 10;
        while (!this.isFinished() && changedText)
        {
            // dbg("runCommand: waiting " + mTimeOut / 1000 + " seconds while command execution is ongoing... " + count);
            // shortWait(mTimeOut);
            // shortWait(2000); // wait 2 seconds.
            //waitFor(mTimeOut);
            waitFor(2000, false); // wait but don't kill

            if (ow != null)
            {
                ow.ping();
            }
            // check for changes in the output stream. If there are no changes, the process maybe hangs
            if (!this.isFinished())
            {
                hangcheck--;
                if (hangcheck < 0)
                {
                    String sOutputText = getOutputText();
                    if (sOutputText.length() == memText.length())
                    {
                        changedText = false;
                        // dbg("runCommand Could not detect changes in output stream!!!");
                    }
                    hangcheck = 10;
                    memText = this.getOutputText();
                }
            }
        }

        if (!this.isFinished())
        {
            dbg("runCommand Process ist not finished but there are no changes in output stream.");
            this.kill();
        }
    }

    public boolean isTimedOut()
    {
        return mbTimedOut;
    }

    private void setTimedOut(boolean bTimedOut)
    {
        mbTimedOut = bTimedOut;
    }

    /**
     * Executes the command and returns only when the process
     * exits.
     *
     * @return <code>true</code> if process was successfully
     * started and correcly exits (exit code doesn't affect
     * to this result).
     */
    public boolean executeSynchronously()
    {
        execute();
        return waitFor(mTimeOut);
    }

    /**
     * Executes the command immediately returns. The process
     * remains in running state. Control of its state should
     * be made by <code>waitFor</code> methods.
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

            try
            {
                Thread.sleep(1000 * counter); // 5000
            }
            catch (java.lang.InterruptedException e)
            {
            }
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

    /**
     * Returns the time in seconds since 1st January 1970
     * @return
     */
    public static long getSystemTime()
    {
        // Calendar cal = new GregorianCalendar();
        // final long nTime = cal.getTimeInMillis();
        final long nTime = System.currentTimeMillis();
        return nTime;
    }
    private long m_nExactStartTimeInMillisec;

    private void initialExactStartTime()
    {
        m_nExactStartTimeInMillisec = getSystemTime();
    }

    public long getProcessStartTime()
    {
        return m_nExactStartTimeInMillisec;
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

    protected void execute()
    {
        if (isStarted())
        {
            throw new RuntimeException(
                    "The process handler has already been executed.");
        }
        final Runtime runtime = Runtime.getRuntime();
        try
        {
            if (cmdLine == null)
            {
                log.println(utils.getDateTime() + "execute: Starting command from array: ");
                for (int i = 0; i < cmdLineArray.length; i++)
                {
                    log.println(cmdLineArray[i]);
                    // log.print(" ");
                }
                showEnvVars();
                log.println("");
                initialExactStartTime();
                initializeProcessKiller();
                m_aProcess = runtime.exec(cmdLineArray, envVars);
            }
            else
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
            }
            isStarted = true;
        }
        catch (java.io.IOException e)
        {
            if (cmdLine == null)
            {
                log.println(utils.getDateTime() + "execute: The command array can't be started: " + e);
            }
            else
            {
                log.println(utils.getDateTime() + "execute: The command " + cmdLine + " can't be started: " + e);
            }
            return;
        }
        dbg("execute: pump io-streams");
        stdout = new Pump(m_aProcess.getInputStream(), log, "out > ", bUseOutput);
        stderr = new Pump(m_aProcess.getErrorStream(), log, "err > ", bUseOutput);
        stdIn = new PrintStream(m_aProcess.getOutputStream());

        // int nExitValue = m_aProcess.exitValue();
        // int dummy = 0;

        dbg("execute: flush io-streams");

        flushInput();
    }

    /**
     * This method is useful when the process was executed
     * asynchronously. Waits for process to exit and return
     * its result.
     *
     * @return <code>true</code> if process correctly exited
     * (exit code doesn't affect to this result).
     */
    public boolean waitFor()
    {
        return waitFor(0);
    }

    /**
     * This method is useful when the process was executed
     * asynchronously. Waits during specified time for process
     * to exit and return its status.
     *
     * @param timeout      > 0
     *                      Waits specified time in miliSeconds for
     *                      process to exit and return its status.
     *
     *                      = 0
     *                      Waits for the process to end regulary
     *
     *                      < 0
     *                      Kills the process immediately
     *
     * @return <code>true</code> if process correctly exited
     * (exit code doesn't affect to this result).
     */
    public boolean waitFor(long timeout)
    {
        return waitFor(timeout, true);
    }

    private boolean waitFor(long timeout, boolean bKillProcessAfterTimeout)
    {
        if (isFinished())
        {
            return true;
        }
        if (!isStarted())
        {
            return false;
        }

        if (timeout == 0)
        {
            try
            {
                m_aProcess.waitFor();
            }
            catch (InterruptedException e)
            {
                log.println("The process was interrupted: " + e);
            }
            isFinished = true;
            try
            {
                exitValue = m_aProcess.exitValue();
            }
            catch (IllegalThreadStateException e)
            {
            }
        }
        else
        {
            try
            {
                while (!isFinished && timeout > 0)
                {
                    isFinished = true;
                    Thread.sleep(1000);
                    timeout -= 1000;
                    try
                    {
                        exitValue = m_aProcess.exitValue(); // throws exception if not finished
                    }
                    catch (IllegalThreadStateException e)
                    {
                        isFinished = false;
                    }
                }
                if (timeout < 0)
                {
                    setTimedOut(true);
                    log.println("The process has timed out!");
                }
            }
            catch (InterruptedException ex)
            {
                log.println("The process was interrupted: " + ex);
            }
        }

        if (bKillProcessAfterTimeout == true)
        {
            if (!isFinished)
            {
                log.println("Going to destroy the process!!");
                m_aProcess.destroy();
                log.println("Process has been destroyed!");
            }
        }
//  Removed as hung up in SDK test 'PathSettings'
//        try {
//            stdout.join();
//            stderr.join();
//        } catch (InterruptedException e) {}

        return isFinished();
    }

    protected void flushInput()
    {
        if (stdIn == null)
        {
            return;
        }

        synchronized(stdInBuff)
        {
            stdIn.print(stdInBuff);
            stdIn.flush();
            stdInBuff = "";
        }
    }

    /**
     * Returns the text output by external command to stdout.
     * @return the text output by external command to stdout
     */
    public String getOutputText()
    {
        if (stdout == null)
        {
            return "";
        }
        else
        {
            return stdout.getStringBuffer();
        }
    }

    /**
     * Returns the text output by external command to stderr.
     * @return the text output by external command to stderr
     */
    public String getErrorText()
    {
        if (stderr == null)
        {
            return "";
        }
        else
        {
            return stderr.getStringBuffer();
        }
    }

    /**
     * Prints the string specified to sdtin of external
     * command. '\n' is not added so if you need you
     * should terminate the string with '\n'. <p>
     *
     * The method can also be called before the command
     * starts its execution. Then the text is buffered
     * and transfered to command when it will be started.
     * @param str
     */
    public void printInputText(String str)
    {
        stdInBuff += str;
        flushInput();
    }

    /**
     * Returns information about was the command started or
     * not.
     *
     * @return <code>true</code> if the external command was
     * found and successfully started.
     */
    public boolean isStarted()
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
    public boolean isFinished()
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
            //System.out.println("No ExitValue available");
        }

        return exitValue;
    }

    /** Causes the thread to sleep some time.
     * @param milliseconds
     */
    public static void shortWait(long milliseconds)
    {
        try
        {
            Thread.sleep(milliseconds);
        }
        catch (InterruptedException e)
        {
            System.out.println("While waiting :" + e);
        }
    }

    private void dbg(String message)
    {
        if (debug)
        {
            log.println(utils.getDateTime() + "PH." + message);
        }
    }

    public void noOutput()
    {
        bUseOutput = false;
    }
    // -------------------------------------------------------------------------
    class ProcessWatcher extends Thread
    {

        private int m_nTimeoutInSec;
        private String m_sProcessToStart;
        private boolean m_bInterrupt;

        public ProcessWatcher(int _nTimeOut, String _sProcess)
        {
            m_nTimeoutInSec = _nTimeOut;
            m_sProcessToStart = _sProcess;
            m_bInterrupt = false;
        }

        /**
         * returns true, if the thread should hold on
         * @return
         */
        public synchronized boolean isInHoldOn()
        {
            return m_bInterrupt;
        }
        /**
         * Marks the thread to hold on, next time
         * STUPID: The thread must poll this flag itself.
         *
         * Reason: interrupt() seems not to work as expected.
         */
        public synchronized void holdOn()
        {
            m_bInterrupt = true;
            interrupt();
        }

        public void run()
        {
            while (m_nTimeoutInSec > 0)
            {
                m_nTimeoutInSec--;
                try
                {
                    sleep(1000);
                }
                catch(java.lang.InterruptedException e)
                {
                    // interrupt flag is set back to 'not interrupted' :-(
                }
                if (isInHoldOn())
                {
                    break;
                }
            }
            if (m_nTimeoutInSec <= 0 && !isInHoldOn())       // not zero, so we are interrupted.
            {
                system(m_sProcessToStart);
            }
        }

        /**
         * Start an external Process
         * @param _sProcess
         */
        private void system(String _sProcess)
        {
            if (_sProcess == null)
            {
                return;
            }

            try
            {

                // run a _sProcess command
                // using the Runtime exec method:
                Process p = Runtime.getRuntime().exec(_sProcess);

                BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));

                BufferedReader stdError = new BufferedReader(new InputStreamReader(p.getErrorStream()));

                // read the output from the command
                String s;
                while ((s = stdInput.readLine()) != null)
                {
                    System.out.println("out:" + s);
                }

                // read any errors from the attempted command
                while ((s = stdError.readLine()) != null)
                {
                    System.out.println("err:" + s);
                }

            }
            catch (java.io.IOException e)
            {
                System.out.println("exception caught: ");
                e.printStackTrace();
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
            m_aWatcher = new ProcessWatcher(m_nProcessTimeout, m_sProcessKiller);
            m_aWatcher.start();
        }
    }

    /**
     * to stop the extra thread, before he will kill a running office. This will stop the thread.
     */
    public void stopWatcher()
    {
        if (m_aWatcher != null)
        {
            m_aWatcher.holdOn();
            shortWait(5000);
        }
    }
}
