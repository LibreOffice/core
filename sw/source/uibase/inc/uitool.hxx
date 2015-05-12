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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UITOOL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UITOOL_HXX

#include <tools/wintypes.hxx>
#include <vcl/field.hxx>
#include <swtypes.hxx>
#include "swdllapi.h"

class MetricFormatter;
class SfxItemSet;
class SwPageDesc;
class SvxTabStopItem;
class SwWrtShell;
class ListBox;
class SwDocShell;
class SwFrameFormat;
class SwTabCols;
class DateTime;
class SfxViewFrame;

// switch a metric
SW_DLLPUBLIC void SetMetric(MetricFormatter& rCtrl, FieldUnit eUnit);

// fill BoxInfo attribute
SW_DLLPUBLIC void PrepareBoxInfo(SfxItemSet& rSet, const SwWrtShell& rSh);

// Modes for attribute conversion
#define CONV_ATTR_STD    1  // Standard character dialog
#define CONV_ATTR_ENV    2  // Character dialog opened from envelope dialog

/**
 * Convert character specific attributes to general ones used by tab pages.
 *
 * @param[in,out]   rSet    the set in which character attributes are stored
 * @param[in]       nMode   specify the dialog which will be called after conversion
**/
SW_DLLPUBLIC void ConvertAttrCharToGen(SfxItemSet& rSet, const sal_uInt8 nMode);

/**
 * Convert general attributes to the corresponding character attributes.
 * This method is used after executed a character dialog.
 *
 * @param[in,out]   rSet    the set in which character attributes are stored
 * @param[in]       rOrigSet    original itemset used as input for the dialog
 * @param[in]       nMode   specify the dialog which was called before
**/
SW_DLLPUBLIC void ConvertAttrGenToChar(SfxItemSet& rSet, const SfxItemSet& rOrigSet, const sal_uInt8 nMode);

// SfxItemSets <-> PageDesc
void ItemSetToPageDesc( const SfxItemSet& rSet, SwPageDesc& rPageDesc );
void PageDescToItemSet( const SwPageDesc& rPageDesc, SfxItemSet& rSet);

// fill tabs with default tabs
SW_DLLPUBLIC void   MakeDefTabs(SwTwips nDefDist, SvxTabStopItem& rTabs);

// erase DefaultTabs from TabStopArray
//void  EraseDefTabs(SvxTabStopItem& rTabs);

// determine space between 1st and 2nd element
SW_DLLPUBLIC SwTwips GetTabDist(const SvxTabStopItem& rTabs);

// determine whether a Sfx-PageDesc combination exists in the set
// and set this in the set and delete the transport items
// (PageBreak & PageModel) from the set
void SwToSfxPageDescAttr( SfxItemSet& rSet );
void SfxToSwPageDescAttr( const SwWrtShell& rShell, SfxItemSet& rSet );

SW_DLLPUBLIC FieldUnit  GetDfltMetric(bool bWeb);
void        SetDfltMetric(FieldUnit eMetric, bool bWeb);

SW_DLLPUBLIC bool HasCharUnit( bool bWeb );
void SetApplyCharUnit(bool bApplyChar, bool bWeb);

// fill ListBox with all char style templates, except the standard ones
SW_DLLPUBLIC void FillCharStyleListBox(ListBox& rToFill, SwDocShell* pDocSh, bool bSorted = false, bool bWithDefault = false);

//inserts a string sorted into a ListBox,
SW_DLLPUBLIC sal_Int32 InsertStringSorted(const OUString& rEntry, ListBox& rToFill, sal_Int32 nOffset);

// Get table width and alignement
SwTwips GetTableWidth( SwFrameFormat* pFormat, SwTabCols& rCols, sal_uInt16 *pPercent,
        SwWrtShell* pSh );

OUString GetAppLangDateTimeString( const DateTime& );

// search for a command string within the menu structure and execute it
// at the dispatcher if there is one, if executed return true
bool ExecuteMenuCommand( PopupMenu& rMenu, SfxViewFrame& rViewFrame, sal_uInt16 nId );

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_UITOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
