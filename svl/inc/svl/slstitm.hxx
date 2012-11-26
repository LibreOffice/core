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


#ifndef _SFXSLSTITM_HXX
#define _SFXSLSTITM_HXX

#include "svl/svldllapi.h"
#include <tools/list.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Sequence.h>

class SfxImpStringList;

class SVL_DLLPUBLIC SfxStringListItem : public SfxPoolItem
{
protected:
    SfxImpStringList*   pImp;

public:
    POOLITEM_FACTORY()
    SfxStringListItem();
    SfxStringListItem( sal_uInt16 nWhich, const List* pList=NULL );
    SfxStringListItem( sal_uInt16 nWhich, SvStream& rStream );
    SfxStringListItem( const SfxStringListItem& rItem );
    ~SfxStringListItem();

    List *                  GetList();

    const List *            GetList() const
    { return SAL_CONST_CAST(SfxStringListItem *, this)->GetList(); }

#ifndef TF_POOLABLE
    virtual int             IsPoolable() const;
#endif

    // String-Separator: \n
    virtual void            SetString( const XubString& );
    virtual XubString       GetString();

    void                    SetStringList( const com::sun::star::uno::Sequence< rtl::OUString >& rList );
    void                    GetStringList( com::sun::star::uno::Sequence< rtl::OUString >& rList ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream &, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream &, sal_uInt16 nItemVersion ) const;
    void                    Sort( sal_Bool bAscending = sal_True, List* pParallelList = 0 );

    virtual sal_Bool            PutValue  ( const com::sun::star::uno::Any& rVal,
                                         sal_uInt8 nMemberId = 0 );
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal,
                                         sal_uInt8 nMemberId = 0 ) const;
};
#endif
