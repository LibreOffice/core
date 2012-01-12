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
#include <tokens/stmstfi2.hxx>


// NOT FULLY DECLARED SERVICES
#include <tokens/tkpcont2.hxx>


StmBoundsStatu2::StmBoundsStatu2( StateMachineContext &
                                                    o_rOwner,
                                  TkpContext &      i_rFollowUpContext,
                                  uintt             i_nStatusFunctionNr,
                                  bool              i_bIsDefault )
    :   pOwner(&o_rOwner),
        pFollowUpContext(&i_rFollowUpContext),
        nStatusFunctionNr(i_nStatusFunctionNr),
        bIsDefault(i_bIsDefault)
{
}

bool
StmBoundsStatu2::IsADefault() const
{
    return bIsDefault;
}

StmBoundsStatu2 *
StmBoundsStatu2::AsBounds()
{
    return this;
}



