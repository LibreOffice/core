/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVX_SDGLOWMETRICITEM_HXX
#define INCLUDED_SVX_SDGLOWMETRICITEM_HXX

#include <svx/svddef.hxx>
#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>

#include <com/sun/star/table/GlowFormat.hpp>
#include <drawinglayer/attribute/sdrglowattribute.hxx>

class SVX_DLLPUBLIC SdrGlowItem : public SfxPoolItem
{
private:
    css::uno::Reference<css::table::GlowFormat> m_xGlow;

public:
    SdrGlowItem();
    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric,
                                 MapUnit ePresMetric, OUString& rText,
                                 const IntlWrapper&) const override;
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;
    virtual SdrGlowItem* Clone(SfxItemPool* pPool = nullptr) const override;

    drawinglayer::attribute::SdrGlowAttribute GetGlowAttr() const;
};

#endif
