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
#include <ooo/vba/word/XContentControls.hpp>

#include <vbahelper/vbacollectionimpl.hxx>
#include <rtl/ref.hxx>

class SwXTextDocument;

typedef CollTestImplHelper<ooo::vba::word::XContentControls> SwVbaContentControls_BASE;

class SwVbaContentControls : public SwVbaContentControls_BASE
{
public:
    /// @throws css::uno::RuntimeException
    SwVbaContentControls(const css::uno::Reference<ov::XHelperInterface>& xParent,
                         const css::uno::Reference<css::uno::XComponentContext>& xContext,
                         const rtl::Reference<SwXTextDocument>& xTextDocument, const OUString& rTag,
                         const OUString& rTitle);

    // XContentControls
    //css::uno::Reference<ooo::vba::word::XContentControl> SAL_CALL Add(const css::uno::Any& Type, const css::uno::Any& Range) override;

    // XEnumerationAccess
    css::uno::Type SAL_CALL getElementType() override;
    css::uno::Reference<css::container::XEnumeration> SAL_CALL createEnumeration() override;

    // SwVbaContentControls_BASE
    css::uno::Any createCollectionObject(const css::uno::Any& aSource) override;
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
