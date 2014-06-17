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
import java.io.FileInputStream;
import java.io.InputStream;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import javax.xml.stream.Location;

import org.apache.openoffice.ooxml.parser.action.ActionManager;
import org.apache.openoffice.ooxml.parser.action.ActionTrigger;
import org.apache.openoffice.ooxml.parser.action.IAction;

/** This OOXML parser is based on the output of the schema parser.
 *  It exists to debug the schema parser and as illustration and preparation of
 *  the C++ parse (yet to come.)
 *  Because of this, the parser data (set of states and transitions) are
 *  read at runtime while a real parser would do that at compile time.
 */
public class OOXMLParser
{
    class ActionContext
    {
        public Map<String,Integer> TypeCounts = new TreeMap<>();
    }
    /** The parser is called with two arguments:
     *  - A path to where the parser tables with the states and transitions can
     *    be found.
     *  - The XML input file or Zip stream to parse.
     *    The syntax for a Zip stream contains a '#' that separates the filename
     *    to its left from the entry name to its right.
     */
    public static void main (final String ... aArgumentList)
    {
        if (aArgumentList.length<2 ||aArgumentList.length>3)
            throw new RuntimeException("usage: OOXMLParser <parser-tables-path> <XML-input-file> <log-file>?");

        if (aArgumentList.length == 3)
        {
            final File aLogFile = new File(aArgumentList[2]);
            Log.Dbg = new Log(aLogFile);
            System.out.printf("writing log data to %s\n", aLogFile.toString());
        }
        else
        {
            Log.Dbg = null;
            System.out.printf("writing no log data\n");
        }

        new OOXMLParser(aArgumentList[0], aArgumentList[1]);
    }



    private OOXMLParser (
        final String sParseTableFilename,
        final String sInputFilename)
    {
        long nStartTime = System.currentTimeMillis();
        final StateMachine aMachine = new StateMachine(new File(sParseTableFilename));
        final InputStream aIn = GetInputStream(sInputFilename);
        long nEndTime = System.currentTimeMillis();

        final ActionContext aActionContext = new ActionContext();
        AddSomeActions(aMachine.GetActionManager(), aActionContext);

        System.out.printf("initialzed parser in %fs\n", (nEndTime-nStartTime)/1000.0);

        try
        {
            nStartTime = System.currentTimeMillis();
            final Parser aParser = new Parser(aMachine, aIn);
            aParser.Parse();
            final int  nElementCount = aParser.GetElementCount();
            nEndTime = System.currentTimeMillis();
            System.out.printf("parsed %d elements in %fs\n",
                nElementCount,
                (nEndTime-nStartTime)/1000.0);

            System.out.printf("%d different elements found:\n", aActionContext.TypeCounts.size());
            for (final Entry<String, Integer> aEntry : aActionContext.TypeCounts.entrySet())
            {
                System.out.printf("%-32s : %6d\n", aEntry.getKey(), aEntry.getValue());
            }
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
        }
    }




    private static void AddSomeActions (
        final ActionManager aActionManager,
        final ActionContext aActionContext)
    {
        aActionManager.AddElementStartAction(
            "*",
            new IAction()
            {
                @Override public void Run(
                    final ActionTrigger eTrigger,
                    final ElementContext aContext,
                    final String sText,
                    final Location aLocation)
                {
                    Integer nValue = aActionContext.TypeCounts.get(aContext.GetTypeName());
                    if (nValue == null)
                        nValue = 1;
                    else
                        ++nValue;
                    aActionContext.TypeCounts.put(aContext.GetTypeName(), nValue);
                }
            }
        );
        aActionManager.AddElementStartAction(
            ".*CT_Shd",
            new IAction()
            {
                @Override public void Run(
                    final ActionTrigger eTrigger,
                    final ElementContext aContext,
                    final String sText,
                    final Location aLocation)
                {
                    System.out.printf("processing %s of element %s at position %d\n",
                        eTrigger,
                        aContext.GetElementName(),
                        aLocation.getCharacterOffset());

                    if (aContext.GetAttributes().GetAttributeCount() == 0)
                        System.out.printf("    no attributes\n");
                    else
                        for (final Entry<String,String> aAttribute : aContext.GetAttributes().GetAttributes())
                            System.out.printf("    %s -> %s\n", aAttribute.getKey(), aAttribute.getValue());
                }
            }
        );
        aActionManager.AddTextAction(
            ".*CT_Text",
            new IAction()
            {
                @Override public void Run(
                    final ActionTrigger eTrigger,
                    final ElementContext aContext,
                    final String sText,
                    final Location aLocation)
                {
//                    System.out.printf("%s text \"%s\"\n", aContext.GetTypeName(), sText.replace("\n", "\\n"));
                }
            }
        );
    }




    private static InputStream GetInputStream (final String sInputName)
    {
        final InputStream aIn;
        try
        {
            final int nSeparator = sInputName.indexOf('#');
            if (nSeparator >= 0)
            {
                // Split the input name into the file name of the archive and the
                // name of a zip entry.
                final String sArchiveName = sInputName.substring(0, nSeparator);
                String sEntryName = sInputName.substring(nSeparator+1);

                // Normalize and cleanup the entry name.
                sEntryName = sEntryName.replace('\\',  '/');
                if (sEntryName.startsWith("/"))
                    sEntryName = sEntryName.substring(1);

                final ZipFile aZipFile = new ZipFile(new File(sArchiveName));
                final ZipEntry aZipEntry = aZipFile.getEntry(sEntryName);
                aIn = aZipFile.getInputStream(aZipEntry);
            }
            else
            {
                // The input name points to a plain XML file.
                aIn = new FileInputStream(sInputName);
            }
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
            return null;
        }
        return aIn;
    }
}
