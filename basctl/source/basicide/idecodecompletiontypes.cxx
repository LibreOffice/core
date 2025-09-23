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
IdeParamInfo::IdeParamInfo(OUString sName_, OUString sTypeName_, bool bOut, bool bIn)
    : sName(std::move(sName_))
    , sTypeName(std::move(sTypeName_))
    , bIsOut(bOut)
    , bIsIn(bIn)
{
}

IdeDimensionInfo::IdeDimensionInfo(sal_Int32 lower, sal_Int32 upper, bool dynamic)
    : nLowerBound(lower)
    , nUpperBound(upper)
    , bIsDynamic(dynamic)
{
}

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

void IdeSymbolInfo::AddMember(std::shared_ptr<IdeSymbolInfo> pMember)
{
    if (!pMember)
        return;

    mapMembers.insert_or_assign(pMember->sName, std::move(pMember));
}

OUString CreateRootIdentifier(IdeSymbolKind eKind, std::u16string_view sOptionalPayload)
{
    OUStringBuffer sId(u"root:");

    switch (eKind)
    {
        case IdeSymbolKind::ROOT_UNO_APIS:
            sId.append(u"uno_apis");
            break;
        case IdeSymbolKind::ROOT_APPLICATION_LIBS:
            sId.append(u"app_macros");
            break;
        case IdeSymbolKind::ROOT_DOCUMENT_LIBS:
            sId.append(OUString::Concat(u"doc:") + sOptionalPayload);
            break;
        case IdeSymbolKind::ROOT_BASIC_BUILTINS:
            sId.append(u"builtins");
            break;
        default:
            sId.append(u"unknown");
            break;
    }
    return sId.makeStringAndClear();
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
