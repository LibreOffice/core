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



package com.sun.star.filter.config.tools.merge;

//_______________________________________________

import java.lang.*;
import java.util.*;
import java.io.*;
import com.sun.star.filter.config.tools.utils.*;

//_______________________________________________

/**
 *  Its a simple command line tool, which can merge different XML fragments
 *  together. Such fragments must exist as files on disk, will be moved into
 *  one file together on disk.
 *
 *
 */
public class FCFGMerge
{
    //___________________________________________
    // const

    private static final java.lang.String CFGFILE = "com/sun/star/filter/config/tools/merge/FCFGMerge.cfg";
    private static final java.lang.String PROP_LOGLEVEL = "loglevel";

    //___________________________________________
    // main

    public static void main(java.lang.String[] sCmdLine)
    {
        FCFGMerge.printCopyright();

        // create log object in default mode "WARNINGS"
        // If a command line parameter indicates another
        // level - change it!
        Logger aLog = new Logger();

        try
        {
            // merge config file and overwrite properties
            // via command line
            ConfigHelper aCfg = null;
            aCfg = new ConfigHelper(CFGFILE, sCmdLine);

            // update log level
            int nLogLevel = aCfg.getInt(PROP_LOGLEVEL, Logger.LEVEL_WARNINGS);
            aLog = new Logger(nLogLevel);

            // help requested?
            if (aCfg.isHelp())
            {
                FCFGMerge.printHelp();
                System.exit(-1);
            }

            // create new merge object and start operation
            Merger aMerger = new Merger(aCfg, aLog);
            aMerger.merge();
        }
        catch(java.lang.Throwable ex)
        {
            aLog.setException(ex);
            System.exit(-1);
        }

        System.exit(0);
    }

    //___________________________________________

    /** prints out a copyright message on stdout.
     */
    private static void printCopyright()
    {
        java.lang.StringBuffer sOut = new java.lang.StringBuffer(256);
        sOut.append("FCFGMerge\n");
        sOut.append("Copyright: 2003 by Sun Microsystems, Inc.\n");
        sOut.append("All Rights Reserved.\n");
        System.out.println(sOut.toString());
    }

    //___________________________________________

    /** prints out a help message on stdout.
     */
    private static void printHelp()
    {
        java.lang.StringBuffer sOut = new java.lang.StringBuffer(256);
        sOut.append("____________________________________________________________\n");
        sOut.append("usage: FCFGMerge cfg=<file name>\n"                            );
        sOut.append("parameters:\n"                                                 );
        sOut.append("\tcfg=<file name>\n"                                           );
        sOut.append("\t\tmust point to a system file, which contains\n"             );
        sOut.append("\t\tall necessary configuration data for the merge process.\n");
        sOut.append("\n\tFurther cou can specify every parameter allowed in the\n"  );
        sOut.append("\n\tconfig file as command line parameter too, to overwrite\n" );
        sOut.append("\n\tthe value from the file.\n"                                );
        System.out.println(sOut.toString());
    }
}
