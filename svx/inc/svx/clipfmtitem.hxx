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


#ifndef _SVX_CLIPFMTITEM_HXX
#define _SVX_CLIPFMTITEM_HXX

// include ---------------------------------------------------------------

#include <tools/gen.hxx>
#include <svl/poolitem.hxx>
#include "svx/svxdllapi.h"

// class SvxClipboardFmtItem ----------------------------------------------
struct SvxClipboardFmtItem_Impl;

class SVX_DLLPUBLIC SvxClipboardFmtItem : public SfxPoolItem
{
    SvxClipboardFmtItem_Impl* pImpl;
protected:

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

public:
    POOLITEM_FACTORY()
    SvxClipboardFmtItem( sal_uInt16 nId = 0 );
    SvxClipboardFmtItem( const SvxClipboardFmtItem& );
    virtual ~SvxClipboardFmtItem();

    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const;
    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    void AddClipbrdFormat( sal_uIntPtr nId, sal_uInt16 nPos = USHRT_MAX );
    void AddClipbrdFormat( sal_uIntPtr nId, const String& rName,
                            sal_uInt16 nPos = USHRT_MAX );
    sal_uInt16 Count() const;

    sal_uIntPtr GetClipbrdFormatId( sal_uInt16 nPos ) const;
    const String& GetClipbrdFormatName( sal_uInt16 nPos ) const;
};


#endif

