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

#pragma once

#include <vcl/roadmapwizard.hxx>
#include "addresssettings.hxx"
#include "datasourcehandling.hxx"

using vcl::WizardTypes::WizardState;
using vcl::WizardTypes::CommitPageReason;

namespace abp
{
    class OAddressBookSourcePilot final : public vcl::RoadmapWizardMachine
    {
        css::uno::Reference< css::uno::XComponentContext >
                                m_xORB;
        AddressSettings         m_aSettings;

        ODataSource             m_aNewDataSource;
        AddressSourceType       m_eNewDataSourceType;

    public:
        /// ctor
        OAddressBookSourcePilot(
            weld::Window* _pParent,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

        virtual short run() override;

        /// get the service factory which was used to create the dialog
        const css::uno::Reference< css::uno::XComponentContext >&
                                getORB() const { return m_xORB; }
        AddressSettings&        getSettings() { return m_aSettings; }
        const AddressSettings&  getSettings() const { return m_aSettings; }

        const ODataSource&      getDataSource() const { return m_aNewDataSource; }

        bool                connectToDataSource( bool _bForceReConnect );

        void                    travelNext( ) { vcl::RoadmapWizardMachine::travelNext(); }

        /// to be called when the selected type changed
        void                    typeSelectionChanged( AddressSourceType _eType );

    private:
        // OWizardMachine overridables
        virtual std::unique_ptr<BuilderPage> createPage( WizardState _nState ) override;
        virtual void                enterState( WizardState _nState ) override;
        virtual bool                prepareLeaveCurrentState( CommitPageReason _eReason ) override;
        virtual bool                onFinish() override;

        // RoadmapWizardMachine
        virtual OUString            getStateDisplayName( WizardState _nState ) const override;

        /** creates a new data source of the type indicated by m_aSettings
            <p>If another data source has been created before, this one is deleted.</p>
        */
        void implCreateDataSource();

        /// does an automatic field mapping (possible for all types except AST_OTHER)
        void implDoAutoFieldMapping();

        /// guesses a default for the table name, if no valid table is selected
        void implDefaultTableName();

        static bool needAdminInvokationPage( AddressSourceType _eType )
        {
            return  ( AST_OTHER == _eType );
        }
        /// check if with the current settings, we would need to invoke he administration dialog for more details about the data source
        bool needAdminInvokationPage() const
        {
            return  needAdminInvokationPage( m_aSettings.eType );
        }

        static bool needManualFieldMapping( AddressSourceType _eType )
        {
            return  ( AST_OTHER == _eType ) || ( AST_KAB == _eType ) ||
                    ( AST_EVOLUTION == _eType ) || ( AST_EVOLUTION_GROUPWISE == _eType ) ||
                    ( AST_EVOLUTION_LDAP == _eType );
        }
        /// checks if we need a manual (user-guided) field mapping
        bool needManualFieldMapping() const
        {
            return needManualFieldMapping( m_aSettings.eType );
        }

        /// determines whether the given address book type does provide one table only
        static bool needTableSelection( AddressSourceType _eType )
        {
            return  ( AST_KAB != _eType );
        }

        void implCleanup();
        void implCommitAll();

        void impl_updateRoadmap( AddressSourceType _eType );
    };
}   // namespace abp
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
