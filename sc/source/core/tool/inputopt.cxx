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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>

#include <inputopt.hxx>
#include <global.hxx>

using namespace utl;
using namespace com::sun::star::uno;

// ScInputOptions - input options

ScInputOptions::ScInputOptions()
    : nMoveDir(DIR_BOTTOM)
    , bMoveSelection(true)
    , bEnterEdit(false)
    , bExtendFormat(false)
    , bRangeFinder(true)
    , bExpandRefs(false)
    , mbSortRefUpdate(true)
    , bMarkHeader(true)
    , bUseTabCol(false)
    , bTextWysiwyg(false)
    , bReplCellsWarn(true)
    , bLegacyCellSelection(false)
    , bEnterPasteMode(false)
{
}

//  Config Item containing input options

constexpr OUStringLiteral CFGPATH_INPUT = u"Office.Calc/Input";

#define SCINPUTOPT_MOVEDIR                 0
#define SCINPUTOPT_MOVESEL                 1
#define SCINPUTOPT_EDTEREDIT               2
#define SCINPUTOPT_EXTENDFMT               3
#define SCINPUTOPT_RANGEFIND               4
#define SCINPUTOPT_EXPANDREFS              5
#define SCINPUTOPT_SORT_REF_UPDATE         6
#define SCINPUTOPT_MARKHEADER              7
#define SCINPUTOPT_USETABCOL               8
#define SCINPUTOPT_TEXTWYSIWYG             9
#define SCINPUTOPT_REPLCELLSWARN          10
#define SCINPUTOPT_LEGACY_CELL_SELECTION  11
#define SCINPUTOPT_ENTER_PASTE_MODE       12

Sequence<OUString> ScInputCfg::GetPropertyNames()
{
    return {"MoveSelectionDirection",   // SCINPUTOPT_MOVEDIR
            "MoveSelection",            // SCINPUTOPT_MOVESEL
            "SwitchToEditMode",         // SCINPUTOPT_EDTEREDIT
            "ExpandFormatting",         // SCINPUTOPT_EXTENDFMT
            "ShowReference",            // SCINPUTOPT_RANGEFIND
            "ExpandReference",          // SCINPUTOPT_EXPANDREFS
            "UpdateReferenceOnSort",    // SCINPUTOPT_SORT_REF_UPDATE
            "HighlightSelection",       // SCINPUTOPT_MARKHEADER
            "UseTabCol",                // SCINPUTOPT_USETABCOL
            "UsePrinterMetrics",        // SCINPUTOPT_TEXTWYSIWYG
            "ReplaceCellsWarning",      // SCINPUTOPT_REPLCELLSWARN
            "LegacyCellSelection",      // SCINPUTOPT_LEGACY_CELL_SELECTION
            "EnterPasteMode"};          // SCINPUTOPT_ENTER_PASTE_MODE
}

ScInputCfg::ScInputCfg() :
    ConfigItem( CFGPATH_INPUT )
{
    Sequence<OUString> aNames = GetPropertyNames();
    EnableNotification(aNames);
    ReadCfg();
}

void ScInputCfg::ReadCfg()
{
    const Sequence<OUString> aNames = GetPropertyNames();
    const Sequence<Any> aValues = GetProperties(aNames);
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != aNames.getLength())
        return;

    if (sal_Int32 nVal; aValues[SCINPUTOPT_MOVEDIR] >>= nVal)
        SetMoveDir(static_cast<sal_uInt16>(nVal));
    if (bool bVal; aValues[SCINPUTOPT_MOVESEL] >>= bVal)
        SetMoveSelection(bVal);
    if (bool bVal; aValues[SCINPUTOPT_EDTEREDIT] >>= bVal)
        SetEnterEdit(bVal);
    if (bool bVal; aValues[SCINPUTOPT_EXTENDFMT] >>= bVal)
        SetExtendFormat(bVal);
    if (bool bVal; aValues[SCINPUTOPT_RANGEFIND] >>= bVal)
        SetRangeFinder(bVal);
    if (bool bVal; aValues[SCINPUTOPT_EXPANDREFS] >>= bVal)
        SetExpandRefs(bVal);
    if (bool bVal; aValues[SCINPUTOPT_SORT_REF_UPDATE] >>= bVal)
        SetSortRefUpdate(bVal);
    if (bool bVal; aValues[SCINPUTOPT_MARKHEADER] >>= bVal)
        SetMarkHeader(bVal);
    if (bool bVal; aValues[SCINPUTOPT_USETABCOL] >>= bVal)
        SetUseTabCol(bVal);
    if (bool bVal; aValues[SCINPUTOPT_TEXTWYSIWYG] >>= bVal)
        SetTextWysiwyg(bVal);
    if (bool bVal; aValues[SCINPUTOPT_REPLCELLSWARN] >>= bVal)
        SetReplaceCellsWarn(bVal);
    if (bool bVal; aValues[SCINPUTOPT_LEGACY_CELL_SELECTION] >>= bVal)
        SetLegacyCellSelection(bVal);
    if (bool bVal; aValues[SCINPUTOPT_ENTER_PASTE_MODE] >>= bVal)
        SetEnterPasteMode(bVal);
}

void ScInputCfg::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    pValues[SCINPUTOPT_MOVEDIR] <<= static_cast<sal_Int32>(GetMoveDir());
    pValues[SCINPUTOPT_MOVESEL] <<= GetMoveSelection();
    pValues[SCINPUTOPT_EDTEREDIT] <<= GetEnterEdit();
    pValues[SCINPUTOPT_EXTENDFMT] <<= GetExtendFormat();
    pValues[SCINPUTOPT_RANGEFIND] <<= GetRangeFinder();
    pValues[SCINPUTOPT_EXPANDREFS] <<= GetExpandRefs();
    pValues[SCINPUTOPT_SORT_REF_UPDATE] <<= GetSortRefUpdate();
    pValues[SCINPUTOPT_MARKHEADER] <<= GetMarkHeader();
    pValues[SCINPUTOPT_USETABCOL] <<= GetUseTabCol();
    pValues[SCINPUTOPT_TEXTWYSIWYG] <<= GetTextWysiwyg();
    pValues[SCINPUTOPT_REPLCELLSWARN] <<= GetReplaceCellsWarn();
    pValues[SCINPUTOPT_LEGACY_CELL_SELECTION] <<= GetLegacyCellSelection();
    pValues[SCINPUTOPT_ENTER_PASTE_MODE] <<= GetEnterPasteMode();
    PutProperties(aNames, aValues);
}

void ScInputCfg::Notify( const Sequence<OUString>& /* aPropertyNames */ )
{
    ReadCfg();
}

void ScInputCfg::SetOptions( const ScInputOptions& rNew )
{
    *static_cast<ScInputOptions*>(this) = rNew;
    SetModified();
    Commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
