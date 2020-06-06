/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <sal/config.h>

#include <algorithm>
#include <cstring>

#include "bento.hxx"

namespace OpenStormBento
{

size_t
CBenValue::GetValueSize()
{
    size_t Size = 0;
    CBenValueSegment * pCurr = nullptr;
    while ((pCurr = GetNextValueSegment(pCurr)) != nullptr)
        Size += pCurr->GetSize();
    return Size;
}

void
CBenValue::ReadValueData(void * pReadBuffer, size_t Offset,
  size_t Amt, size_t* pAmtRead)
{
    size_t SegOffset = 0;
    *pAmtRead = 0;
    CBenValueSegment * pCurrSeg = nullptr;
    LtcBenContainer * pContainer = GetProperty()->GetContainer();
    BenByte* pBuffer = static_cast<BenByte*>(pReadBuffer);

    /// pReadBuffer -- pointer to buffer of read result, allocated outside this function
    /// Offset          -- read buffer's start offset address, relative value in the whole value stream
    /// Amt             -- read buffer's size
    /// pAmtRead    -- return the actual read size

    /// SegOffset     -- current segment's start address offset, relative value in the whole value stream

    while ((pCurrSeg = GetNextValueSegment(pCurrSeg)) != nullptr)
    {
        if (Amt == 0)               /// whole buffer is full now, so return
            return;

        if (SegOffset <= Offset && Offset < SegOffset + pCurrSeg->GetSize()) /// begin at current segment
        {
            size_t OffsetIntoSeg = Offset - SegOffset;  /// relative value in this value segment stream

            size_t AmtThisSeg = std::min(Amt, pCurrSeg->GetSize() -
                OffsetIntoSeg);           /// size read in this segment, it's minimal value between Amt &
                                          /// remain part from OffsetIntoSeg to the end of this segment

            size_t AmtReadThisSeg; /// actual read size in this segment
            if (pCurrSeg->IsImmediate())
            {
                std::memcpy(pBuffer, pCurrSeg->GetImmediateData() +
                  OffsetIntoSeg, AmtThisSeg);
                AmtReadThisSeg = AmtThisSeg;
            }
            else
            {
                pContainer->SeekToPosition(pCurrSeg->GetPosition() +
                        OffsetIntoSeg);
                pContainer->Read(pBuffer, AmtThisSeg, &AmtReadThisSeg);
            }

            *pAmtRead += AmtReadThisSeg;

            if (AmtThisSeg != AmtReadThisSeg)
                return;

            pBuffer += AmtReadThisSeg;
            Offset += AmtReadThisSeg;
            Amt -= AmtReadThisSeg;
        }

        SegOffset += pCurrSeg->GetSize();
    }
}

}//end namespace OpenStormBento

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
