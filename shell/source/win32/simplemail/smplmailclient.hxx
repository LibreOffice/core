/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef _SMPLMAILCLIENT_HXX_
#define _SMPLMAILCLIENT_HXX_

#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/system/XSimpleMailClient.hpp>

namespace css = ::com::sun::star;

class CSmplMailClient : public cppu::WeakImplHelper1<css::system::XSimpleMailClient>
{
public:
    virtual css::uno::Reference<css::system::XSimpleMailMessage> SAL_CALL createSimpleMailMessage()
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL sendSimpleMailMessage(const css::uno::Reference<css::system::XSimpleMailMessage>& xSimpleMailMessage, sal_Int32 aFlag)
        throw (css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException);

private:
    void validateParameter(const css::uno::Reference<css::system::XSimpleMailMessage>& xSimpleMailMessage, sal_Int32 aFlag);
    void assembleCommandLine(const css::uno::Reference<css::system::XSimpleMailMessage>& xSimpleMailMessage, sal_Int32 aFlag, std::vector<rtl::OUString>& rCommandArgs);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
