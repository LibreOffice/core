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
#ifndef _SVX_CLIPFMTITEM_HXX
#define _SVX_CLIPFMTITEM_HXX

#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include "svx/svxdllapi.h"

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
    void AddClipbrdFormat( sal_uIntPtr nId, const OUString& rName,
                            sal_uInt16 nPos = USHRT_MAX );
    sal_uInt16 Count() const;

    sal_uIntPtr GetClipbrdFormatId( sal_uInt16 nPos ) const;
    const OUString GetClipbrdFormatName( sal_uInt16 nPos ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
