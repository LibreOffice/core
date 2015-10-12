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

#include <osl/diagnose.h>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <comphelper/proparrhlp.hxx>
#include <connectivity/CommonTools.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/container/XNamed.hpp>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/implbase.hxx>

namespace connectivity
{
    namespace sdbcx
    {

        typedef ::cppu::WeakImplHelper< ::com::sun::star::lang::XServiceInfo,
                                        ::com::sun::star::container::XNamed> OView_BASE;



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
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >       m_xMetaData;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

        public:
            DECLARE_SERVICE_INFO();

            OView(bool _bCase,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData);
            OView(  bool _bCase,
                    const OUString& _rName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData,
                    sal_Int32 _nCheckOption = 0,
                    const OUString& _rCommand = OUString(),
                    const OUString& _rSchemaName = OUString(),
                    const OUString& _rCatalogName = OUString());
            virtual ~OView();

            // ODescriptor
            virtual void construct() override;

            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XNamed
            virtual OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setName( const OUString& ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
