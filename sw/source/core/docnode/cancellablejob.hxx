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
#ifndef _CANCELLABLEJOB_HXX
#define _CANCELLABLEJOB_HXX

#include "sal/config.h"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/util/XCancellable.hpp"

#include <rtl/ref.hxx>

class ObservableThread;

class CancellableJob : public ::cppu::WeakImplHelper1<com::sun::star::util::XCancellable>
{
public:
    explicit CancellableJob( const ::rtl::Reference< ObservableThread >& rThread );
    ~CancellableJob() {}

    // ::com::sun::star::util::XCancellable:
    virtual void SAL_CALL cancel() throw (com::sun::star::uno::RuntimeException);

private:
    CancellableJob( CancellableJob& ); // not defined
    void operator =( CancellableJob& ); // not defined

    ::rtl::Reference< ObservableThread > mrThread;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
