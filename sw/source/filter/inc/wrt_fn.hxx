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
#ifndef INCLUDED_SW_SOURCE_FILTER_INC_WRT_FN_HXX
#define INCLUDED_SW_SOURCE_FILTER_INC_WRT_FN_HXX
#include "hintids.hxx"

// einige Forward-Deklarationen
class SwNode;
class SwContentNode;
class Writer;
class SfxPoolItem;
class SfxItemSet;

/* Funktionspointer auf die Attribut-Write-Funktionen */
typedef Writer& (*FnAttrOut)( Writer&, const SfxPoolItem& );
typedef FnAttrOut SwAttrFnTab[ POOLATTR_END - POOLATTR_BEGIN ];

SW_DLLPUBLIC Writer& Out( const SwAttrFnTab, const SfxPoolItem&, Writer& );
SW_DLLPUBLIC Writer& Out_SfxItemSet( const SwAttrFnTab, Writer&, const SfxItemSet&,
                         bool bDeep, bool bTstForDefault = true );

/* Funktionspointer auf die Node-Write-Funktionen */

enum RES_NODE
{
RES_NODE_BEGIN = 0,
    RES_TXTNODE = RES_NODE_BEGIN,
    RES_GRFNODE,
    RES_OLENODE,
RES_NODE_END
};

typedef Writer& (*FnNodeOut)( Writer&, SwContentNode& );
typedef FnNodeOut SwNodeFnTab[ RES_NODE_END - RES_NODE_BEGIN ];

SW_DLLPUBLIC Writer& Out( const SwNodeFnTab, SwNode&, Writer & rWrt );

#endif // INCLUDED_SW_SOURCE_FILTER_INC_WRT_FN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
