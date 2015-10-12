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

#ifndef INCLUDED_EXTENSIONS_SOURCE_ABPILOT_ABPFINALPAGE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_ABPILOT_ABPFINALPAGE_HXX

#include "abspage.hxx"
#include "abptypes.hxx"

#include <svtools/urlcontrol.hxx>
#include <svx/databaselocationinput.hxx>
#include <vcl/edit.hxx>
#include <vcl/vclptr.hxx>


namespace abp
{

    class FinalPage : public AddressBookSourcePage
    {
    protected:
        VclPtr< ::svt::OFileURLControl>  m_pLocation;
        VclPtr<PushButton>       m_pBrowse;
        VclPtr<CheckBox>         m_pRegisterName;
        VclPtr<CheckBox>         m_pEmbed;
        VclPtr<FixedText>        m_pNameLabel;
        VclPtr<FixedText>        m_pLocationLabel;
        VclPtr<Edit>             m_pName;
        VclPtr<FixedText>        m_pDuplicateNameError;

        svx::DatabaseLocationInputController*
                        m_pLocationController;

        StringBag       m_aInvalidDataSourceNames;

    public:
        explicit FinalPage(OAddessBookSourcePilot* _pParent);
        virtual ~FinalPage();
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
        DECL_LINK( OnNameModified, Edit* );
        DECL_LINK_TYPED(OnRegister, Button*, void);
        DECL_LINK_TYPED(OnEmbed, Button*, void);

        bool    isValidName() const;
        void        implCheckName();
        void        setFields();
    };


}   // namespace abp


#endif // INCLUDED_EXTENSIONS_SOURCE_ABPILOT_ABPFINALPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
