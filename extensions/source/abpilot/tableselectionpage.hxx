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

#ifndef INCLUDED_EXTENSIONS_SOURCE_ABPILOT_TABLESELECTIONPAGE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_ABPILOT_TABLESELECTIONPAGE_HXX

#include "abspage.hxx"
#include <vcl/lstbox.hxx>
#include <vcl/vclptr.hxx>


namespace abp
{

    class TableSelectionPage : public AddressBookSourcePage
    {
    protected:
        VclPtr<ListBox>        m_pTableList;

    public:
        explicit TableSelectionPage( OAddressBookSourcePilot* _pParent );
        virtual ~TableSelectionPage();
        virtual void dispose() override;
    protected:
        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;

        // TabDialog overridables
        virtual void        ActivatePage() override;
        virtual void        DeactivatePage() override;

        // OImportPage overridables
        virtual bool        canAdvance() const override;

    private:
        DECL_LINK_TYPED( OnTableSelected, ListBox&, void );
        DECL_LINK_TYPED( OnTableDoubleClicked, ListBox&, void );
    };


}   // namespace abp


#endif // INCLUDED_EXTENSIONS_SOURCE_ABPILOT_TABLESELECTIONPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
