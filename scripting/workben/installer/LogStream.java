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

package installer;
import java.io.PrintStream;
import java.io.FileOutputStream;

import java.util.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;


public class LogStream extends PrintStream
{
    static final private DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss z: ");

    private String getTimeStamp()
    {
         String timeStamp = formatter.format( new Date() );
         return timeStamp;
    }
    public LogStream( String logFileName ) throws java.io.FileNotFoundException
    {
        super( new FileOutputStream( logFileName ) );
    }
    public  void println(String x)
    {
        super.println( getTimeStamp() + x );
    }
    public static void main(String[] args)
    {
        if ( args.length > 0 )
        {
            try
            {
                LogStream log = new LogStream( args[0] );
                System.setErr(log);
                System.setOut(log);
                System.out.println("Test from logger from out");
                System.err.println("Test from logger from err");
                System.out.println("finised test from out");
                System.err.println("finised test from err");
            }
            catch( java.io.FileNotFoundException fe )
            {
                System.err.println("Error creating logStream: " + fe );
                fe.printStackTrace();
            }
        }
        else
        {
            System.err.println("specify log file java LogStream [logfile]");
            System.exit(1);
        }
    }
}
