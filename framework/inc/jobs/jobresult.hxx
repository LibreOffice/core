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

#ifndef INCLUDED_FRAMEWORK_INC_JOBS_JOBRESULT_HXX
#define INCLUDED_FRAMEWORK_INC_JOBS_JOBRESULT_HXX

#include <stdtypes.h>
#include <general.h>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>

#include <rtl/ustring.hxx>

namespace framework{

/**
    @short  represent a result of a finished job execution
    @descr  Such result instance transport all necessary
            data from the code place where the job was finished
            to the outside code, where e.g. listener must be notified.
 */
class JobResult final
{

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

    // member

    private:

        /**
            an user of us must know, which (possible) parts of
            a "pure result" was really set by an executed job.
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
        std::vector< css::beans::NamedValue > m_lArguments;

        /**
            represent the part "DispatchResult"
            It's a fulfilled event type, which was given
            back by the executed job. Any user of us can send
            it to his registered result listener directly.
         */
        css::frame::DispatchResultEvent m_aDispatchResult;

    // native interface

    public:

                 JobResult( const css::uno::Any& aResult );
                 JobResult( const JobResult&                rCopy   );
                 ~JobResult(                                         );

        JobResult& operator=( const JobResult& rCopy );

        bool                                     existPart        ( sal_uInt32 eParts ) const;
        std::vector< css::beans::NamedValue >    getArguments     (                   ) const;
        css::frame::DispatchResultEvent          getDispatchResult(                   ) const;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_JOBS_JOBRESULT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
