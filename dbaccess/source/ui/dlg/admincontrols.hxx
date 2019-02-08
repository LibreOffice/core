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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_ADMINCONTROLS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_ADMINCONTROLS_HXX

#include "adminpages.hxx"

#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

namespace dbaui
{

    // MySQLNativeSettings
    class MySQLNativeSettings : public TabPage
    {
    private:
        VclPtr<FixedText>           m_pDatabaseNameLabel;
        VclPtr<Edit>                m_pDatabaseName;
        VclPtr<RadioButton>         m_pHostPortRadio;
        VclPtr<RadioButton>         m_pSocketRadio;
        VclPtr<RadioButton>         m_pNamedPipeRadio;
        VclPtr<FixedText>           m_pHostNameLabel;
        VclPtr<Edit>                m_pHostName;
        VclPtr<FixedText>           m_pPortLabel;
        VclPtr<NumericField>        m_pPort;
        VclPtr<FixedText>           m_pDefaultPort;
        VclPtr<Edit>                m_pSocket;
        VclPtr<Edit>                m_pNamedPipe;
        Link<void*,void>            m_aControlModificationLink;
        OUString                    m_sHostNameUserText;

        DECL_LINK(RadioToggleHdl, RadioButton&, void);
        DECL_LINK(EditModifyHdl, Edit&, void);

    public:
        MySQLNativeSettings( vcl::Window& _rParent, const Link<void*,void>& _rControlModificationLink );
        virtual ~MySQLNativeSettings() override;
        virtual void dispose() override;
        void fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList );
        void fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList );

        bool FillItemSet( SfxItemSet* _rCoreAttrs );
        void implInitControls( const SfxItemSet& _rSet );

        bool canAdvance() const;
    };

    class DBMySQLNativeSettings
    {
    private:
        std::unique_ptr<weld::Builder> m_xBuilder;
        std::unique_ptr<weld::Widget> m_xContainer;
        std::unique_ptr<weld::Label> m_xDatabaseNameLabel;
        std::unique_ptr<weld::Entry> m_xDatabaseName;
        std::unique_ptr<weld::RadioButton> m_xHostPortRadio;
        std::unique_ptr<weld::RadioButton> m_xSocketRadio;
        std::unique_ptr<weld::RadioButton> m_xNamedPipeRadio;
        std::unique_ptr<weld::Label> m_xHostNameLabel;
        std::unique_ptr<weld::Entry> m_xHostName;
        std::unique_ptr<weld::Label> m_xPortLabel;
        std::unique_ptr<weld::SpinButton> m_xPort;
        std::unique_ptr<weld::Label> m_xDefaultPort;
        std::unique_ptr<weld::Entry> m_xSocket;
        std::unique_ptr<weld::Entry> m_xNamedPipe;
        Link<void*,void> m_aControlModificationLink;
        DECL_LINK(RadioToggleHdl, weld::ToggleButton&, void);
        DECL_LINK(SpinModifyHdl, weld::SpinButton&, void);
        DECL_LINK(EditModifyHdl, weld::Entry&, void);

    public:
        DBMySQLNativeSettings(weld::Widget* pParent, const Link<void*,void>& rControlModificationLink);
        void fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList );
        void fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList );

        bool FillItemSet( SfxItemSet* rCoreAttrs );
        void implInitControls( const SfxItemSet& _rSet );
    };

} // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_ADMINCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
