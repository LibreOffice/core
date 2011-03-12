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

#ifndef __FRAMEWORK_JOBS_JOBRESULT_HXX_
#define __FRAMEWORK_JOBS_JOBRESULT_HXX_

//_______________________________________
// my own includes

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <stdtypes.h>
#include <general.h>

//_______________________________________
// interface includes
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>

//_______________________________________
// other includes
#include <rtl/ustring.hxx>

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
        css::frame::DispatchResultEvent              getDispatchResult(                   ) const;
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBRESULT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
