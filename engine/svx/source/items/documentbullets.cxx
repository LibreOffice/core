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

#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

#include <editeng/numitem.hxx>
#include <officecfg/Office/Common.hxx>

#include <comphelper/sequence.hxx>
#include <o3tl/string_view.hxx>

#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>

#include <cassert>
#include <utility>

namespace
{
/// The length of the longer side of a bullet image drawn in a menu, in pixels.
constexpr tools::Long nMenuImageLength = 16;
}

OUString SvxDocumentBullet::GetIdentifier() const
{
    if (IsGraphic())
        return "G" + OUString::number(static_cast<sal_Int64>(aGraphic.GetChecksum()));

    return "C" + aBulletCharacter + aFontName;
}

OUString SvxDocumentBullet::ToStatusString() const { return aBulletCharacter + aFontName; }

SvxDocumentBullet SvxDocumentBullet::FromStatusString(std::u16string_view aStatusString)
{
    SvxDocumentBullet aBullet;
    if (aStatusString.empty())
        return aBullet;

    sal_Int32 nFontNameStart = 0;
    o3tl::iterateCodePoints(aStatusString, &nFontNameStart);
    aBullet.aBulletCharacter = OUString(aStatusString.substr(0, nFontNameStart));
    aBullet.aFontName = OUString(aStatusString.substr(nFontNameStart));
    return aBullet;
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

std::unique_ptr<weld::Menu> CreateDocumentBulletMenu(weld::Builder& rBuilder,
                                                     weld::MenuButton& rMenuButton,
                                                     const std::vector<SvxDocumentBullet>& rBullets)
{
    if (rBullets.empty())
        return nullptr;

    std::unique_ptr<weld::Menu> xMenu = rBuilder.weld_menu(u"fromdocumentsubmenu"_ustr);
    rMenuButton.set_item_visible(u"fromdocument"_ustr, true);

    ScopedVclPtrInstance<VirtualDevice> pVirtualDevice;
    for (size_t nBullet = 0; nBullet < rBullets.size(); ++nBullet)
    {
        Bitmap aBitmap(rBullets[nBullet].aGraphic.GetBitmap());
        Size aSize(aBitmap.GetSizePixel());
        if (aSize.Width() > nMenuImageLength || aSize.Height() > nMenuImageLength)
        {
            // The longer side is scaled to fit, so the picture keeps its proportions.
            const double fScale
                = aSize.Width() > aSize.Height()
                      ? double(nMenuImageLength) / static_cast<double>(aSize.Width())
                      : double(nMenuImageLength) / static_cast<double>(aSize.Height());
            aBitmap.Scale(fScale, fScale);
        }
        pVirtualDevice->SetOutputSizePixel(aBitmap.GetSizePixel(), false);
        pVirtualDevice->DrawBitmap(Point(), aBitmap);

        xMenu->append("document" + OUString::number(nBullet),
                      SvxResId(RID_SVXSTR_DOCUMENT_BULLET_ENTRY)
                          .replaceFirst("%1", OUString::number(nBullet + 1)),
                      *pVirtualDevice);
    }

    return xMenu;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
