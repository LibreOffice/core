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

package org.apache.openoffice.ooxml.schema.misc;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;

/** Make output with indentation easier.
 */
public class Log
{
    public Log (final File aFile)
    {
        PrintStream aOut = null;
        try
        {
            aOut = new PrintStream(new FileOutputStream(aFile));
        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        }
        maOut = aOut;
        mbIsActive = maOut!=null;
        msIndentation = "";
    }




    public void AddComment (
        final String sFormat,
        final Object ... aArgumentList)
    {
        if (mbIsActive)
        {
            maOut.print(msIndentation);
            maOut.print("// ");
            maOut.printf(sFormat, aArgumentList);
            maOut.print("\n");
        }
    }




    public void StartBlock ()
    {
        if (mbIsActive)
            msIndentation += "    ";
    }




    public void EndBlock ()
    {
        if (mbIsActive)
            msIndentation = msIndentation.substring(4);
    }




    public void printf (
        final String sFormat, final Object ... aArgumentList)
    {
        if (mbIsActive)
        {
            final String sMessage = String.format(sFormat, aArgumentList);
            maOut.print(msIndentation);
            maOut.print(sMessage);
        }
    }




    public void println (
        final String sMessage)
    {
        if (mbIsActive)
        {
            maOut.print(msIndentation);
            maOut.print(sMessage);
            maOut.print("\n");
        }
    }




    public void Close()
    {
        if (mbIsActive)
            maOut.close();
    }




    private final PrintStream maOut;
    private final boolean mbIsActive;
    private String msIndentation;
}
