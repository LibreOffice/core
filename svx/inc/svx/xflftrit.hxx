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

    sal_Bool                    bEnabled;

public:
                            TYPEINFO();

                            XFillFloatTransparenceItem();
                            XFillFloatTransparenceItem(const String& rName, const XGradient& rGradient, sal_Bool bEnable = sal_True );
                            XFillFloatTransparenceItem(SfxItemPool* pPool, const XGradient& rTheGradient, sal_Bool bEnable = sal_True );
                            XFillFloatTransparenceItem( const XFillFloatTransparenceItem& rItem );

    virtual int             operator==( const SfxPoolItem& rItem ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres, SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric, String &rText, const IntlWrapper * pIntlWrapper = 0 ) const;

    sal_Bool                    IsEnabled() const { return bEnabled; }
    void                    SetEnabled( sal_Bool bEnable ) { bEnabled = bEnable; }

    static sal_Bool CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    XFillFloatTransparenceItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
