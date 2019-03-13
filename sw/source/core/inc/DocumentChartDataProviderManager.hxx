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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTCHARTDATAPROVIDEMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTCHARTDATAPROVIDEMANAGER_HXX

#include <IDocumentChartDataProviderAccess.hxx>
#include <o3tl/deleter.hxx>
#include <rtl/ref.hxx>
#include <memory>

class SwTable;
class SwChartDataProvider;
class SwChartLockController_Helper;
class SwDoc;


namespace sw {

class DocumentChartDataProviderManager : public IDocumentChartDataProviderAccess
{

public:

    DocumentChartDataProviderManager( SwDoc& i_rSwdoc );

    SwChartDataProvider * GetChartDataProvider( bool bCreate = false ) const override;

    void CreateChartInternalDataProviders( const SwTable *pTable ) override;

    SwChartLockController_Helper & GetChartControllerHelper() override;

    virtual ~DocumentChartDataProviderManager() override;

private:

    DocumentChartDataProviderManager(DocumentChartDataProviderManager const&) = delete;
    DocumentChartDataProviderManager& operator=(DocumentChartDataProviderManager const&) = delete;

    SwDoc& m_rDoc;

    mutable rtl::Reference<SwChartDataProvider> maChartDataProviderImplRef;
    std::unique_ptr<SwChartLockController_Helper, o3tl::default_delete<SwChartLockController_Helper>> mpChartControllerHelper;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
