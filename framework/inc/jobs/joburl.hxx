/*************************************************************************
 *
 *  $RCSfile: joburl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:48 $
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

#ifndef __FRAMEWORK_JOBS_JOBURL_HXX_
#define __FRAMEWORK_JOBS_JOBURL_HXX_

//_______________________________________
// my own includes

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________
// interface includes

//_______________________________________
// other includes

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
        sal_Bool getEventArgs  (       ::rtl::OUString& sEventArgs   ) const;
        sal_Bool getAliasArgs  (       ::rtl::OUString& sAliasArgs   ) const;
        sal_Bool getServiceArgs(       ::rtl::OUString& sServiceArgs ) const;

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

    #endif // ENABLE_COMPONENT_SELF_CHECK
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBURL_HXX_
