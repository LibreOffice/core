/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DescGetter.java,v $
 * $Revision: 1.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package share;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.StringTokenizer;

import java.util.Vector;


/**
 *
 * Base Interface to get a description for a given TestJob
 *
 */
public abstract class DescGetter {
    public abstract DescEntry[] getDescriptionFor(String entry,
                                                  String DescPath,
                                                  boolean debug);

    protected abstract DescEntry getDescriptionForSingleJob(String job,
                                                            String descPath,
                                                            boolean debug);

    protected abstract String[] createScenario(String descPath, String job,
                                               boolean debug);

    protected DescEntry[] getScenario(String url, String descPath,
                                      boolean debug) {
        Vector entryList = new Vector();
        String line = "";
        BufferedReader scenario = null;
        DescEntry[] entries = null;

        try {
            scenario = new BufferedReader(new FileReader(url));
        } catch (java.io.FileNotFoundException fnfe) {
            System.out.println("Couldn't find file " + url);

            return entries;
        }

        while (line != null) {
            try {
                if (line.startsWith("-o")) {
                    String job = line.substring(3, line.length()).trim();
                    DescEntry aEntry ;
                    // special in case several Interfaces are given comma separated
                    if (job.indexOf(",") < 0) {
                        aEntry = getDescriptionForSingleJob(job, descPath,
                                                                     debug);
                    } else {
                        ArrayList subs = getSubInterfaces(job);
                        String partjob = job.substring(0, job.indexOf(",")).trim();
                        aEntry = getDescriptionForSingleJob(partjob, descPath,
                                                                     debug);

                        if (aEntry != null) {
                            for (int i = 0; i < aEntry.SubEntryCount; i++) {
                                String subEntry = aEntry.SubEntries[i].longName;
                                int cpLength = aEntry.longName.length();
                                subEntry = subEntry.substring(cpLength + 2,
                                                              subEntry.length());

                                if (subs.contains(subEntry)) {
                                    aEntry.SubEntries[i].isToTest = true;
                                }
                            }
                        }
                    }
//                    DescEntry aEntry = getDescriptionForSingleJob(
//                                          line.substring(3).trim(), descPath,
//                                          debug);
                    if (aEntry != null)
                        entryList.add(aEntry);
                } else if (line.startsWith("-sce")) {
                    DescEntry[] subs = getScenario(line.substring(5,
                                                                  line.length())
                                                       .trim(), descPath,
                                                   debug);

                    for (int i = 0; i < subs.length; i++) {
                        entryList.add(subs[i]);
                    }
                } else if (line.startsWith("-p")) {
                    String[] perModule = createScenario(descPath,
                                                        line.substring(3)
                                                            .trim(), debug);

                    for (int i = 0; i < perModule.length; i++) {
                        DescEntry aEntry = getDescriptionForSingleJob(
                                                   perModule[i].substring(3)
                                                               .trim(),
                                                   descPath, debug);
                        if (aEntry != null)
                            entryList.add(aEntry);
                    }
                }

                line = scenario.readLine();
            } catch (java.io.IOException ioe) {
                if (debug) {
                    System.out.println("Exception while reading scenario");
                }
            }
        }

        try {
            scenario.close();
        } catch (java.io.IOException ioe) {
            if (debug) {
                System.out.println("Exception while closeing scenario");
            }
        }

        if (entryList.size() == 0)
            return null;
        entries = new DescEntry[entryList.size()];
        entries = (DescEntry[]) entryList.toArray(entries);

        return entries;
    }

    protected ArrayList getSubInterfaces(String job) {
        ArrayList namesList = new ArrayList();
        StringTokenizer st = new StringTokenizer(job, ",");

        for (int i = 0; st.hasMoreTokens(); i++) {
            String token = st.nextToken();

            if (token.indexOf(".") < 0) {
                namesList.add(token);
            }
        }

        return namesList;
    }
}