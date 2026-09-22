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

#pragma once

#include <svx/svxdllapi.h>

#include <svl/poolitem.hxx>
#include <vcl/graph.hxx>

#include <memory>
#include <vector>

namespace weld
{
class Builder;
class Menu;
class MenuButton;
}

/// One bullet that a document already uses, either a character taken from a font or an image.
/// Exactly one of the two halves carries a value. A character bullet has a character and a font
/// name and an empty graphic, an image bullet has a graphic and an empty character and font name.
struct SVXCORE_DLLPUBLIC SvxDocumentBullet
{
    /// The bullet character, as a string one character long.
    OUString aBulletCharacter;
    /// Family name of the font that the bullet character belongs to.
    OUString aFontName;
    /// The image drawn in place of a bullet character.
    Graphic aGraphic;
    /// The size the image is drawn at, in the measurement unit of the document the bullet came
    /// from. Empty for a character bullet, and for an image whose numbering rule recorded no size
    /// of its own.
    Size aGraphicSize;

    bool IsGraphic() const { return aBulletCharacter.isEmpty(); }

    /// The size to draw the image at, in 1/100 mm. Falls back to the size the picture itself
    /// carries when the numbering rule recorded none.
    Size GetGraphicSizeMM100() const;

    /// A name that tells this bullet apart from every other bullet of the same document. For a
    /// character bullet it is the character followed by the font name. For an image bullet it is
    /// the checksum of the image, which stays the same for as long as the document is open.
    OUString GetIdentifier() const;

    /// Whether the default bullet library already offers this bullet. Always false for an image,
    /// because that library holds characters only. The library is the DefaultBullets setting,
    /// which a user can change.
    bool IsInDefaultLibrary() const;

    /// Two bullets are equal when they name the same thing, which is the same test the list is
    /// built with. Comparing the images themselves would answer false while one of them is
    /// swapped out of memory.
    bool operator==(const SvxDocumentBullet& rOther) const
    {
        return GetIdentifier() == rOther.GetIdentifier();
    }
};

/// The bullets that a document already uses, in the order they were found in it.
class SVXCORE_DLLPUBLIC SvxDocumentBulletsItem final : public SfxPoolItem
{
    std::vector<SvxDocumentBullet> maBullets;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxDocumentBulletsItem)
    SvxDocumentBulletsItem(TypedWhichId<SvxDocumentBulletsItem> nWhich,
                           std::vector<SvxDocumentBullet> aBullets);

    const std::vector<SvxDocumentBullet>& GetBullets() const { return maBullets; }

    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SvxDocumentBulletsItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

/// Builds the submenu that offers the image bullets a document already uses, and shows the entry
/// of rMenuButton that holds it. Each entry is drawn as a small preview of the image and named
/// after its position in rBullets, and the identifier of the entry for rBullets[n] is
/// "document<n>". An empty rBullets gives an empty pointer back and leaves the entry hidden,
/// because a submenu with nothing in it is worse than no entry at all.
///
/// rBuilder must have built a dialog description that carries a menu named "fromdocumentsubmenu"
/// and an item named "fromdocument".
SVXCORE_DLLPUBLIC std::unique_ptr<weld::Menu>
CreateDocumentBulletMenu(weld::Builder& rBuilder, weld::MenuButton& rMenuButton,
                         const std::vector<SvxDocumentBullet>& rBullets);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
