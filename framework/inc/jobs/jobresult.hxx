/*************************************************************************
 *
 *  $RCSfile: jobresult.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-10-11 13:41:08 $
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

#ifndef __FRAMEWORK_JOBS_JOBRESULT_HXX_
#define __FRAMEWORK_JOBS_JOBRESULT_HXX_

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

#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTEVENT_HPP_
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif

//_______________________________________
// other includes

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//_______________________________________
// namespace

namespace framework{

//_______________________________________
// public const

//_______________________________________
/**
    @short  represent a result of a finished job execution
    @descr  Such result instance transport all neccessarry
            data from the code place where the job was finished
            to the outside code, where e.g. listener must be notified.

    @see    Job
    @see    com::sun::star::frame::ProtocolHandler
 */
class JobResult : private ThreadHelpBase
{
    //___________________________________
    // types

    private:

        enum EEventType
        {
            E_UNKNOWN        ,
            E_FEATURESTATE   ,
            E_DISPATCHRESULT ,
            E_BOTH
        };

    private:
        EEventType                      m_eEventType     ;
        css::frame::FeatureStateEvent   m_aFeatureState  ;
        css::frame::DispatchResultEvent m_aDispatchResult;

    public:
    JobResult()
    {
        m_eEventType = E_UNKNOWN;
    }

    JobResult( const css::frame::FeatureStateEvent& aState )
    {
        m_eEventType    = E_FEATURESTATE;
        m_aFeatureState = aState;
    }

    JobResult( const css::frame::DispatchResultEvent& aResult )
    {
        m_eEventType      = E_DISPATCHRESULT;
        m_aDispatchResult = aResult;
    }

    JobResult( const css::frame::FeatureStateEvent&   aState  ,
               const css::frame::DispatchResultEvent& aResult )
    {
        m_eEventType      = E_BOTH ;
        m_aFeatureState   = aState ;
        m_aDispatchResult = aResult;
    }

    void sendFeatureState( const ListenerHash& lListener ) const
    {
        /* TODO */
        LOG_WARNING("JobHandler::addStatusListener()", "not yet implemented")
    }

    void sendDispatchResult( const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) const
    {
        if (
            (xListener.is()) &&
            (
                (m_eEventType==E_DISPATCHRESULT) ||
                (m_eEventType==E_BOTH          )
            )
           )
        {
            xListener->dispatchFinished(m_aDispatchResult);
        }
    }
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBRESULT_HXX_
