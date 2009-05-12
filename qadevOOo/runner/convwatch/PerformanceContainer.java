/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PerformanceContainer.java,v $
 * $Revision: 1.5 $
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

package convwatch;

import java.io.File;
import java.io.FileWriter;
import java.io.RandomAccessFile;
import java.lang.Double;


public class PerformanceContainer /* extends *//* implements */ {
    private long m_nStartTime;

    /*
      simple helper functions to start/stop a timer, to know how long a process need in milliseconds
     */
    public long getStartTime()
        {
            return System.currentTimeMillis();
        }
    public void setStartTime(long _nStartTime)
        {
            m_nStartTime = _nStartTime;
        }

    /*
      return the time, which is done until last startTime()
     */
    private long meanTime(long _nCurrentTimer)
        {
            if (_nCurrentTimer == 0)
            {
                GlobalLogWriter.get().println("Forgotten to initialise a start timer.");
                return 0;
            }
            long nMeanTime = System.currentTimeMillis();
            return nMeanTime - _nCurrentTimer;
        }

    /*
      public long stopTimer()
        {
            if (m_nStartTime == 0)
            {
                System.out.println("Forgotten to initialise start timer.");
                return 0;
            }
            long nStopTime = System.currentTimeMillis();
            return nStopTime - m_nStartTime;
        }
    */

    final static int Load = 0;
    final static int Store = 1;
    final static int Print = 2;
    final static int OfficeStart = 3;
    final static int StoreAsPDF = 4;

    private long m_nTime[];
    private String m_sMSOfficeVersion;

    public PerformanceContainer()
        {
            m_nTime = new long[5];
            // @todo: is this need?
            for (int i=0;i<5;i++)
            {
                m_nTime[i] = 0;
            }
        }

    public void setTime(int _nIndex, long _nValue)
        {
            m_nTime[_nIndex] = _nValue;
        }
    public long getTime(int _nIndex)
        {
            return m_nTime[_nIndex];
        }

    public void startTime(int _nIndex)
        {
            m_nTime[_nIndex] = getStartTime();
        }

    public void stopTime(int _nIndex)
        {
            m_nTime[_nIndex] = meanTime(m_nTime[_nIndex]);
        }

    public String getMSOfficeVersion()
    {
        return m_sMSOfficeVersion;
    }
    public void print(FileWriter out) throws java.io.IOException
        {
            String ls = System.getProperty("line.separator");

            out.write("loadtime="        + String.valueOf(m_nTime[ Load ]) + ls);
            out.write("storetime="       + String.valueOf(m_nTime[ Store ]) + ls);
            out.write("printtime="       + String.valueOf(m_nTime[ Print ]) + ls);
            out.write("officestarttime=" + String.valueOf(m_nTime[ OfficeStart ]) + ls);
            out.write("storeaspdftime="  + String.valueOf(m_nTime[ StoreAsPDF ]) + ls);
        }

    public static double stringToDouble(String _sStr)
        {
            double nValue = 0;
            try
            {
                nValue = Double.parseDouble( _sStr );
            }
            catch (NumberFormatException e)
            {
                GlobalLogWriter.get().println("Can't convert string to double " + _sStr);
            }
            return nValue;
        }

    public static long secondsToMilliSeconds(double _nSeconds)
        {
            return (long)(_nSeconds * 1000.0);
        }

    /*
      Helper function, which read some values from a given file

      sample of wordinfofile
    name=c:\doc-pool\wntmsci\samples\msoffice\word\LineSpacing.doc
    WordVersion=11.0
    WordStartTime=0.340490102767944
    WordLoadTime=0.650935888290405
    WordPrintTime=0.580835103988647
     */
    public void readWordValuesFromFile(String sFilename)
        {
            File aFile = new File(sFilename);
            if (! aFile.exists())
            {
                GlobalLogWriter.get().println("couldn't find file " + sFilename);
                return;
            }

            RandomAccessFile aRandomAccessFile = null;
            try
            {
                aRandomAccessFile = new RandomAccessFile(aFile,"r");
                String sLine = "";
                while (sLine != null)
                {
                    sLine = aRandomAccessFile.readLine();
                    if ( (sLine != null) &&
                         (! (sLine.length() < 2) ) &&
                         (! sLine.startsWith("#")))
                    {
                        if (sLine.startsWith("WordStartTime="))
                        {
                            String sTime = sLine.substring(14);
                            m_nTime[OfficeStart] = secondsToMilliSeconds(stringToDouble(sTime));
                        }
                        else if (sLine.startsWith("WordLoadTime="))
                        {
                            String sTime = sLine.substring(13);
                            m_nTime[Load] = secondsToMilliSeconds(stringToDouble(sTime));
                        }
                        else if (sLine.startsWith("WordPrintTime="))
                        {
                            String sTime = sLine.substring(14);
                            m_nTime[Print] = secondsToMilliSeconds(stringToDouble(sTime));
                        }
                        else if (sLine.startsWith("WordVersion="))
                        {
                            String sMSOfficeVersion = sLine.substring(12);
                            m_sMSOfficeVersion = "Word:" + sMSOfficeVersion;
                        }
                        else if (sLine.startsWith("ExcelVersion="))
                        {
                            String sMSOfficeVersion = sLine.substring(13);
                            m_sMSOfficeVersion = "Excel:" + sMSOfficeVersion;
                        }
                        else if (sLine.startsWith("PowerPointVersion="))
                        {
                            String sMSOfficeVersion = sLine.substring(18);
                            m_sMSOfficeVersion = "PowerPoint:" + sMSOfficeVersion;
                        }
                    }
                }
            }
            catch (java.io.FileNotFoundException fne)
            {
                GlobalLogWriter.get().println("couldn't open file " + sFilename);
                GlobalLogWriter.get().println("Message: " + fne.getMessage());
            }
            catch (java.io.IOException ie)
            {
                GlobalLogWriter.get().println("Exception while reading file " + sFilename);
                GlobalLogWriter.get().println("Message: " + ie.getMessage());
            }
            try
            {
                aRandomAccessFile.close();
            }
            catch (java.io.IOException ie)
            {
                GlobalLogWriter.get().println("Couldn't close file " + sFilename);
                GlobalLogWriter.get().println("Message: " + ie.getMessage());
            }
        }

    public static void main(String[] args) {

/*
 BorderRemover a = new BorderRemover();
        try
        {
            a.createNewImageWithoutBorder(args[0], args[1]);
        }
        catch(java.io.IOException e)
        {
            System.out.println("Exception caught.");
        }
 */
    }
}
