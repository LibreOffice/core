/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CFGITEMS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CFGITEMS_HXX

#include <svl/poolitem.hxx>
#include <swdllapi.h>
#include <printdata.hxx>

#include <cmdid.h>
#include <sfx2/zoomitem.hxx>

// OS 12.01.95
// Item for settings dialog - printer/add-ons
class SW_DLLPUBLIC SwAddPrinterItem final : public SfxPoolItem, public SwPrintData
{
    using  SwPrintData::operator ==;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SwAddPrinterItem)
    SwAddPrinterItem();
    SwAddPrinterItem( const SwPrintData& rPrtData );

    virtual SwAddPrinterItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool         operator==( const SfxPoolItem& ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
