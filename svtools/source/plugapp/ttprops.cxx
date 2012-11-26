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
#include "precompiled_svtools.hxx"
#include <svtools/ttprops.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>

TTProperties::~TTProperties()
{
}

sal_Bool TTProperties::RequestProperty( sal_uInt16 nRequest )
{
    if ( (( nRequest & TT_PR_ONCE ) == 0) || (nDonePRs & (nRequest & 0x0ff)) == 0 )
    {
        nActualPR = nRequest;
        nDonePRs |= nRequest;
        GetpApp()->Property( *this );
        return nActualPR == 0;
    }
    return sal_True;
}


sal_Bool TTProperties::GetSlots()
{
    RequestProperty( TT_PR_SLOTS );
    return HasSlots();
}

sal_uInt16 TTProperties::ExecuteFunction( sal_uInt16 nSID, SfxPoolItem** ppArgs, sal_uInt16 nMode )
{
    mnSID = nSID;
    mppArgs = ppArgs;
    mnMode = nMode;
    RequestProperty( TT_PR_DISPATCHER );
    mppArgs = NULL;
    return nActualPR;
}

sal_Bool TTProperties::Img( Bitmap *pBmp )
{
    sal_Bool bRet;
    mpBmp = pBmp;
    bRet = RequestProperty( TT_PR_IMG );
    mpBmp = NULL;
    return bRet;
}

SvtResId TTProperties::GetSvtResId( sal_uInt16 nId )
{
    return SvtResId( nId );
}

