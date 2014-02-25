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

 #ifndef INCLUDED_SW_INC_IDOCUMENTCHARTDATAPROVIDERACCESS_HXX
 #define INCLUDED_SW_INC_IDOCUMENTCHARTDATAPROVIDERACCESS_HXX

#include "com/sun/star/uno/Reference.hxx"

namespace com { namespace sun { namespace star { namespace frame {
    class XModel;
}}}}

class SwTable;
class SwChartDataProvider;
class SwChartLockController_Helper;

 /** Give access to the data-provider for chart
 */
 class IDocumentChartDataProviderAccess
 {
 public:

    /**
        returns or creates the data-provider for chart
    */
    virtual SwChartDataProvider * GetChartDataProvider( bool bCreate = false ) const = 0;

    /**
        calls createInternalDataProvider for all charts using the specified table
    */
    virtual void CreateChartInternalDataProviders( const SwTable *pTable ) = 0;

    /**
        returns chart controller helper that is used to lock/unlock all
        charts when relevant parts of tables are changed
    */
    virtual SwChartLockController_Helper & GetChartControllerHelper() = 0;

 protected:
    virtual ~IDocumentChartDataProviderAccess() {};
 };

 #endif // INCLUDED_SW_INC_IDOCUMENTCHARTDATAPROVIDERACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
