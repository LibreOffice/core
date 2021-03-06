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

#pragma once

#include <rtl/ustring.hxx>

namespace framework{

#define JOBURL_EVENT_STR           "event="
#define JOBURL_EVENT_LEN           6

#define JOBURL_ALIAS_STR           "alias="
#define JOBURL_ALIAS_LEN           6

#define JOBURL_SERVICE_STR         "service="
#define JOBURL_SERVICE_LEN         8

#define JOBURL_PART_SEPARATOR      ';'
#define JOBURL_PARTARGS_SEPARATOR  ','

/**
    @short  can be used to parse, validate and work with job URL's
    @descr  Job URLs are specified by the following syntax:
                vnd.sun.star.job:{[event=<name>]|[alias=<name>]|[service=<name>]}
            This class can analyze this structure and separate it into his different parts.
            After doing that these parts are accessible by the methods of this class.
 */
class JobURL
{

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

    // types

    private:

        /** knows the state of this URL instance */
        sal_uInt32 m_eRequest;

        /** holds the event part of a job URL */
        OUString m_sEvent;

        /** holds the alias part of a job URL */
        OUString m_sAlias;

        /** holds the service part of a job URL */
        OUString m_sService;

    // native interface

    public:

                 JobURL        ( const OUString& sURL         );
        bool isValid       (                                     ) const;
        bool getEvent      (       OUString& sEvent       ) const;
        bool getAlias      (       OUString& sAlias       ) const;
        bool getService    (       OUString& sService     ) const;

    // private helper

    private:

        static bool implst_split( const OUString& sPart           ,
                                      const char*        pPartIdentifier ,
                                            sal_Int32        nPartLength     ,
                                            OUString& rPartValue      ,
                                            OUString& rPartArguments  );
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
