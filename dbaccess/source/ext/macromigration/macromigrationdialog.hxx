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

#ifndef INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_MACROMIGRATIONDIALOG_HXX
#define INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_MACROMIGRATIONDIALOG_HXX

#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>

#include <svtools/roadmapwizard.hxx>

#include <memory>

namespace dbmm
{

    // MacroMigrationDialog
    struct MacroMigrationDialog_Data;
    typedef ::svt::RoadmapWizard    MacroMigrationDialog_Base;
    class MacroMigrationDialog : public MacroMigrationDialog_Base
    {
    public:
        MacroMigrationDialog(
            vcl::Window* _pParent,
            const css::uno::Reference< css::uno::XComponentContext >& _rContext,
            const css::uno::Reference< css::sdb::XOfficeDatabaseDocument >& _rxDocument
        );

        const css::uno::Reference< css::uno::XComponentContext >&
                getComponentContext() const;
        const css::uno::Reference< css::sdb::XOfficeDatabaseDocument >&
                getDocument() const;

    protected:
        // OWizardMachine overridables
        virtual void            enterState( WizardState _nState ) override;
        virtual bool            prepareLeaveCurrentState( CommitPageReason _eReason ) override;
        virtual bool            leaveState( WizardState _nState ) override;
        virtual WizardState     determineNextState( WizardState _nCurrentState ) const override;
        virtual bool            onFinish() override;

        // Dialog overridables
        virtual bool    Close() override;
        virtual short   Execute() override;

    private:
        void    impl_showCloseDocsError( bool _bShow );
        bool    impl_closeSubDocs_nothrow();
        bool    impl_backupDocument_nothrow() const;
        void    impl_reloadDocument_nothrow( bool _bMigrationSuccess );

    private:
        DECL_LINK_TYPED( OnStartMigration, void*, void );

    private:
        ::std::unique_ptr< MacroMigrationDialog_Data >    m_pData;
    };

} // namespace dbmm

#endif // INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_MACROMIGRATIONDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
