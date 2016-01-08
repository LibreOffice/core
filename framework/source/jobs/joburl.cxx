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
    #ifdef ENABLE_COMPONENT_SELF_CHECK
    JobURL::impldbg_checkIt();
    #endif

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
                m_sEventArgs = sPartArguments;
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
                m_sAliasArgs = sPartArguments;
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
                m_sServiceArgs = sPartArguments;
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
                not a must. Thats why the return value must be used too, to detect a missing
                event value.

    @param      sEvent
                    returns the possible existing event value
                    e.g. "vnd.sun.star.job:event=myEvent" returns "myEvent"

    @return     <TRUE/> if an event part of the job URL exist and the out parameter
                sEvent was filled.

    @attention  The out parameter will be reseted every time. Don't use it if method returns <FALSE/>!
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
                not a must. Thats why the return value must be used too, to detect a missing
                alias value.

    @param      sAlias
                    returns the possible existing alias value
                    e.g. "vnd.sun.star.job:alias=myAlias" returns "myAlias"

    @return     <TRUE/> if an alias part of the job URL exist and the out parameter
                sAlias was filled.

    @attention  The out parameter will be reseted every time. Don't use it if method returns <FALSE/>!
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
                not a must. Thats why the return value must be used too, to detect a missing
                service value.

    @param      sAlias
                    returns the possible existing service value
                    e.g. "vnd.sun.star.job:service=com.sun.star.Service" returns "com.sun.star.Service"

    @return     <TRUE/> if an service part of the job URL exist and the out parameter
                sService was filled.

    @attention  The out parameter will be reseted every time. Don't use it if method returns <FALSE/>!
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
                    returns the part value if <var>sPart</var> was splitted successfully

    @param      rPartArguments
                    returns the part arguments if <var>sPart</var> was splitted successfully

    @return     <TRUE/> if the identifier could be found and the string was splitted.
                <FALSE/> otherwise.
*/
bool JobURL::implst_split( /*IN*/  const OUString& sPart           ,
                               /*IN*/  const sal_Char*        pPartIdentifier ,
                               /*IN*/        sal_Int32        nPartLength     ,
                               /*OUT*/       OUString& rPartValue      ,
                               /*OUT*/       OUString& rPartArguments  )
{
    // first search for the given identifier
    bool bPartFound = (sPart.matchIgnoreAsciiCaseAsciiL(pPartIdentifier,nPartLength));

    // If it exist - we can split the part and return sal_True.
    // Otherwhise we do nothing and return sal_False.
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

/**
    @short      special debug method
    @descr      It's the entry point method to start a self component check for this class.
                It's used for internal purposes only and never a part of a legal product.
                Use it for testing and debug only!
*/
#ifdef ENABLE_COMPONENT_SELF_CHECK


void JobURL::impldbg_checkIt()
{
    // check simple URL's
    JobURL::impldbg_checkURL("vnd.sun.star.job:event=onMyEvent"    , E_EVENT  , "onMyEvent", ""       , ""           , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:alias=myAlias"      , E_ALIAS  , ""         , "myAlias", ""           , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.Service", E_SERVICE, ""         , ""       , "css.Service", NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=;"          , E_UNKNOWN, ""         , ""       , ""           , NULL, NULL, NULL);

    // check combinations
    // Note: No additional spaces or tabs are allowed after a separator occurred.
    // Tab and spaces before a separator will be used as value!
    JobURL::impldbg_checkURL("vnd.sun.star.job:event=onMyEvent;alias=myAlias;service=css.Service"  , E_EVENT | E_ALIAS | E_SERVICE , "onMyEvent", "myAlias", "css.Service" , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.Service;alias=myAlias"                  , E_ALIAS | E_SERVICE           , ""         , "myAlias", "css.Service" , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.Service;alias=myAlias"                 , E_ALIAS | E_SERVICE           , ""         , "myAlias", "css.Service ", NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.Service; alias=myAlias"                 , E_UNKNOWN                     , ""         , ""       , ""            , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job : event=onMyEvent"                                  , E_UNKNOWN                     , ""         , ""       , ""            , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:event=onMyEvent;event=onMyEvent;service=css.Service", E_UNKNOWN                     , ""         , ""       , ""            , NULL, NULL, NULL);

    // check upper/lower case
    // fix parts of the URL are case insensitive (e.g. "vnd.SUN.star.job:eVEnt=")
    // values are case sensitive                 (e.g. "myAlias"                )
    JobURL::impldbg_checkURL("vnd.SUN.star.job:eVEnt=onMyEvent;aliAs=myAlias;serVice=css.Service", E_EVENT | E_ALIAS | E_SERVICE , "onMyEvent", "myAlias", "css.Service" , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.SUN.star.job:eVEnt=onMyEVENT;aliAs=myALIAS;serVice=css.SERVICE", E_EVENT | E_ALIAS | E_SERVICE , "onMyEVENT", "myALIAS", "css.SERVICE" , NULL, NULL, NULL);

    // check stupid URLs
    JobURL::impldbg_checkURL("vnd.sun.star.jobs:service=css.Service"    , E_UNKNOWN, "", "", "", NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job service=css.Service"     , E_UNKNOWN, "", "", "", NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service;css.Service"     , E_UNKNOWN, "", "", "", NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service;"                , E_UNKNOWN, "", "", "", NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:;alias;service;event="   , E_UNKNOWN, "", "", "", NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:alias=a;service=s;event=", E_UNKNOWN, "", "", "", NULL, NULL, NULL);

    // check argument handling
    JobURL::impldbg_checkURL("vnd.sun.star.job:event=onMyEvent?eventArg1,eventArg2=3,eventArg4,"            , E_EVENT          , "onMyEvent", ""       , ""             , "eventArg1,eventArg2=3,eventArg4,", NULL                 , NULL          );
    JobURL::impldbg_checkURL("vnd.sun.star.job:alias=myAlias?aliasArg1,aliasarg2"                           , E_EVENT          , ""         , "myAlias", ""             , NULL                              , "aliasArg1,aliasarg2", NULL          );
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.myService?serviceArg1"                           , E_EVENT          , ""         , ""       , "css.myService", NULL                              , NULL                 , "serviceArg1" );
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.myService?serviceArg1;alias=myAlias?aliasArg=564", E_EVENT | E_ALIAS, ""         , "myAlias", "css.myService", NULL                              , "aliasArg=564"       , "serviceArg1" );
}

/**
    @short      helper debug method
    @descr      It uses the given parameter to create a new instance of a JobURL.
                They results will be compared with the expected ones.
                The a log will be written, which contains some detailed information
                for this sub test.

    @param      pURL
                    the job URL, which should be checked

    @param      eExpectedPart
                    the expected result

    @param      pExpectedEvent
                    the expected event value

    @param      pExpectedAlias
                    the expected alias value

    @param      pExpectedService
                    the expected service value

    @param      pExpectedEventArgs
                    the expected event arguments

    @param      pExpectedAliasArgs
                    the expected alias arguments

    @param      pExpectedServiceArgs
                    the expected service arguments
*/
void JobURL::impldbg_checkURL( /*IN*/ const sal_Char*  pURL                 ,
                               /*IN*/       sal_uInt32 eExpectedPart        ,
                               /*IN*/ const sal_Char*  pExpectedEvent       ,
                               /*IN*/ const sal_Char*  pExpectedAlias       ,
                               /*IN*/ const sal_Char*  pExpectedService     ,
                               /*IN*/ const sal_Char*  pExpectedEventArgs   ,
                               /*IN*/ const sal_Char*  pExpectedAliasArgs   ,
                               /*IN*/ const sal_Char*  pExpectedServiceArgs )
{
    OUString sEvent;
    OUString sAlias;
    OUString sService;
    OUString sEventArgs;
    OUString sAliasArgs;
    OUString sServiceArgs;
    OUString sURL    (OUString::createFromAscii(pURL));
    sal_Bool        bOK     = sal_True;

    JobURL aURL(sURL);

    // check if URL is invalid
    if (eExpectedPart==E_UNKNOWN)
        bOK = !aURL.isValid();

    // check if URL has the expected event part
    if (
        (bOK                                 ) &&
        ((eExpectedPart & E_EVENT) == E_EVENT)
       )
    {
        bOK = (
                (aURL.isValid()                          ) &&
                (aURL.getEvent(sEvent)                   ) &&
                (!sEvent.isEmpty()                       ) &&
                (sEvent.equalsAscii(pExpectedEvent))
              );

        if (bOK && pExpectedEventArgs!=NULL)
        {
            bOK = (
                    (aURL.getEventArgs(sEventArgs)                   ) &&
                    (sEventArgs.equalsAscii(pExpectedEventArgs))
                  );
        }
    }

    // check if URL has no event part
    if (
        (bOK                                 ) &&
        ((eExpectedPart & E_EVENT) != E_EVENT)
       )
    {
        bOK = (
                (!aURL.getEvent(sEvent)        ) &&
                (sEvent.isEmpty()              ) &&
                (!aURL.getEventArgs(sEventArgs)) &&
                (sEventArgs.isEmpty()          )
              );
    }

    // check if URL has the expected alias part
    if (
        (bOK                                 ) &&
        ((eExpectedPart & E_ALIAS) == E_ALIAS)
       )
    {
        bOK = (
                (aURL.isValid()                          ) &&
                (aURL.getAlias(sAlias)                   ) &&
                (!sAlias.isEmpty()                       ) &&
                (sAlias.equalsAscii(pExpectedAlias))
              );

        if (bOK && pExpectedAliasArgs!=NULL)
        {
            bOK = (
                    (aURL.getAliasArgs(sAliasArgs)                   ) &&
                    (sAliasArgs.equalsAscii(pExpectedAliasArgs))
                  );
        }
    }

    // check if URL has the no alias part
    if (
        (bOK                                 ) &&
        ((eExpectedPart & E_ALIAS) != E_ALIAS)
       )
    {
        bOK = (
                (!aURL.getAlias(sAlias)        ) &&
                (sAlias.isEmpty()              ) &&
                (!aURL.getAliasArgs(sAliasArgs)) &&
                (sAliasArgs.isEmpty()          )
              );
    }

    // check if URL has the expected service part
    if (
        (bOK                                     ) &&
        ((eExpectedPart & E_SERVICE) == E_SERVICE)
       )
    {
        bOK = (
                (aURL.isValid()                              ) &&
                (aURL.getService(sService)                   ) &&
                (!sService.isEmpty()                         ) &&
                (sService.equalsAscii(pExpectedService))
              );

        if (bOK && pExpectedServiceArgs!=NULL)
        {
            bOK = (
                    (aURL.getServiceArgs(sServiceArgs)                   ) &&
                    (sServiceArgs.equalsAscii(pExpectedServiceArgs))
                  );
        }
    }

    // check if URL has the no service part
    if (
        (bOK                                     ) &&
        ((eExpectedPart & E_SERVICE) != E_SERVICE)
       )
    {
        bOK = (
                (!aURL.getService(sService)        ) &&
                (sService.isEmpty()                ) &&
                (!aURL.getServiceArgs(sServiceArgs)) &&
                (sServiceArgs.isEmpty()            )
              );
    }

    OUStringBuffer sMsg(256);

    sMsg.append(sURL);
    sMsg.appendAscii("\" ");

    if (bOK)
    {
        sMsg.appendAscii("... OK\n");
    }
    else
    {
        sMsg.appendAscii("... failed. ");
        sMsg.appendAscii("Expected: ");
        if (eExpectedPart==E_UNKNOWN)
            sMsg.appendAscii("E_UNKNOWN");
        if ((eExpectedPart & E_EVENT) == E_EVENT)
        {
            sMsg.appendAscii("| E_EVENT e=\"");
            sMsg.appendAscii(pExpectedEvent  );
            sMsg.appendAscii("\""            );
        }
        if ((eExpectedPart & E_ALIAS) == E_ALIAS)
        {
            sMsg.appendAscii("| E_ALIAS a=\"");
            sMsg.appendAscii(pExpectedAlias  );
            sMsg.appendAscii("\""            );
        }
        if ((eExpectedPart & E_SERVICE) == E_SERVICE)
        {
            sMsg.appendAscii("| E_SERVICE s=\"");
            sMsg.appendAscii(pExpectedService  );
            sMsg.appendAscii("\""              );
        }
        sMsg.appendAscii(", Actual: "     );
        sMsg.append     (aURL.impldbg_toString());
    }

    SAL_INFO("fwk.joburl", OUString(sMsg));
}

/**
    @short      helper debug method
    @descr      It returns a representation of the internal object state
                as string notation.

    @returns    The formatted string representation.
*/
OUString JobURL::impldbg_toString() const
{
    SolarMutexGuard g;

    OUStringBuffer sBuffer(256);

    if (m_eRequest==E_UNKNOWN)
        sBuffer.appendAscii("E_UNKNOWN");
    if ((m_eRequest & E_EVENT) == E_EVENT)
        sBuffer.appendAscii("| E_EVENT");
    if ((m_eRequest & E_ALIAS) == E_ALIAS)
        sBuffer.appendAscii("| E_ALIAS");
    if ((m_eRequest & E_SERVICE) == E_SERVICE)
        sBuffer.appendAscii("| E_SERVICE");
    sBuffer.appendAscii("{ e=\""   );
    sBuffer.append     (m_sEvent   );
    sBuffer.appendAscii("\" - a=\"");
    sBuffer.append     (m_sAlias   );
    sBuffer.appendAscii("\" - s=\"");
    sBuffer.append     (m_sService );
    sBuffer.appendAscii("\" }"     );

    return sBuffer.makeStringAndClear();
}

sal_Bool JobURL::getServiceArgs( /*OUT*/ OUString& sServiceArgs ) const
{
    SolarMutexGuard g;

             sServiceArgs.clear();
    sal_Bool bSet         = ((m_eRequest & E_SERVICE) == E_SERVICE);
    if (bSet)
        sServiceArgs = m_sServiceArgs;

    return bSet;
}

sal_Bool JobURL::getEventArgs( /*OUT*/ OUString& sEventArgs ) const
{
    SolarMutexGuard g;

             sEventArgs.clear();
    sal_Bool bSet       = ((m_eRequest & E_EVENT) == E_EVENT);
    if (bSet)
        sEventArgs = m_sEventArgs;

    return bSet;
}

sal_Bool JobURL::getAliasArgs( /*OUT*/ OUString& sAliasArgs ) const
{
    SolarMutexGuard g;

             sAliasArgs.clear();
    sal_Bool bSet       = ((m_eRequest & E_ALIAS) == E_ALIAS);
    if (bSet)
        sAliasArgs = m_sAliasArgs;

    return bSet;
}

#endif // ENABLE_COMPONENT_SELF_CHECK

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
