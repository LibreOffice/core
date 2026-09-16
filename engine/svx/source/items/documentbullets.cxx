/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/documentbullets.hxx>

#include <editeng/numitem.hxx>
#include <officecfg/Office/Common.hxx>

#include <comphelper/sequence.hxx>

#include <cassert>
#include <utility>

OUString SvxDocumentBullet::GetIdentifier() const
{
    if (IsGraphic())
        return "G" + OUString::number(static_cast<sal_Int64>(aGraphic.GetChecksum()));

    return "C" + aBulletCharacter + aFontName;
}

Size SvxDocumentBullet::GetGraphicSizeMM100() const
{
    if (!aGraphicSize.IsEmpty())
        return aGraphicSize;

    return SvxNumberFormat::GetGraphicSizeMM100(&aGraphic);
}

bool SvxDocumentBullet::IsInDefaultLibrary() const
{
    if (IsGraphic())
        return false;

    const cpo::uno::Sequence<OUString> aCharacters
        = officecfg::Office::Common::BulletsNumbering::DefaultBullets::get();

    // The character alone decides, because the same bullet reaches a document under more than one
    // font name and offering it a second time helps nobody.
    return comphelper::findValue(aCharacters, aBulletCharacter) != -1;
}

SvxDocumentBulletsItem::SvxDocumentBulletsItem(TypedWhichId<SvxDocumentBulletsItem> nWhich,
                                               std::vector<SvxDocumentBullet> aBullets)
    : SfxPoolItem(nWhich)
    , maBullets(std::move(aBullets))
{
}

bool SvxDocumentBulletsItem::operator==(const SfxPoolItem& rAttr) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return maBullets == static_cast<const SvxDocumentBulletsItem&>(rAttr).maBullets;
}

SvxDocumentBulletsItem* SvxDocumentBulletsItem::Clone(SfxItemPool*) const
{
    return new SvxDocumentBulletsItem(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
