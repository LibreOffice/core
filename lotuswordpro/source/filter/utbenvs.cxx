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
#include "first.hxx"
#include <assert.h>
namespace OpenStormBento
{
/*
 * useless in SODC
void	LtcUtBenValueStream::GetAmountLeft(ULONG * pAmtLeft)
{
    if (cCurrentPosition >= m_ulValueLength)
    {
        *pAmtLeft = 0;
    }
    else
    {
        *pAmtLeft = m_ulValueLength - cCurrentPosition;
    }
}
*/
/**
*	Value stream read function
*	@date	07/05/2004
*	@param	data pointer of bytes read
*	@param	number of bytes to be read
*	@return	number of bytes read
*/
ULONG	LtcUtBenValueStream::GetData( void* pData, ULONG nSize )
{
    //unsigned long AmtLeft;
    ULONG AmtRead;
    //GetAmountLeft(&AmtLeft);

    //unsigned long AmtShouldRead = UtMin(nSize, AmtLeft);
    /*BenError Err = */cpValue->ReadValueData(pData, cCurrentPosition, nSize,
      &AmtRead);
    cCurrentPosition += AmtRead;

    return AmtRead;
}
 /**
*	Value stream write function, not suppoted now
*	@date	07/05/2004
*	@param	pointer of saved buffer
*	@param	size of buffer to be written
*	@return	number of bytes written into value stream
*/
ULONG	LtcUtBenValueStream::PutData( const void* /*pData*/, ULONG nSize )
{
    /* Because we only support IMPORT filter, PutData implementation is ignored
        It won't bring negative influence to read-only stream object */
    assert(false);
    return nSize;
}
/**
*	Seek function of value stream
*	@date	07/05/2004
*	@param	position in value stream
*	@return	current position in value stream
*/
 ULONG	LtcUtBenValueStream::SeekPos( ULONG nPos )
{
    if (nPos <= m_ulValueLength)
        cCurrentPosition = nPos;
    else
        cCurrentPosition = m_ulValueLength;
    return cCurrentPosition;
}
/**
*	Set buffer size function
*	@date	07/05/2004
*	@param 	size of buffer
*	@return
*/
 void	LtcUtBenValueStream::SetSize( ULONG /*nSize*/ )
{
    //pLtcBenContainer pContainer = cpValue->GetContainer();
    //pContainer->GetStream()->SetStreamSize(nSize);

    return;
}
/**
*	Flush data funciton, not supported now
*	@date	07/05/2004
*	@param
*	@return
*/
void	LtcUtBenValueStream::FlushData()
{
    /* Because we only support IMPORT filter, FlushData implementation is ignored
        It won't bring negative influence to read-only stream object
    pLtcBenContainer pContainer = cpValue->GetContainer();
    pContainer->GetStream()->Flush();*/
    assert(false);
    return;
}
/**
*	Construction
*	@date	07/05/2004
*	@param	pointer to CBenValue object
*	@return
*/
LtcUtBenValueStream::LtcUtBenValueStream(pCBenValue pValue)
{
    // Calculate the length of the whole value stream
    cCurrentPosition = 0;
    m_ulValueLength = pValue->GetValueSize();
    cpValue = pValue;
}

LtcUtBenValueStream::~LtcUtBenValueStream()
{
}

} // end namespace OpenStormBento

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
