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


#ifndef _SVX_BOLNITEM_HXX
#define _SVX_BOLNITEM_HXX

// include ---------------------------------------------------------------



#include <svl/poolitem.hxx>
#include "editeng/editengdllapi.h"



// class SvxLineItem -----------------------------------------------------


/*
[Beschreibung]
Dieses Item transportiert eine SvxBorderLine.
*/

class SvxBorderLine;

class EDITENG_DLLPUBLIC SvxLineItem : public SfxPoolItem
{
public:
    POOLITEM_FACTORY()
    SvxLineItem( const sal_uInt16 nId = 0 );
    SvxLineItem( const SvxLineItem& rCpy );
    ~SvxLineItem();
    SvxLineItem &operator=( const SvxLineItem& rLine );

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual void             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    const   SvxBorderLine*  GetLine     () const { return pLine; }
    void                    SetLine     ( const SvxBorderLine *pNew );

private:
    SvxBorderLine*  pLine;
};




#endif
