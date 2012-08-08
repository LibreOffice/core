/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_BOLNITEM_HXX
#define _SVX_BOLNITEM_HXX

#include <svl/poolitem.hxx>
#include "editeng/editengdllapi.h"



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
    TYPEINFO();

    SvxLineItem( const sal_uInt16 nId );
    SvxLineItem( const SvxLineItem& rCpy );
    ~SvxLineItem();
    SvxLineItem &operator=( const SvxLineItem& rLine );

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual bool             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    const   editeng::SvxBorderLine*  GetLine     () const { return pLine; }
    void                    SetLine     ( const editeng::SvxBorderLine *pNew );

private:
    editeng::SvxBorderLine*  pLine;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
