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
#ifndef _SVX_WRITINGMODEITEM_HXX
#define _SVX_WRITINGMODEITEM_HXX

#include <com/sun/star/text/WritingMode.hpp>
#include <svl/intitem.hxx>
#include "editeng/editengdllapi.h"

// class SvxWritingModeItem ----------------------------------------------

class EDITENG_DLLPUBLIC SvxWritingModeItem : public SfxUInt16Item
{
public:
    TYPEINFO();

    SvxWritingModeItem( ::com::sun::star::text::WritingMode eValue /*= com::sun::star::text::WritingMode_LR_TB*/,
                sal_uInt16 nWhich /*= SDRATTR_TEXTDIRECTION*/ );
    virtual ~SvxWritingModeItem();

    SvxWritingModeItem& operator=( const SvxWritingModeItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream & rStrm, sal_uInt16 nIVer) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper * = 0 ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

#endif // #ifndef _SVX_WRITINGMODEITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
