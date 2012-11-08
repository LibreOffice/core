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

#include <jobs/joburl.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <general.h>

#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

namespace framework{

/**
    @short      special ctor
    @descr      It initialize this new instance with a (hopyfully) valid job URL.
                This URL will be parsed. After that we set our members right,
                so other interface methods of this class can be used to get
                all items of this URL. Of course it will be possible to know,
                if this URL was valid too.

    @param      sURL
                    the job URL for parsing
*/
JobURL::JobURL( /*IN*/ const ::rtl::OUString& sURL )
    : ThreadHelpBase( &Application::GetSolarMutex() )
{
    m_eRequest = E_UNKNOWN;

    // syntax: vnd.sun.star.job:{[event=<name>],[alias=<name>],[service=<name>]}

    // check for "vnd.sun.star.job:"
    if (sURL.matchIgnoreAsciiCaseAsciiL(JOBURL_PROTOCOL_STR,JOBURL_PROTOCOL_LEN,0))
    {
        sal_Int32 t = JOBURL_PROTOCOL_LEN;
        do
        {
            // seperate all token of "{[event=<name>],[alias=<name>],[service=<name>]}"
            ::rtl::OUString sToken = sURL.getToken(0, JOBURL_PART_SEPERATOR, t);
            ::rtl::OUString sPartValue    ;
            ::rtl::OUString sPartArguments;

            // check for "event="
            if (
                (JobURL::implst_split(sToken,JOBURL_EVENT_STR,JOBURL_EVENT_LEN,sPartValue,sPartArguments)) &&
                (!sPartValue.isEmpty())
               )
            {
                // set the part value
                m_sEvent     = sPartValue    ;
                m_sEventArgs = sPartArguments;
                m_eRequest  |= E_EVENT       ;
            }
            else
            // check for "alias="
            if (
                (JobURL::implst_split(sToken,JOBURL_ALIAS_STR,JOBURL_ALIAS_LEN,sPartValue,sPartArguments)) &&
                (!sPartValue.isEmpty())
               )
            {
                // set the part value
                m_sAlias     = sPartValue    ;
                m_sAliasArgs = sPartArguments;
                m_eRequest  |= E_ALIAS       ;
            }
            else
            // check for "service="
            if (
                (JobURL::implst_split(sToken,JOBURL_SERVICE_STR,JOBURL_SERVICE_LEN,sPartValue,sPartArguments)) &&
                (!sPartValue.isEmpty())
               )
            {
                // set the part value
                m_sService     = sPartValue    ;
                m_sServiceArgs = sPartArguments;
                m_eRequest    |= E_SERVICE     ;
            }
        }
        while(t!=-1);
    }
}

//________________________________
/**
    @short      knows, if this job URL object hold a valid URL inside

    @return     <TRUE/> if it represent a valid job URL.
*/
sal_Bool JobURL::isValid() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return (m_eRequest!=E_UNKNOWN);
}

//________________________________
/**
    @short      get the event item of this job URL
    @descr      Because the three possible parts of such URL (event, alias, service)
                can't be combined, this method can(!) return a valid value - but it's
                not a must. Thats why the return value must be used too, to detect a missing
                event value.

    @param      sEvent
                    returns the possible existing event value
                    e.g. "vnd.sun.star.job:event=myEvent" returns "myEvent"

    @return     <TRUE/> if an event part of the job URL exist and the out parameter
                sEvent was filled.

    @attention  The out parameter will be reseted everytime. Don't use it if method returns <FALSE/>!
*/
sal_Bool JobURL::getEvent( /*OUT*/ ::rtl::OUString& sEvent ) const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

             sEvent = ::rtl::OUString();
    sal_Bool bSet   = ((m_eRequest & E_EVENT) == E_EVENT);
    if (bSet)
        sEvent = m_sEvent;

    aReadLock.unlock();
    /* } SAFE */

    return bSet;
}

//________________________________
/**
    @short      get the alias item of this job URL
    @descr      Because the three possible parts of such URL (event, alias, service)
                can't be combined, this method can(!) return a valid value - but it's
                not a must. Thats why the return value must be used too, to detect a missing
                alias value.

    @param      sAlias
                    returns the possible existing alias value
                    e.g. "vnd.sun.star.job:alias=myAlias" returns "myAlias"

    @return     <TRUE/> if an alias part of the job URL exist and the out parameter
                sAlias was filled.

    @attention  The out parameter will be reseted everytime. Don't use it if method returns <FALSE/>!
*/
sal_Bool JobURL::getAlias( /*OUT*/ ::rtl::OUString& sAlias ) const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

             sAlias = ::rtl::OUString();
    sal_Bool bSet   = ((m_eRequest & E_ALIAS) == E_ALIAS);
    if (bSet)
        sAlias = m_sAlias;

    aReadLock.unlock();
    /* } SAFE */

    return bSet;
}

//________________________________
/**
    @short      get the service item of this job URL
    @descr      Because the three possible parts of such URL (event, service, service)
                can't be combined, this method can(!) return a valid value - but it's
                not a must. Thats why the return value must be used too, to detect a missing
                service value.

    @param      sAlias
                    returns the possible existing service value
                    e.g. "vnd.sun.star.job:service=com.sun.star.Service" returns "com.sun.star.Service"

    @return     <TRUE/> if an service part of the job URL exist and the out parameter
                sService was filled.

    @attention  The out parameter will be reseted everytime. Don't use it if method returns <FALSE/>!
*/
sal_Bool JobURL::getService( /*OUT*/ ::rtl::OUString& sService ) const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

             sService = ::rtl::OUString();
    sal_Bool bSet     = ((m_eRequest & E_SERVICE) == E_SERVICE);
    if (bSet)
        sService = m_sService;

    aReadLock.unlock();
    /* } SAFE */

    return bSet;
}

//________________________________
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
                    returns the part value if <var>sPart</var> was splitted successfully

    @param      rPartArguments
                    returns the part arguments if <var>sPart</var> was splitted successfully

    @return     <TRUE/> if the identifier could be found and the string was splitted.
                <FALSE/> otherwhise.
*/
sal_Bool JobURL::implst_split( /*IN*/  const ::rtl::OUString& sPart           ,
                               /*IN*/  const sal_Char*        pPartIdentifier ,
                               /*IN*/        sal_Int32        nPartLength     ,
                               /*OUT*/       ::rtl::OUString& rPartValue      ,
                               /*OUT*/       ::rtl::OUString& rPartArguments  )
{
    // first search for the given identifier
    sal_Bool bPartFound = (sPart.matchIgnoreAsciiCaseAsciiL(pPartIdentifier,nPartLength,0));

    // If it exist - we can split the part and return sal_True.
    // Otherwhise we do nothing and return sal_False.
    if (bPartFound)
    {
        // But may the part has optional arguments - seperated by a "?".
        // Do so - we set the return value with the whole part string.
        // Arguments will be set to an empty string as default.
        // If we detect the right sign - we split the arguments and overwrite the default.
        ::rtl::OUString sValueAndArguments = sPart.copy(nPartLength);
        ::rtl::OUString sValue             = sValueAndArguments     ;
        ::rtl::OUString sArguments;

        sal_Int32 nArgStart = sValueAndArguments.indexOf('?',0);
        if (nArgStart!=-1)
        {
            sValue     = sValueAndArguments.copy(0,nArgStart);
            ++nArgStart; // ignore '?'!
            sArguments = sValueAndArguments.copy(nArgStart);
        }

        rPartValue     = sValue    ;
        rPartArguments = sArguments;
    }

    return bPartFound;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
