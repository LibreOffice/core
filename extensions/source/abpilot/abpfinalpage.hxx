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

#ifndef EXTENSIONS_ABP_ABPFINALPAGE_HXX
#define EXTENSIONS_ABP_ABPFINALPAGE_HXX

#include "abspage.hxx"
#include "abptypes.hxx"

#include <svtools/urlcontrol.hxx>
#include <svx/databaselocationinput.hxx>
#include <vcl/edit.hxx>


namespace abp
{



    //= FinalPage

    class FinalPage : public AddressBookSourcePage
    {
    protected:
        FixedText       m_aExplanation;
        FixedText       m_aLocationLabel;
        ::svt::OFileURLControl  m_aLocation;
        PushButton      m_aBrowse;
        CheckBox        m_aRegisterName;
        FixedText       m_aNameLabel;
        Edit            m_aName;
        FixedText       m_aDuplicateNameError;

        ::svx::DatabaseLocationInputController
                        m_aLocationController;

        StringBag       m_aInvalidDataSourceNames;

    public:
        FinalPage( OAddessBookSourcePilot* _pParent );

    protected:
        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

        // TabDialog overridables
        virtual void        ActivatePage();
        virtual void        DeactivatePage();

        // OImportPage overridables
        virtual bool        canAdvance() const;

    private:
        DECL_LINK( OnNameModified, Edit* );
        DECL_LINK(OnRegister, void *);

        sal_Bool    isValidName() const;
        void        implCheckName();
        void        setFields();
    };


}   // namespace abp


#endif // EXTENSIONS_ABP_ABPFINALPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
