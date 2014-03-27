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

#ifndef EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX
#define EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX

#include "abspage.hxx"


namespace abp
{



    //= AdminDialogInvokationPage

    class AdminDialogInvokationPage : public AddressBookSourcePage
    {
    protected:
        FixedText       m_aExplanation;
        PushButton      m_aInvokeAdminDialog;
        FixedText       m_aErrorMessage;

        sal_Bool        m_bSuccessfullyExecutedDialog;

    public:
        AdminDialogInvokationPage( OAddessBookSourcePilot* _pParent );

    protected:
        // TabDialog overridables
        virtual void        ActivatePage() SAL_OVERRIDE;
        virtual void        initializePage() SAL_OVERRIDE;

        // OImportPage overridables
        virtual bool        canAdvance() const SAL_OVERRIDE;

    private:
        DECL_LINK( OnInvokeAdminDialog, void* );

        void implTryConnect();
        void implUpdateErrorMessage();
    };


}   // namespace abp


#endif // EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
