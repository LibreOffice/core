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

#pragma once

#include "abspage.hxx"
#include "addresssettings.hxx"
#include <vcl/weld.hxx>

namespace abp
{

    class TypeSelectionPage final : public AddressBookSourcePage
    {
        std::unique_ptr<weld::RadioButton> m_xEvolution;
        std::unique_ptr<weld::RadioButton> m_xEvolutionGroupwise;
        std::unique_ptr<weld::RadioButton> m_xEvolutionLdap;
        std::unique_ptr<weld::RadioButton> m_xThunderbird;
        std::unique_ptr<weld::RadioButton> m_xKab;
        std::unique_ptr<weld::RadioButton> m_xMacab;
        std::unique_ptr<weld::RadioButton> m_xOther;

        struct ButtonItem {
            weld::RadioButton* m_pItem;
            AddressSourceType m_eType;
            bool         m_bVisible;

            ButtonItem( weld::RadioButton *pItem,
                        AddressSourceType eType,
                        bool         bVisible ) :
                    m_pItem( pItem ),
                    m_eType( eType ),
                    m_bVisible( bVisible )
            {}
        };

        std::vector< ButtonItem > m_aAllTypes;

    public:
        explicit TypeSelectionPage(weld::Container* pPage, OAddressBookSourcePilot* pController);
        virtual ~TypeSelectionPage() override;

        // retrieves the currently selected type
        AddressSourceType   getSelectedType() const;

    private:
        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;

        // BuilderPage overridables
        virtual void        Activate() override;
        virtual void        Deactivate() override;

        // OImportPage overridables
        virtual bool        canAdvance() const override;

        DECL_LINK( OnTypeSelected, weld::Button&, void );

        void                selectType( AddressSourceType _eType );
    };


}   // namespace abp
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
