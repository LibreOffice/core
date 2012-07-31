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

#ifndef SC_RESULT_HXX
#define SC_RESULT_HXX

#include <vcl/timer.hxx>
#include <tools/string.hxx>
#include <boost/ptr_container/ptr_vector.hpp>


#include <com/sun/star/sheet/XVolatileResult.hpp>

#include <cppuhelper/implbase1.hxx> // helper for implementations


//class XResultListenerRef;
typedef ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >* XResultListenerPtr;
typedef boost::ptr_vector<XResultListenerPtr> XResultListenerArr_Impl;


class ScAddInResult : public cppu::WeakImplHelper1<
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

//                          SMART_UNO_DECLARATION( ScAddInResult, UsrObject );

//  virtual BOOL            queryInterface( Uik, XInterfaceRef& );
//  virtual XIdlClassRef    getIdlClass(void);

                            // XVolatileResult
    virtual void SAL_CALL addResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
