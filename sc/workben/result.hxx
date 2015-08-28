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

#ifndef INCLUDED_SC_WORKBEN_RESULT_HXX
#define INCLUDED_SC_WORKBEN_RESULT_HXX

#include <vcl/timer.hxx>
#include <rtl/ustring.hxx>
#include <boost/ptr_container/ptr_vector.hpp>

#include <com/sun/star/sheet/XVolatileResult.hpp>

#include <cppuhelper/implbase.hxx>

typedef ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >* XResultListenerPtr;
typedef boost::ptr_vector<XResultListenerPtr> XResultListenerArr_Impl;

class ScAddInResult : public cppu::WeakImplHelper<
                                com::sun::star::sheet::XVolatileResult>
{
private:
    String                  aArg;
    long                    nTickCount;
    XResultListenerArr_Impl aListeners;
    Timer                   aTimer;

    DECL_LINK( TimeoutHdl, Timer* );

    void                    NewValue();

public:
                            ScAddInResult(const String& rStr);
    virtual                 ~ScAddInResult();

                            // XVolatileResult
    virtual void SAL_CALL addResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
