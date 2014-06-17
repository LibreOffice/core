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
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Queue;
import java.util.Set;
import java.util.Vector;

import javax.xml.stream.XMLStreamException;

import org.apache.openoffice.ooxml.schema.automaton.FiniteAutomatonContainer;
import org.apache.openoffice.ooxml.schema.automaton.NonValidatingCreator;
import org.apache.openoffice.ooxml.schema.automaton.ValidatingCreator;
import org.apache.openoffice.ooxml.schema.generator.LogGenerator;
import org.apache.openoffice.ooxml.schema.generator.ParserTablesGenerator;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.parser.SchemaParser;
import org.apache.openoffice.ooxml.schema.simple.SimpleTypeContainer;

public class SchemaReader
{
    public static void main (final String ... aArgumentList)
    {
        if (aArgumentList.length != 1)
        {
            System.err.printf("usage: SchemaParser <driver-file>\n");
            System.err.printf(" driver file can contain these lines:\n");
            System.err.printf("# Comments\n");
            System.err.printf("    are ignored\n");
            System.err.printf("schema <mark> <file-name>\n");
            System.err.printf("    specifies a top-level schema file to read\n");
            System.err.printf("output-schema <file-name>\n");
            System.err.printf("    write schema information to file\n");
            System.err.printf("output-optimized-schema <file-name>\n");
            System.err.printf("    write information about optimized schema to file\n");
            System.exit(1);
        }

        final SchemaReader aReader = new SchemaReader(new File(aArgumentList[0]));
        aReader.Run();
    }




    private SchemaReader (final File aDriverFile)
    {
        maSchemaBase = new SchemaBase();
        maTopLevelSchemas = new HashMap<>();
        maMainSchemaFiles = new Vector<>();
        maSchemaFiles = new HashSet<>();
        maWorkList = new LinkedList<>();
        maOutputOperations = new Vector<>();
        mnTotalLineCount = 0;
        mnTotalByteCount = 0;

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

        try
        {
            final BufferedReader aIn = new BufferedReader(new FileReader(aDriverFile));
            while(true)
            {
                String sLine = aIn.readLine();
                if (sLine == null)
                    break;
                // Lines starting with # are comment lines and are ignored.
                if (sLine.matches("^\\s*#.*"))
                    continue;
                // Lines containing only whitespace are also ignored.
                else if (sLine.matches("^\\s*$"))
                    continue;

                // Handle line continuation.
                while (sLine.endsWith("\\"))
                    sLine = sLine.substring(0, sLine.length()-1) + aIn.readLine();

                final Vector<String> aParts = SplitLine(sLine);
                switch (aParts.get(0))
                {
                    case "schema":
                        maMainSchemaFiles.add(new String[]{aParts.get(1), aParts.get(2)});
                        break;

                    case "output-schema":
                        maOutputOperations.add(new Runnable()
                        {
                            final File maFile = CreateCheckedOutputFile(aParts.get(1));
                            @Override public void run()
                            {
                                WriteSchema(maFile);
                            }
                        });
                        break;

                    case "output-optimized-schema":
                        maOutputOperations.add(new Runnable()
                        {
                            final File maFile = CreateCheckedOutputFile(aParts.get(1));
                            @Override public void run()
                            {
                                WriteOptimizedSchema(maFile);
                            }
                        });
                        break;

                    case "output-nonvalidating-parse-tables":
                        maOutputOperations.add(new Runnable()
                        {
                            final File maAutomatonLogFile = CreateCheckedOutputFile(aParts.get(1));
                            final File maSimpleTypeLogFile = CreateCheckedOutputFile(aParts.get(2));
                            final File maParseTableFile = CreateCheckedOutputFile(aParts.get(3));
                            @Override public void run()
                            {
                                WriteNonValidatingParseTables(
                                    maAutomatonLogFile,
                                    maSimpleTypeLogFile,
                                    maParseTableFile);
                            }
                        });
                        break;

                    case "output-validating-parse-tables":
                        maOutputOperations.add(new Runnable()
                        {
                            final File maAutomatonLogFile = CreateCheckedOutputFile(aParts.get(1));
                            final File maSimpleTypeLogFile = CreateCheckedOutputFile(aParts.get(2));
                            final File maParseTableFile = CreateCheckedOutputFile(aParts.get(3));
                            @Override public void run()
                            {
                                WriteValidatingParseTables(
                                    maAutomatonLogFile,
                                    maSimpleTypeLogFile,
                                    maParseTableFile);
                            }
                        });
                        break;

                    default:
                        System.err.printf("unknown command '%s' in driver file", aParts.get(0));
                        System.exit(1);
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

        maOptimizedSchemaBase = maSchemaBase.GetOptimizedSchema(maTopLevelSchemas.values());
        for (final Entry<String, Schema> aEntry : maTopLevelSchemas.entrySet())
            aEntry.setValue(aEntry.getValue().GetOptimizedSchema(maOptimizedSchemaBase));

        System.out.printf("    optimization left %d complex types and %d simple types\n",
            maOptimizedSchemaBase.ComplexTypes.GetCount(),
            maOptimizedSchemaBase.SimpleTypes.GetCount());

        for (final Runnable aOperation : maOutputOperations)
        {
            aOperation.run();
        }
    }




    private void ParseSchemaFiles ()
        throws XMLStreamException
    {
        System.out.printf("parsing %d main schema files\n", maMainSchemaFiles.size());

        for (final String[] aEntry : maMainSchemaFiles)
        {
            final String sMainSchemaShortname = aEntry[0];
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

            final Schema aSchema = new Schema(sMainSchemaShortname, maSchemaBase);
            ParseSchemaFile(sMainSchemaFile, aSchema);
            maTopLevelSchemas.put(sMainSchemaShortname, aSchema);
        }

        long nStartTime = System.currentTimeMillis();
        while ( ! maWorkList.isEmpty())
        {
            ParseSchemaFile(maWorkList.poll(), null);
        }
        long nEndTime = System.currentTimeMillis();

        System.out.printf("parsed %d schema files with a total of %d lines and %d bytes in %fs\n",
            maSchemaFiles.size(),
            mnTotalLineCount,
            mnTotalByteCount,
            (nEndTime-nStartTime)/1000.0);
        System.out.printf("    found %d complex types and %d simple types\n",
            maSchemaBase.ComplexTypes.GetCount(),
            maSchemaBase.SimpleTypes.GetCount());

        int nTopLevelElementCount = 0;
        for (final Schema aSchema : maTopLevelSchemas.values())
            nTopLevelElementCount += aSchema.TopLevelElements.GetCount();
        System.out.printf("    the %d top level schemas have %d elements\n",
                maTopLevelSchemas.size(),
                nTopLevelElementCount);
    }




    private void ParseSchemaFile (
            final String sSchemaFilename,
            final Schema aSchema)
                    throws XMLStreamException
    {
        System.out.printf("parsing %s\n", sSchemaFilename);
        maSchemaFiles.add(sSchemaFilename);

        final SchemaParser aParser = new SchemaParser(new File(sSchemaFilename), aSchema, maSchemaBase);
        aParser.Parse();

        mnTotalLineCount += aParser.GetLineCount();
        mnTotalByteCount += aParser.GetByteCount();
        for (final File aFile : aParser.GetImportedSchemaFilenames())
            AddSchemaReference(aFile.getAbsolutePath());
    }




    private void AddSchemaReference (final String sSchemaFilename)
    {
        if ( ! maSchemaFiles.contains(sSchemaFilename))
        {
            if (sSchemaFilename == null)
                throw new RuntimeException();

            // We don't know yet the file name of the schema, so just store null to mark the schema name as 'known'.
            maSchemaFiles.add(sSchemaFilename);
            maWorkList.add(sSchemaFilename);
        }
    }




    /** Split the given string at whitespace but not at whitespace inside double quotes.
     *
     */
    private Vector<String> SplitLine (final String sLine)
    {
        final Vector<String> aParts = new Vector<>();

        boolean bIsInsideQuotes = false;
        for (final String sPart : sLine.split("\""))
        {
            if (bIsInsideQuotes)
                aParts.add(sPart);
            else
                for (final String sInnerPart : sPart.split("\\s+"))
                {
                    if (sInnerPart == null)
                        throw new RuntimeException();
                    else if ( ! sInnerPart.isEmpty())
                        aParts.add(sInnerPart);
                }

            bIsInsideQuotes = ! bIsInsideQuotes;
        }

        return aParts;
    }




    /** Create a File object for a given file name.
     *  Check that the file is writable, i.e. its directory exists and that if
     *  the file already exists it can be replaced.
     *  Throws a RuntimeException when a check fails.
     */
    private File CreateCheckedOutputFile (final String sFilename)
    {
        final File aFile = new File(sFilename);
        if ( ! aFile.getParentFile().exists())
            throw new RuntimeException("directory of "+sFilename+" does not exist: can not create file");
        if (aFile.exists() && ! aFile.canWrite())
            throw new RuntimeException("file "+sFilename+" already exists and can not be replaced");
        return aFile;
    }




    private void WriteSchema (final File aOutputFile)
    {
        LogGenerator.Write(aOutputFile, maSchemaBase, maTopLevelSchemas.values());
    }




    private void WriteOptimizedSchema (final File aOutputFile)
    {
        LogGenerator.Write(aOutputFile, maOptimizedSchemaBase, maTopLevelSchemas.values());
    }




    private void WriteNonValidatingParseTables (
        final File aAutomatonLogFile,
        final File aSimpleTypeLogFile,
        final File aParseTableFile)
    {
        long nStartTime = System.currentTimeMillis();
        final NonValidatingCreator aCreator = new NonValidatingCreator(maOptimizedSchemaBase, aAutomatonLogFile);
        FiniteAutomatonContainer aAutomatons = aCreator.Create(maTopLevelSchemas.values());
        long nEndTime = System.currentTimeMillis();
        System.out.printf(
            "created %d non-validating automatons with %d states and %d transitions in %fs\n",
            aAutomatons.GetAutomatonCount(),
            aAutomatons.GetStateCount(),
            aAutomatons.GetTransitionCount(),
            (nEndTime-nStartTime)/1000.0);

        nStartTime = System.currentTimeMillis();
        final SimpleTypeContainer aSimpleTypes = SimpleTypeContainer.Create(
            maOptimizedSchemaBase,
            aSimpleTypeLogFile);
        nEndTime = System.currentTimeMillis();
        System.out.printf(
            "created %d simple type descriptions in %fs\n",
            aSimpleTypes.GetSimpleTypeCount(),
            (nEndTime-nStartTime)/1000.0);

        new ParserTablesGenerator(
            aAutomatons,
            maOptimizedSchemaBase.Namespaces,
            aSimpleTypes,
            maOptimizedSchemaBase.AttributeValueToIdMap)
            .Generate(aParseTableFile);
    }




    private void WriteValidatingParseTables (
        final File aAutomatonLogFile,
        final File aSimpleTypeLogFile,
        final File aParseTableFile)
    {
        long nStartTime = System.currentTimeMillis();
        final ValidatingCreator aCreator = new ValidatingCreator(maOptimizedSchemaBase, aAutomatonLogFile);
        FiniteAutomatonContainer aAutomatons = aCreator.Create();
        long nEndTime = System.currentTimeMillis();
        System.out.printf(
            "created %d validating stack automatons with %d states and %d transitions in %fs\n",
            aAutomatons.GetAutomatonCount(),
            aAutomatons.GetStateCount(),
            aAutomatons.GetTransitionCount(),
            (nEndTime-nStartTime)/1000.0);


        nStartTime = System.currentTimeMillis();
        aAutomatons = aAutomatons.CreateDFAs();
        nEndTime = System.currentTimeMillis();
        System.out.printf(
            "created %d deterministic automatons with %d states and %d transitions in %fs\n",
            aAutomatons.GetAutomatonCount(),
            aAutomatons.GetStateCount(),
            aAutomatons.GetTransitionCount(),
            (nEndTime-nStartTime)/1000.0);

        nStartTime = System.currentTimeMillis();
        aAutomatons = aAutomatons.MinimizeDFAs();
        nEndTime = System.currentTimeMillis();
        System.out.printf(
            "minimized automaton in %fs, there are now %d states and %d transitions\n",
            (nEndTime-nStartTime)/1000.0,
            aAutomatons.GetStateCount(),
            aAutomatons.GetTransitionCount());

        nStartTime = System.currentTimeMillis();
        final SimpleTypeContainer aSimpleTypes = SimpleTypeContainer.Create(
            maOptimizedSchemaBase,
            aSimpleTypeLogFile);
        nEndTime = System.currentTimeMillis();
        System.out.printf(
            "created %d simple type descriptions in %fs\n",
            aSimpleTypes.GetSimpleTypeCount(),
            (nEndTime-nStartTime)/1000.0);

        new ParserTablesGenerator(
            aAutomatons,
            maOptimizedSchemaBase.Namespaces,
            aSimpleTypes,
            maOptimizedSchemaBase.AttributeValueToIdMap)
            .Generate(aParseTableFile);
    }




    private final SchemaBase maSchemaBase;
    private SchemaBase maOptimizedSchemaBase;
    private final Map<String,Schema> maTopLevelSchemas;
    private final Vector<String[]> maMainSchemaFiles;
    private final Queue<String> maWorkList;
    private final Vector<Runnable> maOutputOperations;
    private final Set<String> maSchemaFiles;
    private int mnTotalLineCount;
    private int mnTotalByteCount;
}
