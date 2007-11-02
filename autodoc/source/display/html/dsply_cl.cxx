/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsply_cl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:24:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
