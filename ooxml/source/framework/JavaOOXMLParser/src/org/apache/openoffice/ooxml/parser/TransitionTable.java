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

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

public class TransitionTable
{
    public TransitionTable (final Vector<String[]> aData)
    {
        maTransitions = new HashMap<>();
        int nTransitionCount = 0;

        for (final String[] aLine : aData)
        {
            // Create new transition.
            final int nStartStateId = Integer.parseInt(aLine[1]);
            final int nEndStateId = Integer.parseInt(aLine[2]);
            final int nElementId = Integer.parseInt(aLine[3]);
            final int nElementStateId = Integer.parseInt(aLine[4]);
            final Transition aTransition = new Transition(
                nStartStateId,
                nEndStateId,
                nElementId,
                nElementStateId);
            ++nTransitionCount;

            Map<Integer,Transition> aPerElementTransitions = maTransitions.get(aTransition.GetStartStateId());
            if (aPerElementTransitions == null)
            {
                aPerElementTransitions = new HashMap<>();
                maTransitions.put(aTransition.GetStartStateId(), aPerElementTransitions);
            }
            aPerElementTransitions.put(aTransition.GetElementId(), aTransition);
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




    private final Map<Integer,Map<Integer,Transition>> maTransitions;
}
