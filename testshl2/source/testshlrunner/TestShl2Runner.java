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
import java.util.Properties;

import java.io.FileInputStream;

// -----------------------------------------------------------------------------
/**
 *  This class offers helper function, to easily connect to a database
 *  delete values and insert values.
 */

class PropertyHelper
{
    public static Properties getProperties(String name)
        {
        Properties prop = new Properties();
        FileInputStream propFile = null;
        try {
            propFile = new FileInputStream(name);
            prop.load(propFile);
            propFile.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return prop;
    }

    public static void setProperties(Properties prop, String name) {
        FileOutputStream propFile = null;
        try {
            propFile = new FileOutputStream(name);
            prop.store(propFile,"");
            propFile.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

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
            String sAllIsRight = "PASSED#OK#";
            ArrayList sFailedStates = new ArrayList();
            boolean bFailed = false;
            for (int i=0; i<m_sStates.size();i++)
            {
                String sStatus = (String) m_sStates.get(i);
                if (!sStatus.equals(sAllIsRight))
                {
                    bFailed = true;
                    sFailedStates.add(sStatus);
                }
            }
            if (bFailed == false)
            {
                return "PASSED#OK#";
            }

            // interpret the failures
            return "PASSED#FAILED#";
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

                String db_line= Quote(_sProjectName) + sComma +
                    Quote( aEntry.m_sClassName) + sComma +
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

    Properties m_aProps;

    String m_sVersion = "udk399";
    String m_sExtension = "";
    String m_sProjectDir = "";
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

//    public TestShl2Runner(String bdir, String pf, String project, String ver, String pre, String localenv)
    public TestShl2Runner(String filename)
        {
            String fs = System.getProperty("file.separator");

            m_aProps = PropertyHelper.getProperties(filename);

            m_sEnvironment     = m_aProps.getProperty("ENV");         // unxsols3.pro
            m_sProjectName     = m_aProps.getProperty("PROJECTNAME"); // sal
            m_sProjectDir      = m_aProps.getProperty("BASEDIR") + fs + m_sProjectName;  // /usr/qaapi/projects/udk/sal
            m_sVersion         = m_aProps.getProperty("BUILD");       // udk305
            m_sShellExecutable = m_aProps.getProperty("SHELL");       // /bin/tcsh
            m_sLocalEnv        = m_aProps.getProperty("LOCALENV");    // /usr/qaapi/projects/solartmp

            // if (m_sVersion.indexOf("-pro") > 1) m_sExtension=".pro";

            startAll();
        }

    /**
     * The main Method makes it possible to run this class standalone
     * @param args the command line arguments
     */
    public static void main(String args[])
        {
            if (args.length != 1)
            {
                System.out.println("Usage: TestShl2Runner <config.file>");
                System.out.println("Example: TestShl2Runner /usr/qaapi/projects/udk/sal/qa/configfile");
                System.exit(1);
            }

            TestShl2Runner aRunner = new TestShl2Runner(args[0]);
        }

    // -----------------------------------------------------------------------------

    ArrayList getJobs()
    {
        String fs = System.getProperty("file.separator");
        String sJobFile = m_sProjectDir + fs + m_aProps.getProperty("INPUT") + fs + m_aProps.getProperty("JOBFILE");
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
            // Object[] the_array = cleanup(m_sProjectDir + fs + "qa" + fs + "sce" + fs,scene_files);
            // System.out.println("...done");

            //create a script to build the sources and run the tests
            System.out.println("creating start script for testprocess");
            String scriptName = createScript(aJobList);
            System.out.println("...done");

            //start the script
            System.out.println("starting script for testprocess");
            String sScript = m_sShellExecutable + " " + getOutputDir() + fs + scriptName;
            executeScript(sScript);
            System.out.println("...done");

            //remove old values from db-table
            DBHelper.SQLdeleteValues(m_sProjectName,m_sEnvironment);
            DBHelper.SQLdeleteValues(m_sProjectName,m_sEnvironment);

            //parse the output and store it
            parseOut (aJobList, m_sProjectName, m_sEnvironment, m_sProjectDir);
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

    public String getSignalFileParameter()
    {
        String sParameter;
        sParameter = " -sf /var/tmp/signalfile_" + m_sProjectName + "_" + m_sEnvironment + ".txt ";
        return sParameter;
    }

    public String getLogParameter(String job)
    {
        String sParameter = " -log " + getLogName(job) + " ";
        return sParameter;
    }

    public String getLogName(String job)
    {
        // create the ' -log file '
        String fs = System.getProperty("file.separator");
        String sLogName = getOutputDir() + fs + job + ".log";
        return sLogName;
    }

    public String getInputDir()
        {
            String fs = System.getProperty("file.separator");
            String sInputDir = m_sProjectDir + fs + m_aProps.getProperty("INPUT");
            return sInputDir;
        }


    public String getOutputDir()
        {
            String fs = System.getProperty("file.separator");
            String sOutputDir = m_sProjectDir + fs + m_sEnvironment + fs + m_aProps.getProperty("OUTPUT");

            File aFile = new File(sOutputDir);
            aFile.mkdirs();
            return sOutputDir;
        }

    public String getCAXParameter()
    {
        String sCAX = " ";
        // Due to the fu....g java that we havn't access to our well formed ;-)
        // environment variables the simple switch -cax do not longer work :-(

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

    public void PreNPost(FileWriter out, String _sWhat) throws IOException
        {
            String ls = System.getProperty("line.separator");

            out.write(getRemark() + " do " + _sWhat + ls);
            for (int i = 1;i<9;i++)
            {
                String sWhat = m_aProps.getProperty(_sWhat + i);
                if (sWhat != null)
                {
                    out.write(sWhat + ls);
                }
            }
            out.write(ls);
        }

    public String getChangeDirFkt()
        {
            if (m_sEnvironment.startsWith("wnt"))
            {
                return "cdd";
            }
            return "cd";
        }
    public String getRemark()
        {
            if (m_sEnvironment.startsWith("wnt"))
            {
                return "rem ";
            }
            return "# ";
        }

    public void do_setsolar(FileWriter out, String _sPreExec) throws IOException
        {
            String ls = System.getProperty("line.separator");

            out.write(getRemark() + " do a setsolar" + ls);
            out.write(_sPreExec + " setsolar -" + m_sVersion + getCAXParameter() + getProParameter() + getCompEnvName() + ls);
            out.write(ls);
        }

    public void do_cvs(FileWriter out, String _sPreExec, ArrayList _aJobList) throws IOException
        {
            String fs = System.getProperty("file.separator");
            String ls = System.getProperty("line.separator");
            out.write(getChangeDirFkt() + " " + getInputDir() + ls);
            out.write(getRemark() + " do a cvs and a dmake" + ls);

            out.write(_sPreExec + " " + "cvs -d" + m_aProps.getProperty("CVSROOT") + " update -d" + ls);

            for (int i=0; i<_aJobList.size();i++)
            {
                String sJob = (String) _aJobList.get(i);

                out.write(getChangeDirFkt() + " " + getInputDir() + fs + sJob + ls);
                // setenv VCSID lla
                // set VCSID=lla
                out.write(_sPreExec + " " + "dmake -u" + ls);
            }
            out.write(ls);
        }

    public void do_testtool(FileWriter out, String _sPreExec, ArrayList _aJobList) throws IOException
        {
            String ls = System.getProperty("line.separator");
            out.write(getRemark() + " call the test tool" + ls);
            out.write(getChangeDirFkt() + " " + getOutputDir() + ls);
            if (m_sEnvironment.startsWith("wnt"))
            {
                out.write("set DISABLE_SAL_DBGBOX=t" + ls);
            }

            for (int i=0; i<_aJobList.size();i++)
            {
                String sLine = (String) _aJobList.get(i);
                // String sJob = getJob(sLine);
                out.write(_sPreExec + " " + m_aProps.getProperty("TESTTOOL") + " " + getLibName(sLine, m_sEnvironment) + getLogParameter(sLine) + getSignalFileParameter() + ls);
            }
            out.write(ls);
        }

    public String createwntmsci(ArrayList _aJobList) throws IOException
        {
            String fs = System.getProperty("file.separator");
            String ls = System.getProperty("line.separator");

            String sBatchFile = m_aProps.getProperty("BATCHFILE");
            File outputFile = new File(getOutputDir(), sBatchFile);
            FileWriter out = new FileWriter(outputFile.toString());

            out.write("rem do not edit, will be created automatically by TestShl2Runner.java" + ls);

            // String sBuildEnvironment = "wntmsci9";
            PreNPost(out, "PRE");

            String sBatchPreExec = "call";
            do_setsolar(out, sBatchPreExec);
            do_cvs(out, sBatchPreExec, _aJobList);
            do_testtool(out, sBatchPreExec, _aJobList);

            PreNPost(out, "POST");

            // the exit is alsolute need here, because we do not get back, until
            // shell is stopped with exit.
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

            String sBatchFile = m_aProps.getProperty("BATCHFILE");
            File outputFile = new File(getOutputDir(), sBatchFile);
            FileWriter out = new FileWriter(outputFile.toString());

            out.write("#!/bin/tcsh" + ls);          // shebang
            out.write("# do not edit, will be created automatically by TestShl2Runner.java" + ls);

            PreNPost(out, "PRE");

            String sBatchPreExec = "";
            do_setsolar(out, sBatchPreExec);
            do_cvs(out, sBatchPreExec, _aJobList);
            do_testtool(out, sBatchPreExec, _aJobList);

            PreNPost(out, "POST");

            out.write("exit" + ls);
            out.close();

            // set excution bits
            String sExec = "chmod u+x " + getOutputDir() + fs + sBatchFile;
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
            res = m_sProjectDir + fs + m_sEnvironment + fs + purelibname;
            return res;
        }

    // -----------------------------------------------------------------------------

/*
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
*/

    public void parseOut (ArrayList sJobList, String _sProjectName, String _sEnvironment, String basedir)
        {
            //parse the out-files and store the data
            String fs = System.getProperty("file.separator");
            for (int i=0; i<sJobList.size();i++)
            {
                String sJob = (String) sJobList.get(i);
                String outFile = getLogName(sJob);
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
            boolean bBackValue = aHandler.executeSynchronously();
            return bBackValue;
            // return false;
        }
}

