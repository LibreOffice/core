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
#ifndef _SDTFCHIM_HXX
#define _SDTFCHIM_HXX

#include <svx/svddef.hxx>

class SVX_DLLPUBLIC SdrTextFixedCellHeightItem : public SfxBoolItem
{
public:

    TYPEINFO();
    SdrTextFixedCellHeightItem( BOOL bUseFixedCellHeight = FALSE );
    SVX_DLLPRIVATE SdrTextFixedCellHeightItem( SvStream & rStream, sal_uInt16 nVersion );

    SVX_DLLPRIVATE virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation,
                                    SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                        String &rText, const IntlWrapper * = 0) const;

    SVX_DLLPRIVATE virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 nItem ) const;
    SVX_DLLPRIVATE virtual SvStream&            Store( SvStream&, sal_uInt16 nVersion ) const;
    SVX_DLLPRIVATE virtual SfxPoolItem*     Clone( SfxItemPool* pPool = NULL ) const;
    SVX_DLLPRIVATE virtual  sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

    SVX_DLLPRIVATE virtual  bool QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    SVX_DLLPRIVATE virtual  bool PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
