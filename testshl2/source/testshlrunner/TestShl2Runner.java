/*************************************************************************
 *
 *  $RCSfile: TestShl2Runner.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2003-01-20 11:05:21 $
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


// package main;

import java.io.File;
import java.io.FileWriter;
import java.io.RandomAccessFile;
import java.io.PrintWriter;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.Statement;
import java.sql.ResultSet;

import java.util.HashMap;
import java.util.Set;
import java.util.Iterator;

// -----------------------------------------------------------------------------
/**
 *  This class offers helper function, to easily connect to a database
 *  delete values and insert values.
 */

class DBHelper
{
    /**
     * This method inserts given values into<br>
     * the table 'states'
     * @param values a set of comma separated values to be inserted
     */

    public static void SQLinsertValues(String values)
        {
            Statement oStmt = null;
            Connection oCon = null;
            try
            {
                oCon = getMySQLConnection();
                oStmt = oCon.createStatement();
                ResultSet oResult = oStmt.executeQuery("insert into states values ("
                                                       + values + ")");
            }
            catch(Exception e)
            {
                System.out.println("Couldn't insert values to db");
                e.printStackTrace();
            } finally
            {
                try
                {
                    oCon.close();
                    oCon = null;
                }
                catch (Exception e) {}
            }
        }
    /**
     * This method establishes a Connection<br>
     * with the database 'module_unit' on jakobus
     */

    public static Connection getMySQLConnection() throws Exception
        {
            Class.forName("org.gjt.mm.mysql.Driver");
            Connection mysql = DriverManager.getConnection(
                "jdbc:mysql://jakobus:3306/module_unit","admin","admin");
            return mysql;
        }
    /**
     * This method removes all entries of the given<br>
     * module/platform combination
     * @param mdl the name of the module, e.g. sal
     * @param os the name of the platform, e.g. unxsols
     */

    public static void SQLdeleteValues(String _sModulName, String _sEnvironment)
        {
            Statement oStmt = null;
            Connection oCon = null;
            try
            {
                oCon = getMySQLConnection();
                oStmt = oCon.createStatement();
                ResultSet oResult = oStmt.executeQuery("delete from states where mdl='"
                                                       + _sModulName + "' AND pf='" + _sEnvironment + "'");
            }
            catch(Exception e)
            {
                System.out.println("Couldn't delete values from db");
            }
            finally
            {
                try
                {
                    oCon.close();
                    oCon = null;
                }
                catch (Exception e) {}
            }
        }
}

// -----------------------------------------------------------------------------

/**
 * This class helps to build only one output line if the classname and the methodname are the same.
 */

class CurrentEntry
{
    public String m_sDate;
    public String m_sClassName;
    public String m_sMethodName;

    ArrayList m_sMethodTestNames = new ArrayList();
    ArrayList m_sComments = new ArrayList();
    ArrayList m_sStates = new ArrayList();

    public String m_sCommentContainer = "";

    CurrentEntry()
        {}

    public String getState()
        {
            // build the right State
            return "FAILED#HARDCODED#";
        }

    public void add(String _sDate, String _sClassName, String _sMethodName, String _sMethodTestName, String _sState, String _sComment)
        {
            m_sDate = _sDate;
            m_sClassName = _sClassName;
            m_sMethodName = _sMethodName;
            m_sMethodTestNames.add(_sMethodTestName);
            m_sStates.add(_sState);

            if (_sComment.length() > 0)
            {
                String sComment = _sComment.replace('\'',' ');

                m_sComments.add(sComment);

                if (m_sCommentContainer.length() > 0)
                {
                    m_sCommentContainer += ";<BR>";
                }
                m_sCommentContainer += _sMethodTestName + "(): " + _sComment;
            }
        }

}
// -----------------------------------------------------------------------------

/**
 * HashMap Helper. At the moment classname and Methodname build a key value
 * if they already exist, we store additional information into the already existing value
 * if not, build an new entry.
 */

class DatabaseEntry
{
    HashMap aMap = new HashMap();

    public DatabaseEntry()
        {
        }

    public String Quote(String _sToQuote)
        {
            String ts = "'";
            return ts + _sToQuote + ts;
        }

    /**
     * Write down all collected lines into the database.
     */
    public void writeDown(String _sProjectName, String _sEnvironment)
        {
            System.out.println(" ");
            Set aSet = (Set)aMap.keySet();
            Iterator aIter = aSet.iterator();
            String sComma = ",";

            while (aIter.hasNext())
            {
                String sKey = (String)aIter.next();
                CurrentEntry aEntry = (CurrentEntry)aMap.get(sKey);

                String db_line= Quote(aEntry.m_sClassName) + sComma +
                    Quote( _sProjectName) + sComma +
                    Quote( aEntry.m_sMethodName) + sComma +
                    Quote( aEntry.getState()) + sComma +
                    Quote( _sEnvironment) + sComma +
                    Quote( aEntry.m_sDate) + sComma +
                    Quote( aEntry.m_sCommentContainer);
                System.out.println(db_line);
                DBHelper.SQLinsertValues(db_line);
            }
        }


    public void add(String _sDate, String _sClassName,
                    String _sMethodName, String _sMethodTestName,
                    String _sState, String _sComment)
        {
            String sKey = _sClassName + "." + _sMethodName;
            if (aMap.containsKey(sKey))
            {
                CurrentEntry aEntry = (CurrentEntry)aMap.get(sKey);
                aEntry.add(_sDate, _sClassName, _sMethodName, _sMethodTestName, _sState, _sComment);
            }
            else
            {
                CurrentEntry aEntry = new CurrentEntry();
                aEntry.add(_sDate, _sClassName, _sMethodName, _sMethodTestName, _sState, _sComment);
                aMap.put(sKey, aEntry);
            }
        }
}

/**
 *
 * This class will run the testshl2 script
 *
 * According to a given baseDir it'll first
 * get all scenario files, clean them up<br>
 * Then all sources will be compiled and
 * the scenarios started.<br>
 * After the a scenarios are finished the result
 * will be stored in the database.
 * @version 0.1
 */
public class TestShl2Runner
{

    String m_sVersion = "udk399";
    String m_sExtension = "";
    String m_sBaseDir = "";
    String m_sEnvironment = "";
    String m_sProjectName = "";
    String m_sShellExecutable="";
    String m_sLocalEnv="";

    /**
     * The constructor to use this class from any other java-class
     * @param bdir the base directory where the qa-subfolder can be found
     * @param pf the platform e.g. unxsols
     * @param mod the module e.g. sal
     * @param ver the version e.g. udk304
     * @param pre the path to the shell, may be "", but is needed on Windows<br>
     * Systems to ensure that the created scripts run in an 4NT-shell
     */

    public TestShl2Runner(String bdir, String pf, String project, String ver, String pre, String localenv)
        {
            String prefix="tcsh ";

            if (pre.length() > 1)
            {
                prefix = pre + " ";
            }

            m_sBaseDir = bdir;
            m_sEnvironment = pf;
            m_sProjectName = project;
            m_sVersion = ver;
            m_sShellExecutable = prefix;
            m_sLocalEnv = localenv;

            // if (m_sVersion.indexOf("-pro") > 1) m_sExtension=".pro";

            startAll();
        }

    /**
     * The main Method makes it possible to run this class standalone
     * @param args the command line arguments
     */
    public static void main(String args[])
        {

            if (args.length < 4)
            {
                System.out.println("Usage: TestShl2Runner <bdir> <platform> <module> <version> <shell> <localenv>");
                System.out.println("Example: TestShl2Runner /usr/qaapi/projects/udk/sal unxsols3 sal udk304");
                System.exit(1);
            }

            String sShellExecutable="tcsh ";

            if (args.length >= 5)
            {
                sShellExecutable = args[4] + " ";
            }

            // m_sBaseDir = args[0];
            // m_sEnvironment = args[1];
            // m_sModulName = args[2];
            // m_sVersion = args[3];

            // if (m_sVersion.indexOf("-pro") > 1) m_sExtension=".pro";

            TestShl2Runner aRunner = new TestShl2Runner(args[0], args[1], args[2], args[3], sShellExecutable, args[5]);
            // aRunner.startAll();

            // System.exit(0);
        }

    // -----------------------------------------------------------------------------

    ArrayList getJobs()
    {
        String fs = System.getProperty("file.separator");
        String sJobFile = m_sBaseDir + fs + "qa" + fs + "jobs.txt";
        ArrayList aLines = getLines(sJobFile);
        return aLines;
    }

    // -----------------------------------------------------------------------------

    public void startAll()
        {
            String fs = System.getProperty("file.separator");

            //get all scenario-files
            System.out.println("Getting jobs");
            ArrayList aJobList = getJobs();

            //cleanup scenorio-files and return a working array
            // Object[] the_array = cleanup(m_sBaseDir + fs + "qa" + fs + "sce" + fs,scene_files);
            // System.out.println("...done");

            //create a script to build the sources and run the tests
            System.out.println("creating start script for testprocess");
            String scriptName = createScript(aJobList);
            System.out.println("...done");

            //start the script
            System.out.println("starting script for testprocess");
            String sScript = m_sShellExecutable + getOutputDir() + fs + scriptName;
            executeScript(sScript);
            System.out.println("...done");

            //remove old values from db-table
            DBHelper.SQLdeleteValues(m_sProjectName,m_sEnvironment);

            //parse the output and store it
            parseOut (aJobList, m_sProjectName, m_sEnvironment, m_sBaseDir);
        }

    /**
     * This method reads the lines of a file and puts<br>
     * them into an ArrayList
     * @param filename the name of the file to be read
     */

    public ArrayList getLines(String filename)
        {
            File the_file = new File(filename);
            ArrayList the_lines = new ArrayList();
            if (! the_file.exists())
            {
                System.out.println("couldn't find file " + filename);
                return the_lines;
            }
            RandomAccessFile the_reader = null;
            try
            {
                the_reader = new RandomAccessFile(the_file,"r");
                String aLine = "";
                while (aLine != null)
                {
                    aLine = the_reader.readLine();
                    if ( (aLine != null) &&
                         (! (aLine.length() < 2) ) &&
                         (! aLine.startsWith("#")))
                    {
                        the_lines.add(aLine);
                    }
                }
            }
            catch (java.io.FileNotFoundException fne)
            {
                System.out.println("couldn't open file " + filename);
                System.out.println("Message: " + fne.getMessage());
            }
            catch (java.io.IOException ie)
            {
                System.out.println("Exception while reading file " + filename);
                System.out.println("Message: " + ie.getMessage());
            }
            try
            {
                the_reader.close();
            }
            catch (java.io.IOException ie)
            {
                System.out.println("Couldn't close file " + filename);
                System.out.println("Message: " + ie.getMessage());
            }
            return the_lines;
        }



    /**
     * This method calls the createScript-methods<br>
     * for the given platform
     * @param basedir the name of the base-directory, e.g. /usr/qaapi/projects/udk/sal
     * @param qajobs jobs to be done
     */

    public String createScript(ArrayList _aJobList)
        {
            String res = "";
            if (m_sEnvironment.startsWith("wnt"))
            {
                try
                {
                    res = createwntmsci(_aJobList);
                }
                catch (IOException ioe)
                {
                    System.out.println("Couldn't create Script");
                }
            }
            else if (m_sEnvironment.startsWith("unx"))
            {
                try
                {
                    res = createunx(_aJobList);
                }
                catch (IOException ioe)
                {
                    System.out.println("Couldn't create Script");
                }
            }
            else
            {
                // can't handle unknown environment
            }
            return res;
        }

    public String getLogParameter(String job)
    {
        String sParameter = " -log " + getLogName(job);
        return sParameter;
    }

    public String getLogName(String job)
    {
        // create the ' -log file '
        String sLogName = job + ".log ";
        return sLogName;
    }

    public String getOutputDir()
        {
            String fs = System.getProperty("file.separator");
            String sOutputDir = m_sBaseDir + fs + m_sEnvironment + fs + "qa";

            File aFile = new File(sOutputDir);
            aFile.mkdirs();
            return sOutputDir;
        }

    public String getCAXParameter()
    {
        String sCAX = " ";
        // Due to the fu....g java that we havn't access to our well formed ;-)
        // environment variables the switch -cax do not longer work :-(

        // String sSolTmp = System.getenv("SOL_TMP");
        if (m_sLocalEnv.length() > 0)
        {
            sCAX = " -cax -tmp " + m_sLocalEnv;
        }
        // else
        // {
            // due to the fact that we don't want to get hole lokal env if no one exist, do nothing here.
        // }
        return sCAX;
    }

    public String getProParameter()
        {
            String sPro = " ";
            if (m_sEnvironment.endsWith(".pro"))
            {
                sPro = " -pro ";
            }
            return sPro;
        }

    public String getCompEnvName()
        {
            String sCompEnv;
            if (m_sEnvironment.endsWith(".pro"))
            {
                sCompEnv = String.copyValueOf(m_sEnvironment.toCharArray(), 0, 8);
            }
            else
            {
                sCompEnv = m_sEnvironment;
            }
            return " " + sCompEnv;
        }

    /**
     * This method create the script needed to compile and run the<br>
     * the tests on the Windows platform
     * @param basedir the name of the base-directory, e.g. /usr/qaapi/projects/udk/sal
     * @param qajobs jobs to be done
     */

    public String createwntmsci(ArrayList _aJobList) throws IOException
        {
            String fs = System.getProperty("file.separator");
            String ls = System.getProperty("line.separator");

            String sOutputDir = getOutputDir();
            String sBatchFile = "runTestshl.btm";
            File outputFile = new File(sOutputDir, sBatchFile);
            FileWriter out = new FileWriter(outputFile.toString());

            out.write("@rem do not edit, will be created automatically by TestShl2Runner.java" + ls);
            out.write("@rem @echo off" + ls);

            // String sBuildEnvironment = "wntmsci9";
            out.write("set path=r:\\etw;%PATH%" + ls);
            out.write("call setsolar -" + m_sVersion + getCAXParameter() + getProParameter() + getCompEnvName() + ls);
            out.write("cdd " + sOutputDir + ls);

            out.write("set DISABLE_SAL_DBGBOX=t" + ls);

            for (int i=0; i<_aJobList.size();i++)
            {
                String sLine = (String) _aJobList.get(i);
                // String sJob = getJob(sLine);
                out.write("call testshl2 " + getLibName(sLine, m_sEnvironment) + getLogParameter(sLine) + ls);
            }

            // for (int j=0; j<qajobs.length;j++)
            // {
            //     String job = (String) qajobs[j];
            //     out.write("rem call testshl2 " + getLibName(job,sBuildEnvironment) + getLogName(job) + ls);
            // }

            // out.write("echo \"done\" > " + sOutputDir + fs + sBatchFile + ".done" + ls);
            out.write("exit" + ls);
            out.close();
            return sBatchFile;
        }

    /**
     * This method create the script needed to compile and run the<br>
     * the tests on the Unix platform
     * @param basedir the name of the base-directory, e.g. /usr/qaapi/projects/udk/sal
     * @param qajobs jobs to be done
     */

    public String createunx(ArrayList _aJobList) throws IOException
        {
            String fs = System.getProperty("file.separator");
            String ls = System.getProperty("line.separator");
            String sBatchFile = "runTestshl";
            String sOutputDir = getOutputDir();
            File outputFile = new File(sOutputDir, sBatchFile);
            FileWriter out = new FileWriter(outputFile.toString());

            out.write("#!/bin/tcsh" + ls);
            out.write("setenv SHELL /bin/tcsh" + ls);

            // special for LINUX
            if (m_sEnvironment.startsWith("unxlngi"))
            {
                out.write("source ~/staroffice.cshrc" + ls);
            }

            out.write("setsolar -" + m_sVersion + getCAXParameter() + getProParameter() + getCompEnvName() + ls);
            out.write("cd " + sOutputDir + ls);
            // out.write("cd " + basedir + ps + "qa" + ls);
            // out.write("dmake" + ls);

            // for (int j=0; j<qajobs.length;j++)
            // {
            //     String job = (String) qajobs[j];
            //     out.write("testshl " + getLibName(job,pform) + " ../qa/sce/" + job +
            //               " -log -msg" + ls);
            // }
            out.write("echo \"done\" > " + sOutputDir + fs + sBatchFile + ".done" + ls);
            out.write("exit" + ls);
            out.close();

            // set excution bits
            String sExec = "chmod u+x " + sOutputDir + fs + sBatchFile;
            Runtime.getRuntime().exec(sExec);
            return sBatchFile;
        }

    // -----------------------------------------------------------------------------

    /**
     * This method extracts the libname from a given<br>
     * job and adds a relative path based on the platform
     * @param job the job to be done
     * @param os the platform
     */

    public String getLibName(String job, String _sEnvironment)
        {
            String fs = System.getProperty("file.separator");
            // BACK: String like "../wntmsci9.pro/bin/rtl_OString.dll"

            String res = "";
            int nIndex = job.indexOf(".");
            // if (nIndex > 0)
            // {
            // }
            String sPureFilename = job;
            String purelibname = "";
            if (_sEnvironment.startsWith("wnt"))
            {
                purelibname = "bin" + fs + sPureFilename + ".dll";
            }
            else if (_sEnvironment.startsWith("unx"))
            {
                purelibname = "lib" + fs + "lib" + sPureFilename + ".so";
            }
            else
            {
                // _sEnvironment unknown
                // throw ...
            }

            // we are in the wntmsci9[.pro]/qa directory
            // to go to the bin directory we have to go to
            // ../bin or ../lib
            res = ".." + fs + purelibname;
            return res;
        }

    // -----------------------------------------------------------------------------

    public Object[] cleanup(String sceneDir, String[] scene_files)
        {
            ArrayList res = new ArrayList();
            for (int j=0;j<scene_files.length;j++)
            {
                if (scene_files[j].endsWith(".sce"))
                {
                    try
                    {
                        removeRemark(sceneDir,scene_files[j]);
                    }
                    catch (IOException ioe)
                    {
                        System.out.println("Couldn't cleanup Scene-File " + scene_files[j]);
                    }
                    String pure = scene_files[j].substring(0,
                                                           scene_files[j].indexOf("."));
                    res.add(pure + ".qadev");
                }
            }
            return res.toArray();
        }

    public void removeRemark(String sceneDir,String scene_file)
        throws IOException
        {
            ArrayList org = getLines(sceneDir + scene_file);
            String pure = scene_file.substring(0,scene_file.indexOf("."));
            String ls = System.getProperty("line.separator");
            File outputFile = new File(sceneDir,pure + ".qadev");
            FileWriter out = new FileWriter(outputFile.toString());
            for (int k=0;k<org.size();k++)
            {
                String toWrite = (String)org.get(k);
                if (toWrite.startsWith("#"))
                {
                    toWrite=toWrite.substring(1);
                }
                out.write(toWrite + ls);
            }
            out.close();
        }

    public void parseOut (ArrayList sJobList, String _sProjectName, String _sEnvironment, String basedir)
        {
            //parse the out-files and store the data
            String fs = System.getProperty("file.separator");
            for (int i=0; i<sJobList.size();i++)
            {
                String sJob = (String) sJobList.get(i);
                String unitName = "";
                String outFile = getOutputDir() + fs + getLogName(sJob);
                ArrayList out_lines = getLines(outFile);
                String date="";

                DatabaseEntry aEntry = new DatabaseEntry();

                for (int j=0;j<out_lines.size();j++)
                {
                    String sLine = (String)out_lines.get(j);
                    if (sLine.startsWith("["))
                    {
                        date = sLine.substring(1, 11);
                        date = date.replace('.','-');
                    }
                    else
                    {
                        if (sLine.startsWith("Test #PASSED#")) break;
                        // if (out_lines.get(j).equals(
                        //     "         ----------            ")) break;

                        try
                        {
                            String sName = sLine.substring(0, sLine.indexOf(';'));

                            // count dot's
                            int nFirstDot = sName.indexOf('.');
                            int nSecondDot = sName.indexOf('.', nFirstDot + 1);
                            String sClassName = "";
                            String sMethodName = "";
                            String sMethodTestName = "";

                            if (nFirstDot > 0)
                            {
                                sClassName = sName.substring(0, nFirstDot);
                                if (nSecondDot > 0)
                                {
                                    sMethodName = sName.substring(nFirstDot + 1, nSecondDot);
                                    sMethodTestName = sName.substring(nSecondDot + 1);
                                }
                                else
                                {
                                    sMethodName = sName.substring(nFirstDot);
                                }
                            }
                            else
                            {
                                // no FirstDot, also no secondDot!
                                sClassName = sName;
                            }

                            String sComment = sLine.substring(sLine.lastIndexOf('#') + 1);

                            // get state
                            int nSecondNo = sLine.indexOf('#');
                            nSecondNo = sLine.indexOf('#', nSecondNo + 1);
                            String sState = sLine.substring(sLine.indexOf(";") + 1, nSecondNo + 1);

                            aEntry.add(date, sClassName, sMethodName, sMethodTestName, sState, sComment);
                        }
                        catch(IndexOutOfBoundsException e)
                        {
                            System.out.println("IndexOutOfBoundException: caught in parseOut()");
                        }
                    }
                }
                aEntry.writeDown(_sProjectName, _sEnvironment);
            }
        }

    // -----------------------------------------------------------------------------

    private  void shortWait(int nMilliSec)
        {
            try
            {
                Thread.sleep(nMilliSec);
            }
            catch (InterruptedException ex)
            {
            }

        }

    // -----------------------------------------------------------------------------

    public boolean executeScript(String scriptFile)
        {
            System.out.println("Running " + scriptFile);

            // Process testshl = Runtime.getRuntime().exec(scriptFile);
            ProcessHandler aHandler = new ProcessHandler(scriptFile);
            return aHandler.executeSynchronously();
            // return false;
        }
}

