/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "com/sun/star/deployment/VersionException.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/deployment/InstallException.hpp"
#include "com/sun/star/deployment/DependencyException.hpp"
#include "com/sun/star/deployment/PlatformException.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "dp_commandenvironments.hxx"

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


Reference<task::XInteractionHandler> BaseCommandEnv::getInteractionHandler()
throw (uno::RuntimeException)
{
    return this;
}


Reference<ucb::XProgressHandler> BaseCommandEnv::getProgressHandler()
throw (uno::RuntimeException)
{
    return this;
}

void BaseCommandEnv::handle(
    Reference< task::XInteractionRequest> const & /*xRequest*/ )
    throw (uno::RuntimeException)
{
}

void BaseCommandEnv::handle_(bool approve, bool abort,
                             Reference< task::XInteractionRequest> const & xRequest )
{
    if (approve == false && abort == false)
    {
        
        if (m_forwardHandler.is())
            m_forwardHandler->handle(xRequest);
        else
            return; 
    }
    else
    {
        
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
                    
                    approve = false;
                }
            }
            else if (abort) {
                Reference< task::XInteractionAbort > xInteractionAbort(
                    pConts[ pos ], uno::UNO_QUERY );
                if (xInteractionAbort.is()) {
                    xInteractionAbort->select();
                    
                    abort = false;
                }
            }
        }
    }

}


void BaseCommandEnv::push( uno::Any const & /*Status*/ )
throw (uno::RuntimeException)
{
}

void BaseCommandEnv::update( uno::Any const & /*Status */)
throw (uno::RuntimeException)
{
}

void BaseCommandEnv::pop() throw (uno::RuntimeException)
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

void TmpRepositoryCommandEnv::handle(
    Reference< task::XInteractionRequest> const & xRequest )
    throw (uno::RuntimeException)
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::VersionException verExc;
    deployment::LicenseException licExc;
    deployment::InstallException instExc;

    bool approve = false;
    bool abort = false;

    if ((request >>= verExc)
        || (request >>= licExc)
        || (request >>= instExc))
    {
        approve = true;
    }

    handle_(approve, abort, xRequest);
}


LicenseCommandEnv::LicenseCommandEnv(
    css::uno::Reference< css::task::XInteractionHandler> const & handler,
    bool bSuppressLicense,
    OUString const & repository):
    BaseCommandEnv(handler), m_repository(repository),
    m_bSuppressLicense(bSuppressLicense)
{
}

void LicenseCommandEnv::handle(
    Reference< task::XInteractionRequest> const & xRequest )
    throw (uno::RuntimeException)
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::LicenseException licExc;

    bool approve = false;
    bool abort = false;

    if (request >>= licExc)
    {
        if (m_bSuppressLicense
            || m_repository == "bundled"
            || licExc.AcceptBy == "admin")
        {
            
            
            
            
            approve = true;
        }
    }

    handle_(approve, abort, xRequest);
}



NoLicenseCommandEnv::NoLicenseCommandEnv(
    css::uno::Reference< css::task::XInteractionHandler> const & handler):
    BaseCommandEnv(handler)
{
}

void NoLicenseCommandEnv::handle(
    Reference< task::XInteractionRequest> const & xRequest )
    throw (uno::RuntimeException)
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::LicenseException licExc;

    bool approve = false;
    bool abort = false;

    if (request >>= licExc)
    {
        approve = true;
    }
    handle_(approve, abort, xRequest);
}

SilentCheckPrerequisitesCommandEnv::SilentCheckPrerequisitesCommandEnv()
{
}

void SilentCheckPrerequisitesCommandEnv::handle(
       Reference< task::XInteractionRequest> const & xRequest )
    throw (uno::RuntimeException)
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::LicenseException licExc;
    deployment::PlatformException platformExc;
    deployment::DependencyException depExc;

    if (request >>= licExc)
    {
        handle_(true, false, xRequest); 
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
