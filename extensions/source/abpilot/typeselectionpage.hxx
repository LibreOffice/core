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

#ifndef INCLUDED_EXTENSIONS_SOURCE_ABPILOT_TYPESELECTIONPAGE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_ABPILOT_TYPESELECTIONPAGE_HXX

#include "abspage.hxx"
#include "addresssettings.hxx"
#include <vcl/edit.hxx>
#include <vcl/vclptr.hxx>


namespace abp
{

    class TypeSelectionPage : public AddressBookSourcePage
    {
    protected:
        VclPtr<RadioButton> m_pEvolution;
        VclPtr<RadioButton> m_pEvolutionGroupwise;
        VclPtr<RadioButton> m_pEvolutionLdap;
        VclPtr<RadioButton> m_pMORK;
        VclPtr<RadioButton> m_pThunderbird;
        VclPtr<RadioButton> m_pKab;
        VclPtr<RadioButton> m_pMacab;
        VclPtr<RadioButton> m_pLDAP;
        VclPtr<RadioButton> m_pOutlook;
        VclPtr<RadioButton> m_pOE;
        VclPtr<RadioButton> m_pOther;

        struct ButtonItem {
            VclPtr<RadioButton> m_pItem;
            AddressSourceType m_eType;
            bool         m_bVisible;

            ButtonItem( RadioButton *pItem,
                        AddressSourceType eType,
                        bool         bVisible ) :
                    m_pItem( pItem ),
                    m_eType( eType ),
                    m_bVisible( bVisible )
            {}
        };

        ::std::vector< ButtonItem > m_aAllTypes;

    public:
        explicit TypeSelectionPage( OAddessBookSourcePilot* _pParent );
        virtual ~TypeSelectionPage();
        virtual void        dispose() SAL_OVERRIDE;

        // retrieves the currently selected type
        AddressSourceType   getSelectedType() const;

    protected:
        // OWizardPage overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;

        // TabDialog overridables
        virtual void        ActivatePage() SAL_OVERRIDE;
        virtual void        DeactivatePage() SAL_OVERRIDE;

        // OImportPage overridables
        virtual bool        canAdvance() const SAL_OVERRIDE;

    private:
        DECL_LINK_TYPED( OnTypeSelected, Button*, void );

        void                selectType( AddressSourceType _eType );
    };


}   // namespace abp


#endif // INCLUDED_EXTENSIONS_SOURCE_ABPILOT_TYPESELECTIONPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
