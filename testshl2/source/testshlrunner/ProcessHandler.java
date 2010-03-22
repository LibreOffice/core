/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

import java.io.InputStream;
import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.io.PrintWriter;
import java.io.PrintStream;
import java.io.LineNumberReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

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
class Pump extends Thread {
    private LineNumberReader reader;
    private String pref ;
    private Pump thread ;
    private StringBuffer buf = new StringBuffer(256);
    private PrintWriter log ;

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
    public Pump(InputStream is, PrintWriter log, String outPrefix) {
        this.pref = outPrefix == null ? "" : outPrefix ;
        reader = new LineNumberReader(new InputStreamReader(is));
        this.log = log ;
        start() ;
    }

    public void run() {
        try {
            String line = reader.readLine() ;
            while (line != null) {
                log.println(pref + line);
                log.flush();
                buf.append(line).append('\n');
                line = reader.readLine() ;
            }
        } catch (java.io.IOException e) {
            log.println(pref + "Exception occured: " + e) ;
        }
    }

    /**
     * Returns the text collected from input stream.
     */
    public String getStringBuffer() {
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
public class ProcessHandler {
    private String cmdLine;
    private String[] envVars = null;
    private File workDir = null;
    private PrintWriter log;

    private int exitValue = -1;
    private boolean isFinished = false;
    private boolean isStarted = false;

    private String stdInBuff = "";
    private Pump stdout = null ;
    private Pump stderr = null ;
    private PrintStream stdIn = null ;

    private Process proc = null ;

    /**
     * Creates instance with specified external command and
     * log stream where debug info is printed and output
     * of external command.
     */
    public ProcessHandler(String cmdLine, PrintWriter log) {
        this(cmdLine, log, null, null);
    }
    /**
     * Creates instance with specified external command.
     * Debug info and output
     * of external commandis printed to stdout.
     */
    public ProcessHandler(String cmdLine) {
        this(cmdLine, null, null, null);
    }
    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory  and
     * log stream where debug info and output
     * of external command is printed .
     * The specified environment variables are set for the new process.
     * If log stream is null, logging is printed to stdout.
     */
    public ProcessHandler(String cmdLine, PrintWriter log,
                                            File workDir, String[] envVars) {
        this.cmdLine = cmdLine ;
        this.workDir = workDir;
        this.log = log;
        this.cmdLine = cmdLine ;
        this.envVars = envVars;
        if (log == null)
            this.log =  new PrintWriter(new OutputStreamWriter(System.out));
        else
            this.log = log;
    }
    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory.
     * Debug info and output
     * of external commandis printed to stdout.
     */
    public ProcessHandler(String cmdLine, File workDir) {
        this(cmdLine, null, workDir, null);
    }

    /**
     * Executes the command and returns only when the process
     * exits.
     *
     * @return <code>true</code> if process was successfully
     * started and correcly exits (exit code doesn't affect
     * to this result).
     */
    public boolean executeSynchronously() {
        execute() ;
        return waitFor() ;
    }

    /**
     * Executes the command immediately returns. The process
     * remains in running state. Control of its state should
     * be made by <code>waitFor</code> methods.
     *
     * @return <code>true</code> if process was successfully
     * started.
     */
    public boolean executeAsynchronously() {
        execute() ;
        return isStarted() ;
    }

    public void kill() {
        if (!isStarted()) return;
        proc.destroy();
        isStarted = false;
    }

    protected void execute() {
        if (isStarted()) {
            throw new RuntimeException(
                    "The process handler has already been executed.") ;
        }
        Runtime runtime = Runtime.getRuntime() ;
        try {
            log.println("Starting command: " + cmdLine) ;
            if (workDir != null) {
                proc = runtime.exec(cmdLine, envVars, workDir) ;
            } else {
                proc = runtime.exec(cmdLine, envVars) ;
            }

            isStarted = true ;
        } catch (java.io.IOException e) {
            log.println("The command "+cmdLine+" can't be started: " + e);
            return;
        }
        stdout = new Pump(proc.getInputStream(), log, "out > ");
        stderr = new Pump(proc.getErrorStream(), log, "err > ");
        stdIn = new PrintStream(proc.getOutputStream()) ;

        flushInput() ;
    }

    /**
     * This method is useful when the process was executed
     * asynchronously. Waits for process to exit and return
     * its result.
     *
     * @return <code>true</code> if process correctly exited
     * (exit code doesn't affect to this result).
     */
    public boolean waitFor() {
        return waitFor(0) ;
    }

    /**
     * This method is useful when the process was executed
     * asynchronously. Waits during specified time for process
     * to exit and return its status.
     *
     * @return <code>true</code> if process correctly exited
     * (exit code doesn't affect to this result).
     */
    public boolean waitFor(long timeout) {
        if (isFinished()) return true ;
        if (!isStarted()) return false ;

        if (timeout == 0) {
            try {
                proc.waitFor() ;
            } catch (InterruptedException e) {
                log.println("The process was interrupted: " + e);
            }
            isFinished = true ;
            try {
                exitValue = proc.exitValue() ;
            } catch (IllegalThreadStateException e) {}
        } else {
            try {
                while (!isFinished && timeout > 0) {
                    isFinished = true ;
                    Thread.sleep(1000);
                    timeout -= 1000 ;
                    try {
                        exitValue = proc.exitValue() ;
                    } catch (IllegalThreadStateException e) {
                        isFinished = false ;
                    }
                }
            } catch (InterruptedException ex) {
                log.println("The process was interrupted: " + ex);
            }
        }

        if (!isFinished) {
            proc.destroy();
        }

        try {
            stdout.join();
            stderr.join();
        } catch (InterruptedException e) {}

        return isFinished() ;
    }

    protected void flushInput() {
        if (stdIn == null) return ;

        synchronized(stdInBuff) {
            stdIn.print(stdInBuff);
            stdIn.flush();
            stdInBuff = "" ;
        }
    }

    /**
     * Returns the text output by external command to stdout.
     */
    public String getOutputText() {
        return stdout.getStringBuffer();
    }
    /**
     * Returns the text output by external command to stderr.
     */
    public String getErrorText() {
        return stderr.getStringBuffer();
    }

    /**
     * Prints the string specified to sdtin of external
     * command. '\n' is not added so if you need you
     * should terminate the string with '\n'. <p>
     *
     * The method can also be called before the command
     * starts its execution. Then the text is buffered
     * and transfered to command when it will be started.
     */
    public void printInputText(String str) {
        stdInBuff += str ;
        flushInput();
    }

    /**
     * Returns information about was the command started or
     * not.
     *
     * @return <code>true</code> if the external command was
     * found and successfully started.
     */
    public boolean isStarted() {
        return isStarted ;
    }

    /**
     * Returns the information about the final state of command
     * execution.
     *
     * @return <code>true</code> if the command correctly starts,
     * exits and was not interrupted due to timeout.
     */
    public boolean isFinished() {
        return isFinished ;
    }

    /**
     * Returns exit code of the external command.
     *
     * @return exit code of command if it was finished,
     * -1 if not.
     */
    public int getExitCode() {
        return exitValue ;
    }
}
