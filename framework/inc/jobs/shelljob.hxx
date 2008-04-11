/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shelljob.hxx,v $
 * $Revision: 1.4 $
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

#ifndef __FRAMEWORK_JOBS_SHELLJOB_HXX_
#define __FRAMEWORK_JOBS_SHELLJOB_HXX_

//_______________________________________________
// my own includes

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

//_______________________________________________
// other includes
#include <cppuhelper/weak.hxx>

//_______________________________________________
// uno includes
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
class ShellJob : public css::lang::XTypeProvider
               , public css::lang::XServiceInfo
               , public css::task::XJob
               , private ThreadHelpBase
               , public ::cppu::OWeakObject
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
        // css.uno.XInterface
        // css.lang.XTypeProvider
        // css.lang.XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
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

            @return TRUE if command was executed successfully; FALSE otherwise.
         */
        ::sal_Bool impl_execute(const ::rtl::OUString&                       sCommand      ,
                                const css::uno::Sequence< ::rtl::OUString >& lArguments    ,
                                      ::sal_Bool                             bCheckExitCode);
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_SHELLJOB_HXX_
