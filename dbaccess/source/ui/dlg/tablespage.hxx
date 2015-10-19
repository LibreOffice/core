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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_TABLESPAGE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_TABLESPAGE_HXX

#include "adminpages.hxx"
#include <com/sun/star/i18n/XCollator.hpp>
#include <osl/mutex.hxx>
#include <vcl/layout.hxx>
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
        VclPtr<VclContainer>           m_pTables;
        VclPtr<OTableTreeListBox>      m_pTablesList;

        OUString                       m_sCatalogSeparator;
        bool                           m_bCatalogAtStart : 1;

        css::uno::Reference< css::sdbc::XConnection >
                                       m_xCurrentConnection;   /// valid as long as the page is active
        css::uno::Reference< css::i18n::XCollator >
                                       m_xCollator;
        VclPtr<OTableSubscriptionDialog> m_pTablesDlg;

    public:
        virtual bool            FillItemSet(SfxItemSet* _rCoreAttrs) override;
        virtual sfxpg           DeactivatePage(SfxItemSet* _pSet) override;
        using OGenericAdministrationPage::DeactivatePage;

        virtual void            StateChanged( StateChangedType nStateChange ) override;
        virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

        OTableSubscriptionPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs ,OTableSubscriptionDialog* _pTablesDlg);
        virtual ~OTableSubscriptionPage();
        virtual void dispose() override;

    protected:
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) override;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) override;

        DECL_LINK_TYPED( OnTreeEntryCompare, const SvSortData&, sal_Int32 );
        DECL_LINK_TYPED( OnTreeEntryChecked, void*, void );
        DECL_LINK_TYPED( OnTreeEntryButtonChecked, SvTreeListBox*, void );

    private:

        /** check the tables in <member>m_aTablesList</member> according to <arg>_rTables</arg>
        */
        void implCheckTables(const css::uno::Sequence< OUString >& _rTables);

        /// returns the next sibling, if not available, the next sibling of the parent, a.s.o.
        SvTreeListEntry* implNextSibling(SvTreeListEntry* _pEntry) const;

        /** return the current selection in <member>m_aTablesList</member>
        */
        css::uno::Sequence< OUString > collectDetailedSelection() const;

        /// (un)check all entries
        void CheckAll( bool bCheck = true );

        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        // checks the tables according to the filter given
        // in oppsofite to implCheckTables, this method handles the case of an empty sequence, too ...
        void implCompleteTablesCheck( const css::uno::Sequence< OUString >& _rTableFilter );
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_TABLESPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
