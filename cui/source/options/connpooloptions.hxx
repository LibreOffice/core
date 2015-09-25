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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_CONNPOOLOPTIONS_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_CONNPOOLOPTIONS_HXX

#include <svl/solar.hrc>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svx/databaseregistrationui.hxx>


namespace offapp
{

    struct DriverPooling;
    class DriverListControl;
    class ConnectionPoolOptionsPage : public SfxTabPage
    {
        using TabPage::ActivatePage;

    protected:
        VclPtr<CheckBox>               m_pEnablePooling;
        VclPtr<FixedText>              m_pDriversLabel;
        VclPtr<DriverListControl>      m_pDriverList;
        VclPtr<FixedText>              m_pDriverLabel;
        VclPtr<FixedText>              m_pDriver;
        VclPtr<CheckBox>               m_pDriverPoolingEnabled;
        VclPtr<FixedText>              m_pTimeoutLabel;
        VclPtr<NumericField>           m_pTimeout;

    public:
        ConnectionPoolOptionsPage(vcl::Window* _pParent, const SfxItemSet& _rAttrSet);
        virtual ~ConnectionPoolOptionsPage();
        virtual void dispose() SAL_OVERRIDE;
        static VclPtr<SfxTabPage>  Create(vcl::Window* _pParent, const SfxItemSet* _rAttrSet);

    protected:
        virtual bool        Notify( NotifyEvent& _rNEvt ) SAL_OVERRIDE;

        virtual bool        FillItemSet(SfxItemSet* _rSet) SAL_OVERRIDE;
        virtual void        Reset(const SfxItemSet* _rSet) SAL_OVERRIDE;
        virtual void        ActivatePage( const SfxItemSet& _rSet) SAL_OVERRIDE;

    protected:
        DECL_LINK_TYPED( OnEnabledDisabled, Button*, void );
        DECL_LINK_TYPED( OnDriverRowChanged, const DriverPooling*, void );

        void implInitControls(const SfxItemSet& _rSet, bool _bFromReset);

        void commitTimeoutField();
    };

} // namespace offapp

#endif // INCLUDED_CUI_SOURCE_OPTIONS_CONNPOOLOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
