/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SimpleFileSemaphore.java,v $
 * $Revision: 1.3 $
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
import java.io.RandomAccessFile;

public class SimpleFileSemaphore /* extends *//* implements */
{
    String m_sInternSemaphoreFile;
    File m_aInternSemaphoreFile;
    GlobalLogWriter m_aLog;

    public static void sleep( int _nSeconds)
        {
            // wait a second here
            try
            {
                java.lang.Thread.sleep(_nSeconds * 1000);
            }
            catch (java.lang.InterruptedException e2)
            {
            }
        }

    public SimpleFileSemaphore() throws IllegalArgumentException
        {
            String sInternFileName;
            if (OSHelper.isWindows())
            {
                sInternFileName = "C:/Temp/ConvwatchOOoSemaphore.txt";
            }
            else if (OSHelper.isUnix())
            {
                sInternFileName = "/tmp/ConvwatchOOoSemaphore.txt";
            }
            else
            {
                m_sInternSemaphoreFile = null;
                throw new IllegalArgumentException("Unknown System, can't initialise SimpleFileSemaphore");
            }

            m_sInternSemaphoreFile = sInternFileName;
            m_aInternSemaphoreFile = new File(sInternFileName);
        }

    public File getSemaphoreFile()
        {
            return m_aInternSemaphoreFile;
        }
// ------------------------------------------------------------------------------
// wait until resource is available
    public void P(File _aSemaphore)
        {
            int nCount = 0;
            int nCheckLoop = 1;

            while ( nCheckLoop == 1)
            {
                // check if resource is available, if not, wait.
                if ( _aSemaphore.exists() )
                {
                    m_aLog.get().println( "Active wait since " + nCount + "sec..");
                    nCount ++;
                    sleep( 1 );
                }
                else
                {
                    sleep( 1 );
                    if ( _aSemaphore.exists() )
                    {
                        // ups
                        m_aLog.get().println( "ups...");
                    }
                    else
                    {
                        nCheckLoop = 0;
                    }
                }
            }

            // block resource by ourself
            try
            {
                RandomAccessFile aWriter = new RandomAccessFile(_aSemaphore, "rw");
                aWriter.writeByte((int)1);
                aWriter.close();
            }

            catch (java.io.FileNotFoundException fne)
            {
                m_aLog.get().println( "caught: FileNotFoundException");
            }
            catch(java.io.IOException ie)
            {
                m_aLog.get().println( "caught: IOException");
            }
        }

// ------------------------------------------------------------------------------
// block a resource
    public void V(File _aSemaphore)
        {

            if ( _aSemaphore.exists() )
            {
                _aSemaphore.delete();
            }
            else
            {
                m_aLog.get().println("Could be a problem here? No resource block found.");
            }
        }

    // --------------------------------- Unit test ---------------------------------

    private static boolean SEMAPHORE_SHOULD_EXIST = true;
    private static boolean SEMAPHORE_SHOULD_NOT_EXIST = false;

    private static void assure(boolean _b, String _sText)
        {
            System.out.print(_sText);
            System.out.print("  ");
            if (_b)
            {
                System.out.println("ok");
            }
            else
            {
                System.out.println("FAILED");
            }
        }

    private static void testSemaphoreFile(SimpleFileSemaphore a, boolean _bShouldFileExists)
        {
            System.out.println("Check if semaphore file exists.");
            File aSemaphoreFile = a.getSemaphoreFile();
            if (aSemaphoreFile.exists())
            {
                System.out.println("Name is: " + aSemaphoreFile.getAbsolutePath());
                assure(_bShouldFileExists == SEMAPHORE_SHOULD_EXIST, "Semaphore should exist!");
            }
            else
            {
                assure(_bShouldFileExists == SEMAPHORE_SHOULD_NOT_EXIST, "Semaphore should not exist!");
            }
        }

    public static void main( String[] argv )
        {
            SimpleFileSemaphore a = new SimpleFileSemaphore();

            testSemaphoreFile(a, SEMAPHORE_SHOULD_NOT_EXIST);

            a.P(a.getSemaphoreFile());

            testSemaphoreFile(a, SEMAPHORE_SHOULD_EXIST);

            a.V(a.getSemaphoreFile());

            testSemaphoreFile(a, SEMAPHORE_SHOULD_NOT_EXIST);
        }
}
