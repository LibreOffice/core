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
 *  License at http:
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
#include "first.hxx"
#include "assert.h"
namespace OpenStormBento
{

unsigned long
CBenValue::GetValueSize()
{
    unsigned long Size = 0;
    pCBenValueSegment pCurr = NULL;
    while ((pCurr = GetNextValueSegment(pCurr)) != NULL)
        Size += pCurr->GetSize();
    return Size;
}

BenError
CBenValue::ReadValueData(BenDataPtr pReadBuffer, unsigned long Offset,
  unsigned long Amt, unsigned long * pAmtRead)
{
    BenError Err;
    unsigned long SegOffset = 0;
    *pAmtRead = 0;
    pCBenValueSegment pCurrSeg = NULL;
    pLtcBenContainer pContainer = GetContainer();
    BenByteDataPtr pBuffer = (BenByteDataPtr) pReadBuffer;

    
    
    
    

    

    while ((pCurrSeg = GetNextValueSegment(pCurrSeg)) != NULL)
    {
        if (Amt == 0)               
            return BenErr_OK;

        if (SegOffset <= Offset && Offset < SegOffset + pCurrSeg->GetSize()) 
        {
            unsigned long OffsetIntoSeg = Offset - SegOffset;  

            unsigned long AmtThisSeg = UtMin(Amt, pCurrSeg->GetSize() -
                OffsetIntoSeg);           
                                          

            unsigned long AmtReadThisSeg; 
            if (pCurrSeg->IsImmediate())
            {
                UtHugeMemcpy(pBuffer, pCurrSeg->GetImmediateData() +
                  OffsetIntoSeg, AmtThisSeg);
                AmtReadThisSeg = AmtThisSeg;
            }
            else
            {
                if ((Err = pContainer->SeekToPosition(pCurrSeg->GetPosition() +
                        OffsetIntoSeg)) != BenErr_OK)
                    return Err;

                if ((Err = pContainer->Read(pBuffer, AmtThisSeg,
                                            &AmtReadThisSeg)) != BenErr_OK)
                    return Err;
            }

            *pAmtRead += AmtReadThisSeg;

            if (AmtThisSeg != AmtReadThisSeg)
                return BenErr_UnexpectedEndOfFile;

            pBuffer += AmtReadThisSeg;
            Offset += AmtReadThisSeg;
            Amt -= AmtReadThisSeg;
        }

        SegOffset += pCurrSeg->GetSize();
    }
    return BenErr_OK;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
