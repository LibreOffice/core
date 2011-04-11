/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
