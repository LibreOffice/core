/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filtergrouping.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:19:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SFX2_FILTERGROUPING_HXX
#define SFX2_FILTERGROUPING_HXX

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

#ifndef _SFX_FILEDLGIMPL_HXX
#include "filedlgimpl.hxx"
#endif

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


