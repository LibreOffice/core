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

class Transition
{
    Transition (
        final int nStartStateId,
        final int nEndStateId,
        final int nElementId,
        final int nActionStateId)
    {
        mnStartStateId = nStartStateId;
        mnEndStateId = nEndStateId;
        mnElementId = nElementId;
        mnActionStateId = nActionStateId;
    }




    public int GetStartStateId ()
    {
        return mnStartStateId;
    }




    public int GetEndStateId ()
    {
        return mnEndStateId;
    }




    public int GetElementId ()
    {
        return mnElementId;
    }




    public int GetActionId ()
    {
        return mnActionStateId;
    }




    private final int mnStartStateId;
    private final int mnEndStateId;
    private final int mnElementId;
    private final int mnActionStateId;
}
