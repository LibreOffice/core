/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/text/XTextDocument.hpp>
#include <ooo/vba/word/XFormField.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <IDocumentMarkAccess.hxx>

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XFormField> SwVbaFormField_BASE;

class SwVbaFormField : public SwVbaFormField_BASE
{
private:
    css::uno::Reference<css::text::XTextDocument> m_xTextDocument;
    sw::mark::Fieldmark& m_rFormField;

public:
    /// @throws css::uno::RuntimeException
    SwVbaFormField(const css::uno::Reference<ooo::vba::XHelperInterface>& rParent,
                   const css::uno::Reference<css::uno::XComponentContext>& rContext,
                   const uno::Reference<text::XTextDocument>& xTextDocument,
                   sw::mark::Fieldmark& rFormField);
    ~SwVbaFormField() override;

    // XFormField Methods
    OUString SAL_CALL getDefaultPropertyName() override;

    css::uno::Any SAL_CALL CheckBox() override;
    css::uno::Any SAL_CALL DropDown() override;
    css::uno::Any SAL_CALL TextInput() override;
    css::uno::Any SAL_CALL Previous() override;
    css::uno::Any SAL_CALL Next() override;
    css::uno::Reference<ooo::vba::word::XRange> SAL_CALL Range() override;

    // Indicates which of the three form fields this is: oovbaapi/ooo/vba/word/WdFieldType.idl
    sal_Int32 SAL_CALL getType() override;
    // True if references to the specified form field
    // are automatically updated whenever the field is exited
    sal_Bool SAL_CALL getCalculateOnExit() override;
    void SAL_CALL setCalculateOnExit(sal_Bool bSet) override;
    sal_Bool SAL_CALL getEnabled() override;
    void SAL_CALL setEnabled(sal_Bool bSet) override;
    OUString SAL_CALL getEntryMacro() override;
    void SAL_CALL setEntryMacro(const OUString& rSet) override;
    OUString SAL_CALL getExitMacro() override;
    void SAL_CALL setExitMacro(const OUString& rSet) override;
    /*
     * If the OwnHelp property is set to True,
     * HelpText specifies the text string value.
     * If OwnHelp is set to False, HelpText specifies the name of an AutoText entry
     * that contains help text for the form field.
     */
    OUString SAL_CALL getHelpText() override;
    void SAL_CALL setHelpText(const OUString& rSet) override;
    sal_Bool SAL_CALL getOwnHelp() override;
    void SAL_CALL setOwnHelp(sal_Bool bSet) override;

    OUString SAL_CALL getName() override;
    void SAL_CALL setName(const OUString& rSet) override;
    OUString SAL_CALL getResult() override;
    void SAL_CALL setResult(const OUString& rSet) override;
    /*
     * If the OwnStatus property is set to True,
     * StatusText specifies the status bar value.
     * If OwnStatus is set to False, StatusText specifies the name of an AutoText entry
     * that contains status bar text for the form field.
     */
    OUString SAL_CALL getStatusText() override;
    void SAL_CALL setStatusText(const OUString& rSet) override;
    sal_Bool SAL_CALL getOwnStatus() override;
    void SAL_CALL setOwnStatus(sal_Bool bSet) override;

    // XHelperInterface
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
