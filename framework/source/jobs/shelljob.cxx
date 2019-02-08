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

#include <osl/file.hxx>
#include <osl/process.h>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>

// include interfaces

#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>

namespace framework{


DEFINE_XSERVICEINFO_MULTISERVICE_2(ShellJob                   ,
                                   ::cppu::OWeakObject        ,
                                   SERVICENAME_JOB            ,
                                   IMPLEMENTATIONNAME_SHELLJOB)

DEFINE_INIT_SERVICE(ShellJob,
                    {
                        /*  Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                        */
                    }
                   )

ShellJob::ShellJob(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xContext    (xContext)
{
}

ShellJob::~ShellJob()
{
}

css::uno::Any SAL_CALL ShellJob::execute(const css::uno::Sequence< css::beans::NamedValue >& lJobArguments)
{
    ::comphelper::SequenceAsHashMap lArgs  (lJobArguments);
    /** address job configuration inside argument set provided on method execute(). */
    ::comphelper::SequenceAsHashMap lOwnCfg(lArgs.getUnpackedValueOrDefault("JobConfig", css::uno::Sequence< css::beans::NamedValue >()));

    const OUString                       sCommand               = lOwnCfg.getUnpackedValueOrDefault("Command"             , OUString());
    const css::uno::Sequence< OUString > lCommandArguments      = lOwnCfg.getUnpackedValueOrDefault("Arguments"           , css::uno::Sequence< OUString >());
    const bool                           bDeactivateJobIfDone   = lOwnCfg.getUnpackedValueOrDefault("DeactivateJobIfDone" , true );
    const bool                           bCheckExitCode         = lOwnCfg.getUnpackedValueOrDefault("CheckExitCode"       , true );

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
    css::uno::Sequence< css::beans::NamedValue > aAnswer { { JobConst::ANSWER_DEACTIVATE_JOB(), css::uno::makeAny(true) } };
    return css::uno::makeAny(aAnswer);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
