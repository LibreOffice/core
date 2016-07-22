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
#ifndef INCLUDED_SVX_CLIPFMTITEM_HXX
#define INCLUDED_SVX_CLIPFMTITEM_HXX

#include <rtl/ustring.hxx>
#include <sot/formats.hxx>
#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>
#include <memory>

struct SvxClipboardFormatItem_Impl;

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxClipboardFormatItem : public SfxPoolItem
{
protected:
    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;

public:
    static SfxPoolItem* CreateDefault();
    SvxClipboardFormatItem( sal_uInt16 nId = 0 );
    SvxClipboardFormatItem( const SvxClipboardFormatItem& );
    virtual ~SvxClipboardFormatItem();

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    void AddClipbrdFormat( SotClipboardFormatId nId );
    void AddClipbrdFormat( SotClipboardFormatId nId, const OUString& rName,
                            sal_uInt16 nPos = USHRT_MAX );
    sal_uInt16 Count() const;

    SotClipboardFormatId GetClipbrdFormatId( sal_uInt16 nPos ) const;
    const OUString GetClipbrdFormatName( sal_uInt16 nPos ) const;

private:
    std::unique_ptr<SvxClipboardFormatItem_Impl> pImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
