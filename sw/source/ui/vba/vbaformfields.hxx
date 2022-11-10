/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/word/XFormFields.hpp>

#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper<ooo::vba::word::XFormFields> SwVbaFormFields_BASE;

class SwVbaFormFields : public SwVbaFormFields_BASE
{
public:
    /// @throws css::uno::RuntimeException
    SwVbaFormFields(const css::uno::Reference<ov::XHelperInterface>& xParent,
                    const css::uno::Reference<css::uno::XComponentContext>& xContext,
                    const css::uno::Reference<css::frame::XModel>& xModel);

    // XFormFields
    sal_Bool SAL_CALL getShaded() override;
    void SAL_CALL setShaded(sal_Bool bSet) override;
    //css::uno::Reference<ooo::vba::word::XFormField> SAL_CALL Add(const css::uno::Any& Range, sal_Int32 Type) override;

    // XEnumerationAccess
    css::uno::Type SAL_CALL getElementType() override;
    css::uno::Reference<css::container::XEnumeration> SAL_CALL createEnumeration() override;

    // SwVbaFormFields_BASE
    css::uno::Any createCollectionObject(const css::uno::Any& aSource) override;
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
