/*************************************************************************
 *
 *  $RCSfile: ary_disp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


