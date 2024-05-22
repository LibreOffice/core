/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ooo/vba/word/WdTextFormFieldType.hpp>

#include <sal/log.hxx>

#include "vbaformfieldtextinput.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/**
 * TextInput formfields are inline text objects that are only found in MS Word.
 * They cannot be created in Excel or in Calc.
 *
 * Note that VBA might call this a TextInput, but it might not actually be one,
 * so make good use of getValid()
 */
SwVbaFormFieldTextInput::SwVbaFormFieldTextInput(
    const uno::Reference<ooo::vba::XHelperInterface>& rParent,
    const uno::Reference<uno::XComponentContext>& rContext, sw::mark::IFieldmark& rFormField)
    : SwVbaFormFieldTextInput_BASE(rParent, rContext)
    , m_rTextInput(rFormField)
{
}

SwVbaFormFieldTextInput::~SwVbaFormFieldTextInput() {}

OUString SwVbaFormFieldTextInput::getDefaultPropertyName() { return u"Valid"_ustr; }

sal_Bool SwVbaFormFieldTextInput::getValid()
{
    return IDocumentMarkAccess::GetType(m_rTextInput)
           == IDocumentMarkAccess::MarkType::TEXT_FIELDMARK;
}

OUString SwVbaFormFieldTextInput::getDefault()
{
    if (!getValid())
        return OUString();

    return m_rTextInput.GetContent();
}

void SwVbaFormFieldTextInput::setDefault(const OUString& sSet)
{
    // Hard to know what to do here, since LO doesn't have a default property for text input.
    // This really only makes sense when macro-adding a text input.
    // In that case, we want it to affect the actual text content.
    // However, if the text has already been set by the user, then this shouldn't do anything.
    // Assuming this is only ever set when adding a text input seems the sanest approach.
    if (!getValid() || getDefault() == sSet)
        return;

    m_rTextInput.ReplaceContent(sSet);
}

OUString SwVbaFormFieldTextInput::getFormat()
{
    if (!getValid())
        return OUString();

    SAL_INFO("sw.vba", "SwVbaFormFieldTextInput::getFormat stub");
    return OUString();
}

sal_Int32 SwVbaFormFieldTextInput::getType()
{
    if (!getValid())
        return word::WdTextFormFieldType::wdRegularText;

    SAL_INFO("sw.vba", "SwVbaFormFieldTextInput::getType stub");
    return word::WdTextFormFieldType::wdRegularText;
}

sal_Int32 SwVbaFormFieldTextInput::getWidth()
{
    if (!getValid())
        return 0;

    SAL_INFO("sw.vba", "SwVbaFormFieldTextInput::getWidth stub");
    return 11 * 50;
}

void SwVbaFormFieldTextInput::setWidth(sal_Int32 nWidth)
{
    if (!getValid())
        return;

    SAL_INFO("sw.vba", "SwVbaFormFieldTextInput::setWidth[" << nWidth << "] stub");
}

void SwVbaFormFieldTextInput::Clear()
{
    if (!getValid() || m_rTextInput.GetContent().isEmpty())
        return;

    m_rTextInput.ReplaceContent(u""_ustr);
}

void SwVbaFormFieldTextInput::EditType(sal_Int32 nType, const uno::Any& rDefault,
                                       const uno::Any& rFormat, const uno::Any& rEnabled)
{
    OUString sDefault;
    OUString sFormat;
    bool bEnabled = true;
    rDefault >>= sDefault;
    rFormat >>= sFormat;
    rEnabled >>= bEnabled;
    SAL_INFO("sw.vba", "SwVbaFormFieldTextInput::EditType["
                           << nType << "] sDefault[" << sDefault << "] sFormat[" << sFormat
                           << "] bEnabled[" << bEnabled << "] stub");
}

OUString SwVbaFormFieldTextInput::getServiceImplName() { return u"SwVbaFormFieldTextInput"_ustr; }

uno::Sequence<OUString> SwVbaFormFieldTextInput::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{ u"ooo.vba.word.TextInput"_ustr };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
