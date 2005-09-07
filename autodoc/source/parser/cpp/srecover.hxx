/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srecover.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:33:26 $
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

