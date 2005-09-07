/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ary_disp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:10:17 $
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
#include <ary/re.hxx>
#include <ary/r_disply.hxx>



namespace ary
{

void
Display::DisplaySlot_Rid( ary::Rid i_nId )
{
    const DisplayGate * pGate = Get_ReFinder();
    if (pGate != 0)
    {
         const RepositoryEntity * pRE = pGate->Find_Re( i_nId );
        if (pRE != 0)
        {
             pRE->StoreAt( *this );
            return;
        }
    }

    do_DisplaySlot_Rid( i_nId );
}


void
Display::DisplaySlot_LocalCe( ary::Rid          i_nId,
                              const udmstri &   i_sName )
{
    const DisplayGate * pGate = Get_ReFinder();
    if (pGate != 0)
    {
         const RepositoryEntity * pRE = pGate->Find_Re( i_nId );
        if (pRE != 0)
        {
             pRE->StoreAt( *this );
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
Display::do_DisplaySlot_Lid( ary::Lid i_nId )
{
}

void
Display::do_DisplaySlot_LocalCe( ary::Rid           i_nId,
                                 const udmstri &    i_sName )
{
}

const DisplayGate *
Display::inq_Get_ReFinder() const
{
    return 0;
}



}   // namespace ary


