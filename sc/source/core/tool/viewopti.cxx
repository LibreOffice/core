/*************************************************************************
 *
 *  $RCSfile: viewopti.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <vcl/svapp.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "global.hxx"
#include "globstr.hrc"
#include "cfgids.hxx"
#include "viewopti.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "miscuno.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

//------------------------------------------------------------------

TYPEINIT1(ScTpViewItem, SfxPoolItem);

#define SC_VERSION ((USHORT)302)


//========================================================================
// class ScGridOptions
//========================================================================


void ScGridOptions::SetDefaults()
{
    *this = ScGridOptions();

    //  Raster-Defaults sind jetzt zwischen den Apps unterschiedlich
    //  darum hier selber eintragen (alles in 1/100mm)

    MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
    if ( eSys == MEASURE_METRIC )
    {
        nFldDrawX = 1000;   // 1cm
        nFldDrawY = 1000;
        nFldSnapX = 1000;
        nFldSnapY = 1000;
    }
    else
    {
        nFldDrawX = 1270;   // 0,5"
        nFldDrawY = 1270;
        nFldSnapX = 1270;
        nFldSnapY = 1270;
    }
    nFldDivisionX = 1;
    nFldDivisionY = 1;
}

//------------------------------------------------------------------------

const ScGridOptions& ScGridOptions::operator=( const ScGridOptions& rCpy )
{
    nFldDrawX       = rCpy.nFldDrawX;       // UINT32
    nFldDrawX       = rCpy.nFldDrawX;
    nFldDivisionX   = rCpy.nFldDivisionX;
    nFldDrawY       = rCpy.nFldDrawY;
    nFldDivisionY   = rCpy.nFldDivisionY;
    nFldSnapX       = rCpy.nFldSnapX;
    nFldSnapY       = rCpy.nFldSnapY;
    bUseGridsnap    = rCpy.bUseGridsnap;    // BitBool
    bSynchronize    = rCpy.bSynchronize;
    bGridVisible    = rCpy.bGridVisible;
    bEqualGrid      = rCpy.bEqualGrid;

    return *this;
}

//------------------------------------------------------------------------

int ScGridOptions::operator==( const ScGridOptions& rCpy ) const
{
    return (   nFldDrawX        == rCpy.nFldDrawX
            && nFldDrawX        == rCpy.nFldDrawX
            && nFldDivisionX    == rCpy.nFldDivisionX
            && nFldDrawY        == rCpy.nFldDrawY
            && nFldDivisionY    == rCpy.nFldDivisionY
            && nFldSnapX        == rCpy.nFldSnapX
            && nFldSnapY        == rCpy.nFldSnapY
            && bUseGridsnap     == rCpy.bUseGridsnap
            && bSynchronize     == rCpy.bSynchronize
            && bGridVisible     == rCpy.bGridVisible
            && bEqualGrid       == rCpy.bEqualGrid );
}


//------------------------------------------------------------------------

SvStream& operator>>( SvStream& rStream, ScGridOptions& rOpt )
{
    BYTE nDummy;
    rStream >> rOpt.nFldDrawX;
    rStream >> rOpt.nFldDrawY;
    rStream >> rOpt.nFldDivisionX;
    rStream >> rOpt.nFldDivisionY;
    rStream >> rOpt.nFldSnapX;
    rStream >> rOpt.nFldSnapY;
    rStream >> nDummy; rOpt.bUseGridsnap = (BOOL)nDummy;
    rStream >> nDummy; rOpt.bSynchronize = (BOOL)nDummy;
    rStream >> nDummy; rOpt.bGridVisible = (BOOL)nDummy;
    rStream >> nDummy; rOpt.bEqualGrid   = (BOOL)nDummy;

    return rStream;
}

//------------------------------------------------------------------------

SvStream& operator<<( SvStream& rStream, const ScGridOptions& rOpt )
{
    rStream << rOpt.nFldDrawX;
    rStream << rOpt.nFldDrawY;
    rStream << rOpt.nFldDivisionX;
    rStream << rOpt.nFldDivisionY;
    rStream << rOpt.nFldSnapX;
    rStream << rOpt.nFldSnapY;
    rStream << (BOOL)rOpt.bUseGridsnap;
    rStream << (BOOL)rOpt.bSynchronize;
    rStream << (BOOL)rOpt.bGridVisible;
    rStream << (BOOL)rOpt.bEqualGrid;

    return rStream;
}

//========================================================================
// class ScViewOptions
//========================================================================

ScViewOptions::ScViewOptions()
{
    SetDefaults();
}

//------------------------------------------------------------------------

ScViewOptions::ScViewOptions( const ScViewOptions& rCpy )
{
    *this = rCpy;
}

//------------------------------------------------------------------------

__EXPORT ScViewOptions::~ScViewOptions()
{
}

//------------------------------------------------------------------------

void ScViewOptions::SetDefaults()
{
    aOptArr[ VOPT_FORMULAS    ] =
    aOptArr[ VOPT_SYNTAX      ] =
    aOptArr[ VOPT_HELPLINES   ] =
    aOptArr[ VOPT_BIGHANDLES  ] = FALSE;
    aOptArr[ VOPT_NOTES       ] =
    aOptArr[ VOPT_NULLVALS    ] =
    aOptArr[ VOPT_VSCROLL     ] =
    aOptArr[ VOPT_HSCROLL     ] =
    aOptArr[ VOPT_TABCONTROLS ] =
    aOptArr[ VOPT_OUTLINER    ] =
    aOptArr[ VOPT_HEADER      ] =
    aOptArr[ VOPT_GRID        ] =
    aOptArr[ VOPT_ANCHOR      ] =
    aOptArr[ VOPT_PAGEBREAKS  ] =
    aOptArr[ VOPT_SOLIDHANDLES] =
    aOptArr[ VOPT_CLIPMARKS   ] = TRUE;

    aModeArr[VOBJ_TYPE_OLE ]  =
    aModeArr[VOBJ_TYPE_CHART] =
    aModeArr[VOBJ_TYPE_DRAW ] = VOBJ_MODE_SHOW;

    aGridCol     = Color( COL_LIGHTGRAY );
    aGridColName = ScGlobal::GetRscString( STR_GRIDCOLOR );

    aGridOpt.SetDefaults();

    bHideAutoSpell = FALSE;
}

//------------------------------------------------------------------------

Color ScViewOptions::GetGridColor( String* pStrName ) const
{
    if ( pStrName )
        *pStrName = aGridColName;

    return aGridCol;
}

//------------------------------------------------------------------------

const ScViewOptions& ScViewOptions::operator=( const ScViewOptions& rCpy )
{
    USHORT i;

    for ( i=0; i<MAX_OPT; i++ )  aOptArr [i] = rCpy.aOptArr[i];
    for ( i=0; i<MAX_TYPE; i++ ) aModeArr[i] = rCpy.aModeArr[i];

    aGridCol        = rCpy.aGridCol;
    aGridColName    = rCpy.aGridColName;
    aGridOpt        = rCpy.aGridOpt;
    bHideAutoSpell  = rCpy.bHideAutoSpell;

    return *this;
}

//------------------------------------------------------------------------

int ScViewOptions::operator==( const ScViewOptions& rOpt ) const
{
    BOOL    bEqual = TRUE;
    USHORT  i;

    for ( i=0; i<MAX_OPT && bEqual; i++ )  bEqual = (aOptArr [i] == rOpt.aOptArr[i]);
    for ( i=0; i<MAX_TYPE && bEqual; i++ ) bEqual = (aModeArr[i] == rOpt.aModeArr[i]);

    bEqual = bEqual && (aGridCol       == rOpt.aGridCol);
    bEqual = bEqual && (aGridColName   == rOpt.aGridColName);
    bEqual = bEqual && (aGridOpt       == rOpt.aGridOpt);
    bEqual = bEqual && (bHideAutoSpell == rOpt.bHideAutoSpell);

    return bEqual;
}

//------------------------------------------------------------------------

SvStream& operator>>( SvStream& rStream, ScViewOptions& rOpt )
{
    USHORT  i;
    BYTE    n;

    ScReadHeader aHdr( rStream );

    for ( i=0; i<=VOPT_GRID; i++ )          // kompatibel bleiben -> nur bis VOPT_GRID
        rStream >> rOpt.aOptArr[i];

    for ( i=0; i<MAX_TYPE; i++ )
        rStream >> n, rOpt.aModeArr[i] = (ScVObjMode)n;

    rStream >> rOpt.aGridCol;
    rStream.ReadByteString( rOpt.aGridColName, rStream.GetStreamCharSet() );

    if( aHdr.BytesLeft() )
        rStream >> rOpt.aOptArr[VOPT_HELPLINES];

    if( aHdr.BytesLeft() )
        rStream >> rOpt.aGridOpt;

    if( aHdr.BytesLeft() )
        rStream >> rOpt.bHideAutoSpell;

    if( aHdr.BytesLeft() )
        rStream >> rOpt.aOptArr[VOPT_ANCHOR];

    if( aHdr.BytesLeft() )
        rStream >> rOpt.aOptArr[VOPT_PAGEBREAKS];

    if( aHdr.BytesLeft() )
        rStream >> rOpt.aOptArr[VOPT_SOLIDHANDLES];

    if( aHdr.BytesLeft() )
        rStream >> rOpt.aOptArr[VOPT_CLIPMARKS];

    if( aHdr.BytesLeft() )
        rStream >> rOpt.aOptArr[VOPT_BIGHANDLES];

    return rStream;
}

//------------------------------------------------------------------------

void ScViewOptions::Save(SvStream& rStream, BOOL bConfig) const
{
    USHORT i;

    ScWriteHeader aHdr( rStream, 68 );

    for ( i=0; i<=VOPT_GRID; i++ )          // kompatibel bleiben -> nur bis VOPT_GRID
        rStream << aOptArr[i];

    for ( i=0; i<MAX_TYPE; i++ )
        rStream << (BYTE)aModeArr[i];

    rStream << aGridCol;
    rStream.WriteByteString( aGridColName, rStream.GetStreamCharSet() );
    rStream << aOptArr[VOPT_HELPLINES];
    rStream << aGridOpt;
    rStream << bHideAutoSpell;
    rStream << aOptArr[VOPT_ANCHOR];
    rStream << aOptArr[VOPT_PAGEBREAKS];
    rStream << aOptArr[VOPT_SOLIDHANDLES];

    if ( bConfig || rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )      // nicht bei 4.0 Export
    {
        rStream << aOptArr[VOPT_CLIPMARKS];

        //  big handles are not saved in 5.0-documents to avoid warning messages
        //! save to files after 5.0 !!!

        if ( bConfig )
            rStream << aOptArr[VOPT_BIGHANDLES];
    }
}

//------------------------------------------------------------------------

SvxGridItem* ScViewOptions::CreateGridItem( USHORT nId /* = SID_ATTR_GRID_OPTIONS */ ) const
{
    SvxGridItem* pItem = new SvxGridItem( nId );

    pItem->SetFldDrawX      ( aGridOpt.GetFldDrawX() );
    pItem->SetFldDivisionX  ( aGridOpt.GetFldDivisionX() );
    pItem->SetFldDrawY      ( aGridOpt.GetFldDrawY() );
    pItem->SetFldDivisionY  ( aGridOpt.GetFldDivisionY() );
    pItem->SetFldSnapX      ( aGridOpt.GetFldSnapX() );
    pItem->SetFldSnapY      ( aGridOpt.GetFldSnapY() );
    pItem->SetUseGridSnap   ( aGridOpt.GetUseGridSnap() );
    pItem->SetSynchronize   ( aGridOpt.GetSynchronize() );
    pItem->SetGridVisible   ( aGridOpt.GetGridVisible() );
    pItem->SetEqualGrid     ( aGridOpt.GetEqualGrid() );

    return pItem;
}

//========================================================================
//      ScTpViewItem - Daten fuer die ViewOptions-TabPage
//========================================================================

ScTpViewItem::ScTpViewItem( USHORT nWhich ) : SfxPoolItem( nWhich )
{
}

//------------------------------------------------------------------------

ScTpViewItem::ScTpViewItem( USHORT nWhich, const ScViewOptions& rOpt )
    :   SfxPoolItem ( nWhich ),
        theOptions  ( rOpt )
{
}

//------------------------------------------------------------------------

ScTpViewItem::ScTpViewItem( const ScTpViewItem& rItem )
    :   SfxPoolItem ( rItem ),
        theOptions  ( rItem.theOptions )
{
}

//------------------------------------------------------------------------

__EXPORT ScTpViewItem::~ScTpViewItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScTpViewItem::GetValueText() const
{
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("ScTpViewItem") );
}

//------------------------------------------------------------------------

int __EXPORT ScTpViewItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTpViewItem& rPItem = (const ScTpViewItem&)rItem;

    return ( theOptions == rPItem.theOptions );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScTpViewItem::Clone( SfxItemPool * ) const
{
    return new ScTpViewItem( *this );
}

//------------------------------------------------------------------------

ScLinkConfigItem::ScLinkConfigItem( const rtl::OUString rSubTree ) :
    ConfigItem( rSubTree )
{
}

void ScLinkConfigItem::SetCommitLink( const Link& rLink )
{
    aCommitLink = rLink;
}

void ScLinkConfigItem::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames )
{
    //! not implemented yet...
}

void ScLinkConfigItem::Commit()
{
    aCommitLink.Call( this );
}

//==================================================================
//  Config Item containing view options
//==================================================================

#define CFGPATH_LAYOUT      "Office.Calc/Layout"

#define SCLAYOUTOPT_GRIDLINES       0
#define SCLAYOUTOPT_GRIDCOLOR       1
#define SCLAYOUTOPT_PAGEBREAK       2
#define SCLAYOUTOPT_GUIDE           3
#define SCLAYOUTOPT_SIMPLECONT      4
#define SCLAYOUTOPT_LARGECONT       5
#define SCLAYOUTOPT_COLROWHDR       6
#define SCLAYOUTOPT_HORISCROLL      7
#define SCLAYOUTOPT_VERTSCROLL      8
#define SCLAYOUTOPT_SHEETTAB        9
#define SCLAYOUTOPT_OUTLINE         10
#define SCLAYOUTOPT_COUNT           11

#define CFGPATH_DISPLAY     "Office.Calc/Content/Display"

#define SCDISPLAYOPT_FORMULA        0
#define SCDISPLAYOPT_ZEROVALUE      1
#define SCDISPLAYOPT_NOTETAG        2
#define SCDISPLAYOPT_VALUEHI        3
#define SCDISPLAYOPT_ANCHOR         4
#define SCDISPLAYOPT_TEXTOVER       5
#define SCDISPLAYOPT_OBJECTGRA      6
#define SCDISPLAYOPT_CHART          7
#define SCDISPLAYOPT_DRAWING        8
#define SCDISPLAYOPT_COUNT          9

#define CFGPATH_GRID        "Office.Calc/Grid"

#define SCGRIDOPT_RESOLU_X          0
#define SCGRIDOPT_RESOLU_Y          1
#define SCGRIDOPT_SUBDIV_X          2
#define SCGRIDOPT_SUBDIV_Y          3
#define SCGRIDOPT_OPTION_X          4
#define SCGRIDOPT_OPTION_Y          5
#define SCGRIDOPT_SNAPTOGRID        6
#define SCGRIDOPT_SYNCHRON          7
#define SCGRIDOPT_VISIBLE           8
#define SCGRIDOPT_SIZETOGRID        9
#define SCGRIDOPT_COUNT             10


Sequence<OUString> ScViewCfg::GetLayoutPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Line/GridLine",            // SCLAYOUTOPT_GRIDLINES
        "Line/GridLineColor",       // SCLAYOUTOPT_GRIDCOLOR
        "Line/PageBreak",           // SCLAYOUTOPT_PAGEBREAK
        "Line/Guide",               // SCLAYOUTOPT_GUIDE
        "Line/SimpleControlPoint",  // SCLAYOUTOPT_SIMPLECONT
        "Line/LargeControlPoint",   // SCLAYOUTOPT_LARGECONT
        "Window/ColumnRowHeader",   // SCLAYOUTOPT_COLROWHDR
        "Window/HorizontalScroll",  // SCLAYOUTOPT_HORISCROLL
        "Window/VerticalScroll",    // SCLAYOUTOPT_VERTSCROLL
        "Window/SheetTab",          // SCLAYOUTOPT_SHEETTAB
        "Window/OutlineSymbol"      // SCLAYOUTOPT_OUTLINE
    };
    Sequence<OUString> aNames(SCLAYOUTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCLAYOUTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScViewCfg::GetDisplayPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Formula",                  // SCDISPLAYOPT_FORMULA
        "ZeroValue",                // SCDISPLAYOPT_ZEROVALUE
        "NoteTag",                  // SCDISPLAYOPT_NOTETAG
        "ValueHighlighting",        // SCDISPLAYOPT_VALUEHI
        "Anchor",                   // SCDISPLAYOPT_ANCHOR
        "TextOverflow",             // SCDISPLAYOPT_TEXTOVER
        "ObjectGraphic",            // SCDISPLAYOPT_OBJECTGRA
        "Chart",                    // SCDISPLAYOPT_CHART
        "DrawingObject"             // SCDISPLAYOPT_DRAWING
    };
    Sequence<OUString> aNames(SCDISPLAYOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCDISPLAYOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScViewCfg::GetGridPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Resolution/XAxis",             // SCGRIDOPT_RESOLU_X
        "Resolution/YAxis",             // SCGRIDOPT_RESOLU_Y
        "Subdivision/XAxis",            // SCGRIDOPT_SUBDIV_X
        "Subdivision/YAxis",            // SCGRIDOPT_SUBDIV_Y
        "Option/XAxis",                 // SCGRIDOPT_OPTION_X
        "Option/YAxis",                 // SCGRIDOPT_OPTION_Y
        "Option/SnapToGrid",            // SCGRIDOPT_SNAPTOGRID
        "Option/Synchronize",           // SCGRIDOPT_SYNCHRON
        "Option/VisibleGrid",           // SCGRIDOPT_VISIBLE
        "Option/SizeToGrid"             // SCGRIDOPT_SIZETOGRID
    };
    Sequence<OUString> aNames(SCGRIDOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCGRIDOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}


ScViewCfg::ScViewCfg() :
    aLayoutItem( OUString::createFromAscii( CFGPATH_LAYOUT ) ),
    aDisplayItem( OUString::createFromAscii( CFGPATH_DISPLAY ) ),
    aGridItem( OUString::createFromAscii( CFGPATH_GRID ) )
{
    sal_Int32 nIntVal;

    Sequence<OUString> aNames = GetLayoutPropertyNames();
    Sequence<Any> aValues = aLayoutItem.GetProperties(aNames);
    aLayoutItem.EnableNotification(aNames);
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
                    case SCLAYOUTOPT_GRIDCOLOR:
                        if ( pValues[nProp] >>= nIntVal )
                            SetGridColor( Color(nIntVal), EMPTY_STRING );
                        break;
                    case SCLAYOUTOPT_GRIDLINES:
                        SetOption( VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_PAGEBREAK:
                        SetOption( VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_GUIDE:
                        SetOption( VOPT_HELPLINES, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_SIMPLECONT:
                        // content is reversed
                        SetOption( VOPT_SOLIDHANDLES, !ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_LARGECONT:
                        SetOption( VOPT_BIGHANDLES, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_COLROWHDR:
                        SetOption( VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_HORISCROLL:
                        SetOption( VOPT_HSCROLL, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_VERTSCROLL:
                        SetOption( VOPT_VSCROLL, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_SHEETTAB:
                        SetOption( VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCLAYOUTOPT_OUTLINE:
                        SetOption( VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
    }
    aLayoutItem.SetCommitLink( LINK( this, ScViewCfg, LayoutCommitHdl ) );

    aNames = GetDisplayPropertyNames();
    aValues = aDisplayItem.GetProperties(aNames);
    aDisplayItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
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
                    case SCDISPLAYOPT_FORMULA:
                        SetOption( VOPT_FORMULAS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCDISPLAYOPT_ZEROVALUE:
                        SetOption( VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCDISPLAYOPT_NOTETAG:
                        SetOption( VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCDISPLAYOPT_VALUEHI:
                        SetOption( VOPT_SYNTAX, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCDISPLAYOPT_ANCHOR:
                        SetOption( VOPT_ANCHOR, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCDISPLAYOPT_TEXTOVER:
                        SetOption( VOPT_CLIPMARKS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCDISPLAYOPT_OBJECTGRA:
                        if ( pValues[nProp] >>= nIntVal )
                            SetObjMode( VOBJ_TYPE_OLE, (ScVObjMode) nIntVal );
                        break;
                    case SCDISPLAYOPT_CHART:
                        if ( pValues[nProp] >>= nIntVal )
                            SetObjMode( VOBJ_TYPE_CHART, (ScVObjMode) nIntVal );
                        break;
                    case SCDISPLAYOPT_DRAWING:
                        if ( pValues[nProp] >>= nIntVal )
                            SetObjMode( VOBJ_TYPE_DRAW, (ScVObjMode) nIntVal );
                        break;
                }
            }
        }
    }
    aDisplayItem.SetCommitLink( LINK( this, ScViewCfg, DisplayCommitHdl ) );

    ScGridOptions aGrid = GetGridOptions();     //! initialization necessary?
    aNames = GetGridPropertyNames();
    aValues = aGridItem.GetProperties(aNames);
    aGridItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
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
                    case SCGRIDOPT_RESOLU_X:
                        if (pValues[nProp] >>= nIntVal) aGrid.SetFldDrawX( nIntVal );
                        break;
                    case SCGRIDOPT_RESOLU_Y:
                        if (pValues[nProp] >>= nIntVal) aGrid.SetFldDrawY( nIntVal );
                        break;
                    case SCGRIDOPT_SUBDIV_X:
                        if (pValues[nProp] >>= nIntVal) aGrid.SetFldDivisionX( nIntVal );
                        break;
                    case SCGRIDOPT_SUBDIV_Y:
                        if (pValues[nProp] >>= nIntVal) aGrid.SetFldDivisionY( nIntVal );
                        break;
                    case SCGRIDOPT_OPTION_X:
                        if (pValues[nProp] >>= nIntVal) aGrid.SetFldSnapX( nIntVal );
                        break;
                    case SCGRIDOPT_OPTION_Y:
                        if (pValues[nProp] >>= nIntVal) aGrid.SetFldSnapY( nIntVal );
                        break;
                    case SCGRIDOPT_SNAPTOGRID:
                        aGrid.SetUseGridSnap( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCGRIDOPT_SYNCHRON:
                        aGrid.SetSynchronize( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCGRIDOPT_VISIBLE:
                        aGrid.SetGridVisible( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCGRIDOPT_SIZETOGRID:
                        aGrid.SetEqualGrid( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
    }
    SetGridOptions( aGrid );
    aGridItem.SetCommitLink( LINK( this, ScViewCfg, GridCommitHdl ) );
}

IMPL_LINK( ScViewCfg, LayoutCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetLayoutPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCLAYOUTOPT_GRIDCOLOR:
                pValues[nProp] <<= (sal_Int32) GetGridColor().GetColor();
                break;
            case SCLAYOUTOPT_GRIDLINES:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_GRID ) );
                break;
            case SCLAYOUTOPT_PAGEBREAK:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_PAGEBREAKS ) );
                break;
            case SCLAYOUTOPT_GUIDE:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_HELPLINES ) );
                break;
            case SCLAYOUTOPT_SIMPLECONT:
                // content is reversed
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], !GetOption( VOPT_SOLIDHANDLES ) );
                break;
            case SCLAYOUTOPT_LARGECONT:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_BIGHANDLES ) );
                break;
            case SCLAYOUTOPT_COLROWHDR:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_HEADER ) );
                break;
            case SCLAYOUTOPT_HORISCROLL:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_HSCROLL ) );
                break;
            case SCLAYOUTOPT_VERTSCROLL:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_VSCROLL ) );
                break;
            case SCLAYOUTOPT_SHEETTAB:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_TABCONTROLS ) );
                break;
            case SCLAYOUTOPT_OUTLINE:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_OUTLINER ) );
                break;
        }
    }
    aLayoutItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK( ScViewCfg, DisplayCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetDisplayPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCDISPLAYOPT_FORMULA:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_FORMULAS ) );
                break;
            case SCDISPLAYOPT_ZEROVALUE:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_NULLVALS ) );
                break;
            case SCDISPLAYOPT_NOTETAG:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_NOTES ) );
                break;
            case SCDISPLAYOPT_VALUEHI:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_SYNTAX ) );
                break;
            case SCDISPLAYOPT_ANCHOR:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_ANCHOR ) );
                break;
            case SCDISPLAYOPT_TEXTOVER:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetOption( VOPT_CLIPMARKS ) );
                break;
            case SCDISPLAYOPT_OBJECTGRA:
                pValues[nProp] <<= (sal_Int32) GetObjMode( VOBJ_TYPE_OLE );
                break;
            case SCDISPLAYOPT_CHART:
                pValues[nProp] <<= (sal_Int32) GetObjMode( VOBJ_TYPE_CHART );
                break;
            case SCDISPLAYOPT_DRAWING:
                pValues[nProp] <<= (sal_Int32) GetObjMode( VOBJ_TYPE_DRAW );
                break;
        }
    }
    aDisplayItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK( ScViewCfg, GridCommitHdl, void *, EMPTYARG )
{
    const ScGridOptions& rGrid = GetGridOptions();

    Sequence<OUString> aNames = GetGridPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCGRIDOPT_RESOLU_X:
                pValues[nProp] <<= (sal_Int32) rGrid.GetFldDrawX();
                break;
            case SCGRIDOPT_RESOLU_Y:
                pValues[nProp] <<= (sal_Int32) rGrid.GetFldDrawY();
                break;
            case SCGRIDOPT_SUBDIV_X:
                pValues[nProp] <<= (sal_Int32) rGrid.GetFldDivisionX();
                break;
            case SCGRIDOPT_SUBDIV_Y:
                pValues[nProp] <<= (sal_Int32) rGrid.GetFldDivisionY();
                break;
            case SCGRIDOPT_OPTION_X:
                pValues[nProp] <<= (sal_Int32) rGrid.GetFldSnapX();
                break;
            case SCGRIDOPT_OPTION_Y:
                pValues[nProp] <<= (sal_Int32) rGrid.GetFldSnapY();
                break;
            case SCGRIDOPT_SNAPTOGRID:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], rGrid.GetUseGridSnap() );
                break;
            case SCGRIDOPT_SYNCHRON:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], rGrid.GetSynchronize() );
                break;
            case SCGRIDOPT_VISIBLE:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], rGrid.GetGridVisible() );
                break;
            case SCGRIDOPT_SIZETOGRID:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], rGrid.GetEqualGrid() );
                break;
        }
    }
    aGridItem.PutProperties(aNames, aValues);

    return 0;
}

void ScViewCfg::SetOptions( const ScViewOptions& rNew )
{
    *(ScViewOptions*)this = rNew;
    aLayoutItem.SetModified();
    aDisplayItem.SetModified();
    aGridItem.SetModified();
}


