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
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

/** This OOXML parser is based on the output of the schema parser.
 *  It exists to debug the schema parser and as illustration and preparation of
 *  the C++ parse (yet to come.)
 *  Because of this, the parser data (set of states and transitions) are
 *  read at runtime while a real parser would do that at compile time.
 */
public class OOXMLParser
{
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

        long nStartTime = System.currentTimeMillis();
        final StateMachine aMachine = new StateMachine(new File(aArgumentList[0]));
        final InputStream aIn = GetInputStream(aArgumentList[1]);
        final XMLStreamReader aReader = GetStreamReader(aIn, aArgumentList[1]);
        long nEndTime = System.currentTimeMillis();
        System.out.printf("initialzed parser in %fs\n", (nEndTime-nStartTime)/1000.0);

        try
        {
            if (aReader != null)
            {
                nStartTime = System.currentTimeMillis();
                final int  nElementCount = Parse(aReader, aMachine);
                nEndTime = System.currentTimeMillis();
                System.out.printf("parsed %d elements in %fs\n",
                    nElementCount,
                    (nEndTime-nStartTime)/1000.0);
            }
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
        }
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




    private static XMLStreamReader GetStreamReader (
        final InputStream aIn,
        final String sDescription)
    {
        if (aIn == null)
            return null;

        try
        {
            final XMLInputFactory aFactory = (XMLInputFactory)XMLInputFactory.newInstance();
            aFactory.setProperty(XMLInputFactory.IS_REPLACING_ENTITY_REFERENCES, false);
            aFactory.setProperty(XMLInputFactory.IS_SUPPORTING_EXTERNAL_ENTITIES, false);
            aFactory.setProperty(XMLInputFactory.IS_COALESCING, false);

            return (XMLStreamReader)aFactory.createXMLStreamReader(
                sDescription,
                aIn);
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
            return null;
        }
    }




    private static int Parse (
        final XMLStreamReader aReader,
        final StateMachine aMachine)
    {
        int nElementCount = 0;
        try
        {
            final AttributeProvider aAttributeProvider = new AttributeProvider(aReader);
            while (aReader.hasNext())
            {
                final int nCode = aReader.next();
                switch(nCode)
                {
                    case XMLStreamReader.START_ELEMENT:
                        ++nElementCount;
                        if (aMachine.IsInSkipState())
                        {
                            if (Log.Dbg != null)
                                Log.Dbg.printf("is skip state -> starting to skip\n");
                            nElementCount += Skip(aReader);
                        }
                        else if ( ! aMachine.ProcessStartElement(
                            aReader.getNamespaceURI(),
                            aReader.getLocalName(),
                            aReader.getLocation(),
                            aAttributeProvider))
                        {
                            if (Log.Dbg != null)
                                Log.Dbg.printf("starting to skip to recover from error\n");
                            nElementCount += Skip(aReader);
                        }
                        break;

                    case XMLStreamReader.END_ELEMENT:
                        aMachine.ProcessEndElement(
                            aReader.getNamespaceURI(),
                            aReader.getLocalName(),
                            aReader.getLocation());
                        break;

                    case XMLStreamReader.CHARACTERS:
                        final String sText = aReader.getText();
                        if (Log.Dbg != null)
                            Log.Dbg.printf("text [%s]\n", sText.replace("\n", "\\n"));
                        aMachine.ProcessCharacters(sText);
                        break;

                    case XMLStreamReader.END_DOCUMENT:
                        Log.Std.printf("--- end of document ---\n");
                        break;

                    default:
                        Log.Err.printf("can't handle XML event of type %d\n", nCode);
                }
            }

            aReader.close();
        }
        catch (final XMLStreamException aException)
        {
            aException.printStackTrace();
        }

        return nElementCount;
    }




    private static int Skip (final XMLStreamReader aReader)
    {
        if (Log.Dbg != null)
        {
            Log.Dbg.printf("starting to skip on %s at L%dC%d\n",
                aReader.getLocalName(),
                aReader.getLocation().getLineNumber(),
                aReader.getLocation().getColumnNumber());
            Log.Dbg.IncreaseIndentation();
        }

        // We are called when processing a start element.  This means that we are
        // already at relative depth 1.
        int nRelativeDepth = 1;
        int nElementCount = 0;
        try
        {
            while (aReader.hasNext())
            {
                final int nCode = aReader.next();
                switch (nCode)
                {
                    case XMLStreamReader.START_ELEMENT:
                        ++nRelativeDepth;
                        ++nElementCount;
                        if (Log.Dbg != null)
                        {
                            Log.Dbg.printf("skipping start element %s\n", aReader.getLocalName());
                            Log.Dbg.IncreaseIndentation();
                        }
                        break;

                    case XMLStreamReader.END_ELEMENT:
                        --nRelativeDepth;
                        if (Log.Dbg != null)
                            Log.Dbg.DecreaseIndentation();
                        if (nRelativeDepth <= 0)
                        {
                            if (Log.Dbg != null)
                                Log.Dbg.printf("leaving skip mode on %s\n", aReader.getLocalName());
                            return nElementCount;
                        }
                        break;

                    case XMLStreamReader.END_DOCUMENT:
                        throw new RuntimeException("saw end of document while skipping elements\n");

                    case XMLStreamReader.CHARACTERS:
                        SkipText(aReader.getText());
                        break;

                    default:
                        if (Log.Dbg != null)
                            Log.Dbg.printf("%s\n",  nCode);
                        break;
                }
            }
        }
        catch (final XMLStreamException aException)
        {
            aException.printStackTrace();
        }
        return nElementCount;
    }




    private static void SkipText (final String sText)
    {
        if (Log.Dbg != null)
            Log.Dbg.printf("skipping text [%s]\n", sText.replace("\n", "\\n"));
    }
}
