/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVL_SLSTITM_HXX
#define INCLUDED_SVL_SLSTITM_HXX

#include <vector>

#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Sequence.h>

class SfxImpStringList;

class SVL_DLLPUBLIC SfxStringListItem : public SfxPoolItem
{
protected:
    SfxImpStringList*   pImp;

public:
    static SfxPoolItem* CreateDefault();

    SfxStringListItem();
    SfxStringListItem( sal_uInt16 nWhich, const std::vector<OUString> *pList=NULL );
    SfxStringListItem( sal_uInt16 nWhich, SvStream& rStream );
    SfxStringListItem( const SfxStringListItem& rItem );
    virtual ~SfxStringListItem();

    std::vector<OUString>&       GetList();

    const std::vector<OUString>& GetList() const;

    // String-Separator: \n
    void                    SetString( const OUString& );
    OUString                GetString();

    void                    SetStringList( const css::uno::Sequence< OUString >& rList );
    void                    GetStringList( css::uno::Sequence< OUString >& rList ) const;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper * = 0 ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*    Create( SvStream &, sal_uInt16 nVersion ) const override;
    virtual SvStream&       Store( SvStream &, sal_uInt16 nItemVersion ) const override;

    virtual bool            PutValue  ( const css::uno::Any& rVal,
                                         sal_uInt8 nMemberId ) override;
    virtual bool            QueryValue( css::uno::Any& rVal,
                                         sal_uInt8 nMemberId = 0 ) const override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
