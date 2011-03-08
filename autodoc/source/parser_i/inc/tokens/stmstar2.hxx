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

#ifndef ADC_STMSTAR2_HXX
#define ADC_STMSTAR2_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/stmstat2.hxx>
    // COMPONENTS
    // PARAMETERS
    // SERVICES


class StmArrayStatu2 : public StmStatu2
{
  public:
    // LIFECYCLE
                        StmArrayStatu2(
                            intt            i_nStatusSize,
                            const INT16 *   in_aArrayModel,
                            uintt           i_nTokenId,
                            bool            in_bIsDefault );
                        ~StmArrayStatu2();

    // INQUIRY
    StmStatu2::Branch   NextBy(
                            intt            in_nFollowersIndex) const;
    UINT16              TokenId() const     { return nTokenId; }
    virtual bool        IsADefault() const;

    // ACCESS
    virtual StmArrayStatu2 *
                        AsArray();
    bool                SetBranch(
                            intt            in_nBranchIx,
                            StmStatu2::Branch
                                            in_nBranch );
    void                SetTokenId(
                            UINT16          in_nTokenId );
  private:
    StmStatu2::Branch * dpBranches;
    intt                nNrOfBranches;
    UINT16              nTokenId;
    bool                bIsADefault;
};


// IMPLEMENTATION

inline void
StmArrayStatu2::SetTokenId( UINT16 in_nTokenId )
    { nTokenId = in_nTokenId; }



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
