/*************************************************************************
 *
 *  $RCSfile: stmstarr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:33 $
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
#include <tokens/stmstarr.hxx>


// NOT FULLY DECLARED SERVICES
#include <x_parse.hxx>



StmArrayStatus::StmArrayStatus( intt            i_nStatusSize,
                                const INT16 *   in_aArrayModel,
                                F_CRTOK         i_fTokenCreateFunction,
                                bool            in_bIsDefault )
    :   dpBranches(new StmStatus::Branch[i_nStatusSize]),
        nNrOfBranches(i_nStatusSize),
        fTokenCreateFunction(i_fTokenCreateFunction),
        bIsADefault(in_bIsDefault)
{
    if (in_aArrayModel != 0)
    {
        intt count = 0;
        for (const INT16 * get = in_aArrayModel; count < nNrOfBranches; count++, get++)
            dpBranches[count] = *get;
    }
    else //
    {
        memset(dpBranches, 0, nNrOfBranches);
    }  // endif
}

StmArrayStatus::~StmArrayStatus()
{
    delete [] dpBranches;
}

void
StmArrayStatus::SetBranches( intt                in_nStartBranchIx,
                             StmStatus::Branch * in_aBranchValues,
                             intt                in_nNrofValues )
{
    precond(in_nStartBranchIx >= 0);
    precond(in_aBranchValues != 0);
    precond( in_nNrofValues > 0
             AND in_nStartBranchIx + in_nNrofValues <= nNrOfBranches );

    memcpy(&dpBranches[in_nStartBranchIx],in_aBranchValues,in_nNrofValues);
}



bool
StmArrayStatus::SetBranch( intt              in_nBranchIx,
                           StmStatus::Branch in_nBranch )
{
    if ( csv::in_range(intt(0), in_nBranchIx, intt(nNrOfBranches) ) )
    {
        dpBranches[in_nBranchIx] = in_nBranch;
        return true;
    }
    return false;
}


StmStatus::Branch
StmArrayStatus::NextBy(intt in_nIndex) const
{
    if (in_nIndex < 0)
        throw X_Parser(X_Parser::x_InvalidChar, "", udmstri::Null_(), 0);

    return in_nIndex < nNrOfBranches
                ?   dpBranches[in_nIndex]
                :   dpBranches[nNrOfBranches - 1];
}


bool
StmArrayStatus::IsADefault() const
{
    return bIsADefault;
}

StmArrayStatus *
StmArrayStatus::AsArray()
{
    return this;
}

