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

#include <classes/taskcreator.hxx>
#include "services/taskcreatorsrv.hxx"
#include <threadhelp/readguard.hxx>
#include <loadenv/targethelper.hxx>
#include <services.h>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <comphelper/configurationhelper.hxx>
#include <vcl/svapp.hxx>

namespace framework{

/*-****************************************************************************************************//**
    @short      initialize instance with neccessary informations
    @descr      We need a valid uno service manager to create or instanciate new services.
                All other informations to create frames or tasks come in on right interface methods.

    @param      xSMGR
                    points to the valid uno service manager
*//*-*****************************************************************************************************/
TaskCreator::TaskCreator( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    : ThreadHelpBase(       )
    , m_xSMGR       ( xSMGR )
{
}

/*-****************************************************************************************************//**
    @short      deinitialize instance
    @descr      We should release all used resource which are not needed any longer.
*//*-*****************************************************************************************************/
TaskCreator::~TaskCreator()
{
    m_xSMGR.clear();
}

/*-****************************************************************************************************//**
    TODO document me
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > TaskCreator::createTask( const ::rtl::OUString& sName    ,
                                                                         sal_Bool         bVisible )
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::lang::XSingleServiceFactory > xCreator;
    ::rtl::OUString sCreator = IMPLEMENTATIONNAME_FWK_TASKCREATOR;

    try
    {
        if (
            ( TargetHelper::matchSpecialTarget(sName, TargetHelper::E_BLANK  ) ) ||
            ( TargetHelper::matchSpecialTarget(sName, TargetHelper::E_DEFAULT) )
           )
        {
            ::comphelper::ConfigurationHelper::readDirectKey(xSMGR,
                "org.openoffice.Office.TabBrowse",
                "TaskCreatorService",
                "ImplementationName",
                ::comphelper::ConfigurationHelper::E_READONLY) >>= sCreator;
        }

        xCreator = css::uno::Reference< css::lang::XSingleServiceFactory >(
                    xSMGR->createInstance(sCreator), css::uno::UNO_QUERY_THROW);
    }
    catch(const css::uno::Exception&)
    {}

    // no catch here ... without an task creator service we cant open ANY document window within the office.
    // Thats IMHO not a good idea. Then we should accept the stacktrace showing us the real problem.
    // BTW: The used fallback creator service (IMPLEMENTATIONNAME_FWK_TASKCREATOR) is implemented in the same
    // library then these class here ... Why we should not be able to create it ?
    if ( ! xCreator.is())
        xCreator = css::uno::Reference< css::lang::XSingleServiceFactory >(
                    xSMGR->createInstance(IMPLEMENTATIONNAME_FWK_TASKCREATOR), css::uno::UNO_QUERY_THROW);

    css::uno::Sequence< css::uno::Any > lArgs(5);
    css::beans::NamedValue              aArg    ;

    aArg.Name    = rtl::OUString(ARGUMENT_PARENTFRAME);
    aArg.Value <<= css::uno::Reference< css::frame::XFrame >(xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);
    lArgs[0]   <<= aArg;

    aArg.Name    = rtl::OUString(ARGUMENT_CREATETOPWINDOW);
    aArg.Value <<= sal_True;
    lArgs[1]   <<= aArg;

    aArg.Name    = rtl::OUString(ARGUMENT_MAKEVISIBLE);
    aArg.Value <<= bVisible;
    lArgs[2]   <<= aArg;

    aArg.Name    = rtl::OUString(ARGUMENT_SUPPORTPERSISTENTWINDOWSTATE);
    aArg.Value <<= sal_True;
    lArgs[3]   <<= aArg;

    aArg.Name    = rtl::OUString(ARGUMENT_FRAMENAME);
    aArg.Value <<= sName;
    lArgs[4]   <<= aArg;

    css::uno::Reference< css::frame::XFrame > xTask(xCreator->createInstanceWithArguments(lArgs), css::uno::UNO_QUERY_THROW);
    return xTask;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
