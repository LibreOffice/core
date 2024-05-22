/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/log.hxx>

#include "vbaformfieldcheckbox.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/**
 * CheckBoxes are inline text objects that are only found in MS Word.
 * They cannot be created in Excel or in Calc.
 *
 * Note that VBA might call this a Checkbox, but it might not actually be one,
 * so make good use of getValid()
 */
SwVbaFormFieldCheckBox::SwVbaFormFieldCheckBox(
    const uno::Reference<ooo::vba::XHelperInterface>& rParent,
    const uno::Reference<uno::XComponentContext>& rContext, sw::mark::IFieldmark& rFormField)
    : SwVbaFormFieldCheckBox_BASE(rParent, rContext)
    , m_pCheckBox(dynamic_cast<sw::mark::ICheckboxFieldmark*>(&rFormField))
{
}

SwVbaFormFieldCheckBox::~SwVbaFormFieldCheckBox() {}

OUString SwVbaFormFieldCheckBox::getDefaultPropertyName() { return u"Valid"_ustr; }

sal_Bool SwVbaFormFieldCheckBox::getValid()
{
    return m_pCheckBox
           && IDocumentMarkAccess::GetType(*m_pCheckBox)
                  == IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK;
}

sal_Bool SwVbaFormFieldCheckBox::getAutoSize()
{
    if (!getValid())
        return false;

    SAL_INFO("sw.vba", "SwVbaFormFieldCheckBox::getAutoSize stub");
    return true;
}

void SwVbaFormFieldCheckBox::setAutoSize(sal_Bool /*bSet*/)
{
    if (!getValid())
        return;

    SAL_INFO("sw.vba", "SwVbaFormFieldCheckBox::setAutoSize stub");
}

sal_Bool SwVbaFormFieldCheckBox::getDefault()
{
    if (!getValid())
        return false;

    return getValue();
}

void SwVbaFormFieldCheckBox::setDefault(sal_Bool bSet)
{
    if (!getValid())
        return;

    // Hard to know what to do here, since LO doesn't have a default property for checkboxes.
    // Setting this really only makes sense when macro-adding a checkbox.
    // In that case, we want it to affect the actual checkbox.
    // However, if the checkbox has already been set by the user, then this shouldn't do anything.
    // Assuming this is only ever called when adding a checkbox seems the sanest approach.
    setValue(bSet);
}

// Returns the size of a check box, in points
sal_Int32 SwVbaFormFieldCheckBox::getSize()
{
    if (!getValid())
        return 0;

    SAL_INFO("sw.vba", "SwVbaFormFieldCheckBox::getSize stub");
    return 11;
}

void SwVbaFormFieldCheckBox::setSize(sal_Int32 nSet)
{
    if (!getValid())
        return;

    SAL_INFO("sw.vba", "SwVbaFormFieldCheckBox::setSize[" << nSet << "] stub");
}

sal_Bool SwVbaFormFieldCheckBox::getValue() { return getValid() && m_pCheckBox->IsChecked(); }

void SwVbaFormFieldCheckBox::setValue(sal_Bool bSet)
{
    if (!getValid() || !getValue() == !bSet)
        return;

    m_pCheckBox->SetChecked(bSet);
}

OUString SwVbaFormFieldCheckBox::getServiceImplName() { return u"SwVbaFormFieldCheckBox"_ustr; }

uno::Sequence<OUString> SwVbaFormFieldCheckBox::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{ u"ooo.vba.word.CheckBox"_ustr };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
