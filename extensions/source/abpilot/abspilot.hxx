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

#ifndef EXTENSIONS_ABSPILOT_HXX
#define EXTENSIONS_ABSPILOT_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                m_xORB;
        AddressSettings         m_aSettings;

        ODataSource             m_aNewDataSource;
        AddressSourceType       m_eNewDataSourceType;

    public:
        /// ctor
        OAddessBookSourcePilot(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        /// get the service factory which was used to create the dialog
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
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
        virtual String              getStateDisplayName( WizardState _nState ) const;

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
