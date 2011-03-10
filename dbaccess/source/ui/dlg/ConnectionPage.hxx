/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef DBAUI_CONNECTIONPAGE_HXX
#define DBAUI_CONNECTIONPAGE_HXX

#include "ConnectionHelper.hxx"
#include "adminpages.hxx"
#include <ucbhelper/content.hxx>
#include "curledit.hxx"

//.........................................................................
namespace dbaui
{
//.........................................................................

    class IDatabaseSettingsDialog;
    //=========================================================================
    //= OConnectionTabPage
    //=========================================================================

    /** implements the connection page of teh data source properties dialog.
    */
    class OConnectionTabPage : public OConnectionHelper
    {
        sal_Bool            m_bUserGrabFocus : 1;
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
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DETAILPAGES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
