/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shelljob.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:54:22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// include own header

#include <jobs/shelljob.hxx>
#include <jobs/jobconst.hxx>
#include <threadhelp/readguard.hxx>
#include <services.h>

//_______________________________________________
// include others

#include <osl/file.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>

//_______________________________________________
// include interfaces

#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// definitions

/** adress job configuration inside argument set provided on method execute(). */
static const ::rtl::OUString PROP_JOBCONFIG = ::rtl::OUString::createFromAscii("JobConfig");

/** adress job configuration property "Command". */
static const ::rtl::OUString PROP_COMMAND = ::rtl::OUString::createFromAscii("Command");

/** adress job configuration property "Arguments". */
static const ::rtl::OUString PROP_ARGUMENTS = ::rtl::OUString::createFromAscii("Arguments");

/** adress job configuration property "NeedsSystemPathConversion". */
static const ::rtl::OUString PROP_NEEDSSYSTEMPATHCONVERSION = ::rtl::OUString::createFromAscii("NeedsSystemPathConversion");

/** adress job configuration property "DeactivateJobIfDone". */
static const ::rtl::OUString PROP_DEACTIVATEJOBIFDONE = ::rtl::OUString::createFromAscii("DeactivateJobIfDone");

/** special and magic number to make XSystemShellExecute working .-)

    The problem behind: SystemShellExecute is used inside SFX to open hyperlinks
    within a browser. But such URLs must be encoded ... otherwise they can be used
    differently by adding real shell commands at the end of the URL !

    On the other side real shell commands cant be used at the XSystemShellExecute
    so. Because a list of arguments will be handled as one single argument then.

    Solution: We must have two services differ between thos two use cases ...
    Workaround: XSystemShellExecute uses last parameter (long flags) ins special manner
    to know that the given command is realy a shell command and not an URL.
    And the answer to all questions is (as everytime) ... 42 :-))
*/
static const ::sal_Int32 ANSWER_TO_ALL_QUESTIONS = 42;

//-----------------------------------------------

DEFINE_XINTERFACE_3(ShellJob                                   ,
                    OWeakObject                                ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo  ),
                    DIRECT_INTERFACE(css::task::XJob          ))

DEFINE_XTYPEPROVIDER_3(ShellJob                 ,
                       css::lang::XTypeProvider ,
                       css::lang::XServiceInfo  ,
                       css::task::XJob          )

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
css::uno::Any SAL_CALL ShellJob::execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
    throw(css::lang::IllegalArgumentException,
          css::uno::Exception                ,
          css::uno::RuntimeException         )
{
    ::comphelper::SequenceAsHashMap lArgs  (lArguments);
    ::comphelper::SequenceAsHashMap lOwnCfg(lArgs.getUnpackedValueOrDefault(PROP_JOBCONFIG, css::uno::Sequence< css::beans::NamedValue >()));

    ::rtl::OUString sCommand                   = lOwnCfg.getUnpackedValueOrDefault(PROP_COMMAND                  , ::rtl::OUString());
    ::rtl::OUString sArguments                 = lOwnCfg.getUnpackedValueOrDefault(PROP_ARGUMENTS                , ::rtl::OUString());
    ::sal_Bool      bNeedsSystemPathConversion = lOwnCfg.getUnpackedValueOrDefault(PROP_NEEDSSYSTEMPATHCONVERSION, sal_False        );
    ::sal_Bool      bDeactivateJobIfDone       = lOwnCfg.getUnpackedValueOrDefault(PROP_DEACTIVATEJOBIFDONE      , sal_True         );

    // replace all might existing place holder.
    ::rtl::OUString sCompleteCommand = impl_substituteCommandVariables(sCommand);

    // see if URL->system path conversion is needed.
    if (bNeedsSystemPathConversion)
        sCompleteCommand = impl_convertCommandURL2SystemPath(sCompleteCommand);

    // Command is required as minimum.
    // If it does not exists ... or conversion to system path failed
    // we cant do our job.
    // Deactivate such miss configured job silently .-)
    if (sCompleteCommand.getLength() < 1)
        return ShellJob::impl_generateAnswer4Deactivation();

    // do it
    ::sal_Bool bDone = impl_execute(sCompleteCommand, sArguments);
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
              css::uno::Reference< css::util::XStringSubstitution > xSubst           (  xSMGR->createInstance(SERVICENAME_SUBSTITUTEPATHVARIABLES), css::uno::UNO_QUERY_THROW);
        const ::sal_Bool                                            bSubstRequired   = sal_True;
        const ::rtl::OUString                                       sCompleteCommand = xSubst->substituteVariables(sCommand, bSubstRequired);

        return sCompleteCommand;
    }
    catch(const css::uno::Exception&)
    {}

    return ::rtl::OUString();
}

//-----------------------------------------------
::rtl::OUString ShellJob::impl_convertCommandURL2SystemPath(const ::rtl::OUString& sURL)
{
    ::rtl::OUString sPath;

    if (::osl::FileBase::getSystemPathFromFileURL(sURL, sPath) == ::osl::FileBase::E_None)
        return sPath;

    return ::rtl::OUString();
}

//-----------------------------------------------
::sal_Bool ShellJob::impl_execute(const ::rtl::OUString& sCommand  ,
                                  const ::rtl::OUString& sArguments)
{
    // SYNCHRONIZED ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SYNCHRONIZED

    try
    {
        css::uno::Reference< css::system::XSystemShellExecute > xExecute(xSMGR->createInstance(SERVICENAME_SYSTEMSHELLEXECUTE), css::uno::UNO_QUERY_THROW);
#ifdef UNX
        xExecute->execute(sCommand, sArguments, ANSWER_TO_ALL_QUESTIONS);
#elif defined( WNT )
        xExecute->execute(sCommand, sArguments, css::system::SystemShellExecuteFlags::NO_SYSTEM_ERROR_MESSAGE);
#endif
    }
    catch(const css::uno::Exception&)
    {
        return sal_False;
    }

    return sal_True;
}

} // namespace framework
