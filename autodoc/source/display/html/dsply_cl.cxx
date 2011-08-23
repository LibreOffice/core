/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
