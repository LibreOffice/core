/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


