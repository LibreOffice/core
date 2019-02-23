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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VDESCRIPTOR_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VDESCRIPTOR_HXX

#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/stl_types.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
    namespace sdbcx
    {

        // = ODescriptor

        typedef ::comphelper::OPropertyContainer ODescriptor_PBASE;
        class OOO_DLLPUBLIC_DBTOOLS ODescriptor
                    :public ODescriptor_PBASE
                    ,public css::lang::XUnoTunnel
        {
        protected:
            OUString         m_Name;

            /** helper for derived classes to implement OPropertyArrayUsageHelper::createArrayHelper

                This method just calls describeProperties, and flags all properties as READONLY if and
                only if we do *not* act as descriptor, but as final object.

                @seealso    isNew
            */
            ::cppu::IPropertyArrayHelper*   doCreateArrayHelper() const;

        private:
            comphelper::UStringMixEqual m_aCase;
            bool                        m_bNew;

        public:
            ODescriptor(::cppu::OBroadcastHelper& _rBHelper, bool _bCase, bool _bNew = false);

            virtual ~ODescriptor() override;

            bool     isNew()  const         { return m_bNew;    }
            void     setNew(bool _bNew);

            bool     isCaseSensitive() const { return m_aCase.isCaseSensitive(); }

            virtual void construct();

            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            /// @throws css::uno::RuntimeException
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  );

            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            static ODescriptor* getImplementation( const css::uno::Reference< css::uno::XInterface >& _rxSomeComp );
            // retrieves the ODescriptor implementation of a given UNO component, and returns its ->isNew flag
            static bool isNew( const css::uno::Reference< css::uno::XInterface >& _rxDescriptor );
        };
    }

}
#endif // INCLUDED_CONNECTIVITY_SDBCX_VDESCRIPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
