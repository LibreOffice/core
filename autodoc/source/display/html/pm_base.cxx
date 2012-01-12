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
#include "pm_base.hxx"


// NOT FULLY DEFINED SERVICES
#include "opageenv.hxx"
#include "pagemake.hxx"


//********************       SpecializedPageMaker        *********************//

SpecializedPageMaker::SpecializedPageMaker( PageDisplay &  io_rPage )
    :   pEnv( &io_rPage.Env() ),
        pCurOut( &io_rPage.CurOut() ),
        pPage( &io_rPage )
{
}

void
SpecializedPageMaker::Write_NavBar()
{
     // Dummy
}

void
SpecializedPageMaker::Write_TopArea()
{
     // Dummy
}

void
SpecializedPageMaker::Write_DocuArea()
{
     // Dummy
}

//void
//SpecializedPageMaker::Write_ChildList( ary::SlotAccessId   ,
//                                       const char *        ,
//                                       const char *        )
//{
//  // Dummy
//}

csi::xml::Element &
SpecializedPageMaker::CurOut()
{
    return Page().CurOut();
}

