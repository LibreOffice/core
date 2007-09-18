/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: x_ary.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 13:45:05 $
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
#include <ary/x_ary.hxx>


// NOT FULLY DECLARED SERVICES



namespace ary
{


X_Ary::X_Ary( E_Event i_eEvent )
    :   eEvent(i_eEvent)
{
}

void
X_Ary::GetInfo( ostream &      o_rOutputMedium ) const
{
    switch (eEvent)
    {
        case x_MultipleRepository:
            o_rOutputMedium << "Tried to create a repository instance, though there exists one already.";
            break;
        case x_MissingRepository:
            o_rOutputMedium << "Tried to access the repository, though there exists none.";
            break;
        case x_EntityNotFound:
            o_rOutputMedium << "Code entity not found in repository.";
            break;
        case x_ConflictingNames:
            o_rOutputMedium << "Name of code entity occurs double in different versions.";
            break;
        case x_UnexpectedTypeOfObject:
            o_rOutputMedium << "Name- or id-mismatch: Code entity had other type than expected.";
            break;
        case x_Any:
        default:
            o_rOutputMedium << "Unspecified exception in repository.";
    }   // end switch
    o_rOutputMedium << Endl();
}


}   // namespace ary

