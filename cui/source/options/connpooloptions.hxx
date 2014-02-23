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

    //= ConnectionPoolOptionsPage

    class DriverListControl;
    class ConnectionPoolOptionsPage : public SfxTabPage
    {
        using TabPage::ActivatePage;

    protected:
        CheckBox*               m_pEnablePooling;
        FixedText*              m_pDriversLabel;
        DriverListControl*      m_pDriverList;
        FixedText*              m_pDriverLabel;
        FixedText*              m_pDriver;
        CheckBox*               m_pDriverPoolingEnabled;
        FixedText*              m_pTimeoutLabel;
        NumericField*           m_pTimeout;

    protected:
        ConnectionPoolOptionsPage(Window* _pParent, const SfxItemSet& _rAttrSet);

    public:
        static SfxTabPage*  Create(Window* _pParent, const SfxItemSet& _rAttrSet);

    protected:
        virtual bool        Notify( NotifyEvent& _rNEvt );

        virtual sal_Bool    FillItemSet(SfxItemSet& _rSet);
        virtual void        Reset(const SfxItemSet& _rSet);
        virtual void        ActivatePage( const SfxItemSet& _rSet);

    protected:
        DECL_LINK( OnEnabledDisabled, const CheckBox* );
        DECL_LINK( OnDriverRowChanged, const void* );

        void implInitControls(const SfxItemSet& _rSet, sal_Bool _bFromReset);

        void commitTimeoutField();
    };

} // namespace offapp

#endif // INCLUDED_CUI_SOURCE_OPTIONS_CONNPOOLOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
