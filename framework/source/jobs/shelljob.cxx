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


//_______________________________________________
// include own header

#include <jobs/shelljob.hxx>
#include <jobs/jobconst.hxx>
#include <threadhelp/readguard.hxx>
#include <services.h>

//_______________________________________________
// include others

#include <osl/file.hxx>
#include <osl/process.h>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>

//_______________________________________________
// include interfaces

#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>


namespace framework{


/** address job configuration inside argument set provided on method execute(). */
static const ::rtl::OUString PROP_JOBCONFIG("JobConfig");

/** address job configuration property "Command". */
static const ::rtl::OUString PROP_COMMAND("Command");

/** address job configuration property "Arguments". */
static const ::rtl::OUString PROP_ARGUMENTS("Arguments");

/** address job configuration property "DeactivateJobIfDone". */
static const ::rtl::OUString PROP_DEACTIVATEJOBIFDONE("DeactivateJobIfDone");

/** address job configuration property "CheckExitCode". */
static const ::rtl::OUString PROP_CHECKEXITCODE("CheckExitCode");

//-----------------------------------------------

DEFINE_XSERVICEINFO_MULTISERVICE(ShellJob                   ,
                                 ::cppu::OWeakObject        ,
                                 SERVICENAME_JOB            ,
                                 IMPLEMENTATIONNAME_SHELLJOB)

DEFINE_INIT_SERVICE(ShellJob,
                    {
                        /*  Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//-----------------------------------------------
ShellJob::ShellJob(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase(     )
    , m_xSMGR       (xSMGR)
{
}

//-----------------------------------------------
ShellJob::~ShellJob()
{
}

//-----------------------------------------------
css::uno::Any SAL_CALL ShellJob::execute(const css::uno::Sequence< css::beans::NamedValue >& lJobArguments)
    throw(css::lang::IllegalArgumentException,
          css::uno::Exception                ,
          css::uno::RuntimeException         )
{
    ::comphelper::SequenceAsHashMap lArgs  (lJobArguments);
    ::comphelper::SequenceAsHashMap lOwnCfg(lArgs.getUnpackedValueOrDefault(PROP_JOBCONFIG, css::uno::Sequence< css::beans::NamedValue >()));

    const ::rtl::OUString                       sCommand                   = lOwnCfg.getUnpackedValueOrDefault(PROP_COMMAND                  , ::rtl::OUString());
    const css::uno::Sequence< ::rtl::OUString > lCommandArguments          = lOwnCfg.getUnpackedValueOrDefault(PROP_ARGUMENTS                , css::uno::Sequence< ::rtl::OUString >());
    const ::sal_Bool                            bDeactivateJobIfDone       = lOwnCfg.getUnpackedValueOrDefault(PROP_DEACTIVATEJOBIFDONE      , sal_True         );
    const ::sal_Bool                            bCheckExitCode             = lOwnCfg.getUnpackedValueOrDefault(PROP_CHECKEXITCODE            , sal_True         );

    // replace all might existing place holder.
    ::rtl::OUString sRealCommand = impl_substituteCommandVariables(sCommand);

    // Command is required as minimum.
    // If it does not exists ... we cant do our job.
    // Deactivate such miss configured job silently .-)
    if (sRealCommand.isEmpty())
        return ShellJob::impl_generateAnswer4Deactivation();

    // do it
    ::sal_Bool bDone = impl_execute(sRealCommand, lCommandArguments, bCheckExitCode);
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

//-----------------------------------------------
css::uno::Any ShellJob::impl_generateAnswer4Deactivation()
{
    css::uno::Sequence< css::beans::NamedValue > aAnswer(1);
    aAnswer[0].Name  = JobConst::ANSWER_DEACTIVATE_JOB();
    aAnswer[0].Value = css::uno::makeAny(sal_True);

    return css::uno::makeAny(aAnswer);
}

//-----------------------------------------------
::rtl::OUString ShellJob::impl_substituteCommandVariables(const ::rtl::OUString& sCommand)
{
    // SYNCHRONIZED ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SYNCHRONIZED

    try
    {
        css::uno::Reference< css::uno::XComponentContext >    xContext( comphelper::getComponentContext(xSMGR) );
        css::uno::Reference< css::util::XStringSubstitution > xSubst(  css::util::PathSubstitution::create(xContext) );
        const ::sal_Bool                                      bSubstRequired   = sal_True;
        const ::rtl::OUString                                 sCompleteCommand = xSubst->substituteVariables(sCommand, bSubstRequired);

        return sCompleteCommand;
    }
    catch(const css::uno::Exception&)
    {}

    return ::rtl::OUString();
}

//-----------------------------------------------
::sal_Bool ShellJob::impl_execute(const ::rtl::OUString&                       sCommand      ,
                                  const css::uno::Sequence< ::rtl::OUString >& lArguments    ,
                                        ::sal_Bool                             bCheckExitCode)
{
          ::rtl_uString**  pArgs    = NULL;
    const ::sal_Int32      nArgs    = lArguments.getLength ();
          oslProcessOption nOptions = osl_Process_WAIT;
          oslProcess       hProcess(0);

    if (nArgs > 0)
        pArgs = reinterpret_cast< ::rtl_uString** >(const_cast< ::rtl::OUString* >(lArguments.getConstArray()));

    oslProcessError eError = osl_executeProcess(sCommand.pData, pArgs, nArgs, nOptions, NULL, NULL, NULL, 0, &hProcess);

    // executable not found or couldnt be started
    if (eError != osl_Process_E_None)
        return sal_False;

    ::sal_Bool bRet = sal_True;
    if (bCheckExitCode)
    {
        // check its return codes ...
        oslProcessInfo aInfo;
        aInfo.Size = sizeof (oslProcessInfo);
        eError = osl_getProcessInfo(hProcess, osl_Process_EXITCODE, &aInfo);

        if (eError != osl_Process_E_None)
            bRet = sal_False;
        else
            bRet = (aInfo.Code == 0);
    }
    osl_freeProcessHandle(hProcess);
    return bRet;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
