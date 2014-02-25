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

package convwatch;

import java.io.File;
import java.io.RandomAccessFile;
import helper.OSHelper;

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
                    GlobalLogWriter.get().println( "Active wait since " + nCount + "sec..");
                    nCount ++;
                    sleep( 1 );
                }
                else
                {
                    sleep( 1 );
                    if ( _aSemaphore.exists() )
                    {
                        // ups
                        GlobalLogWriter.get().println( "ups...");
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
                aWriter.writeByte(1);
                aWriter.close();
            }

            catch (java.io.FileNotFoundException fne)
            {
                GlobalLogWriter.get().println( "caught: FileNotFoundException");
            }
            catch(java.io.IOException ie)
            {
                GlobalLogWriter.get().println( "caught: IOException");
            }
        }


// block a resource
    public void V(File _aSemaphore)
        {

            if ( _aSemaphore.exists() )
            {
                _aSemaphore.delete();
            }
            else
            {
                GlobalLogWriter.get().println("Could be a problem here? No resource block found.");
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
