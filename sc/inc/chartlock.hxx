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

#ifndef SC_CHARTLOCK_HXX
#define SC_CHARTLOCK_HXX


#include <vcl/timer.hxx>

#include <cppuhelper/weakref.hxx>
#include <com/sun/star/frame/XModel.hpp>

#include <memory>

class ScDocument;

/** All current charts in the calc will be locked in constructor and unlocked in destructor.
*/
class ScChartLockGuard
{
public:
                    ScChartLockGuard( ScDocument* pDoc );
    virtual         ~ScChartLockGuard();

    void            AlsoLockThisChart( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xModel );

private:
    std::vector< ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XModel > > maChartModels;

    ScChartLockGuard();
    ScChartLockGuard( const ScChartLockGuard& );
};

/** Use this to lock all charts in the calc for a little time.
    They will unlock automatically unless you call StartOrContinueLocking() again.
*/
class ScTemporaryChartLock
{
public:
                    ScTemporaryChartLock( ScDocument* pDoc );
    virtual         ~ScTemporaryChartLock();

    void            StartOrContinueLocking();
    void            StopLocking();
    void            AlsoLockThisChart( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xModel );

private:
    ScDocument*                         mpDoc;
    Timer                               maTimer;
    std::auto_ptr< ScChartLockGuard >   mapScChartLockGuard;

    DECL_LINK( TimeoutHdl, Timer* );

    ScTemporaryChartLock();
    ScTemporaryChartLock( const ScTemporaryChartLock& );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
