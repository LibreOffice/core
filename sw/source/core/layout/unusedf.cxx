/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"
#include "errhdl.hxx"


void SwFrm::Format( const SwBorderAttrs * )
{
    ASSERT( sal_False, "Format() der Basisklasse gerufen." );
}

void SwFrm::Paint(SwRect const&, SwPrintData const*const) const
{
    ASSERT( sal_False, "Paint() der Basisklasse gerufen." );
}

sal_Bool SwCntntFrm::WouldFit( SwTwips &, sal_Bool&, sal_Bool )
{
    ASSERT( sal_False, "WouldFit des CntntFrm gerufen." );
    return sal_False;
}

bool SwFrm::FillSelection( SwSelectionList& , const SwRect& ) const
{
    ASSERT( false, "Don't call this function at the base class!" );
    return false;
}

sal_Bool SwFrm::GetCrsrOfst( SwPosition *, Point&, SwCrsrMoveState*  ) const
{
    ASSERT( sal_False, "GetCrsrOfst der Basisklasse, hi!" );
    return sal_False;
}

#ifdef DBG_UTIL

void SwRootFrm::Cut()
{
    ASSERT( sal_False, "Cut() des RootFrm gerufen." );
}

void SwRootFrm::Paste( SwFrm *, SwFrm * )
{
    ASSERT( sal_False, "Paste() des RootFrm gerufen." );
}

void SwFlyFrm::Paste( SwFrm *, SwFrm * )
{
    ASSERT( sal_False, "Paste() des FlyFrm gerufen." );
}

#endif

sal_Bool SwFrm::GetCharRect( SwRect&, const SwPosition&,
                         SwCrsrMoveState* ) const
{
    ASSERT( sal_False, "GetCharRect() der Basis gerufen." );
    return sal_False;
}


