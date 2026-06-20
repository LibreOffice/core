/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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
    OpenDataSource(const OUString& Name, const cpo::uno::Any& Format,
                   const cpo::uno::Any& ConfirmConversions, const cpo::uno::Any& ReadOnly,
                   const cpo::uno::Any& LinkToSource, const cpo::uno::Any& AddToRecentFiles,
                   const cpo::uno::Any& PasswordDocument, const cpo::uno::Any& PasswordTemplate,
                   const cpo::uno::Any& Revert, const cpo::uno::Any& WritePasswordDocument,
                   const cpo::uno::Any& WritePasswordTemplate, const cpo::uno::Any& Connection,
                   const cpo::uno::Any& SQLStatement, const cpo::uno::Any& SQLStatement1,
                   const cpo::uno::Any& OpenExclusive, const cpo::uno::Any& SubType) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAMAILMERGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
