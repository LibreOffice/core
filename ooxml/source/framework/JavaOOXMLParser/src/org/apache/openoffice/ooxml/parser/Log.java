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

package org.apache.openoffice.ooxml.parser;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

public class Log
{
    public static Log Std = new Log(System.out);
    public static Log Err = new Log(System.err);
    public static Log Dbg = Std;


    public Log (final OutputStream aOut)
    {
        maOut = aOut;
        msIndentation = "";
    }




    public Log (final String sFilename)
    {
        this(CreateFileOutputStream(sFilename));
    }




    public Log (final File aFile)
    {
        this(CreateFileOutputStream(aFile));
    }




    private static OutputStream CreateFileOutputStream (final File aFile)
    {
        try
        {
            return new FileOutputStream(aFile);
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
            return null;
        }
    }




    public void printf (final String sFormat, final Object ... aArgumentList)
    {
        try
        {
            maOut.write(msIndentation.getBytes());
            maOut.write(String.format(sFormat, aArgumentList).getBytes());
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }




    public void IncreaseIndentation ()
    {
        msIndentation += "    ";
    }




    public void DecreaseIndentation ()
    {
        msIndentation = msIndentation.substring(4);
    }




    private static OutputStream CreateFileOutputStream (final String sFilename)
    {
        OutputStream aOut;
        try
        {
            aOut = new FileOutputStream(sFilename);
            return aOut;
        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
            return null;
        }
    }




    private final OutputStream maOut;
    private String msIndentation;
}
