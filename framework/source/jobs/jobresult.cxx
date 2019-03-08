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

#include <jobs/jobresult.hxx>
#include <jobs/jobconst.hxx>
#include <general.h>
#include <services.h>

#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>

namespace framework{

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
                        any old values there. There will no check for changes and we don't
                        support any merge feature here. They are written only. The job has
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
                        The job wish to be disabled. But note: There is no way, to enable it later
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
{
    // reset the flag mask!
    // It will reset the accessible state of this object.
    // That can be useful if something will fail here ...
    m_eParts = E_NOPART;

    // analyze the result and update our other members
    ::comphelper::SequenceAsHashMap aProtocol(aResult);
    if ( aProtocol.empty() )
        return;

    ::comphelper::SequenceAsHashMap::const_iterator pIt = aProtocol.find(JobConst::ANSWER_DEACTIVATE_JOB());
    if (pIt != aProtocol.end())
    {
        /**
            an executed job can force his deactivation
            But we provide this information here only.
            Doing so is part of any user of us.
         */
        bool bDeactivate(false);
        pIt->second >>= bDeactivate;
        if (bDeactivate)
            m_eParts |= E_DEACTIVATE;
    }

    pIt = aProtocol.find(JobConst::ANSWER_SAVE_ARGUMENTS());
    if (pIt != aProtocol.end())
    {
        css::uno::Sequence<css::beans::NamedValue> aTmp;
        pIt->second >>= aTmp;
        comphelper::sequenceToContainer(m_lArguments, aTmp);
        if (m_lArguments.empty())
            m_eParts |= E_ARGUMENTS;
    }

    pIt = aProtocol.find(JobConst::ANSWER_SEND_DISPATCHRESULT());
    if (pIt != aProtocol.end())
    {
        if (pIt->second >>= m_aDispatchResult)
            m_eParts |= E_DISPATCHRESULT;
    }
}

/**
    @short      copy dtor
*/
JobResult::JobResult( const JobResult& rCopy )
{
    m_eParts          = rCopy.m_eParts;
    m_lArguments      = rCopy.m_lArguments;
    m_aDispatchResult = rCopy.m_aDispatchResult;
}

/**
    @short      standard dtor
    @descr      Free all internally used resources at the end of living.
*/
JobResult::~JobResult()
{
    // Nothing really to do here.
}

/**
    @short      =operator
    @descr      Must be implemented to overwrite this instance with another one.

    @param      rCopy
                    reference to the other instance, which should be used for copying.
*/
JobResult& JobResult::operator=( const JobResult& rCopy )
{
    SolarMutexGuard g;
    m_eParts          = rCopy.m_eParts;
    m_lArguments      = rCopy.m_lArguments;
    m_aDispatchResult = rCopy.m_aDispatchResult;
    return *this;
}

/**
    @short      checks for existing parts of the analyzed result
    @descr      The internal flag mask was set after analyzing of the pure result.
                An user of us can check here, if the required part was really part
                of this result. Otherwise it would use invalid information ...
                by using our other members!

    @param      eParts
                    a flag mask too, which will be compared with our internally set one.

    @return     We return true only, if any set flag of the given mask match.
*/
bool JobResult::existPart( sal_uInt32 eParts ) const
{
    SolarMutexGuard g;
    return ((m_eParts & eParts) == eParts);
}

/**
    @short      provides access to our internal members
    @descr      The return value will be valid only in case a call of
                existPart(E_...) before returned true!

    @return     It returns the state of the internal member
                without any checks!
*/
std::vector< css::beans::NamedValue > JobResult::getArguments() const
{
    SolarMutexGuard g;
    return m_lArguments;
}

css::frame::DispatchResultEvent JobResult::getDispatchResult() const
{
    SolarMutexGuard g;
    return m_aDispatchResult;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
