/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimpleFileSemaphore.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-06-04 13:30:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
