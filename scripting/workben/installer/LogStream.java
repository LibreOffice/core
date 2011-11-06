/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package installer;
import java.io.PrintStream;
import java.io.FileOutputStream;

import java.util.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;


//import java.io.PrintWriter;
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
