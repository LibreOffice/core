/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ooo/vba/word/WdFieldType.hpp>

#include <sal/log.hxx>

#include <doc.hxx>
#include <docsh.hxx>

#include "vbaformfield.hxx"
#include "vbaformfieldcheckbox.hxx"
#include "vbaformfielddropdown.hxx"
#include "vbaformfieldtextinput.hxx"
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/**
 * Information about the method and properties of FormFields was gathered from
 * https://www.codevba.com/Word/FormField.htm
 *
 * FormFields are inline text objects that are only found in MS Word.
 * They cannot be created in Excel or in Calc.
 *
 * There are three specific kinds of FormFields: CheckBox, DropDown, and TextInput.
 */
SwVbaFormField::SwVbaFormField(const uno::Reference<ooo::vba::XHelperInterface>& rParent,
                               const uno::Reference<uno::XComponentContext>& rContext,
                               const uno::Reference<frame::XModel>& xModel,
                               sw::mark::IFieldmark& rFormField)
    : SwVbaFormField_BASE(rParent, rContext)
    , mxModel(xModel)
    , m_rFormField(rFormField)
{
}

SwVbaFormField::~SwVbaFormField() {}

uno::Any SAL_CALL SwVbaFormField::CheckBox()
{
    return uno::Any(uno::Reference<word::XCheckBox>(
        new SwVbaFormFieldCheckBox(mxParent, mxContext, m_rFormField)));
}

uno::Any SAL_CALL SwVbaFormField::DropDown()
{
    return uno::Any(uno::Reference<word::XDropDown>(
        new SwVbaFormFieldDropDown(mxParent, mxContext, m_rFormField)));
}

uno::Any SAL_CALL SwVbaFormField::TextInput()
{
    return uno::Any(uno::Reference<word::XTextInput>(
        new SwVbaFormFieldTextInput(mxParent, mxContext, m_rFormField)));
}

uno::Any SAL_CALL SwVbaFormField::Previous()
{
    SwDoc* pDoc = word::getDocShell(mxModel)->GetDoc();
    if (!pDoc)
        return uno::Any();

    const IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    if (!pMarkAccess)
        return uno::Any();

    sw::mark::IFieldmark* pFieldMark = pMarkAccess->getFieldmarkBefore(m_rFormField.GetMarkPos());

    // DateFields are a LO specialty, and do not exist natively in MS documents. Ignore if added...
    auto pDateField = dynamic_cast<sw::mark::IDateFieldmark*>(pFieldMark);
    while (pDateField)
    {
        pFieldMark = pMarkAccess->getFieldmarkBefore(pDateField->GetMarkPos());
        pDateField = dynamic_cast<sw::mark::IDateFieldmark*>(pFieldMark);
    }

    if (!pFieldMark)
        return uno::Any();

    return uno::Any(uno::Reference<word::XFormField>(
        new SwVbaFormField(mxParent, mxContext, mxModel, *pFieldMark)));
}

uno::Any SAL_CALL SwVbaFormField::Next()
{
    SwDoc* pDoc = word::getDocShell(mxModel)->GetDoc();
    if (!pDoc)
        return uno::Any();

    const IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    if (!pMarkAccess)
        return uno::Any();

    sw::mark::IFieldmark* pFieldMark = pMarkAccess->getFieldmarkAfter(m_rFormField.GetMarkPos());

    // DateFields are a LO specialty, and do not exist natively in MS documents. Ignore if added...
    auto pDateField = dynamic_cast<sw::mark::IDateFieldmark*>(pFieldMark);
    while (pDateField)
    {
        pFieldMark = pMarkAccess->getFieldmarkAfter(pDateField->GetMarkPos());
        pDateField = dynamic_cast<sw::mark::IDateFieldmark*>(pFieldMark);
    }

    if (!pFieldMark)
        return uno::Any();

    return uno::Any(uno::Reference<word::XFormField>(
        new SwVbaFormField(mxParent, mxContext, mxModel, *pFieldMark)));
}

uno::Any SAL_CALL SwVbaFormField::Range()
{
    SAL_INFO("sw.vba", "SwVbaFormField::getRange stub");
    return uno::Any();
}

OUString SwVbaFormField::getDefaultPropertyName() { return "Type"; }

sal_Int32 SwVbaFormField::getType()
{
    IDocumentMarkAccess::MarkType aType = IDocumentMarkAccess::GetType(m_rFormField);
    if (aType == IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK)
        return ooo::vba::word::WdFieldType::wdFieldFormCheckBox;
    else if (aType == IDocumentMarkAccess::MarkType::TEXT_FIELDMARK)
        return ooo::vba::word::WdFieldType::wdFieldFormTextInput;
    return ooo::vba::word::WdFieldType::wdFieldFormDropDown;
}

sal_Bool SwVbaFormField::getCalculateOnExit()
{
    SAL_INFO("sw.vba", "SwVbaFormField::getCalculateOnExit stub");
    return false;
}

void SwVbaFormField::setCalculateOnExit(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaFormField::setCalculateOnExit stub");
}

sal_Bool SwVbaFormField::getEnabled()
{
    SAL_INFO("sw.vba", "SwVbaFormField::getEnabled stub");
    return true;
}

void SwVbaFormField::setEnabled(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaFormField::setEnabled stub");
}

OUString SwVbaFormField::getEntryMacro()
{
    OUString sMacro;
    (*m_rFormField.GetParameters())["EntryMacro"] >>= sMacro;
    return sMacro;
}

void SwVbaFormField::setEntryMacro(const OUString& rSet)
{
    (*m_rFormField.GetParameters())["EntryMacro"] <<= rSet;
}

OUString SwVbaFormField::getExitMacro()
{
    OUString sMacro;
    (*m_rFormField.GetParameters())["ExitMacro"] >>= sMacro;
    return sMacro;
}

void SwVbaFormField::setExitMacro(const OUString& rSet)
{
    (*m_rFormField.GetParameters())["ExitMacro"] <<= rSet;
}

OUString SwVbaFormField::getHelpText() { return m_rFormField.GetFieldHelptext(); }

void SwVbaFormField::setHelpText(const OUString& rSet) { m_rFormField.SetFieldHelptext(rSet); }

sal_Bool SwVbaFormField::getOwnHelp()
{
    SAL_INFO("sw.vba", "SwVbaFormField::getOwnHelp stub");
    return true;
}

void SwVbaFormField::setOwnHelp(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaFormField::setOwnHelp stub");
}

OUString SwVbaFormField::getName() { return m_rFormField.GetName(); }

void SwVbaFormField::setName(const OUString& rSet)
{
    SAL_INFO("sw.vba", "SwVbaFormField::setName[" << rSet << "] stub");
}

OUString SwVbaFormField::getResult() { return m_rFormField.GetContent(); }

void SwVbaFormField::setResult(const OUString& rSet)
{
    if (dynamic_cast<sw::mark::ICheckboxFieldmark*>(&m_rFormField))
        m_rFormField.ReplaceContent("false");
    else
        m_rFormField.ReplaceContent(rSet);
}

OUString SwVbaFormField::getStatusText()
{
    SAL_INFO("sw.vba", "SwVbaFormField::getStatusText stub");
    return OUString();
}

void SwVbaFormField::setStatusText(const OUString& rSet)
{
    SAL_INFO("sw.vba", "SwVbaFormField::setStatusText[" << rSet << "] stub");
}

sal_Bool SwVbaFormField::getOwnStatus()
{
    SAL_INFO("sw.vba", "SwVbaFormField::getOwnStatus stub");
    return true;
}

void SwVbaFormField::setOwnStatus(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaFormField::setOwnStatus stub");
}

OUString SwVbaFormField::getServiceImplName() { return "SwVbaFormField"; }

uno::Sequence<OUString> SwVbaFormField::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{ "ooo.vba.word.FormField" };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
