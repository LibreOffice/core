/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inputopt.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:30:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include <tools/debug.hxx>

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
using namespace rtl;
using namespace com::sun::star::uno;

//------------------------------------------------------------------

//  Version, ab der das Item kompatibel ist
#define SC_VERSION ((USHORT)351)


//========================================================================
//      ScInputOptions - Eingabe-Optionen
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
    bMoveSelection  = TRUE;
    bEnterEdit      = FALSE;
    bExtendFormat   = FALSE;
    bRangeFinder    = TRUE;
    bExpandRefs     = FALSE;
    bMarkHeader     = TRUE;
    bUseTabCol      = FALSE;
    bTextWysiwyg    = FALSE;
    bReplCellsWarn  = TRUE;
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

    return *this;
}

//------------------------------------------------------------------------

SvStream& operator>>( SvStream& rStream, ScInputOptions& rOpt )
{
    rOpt.SetDefaults();

    ScReadHeader aHdr( rStream );

    rStream >> rOpt.nMoveDir;
    rStream >> rOpt.bMoveSelection;
    rStream >> rOpt.bEnterEdit;
    rStream >> rOpt.bExtendFormat;

    if (aHdr.BytesLeft())
        rStream >> rOpt.bRangeFinder;           // ab 363

    if (aHdr.BytesLeft())
        rStream >> rOpt.bExpandRefs;            // ab 364v

    if (aHdr.BytesLeft())
        rStream >> rOpt.bMarkHeader;            // ab 364irgendwas

    if (aHdr.BytesLeft())
        rStream >> rOpt.bUseTabCol;             // ab 373d

    // newer additions are not in old file format

    return rStream;
}

//------------------------------------------------------------------------

SvStream& operator<<( SvStream& rStream, const ScInputOptions& rOpt )
{
    ScWriteHeader aHdr( rStream, 6 );

    rStream << rOpt.nMoveDir;
    rStream << rOpt.bMoveSelection;
    rStream << rOpt.bEnterEdit;
    rStream << rOpt.bExtendFormat;
    rStream << rOpt.bRangeFinder;
    rStream << rOpt.bExpandRefs;
    rStream << rOpt.bMarkHeader;
    rStream << rOpt.bUseTabCol;

    // newer additions are not in old file format

    return rStream;
}

//==================================================================
//  Config Item containing input options
//==================================================================

#define CFGPATH_INPUT           "Office.Calc/Input"

#define SCINPUTOPT_MOVEDIR          0
#define SCINPUTOPT_MOVESEL          1
#define SCINPUTOPT_EDTEREDIT        2
#define SCINPUTOPT_EXTENDFMT        3
#define SCINPUTOPT_RANGEFIND        4
#define SCINPUTOPT_EXPANDREFS       5
#define SCINPUTOPT_MARKHEADER       6
#define SCINPUTOPT_USETABCOL        7
#define SCINPUTOPT_TEXTWYSIWYG      8
#define SCINPUTOPT_REPLCELLSWARN    9
#define SCINPUTOPT_COUNT            10

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
        "ReplaceCellsWarning"       // SCINPUTOPT_REPLCELLSWARN
    };
    Sequence<OUString> aNames(SCINPUTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCINPUTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

ScInputCfg::ScInputCfg() :
    ConfigItem( OUString::createFromAscii( CFGPATH_INPUT ) )
{
    sal_Int32 nIntVal;

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCINPUTOPT_MOVEDIR:
                        if ( pValues[nProp] >>= nIntVal )
                            SetMoveDir( (USHORT)nIntVal );
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
                }
            }
        }
    }
}


void ScInputCfg::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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
        }
    }
    PutProperties(aNames, aValues);
}

void ScInputCfg::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERROR("properties have been changed")
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


