/*************************************************************************
 *
 *  $RCSfile: Summarizer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:17:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
public class Summarizer {

    /**
     *
     * gets the state for a SuperEntry according to its subentries
     */
    public void summarizeUp(DescEntry entry) {
        if ( ( entry.State != null ) && entry.State != "UNKNOWN") return;
        int count = entry.SubEntryCount;
        int knownIssues = 0;
        Vector failures = new Vector();
        Vector states = new Vector();
        for (int i=0; i<count; i++) {
            if (entry.SubEntries[i].State == null) {
                entry.SubEntries[i].State = "PASSED.FAILED";
            }
            if (entry.SubEntries[i].State == "known issue") {
                entry.SubEntries[i].State = "PASSED.OK";
                knownIssues++;
            }
            if (!entry.SubEntries[i].State.endsWith("OK")) {
                failures.add(entry.SubEntries[i].entryName);
                states.add(entry.SubEntries[i].State);
            }
        }
        if (failures.size()>0) {
            String errMsg = "";
            String state = "PASSED.FAILED";
            for (int j=0; j<failures.size();j++) {
                if (states.elementAt(j).equals("not part of the job")) {
                    state = "Not possible since not all Interfaces/Services have been checked";
                } else errMsg +=
                        failures.elementAt(j)+" - "+states.elementAt(j)+"\r\n";
            }
            entry.hasErrorMsg=true;
            entry.ErrorMsg = errMsg;
            entry.State = state;
        } else if (entry.EntryType.equals("component") && knownIssues > 0) {
            entry.State = "PASSED(with known issues).OK";
        } else {
            entry.State = "PASSED.OK";
        }
    }

    public static void summarizeDown(DescEntry entry, String state) {
        if ( ( entry.State == null ) || entry.State == "UNKNOWN")
            entry.State = state;
        for (int i=0; i<entry.SubEntryCount; i++) {
            summarizeDown(entry.SubEntries[i], entry.State);
        }
    }

}
