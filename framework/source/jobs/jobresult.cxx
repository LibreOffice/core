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


//________________________________
//  my own includes

#include <jobs/jobresult.hxx>
#include <jobs/jobconst.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <general.h>
#include <services.h>

//________________________________
//  interface includes

//________________________________
//  includes of other projects

#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequenceashashmap.hxx>

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
                and analyze it. Doing so, we update our other members.

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

    // analyze the result and update our other members
    ::comphelper::SequenceAsHashMap aProtocol(aResult);
    if ( aProtocol.empty() )
        return;

    ::comphelper::SequenceAsHashMap::const_iterator pIt = aProtocol.end();

    pIt = aProtocol.find(JobConst::ANSWER_DEACTIVATE_JOB());
    if (pIt != aProtocol.end())
    {
        pIt->second >>= m_bDeactivate;
        if (m_bDeactivate)
            m_eParts |= E_DEACTIVATE;
    }

    pIt = aProtocol.find(JobConst::ANSWER_SAVE_ARGUMENTS());
    if (pIt != aProtocol.end())
    {
        pIt->second >>= m_lArguments;
        if (m_lArguments.getLength() > 0)
            m_eParts |= E_ARGUMENTS;
    }

    pIt = aProtocol.find(JobConst::ANSWER_SEND_DISPATCHRESULT());
    if (pIt != aProtocol.end())
    {
        if (pIt->second >>= m_aDispatchResult)
            m_eParts |= E_DISPATCHRESULT;
    }
}

//________________________________
/**
    @short      copy dtor
    @descr      -
*/
JobResult::JobResult( const JobResult& rCopy )
    : ThreadHelpBase(&Application::GetSolarMutex())
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
    @descr      Free all internaly used resources at the end of living.
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

css::frame::DispatchResultEvent JobResult::getDispatchResult() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aDispatchResult;
    /* } SAFE */
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
