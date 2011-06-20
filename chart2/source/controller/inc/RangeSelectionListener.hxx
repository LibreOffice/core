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
#ifndef CHART2_RANGESELECTIONLISTENER_HXX
#define CHART2_RANGESELECTIONLISTENER_HXX

#include "ControllerLockGuard.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XRangeSelectionListener.hpp>

namespace chart
{

class RangeSelectionListenerParent
{
public:
    virtual void listeningFinished( const ::rtl::OUString & rNewRange ) = 0;
    virtual void disposingRangeSelection() = 0;
};

// ----------------------------------------

class RangeSelectionListener : public
    ::cppu::WeakImplHelper1<
        ::com::sun::star::sheet::XRangeSelectionListener >
{
public:
    explicit RangeSelectionListener(
        RangeSelectionListenerParent & rParent,
        const ::rtl::OUString & rInitialRange,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >& xModelToLockController );
    virtual ~RangeSelectionListener();

protected:
    // ____ XRangeSelectionListener ____
    virtual void SAL_CALL done( const ::com::sun::star::sheet::RangeSelectionEvent& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL aborted( const ::com::sun::star::sheet::RangeSelectionEvent& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

private:
    RangeSelectionListenerParent & m_rParent;
    ::rtl::OUString m_aRange;
    ControllerLockGuard m_aControllerLockGuard;
};

} //  namespace chart

// CHART2_RANGESELECTIONLISTENER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
