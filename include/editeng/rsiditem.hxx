/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_EDITENG_RSIDITEM_HXX
#define INCLUDED_EDITENG_RSIDITEM_HXX

#include <svl/intitem.hxx>
#include <editeng/editengdllapi.h>


// SvxRsidItem


class EDITENG_DLLPUBLIC SvxRsidItem : public SfxUInt32Item
{
public:
    SvxRsidItem( sal_uInt32 nRsid, sal_uInt16 nId ) : SfxUInt32Item( nId, nRsid ) {}

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SfxPoolItem* Clone( SfxItemPool* pPool = nullptr ) const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif // INCLUDED_EDITENG_RSIDITEM_HXX
