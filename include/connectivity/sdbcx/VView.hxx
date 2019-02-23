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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VVIEW_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VVIEW_HXX

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::sdbc { class XDatabaseMetaData; }

namespace connectivity
{
    namespace sdbcx
    {

        typedef ::cppu::WeakImplHelper< css::lang::XServiceInfo,
                                        css::container::XNamed> OView_BASE;


        class OOO_DLLPUBLIC_DBTOOLS OView :
                        public ::comphelper::OMutexAndBroadcastHelper,
                        public OView_BASE,
                        public ::comphelper::OIdPropertyArrayUsageHelper<OView>,
                        public ODescriptor
        {
        protected:
            OUString m_CatalogName;
            OUString m_SchemaName;
            OUString m_Command;
            sal_Int32       m_CheckOption;
            // need for the getName method
            css::uno::Reference< css::sdbc::XDatabaseMetaData >       m_xMetaData;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

        public:
            DECLARE_SERVICE_INFO();

            OView(bool _bCase,const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _xMetaData);
            OView(  bool _bCase,
                    const OUString& _rName,
                    const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _xMetaData,
                    const OUString& _rCommand = OUString(),
                    const OUString& _rSchemaName = OUString(),
                    const OUString& _rCatalogName = OUString());
            virtual ~OView() override;

            // ODescriptor
            virtual void construct() override;

            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
            // XNamed
            virtual OUString SAL_CALL getName(  ) override;
            virtual void SAL_CALL setName( const OUString& ) override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
