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

#ifndef DBAUI_CONNECTIONPAGE_HXX
#define DBAUI_CONNECTIONPAGE_HXX

#include "ConnectionHelper.hxx"
#include "adminpages.hxx"
#include <ucbhelper/content.hxx>
#include "curledit.hxx"

namespace dbaui
{

    // OConnectionTabPage

    /** implements the connection page of the data source properties dialog.
    */
    class OConnectionTabPage : public OConnectionHelper
    {
    protected:
        // connection
        FixedLine           m_aFL1;
        // user authentification
        FixedLine           m_aFL2;
        FixedText           m_aUserNameLabel;
        Edit                m_aUserName;
        CheckBox            m_aPasswordRequired;

        // jdbc driver
        FixedLine           m_aFL3;
        FixedText           m_aJavaDriverLabel;
        Edit                m_aJavaDriver;
        PushButton          m_aTestJavaDriver;

        // connection test
        PushButton          m_aTestConnection;

        // called when the test connection button was clicked
        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);

    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual sal_Bool        FillItemSet (SfxItemSet& _rCoreAttrs);

        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        inline void enableConnectionURL() { m_aConnectionURL.SetReadOnly(sal_False); }
        inline void disableConnectionURL() { m_aConnectionURL.SetReadOnly(); }

        /** changes the connection URL.
            <p>The new URL must be of the type which is currently selected, only the parts which do not
            affect the type may be changed (compared to the previous URL).</p>
        */
    private:
        OConnectionTabPage(Window* pParent, const SfxItemSet& _rCoreAttrs);
            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~OConnectionTabPage();

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
        /** enables the test connection button, if allowed
        */
        virtual bool checkTestConnection();
    };
}   // namespace dbaui

#endif // _DBAUI_DETAILPAGES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
