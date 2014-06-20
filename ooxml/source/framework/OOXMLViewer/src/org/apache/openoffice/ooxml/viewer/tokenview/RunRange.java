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

package org.apache.openoffice.ooxml.viewer.tokenview;

import java.util.Vector;

public class RunRange<TokenType>
{
    RunRange (final Vector<Run<TokenType>> aRuns)
    {
        maRuns = aRuns;
    }




    public int FindTokens (
        @SuppressWarnings("unchecked")
        final TokenType ... eTypeList)
    {
        return FindTokens(0, eTypeList);
    }




    public int FindTokens (
        final int nFirstIndex,
        @SuppressWarnings("unchecked")
        final TokenType ... eTypeList)
    {
        for (int nIndex=nFirstIndex; nIndex<maRuns.size()-eTypeList.length; ++nIndex)
        {
            boolean bMatches = true;
            for (int nInnerIndex=0; nInnerIndex<eTypeList.length && bMatches; ++nInnerIndex)
                if (maRuns.get(nIndex+nInnerIndex).GetTokenType()
                    != eTypeList[nInnerIndex])
                {
                    bMatches = false;
                }
            if (bMatches)
                return nIndex;
        }

        return -1;
    }




    public boolean IsEmpty ()
    {
        return maRuns.isEmpty();
    }




    public Run<TokenType> Get (final int nIndex)
    {
        return maRuns.get(nIndex);
    }




    private final Vector<Run<TokenType>> maRuns;
}
