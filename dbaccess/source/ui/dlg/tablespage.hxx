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

#ifndef _DBAUI_TABLESPAGE_HXX_
#define _DBAUI_TABLESPAGE_HXX_

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _COM_SUN_STAR_I18N_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _DBAUI_TABLETREE_HXX_
#include "tabletree.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif


//.........................................................................
namespace dbaui
{
//.........................................................................

    //========================================================================
    //= OTableSubscriptionPage
    //========================================================================
    class OTableSubscriptionDialog;
    class OTableSubscriptionPage
            :public OGenericAdministrationPage
    {
    private:
        FixedLine               m_aTables;
        OTableTreeListBox       m_aTablesList;
        FixedText               m_aExplanation;

        ::rtl::OUString         m_sCatalogSeparator;
        sal_Bool                m_bCheckedAll : 1;
        sal_Bool                m_bCatalogAtStart : 1;

        ::osl::Mutex            m_aNotifierMutex;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                m_xCurrentConnection;   /// valid as long as the page is active
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >
                                m_xCollator;
        OTableSubscriptionDialog* m_pTablesDlg;

    public:
        virtual sal_Bool            FillItemSet(SfxItemSet& _rCoreAttrs);
        virtual int             DeactivatePage(SfxItemSet* _pSet);
        using OGenericAdministrationPage::DeactivatePage;

        virtual void            StateChanged( StateChangedType nStateChange );
        virtual void            DataChanged( const DataChangedEvent& rDCEvt );


        /** will be called when the controls need to be resized.
        */
        virtual void            resizeControls(const Size& _rDiff);

        OTableSubscriptionPage( Window* pParent, const SfxItemSet& _rCoreAttrs ,OTableSubscriptionDialog* _pTablesDlg);
        virtual ~OTableSubscriptionPage();

    protected:
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

        DECL_LINK( OnTreeEntryCompare, const SvSortData* );
        DECL_LINK( OnTreeEntryChecked, Control* );

    private:


        /** check the tables in <member>m_aTablesList</member> according to <arg>_rTables</arg>
        */
        void implCheckTables(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTables);

        /// returns the next sibling, if not available, the next sibling of the parent, a.s.o.
        SvLBoxEntry* implNextSibling(SvLBoxEntry* _pEntry) const;

        /** return the current selection in <member>m_aTablesList</member>
        */
        ::com::sun::star::uno::Sequence< ::rtl::OUString > collectDetailedSelection() const;

        /// (un)check all entries
        void CheckAll( sal_Bool bCheck = sal_True );

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        // checks the tables according to the filter given
        // in oppsofite to implCheckTables, this method handles the case of an empty sequence, too ...
        void implCompleteTablesCheck( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter );
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_TABLESPAGE_HXX_

