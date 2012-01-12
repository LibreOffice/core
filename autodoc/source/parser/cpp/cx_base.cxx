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
#include "cx_base.hxx"

// NOT FULLY DECLARED SERVICES
#include <tokens/token.hxx>
#include "c_dealer.hxx"



namespace cpp {



Cx_Base::Cx_Base( TkpContext * io_pFollowUpContext )
    :   pDealer(0),
        pFollowUpContext(io_pFollowUpContext)
        // pNewToken
{
}

bool
Cx_Base::PassNewToken()
{
    if (pNewToken)
    {
        pNewToken.Release()->DealOut(Dealer());
        return true;
    }
    return false;
}

TkpContext &
Cx_Base::FollowUpContext()
{
    csv_assert(pFollowUpContext != 0);
    return *pFollowUpContext;
}

void
Cx_Base::AssignDealer( Distributor &  o_rDealer )
{
    pDealer = &o_rDealer;
}


}   // namespace cpp




