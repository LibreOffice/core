/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column:100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vbamailmerge.hxx"

#include <ooo/vba/word/WdMailMergeMainDocType.hpp>

SwVbaMailMerge::SwVbaMailMerge(const css::uno::Reference<ooo::vba::XHelperInterface>& xParent,
                               const css::uno::Reference<css::uno::XComponentContext>& xContext)
    : SwVbaMailMerge_BASE(xParent, xContext)
    , m_nMainDocType(ooo::vba::word::WdMailMergeMainDocType::wdNotAMergeDocument)
{
}

SwVbaMailMerge::~SwVbaMailMerge() {}

rtl::Reference<SwVbaMailMerge> const&
SwVbaMailMerge::get(const css::uno::Reference<ooo::vba::XHelperInterface>& xParent,
                    const css::uno::Reference<css::uno::XComponentContext>& xContext)
{
    static rtl::Reference<SwVbaMailMerge> xInstance(new SwVbaMailMerge(xParent, xContext));

    return xInstance;
}

sal_Int32 SAL_CALL SwVbaMailMerge::getMainDocumentType() { return m_nMainDocType; }

void SAL_CALL SwVbaMailMerge::setMainDocumentType(sal_Int32 _maindocumenttype)
{
    m_nMainDocType = _maindocumenttype;
}

// Completely dummy, no-op.
void SAL_CALL SwVbaMailMerge::OpenDataSource(
    const OUString&, const css::uno::Any&, const css::uno::Any&, const css::uno::Any&,
    const css::uno::Any&, const css::uno::Any&, const css::uno::Any&, const css::uno::Any&,
    const css::uno::Any&, const css::uno::Any&, const css::uno::Any&, const css::uno::Any&,
    const css::uno::Any&, const css::uno::Any&, const css::uno::Any&, const css::uno::Any&)
{
}

OUString SwVbaMailMerge::getServiceImplName() { return u"SwVbaMailMerge"_ustr; }

css::uno::Sequence<OUString> SwVbaMailMerge::getServiceNames()
{
    static css::uno::Sequence<OUString> const aServiceNames{ u"ooo.vba.word.MailMerge"_ustr };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
