/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basctl/idecodecompletiontypes.hxx>
#include <rtl/ustrbuf.hxx>

namespace basctl
{
IdeSymbolInfo::IdeSymbolInfo(std::u16string_view rName, IdeSymbolKind eTheKind,
                             std::u16string_view rParentIdentifier)
    : sName(rName)
    , eKind(eTheKind)
{
    if (rParentIdentifier.empty())
    {
        sIdentifier = OUString::Concat(OUStringLiteral(u"root:")) + rName;
    }
    else
    {
        // Child ID = Parent's Full ID + ":" + Child's Name
        sIdentifier = OUString::Concat(rParentIdentifier) + u":" + rName;
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
