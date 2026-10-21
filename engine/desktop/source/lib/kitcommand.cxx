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

#include "kitcommand.hxx"

#include <map>

#include <rtl/ustring.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svdview.hxx>

namespace kitcommand
{
void setTextCursor(SfxViewShell& rViewShell, std::u16string_view aCommand)
{
    const std::map<OUString, OUString> aParameters = KitHelper::parseCommandParameters(aCommand);
    const auto aObjectIterator = aParameters.find(u"object"_ustr);
    const auto aParagraphIterator = aParameters.find(u"paragraph"_ustr);
    const auto aIndexIterator = aParameters.find(u"index"_ustr);
    if (aObjectIterator == aParameters.end() || aParagraphIterator == aParameters.end()
        || aIndexIterator == aParameters.end())
        return;

    const sal_Int32 nParagraph = aParagraphIterator->second.toInt32();
    const sal_Int32 nIndex = aIndexIterator->second.toInt32();
    const auto aAnchorParagraphIterator = aParameters.find(u"anchorparagraph"_ustr);
    const auto aAnchorIndexIterator = aParameters.find(u"anchorindex"_ustr);
    const bool bHasAnchor = aAnchorParagraphIterator != aParameters.end()
                            && aAnchorIndexIterator != aParameters.end();

    // The states that follow the caret, such as its font, are computed again.
    SdrView* pDrawView = rViewShell.GetDrawView();
    if (pDrawView
        && pDrawView->SetKitTextCursor(
               aObjectIterator->second.toUInt64(), nParagraph, nIndex,
               bHasAnchor ? aAnchorParagraphIterator->second.toInt32() : nParagraph,
               bHasAnchor ? aAnchorIndexIterator->second.toInt32() : nIndex))
        rViewShell.GetViewFrame().GetBindings().InvalidateAll(false);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
