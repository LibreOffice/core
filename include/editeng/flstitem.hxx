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
#ifndef INCLUDED_EDITENG_FLSTITEM_HXX
#define INCLUDED_EDITENG_FLSTITEM_HXX

#include <svl/poolitem.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <editeng/editengdllapi.h>

// class SvxFontListItem -------------------------------------------------

class FontList;

/*
    [Description]

    This item serves as a transport medium for a font list. The font list is
    not copied and not deleted!
*/

class EDITENG_DLLPUBLIC SvxFontListItem : public SfxPoolItem
{
private:
    const FontList*                 pFontList;
    css::uno::Sequence< OUString >  aFontNameSeq;

public:
    SvxFontListItem( const FontList* pFontLst,
                     const sal_uInt16 nId  );

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    const FontList*         GetFontList() const { return pFontList; }
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
