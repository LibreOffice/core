    /*************************************************************************
 *
 *  $RCSfile: jobresult.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//________________________________
//  my own includes

#ifndef __FRAMEWORK_JOBS_JOBRESULT_HXX_
#include <jobs/jobresult.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//________________________________
//  interface includes

//________________________________
//  includes of other projects

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//________________________________
//  namespace

namespace framework{

//________________________________
//  non exported const

//________________________________
//  non exported definitions

//________________________________
//  declarations

//________________________________
/**
    @short      standard dtor
    @descr      It does nothing else ...
                but it marks this new instance as non valid!
*/
JobResult::JobResult()
    : ThreadHelpBase(&Application::GetSolarMutex())
{
    // reset the flag mask!
    // It will reset the accessible state of this object.
    // That can be usefull if something will fail here ...
    m_eParts = E_NOPART;
}

//________________________________
/**
    @short      special ctor
    @descr      It initialize this new instance with a pure job execution result
                and analyze it. Doing so, we actualize our other members.

                <p>
                It's a list of named values, packed inside this any.
                Following protocol is used:
                <p>
                <ul>
                    <li>
                        "SaveArguments" [sequence< css.beans.NamedValue >]
                        <br>
                        The returned list of (for this generic implementation unknown!)
                        properties, will be written directly to the configuration and replace
                        any old values there. There will no check for changes and we doesn't
                        support any mege feature here. They are written only. The job has
                        to modify this list.
                    </li>
                    <li>
                        "SendDispatchResult" [css.frame.DispatchResultEvent]
                        <br>
                        The given event is send to all current registered listener.
                        But it's not guaranteed. In case no listener are available or
                        this job isn't part of the dispatch environment (because it was used
                        by the css..task.XJobExecutor->trigger() implementation) this option
                        will be ignored.
                    </li>
                    <li>
                        "Deactivate" [boolean]
                        <br>
                        The job whish to be disabled. But note: There is no way, to enable it later
                        again by using this implementation. It can be done by using the configuration
                        only. (Means to register this job again.)
                        If a job knows, that there exist some status or result listener, it must use
                        the options "SendDispatchStatus" and "SendDispatchResult" (see before) too, to
                        inform it about the deactivation of this service.
                    </li>
                </ul>

    @param      aResult
                    the job result
*/
JobResult::JobResult( /*IN*/ const css::uno::Any& aResult )
    : ThreadHelpBase(&Application::GetSolarMutex())
{
    // safe the pure result
    // May someone need it later ...
    m_aPureResult = aResult;

    // reset the flag mask!
    // It will reset the accessible state of this object.
    // That can be usefull if something will fail here ...
    m_eParts = E_NOPART;

    // analyze the result and actualize our other members
    css::uno::Sequence< css::beans::NamedValue > lProtocol;
    if (!(aResult >>= lProtocol))
        return;

    sal_Int32 nCount = lProtocol.getLength();
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        if (
            (lProtocol[i].Name.equalsIgnoreAsciiCaseAsciiL("Deactivate",10)) &&
            (lProtocol[i].Value >>= m_bDeactivate                          )
           )
        {
            m_eParts |= E_DEACTIVATE;
        }
        else
        if (
            (lProtocol[i].Name.equalsIgnoreAsciiCaseAsciiL("SaveArguments",13)) &&
            (lProtocol[i].Value >>= m_lArguments                              )
           )
        {
            m_eParts |= E_ARGUMENTS;
        }
        else
        if (
            (lProtocol[i].Name.equalsIgnoreAsciiCaseAsciiL("SendDispatchResult",18)) &&
            (lProtocol[i].Value >>= m_aDispatchResult                              )
           )
        {
            m_eParts |= E_DISPATCHRESULT;
        }
    }
}

//________________________________
/**
    @short      copy dtor
    @descr      -
*/
JobResult::JobResult( const JobResult& rCopy )
{
    m_aPureResult     = rCopy.m_aPureResult     ;
    m_eParts          = rCopy.m_eParts          ;
    m_lArguments      = rCopy.m_lArguments      ;
    m_bDeactivate     = rCopy.m_bDeactivate     ;
    m_aDispatchResult = rCopy.m_aDispatchResult ;
}

//________________________________
/**
    @short      standard dtor
    @descr      Free all internaly used ressources at the end of living.
*/
JobResult::~JobResult()
{
    // Nothing realy to do here.
}

//________________________________
/**
    @short      =operator
    @descr      Must be implemented to overwrite this instance with another one.

    @param      rCopy
                    reference to the other instance, which should be used for copying.
*/
void JobResult::operator=( const JobResult& rCopy )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_aPureResult     = rCopy.m_aPureResult     ;
    m_eParts          = rCopy.m_eParts          ;
    m_lArguments      = rCopy.m_lArguments      ;
    m_bDeactivate     = rCopy.m_bDeactivate     ;
    m_aDispatchResult = rCopy.m_aDispatchResult ;
    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      checks for existing parts of the analyzed result
    @descr      The internal flag mask was set after analyzing of the pure result.
                An user of us can check here, if the required part was realy part
                of this result. Otherwhise it would use invalid informations ...
                by using our other members!

    @param      eParts
                    a flag mask too, which will be compared with our internaly set one.

    @return     We return true only, if any set flag of the given mask match.
*/
sal_Bool JobResult::existPart( sal_uInt32 eParts ) const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return ((m_eParts & eParts) == eParts);
    /* } SAFE */
}

//________________________________
/**
    @short      provides access to our internal members
    @descr      The return value will be valid only in case a call of
                existPart(E_...) before returned true!

    @return     It returns the state of the internal member
                without any checks!
*/
css::uno::Sequence< css::beans::NamedValue > JobResult::getArguments() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_lArguments;
    /* } SAFE */
}

//________________________________

sal_Bool JobResult::getDeactivate() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_bDeactivate;
    /* } SAFE */
}

//________________________________

css::frame::DispatchResultEvent JobResult::getDispatchResult() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aDispatchResult;
    /* } SAFE */
}

} // namespace framework
