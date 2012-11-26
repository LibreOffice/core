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



#ifndef _SVX_XFLFTRIT_HXX
#define _SVX_XFLFTRIT_HXX

#include <svx/xflgrit.hxx>
#include "svx/svxdllapi.h"

//-----------------------------------
// class XFillFloatTransparenceItem -
//-----------------------------------

class SVX_DLLPUBLIC XFillFloatTransparenceItem : public XFillGradientItem
{
private:

    long                    nDummy1;
    long                    nDummy2;
    sal_Bool                    bEnabled;

public:
                            XFillFloatTransparenceItem();
                            XFillFloatTransparenceItem( sal_Int32 nIndex, const XGradient& rGradient, sal_Bool bEnable = sal_True );
                            XFillFloatTransparenceItem(const String& rName, const XGradient& rGradient, sal_Bool bEnable = sal_True );
                            XFillFloatTransparenceItem(SfxItemPool* pPool, const XGradient& rTheGradient, sal_Bool bEnable = sal_True );
                            XFillFloatTransparenceItem(SfxItemPool* pPool );
                            XFillFloatTransparenceItem( const XFillFloatTransparenceItem& rItem );

    virtual int             operator==( const SfxPoolItem& rItem ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres, SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric, String &rText, const IntlWrapper * pIntlWrapper = 0 ) const;

    sal_Bool                    IsEnabled() const { return bEnabled; }
    void                    SetEnabled( sal_Bool bEnable ) { bEnabled = bEnable; }

    static sal_Bool CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    XFillFloatTransparenceItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif
