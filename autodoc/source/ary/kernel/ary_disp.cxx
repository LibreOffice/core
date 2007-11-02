/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ary_disp.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:11:11 $
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
