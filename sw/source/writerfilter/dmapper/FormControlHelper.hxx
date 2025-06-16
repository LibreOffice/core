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
#pragma once

#include "FFDataHandler.hxx"
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include "FieldTypes.hxx"
#include <rtl/ref.hxx>

class SwXTextDocument;
class SwXFieldmark;
class SwFmDrawPage;

namespace writerfilter::dmapper
{
class FormControlHelper : public virtual SvRefBase
{
public:
    typedef tools::SvRef<FormControlHelper> Pointer_t;
    FormControlHelper(FieldId eFieldId, rtl::Reference<SwXTextDocument> const& rTextDocument,
                      FFDataHandler::Pointer_t pFFData);
    ~FormControlHelper() override;

    void insertControl(css::uno::Reference<css::text::XTextRange> const& xTextRange);
    void processField(rtl::Reference<SwXFieldmark> const& xFormField);
    bool hasFFDataHandler() const { return (m_pFFData != nullptr); }

private:
    FFDataHandler::Pointer_t m_pFFData;
    FieldId m_eFieldId;
    css::awt::Size maSize;
    rtl::Reference<SwFmDrawPage> mxDrawPage;
    css::uno::Reference<css::form::XForm> mxForm;
    css::uno::Reference<css::form::XFormComponent> mxFormComponent;
    rtl::Reference<SwXTextDocument> mxTextDocument;

    rtl::Reference<SwFmDrawPage> const& getDrawPage();
    css::uno::Reference<css::form::XForm> const& getForm();
    css::uno::Reference<css::container::XIndexContainer> getFormComps();
    bool createCheckbox(css::uno::Reference<css::text::XTextRange> const& xTextRange,
                        const OUString& rControlName);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
