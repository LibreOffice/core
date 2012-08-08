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

#ifndef __FRAMEWORK_JOBS_SHELLJOB_HXX_
#define __FRAMEWORK_JOBS_SHELLJOB_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// declarations

//_______________________________________________
/** @short  implements a job component which can be used
            to execute system shell commands.

    @descr  Because the job will be implemented generic
            it can be bound to any event where jobs can be
            registered for. Further there is a generic
            way to configure the shell command and it's list
            of arguments.

    @author as96863
 */
class ShellJob : private ThreadHelpBase
               ,public ::cppu::WeakImplHelper2< ::com::sun::star::lang::XServiceInfo,::com::sun::star::task::XJob >
{
    //-------------------------------------------
    // member
    private:

        //.......................................
        /** @short  reference to an uno service manager. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

    //-------------------------------------------
    // native interface
    public:

        //---------------------------------------
        /** @short  create new instance of this class.

            @param  xSMGR
                    reference to the uno service manager, which created this instance.
                    Can be used later to create own needed uno resources on demand.
         */
        ShellJob(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------
        /** @short  does nothing real ...

            @descr  But it should exists as virtual function,
                    so this class cant make trouble
                    related to inline/symbols etcpp.!
         */
        virtual ~ShellJob();

    //-------------------------------------------
    // uno interface
    public:

        //---------------------------------------
        // css.lang.XServiceInfo
        DECLARE_XSERVICEINFO

        // css.task.XJob
        virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
            throw(css::lang::IllegalArgumentException,
                  css::uno::Exception                ,
                  css::uno::RuntimeException         );

    //-------------------------------------------
    // helper
    private:

        //---------------------------------------
        /** generate a return value for method execute()
            which will force deactivation of this job for further requests.

            @return an Any following the job protocol for deactivation.
         */
        static css::uno::Any impl_generateAnswer4Deactivation();

        //---------------------------------------
        /** substitute all might existing placeholder variables
            within the configured command.

            The command is part of the job configuration.
            These will make changes more easy (no code changes required).
            Further the command can use placeholder as they are supported
            by the global substitution service (e.g. $(prog) etcpp)

            @param  sCommand
                    the command containing placeholder variables.

            @return the substituted command.
         */
        ::rtl::OUString impl_substituteCommandVariables(const ::rtl::OUString& sCommand);

        //---------------------------------------
        /** executes the command.

            @param  sCommand
                    the absolute command as URL or system path (without any argument !).

            @param  lArguments
                    the complete list of arguments configured for these job.

            @param  bCheckExitCode
                    bind the execution result to the exit code of the started process.
                    If it's set to false we return false only in case executable couldnt be found
                    or couldnt be started.

            @return sal_True if command was executed successfully; sal_False otherwise.
         */
        ::sal_Bool impl_execute(const ::rtl::OUString&                       sCommand      ,
                                const css::uno::Sequence< ::rtl::OUString >& lArguments    ,
                                      ::sal_Bool                             bCheckExitCode);
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_SHELLJOB_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
