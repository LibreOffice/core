/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#define INCLUDED_SVTOOLS_HELPOPT_HXX

#include "svtools/svtdllapi.h"

#ifndef INCLUDED_LIST
#include <list>
#define INCLUDED_LIST
#endif
#include <tools/string.hxx>
#include <unotools/options.hxx>

typedef std::list< sal_Int32 > IdList;

class SvtHelpOptions_Impl;

class SVT_DLLPUBLIC SvtHelpOptions: public utl::detail::Options
{
    SvtHelpOptions_Impl*    pImp;

public:
                    SvtHelpOptions();
                    virtual ~SvtHelpOptions();

    void            SetExtendedHelp( sal_Bool b );
    sal_Bool        IsExtendedHelp() const;
    void            SetHelpTips( sal_Bool b );
    sal_Bool        IsHelpTips() const;

    void            SetHelpAgentAutoStartMode( sal_Bool b );
    sal_Bool        IsHelpAgentAutoStartMode() const;
    void            SetHelpAgentTimeoutPeriod( sal_Int32 _nSeconds );
    sal_Int32       GetHelpAgentTimeoutPeriod( ) const;
    void            SetHelpAgentRetryLimit( sal_Int32 _nTrials );
    sal_Int32       GetHelpAgentRetryLimit( ) const;

    const String&   GetHelpStyleSheet()const;
    void            SetHelpStyleSheet(const String& rStyleSheet);

    /** retrieves the help agent's ignore counter for the given URL.
        <p> If the counter returned 0, the agent should silently drop any requests for this URL.<br/>
            If the counter is greater 0, the agent should display the URL and, if the user ignores it,
            decrement the counter by 1.
        </p>
    */
    sal_Int32       getAgentIgnoreURLCounter( const ::rtl::OUString& _rURL );
    /** decrements the help agent's ignore counter for the given URL
        @see getAgentIgnoreURLCounter
    */
    void            decAgentIgnoreURLCounter( const ::rtl::OUString& _rURL );
    /** resets the help agent's ignore counter for the given URL
    */
    void            resetAgentIgnoreURLCounter( const ::rtl::OUString& _rURL );
    /** resets the help agent's ignore counter for all URL's
    */
    void            resetAgentIgnoreURLCounter();

    void            SetWelcomeScreen( sal_Bool b );
    sal_Bool        IsWelcomeScreen() const;

    IdList*         GetPIStarterList();
    void            AddToPIStarterList( sal_Int32 nId );
    void            RemoveFromPIStarterList( sal_Int32 nId );

    String          GetLocale() const;
    String          GetSystem() const;
};

#endif

