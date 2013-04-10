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



#ifndef _SVX_XLNSTIT_HXX
#define _SVX_XLNSTIT_HXX

#include <svx/xit.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/polygon/b2dpolypolygon.hxx>

class SdrModel;

//------------------------
// class XLineStartItem
//------------------------
class SVX_DLLPUBLIC XLineStartItem : public NameOrIndex
{
    basegfx::B2DPolyPolygon     maPolyPolygon;

public:
            TYPEINFO();
            XLineStartItem(sal_Int32 nIndex = -1);
            XLineStartItem(const String& rName, const basegfx::B2DPolyPolygon& rPolyPolygon);
            XLineStartItem(SfxItemPool* pPool, const basegfx::B2DPolyPolygon& rPolyPolygon);
            XLineStartItem(SfxItemPool* pPool );
            XLineStartItem(const XLineStartItem& rItem);
            XLineStartItem(SvStream& rIn);

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

    basegfx::B2DPolyPolygon GetLineStartValue(const XLineEndList* pTable = 0) const;
    void SetLineStartValue(const basegfx::B2DPolyPolygon& rPolyPolygon) { maPolyPolygon = rPolyPolygon; Detach(); }

    XLineStartItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif
