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



#ifndef _SDGGAITM_HXX
#define _SDGGAITM_HXX

#include <svl/intitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//----------------------
// SdrGrafGamma100Item -
//----------------------

class SVX_DLLPUBLIC SdrGrafGamma100Item : public SfxUInt32Item
{
public:

                            SdrGrafGamma100Item( sal_uIntPtr nGamma100 = 100 ) : SfxUInt32Item( SDRATTR_GRAFGAMMA, nGamma100 ) {}
                            SdrGrafGamma100Item( SvStream& rIn ) : SfxUInt32Item( SDRATTR_GRAFGAMMA, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

#endif // _SDGGAITM_HXX
