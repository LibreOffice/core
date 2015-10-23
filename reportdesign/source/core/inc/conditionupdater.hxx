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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_CONDITIONUPDATER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_CONDITIONUPDATER_HXX

#include "conditionalexpression.hxx"

#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/report/XReportControlModel.hpp>

#include <boost/noncopyable.hpp>


namespace rptui
{



    //= ConditionUpdater

    class ConditionUpdater : public ::boost::noncopyable
    {
    public:
        ConditionUpdater();
        ~ConditionUpdater();

        /// notifies the object about the change of a property value, somewhere in the report definition
        void    notifyPropertyChange( const css::beans::PropertyChangeEvent& _rEvent );

    private:
        /// does late initializations, return whether or not successful
        bool impl_lateInit_nothrow();

        void impl_adjustFormatConditions_nothrow(
            const css::uno::Reference< css::report::XReportControlModel >& _rxRptControlModel,
            const OUString& _rOldDataSource,
            const OUString& _rNewDataSource
        );

    private:
        ConditionalExpressions  m_aConditionalExpressions;
    };


} // namespace rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_CONDITIONUPDATER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
