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

#ifndef _DBAUI_TABLESPAGE_HXX_
#define _DBAUI_TABLESPAGE_HXX_

#include "adminpages.hxx"
#include <com/sun/star/i18n/XCollator.hpp>
#include <osl/mutex.hxx>
#include <comphelper/stl_types.hxx>
#include <vcl/fixed.hxx>
#include "tabletree.hxx"
#include <com/sun/star/sdbc/XConnection.hpp>

namespace dbaui
{

    // OTableSubscriptionPage
    class OTableSubscriptionDialog;
    class OTableSubscriptionPage
            :public OGenericAdministrationPage
    {
    private:
        FixedLine               m_aTables;
        OTableTreeListBox       m_aTablesList;
        FixedText               m_aExplanation;

        OUString         m_sCatalogSeparator;
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
        void implCheckTables(const ::com::sun::star::uno::Sequence< OUString >& _rTables);

        /// returns the next sibling, if not available, the next sibling of the parent, a.s.o.
        SvTreeListEntry* implNextSibling(SvTreeListEntry* _pEntry) const;

        /** return the current selection in <member>m_aTablesList</member>
        */
        ::com::sun::star::uno::Sequence< OUString > collectDetailedSelection() const;

        /// (un)check all entries
        void CheckAll( sal_Bool bCheck = sal_True );

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        // checks the tables according to the filter given
        // in oppsofite to implCheckTables, this method handles the case of an empty sequence, too ...
        void implCompleteTablesCheck( const ::com::sun::star::uno::Sequence< OUString >& _rTableFilter );
    };

}   // namespace dbaui

#endif // _DBAUI_TABLESPAGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
