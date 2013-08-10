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
