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

#include <sdbcx/VGroup.hxx>
#include <ado/Awrapadox.hxx>

namespace connectivity::ado
{
        typedef sdbcx::OGroup OGroup_ADO;
        class OCatalog;

        class OAdoGroup :   public OGroup_ADO
        {
            WpADOGroup      m_aGroup;
            OCatalog*       m_pCatalog;

            static sal_Int32 MapRight(RightsEnum _eNum);
            static RightsEnum Map2Right(sal_Int32 _eNum);
            static ObjectTypeEnum MapObjectType(sal_Int32 ObjType);
        protected:
            virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue,sal_Int32 nHandle) const override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const css::uno::Any& rValue) override;

        public:
            virtual void refreshUsers() override;
        public:
            OAdoGroup(OCatalog* _pParent,bool _bCase, ADOGroup* _pGroup=nullptr);
            OAdoGroup(OCatalog* _pParent,bool _bCase, const OUString& Name);

            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) override;
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) override;
            virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) override;
            virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) override;

            WpADOGroup getImpl() const { return m_aGroup; }
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
