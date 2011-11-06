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


#ifndef _UIITEMS_HXX
#define _UIITEMS_HXX

#include <svl/intitem.hxx>
#include "swdllapi.h"
#include "cmdid.h"
#include "pagedesc.hxx"

class SwNumRule;
class IntlWrapper;

/*--------------------------------------------------------------------
    Beschreibung: Container fuer FootNote
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwPageFtnInfoItem : public SfxPoolItem
{
    SwPageFtnInfo aFtnInfo;

public:

    SwPageFtnInfoItem(const sal_uInt16 nId, SwPageFtnInfo& rInfo);
    SwPageFtnInfoItem(const SwPageFtnInfoItem& rItem );
    ~SwPageFtnInfoItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    SwPageFtnInfo& GetPageFtnInfo()             { return aFtnInfo; }
    const SwPageFtnInfo& GetPageFtnInfo() const { return aFtnInfo; }
    void SetPageFtnInfo(SwPageFtnInfo& rInf)    { aFtnInfo = rInf; }
};

/*******************************************************************/

class SW_DLLPUBLIC SwPtrItem : public SfxPoolItem
{
    void* pMisc;

public:
    SwPtrItem( const sal_uInt16 nId = FN_PARAM_GRF_DIALOG, void* pPtr = 0);
    SwPtrItem( const SwPtrItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void    SetValue(void * pPtr)   { pMisc= pPtr; }
    void*   GetValue() const        { return pMisc; }
};

/*******************************************************************/

class SW_DLLPUBLIC SwUINumRuleItem : public SfxPoolItem
{
    SwNumRule* pRule;

public:
    SwUINumRuleItem( const SwNumRule& rRule, const sal_uInt16 = FN_PARAM_ACT_NUMBER);
    SwUINumRuleItem( const SwUINumRuleItem& rItem );
    virtual ~SwUINumRuleItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    const SwNumRule* GetNumRule() const         { return pRule; }
          SwNumRule* GetNumRule()               { return pRule; }
};

/* -----------------17.06.98 17:40-------------------
 *
 * --------------------------------------------------*/
class SwBackgroundDestinationItem : public SfxUInt16Item
{
public:
    SwBackgroundDestinationItem(sal_uInt16  nWhich, sal_uInt16 nValue);

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
};

#endif // _UIITEMS_HXX
