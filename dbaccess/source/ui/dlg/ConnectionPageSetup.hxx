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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_CONNECTIONPAGESETUP_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_CONNECTIONPAGESETUP_HXX
#include "ConnectionHelper.hxx"

#include "adminpages.hxx"
#include <ucbhelper/content.hxx>
#include "curledit.hxx"

#include <vcl/field.hxx>

namespace dbaui
{

    // OConnectionTabPageSetup

    /** implements the connection page of the data source properties dialog.
    */
    class OConnectionTabPageSetup : public OConnectionHelper
    {
        bool            m_bUserGrabFocus : 1;
    protected:

        FixedText           *m_pHelpText;
        FixedText           *m_pHeaderText;

        // called when the test connection button was clicked
        DECL_LINK(OnEditModified,Edit*);

    public:
        static  OGenericAdministrationPage* CreateDbaseTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateMSAccessTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateADOTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateODBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateUserDefinedTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) SAL_OVERRIDE;

        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;

        inline void enableConnectionURL() { m_pConnectionURL->SetReadOnly(false); }
        inline void disableConnectionURL() { m_pConnectionURL->SetReadOnly(); }

        /** changes the connection URL.
            <p>The new URL must be of the type which is currently selected, only the parts which do not
            affect the type may be changed (compared to the previous URL).</p>
        */
        void     changeConnectionURL( const OUString& _rNewDSN );
        OUString getConnectionURL( ) const;

    protected:
        OConnectionTabPageSetup(vcl::Window* pParent, const OString& _rId, const OUString& _rUIXMLDescription, const SfxItemSet& _rCoreAttrs, sal_uInt16 _nHelpTextResId, sal_uInt16 _nHeaderResId, sal_uInt16 _nUrlResId);
        virtual bool checkTestConnection() SAL_OVERRIDE;
            // nControlFlags is a combination of the CBTP_xxx-constants
        virtual ~OConnectionTabPageSetup();
    };

}   // namespace dbaui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
