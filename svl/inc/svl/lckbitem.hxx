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


#ifndef _LCKBITEM_HXX
#define _LCKBITEM_HXX

#include "svl/svldllapi.h"
#include <tools/solar.h>
#include <tools/stream.hxx>
#include <svl/poolitem.hxx>

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxLockBytesItem : public SfxPoolItem
{
    SvLockBytesRef          _xVal;

public:
    POOLITEM_FACTORY()
                            SfxLockBytesItem();
                            SfxLockBytesItem( sal_uInt16 nWhich,
                                              SvLockBytes *pLockBytes );
                            SfxLockBytesItem( sal_uInt16 nWhich, SvStream & );
                            SfxLockBytesItem( const SfxLockBytesItem& );
                            ~SfxLockBytesItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nItemVersion) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;

    SvLockBytes*            GetValue() const { return _xVal; }

    virtual sal_Bool            PutValue  ( const com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 );
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const;
};

#endif

