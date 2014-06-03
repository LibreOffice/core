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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class TransitionTable
{
    public TransitionTable (final File aDataLocation)
    {
        maTransitions = new HashMap<>();
        maSkipStates = new HashSet<>();
        int nTransitionCount = 0;

        try
        {
            final BufferedReader aReader = new BufferedReader(new FileReader(new File(aDataLocation, "transitions.lst")));
            while(true)
            {
                // Read line, ignore comments, split into parts at whitespace.
                final String sLine = aReader.readLine();
                if (sLine == null)
                    break;
                if (sLine.startsWith("#"))
                    continue;
                final String[] aParts = sLine.split("\\s+");

                // Create new transition.
                final int nStartStateId = Integer.parseInt(aParts[0]);
                final int nEndStateId = Integer.parseInt(aParts[1]);
                final int nElementId = Integer.parseInt(aParts[2]);
                final int nActionStateId = Integer.parseInt(aParts[4]);
                if (nElementId==-1 && nActionStateId==-1)
                    maSkipStates.add(nStartStateId);
                else
                {
                    final Transition aTransition = new Transition(
                        nStartStateId,
                        nEndStateId,
                        nElementId,
                        aParts[3],
                        nActionStateId);
                    ++nTransitionCount;

                    Map<Integer,Transition> aPerElementTransitions = maTransitions.get(aTransition.GetStartStateId());
                    if (aPerElementTransitions == null)
                    {
                        aPerElementTransitions = new HashMap<>();
                        maTransitions.put(aTransition.GetStartStateId(), aPerElementTransitions);
                    }
                    aPerElementTransitions.put(aTransition.GetElementId(), aTransition);
                }
            }
            aReader.close();
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
        }
        Log.Std.printf("read %d transitions\n",  nTransitionCount);
    }




    public Transition GetTransition (
        final int nStateId,
        final int nElementId)
    {
        Map<Integer,Transition> aPerElementTransitions = maTransitions.get(nStateId);
        if (aPerElementTransitions == null)
            return null;
        else
            return aPerElementTransitions.get(nElementId);
    }




    public Set<Integer> GetSkipStates ()
    {
        return maSkipStates;
    }




    private final Map<Integer,Map<Integer,Transition>> maTransitions;
    private final Set<Integer> maSkipStates;
}
