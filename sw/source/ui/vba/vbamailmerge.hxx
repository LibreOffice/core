/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAMAILMERGE_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAMAILMERGE_HXX

#include <ooo/vba/word/XMailMerge.hpp>
#include <rtl/ref.hxx>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XMailMerge> SwVbaMailMerge_BASE;

// Singleton class. Get the single instance using the get() method.

class SwVbaMailMerge : public SwVbaMailMerge_BASE
{
    sal_Int32 m_nMainDocType;

    SwVbaMailMerge(const css::uno::Reference<ooo::vba::XHelperInterface>& xParent,
                   const css::uno::Reference<css::uno::XComponentContext>& xContext);

public:
    virtual ~SwVbaMailMerge() override;

    static rtl::Reference<SwVbaMailMerge> const&
    get(const css::uno::Reference<ooo::vba::XHelperInterface>& xParent,
        const css::uno::Reference<css::uno::XComponentContext>& xContext);

    // XMailMerge
    virtual sal_Int32 SAL_CALL getMainDocumentType() override;
    virtual void SAL_CALL setMainDocumentType(sal_Int32 _maindocumenttype) override;

    virtual void SAL_CALL
    OpenDataSource(const OUString& Name, const css::uno::Any& Format,
                   const css::uno::Any& ConfirmConversions, const css::uno::Any& ReadOnly,
                   const css::uno::Any& LinkToSource, const css::uno::Any& AddToRecentFiles,
                   const css::uno::Any& PasswordDocument, const css::uno::Any& PasswordTemplate,
                   const css::uno::Any& Revert, const css::uno::Any& WritePasswordDocument,
                   const css::uno::Any& WritePasswordTemplate, const css::uno::Any& Connection,
                   const css::uno::Any& SQLStatement, const css::uno::Any& SQLStatement1,
                   const css::uno::Any& OpenExclusive, const css::uno::Any& SubType) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAMAILMERGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
