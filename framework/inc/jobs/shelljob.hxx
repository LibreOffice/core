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

#pragma once

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace framework{

/** @short  implements a job component which can be used
            to execute system shell commands.

    @descr  Because the job will be implemented generic
            it can be bound to any event where jobs can be
            registered for. Further there is a generic
            way to configure the shell command and it's list
            of arguments.
 */
class ShellJob final : public ::cppu::WeakImplHelper< css::lang::XServiceInfo,css::task::XJob >
{

    // member
    private:

        /** @short  reference to a uno service manager. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

    // native interface
    public:

        /** @short  create new instance of this class.

            @param  xContext
                    reference to the uno service manager, which created this instance.
                    Can be used later to create own needed uno resources on demand.
         */
        ShellJob(css::uno::Reference< css::uno::XComponentContext > xContext);

        /** @short  does nothing real ...

            @descr  But it should exists as virtual function,
                    so this class can't make trouble
                    related to inline/symbols etcpp.!
         */
        virtual ~ShellJob() override;

    // uno interface
    public:

        /* interface XServiceInfo */
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // css.task.XJob
        virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments) override;

    // helper
    private:

        /** generate a return value for method execute()
            which will force deactivation of this job for further requests.

            @return an Any following the job protocol for deactivation.
         */
        static css::uno::Any impl_generateAnswer4Deactivation();

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
        OUString impl_substituteCommandVariables(const OUString& sCommand);

        /** executes the command.

            @param  sCommand
                    the absolute command as URL or system path (without any argument !).

            @param  lArguments
                    the complete list of arguments configured for these job.

            @param  bCheckExitCode
                    bind the execution result to the exit code of the started process.
                    If it's set to false we return false only in case executable couldn't be found
                    or couldn't be started.

            @return sal_True if command was executed successfully; sal_False otherwise.
         */
        static bool impl_execute(const OUString&                       sCommand      ,
                                const css::uno::Sequence< OUString >& lArguments    ,
                                      bool                             bCheckExitCode);
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
