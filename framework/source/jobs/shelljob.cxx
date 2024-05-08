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

// include own header

#include <jobs/shelljob.hxx>
#include <jobs/jobconst.hxx>
#include <services.h>

// include others

#include <osl/process.h>
#include <comphelper/sequenceashashmap.hxx>

// include interfaces

#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <utility>

namespace framework{


// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL ShellJob::getImplementationName()
{
    return u"com.sun.star.comp.framework.ShellJob"_ustr;
}

sal_Bool SAL_CALL ShellJob::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ShellJob::getSupportedServiceNames()
{
    return { SERVICENAME_JOB };
}


ShellJob::ShellJob(css::uno::Reference< css::uno::XComponentContext >  xContext)
    : m_xContext    (std::move(xContext))
{
}

ShellJob::~ShellJob()
{
}

css::uno::Any SAL_CALL ShellJob::execute(const css::uno::Sequence< css::beans::NamedValue >& lJobArguments)
{
    ::comphelper::SequenceAsHashMap lArgs  (lJobArguments);
    /** address job configuration inside argument set provided on method execute(). */
    ::comphelper::SequenceAsHashMap lOwnCfg(lArgs.getUnpackedValueOrDefault(u"JobConfig"_ustr, css::uno::Sequence< css::beans::NamedValue >()));

    const OUString                       sCommand               = lOwnCfg.getUnpackedValueOrDefault(u"Command"_ustr             , OUString());
    const css::uno::Sequence< OUString > lCommandArguments      = lOwnCfg.getUnpackedValueOrDefault(u"Arguments"_ustr           , css::uno::Sequence< OUString >());
    const bool                           bDeactivateJobIfDone   = lOwnCfg.getUnpackedValueOrDefault(u"DeactivateJobIfDone"_ustr , true );
    const bool                           bCheckExitCode         = lOwnCfg.getUnpackedValueOrDefault(u"CheckExitCode"_ustr       , true );

    // replace all might existing place holder.
    OUString sRealCommand = impl_substituteCommandVariables(sCommand);

    // Command is required as minimum.
    // If it does not exists ... we can't do our job.
    // Deactivate such miss configured job silently .-)
    if (sRealCommand.isEmpty())
        return ShellJob::impl_generateAnswer4Deactivation();

    // do it
    bool bDone = impl_execute(sRealCommand, lCommandArguments, bCheckExitCode);
    if (! bDone)
        return css::uno::Any();

    // Job was done ... user configured deactivation of this job
    // in such case.
    if (bDeactivateJobIfDone)
        return ShellJob::impl_generateAnswer4Deactivation();

    // There was no decision about deactivation of this job.
    // So we have to return nothing here !
    return css::uno::Any();
}

css::uno::Any ShellJob::impl_generateAnswer4Deactivation()
{
    css::uno::Sequence< css::beans::NamedValue > aAnswer { { JobConst::ANSWER_DEACTIVATE_JOB, css::uno::Any(true) } };
    return css::uno::Any(aAnswer);
}

OUString ShellJob::impl_substituteCommandVariables(const OUString& sCommand)
{
    try
    {
        css::uno::Reference< css::util::XStringSubstitution > xSubst(  css::util::PathSubstitution::create(m_xContext) );
        const bool                                      bSubstRequired   = true;
        const OUString                                 sCompleteCommand = xSubst->substituteVariables(sCommand, bSubstRequired);

        return sCompleteCommand;
    }
    catch(const css::uno::Exception&)
    {}

    return OUString();
}

bool ShellJob::impl_execute(const OUString&                       sCommand      ,
                            const css::uno::Sequence< OUString >& lArguments    ,
                                  bool                            bCheckExitCode)
{
    ::rtl_uString**   pArgs    = nullptr;
    const ::sal_Int32 nArgs    = lArguments.getLength ();
    oslProcess        hProcess(nullptr);

    if (nArgs > 0)
        pArgs = reinterpret_cast< ::rtl_uString** >(const_cast< OUString* >(lArguments.getConstArray()));

    oslProcessError eError = osl_executeProcess(sCommand.pData, pArgs, nArgs, osl_Process_WAIT, nullptr, nullptr, nullptr, 0, &hProcess);

    // executable not found or couldn't be started
    if (eError != osl_Process_E_None)
        return false;

    bool bRet = true;
    if (bCheckExitCode)
    {
        // check its return codes ...
        oslProcessInfo aInfo;
        aInfo.Size = sizeof (oslProcessInfo);
        eError = osl_getProcessInfo(hProcess, osl_Process_EXITCODE, &aInfo);

        if (eError != osl_Process_E_None)
            bRet = false;
        else
            bRet = (aInfo.Code == 0);
    }
    osl_freeProcessHandle(hProcess);
    return bRet;
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_ShellJob_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::ShellJob(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
