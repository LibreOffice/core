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

#ifndef __EDITENG_JUSTIFYITEM_HXX__
#define __EDITENG_JUSTIFYITEM_HXX__

#include "editeng/editengdllapi.h"
#include "editeng/svxenum.hxx"
#include <svl/eitem.hxx>
#include <sal/types.h>

class EDITENG_DLLPUBLIC SvxHorJustifyItem: public SfxEnumItem
{
public:
    TYPEINFO();

    explicit SvxHorJustifyItem( const sal_uInt16 nId );

    SvxHorJustifyItem(
        const SvxCellHorJustify eJustify /*= SVX_HOR_JUSTIFY_STANDARD*/,
        const sal_uInt16 nId );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual sal_uInt16          GetValueCount() const;
    virtual OUString        GetValueText( sal_uInt16 nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

    inline  SvxHorJustifyItem& operator=(const SvxHorJustifyItem& rHorJustify)
            {
                SetValue( rHorJustify.GetValue() );
                return *this;
            }
};

//------------------------------------------------------------------------

class EDITENG_DLLPUBLIC SvxVerJustifyItem: public SfxEnumItem
{
public:
    TYPEINFO();

    explicit SvxVerJustifyItem( const sal_uInt16 nId  );

    SvxVerJustifyItem(
        const SvxCellVerJustify eJustify /*= SVX_VER_JUSTIFY_STANDARD*/,
        const sal_uInt16 nId  );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual sal_uInt16          GetValueCount() const;
    virtual OUString        GetValueText( sal_uInt16 nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

    inline  SvxVerJustifyItem& operator=(const SvxVerJustifyItem& rVerJustify)
            {
                SetValue( rVerJustify.GetValue() );
                return *this;
            }
};

//------------------------------------------------------------------------

class EDITENG_DLLPUBLIC SvxJustifyMethodItem: public SfxEnumItem
{
public:
    SvxJustifyMethodItem(
        const SvxCellJustifyMethod eMethod,
        const sal_uInt16 nId );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual sal_uInt16          GetValueCount() const;
    virtual OUString        GetValueText( sal_uInt16 nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

    SvxJustifyMethodItem& operator=(const SvxJustifyMethodItem& r);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
