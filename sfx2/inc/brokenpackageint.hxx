/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: brokenpackageint.hxx,v $
 * $Revision: 1.5 $
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

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <com/sun/star/document/BrokenPackageRequest.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <framework/interaction.hxx>

using namespace ::framework;
typedef ContinuationBase< ::com::sun::star::task::XInteractionApprove > SfxContinuationApprove;
typedef ContinuationBase< ::com::sun::star::task::XInteractionDisapprove > SfxContinuationDisapprove;

class SFX2_DLLPUBLIC RequestPackageReparation : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    ::com::sun::star::uno::Any m_aRequest;

    ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
                > m_lContinuations;

    SfxContinuationApprove* m_pApprove;
    SfxContinuationDisapprove*  m_pDisapprove;

public:
    RequestPackageReparation( ::rtl::OUString aName );

    sal_Bool    isApproved() { return m_pApprove->isSelected(); }

    virtual ::com::sun::star::uno::Any SAL_CALL getRequest()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
            > SAL_CALL getContinuations()
        throw( ::com::sun::star::uno::RuntimeException );
};

class SFX2_DLLPUBLIC NotifyBrokenPackage : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    ::com::sun::star::uno::Any m_aRequest;

    ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
                > m_lContinuations;

    ContinuationAbort*  m_pAbort;

public:
    NotifyBrokenPackage( ::rtl::OUString aName );

    sal_Bool    isAborted() { return m_pAbort->isSelected(); }

    virtual ::com::sun::star::uno::Any SAL_CALL getRequest()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
            > SAL_CALL getContinuations()
        throw( ::com::sun::star::uno::RuntimeException );
};

