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

#pragma once

#include "PropertyMap.hxx"

#include "TableManager.hxx"
#include <dmapper/resourcemodel.hxx>

#include <vector>

namespace writerfilter::dmapper {

class DomainMapper;

class TablePropertiesHandler final : public virtual SvRefBase
{
private:
    PropertyMapPtr m_pCurrentProperties;
    std::vector<css::beans::PropertyValue>* m_pCurrentInteropGrabBag;
    TableManager* m_pTableManager;

public:
    TablePropertiesHandler();

    bool sprm(Sprm & sprm);

    void SetTableManager( TableManager* pTableManager )
    {
        m_pTableManager = pTableManager;
    };

    void SetProperties( PropertyMapPtr pProperties )
    {
        m_pCurrentProperties = pProperties;
    };

    void SetInteropGrabBag(std::vector<css::beans::PropertyValue>& rValue);

private:

    void cellProps( TablePropertyMapPtr pProps )
    {
        if ( m_pTableManager )
            m_pTableManager->cellProps( pProps );
        else
            m_pCurrentProperties->InsertProps(pProps.get());
    };

    void insertRowProps( TablePropertyMapPtr pProps )
    {
        if ( m_pTableManager )
            m_pTableManager->insertRowProps( pProps );
        else
            m_pCurrentProperties->InsertProps(pProps.get());
    };

    void tableExceptionProps( TablePropertyMapPtr pProps )
    {
        if ( m_pTableManager )
            m_pTableManager->tableExceptionProps( pProps );
        else
            m_pCurrentProperties->InsertProps(pProps.get());
    };

    void insertTableProps( TablePropertyMapPtr pProps )
    {
        if ( m_pTableManager )
            m_pTableManager->insertTableProps( pProps );
        else
            m_pCurrentProperties->InsertProps(pProps.get());
    };
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
