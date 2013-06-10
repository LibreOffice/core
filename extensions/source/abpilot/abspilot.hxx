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

#ifndef EXTENSIONS_ABSPILOT_HXX
#define EXTENSIONS_ABSPILOT_HXX

#include <svtools/roadmapwizard.hxx>
#include "addresssettings.hxx"
#include <vcl/fixed.hxx>
#include "datasourcehandling.hxx"

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= OAddessBookSourcePilot
    //=====================================================================
    typedef ::svt::RoadmapWizard OAddessBookSourcePilot_Base;
    class OAddessBookSourcePilot : public OAddessBookSourcePilot_Base
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                m_xORB;
        AddressSettings         m_aSettings;

        ODataSource             m_aNewDataSource;
        AddressSourceType       m_eNewDataSourceType;

    public:
        /// ctor
        OAddessBookSourcePilot(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB);
        /// dtor
        ~OAddessBookSourcePilot();

        /// get the service factory which was used to create the dialog
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&
                                getORB() { return m_xORB; }
        AddressSettings&        getSettings() { return m_aSettings; }
        const AddressSettings&  getSettings() const { return m_aSettings; }

        const ODataSource&      getDataSource() const { return m_aNewDataSource; }

        sal_Bool                connectToDataSource( sal_Bool _bForceReConnect );

        void                    travelNext( ) { OAddessBookSourcePilot_Base::travelNext(); }

        /// to be called when the selected type changed
        void                    typeSelectionChanged( AddressSourceType _eType );

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage( WizardState _nState );
        virtual void                enterState( WizardState _nState );
        virtual sal_Bool            prepareLeaveCurrentState( CommitPageReason _eReason );
        virtual sal_Bool            onFinish();

        // RoadmapWizard
        virtual OUString            getStateDisplayName( WizardState _nState ) const;

        virtual sal_Bool    Close();

    private:
        DECL_LINK( OnCancelClicked, void* );

        /** creates a new data source of the type indicated by m_aSettings
            <p>If another data source has been created before, this one is deleted.</p>
        */
        void implCreateDataSource();

        /// does an automatic field mapping (possible for all types except AST_OTHER)
        void implDoAutoFieldMapping();

        /// guesses a default for the table name, if no valid table is selected
        void implDefaultTableName();

        inline sal_Bool needAdminInvokationPage( AddressSourceType _eType ) const
        {
            return  (   ( AST_LDAP == _eType )
                    ||  ( AST_OTHER == _eType )
                    );
        }
        /// check if with the current settings, we would need to invoke he administration dialog for more details about the data source
        inline sal_Bool needAdminInvokationPage() const
        {
            return  needAdminInvokationPage( m_aSettings.eType );
        }

        inline sal_Bool needManualFieldMapping( AddressSourceType _eType ) const
        {
            return  ( AST_OTHER == _eType ) || ( AST_KAB == _eType ) ||
                    ( AST_EVOLUTION == _eType ) || ( AST_EVOLUTION_GROUPWISE == _eType ) ||
                    ( AST_EVOLUTION_LDAP == _eType );
        }
        /// checks if we need a manual (user-guided) field mapping
        inline sal_Bool needManualFieldMapping() const
        {
            return needManualFieldMapping( m_aSettings.eType );
        }

        /// determines whether the given address book type does provide one table only
        inline bool needTableSelection( AddressSourceType _eType ) const
        {
            return  ( AST_LDAP != _eType ) && ( AST_KAB != _eType );
        }

        inline bool needTableSelection() const
        {
            return needTableSelection( m_aSettings.eType );
        }

        void implCleanup();
        void implCommitAll();

        void impl_updateRoadmap( AddressSourceType _eType );
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABSPILOT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
