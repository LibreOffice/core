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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AGROUP_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AGROUP_HXX

#include <connectivity/sdbcx/VGroup.hxx>
#include "ado/Awrapadox.hxx"

namespace connectivity
{
    namespace ado
    {
        typedef sdbcx::OGroup OGroup_ADO;
        class OCatalog;

        class OAdoGroup :   public OGroup_ADO
        {
            WpADOGroup      m_aGroup;
            OCatalog*       m_pCatalog;

            sal_Int32       MapRight(RightsEnum _eNum);
            RightsEnum      Map2Right(sal_Int32 _eNum);
            ObjectTypeEnum  MapObjectType(sal_Int32 _ObjType);
        protected:
            virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue,sal_Int32 nHandle) const;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const css::uno::Any& rValue)throw (css::uno::Exception);

        public:
            virtual void refreshUsers();
        public:
            OAdoGroup(OCatalog* _pParent,sal_Bool _bCase, ADOGroup* _pGroup=NULL);
            OAdoGroup(OCatalog* _pParent,sal_Bool _bCase, const OUString& _Name);

            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException);
            static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(css::sdbc::SQLException, css::uno::RuntimeException);

            WpADOGroup getImpl() const { return m_aGroup; }
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AGROUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
