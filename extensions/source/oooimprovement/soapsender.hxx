/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef EXTENSIONS_OOOIMPROVEMENT_SOAPSENDER_HXX
#define EXTENSIONS_OOOIMPROVEMENT_SOAPSENDER_HXX

#include "soaprequest.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URL.hpp>
#include <rtl/ustring.hxx>


namespace oooimprovement
{
    #ifdef css
        #error css defined globally
    #endif
    #define css ::com::sun::star
    class SoapSender
    {
        public:
            SoapSender(
                const css::uno::Reference< css::lang::XMultiServiceFactory> sf,
                const ::rtl::OUString& url);
            void send(const SoapRequest& request) const;
        private:
            const css::uno::Reference< css::lang::XMultiServiceFactory> m_ServiceFactory;
            const ::rtl::OUString m_Url;
    };
    #undef css
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
