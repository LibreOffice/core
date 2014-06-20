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

package org.apache.openoffice.ooxml.schema.automaton;

import java.util.HashMap;
import java.util.Map;

/** A container of states that spans all StateContext objects that represent each
 *  a single complex type.
 */
public class StateContainer
{
    public StateContainer ()
    {
        maNameToStateMap = new HashMap<>();
    }




    boolean HasState (final String sFullname)
    {
        return maNameToStateMap.containsKey(sFullname);
    }




    State GetStateForFullname (final String sFullname)
    {
        return maNameToStateMap.get(sFullname);
    }




    public void AddState (final State aState)
    {
        maNameToStateMap.put(aState.GetFullname(), aState);
    }




    public void RemoveState (final State aState)
    {
        maNameToStateMap.remove(aState);
    }




    private final Map<String,State> maNameToStateMap;
}
