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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_CERTIFICATEEXTENSION_XMLSECIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_CERTIFICATEEXTENSION_XMLSECIMPL_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
#include <com/sun/star/security/XCertificateExtension.hpp>
#include "certificateextension_certextn.hxx"

class CertificateExtension_XmlSecImpl : public ::cppu::WeakImplHelper<
    css::security::XCertificateExtension >
{
    private:
        CertificateExtension_CertExtn m_Extn;

    public:
        //Methods from XCertificateExtension
        virtual sal_Bool SAL_CALL isCritical() throw( css::uno::RuntimeException, std::exception ) override
        {
            return m_Extn.m_critical;
        }

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getExtensionId() throw( css::uno::RuntimeException, std::exception ) override
        {
            return m_Extn.m_xExtnId;
        }

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getExtensionValue() throw( css::uno::RuntimeException, std::exception ) override
        {
            return m_Extn.m_xExtnValue;
        }

        void setCertExtn(unsigned char* value, unsigned int vlen, unsigned char* id, unsigned int idlen, bool critical)
        {
            m_Extn.setCertExtn(value, vlen, id, idlen, critical);
        }
};

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_CERTIFICATEEXTENSION_XMLSECIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
