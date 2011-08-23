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
#include <tokens/stmstar2.hxx>


// NOT FULLY DECLARED SERVICES
#include <x_parse2.hxx>



StmArrayStatu2::StmArrayStatu2( intt			i_nStatusSize,
                                const INT16 * 	in_aArrayModel,
                                uintt			i_nTokenId,
                                bool			in_bIsDefault )
    :	dpBranches(new StmStatu2::Branch[i_nStatusSize]),
        nNrOfBranches(i_nStatusSize),
        nTokenId(UINT16(i_nTokenId)),
        bIsADefault(in_bIsDefault)
{
    // KORR_FUTURE: Interface of StmArrayStatu2() has to be changed.
    csv_assert(i_nTokenId < 64536);

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

StmArrayStatu2::~StmArrayStatu2()
{
    delete [] dpBranches;
}

bool
StmArrayStatu2::SetBranch( intt    		 	 in_nBranchIx,
                           StmStatu2::Branch in_nBranch )
{
    if ( csv::in_range(intt(0), in_nBranchIx, intt(nNrOfBranches) ) )
    {
        dpBranches[in_nBranchIx] = in_nBranch;
        return true;
    }
    return false;
}


StmStatu2::Branch
StmArrayStatu2::NextBy(intt in_nIndex) const
{
    if (in_nIndex < 0)
        throw X_AutodocParser(X_AutodocParser::x_InvalidChar);

    return in_nIndex < nNrOfBranches
                ?	dpBranches[in_nIndex]
                :   dpBranches[nNrOfBranches - 1];
}


bool
StmArrayStatu2::IsADefault() const
{
    return bIsADefault;
}

StmArrayStatu2 *
StmArrayStatu2::AsArray() 
{
    return this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
