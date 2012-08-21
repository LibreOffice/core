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
package share;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.StringTokenizer;

/**
 *
 * Base Interface to get a description for a given TestJob
 *
 */
public abstract class DescGetter
{

    public abstract DescEntry[] getDescriptionFor(String entry,
            String DescPath,
            boolean debug);

    protected abstract DescEntry getDescriptionForSingleJob(String job,
            String descPath,
            boolean debug);

    protected abstract String[] createScenario(String descPath, String job,
            boolean debug);

    protected DescEntry[] getScenario(String url, String descPath,
            boolean debug)
    {
        ArrayList<DescEntry> entryList = new ArrayList<DescEntry>();
        String line = "";
        BufferedReader scenario = null;
        DescEntry[] entries = null;

        try
        {
            scenario = new BufferedReader(new FileReader(url));
        }
        catch (java.io.FileNotFoundException fnfe)
        {
            System.out.println("Couldn't find file " + url);

            return entries;
        }

        while (line != null)
        {
            try
            {
                if (line.startsWith("-o"))
                {
                    String job = line.substring(3, line.length()).trim();
                    DescEntry aEntry;
                    // special in case several Interfaces are given comma separated
                    if (job.indexOf(",") < 0)
                    {
                        aEntry = getDescriptionForSingleJob(job, descPath,
                                debug);
                    }
                    else
                    {
                        ArrayList<String> subs = getSubInterfaces(job);
                        String partjob = job.substring(0, job.indexOf(",")).trim();
                        aEntry = getDescriptionForSingleJob(partjob, descPath,
                                debug);

                        if (aEntry != null)
                        {
                            for (int i = 0; i < aEntry.SubEntryCount; i++)
                            {
                                String subEntry = aEntry.SubEntries[i].longName;
                                int cpLength = aEntry.longName.length();
                                subEntry = subEntry.substring(cpLength + 2,
                                        subEntry.length());

                                if (subs.contains(subEntry))
                                {
                                    aEntry.SubEntries[i].isToTest = true;
                                }
                            }
                        }
                    }
//                    DescEntry aEntry = getDescriptionForSingleJob(
//                                          line.substring(3).trim(), descPath,
//                                          debug);
                    if (aEntry != null)
                    {
                        entryList.add(aEntry);
                    }
                }
                else if (line.startsWith("-sce"))
                {
                    DescEntry[] subs = getScenario(line.substring(5,
                            line.length()).trim(), descPath,
                            debug);

                    for (int i = 0; i < subs.length; i++)
                    {
                        entryList.add(subs[i]);
                    }
                }
                else if (line.startsWith("-p"))
                {
                    String[] perModule = createScenario(descPath,
                            line.substring(3).trim(), debug);

                    for (int i = 0; i < perModule.length; i++)
                    {
                        DescEntry aEntry = getDescriptionForSingleJob(
                                perModule[i].substring(3).trim(),
                                descPath, debug);
                        if (aEntry != null)
                        {
                            entryList.add(aEntry);
                        }
                    }
                }

                line = scenario.readLine();
            }
            catch (java.io.IOException ioe)
            {
                if (debug)
                {
                    System.out.println("Exception while reading scenario");
                }
            }
        }

        try
        {
            scenario.close();
        }
        catch (java.io.IOException ioe)
        {
            if (debug)
            {
                System.out.println("Exception while closeing scenario");
            }
        }

        if (entryList.size() == 0)
        {
            return null;
        }
        entries = new DescEntry[entryList.size()];
        entries = entryList.toArray(entries);

        return entries;
    }

    protected ArrayList<String> getSubInterfaces(String job)
    {
        ArrayList<String> namesList = new ArrayList<String>();
        StringTokenizer st = new StringTokenizer(job, ",");

        for (; st.hasMoreTokens();)
        {
            String token = st.nextToken();

            if (token.indexOf(".") < 0)
            {
                namesList.add(token);
            }
        }

        return namesList;
    }
}