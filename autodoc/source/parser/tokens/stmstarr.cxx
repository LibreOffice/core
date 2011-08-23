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
#include <tokens/stmstarr.hxx>


// NOT FULLY DECLARED SERVICES
#include <x_parse.hxx>



StmArrayStatus::StmArrayStatus( intt			i_nStatusSize,
                                const INT16 * 	in_aArrayModel,
                                F_CRTOK			i_fTokenCreateFunction,
                                bool			in_bIsDefault )
    :	dpBranches(new StmStatus::Branch[i_nStatusSize]),
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

bool
StmArrayStatus::SetBranch( intt    		 	 in_nBranchIx,
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
                ?	dpBranches[in_nIndex]
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
