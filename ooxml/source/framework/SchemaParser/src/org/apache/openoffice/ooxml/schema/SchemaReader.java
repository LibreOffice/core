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

package org.apache.openoffice.ooxml.schema;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.xml.stream.XMLStreamException;

import org.apache.openoffice.ooxml.schema.generator.LogGenerator;
import org.apache.openoffice.ooxml.schema.generator.ParserTablesGenerator;
import org.apache.openoffice.ooxml.schema.generator.automaton.NonValidatingCreator;
import org.apache.openoffice.ooxml.schema.generator.automaton.StackAutomaton;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;
import org.apache.openoffice.ooxml.schema.parser.SchemaParser;
import org.apache.openoffice.ooxml.schema.parser.XmlNamespace;

public class SchemaReader
{
    public static void main (final String ... aArgumentList)
    {
        if (aArgumentList.length != 1)
        {
            System.err.printf("usage: SchemaParser <driver-file>\n");
            System.exit(1);
        }

        final SchemaReader aReader = new SchemaReader(new File(aArgumentList[0]));
        aReader.Run();
    }




    private SchemaReader (final File aDriverFile)
    {
        maSchema = new Schema();
        maMainSchemaFiles = new Vector<>();
        maSchemaFiles = new HashSet<>();
        maTodo = new LinkedList<String>();
        mnTotalLineCount = 0;
        mnTotalByteCount = 0;

        XmlNamespace.Apply(maSchema);

        ParseDriverFile(aDriverFile);
    }




    /** Read and parse the driver file that specifies which schema files to read
     *  and where the output should go.
     */
    private void ParseDriverFile (final File aDriverFile)
    {
        if (aDriverFile == null || ! aDriverFile.exists() || ! aDriverFile.canRead())
        {
            System.err.printf("can not read driver file\n");
            System.exit(1);
        }

        final Pattern aSchemaPattern = Pattern.compile("^\\s*schema\\s+(.*)\\s+(.*)");
        final Pattern aOutputPattern = Pattern.compile("^\\s*output-directory\\s+(.*)");
        try
        {
            final BufferedReader aIn = new BufferedReader(new FileReader(aDriverFile));
            while(true)
            {
                final String sLine = aIn.readLine();
                if (sLine == null)
                    break;
                // Lines starting with # are comment lines and are ignored.
                if (sLine.matches("^\\s*#"))
                    continue;
                // Lines containing only whitespace are also ignored.
                else if (sLine.matches("^\\s*$"))
                    continue;

                Matcher aMatcher = aSchemaPattern.matcher(sLine);
                if (aMatcher.matches())
                {
                    maMainSchemaFiles.add(new String[]{aMatcher.group(1), aMatcher.group(2)});
                }
                else
                {
                    aMatcher = aOutputPattern.matcher(sLine);
                    if (aMatcher.matches())
                    {
                        maOutputDirectory = new File(aMatcher.group(1));
                        if (maOutputDirectory.exists() && ! maOutputDirectory.canWrite())
                        {
                            System.err.printf("can not write output file '%s' \n", maOutputDirectory.toString());
                            System.exit(1);
                        }
                    }
                }


            }
            aIn.close();
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
        }
    }




    private void Run ()
    {
        try
        {
            ParseSchemaFiles();
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
        }
        final Schema aOptimizedSchema = maSchema.GetOptimizedSchema();

        System.out.printf("    used are %d complex types, %d simple types, %d groups and %d top level elements\n",
            aOptimizedSchema.ComplexTypes.GetCount(),
            aOptimizedSchema.SimpleTypes.GetCount(),
            aOptimizedSchema.Groups.GetCount(),
            aOptimizedSchema.TopLevelElements.GetCount());

        LogGenerator.Write(maSchema, new File(maOutputDirectory, "original-schema.txt"));
        LogGenerator.Write(aOptimizedSchema, new File(maOutputDirectory, "bla.txt"));

        final StackAutomaton aAutomaton = CreateStackAutomaton(aOptimizedSchema);

        new ParserTablesGenerator(aAutomaton).Generate(new File("/tmp/ooxml-parser"));
    }




    private void ParseSchemaFiles ()
        throws XMLStreamException
    {
        System.out.printf("parsing %d main schema files\n", maMainSchemaFiles.size());

        for (final String[] aEntry : maMainSchemaFiles)
        {
            final String sShortName = aEntry[0];
            final String sMainSchemaFile = aEntry[1];
            final File aMainSchemaFile = new File(sMainSchemaFile);
            if ( ! aMainSchemaFile.exists())
            {
                System.err.printf("    schema file does not exist\n");
                System.exit(1);
            }
            if ( ! aMainSchemaFile.canRead())
            {
                System.err.printf("can not read schema file\n");
                System.exit(1);
            }

            AddSchemaReference(sMainSchemaFile);
        }

        long nStartTime = System.currentTimeMillis();

        while ( ! maTodo.isEmpty())
        {
            final String sSchemaName = maTodo.poll();
            System.out.printf("parsing %s\n", sSchemaName);
            maSchemaFiles.add(sSchemaName);

            final SchemaParser aParser = new SchemaParser(new File(sSchemaName), maSchema);
            aParser.Parse();

            mnTotalLineCount += aParser.GetLineCount();
            mnTotalByteCount += aParser.GetByteCount();
            for (final File aFile : aParser.GetImportedSchemaFilenames())
                AddSchemaReference(aFile.getAbsolutePath());
        }
        long nEndTime = System.currentTimeMillis();
        System.out.printf("parsed %d schema files with a total of %d lines and %d bytes in %fs\n",
            maSchemaFiles.size(),
            mnTotalLineCount,
            mnTotalByteCount,
            (nEndTime-nStartTime)/1000.0);
        System.out.printf("    found %d complex types, %d simple types, %d groups and %d top level elements\n",
            maSchema.ComplexTypes.GetCount(),
            maSchema.SimpleTypes.GetCount(),
            maSchema.Groups.GetCount(),
            maSchema.TopLevelElements.GetCount());
    }




    private void AddSchemaReference (final String sSchemaFilename)
    {
        if ( ! maSchemaFiles.contains(sSchemaFilename))
        {
            if (sSchemaFilename == null)
                throw new RuntimeException();

            // We don't know yet the file name of the schema, so just store null to mark the schema name as 'known'.
            maSchemaFiles.add(sSchemaFilename);
            maTodo.add(sSchemaFilename);
        }
    }




    private static StackAutomaton CreateStackAutomaton (final Schema aSchema)
    {
        long nStartTime = System.currentTimeMillis();
        StackAutomaton aAutomaton = new NonValidatingCreator(aSchema).Create(new File("/tmp/schema.log"));
        long nEndTime = System.currentTimeMillis();
        System.out.printf(
            "created stack automaton in %fs, it has %d states and %d transitions\n",
            (nEndTime-nStartTime)/1000.0,
            aAutomaton.GetStateCount(),
            aAutomaton.GetTransitionCount());

        /*
        nStartTime = System.currentTimeMillis();
        aAutomaton = aAutomaton.Optimize();
        nEndTime = System.currentTimeMillis();
        System.out.printf(
            "optimized stack automaton in %fs, it now has %d states and %d transitions\n",
            (nEndTime-nStartTime)/1000.0,
            aAutomaton.GetStateCount(),
            aAutomaton.GetTransitionCount());
        */
        return aAutomaton;
    }




    private final Schema maSchema;
    private final Vector<String[]> maMainSchemaFiles;
    private File maOutputDirectory;
    private final Set<String> maSchemaFiles;
    private final Queue<String> maTodo;
    private int mnTotalLineCount;
    private int mnTotalByteCount;
}
