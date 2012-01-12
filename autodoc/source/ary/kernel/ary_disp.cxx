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
#include <ary/ary_disp.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_ce.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>



namespace ary
{

void
Display::DisplaySlot_Rid( ary::Rid i_nId )
{
    const cpp::Gate *
        pGate = Get_ReFinder();
    if (pGate != 0)
    {
         const  ary::cpp::CodeEntity *
             pRE = pGate->Ces().Search_Ce( cpp::Ce_id(i_nId) );
        if (pRE != 0)
        {
             pRE->Accept( *this );
            return;
        }
    }

    do_DisplaySlot_Rid( i_nId );
}


void
Display::DisplaySlot_LocalCe( ary::cpp::Ce_id   i_nId,
                              const String  &   i_sName )
{
    const cpp::Gate *
        pGate = Get_ReFinder();
    if (pGate != 0)
    {
         const ary::cpp::CodeEntity *
             pRE = pGate->Ces().Search_Ce(i_nId);
        if (pRE != 0)
        {
             pRE->Accept( *this );
            return;
        }
    }

    do_DisplaySlot_LocalCe( i_nId, i_sName );
}



// Dummy implementations for class Display

void
Display::do_StartSlot()
{
}

void
Display::do_FinishSlot()
{
}

void
Display::do_DisplaySlot_Rid( ary::Rid )
{
}

void
Display::do_DisplaySlot_LocalCe( ary::cpp::Ce_id    ,
                                 const String  &    )
{
}


}   // namespace ary
