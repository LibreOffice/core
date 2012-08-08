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

#ifndef __FRAMEWORK_JOBS_JOBURL_HXX_
#define __FRAMEWORK_JOBS_JOBURL_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <stdtypes.h>
#include <general.h>

#include <rtl/ustring.hxx>

//_______________________________________
// namespace

namespace framework{

//_______________________________________
// const

#define JOBURL_PROTOCOL_STR        "vnd.sun.star.job:"
#define JOBURL_PROTOCOL_LEN        17

#define JOBURL_EVENT_STR           "event="
#define JOBURL_EVENT_LEN           6

#define JOBURL_ALIAS_STR           "alias="
#define JOBURL_ALIAS_LEN           6

#define JOBURL_SERVICE_STR         "service="
#define JOBURL_SERVICE_LEN         8

#define JOBURL_PART_SEPERATOR      ';'
#define JOBURL_PARTARGS_SEPERATOR  ','

//_______________________________________
/**
    @short  can be used to parse, validate and work with job URL's
    @descr  Job URLs are specified by the following syntax:
                vnd.sun.star.job:{[event=<name>]|[alias=<name>]|[service=<name>]}
            This class can analyze this structure and seperate it into his different parts.
            After doing that these parts are accessible by the methods of this class.
 */
class JobURL : private ThreadHelpBase
{
    //___________________________________
    // types

    private:

        /**
            possible states of a job URL
            Note: These values are used in combination. So they must be
            values in form 2^n.
         */
        enum ERequest
        {
            /// mark a job URL as not valid
            E_UNKNOWN = 0,
            /// it's an event
            E_EVENT   = 1,
            /// it's an alias
            E_ALIAS   = 2,
            /// it's a service
            E_SERVICE = 4
        };

    //___________________________________
    // types

    private:

        /** knows the state of this URL instance */
        sal_uInt32 m_eRequest;

        /** holds the event part of a job URL */
        ::rtl::OUString m_sEvent;

        /** holds the alias part of a job URL */
        ::rtl::OUString m_sAlias;

        /** holds the service part of a job URL */
        ::rtl::OUString m_sService;

        /** holds the event arguments */
        ::rtl::OUString m_sEventArgs;

        /** holds the alias arguments */
        ::rtl::OUString m_sAliasArgs;

        /** holds the service arguments */
        ::rtl::OUString m_sServiceArgs;

    //___________________________________
    // native interface

    public:

                 JobURL        ( const ::rtl::OUString& sURL         );
        sal_Bool isValid       (                                     ) const;
        sal_Bool getEvent      (       ::rtl::OUString& sEvent       ) const;
        sal_Bool getAlias      (       ::rtl::OUString& sAlias       ) const;
        sal_Bool getService    (       ::rtl::OUString& sService     ) const;

    //___________________________________
    // private helper

    private:

        static sal_Bool implst_split( const ::rtl::OUString& sPart           ,
                                      const sal_Char*        pPartIdentifier ,
                                            sal_Int32        nPartLength     ,
                                            ::rtl::OUString& rPartValue      ,
                                            ::rtl::OUString& rPartArguments  );

    //___________________________________
    // debug methods!

    #ifdef ENABLE_COMPONENT_SELF_CHECK

    public:
        static void impldbg_checkIt();

    private:
        static void impldbg_checkURL( const sal_Char*  pURL                 ,
                                            sal_uInt32 eExpectedPart        ,
                                      const sal_Char*  pExpectedEvent       ,
                                      const sal_Char*  pExpectedAlias       ,
                                      const sal_Char*  pExpectedService     ,
                                      const sal_Char*  pExpectedEventArgs   ,
                                      const sal_Char*  pExpectedAliasArgs   ,
                                      const sal_Char*  pExpectedServiceArgs );
        ::rtl::OUString impldbg_toString() const;

        sal_Bool getServiceArgs(       ::rtl::OUString& sServiceArgs ) const;
        sal_Bool getEventArgs  (       ::rtl::OUString& sEventArgs   ) const;
        sal_Bool getAliasArgs  (       ::rtl::OUString& sAliasArgs   ) const;

    #endif // ENABLE_COMPONENT_SELF_CHECK
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBURL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
