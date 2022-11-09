/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAFORMFIELD_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAFORMFIELD_HXX

#include <ooo/vba/word/XFormField.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <IDocumentMarkAccess.hxx>

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XFormField> SwVbaFormField_BASE;

class SwVbaFormField : public SwVbaFormField_BASE
{
private:
    css::uno::Reference<css::frame::XModel> mxModel;
    sw::mark::IFieldmark& m_rFormField;

public:
    /// @throws css::uno::RuntimeException
    SwVbaFormField(const css::uno::Reference<ooo::vba::XHelperInterface>& rParent,
                   const css::uno::Reference<css::uno::XComponentContext>& rContext,
                   const css::uno::Reference<css::frame::XModel>& xModel,
                   sw::mark::IFieldmark& rFormField);
    virtual ~SwVbaFormField() override;

    // XFormField Methods
    virtual OUString SAL_CALL getDefaultPropertyName() override;

    virtual css::uno::Any SAL_CALL CheckBox() override;
    virtual css::uno::Any SAL_CALL DropDown() override;
    virtual css::uno::Any SAL_CALL TextInput() override;
    virtual css::uno::Any SAL_CALL Previous() override;
    virtual css::uno::Any SAL_CALL Next() override;
    virtual css::uno::Any SAL_CALL Range() override;

    // Indicates which of the three form fields this is: oovbaapi/ooo/vba/word/WdFieldType.idl
    virtual sal_Int32 SAL_CALL getType() override;
    // True if references to the specified form field
    // are automatically updated whenever the field is exited
    virtual sal_Bool SAL_CALL getCalculateOnExit() override;
    virtual void SAL_CALL setCalculateOnExit(sal_Bool bSet) override;
    virtual sal_Bool SAL_CALL getEnabled() override;
    virtual void SAL_CALL setEnabled(sal_Bool bSet) override;
    virtual OUString SAL_CALL getEntryMacro() override;
    virtual void SAL_CALL setEntryMacro(const OUString& rSet) override;
    virtual OUString SAL_CALL getExitMacro() override;
    virtual void SAL_CALL setExitMacro(const OUString& rSet) override;
    /*
     * If the OwnHelp property is set to True,
     * HelpText specifies the text string value.
     * If OwnHelp is set to False, HelpText specifies the name of an AutoText entry
     * that contains help text for the form field.
     */
    virtual OUString SAL_CALL getHelpText() override;
    virtual void SAL_CALL setHelpText(const OUString& rSet) override;
    virtual sal_Bool SAL_CALL getOwnHelp() override;
    virtual void SAL_CALL setOwnHelp(sal_Bool bSet) override;

    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& rSet) override;
    virtual OUString SAL_CALL getResult() override;
    virtual void SAL_CALL setResult(const OUString& rSet) override;
    /*
     * If the OwnStatus property is set to True,
     * StatusText specifies the status bar value.
     * If OwnStatus is set to False, StatusText specifies the name of an AutoText entry
     * that contains status bar text for the form field.
     */
    virtual OUString SAL_CALL getStatusText() override;
    virtual void SAL_CALL setStatusText(const OUString& rSet) override;
    virtual sal_Bool SAL_CALL getOwnStatus() override;
    virtual void SAL_CALL setOwnStatus(sal_Bool bSet) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAFORMFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
