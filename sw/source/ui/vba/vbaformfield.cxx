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
#include <unotextrange.hxx>

#include "vbaformfield.hxx"
#include "vbaformfieldcheckbox.hxx"
#include "vbaformfielddropdown.hxx"
#include "vbaformfieldtextinput.hxx"
#include "vbarange.hxx"
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/**
 * FormFields are inline text objects that are only found in MS Word.
 * They cannot be created in Excel or in Calc.
 *
 * There are three specific kinds of FormFields: CheckBox, DropDown, and TextInput.
 */
SwVbaFormField::SwVbaFormField(const uno::Reference<ooo::vba::XHelperInterface>& rParent,
                               const uno::Reference<uno::XComponentContext>& rContext,
                               const uno::Reference<text::XTextDocument>& xTextDocument,
                               sw::mark::IFieldmark& rFormField)
    : SwVbaFormField_BASE(rParent, rContext)
    , m_xTextDocument(xTextDocument)
    , m_rFormField(rFormField)
{
}

SwVbaFormField::~SwVbaFormField() {}

uno::Any SwVbaFormField::CheckBox()
{
    return uno::Any(uno::Reference<word::XCheckBox>(
        new SwVbaFormFieldCheckBox(mxParent, mxContext, m_rFormField)));
}

uno::Any SwVbaFormField::DropDown()
{
    return uno::Any(uno::Reference<word::XDropDown>(
        new SwVbaFormFieldDropDown(mxParent, mxContext, m_rFormField)));
}

uno::Any SwVbaFormField::TextInput()
{
    return uno::Any(uno::Reference<word::XTextInput>(
        new SwVbaFormFieldTextInput(mxParent, mxContext, m_rFormField)));
}

uno::Any SwVbaFormField::Previous()
{
    SwDoc* pDoc = word::getDocShell(m_xTextDocument)->GetDoc();
    if (!pDoc)
        return uno::Any();

    const IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    if (!pMarkAccess)
        return uno::Any();

    sw::mark::IFieldmark* pFieldMark = pMarkAccess->getFieldmarkBefore(m_rFormField.GetMarkPos(),
                                                                       /*bLoop=*/false);

    // DateFields are a LO specialty, and do not exist natively in MS documents. Ignore if added...
    auto pDateField = dynamic_cast<sw::mark::IDateFieldmark*>(pFieldMark);
    while (pDateField)
    {
        pFieldMark = pMarkAccess->getFieldmarkBefore(pDateField->GetMarkPos(), /*bLoop=*/false);
        pDateField = dynamic_cast<sw::mark::IDateFieldmark*>(pFieldMark);
    }

    if (!pFieldMark)
        return uno::Any();

    return uno::Any(uno::Reference<word::XFormField>(
        new SwVbaFormField(mxParent, mxContext, m_xTextDocument, *pFieldMark)));
}

uno::Any SwVbaFormField::Next()
{
    SwDoc* pDoc = word::getDocShell(m_xTextDocument)->GetDoc();
    if (!pDoc)
        return uno::Any();

    const IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    if (!pMarkAccess)
        return uno::Any();

    sw::mark::IFieldmark* pFieldMark = pMarkAccess->getFieldmarkAfter(m_rFormField.GetMarkPos(),
                                                                      /*bLoop=*/false);

    // DateFields are a LO specialty, and do not exist natively in MS documents. Ignore if added...
    auto pDateField = dynamic_cast<sw::mark::IDateFieldmark*>(pFieldMark);
    while (pDateField)
    {
        pFieldMark = pMarkAccess->getFieldmarkAfter(pDateField->GetMarkPos(), /*bLoop=*/false);
        pDateField = dynamic_cast<sw::mark::IDateFieldmark*>(pFieldMark);
    }

    if (!pFieldMark)
        return uno::Any();

    return uno::Any(uno::Reference<word::XFormField>(
        new SwVbaFormField(mxParent, mxContext, m_xTextDocument, *pFieldMark)));
}

uno::Reference<word::XRange> SwVbaFormField::Range()
{
    uno::Reference<word::XRange> xRet;
    SwDoc* pDoc = word::getDocShell(m_xTextDocument)->GetDoc();
    if (pDoc)
    {
        rtl::Reference<SwXTextRange> xText(SwXTextRange::CreateXTextRange(
            *pDoc, m_rFormField.GetMarkStart(), &m_rFormField.GetMarkEnd()));
        if (xText.is())
            xRet = new SwVbaRange(mxParent, mxContext, m_xTextDocument, xText->getStart(),
                                  xText->getEnd());
    }
    return xRet;
}

OUString SwVbaFormField::getDefaultPropertyName() { return u"Type"_ustr; }

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
    (*m_rFormField.GetParameters())[u"EntryMacro"_ustr] >>= sMacro;
    return sMacro;
}

void SwVbaFormField::setEntryMacro(const OUString& rSet)
{
    (*m_rFormField.GetParameters())[u"EntryMacro"_ustr] <<= rSet;
}

OUString SwVbaFormField::getExitMacro()
{
    OUString sMacro;
    (*m_rFormField.GetParameters())[u"ExitMacro"_ustr] >>= sMacro;
    return sMacro;
}

void SwVbaFormField::setExitMacro(const OUString& rSet)
{
    (*m_rFormField.GetParameters())[u"ExitMacro"_ustr] <<= rSet;
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
        m_rFormField.ReplaceContent(u"false"_ustr);
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

OUString SwVbaFormField::getServiceImplName() { return u"SwVbaFormField"_ustr; }

uno::Sequence<OUString> SwVbaFormField::getServiceNames()
{
    static uno::Sequence<OUString> const aServiceNames{ u"ooo.vba.word.FormField"_ustr };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
