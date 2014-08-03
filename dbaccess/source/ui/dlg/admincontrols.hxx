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

#include <svtools/dialogcontrolling.hxx>

namespace dbaui
{

    // MySQLNativeSettings
    class MySQLNativeSettings : public TabPage
    {
    private:
        FixedText           *m_pDatabaseNameLabel;
        Edit                *m_pDatabaseName;
        RadioButton         *m_pHostPortRadio;
        RadioButton         *m_pSocketRadio;
        RadioButton         *m_pNamedPipeRadio;
        FixedText           *m_pHostNameLabel;
        Edit                *m_pHostName;
        FixedText           *m_pPortLabel;
        NumericField        *m_pPort;
        FixedText           *m_pDefaultPort;
        Edit                *m_pSocket;
        Edit                *m_pNamedPipe;

        ::svt::ControlDependencyManager
                            m_aControlDependencies;

    public:
        MySQLNativeSettings( Window& _rParent, const Link& _rControlModificationLink );
        virtual ~MySQLNativeSettings();

        void fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList );
        void fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList );

        bool FillItemSet( SfxItemSet* _rCoreAttrs );
        void implInitControls( const SfxItemSet& _rSet );

        bool canAdvance() const;
    };

} // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_ADMINCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
