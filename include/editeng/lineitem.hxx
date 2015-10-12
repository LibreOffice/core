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
#ifndef INCLUDED_EDITENG_LINEITEM_HXX
#define INCLUDED_EDITENG_LINEITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>



// class SvxLineItem -----------------------------------------------------


/*
[Description]
This Item transports a editeng::SvxBorderLine.
*/

namespace editeng {
    class SvxBorderLine;
}

class EDITENG_DLLPUBLIC SvxLineItem : public SfxPoolItem
{
public:
    TYPEINFO_OVERRIDE();

    explicit SvxLineItem( const sal_uInt16 nId );
    SvxLineItem( const SvxLineItem& rCpy );
    virtual ~SvxLineItem();
    SvxLineItem &operator=( const SvxLineItem& rLine );

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const override;
    virtual bool             ScaleMetrics( long nMult, long nDiv ) override;
    virtual bool             HasMetrics() const override;

    const   editeng::SvxBorderLine*  GetLine     () const { return pLine; }
    void                    SetLine     ( const editeng::SvxBorderLine *pNew );

private:
    editeng::SvxBorderLine*  pLine;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
