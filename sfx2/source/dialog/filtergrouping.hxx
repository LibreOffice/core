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

#ifndef INCLUDED_SFX2_SOURCE_DIALOG_FILTERGROUPING_HXX
#define INCLUDED_SFX2_SOURCE_DIALOG_FILTERGROUPING_HXX

#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include "filedlgimpl.hxx"


namespace sfx2
{


    class TSortedFilterList
    {
    private:
        ::std::vector< OUString > m_lFilters;
        sal_Int32 m_nIterator;

    public:
        explicit TSortedFilterList(const css::uno::Reference< css::container::XEnumeration >& xFilterList);
        const SfxFilter* First();
        const SfxFilter* Next();

    private:
        const SfxFilter* impl_getFilter(sal_Int32 nIndex);
    };


    /** adds the given filters to the filter manager.
        <p>To be used when saving generic files.</p>
    */
    void appendFiltersForSave(
        TSortedFilterList& _rFilterMatcher,
        const css::uno::Reference< css::ui::dialogs::XFilterManager >& _rFilterManager,
        OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl,
        const OUString& _rFactory
    );

    void appendExportFilters(
        TSortedFilterList& _rFilterMatcher,
        const css::uno::Reference< css::ui::dialogs::XFilterManager >& _rFilterManager,
        OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl
        );


    /** adds the given filters to the filter manager.
        <p>To be used when opening generic files.</p>
    */
    void appendFiltersForOpen(
        TSortedFilterList& _rFilterMatcher,
        const css::uno::Reference< css::ui::dialogs::XFilterManager >& _rFilterManager,
        OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl
    );


    /** adds the given extension to the display text.
        <p>To be used when opening or save generic files.</p>
    */
    OUString addExtension(
        const OUString& _rDisplayText,
        const OUString& _rExtension,
        bool _bForOpen,
        FileDialogHelper_Impl& _rFileDlgImpl
    );


}   // namespace sfx2


#endif // INCLUDED_SFX2_SOURCE_DIALOG_FILTERGROUPING_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
