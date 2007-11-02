/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmstarr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:04:24 $
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
    csv_assert(in_nStartBranchIx >= 0);
    csv_assert(in_aBranchValues != 0);
    csv_assert( in_nNrofValues > 0
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
        throw X_Parser(X_Parser::x_InvalidChar, "", String::Null_(), 0);

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

