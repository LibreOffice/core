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


package stats;

import java.util.Vector;
import share.DescEntry;

/**
 *
 * this class summs up the results of the subentries of a given DescEntry<br>
 * and fills the subentries in cases of SKIPPED states
 */
public class Summarizer
{

    /**
     *
     * gets the state for a SuperEntry according to its subentries
     * @param entry
     */
    public void summarizeUp(DescEntry entry)
    {
        if ((entry.State != null) && !entry.State.equals("UNKNOWN"))
        {
            return;
        }
        int count = entry.SubEntryCount;
        int knownIssues = 0;
        Vector failures = new Vector();
        Vector states = new Vector();
        for (int i = 0; i < count; i++)
        {
            if (entry.SubEntries[i].State == null)
            {
                entry.SubEntries[i].State = "PASSED.FAILED";
            }
            if (entry.SubEntries[i].State.equals("known issue"))
            {
                entry.SubEntries[i].State = "PASSED.OK";
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
            String state = "PASSED.FAILED";
            for (int j = 0; j < failures.size(); j++)
            {
                if (states.elementAt(j).equals("not part of the job"))
                {
                    state = "PASSED(some interfaces/services not tested).OK";
                }
                else
                {
                    errMsg +=
                            failures.elementAt(j) + " - " + states.elementAt(j) + "\r\n";
                }
            }
            entry.hasErrorMsg = true;
            entry.ErrorMsg = errMsg;
            entry.State = state;
        }
        else if (entry.EntryType.equals("component") && knownIssues > 0)
        {
            entry.State = "PASSED(with known issues).OK";
        }
        else
        {
            entry.State = "PASSED.OK";
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
