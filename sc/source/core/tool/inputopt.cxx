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

#include "cfgids.hxx"
#include "inputopt.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "global.hxx"
#include "sc.hrc"
#include "miscuno.hxx"

using namespace utl;
using namespace com::sun::star::uno;


//========================================================================
// ScInputOptions - input options
//========================================================================

ScInputOptions::ScInputOptions()
{
    SetDefaults();
}

//------------------------------------------------------------------------

ScInputOptions::ScInputOptions( const ScInputOptions& rCpy )
{
    *this = rCpy;
}

//------------------------------------------------------------------------

ScInputOptions::~ScInputOptions()
{
}

//------------------------------------------------------------------------

void ScInputOptions::SetDefaults()
{
    nMoveDir        = DIR_BOTTOM;
    bMoveSelection  = sal_True;
    bEnterEdit      = false;
    bExtendFormat   = false;
    bRangeFinder    = sal_True;
    bExpandRefs     = false;
    bMarkHeader     = sal_True;
    bUseTabCol      = false;
    bTextWysiwyg    = false;
    bReplCellsWarn  = sal_True;
    bLegacyCellSelection = false;
}

//------------------------------------------------------------------------

const ScInputOptions& ScInputOptions::operator=( const ScInputOptions& rCpy )
{
    nMoveDir        = rCpy.nMoveDir;
    bMoveSelection  = rCpy.bMoveSelection;
    bEnterEdit      = rCpy.bEnterEdit;
    bExtendFormat   = rCpy.bExtendFormat;
    bRangeFinder    = rCpy.bRangeFinder;
    bExpandRefs     = rCpy.bExpandRefs;
    bMarkHeader     = rCpy.bMarkHeader;
    bUseTabCol      = rCpy.bUseTabCol;
    bTextWysiwyg    = rCpy.bTextWysiwyg;
    bReplCellsWarn  = rCpy.bReplCellsWarn;
    bLegacyCellSelection = rCpy.bLegacyCellSelection;

    return *this;
}


//==================================================================
//  Config Item containing input options
//==================================================================

#define CFGPATH_INPUT           "Office.Calc/Input"

#define SCINPUTOPT_MOVEDIR                0
#define SCINPUTOPT_MOVESEL                1
#define SCINPUTOPT_EDTEREDIT              2
#define SCINPUTOPT_EXTENDFMT              3
#define SCINPUTOPT_RANGEFIND              4
#define SCINPUTOPT_EXPANDREFS             5
#define SCINPUTOPT_MARKHEADER             6
#define SCINPUTOPT_USETABCOL              7
#define SCINPUTOPT_TEXTWYSIWYG            8
#define SCINPUTOPT_REPLCELLSWARN          9
#define SCINPUTOPT_LEGACY_CELL_SELECTION 10
#define SCINPUTOPT_COUNT                 11

Sequence<OUString> ScInputCfg::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "MoveSelectionDirection",   // SCINPUTOPT_MOVEDIR
        "MoveSelection",            // SCINPUTOPT_MOVESEL
        "SwitchToEditMode",         // SCINPUTOPT_EDTEREDIT
        "ExpandFormatting",         // SCINPUTOPT_EXTENDFMT
        "ShowReference",            // SCINPUTOPT_RANGEFIND
        "ExpandReference",          // SCINPUTOPT_EXPANDREFS
        "HighlightSelection",       // SCINPUTOPT_MARKHEADER
        "UseTabCol",                // SCINPUTOPT_USETABCOL
        "UsePrinterMetrics",        // SCINPUTOPT_TEXTWYSIWYG
        "ReplaceCellsWarning",      // SCINPUTOPT_REPLCELLSWARN
        "LegacyCellSelection"       // SCINPUTOPT_LEGACY_CELL_SELECTION
    };
    Sequence<OUString> aNames(SCINPUTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCINPUTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

ScInputCfg::ScInputCfg() :
    ConfigItem( OUString( CFGPATH_INPUT ) )
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                sal_Int32 nIntVal = 0;
                switch(nProp)
                {
                    case SCINPUTOPT_MOVEDIR:
                        if ( pValues[nProp] >>= nIntVal )
                            SetMoveDir( (sal_uInt16)nIntVal );
                        break;
                    case SCINPUTOPT_MOVESEL:
                        SetMoveSelection( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_EDTEREDIT:
                        SetEnterEdit( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_EXTENDFMT:
                        SetExtendFormat( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_RANGEFIND:
                        SetRangeFinder( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_EXPANDREFS:
                        SetExpandRefs( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_MARKHEADER:
                        SetMarkHeader( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_USETABCOL:
                        SetUseTabCol( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_TEXTWYSIWYG:
                        SetTextWysiwyg( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_REPLCELLSWARN:
                        SetReplaceCellsWarn( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_LEGACY_CELL_SELECTION:
                        SetLegacyCellSelection( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
    }
}


void ScInputCfg::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCINPUTOPT_MOVEDIR:
                pValues[nProp] <<= (sal_Int32) GetMoveDir();
                break;
            case SCINPUTOPT_MOVESEL:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetMoveSelection() );
                break;
            case SCINPUTOPT_EDTEREDIT:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetEnterEdit() );
                break;
            case SCINPUTOPT_EXTENDFMT:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetExtendFormat() );
                break;
            case SCINPUTOPT_RANGEFIND:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetRangeFinder() );
                break;
            case SCINPUTOPT_EXPANDREFS:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetExpandRefs() );
                break;
            case SCINPUTOPT_MARKHEADER:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetMarkHeader() );
                break;
            case SCINPUTOPT_USETABCOL:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetUseTabCol() );
                break;
            case SCINPUTOPT_TEXTWYSIWYG:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetTextWysiwyg() );
                break;
            case SCINPUTOPT_REPLCELLSWARN:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetReplaceCellsWarn() );
                break;
            case SCINPUTOPT_LEGACY_CELL_SELECTION:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetLegacyCellSelection() );
                break;
        }
    }
    PutProperties(aNames, aValues);
}

void ScInputCfg::Notify( const Sequence<OUString>& /* aPropertyNames */ )
{
    OSL_FAIL("properties have been changed");
}

void ScInputCfg::SetOptions( const ScInputOptions& rNew )
{
    *(ScInputOptions*)this = rNew;
    SetModified();
}

void ScInputCfg::OptionsChanged()
{
    SetModified();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
