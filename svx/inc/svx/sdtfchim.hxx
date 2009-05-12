/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdtfchim.hxx,v $
 * $Revision: 1.3 $
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

class SdrTextFixedCellHeightItem : public SfxBoolItem
{
public:

    TYPEINFO();
    SdrTextFixedCellHeightItem( BOOL bUseFixedCellHeight = FALSE );
    SdrTextFixedCellHeightItem( SvStream & rStream, sal_uInt16 nVersion );

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation,
                                    SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                        String &rText, const IntlWrapper * = 0) const;

    virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 nItem ) const;
    virtual SvStream&           Store( SvStream&, sal_uInt16 nVersion ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool* pPool = NULL ) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

#ifdef SDR_ISPOOLABLE
    virtual int IsPoolable() const;
#endif
};

#endif
