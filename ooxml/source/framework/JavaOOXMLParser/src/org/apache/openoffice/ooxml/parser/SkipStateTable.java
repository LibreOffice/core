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

/** Table of all skip states.
 *
 *  A skip state corresponds to the 'any' element in the schemas.
 *  It means that the content of the element is specified by an extension of the
 *  schema which may or may not be known at parse time.
 *  At the moment the whole element is skipped, i.e. ignored.
 *
 */
public class SkipStateTable
{
    public SkipStateTable (final Iterable<String[]> aData)
    {
        maSkipStates = new HashSet<>();

        for (final String[] aLine : aData)
        {
            // Create new transition.
            final int nStateId = Integer.parseInt(aLine[1]);

            maSkipStates.add(nStateId);
        }
    }




    public boolean Contains (final int nStateId)
    {
        return maSkipStates.contains(nStateId);
    }




    public int GetSkipStateCount ()
    {
        return maSkipStates.size();
    }




    private final Set<Integer> maSkipStates;
}
