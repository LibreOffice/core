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

#include <viewopt.hxx>
#include <cmdid.h>
#include <cfgitems.hxx>
#include <crstate.hxx>

// CTOR for empty Item
SwAddPrinterItem::SwAddPrinterItem():
                SfxPoolItem(FN_PARAM_ADDPRINTER)
{
}

// CTOR from SwPrintOptions
SwAddPrinterItem::SwAddPrinterItem( const SwPrintData& rPrtData ) :
    SfxPoolItem(FN_PARAM_ADDPRINTER)
{
    SwPrintData::operator=(rPrtData);
}

SwAddPrinterItem* SwAddPrinterItem::Clone( SfxItemPool* ) const
{
    return new SwAddPrinterItem( *this );
}

bool SwAddPrinterItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SwAddPrinterItem& rItem = static_cast<const SwAddPrinterItem&>(rAttr);

    return  SwPrintData::operator==(rItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
