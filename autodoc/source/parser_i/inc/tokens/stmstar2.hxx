/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
