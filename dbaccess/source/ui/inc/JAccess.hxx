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

#include "JoinTableView.hxx"
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vcl/vclptr.hxx>

namespace dbaui
{
    class OJoinTableView;
    typedef ::cppu::ImplHelper1< css::accessibility::XAccessible
                                            > OJoinDesignViewAccess_BASE;
    /** the class OJoinDesignViewAccess represents the accessible object for join views
        like the QueryDesign and the RelationDesign
    */
    class OJoinDesignViewAccess     :   public VCLXAccessibleComponent, public OJoinDesignViewAccess_BASE
    {
        VclPtr<OJoinTableView> m_pTableView; // the window which I should give accessibility to

    public:
        /** OJoinDesignViewAccess needs a valid view
        */
        OJoinDesignViewAccess(  OJoinTableView* _pTableView);

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        virtual OUString SAL_CALL getImplementationName() override;

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;

        void notifyAccessibleEvent(
                    const sal_Int16 _nEventId,
                    const css::uno::Any& _rOldValue,
                    const css::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }

        void clearTableView();
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
