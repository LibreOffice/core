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
#include "precompiled_svx.hxx"

#include <com/sun/star/uno/Sequence.hxx>
#include <svx/svdsob.hxx>
//#include "svdglob.hxx"  // StringCache
//#include "svdstr.hrc"   // Namen aus der Resource

////////////////////////////////////////////////////////////////////////////////////////////////////
// SetOfByte

bool SetOfByte::IsEmpty() const
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        if(aData[i])
        {
            return false;
        }
    }

    return true;
}

bool SetOfByte::IsFull() const
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        if(aData[i] != 0xFF)
        {
            return false;
        }
    }

    return true;
}

sal_uInt16 SetOfByte::GetSetCount() const
{
    sal_uInt16 nRet(0);

    for(sal_uInt16 i(0); i < 32; i++)
    {
        sal_uInt8 a(aData[i]);

        if(a)
        {
            if(a & 0x80) nRet++;
            if(a & 0x40) nRet++;
            if(a & 0x20) nRet++;
            if(a & 0x10) nRet++;
            if(a & 0x08) nRet++;
            if(a & 0x04) nRet++;
            if(a & 0x02) nRet++;
            if(a & 0x01) nRet++;
        }
    }

    return nRet;
}

sal_uInt8 SetOfByte::GetSetBit(sal_uInt16 nNum) const
{
    nNum++;
    sal_uInt16 i(0), j(0);
    sal_uInt16 nRet(0);

    while(j < nNum && i < 256)
    {
        if(IsSet(sal_uInt8(i)))
        {
            j++;
        }

        i++;
    }

    if(j == nNum)
    {
        nRet = i - 1;
    }

    return sal_uInt8(nRet);
}

sal_uInt16 SetOfByte::GetClearCount() const
{
    return sal_uInt16(256 - GetSetCount());
}

sal_uInt8 SetOfByte::GetClearBit(sal_uInt16 nNum) const
{
    nNum++;
    sal_uInt16 i(0), j(0);
    sal_uInt16 nRet(0);

    while(j < nNum && i < 256)
    {
        if(!IsSet(sal_uInt8(i)))
        {
            j++;
        }

        i++;
    }

    if(j == nNum)
    {
        nRet = i - 1;
    }

    return sal_uInt8(nRet);
}

void SetOfByte::operator&=(const SetOfByte& r2ndSet)
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        aData[i] &= r2ndSet.aData[i];
    }
}

void SetOfByte::operator|=(const SetOfByte& r2ndSet)
{
    for(sal_uInt16 i(0); i < 32; i++)
    {
        aData[i] |= r2ndSet.aData[i];
    }
}

/** initialize this set with a uno sequence of sal_Int8
*/
void SetOfByte::PutValue( const com::sun::star::uno::Any & rAny )
{
    com::sun::star::uno::Sequence< sal_Int8 > aSeq;

    if( rAny >>= aSeq )
    {
        sal_Int16 nCount = (sal_Int16)aSeq.getLength();

        if( nCount > 32 )
        {
            nCount = 32;
        }

        sal_Int16 nIndex;

        for( nIndex = 0; nIndex < nCount; nIndex++ )
        {
            aData[nIndex] = static_cast< sal_uInt8 >(aSeq[nIndex]);
        }

        for( ; nIndex < 32; nIndex++ )
        {
            aData[nIndex] = 0;
        }
    }
}

/** returns a uno sequence of sal_Int8
*/
void SetOfByte::QueryValue( com::sun::star::uno::Any & rAny ) const
{
    sal_Int16 nNumBytesSet = 0;
    sal_Int16 nIndex;

    for( nIndex = 31; nIndex >= 00; nIndex-- )
    {
        if( 0 != aData[nIndex] )
        {
            nNumBytesSet = nIndex + 1;
            break;
        }
    }

    com::sun::star::uno::Sequence< sal_Int8 > aSeq( nNumBytesSet );

    for( nIndex = 0; nIndex < nNumBytesSet; nIndex++ )
    {
        aSeq[nIndex] = static_cast<sal_Int8>(aData[nIndex]);
    }

    rAny <<= aSeq;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
