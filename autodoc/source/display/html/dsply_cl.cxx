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
#include "dsply_cl.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_gate.hxx>
#include "dsply_da.hxx"
#include "dsply_op.hxx"
#include "hdimpl.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"




ClassDisplayer::ClassDisplayer( OuputPage_Environment & io_rEnv )
    :   pEnv(&io_rEnv)
{
}

ClassDisplayer::~ClassDisplayer()
{
}

void
ClassDisplayer::DisplayFiles_InClass( const ary::cpp::Class & i_rData,
                                      PageDisplay &           io_rPageMaker )
{
    // Classes
    ClassDisplayer aClassDisplayer( Env() );
    DisplaySlot( aClassDisplayer, i_rData, ary::cpp::Class::SLOT_NestedClasses );

    // Enums
    DisplaySlot( io_rPageMaker, i_rData, ary::cpp::Class::SLOT_Enums );

    // Typedefs
    DisplaySlot( io_rPageMaker, i_rData, ary::cpp::Class::SLOT_Typedefs );

    // Operations
    OperationsDisplay aOperationsDisplayer( Env() );

    aOperationsDisplayer.PrepareForStdMembers();
    DisplaySlot( aOperationsDisplayer, i_rData, ary::cpp::Class::SLOT_Operations );

    aOperationsDisplayer.PrepareForStaticMembers();
    DisplaySlot( aOperationsDisplayer, i_rData, ary::cpp::Class::SLOT_StaticOperations );

    aOperationsDisplayer.Create_Files();

    // Data
    DataDisplay aDataDisplayer( Env() );

    aDataDisplayer.PrepareForStdMembers();
    DisplaySlot( aDataDisplayer, i_rData, ary::cpp::Class::SLOT_Data );

    aDataDisplayer.PrepareForStaticMembers();
    DisplaySlot( aDataDisplayer, i_rData, ary::cpp::Class::SLOT_StaticData );

    aDataDisplayer.Create_Files();
}

void
ClassDisplayer::do_Process( const ary::cpp::Class & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    PageDisplay aPageMaker( Env() );
    aPageMaker.Process(i_rData);

    Env().MoveDir_Down2( i_rData );
    DisplayFiles_InClass( i_rData, aPageMaker );
    Env().MoveDir_Up();
}

const ary::cpp::Gate *
ClassDisplayer::inq_Get_ReFinder() const
{
    return & pEnv->Gate();
}
