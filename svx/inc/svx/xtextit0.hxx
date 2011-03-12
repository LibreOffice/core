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

#ifndef _SVX_TEXTIT0_HXX
#define _SVX_TEXTIT0_HXX

#include <svx/xenum.hxx>
#include <svl/eitem.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* FormText-StyleItem
|*
\************************************************************************/

class SVX_DLLPUBLIC XFormTextStyleItem : public SfxEnumItem
{
public:
                            TYPEINFO();
                            XFormTextStyleItem(XFormTextStyle = XFT_NONE);
                            XFormTextStyleItem(SvStream& rIn);
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual sal_uInt16          GetValueCount() const;
    XFormTextStyle          GetValue() const { return (XFormTextStyle) SfxEnumItem::GetValue(); }
    // #FontWork#
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

/*************************************************************************
|*
|* FormText-HideFormItem
|*
\************************************************************************/

class SVX_DLLPUBLIC XFormTextHideFormItem : public SfxBoolItem
{
public:
                            TYPEINFO();
                            XFormTextHideFormItem(sal_Bool bHide = sal_False);
                            XFormTextHideFormItem(SvStream& rIn);
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
