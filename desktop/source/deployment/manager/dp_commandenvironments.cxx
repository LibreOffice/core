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


#include <com/sun/star/deployment/VersionException.hpp>
#include <com/sun/star/deployment/LicenseException.hpp>
#include <com/sun/star/deployment/InstallException.hpp>
#include <com/sun/star/deployment/DependencyException.hpp>
#include <com/sun/star/deployment/PlatformException.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <utility>
#include "dp_commandenvironments.hxx"
#include <osl/diagnose.h>

namespace deployment = com::sun::star::deployment;
namespace task = com::sun::star::task;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;

using ::com::sun::star::uno::Reference;

namespace dp_manager {

BaseCommandEnv::BaseCommandEnv()
{
}

BaseCommandEnv::BaseCommandEnv(
    Reference< task::XInteractionHandler> const & handler)
    : m_forwardHandler(handler)
{
}

BaseCommandEnv::~BaseCommandEnv()
{
}
// XCommandEnvironment

Reference<task::XInteractionHandler> BaseCommandEnv::getInteractionHandler()
{
    return this;
}


Reference<ucb::XProgressHandler> BaseCommandEnv::getProgressHandler()
{
    return this;
}

void BaseCommandEnv::handle(
    Reference< task::XInteractionRequest> const & /*xRequest*/ )
{
}

void BaseCommandEnv::handle_(bool approve,
                             Reference< task::XInteractionRequest> const & xRequest )
{
    if (!approve)
    {
        //not handled so far -> forwarding
        if (m_forwardHandler.is())
            m_forwardHandler->handle(xRequest);
        else
            return; //cannot handle
    }
    else
    {
        // select:
        uno::Sequence< Reference< task::XInteractionContinuation > > conts(
            xRequest->getContinuations() );
        Reference< task::XInteractionContinuation > const * pConts =
            conts.getConstArray();
        sal_Int32 len = conts.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            if (approve) {
                Reference< task::XInteractionApprove > xInteractionApprove(
                    pConts[ pos ], uno::UNO_QUERY );
                if (xInteractionApprove.is()) {
                    xInteractionApprove->select();
                    // don't query again for ongoing continuations:
                    approve = false;
                }
            }
        }
    }

}

// XProgressHandler
void BaseCommandEnv::push( uno::Any const & /*Status*/ )
{
}

void BaseCommandEnv::update( uno::Any const & /*Status */)
{
}

void BaseCommandEnv::pop()
{
}


TmpRepositoryCommandEnv::TmpRepositoryCommandEnv()
{
}

TmpRepositoryCommandEnv::TmpRepositoryCommandEnv(
    css::uno::Reference< css::task::XInteractionHandler> const & handler):
    BaseCommandEnv(handler)
{
}
// XInteractionHandler
void TmpRepositoryCommandEnv::handle(
    Reference< task::XInteractionRequest> const & xRequest )
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::VersionException verExc;
    deployment::LicenseException licExc;
    deployment::InstallException instExc;

    bool approve = false;

    if ((request >>= verExc)
        || (request >>= licExc)
        || (request >>= instExc))
    {
        approve = true;
    }

    handle_(approve, xRequest);
}


LicenseCommandEnv::LicenseCommandEnv(
    css::uno::Reference< css::task::XInteractionHandler> const & handler,
    bool bSuppressLicense,
    OUString repository):
    BaseCommandEnv(handler), m_repository(std::move(repository)),
    m_bSuppressLicense(bSuppressLicense)
{
}
// XInteractionHandler
void LicenseCommandEnv::handle(
    Reference< task::XInteractionRequest> const & xRequest )
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::LicenseException licExc;

    bool approve = false;

    if (request >>= licExc)
    {
        if (m_bSuppressLicense
            || m_repository == "bundled"
            || licExc.AcceptBy == "admin")
        {
            //always approve in bundled case, because we do not support
            //showing licenses anyway.
            //The "admin" already accepted the license when installing the
            // shared extension
            approve = true;
        }
    }

    handle_(approve, xRequest);
}


NoLicenseCommandEnv::NoLicenseCommandEnv(
    css::uno::Reference< css::task::XInteractionHandler> const & handler):
    BaseCommandEnv(handler)
{
}
// XInteractionHandler
void NoLicenseCommandEnv::handle(
    Reference< task::XInteractionRequest> const & xRequest )
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::LicenseException licExc;

    bool approve = false;

    if (request >>= licExc)
    {
        approve = true;
    }
    handle_(approve, xRequest);
}

SilentCheckPrerequisitesCommandEnv::SilentCheckPrerequisitesCommandEnv()
{
}

void SilentCheckPrerequisitesCommandEnv::handle(
       Reference< task::XInteractionRequest> const & xRequest )
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::LicenseException licExc;
    deployment::PlatformException platformExc;
    deployment::DependencyException depExc;

    if (request >>= licExc)
    {
        handle_(true, xRequest); // approve = true
    }
    else if ((request >>= platformExc)
             || (request >>= depExc))
    {
        m_Exception = request;
    }
    else
    {
        m_UnknownException = request;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
