/*************************************************************************
 *
 *  $RCSfile: jobresult.hxx,v $
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

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTEVENT_HPP_
#include <com/sun/star/frame/DispatchResultEvent.hpp>
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
 */
class JobResult : private ThreadHelpBase
{
    //___________________________________
    // types

    public:

        /**
            These enum values are used to build a flag mask of possible set
            parts of an analyzed pure job execution result.
            An user of this class can decide, if a member of us can be valid
            or not. So it can indicate, if a job used the special part inside
            his returned result protocol.
            To be usable as flags - it must be values of set {0,1,2,4,8,16 ...}!
         */
        enum EParts
        {
            E_NOPART            =   0,
            E_ARGUMENTS         =   1,
            E_DEACTIVATE        =   2,
            E_DISPATCHRESULT    =   4
        };

    //___________________________________
    // member

    private:

        /** hold the original pure result, which was given back by an
            executed job
            We analyze it and use it to set all our other members.
         */
        css::uno::Any m_aPureResult;

        /**
            an user of us must know, which (possible) parts of
            a "pure result" was realy set by an executed job.
            Means which other members of this class are valid.
            This mask can be used to find it out.
         */
        sal_uInt32 m_eParts;

        /**
            a job can have persistent data
            They are part of the pure result and will be used to
            write it to the configuration. But that's part of any
            user of us. We provide this information here only.
         */
        css::uno::Sequence< css::beans::NamedValue > m_lArguments;

        /**
            an executed job can force his deactivation
            But we provide this information here only.
            Doing so is part of any user of us.
         */
        sal_Bool m_bDeactivate;

        /**
            represent the part "DispatchResult"
            It's a full filled event type, which was given
            back by the executed job. Any user of us can send
            it to his registered result listener directly.
         */
        css::frame::DispatchResultEvent m_aDispatchResult;

    //___________________________________
    // native interface

    public:

                 JobResult(                                         );
                 JobResult( const com::sun::star::uno::Any& aResult );
                 JobResult( const JobResult&                rCopy   );
        virtual ~JobResult(                                         );

        void operator=( const JobResult& rCopy );

        sal_Bool                                     existPart        ( sal_uInt32 eParts ) const;
        css::uno::Sequence< css::beans::NamedValue > getArguments     (                   ) const;
        sal_Bool                                     getDeactivate    (                   ) const;
        css::frame::DispatchResultEvent              getDispatchResult(                   ) const;
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBRESULT_HXX_
