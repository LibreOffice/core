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

#include <sal/config.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/security/XSanExtension.hpp>
#include <com/sun/star/security/CertAltNameEntry.hpp>
#include <certificateextension_certextn.hxx>
#include <vector>

class SanExtensionImpl : public ::cppu::WeakImplHelper<
    css::security::XSanExtension >
{
    private:
        CertificateExtension_CertExtn m_Extn;
        std::vector<css::security::CertAltNameEntry> m_Entries;

        static OString removeOIDFromString( const OString &oid);

    public:
        //Methods from XCertificateExtension
        virtual sal_Bool SAL_CALL isCritical() override
        {
            return m_Extn.m_critical;
        }

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getExtensionId() override
        {
            return m_Extn.m_xExtnId;
        }

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getExtensionValue() override
        {
            return m_Extn.m_xExtnValue;
        }

        void setCertExtn(unsigned char const * value, unsigned int vlen, unsigned char const * id, unsigned int idlen, bool critical)
        {
            m_Extn.setCertExtn(value, vlen, id, idlen, critical);
        }

        //Methods from XSanExtension

        virtual css::uno::Sequence< css::security::CertAltNameEntry > SAL_CALL getAlternativeNames() override ;
} ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
