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
#ifndef _SVX_CLIPFMTITEM_HXX
#define _SVX_CLIPFMTITEM_HXX

// include ---------------------------------------------------------------

#include <tools/gen.hxx>
#include <svl/poolitem.hxx>
#include "svx/svxdllapi.h"

// class SvxClipboardFmtItem ----------------------------------------------
struct SvxClipboardFmtItem_Impl;

class SVX_DLLPUBLIC SvxClipboardFmtItem : public SfxPoolItem
{
    SvxClipboardFmtItem_Impl* pImpl;
protected:

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

public:
    TYPEINFO();
    SvxClipboardFmtItem( sal_uInt16 nId = 0 );
    SvxClipboardFmtItem( const SvxClipboardFmtItem& );
    virtual ~SvxClipboardFmtItem();

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    void AddClipbrdFormat( sal_uIntPtr nId, sal_uInt16 nPos = USHRT_MAX );
    void AddClipbrdFormat( sal_uIntPtr nId, const String& rName,
                            sal_uInt16 nPos = USHRT_MAX );
    sal_uInt16 Count() const;

    sal_uIntPtr GetClipbrdFormatId( sal_uInt16 nPos ) const;
    const String& GetClipbrdFormatName( sal_uInt16 nPos ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
