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

package org.apache.openoffice.ooxml.parser.action;

import java.util.Iterator;

/** Iterate over two sources of actions, both given as an Iterable<IAction>
 *  object that can be null.
*/
public class ActionIterator implements Iterable<IAction>
{
    public ActionIterator (
        final Iterable<IAction> aOneStateActions,
        final Iterable<IAction> aAllStateActions)
    {
        maOneStateActions = aOneStateActions;
        maAllStateActions = aAllStateActions;
    }




    @Override public Iterator<IAction> iterator()
    {
        return new Iterator<IAction>()
        {
            Iterator<IAction> maIterator = null;
            int mnPhase = 0;

            @Override
            public boolean hasNext()
            {
                while(true)
                {
                    if (mnPhase == 2)
                        return false;
                    else if (mnPhase == 0)
                    {
                        if (maIterator == null)
                            if (maOneStateActions == null)
                            {
                                mnPhase = 1;
                                continue;
                            }
                            else
                                maIterator = maOneStateActions.iterator();
                        if (maIterator.hasNext())
                            return true;
                        else
                        {
                            maIterator = null;
                            mnPhase = 1;
                        }
                    }
                    else if (mnPhase == 1)
                    {
                        if (maIterator == null)
                            if (maAllStateActions == null)
                            {
                                mnPhase = 2;
                                return false;
                            }
                            else
                                maIterator = maAllStateActions.iterator();
                        if (maIterator.hasNext())
                            return true;
                        else
                        {
                            mnPhase = 2;
                        }
                    }
                }
            }




            @Override
            public IAction next()
            {
                return maIterator.next();
            }




            @Override
            public void remove()
            {
            }
        };
    }




    private final Iterable<IAction> maOneStateActions;
    private final Iterable<IAction> maAllStateActions;
}
