/*************************************************************************
 *
 *  $RCSfile: ProcessHandler.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Date: 2003-11-18 16:14:43 $
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

package helper;

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
    private boolean mbTimedOut = false;
    private long mTimeOut = 0;

    private String stdInBuff = "";
    private Pump stdout = null ;
    private Pump stderr = null ;
    private PrintStream stdIn = null ;

    private Process proc = null ;

    /**
     * Creates instance with specified external command.
     * Debug info and output
     * of external command is printed to stdout.
     */
    public ProcessHandler(String cmdLine) {
        this(cmdLine, null, null, null, 0);
    }

    /**
     * Creates instance with specified external command and
     * log stream where debug info is printed and output
     * of external command.
     */
    public ProcessHandler(String cmdLine, PrintWriter log) {
        this(cmdLine, log, null, null, 0);
    }

    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory.
     * Debug info and output
     * of external commandis printed to stdout.
     */
    public ProcessHandler(String cmdLine, File workDir) {
        this(cmdLine, null, workDir, null, 0);
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
        this(cmdLine, log, workDir, envVars, 0);
    }

    /**
     * Creates instance with specified external command which
     * will be executed in the some work directory  and
     *
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
    public ProcessHandler(String cmdLine, PrintWriter log,
                          File workDir, String[] envVars, long timeOut) {
        this.cmdLine = cmdLine ;
        this.workDir = workDir;
        this.log = log;
        this.cmdLine = cmdLine ;
        this.envVars = envVars;
        if (log == null)
            this.log =  new PrintWriter(new OutputStreamWriter(System.out));
        else
            this.log = log;
        this.mTimeOut = timeOut;
    }

    public boolean isTimedOut(){
        return mbTimedOut;
    }

    private void setTimedOut(boolean bTimedOut){
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
    public boolean executeSynchronously() {
        execute() ;
        return waitFor(mTimeOut) ;
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
     * @param  timeOut      > 0
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
                    isFinished = true;
                    Thread.sleep(1000);
                    timeout -= 1000 ;
                    try {
                        exitValue = proc.exitValue(); // throws exception if not finished
                    } catch (IllegalThreadStateException e) {
                        isFinished = false;
                    }
                }
                if(timeout < 0){
                    setTimedOut(true);
                    log.println("The process has timed out!");
                }
            } catch (InterruptedException ex) {
                log.println("The process was interrupted: " + ex);
            }
        }

        if (!isFinished) {
            log.println("Going to destroy the process!!");
            proc.destroy();
            log.println("Process has been destroyed!");
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
        if (stdout == null)
            return "";
        else
            return stdout.getStringBuffer();
    }
    /**
     * Returns the text output by external command to stderr.
     */
    public String getErrorText() {
        if (stderr == null)
            return "";
        else
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
