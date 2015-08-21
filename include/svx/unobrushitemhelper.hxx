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
#ifndef _UNOBRUSHITEMHELPER_HXX
#define _UNOBRUSHITEMHELPER_HXX

#include "svx/svxdllapi.h"
#include <editeng/brushitem.hxx>

//UUUU Helper function definintions for UNO API fallbacks to replace SvxBrushItem. The
// idea is to have fallbacks to create a SvxBrushItem if needed for backwards compatibility
// if needed from the SfxItemSet and vice versa. This is used in cases where e.g. UNO API
// accesses to slots in the SvxBrushItem are used (see cases in SvxBrushItem::QueryValue
// and SvxBrushItem::PutValue as MID_BACK_COLOR and similar).
// To make this work, a cycle of creating a SvxBrushItem from a SfxItemSet, changing a value
// using PutValue, putting back to the SfxItemSet should create the *same* SvxBrushItem
// the next time this will be created using getSvxBrushItemFromSourceSet. For more details,
// see comments at the implementations of that two methods.

// Set the equivalent in the range [XATTR_FILL_FIRST .. XATTR_FILL_LAST] in the given
// SfxItemSet to create the same FillStyle as is expressed by the given SvxBrushItem.
// This method will reset all items in the XATTR_FILL_* range first.
SVX_DLLPUBLIC void setSvxBrushItemAsFillAttributesToTargetSet(
    const SvxBrushItem& rBrush,
    SfxItemSet& rToSet);

// Create a SvxBrushItem as close as possible to the settings in the DrawingLayer
// items in the range [XATTR_FILL_FIRST .. XATTR_FILL_LAST]. Since this is not 100%
// representable this may lead to reduced data. With nBackgroundID a Which-ID for the
// to-be-created SvxBrushItem has to be given (default should be 99 as in RES_BACKGROUND).
SVX_DLLPUBLIC SvxBrushItem getSvxBrushItemFromSourceSet(
    const SfxItemSet& rSourceSet,
    sal_uInt16 nBackgroundID,
    bool bSearchInParents = true,
    bool bXMLImportHack = false);

#endif // _UNOBRUSHITEMHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
