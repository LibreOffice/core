/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SFX2_FILTERGROUPING_HXX
#define SFX2_FILTERGROUPING_HXX

#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include "filedlgimpl.hxx"

class SfxFilterMatcherIter;

//........................................................................
namespace sfx2
{
//........................................................................

    class TSortedFilterList
    {
        private:

            ::std::vector< ::rtl::OUString > m_lFilters;
            sal_Int32 m_nIterator;

        public:

            TSortedFilterList(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >& xFilterList);
            const SfxFilter* First();
            const SfxFilter* Next();

        private:
            const SfxFilter* impl_getFilter(sal_Int32 nIndex);
    };

    //--------------------------------------------------------------------
    /** adds the given filters to the filter manager.
        <p>To be used when saving generic files.</p>
    */
    void appendFiltersForSave(
        TSortedFilterList& _rFilterMatcher,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilterManager >& _rFilterManager,
        ::rtl::OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl,
        const ::rtl::OUString& _rFactory
    );

    void appendExportFilters(
        TSortedFilterList& _rFilterMatcher,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilterManager >& _rFilterManager,
        ::rtl::OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl
        );

    //--------------------------------------------------------------------
    /** adds the given filters to the filter manager.
        <p>To be used when opening generic files.</p>
    */
    void appendFiltersForOpen(
        TSortedFilterList& _rFilterMatcher,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilterManager >& _rFilterManager,
        ::rtl::OUString& /* [out] */ _rFirstNonEmpty,
        FileDialogHelper_Impl& _rFileDlgImpl
    );

    //--------------------------------------------------------------------
    /** adds the given extension to the display text.
        <p>To be used when opening or save generic files.</p>
    */
    ::rtl::OUString addExtension(
        const ::rtl::OUString& _rDisplayText,
        const ::rtl::OUString& _rExtension,
        sal_Bool _bForOpen,
        FileDialogHelper_Impl& _rFileDlgImpl
    );

//........................................................................
}   // namespace sfx2
//........................................................................

#endif // SFX2_FILTERGROUPING_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
