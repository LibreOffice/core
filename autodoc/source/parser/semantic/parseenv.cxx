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
#include <semantic/parseenv.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/doc/d_oldcppdocu.hxx>
#include <x_parse.hxx>


void
ParseEnvironment::Enter( E_EnvStackAction   i_eWayOfEntering )
{
    switch (i_eWayOfEntering)
    {
        case push:
                InitData();
                if ( Parent() != 0 )
                {
                    csv_assert( Parent()->CurSubPeUse() != 0 );
                    Parent()->CurSubPeUse()->InitParse();
                }
                break;
        case pop_success:
                break;
        case pop_failure:
                break;
        default:
            csv_assert(false);
    }   // end switch
}

void
ParseEnvironment::Leave( E_EnvStackAction   i_eWayOfLeaving )
{
    switch (i_eWayOfLeaving)
    {
        case push:
                break;
        case pop_success:
                TransferData();
                if ( Parent() != 0 )
                {
                    csv_assert( Parent()->CurSubPeUse() != 0 );
                    Parent()->CurSubPeUse()->GetResults();
                }
                break;
        case pop_failure:
                break;
        default:
            csv_assert(false);
    }   // end switch
}

ParseEnvironment::ParseEnvironment( ParseEnvironment *  i_pParent )
    :   pParent(i_pParent),
        // pDocu,
        pCurSubPe(0)
{
}
