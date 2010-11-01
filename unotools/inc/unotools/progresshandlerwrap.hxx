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
#include "unotools/unotoolsdllapi.h"

#ifndef _UTL_PROGRESSHANDLERWRAP_HXX_
#define _UTL_PROGRESSHANDLERWRAP_HXX_
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

namespace utl
{

class UNOTOOLS_DLLPUBLIC ProgressHandlerWrap : public ::cppu::WeakImplHelper1< ::com::sun::star::ucb::XProgressHandler >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > m_xStatusIndicator;

public:
    ProgressHandlerWrap( ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > xSI );

    // XProgressHandler
    virtual void SAL_CALL push( const ::com::sun::star::uno::Any& Status )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL update( const ::com::sun::star::uno::Any& Status )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL pop()
        throw (::com::sun::star::uno::RuntimeException);
};

}   // namespace utl

#endif // _UTL_PROGRESSHANDLERWRAP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
