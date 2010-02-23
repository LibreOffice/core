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

#ifndef ADC_CPP_SRECOVER_HXX
#define ADC_CPP_SRECOVER_HXX



// USED SERVICES
    // BASE CLASSES
#include "cxt2ary.hxx"
#include <ary/info/docstore.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp
{

/** Implementation struct for cpp::ContextForAry.
*/
struct ContextForAry::S_RecoveryGuard
{
  public:
                        S_RecoveryGuard();
                        ~S_RecoveryGuard();

    void                Reset()                 { bIsWithinRecovery = false; nBlockBracketsCounter = 0; }

    void                StartWaitingFor_Recovery();

    void                Hdl_SwBracketOpen();
    void                Hdl_SwBracketClose();
    void                Hdl_Semicolon();

    bool                IsWithinRecovery() const;

  private:
    // DATA
    bool                bIsWithinRecovery;
    intt                nBlockBracketsCounter;
};



// IMPLEMENTATION

/*  The implementation is in header, though not all inline, because this file
    is included in cxt2ary.cxx only!
*/

ContextForAry::
S_RecoveryGuard::S_RecoveryGuard()
    :   bIsWithinRecovery(false),
        nBlockBracketsCounter(0)
{
}

ContextForAry::
S_RecoveryGuard::~S_RecoveryGuard()
{
}

inline void
ContextForAry::
S_RecoveryGuard::StartWaitingFor_Recovery()
{
    bIsWithinRecovery = true;
    nBlockBracketsCounter = 0;
}

void
ContextForAry::
S_RecoveryGuard::Hdl_SwBracketOpen()
{
    if ( bIsWithinRecovery )
        ++nBlockBracketsCounter;
}

void
ContextForAry::
S_RecoveryGuard::Hdl_SwBracketClose()
{
    if ( bIsWithinRecovery )
        --nBlockBracketsCounter;
}

inline void
ContextForAry::
S_RecoveryGuard::Hdl_Semicolon()
{
    if (bIsWithinRecovery AND nBlockBracketsCounter == 0)
       bIsWithinRecovery = false;
}

inline bool
ContextForAry::
S_RecoveryGuard::IsWithinRecovery() const
{
    return bIsWithinRecovery;
}



}   // namespace cpp


#endif

