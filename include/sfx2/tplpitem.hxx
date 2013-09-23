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
#ifndef _SFX_TPLPITEM_HXX
#define _SFX_TPLPITEM_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <svl/flagitem.hxx>
#include <svl/style.hrc>
#include <tools/rtti.hxx>

class SFX2_DLLPUBLIC SfxTemplateItem: public SfxFlagItem
{
    OUString aStyle;
public:
    TYPEINFO();
    SfxTemplateItem();
    SfxTemplateItem( sal_uInt16 nWhich,
                     const OUString &rStyle,
                     sal_uInt16 nMask = SFXSTYLEBIT_ALL );
    SfxTemplateItem( const SfxTemplateItem& );

    const OUString&         GetStyleName() const { return aStyle; }

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual sal_uInt8           GetFlagCount() const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
