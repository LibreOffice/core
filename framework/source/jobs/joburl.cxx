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
    #ifdef ENABLE_COMPONENT_SELF_CHECK
    JobURL::impldbg_checkIt();
    #endif

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

//________________________________
/**
    @short      special debug method
    @descr      It's the entry point method to start a self component check for this class.
                It's used for internal purposes only and never a part of a legal product.
                Use it for testing and debug only!
*/
#ifdef ENABLE_COMPONENT_SELF_CHECK

#define LOGFILE_JOBURL  "joburl.log"

void JobURL::impldbg_checkIt()
{
    // check simple URL's
    JobURL::impldbg_checkURL("vnd.sun.star.job:event=onMyEvent"    , E_EVENT  , "onMyEvent", ""       , ""           , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:alias=myAlias"      , E_ALIAS  , ""         , "myAlias", ""           , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.Service", E_SERVICE, ""         , ""       , "css.Service", NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=;"          , E_UNKNOWN, ""         , ""       , ""           , NULL, NULL, NULL);

    // check combinations
    // Note: No additional spaces or tabs are allowed after a seperator occurred.
    // Tab and spaces before a seperator will be used as value!
    JobURL::impldbg_checkURL("vnd.sun.star.job:event=onMyEvent;alias=myAlias;service=css.Service"  , E_EVENT | E_ALIAS | E_SERVICE , "onMyEvent", "myAlias", "css.Service" , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.Service;alias=myAlias"                  , E_ALIAS | E_SERVICE           , ""         , "myAlias", "css.Service" , NULL, NULL, NULL);
    JobURL::impldbg_checkURL("vnd.sun.star.job:service=css.Service ;alias=myAlias"                 , E_ALIAS | E_SERVICE           , ""         , "myAlias", "css.Service ", NULL, NULL, NULL);
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

//________________________________
/**
    @short      helper debug method
    @descr      It uses the given parameter to create a new instance of a JobURL.
                They results will be compared with the exepected ones.
                The a log will be written, which contains some detailed informations
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
    ::rtl::OUString sEvent      ;
    ::rtl::OUString sAlias      ;
    ::rtl::OUString sService    ;
    ::rtl::OUString sEventArgs  ;
    ::rtl::OUString sAliasArgs  ;
    ::rtl::OUString sServiceArgs;
    ::rtl::OUString sURL    (::rtl::OUString::createFromAscii(pURL));
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
                (sEvent.compareToAscii(pExpectedEvent)==0)
              );

        if (bOK && pExpectedEventArgs!=NULL)
        {
            bOK = (
                    (aURL.getEventArgs(sEventArgs)                   ) &&
                    (sEventArgs.compareToAscii(pExpectedEventArgs)==0)
                  );
        };
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
                (sAlias.compareToAscii(pExpectedAlias)==0)
              );

        if (bOK && pExpectedAliasArgs!=NULL)
        {
            bOK = (
                    (aURL.getAliasArgs(sAliasArgs)                   ) &&
                    (sAliasArgs.compareToAscii(pExpectedAliasArgs)==0)
                  );
        };
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
                (sService.compareToAscii(pExpectedService)==0)
              );

        if (bOK && pExpectedServiceArgs!=NULL)
        {
            bOK = (
                    (aURL.getServiceArgs(sServiceArgs)                   ) &&
                    (sServiceArgs.compareToAscii(pExpectedServiceArgs)==0)
                  );
        };
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

    ::rtl::OUStringBuffer sMsg(256);

    sMsg.appendAscii("\"" );
    sMsg.append     (sURL );
    sMsg.appendAscii("\" ");

    if (bOK)
    {
        sMsg.appendAscii("... OK\n");
    }
    else
    {
        sMsg.appendAscii("... failed\n");
        sMsg.appendAscii("expected was: ");
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
        sMsg.appendAscii("\tbut it was  : "     );
        sMsg.append     (aURL.impldbg_toString());
        sMsg.appendAscii("\n"                   );
    }

    WRITE_LOGFILE(LOGFILE_JOBURL, U2B(sMsg.makeStringAndClear()))
}

//________________________________
/**
    @short      helper debug method
    @descr      It returns a representation of the internal object state
                as string notation.

    @returns    The formated string representation.
*/
::rtl::OUString JobURL::impldbg_toString() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    ::rtl::OUStringBuffer sBuffer(256);

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

    aReadLock.unlock();
    /* } SAFE */

    return sBuffer.makeStringAndClear();
}

//________________________________

sal_Bool JobURL::getServiceArgs( /*OUT*/ ::rtl::OUString& sServiceArgs ) const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

             sServiceArgs = ::rtl::OUString();
    sal_Bool bSet         = ((m_eRequest & E_SERVICE) == E_SERVICE);
    if (bSet)
        sServiceArgs = m_sServiceArgs;

    aReadLock.unlock();
    /* } SAFE */

    return bSet;
}

//________________________________

sal_Bool JobURL::getEventArgs( /*OUT*/ ::rtl::OUString& sEventArgs ) const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

             sEventArgs = ::rtl::OUString();
    sal_Bool bSet       = ((m_eRequest & E_EVENT) == E_EVENT);
    if (bSet)
        sEventArgs = m_sEventArgs;

    aReadLock.unlock();
    /* } SAFE */

    return bSet;
}

//________________________________

sal_Bool JobURL::getAliasArgs( /*OUT*/ ::rtl::OUString& sAliasArgs ) const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

             sAliasArgs = ::rtl::OUString();
    sal_Bool bSet       = ((m_eRequest & E_ALIAS) == E_ALIAS);
    if (bSet)
        sAliasArgs = m_sAliasArgs;

    aReadLock.unlock();
    /* } SAFE */

    return bSet;
}

#endif // ENABLE_COMPONENT_SELF_CHECK

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
