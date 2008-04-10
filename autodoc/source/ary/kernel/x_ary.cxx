/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: x_ary.cxx,v $
 * $Revision: 1.6 $
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
#include <ary/x_ary.hxx>


// NOT FULLY DECLARED SERVICES



namespace ary
{


X_Ary::X_Ary( E_Event i_eEvent )
    :   eEvent(i_eEvent)
{
}

void
X_Ary::GetInfo( std::ostream & o_rOutputMedium ) const
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

