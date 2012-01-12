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

#include <precomp.h>
#include <tokens/tkp.hxx>

// NOT FULLY DECLARED SERVICES
#include <tools/tkpchars.hxx>
#include <tokens/tkpcontx.hxx>



TokenParser::TokenParser()
    :   pChars(0),
        bHasMore(false)
{
}

void
TokenParser::Start( CharacterSource & i_rSource )
{
    InitSource(i_rSource);
}

void
TokenParser::GetNextToken()
{
    csv_assert(pChars != 0);

    bHasMore = NOT pChars->IsFinished();

    for ( bool bDone = NOT bHasMore; NOT bDone; )
    {
        CurrentContext().ReadCharChain(*pChars);
        bDone = CurrentContext().PassNewToken();
        SetCurrentContext(CurrentContext().FollowUpContext());
    }
}

void
TokenParser::InitSource( CharacterSource & i_rSource )
{
    pChars = &i_rSource;
    bHasMore = true;
    SetStartContext();
}


