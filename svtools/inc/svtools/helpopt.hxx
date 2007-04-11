/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helpopt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:20:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#define INCLUDED_SVTOOLS_HELPOPT_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef INCLUDED_LIST
#include <list>
#define INCLUDED_LIST
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

typedef std::list< sal_Int32 > IdList;

class SvtHelpOptions_Impl;

class SVT_DLLPUBLIC SvtHelpOptions: public svt::detail::Options
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

