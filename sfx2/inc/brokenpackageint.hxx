/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brokenpackageint.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:46:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_BROKENPACKAGEREQUEST_HPP_
#include <com/sun/star/document/BrokenPackageRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#endif

#ifndef __FRAMEWORK_DISPATCH_INTERACTION_HXX_
#include <framework/interaction.hxx>
#endif

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

