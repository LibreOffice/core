/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vbaformfielddropdownlistentry.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaFormFieldDropDownListEntry::SwVbaFormFieldDropDownListEntry(
    const uno::Reference<ooo::vba::XHelperInterface>& rParent,
    const uno::Reference<uno::XComponentContext>& rContext,
    sw::mark::IDropdownFieldmark& rFormField, sal_Int32 nZIndex)
    : SwVbaFormFieldDropDownListEntry_BASE(rParent, rContext)
    , m_rDropDown(rFormField)
    , m_nZIndex(nZIndex)
{
}

SwVbaFormFieldDropDownListEntry::~SwVbaFormFieldDropDownListEntry() {}

// XListEntry
sal_Int32 SwVbaFormFieldDropDownListEntry::getIndex() { return m_nZIndex + 1; }

OUString SwVbaFormFieldDropDownListEntry::getName()
{
    sal_Int32 nZIndex = m_nZIndex;
    return m_rDropDown.GetContent(&nZIndex);
}

void SwVbaFormFieldDropDownListEntry::setName(const OUString& rSet)
{
    sal_Int32 nZIndex = m_nZIndex;
    m_rDropDown.ReplaceContent(&rSet, &nZIndex);
}

void SwVbaFormFieldDropDownListEntry::Delete() { m_rDropDown.DelContent(m_nZIndex); }

// XHelperInterface
OUString SwVbaFormFieldDropDownListEntry::getServiceImplName()
{
    return u"SwVbaFormFieldDropDownListEntry"_ustr;
}

uno::Sequence<OUString> SwVbaFormFieldDropDownListEntry::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{ u"ooo.vba.word.ListEntry"_ustr };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
