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



#ifndef _SVX_XLNDSIT_HXX
#define _SVX_XLNDSIT_HXX

#include <svx/xit.hxx>

#ifndef _SVX_XLINIIT_HXX //autogen
#include <svx/xdash.hxx>
#endif
#include "svx/svxdllapi.h"

class SdrModel;

//--------------------
// class XLineDashItem
//--------------------
class SVX_DLLPUBLIC XLineDashItem : public NameOrIndex
{
    XDash   aDash;

public:
                            TYPEINFO();
                            XLineDashItem() : NameOrIndex(XATTR_LINEDASH, -1) {}
                            XLineDashItem(sal_Int32 nIndex, const XDash& rTheDash);
                            XLineDashItem(const String& rName, const XDash& rTheDash);
                            XLineDashItem(SfxItemPool* pPool, const XDash& rTheDash);
                            XLineDashItem(SfxItemPool* pPool );
                            XLineDashItem(const XLineDashItem& rItem);
                            XLineDashItem(SvStream& rIn);

    virtual int             operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual SvStream&       Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
    virtual FASTBOOL        HasMetrics() const;
    virtual FASTBOOL        ScaleMetrics(long nMul, long nDiv);

    const XDash&            GetDashValue(const XDashList* pTable = 0) const; // GetValue -> GetDashValue
    void                    SetDashValue(const XDash& rNew)   { aDash = rNew; Detach(); } // SetValue -> SetDashValue

    static sal_Bool CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    XLineDashItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif

