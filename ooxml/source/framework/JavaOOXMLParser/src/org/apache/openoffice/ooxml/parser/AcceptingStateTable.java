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

import java.util.HashSet;
import java.util.Set;

/** List of all accepting states.
 *
 *  The accepting status of states is important when a closing tag is seen.
 *  It denotes the end of the input stream for the state machine of the currently
 *  processed element.  It is an error when the current state is not accepting
 *  when a closing tag is processed.
 */
public class AcceptingStateTable
{
    public AcceptingStateTable (final Iterable<String[]> aData)
    {
        maAcceptingStates = new HashSet<>();

        for (final String[] aLine : aData)
        {
            // Create new transition.
            final int nStateId = Integer.parseInt(aLine[1]);

            maAcceptingStates.add(nStateId);
        }
    }




    public boolean Contains (final int nStateId)
    {
        return maAcceptingStates.contains(nStateId);
    }




    public int GetAcceptingStateCount ()
    {
        return maAcceptingStates.size();
    }




    private final Set<Integer> maAcceptingStates;
}
