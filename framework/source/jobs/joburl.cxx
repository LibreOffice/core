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

#include <sal/config.h>

#include <cstring>

#include <jobs/joburl.hxx>
#include <general.h>

#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

namespace framework{

/**
    @short      special ctor
    @descr      It initialize this new instance with a (hopefully) valid job URL.
                This URL will be parsed. After that we set our members right,
                so other interface methods of this class can be used to get
                all items of this URL. Of course it will be possible to know,
                if this URL was valid too.

    @param      sURL
                    the job URL for parsing
*/
JobURL::JobURL( /*IN*/ const OUString& sURL )
{
    m_eRequest = E_UNKNOWN;

    // syntax: vnd.sun.star.job:{[event=<name>],[alias=<name>],[service=<name>]}

    // check for "vnd.sun.star.job:"
    if (sURL.startsWithIgnoreAsciiCase("vnd.sun.star.job:"))
    {
        sal_Int32 t = std::strlen("vnd.sun.star.job:");
        do
        {
            // separate all token of "{[event=<name>],[alias=<name>],[service=<name>]}"
            OUString sToken = sURL.getToken(0, JOBURL_PART_SEPARATOR, t);
            OUString sPartValue;
            OUString sPartArguments;

            // check for "event="
            if (
                (JobURL::implst_split(sToken,JOBURL_EVENT_STR,JOBURL_EVENT_LEN,sPartValue,sPartArguments)) &&
                (!sPartValue.isEmpty())
               )
            {
                // set the part value
                m_sEvent     = sPartValue;
                m_eRequest  |= E_EVENT;
            }
            else
            // check for "alias="
            if (
                (JobURL::implst_split(sToken,JOBURL_ALIAS_STR,JOBURL_ALIAS_LEN,sPartValue,sPartArguments)) &&
                (!sPartValue.isEmpty())
               )
            {
                // set the part value
                m_sAlias     = sPartValue;
                m_eRequest  |= E_ALIAS;
            }
            else
            // check for "service="
            if (
                (JobURL::implst_split(sToken,JOBURL_SERVICE_STR,JOBURL_SERVICE_LEN,sPartValue,sPartArguments)) &&
                (!sPartValue.isEmpty())
               )
            {
                // set the part value
                m_sService     = sPartValue;
                m_eRequest    |= E_SERVICE;
            }
        }
        while(t!=-1);
    }
}

/**
    @short      knows, if this job URL object hold a valid URL inside

    @return     <TRUE/> if it represent a valid job URL.
*/
bool JobURL::isValid() const
{
    SolarMutexGuard g;
    return (m_eRequest!=E_UNKNOWN);
}

/**
    @short      get the event item of this job URL
    @descr      Because the three possible parts of such URL (event, alias, service)
                can't be combined, this method can(!) return a valid value - but it's
                not a must. That's why the return value must be used too, to detect a missing
                event value.

    @param      sEvent
                    returns the possible existing event value
                    e.g. "vnd.sun.star.job:event=myEvent" returns "myEvent"

    @return     <TRUE/> if an event part of the job URL exist and the out parameter
                sEvent was filled.

    @attention  The out parameter will be reset every time. Don't use it if method returns <FALSE/>!
*/
bool JobURL::getEvent( /*OUT*/ OUString& sEvent ) const
{
    SolarMutexGuard g;

    sEvent.clear();
    bool bSet   = ((m_eRequest & E_EVENT) == E_EVENT);
    if (bSet)
        sEvent = m_sEvent;

    return bSet;
}

/**
    @short      get the alias item of this job URL
    @descr      Because the three possible parts of such URL (event, alias, service)
                can't be combined, this method can(!) return a valid value - but it's
                not a must. that's why the return value must be used too, to detect a missing
                alias value.

    @param      sAlias
                    returns the possible existing alias value
                    e.g. "vnd.sun.star.job:alias=myAlias" returns "myAlias"

    @return     <TRUE/> if an alias part of the job URL exist and the out parameter
                sAlias was filled.

    @attention  The out parameter will be reset every time. Don't use it if method returns <FALSE/>!
*/
bool JobURL::getAlias( /*OUT*/ OUString& sAlias ) const
{
    SolarMutexGuard g;

    sAlias.clear();
    bool bSet   = ((m_eRequest & E_ALIAS) == E_ALIAS);
    if (bSet)
        sAlias = m_sAlias;

    return bSet;
}

/**
    @short      get the service item of this job URL
    @descr      Because the three possible parts of such URL (event, service, service)
                can't be combined, this method can(!) return a valid value - but it's
                not a must. That's why the return value must be used too, to detect a missing
                service value.

    @param      sAlias
                    returns the possible existing service value
                    e.g. "vnd.sun.star.job:service=com.sun.star.Service" returns "com.sun.star.Service"

    @return     <TRUE/> if an service part of the job URL exist and the out parameter
                sService was filled.

    @attention  The out parameter will be reset every time. Don't use it if method returns <FALSE/>!
*/
bool JobURL::getService( /*OUT*/ OUString& sService ) const
{
    SolarMutexGuard g;

    sService.clear();
    bool bSet     = ((m_eRequest & E_SERVICE) == E_SERVICE);
    if (bSet)
        sService = m_sService;

    return bSet;
}

/**
    @short      searches for a special identifier in the given string and split it
    @descr      If the given identifier could be found at the beginning of the given string,
                this method split it into different parts and return it.
                Following schema is used: <partidentifier>=<partvalue>[?<partarguments>]

    @param      sPart
                    the string, which should be analyzed

    @param      pPartIdentifier
                    the part identifier value, which must be found at the beginning of the
                    parameter <var>sPart</var>

    @param      nPartLength
                    the length of the ascii value <var>pPartIdentifier</var>

    @param      rPartValue
                    returns the part value if <var>sPart</var> was split successfully

    @param      rPartArguments
                    returns the part arguments if <var>sPart</var> was split successfully

    @return     <TRUE/> if the identifier could be found and the string was split.
                <FALSE/> otherwise.
*/
bool JobURL::implst_split( /*IN*/  const OUString& sPart           ,
                               /*IN*/  const sal_Char*        pPartIdentifier ,
                               /*IN*/        sal_Int32        nPartLength     ,
                               /*OUT*/       OUString& rPartValue      ,
                               /*OUT*/       OUString& rPartArguments  )
{
    // first search for the given identifier
    bool bPartFound = sPart.matchIgnoreAsciiCaseAsciiL(pPartIdentifier,nPartLength);

    // If it exist - we can split the part and return sal_True.
    // Otherwise we do nothing and return sal_False.
    if (bPartFound)
    {
        // But may the part has optional arguments - separated by a "?".
        // Do so - we set the return value with the whole part string.
        // Arguments will be set to an empty string as default.
        // If we detect the right sign - we split the arguments and overwrite the default.
        OUString sValueAndArguments = sPart.copy(nPartLength);
        OUString sValue             = sValueAndArguments;
        OUString sArguments;

        sal_Int32 nArgStart = sValueAndArguments.indexOf('?');
        if (nArgStart!=-1)
        {
            sValue     = sValueAndArguments.copy(0,nArgStart);
            ++nArgStart; // ignore '?'!
            sArguments = sValueAndArguments.copy(nArgStart);
        }

        rPartValue     = sValue;
        rPartArguments = sArguments;
    }

    return bPartFound;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
