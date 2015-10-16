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
package stats;

import java.util.ArrayList;

import share.DescEntry;

/**
 *
 * this class adds up the results of the subentries of a given DescEntry
 * and fills the subentries in cases of SKIPPED states
 */
public class Summarizer
{

    /**
     *
     * gets the state for a SuperEntry according to its subentries
     */
    public void summarizeUp(DescEntry entry)
    {
        if ((entry.State != null) && !entry.State.equals("UNKNOWN"))
        {
            return;
        }
        int count = entry.SubEntryCount;
        int knownIssues = 0;
        ArrayList<String> failures = new ArrayList<String>();
        ArrayList<String> states = new ArrayList<String>();
        for (int i = 0; i < count; i++)
        {
            if (entry.SubEntries[i].State == null)
            {
                entry.SubEntries[i].State = "COMPLETED.FAILED";
            }
            if (entry.SubEntries[i].State.equals("known issue"))
            {
                entry.SubEntries[i].State = "COMPLETED.OK";
                knownIssues++;
            }
            if (!entry.SubEntries[i].State.endsWith("OK"))
            {
                String sFailure = "[" + entry.SubEntries[i].longName + "]" + " is testcode: [" + entry.SubEntries[i].entryName + "]";
                failures.add(sFailure);
                states.add(entry.SubEntries[i].State);
            }
        }
        if (failures.size() > 0)
        {
            String errMsg = "";
            String state = "COMPLETED.FAILED";
            for (int j = 0; j < failures.size(); j++)
            {
                if (states.get(j).equals("not part of the job"))
                {
                    state = "COMPLETED(some interfaces/services not tested).OK";
                }
                else
                {
                    errMsg +=
                            failures.get(j) + " - " + states.get(j) + "\r\n";
                }
            }
            entry.hasErrorMsg = true;
            entry.ErrorMsg = errMsg;
            entry.State = state;
        }
        else if (entry.EntryType.equals("component") && knownIssues > 0)
        {
            entry.State = "COMPLETED(with known issues).OK";
        }
        else
        {
            entry.State = "COMPLETED.OK";
        }
    }

    public static void summarizeDown(DescEntry entry, String state)
    {
        if ((entry.State == null) || entry.State.equals("UNKNOWN"))
        {
            entry.State = state;
        }
        for (int i = 0; i < entry.SubEntryCount; i++)
        {
            summarizeDown(entry.SubEntries[i], entry.State);
        }
    }
}
