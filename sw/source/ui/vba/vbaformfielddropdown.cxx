/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ooo/vba/word/WdTextFormFieldType.hpp>

#include "vbaformfielddropdown.hxx"
#include "vbaformfielddropdownlistentries.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/**
 * DropDown formfields are inline text objects that are only found in MS Word.
 * They cannot be created in Excel or in Calc.
 *
 * Note that VBA might call this a DropDown, but it might not actually be one,
 * so make good use of getValid()
 */
SwVbaFormFieldDropDown::SwVbaFormFieldDropDown(
    const uno::Reference<ooo::vba::XHelperInterface>& rParent,
    const uno::Reference<uno::XComponentContext>& rContext, ::sw::mark::IFieldmark& rFormField)
    : SwVbaFormFieldDropDown_BASE(rParent, rContext)
    , m_pDropDown(dynamic_cast<sw::mark::IDropdownFieldmark*>(&rFormField))
{
}

SwVbaFormFieldDropDown::~SwVbaFormFieldDropDown() {}

OUString SwVbaFormFieldDropDown::getDefaultPropertyName() { return u"Valid"_ustr; }

sal_Bool SwVbaFormFieldDropDown::getValid()
{
    return m_pDropDown
           && IDocumentMarkAccess::GetType(*m_pDropDown)
                  == IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK;
}

sal_Int32 SwVbaFormFieldDropDown::getDefault() { return getValue(); }

void SwVbaFormFieldDropDown::setDefault(sal_Int32 nSet)
{
    // Hard to know what to do here, since LO doesn't have a default property for DropDowns.
    // Setting this really only makes sense when macro-adding a DropDown.
    // In that case, we want it to affect the actual text content.
    // However, if an item has already been selected by the user, then this shouldn't do anything.
    // Assuming this is only ever set when adding a DropDown seems the sanest approach.
    setValue(nSet);
}

sal_Int32 SwVbaFormFieldDropDown::getValue()
{
    sal_Int32 nRet = 0;
    if (!getValid())
        return nRet;

    --nRet; // send -1, which requests being changed to the selected DropDown's zero-based index
    m_pDropDown->GetContent(&nRet);
    return nRet + 1;
}

void SwVbaFormFieldDropDown::setValue(sal_Int32 nIndex)
{
    if (!getValid() || nIndex == getValue())
        return;

    // switch to zero-based index for implementation
    --nIndex;
    m_pDropDown->ReplaceContent(/*pText=*/nullptr, &nIndex);
}

uno::Any SwVbaFormFieldDropDown::ListEntries(const uno::Any& rIndex)
{
    if (!getValid())
        return uno::Any();

    uno::Reference<XCollection> xCol(
        new SwVbaFormFieldDropDownListEntries(this, mxContext, *m_pDropDown));

    if (rIndex.hasValue())
        return xCol->Item(rIndex, uno::Any());

    return uno::Any(xCol);
}

OUString SwVbaFormFieldDropDown::getServiceImplName() { return u"SwVbaFormFieldDropDown"_ustr; }

uno::Sequence<OUString> SwVbaFormFieldDropDown::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{ u"ooo.vba.word.DropDown"_ustr };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
