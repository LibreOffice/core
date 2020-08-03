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

#include "subcomponents.hxx"

#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace dbaccess
{

    // SubComponentRecovery
    class SubComponentRecovery
    {
    public:
        SubComponentRecovery(
                const css::uno::Reference< css::uno::XComponentContext >& i_rContext,
                const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& i_rController,
                const css::uno::Reference< css::lang::XComponent >& i_rComponent )
            :m_rContext( i_rContext )
            ,m_xDocumentUI( i_rController, css::uno::UNO_SET_THROW )
            ,m_xComponent( i_rComponent )
            ,m_eType( UNKNOWN )
            ,m_aCompDesc()
        {
            impl_identifyComponent_throw();
        }

        SubComponentRecovery(
                const css::uno::Reference< css::uno::XComponentContext >& i_rContext,
                const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& i_rController,
                const SubComponentType i_eType )
            :m_rContext( i_rContext )
            ,m_xDocumentUI( i_rController, css::uno::UNO_SET_THROW )
            ,m_xComponent()
            ,m_eType( i_eType )
            ,m_aCompDesc()
        {
        }

        // only to be used after being constructed with a component
        void    saveToRecoveryStorage(
                    const css::uno::Reference< css::embed::XStorage >& i_rRecoveryStorage,
                    MapCompTypeToCompDescs& io_mapCompDescs
                );

        // only to be used after being constructed with a type
        css::uno::Reference< css::lang::XComponent >
                recoverFromStorage(
                    const css::uno::Reference< css::embed::XStorage >& i_rRecoveryStorage,
                    const OUString& i_rComponentName,
                    const bool i_bForEditing
                );

        static OUString getComponentsStorageName( const SubComponentType i_eType );

    private:
        void    impl_saveSubDocument_throw(
                    const css::uno::Reference< css::embed::XStorage >& i_rObjectStorage
                );

        void    impl_saveQueryDesign_throw(
                    const css::uno::Reference< css::embed::XStorage >& i_rObjectStorage
                );

        css::uno::Reference< css::lang::XComponent >
                impl_recoverSubDocument_throw(
                    const css::uno::Reference< css::embed::XStorage >& i_rRecoveryStorage,
                    const OUString& i_rComponentName,
                    const bool i_bForEditing
                );

        css::uno::Reference< css::lang::XComponent >
                impl_recoverQueryDesign_throw(
                    const css::uno::Reference< css::embed::XStorage >& i_rRecoveryStorage,
                    const OUString& i_rComponentName,
                    const bool i_bForEditing
                );

        void    impl_identifyComponent_throw();

    private:
        const css::uno::Reference< css::uno::XComponentContext >&
                                                m_rContext;
        css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >
                                                m_xDocumentUI;
        const css::uno::Reference< css::lang::XComponent >
                                                m_xComponent;
        SubComponentType                        m_eType;
        SubComponentDescriptor                  m_aCompDesc;
    };

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
