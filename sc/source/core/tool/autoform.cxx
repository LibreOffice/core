/*************************************************************************
 *
 *  $RCSfile: autoform.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-06 10:34:25 $
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


/*------------------------------------------------------------------------

    $Author: nn $ $Date: 2000-11-06 10:34:25 $ $Revision: 1.3 $
    $Logfile:   T:/sc/source/core/tool/autoform.cxv  $ $Workfile:   autoform.cxx  $
    (c) Copyright 1989 - 1994, Star Division GmbH, Hamburg

------------------------------------------------------------------------*/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define READ_OLDVERS

#include "scitems.hxx"
#include <svx/adjitem.hxx>
#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/cntritem.hxx>
#include <svx/colritem.hxx>
#include <svx/crsditem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/postitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/rotmodit.hxx>
#include <sfx2/app.hxx>
#include <svtools/intitem.hxx>
#include <svtools/pathoptions.hxx>
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/system.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>

#include "autoform.hxx"
#include "zforauto.hxx"
#include "globstr.hrc"

//------------------------------------------------------------------------

sal_Char *linker_dummy = "";

//  Standard-Name ist jetzt STR_STYLENAME_STANDARD (wie Vorlagen)
//static const sal_Char __FAR_DATA cStandardName[] = "Standard";

static const sal_Char __FAR_DATA sAutoTblFmtName[] = "autotbl.fmt";

// bis SO5PF
const USHORT AUTOFORMAT_ID_X        = 9501;
const USHORT AUTOFORMAT_ID_358      = 9601;
const USHORT AUTOFORMAT_DATA_ID_X   = 9502;

// ab SO5
//! in nachfolgenden Versionen muss der Betrag dieser IDs groesser sein
const USHORT AUTOFORMAT_ID_504      = 9801;
const USHORT AUTOFORMAT_DATA_ID_504 = 9802;

const USHORT AUTOFORMAT_ID_552      = 9901;
const USHORT AUTOFORMAT_DATA_ID_552 = 9902;

// aktuelle Version
const USHORT AUTOFORMAT_ID          = AUTOFORMAT_ID_552;
const USHORT AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_552;


#ifdef READ_OLDVERS
const USHORT AUTOFORMAT_OLD_ID_OLD  = 4201;
const USHORT AUTOFORMAT_OLD_DATA_ID = 4202;
const USHORT AUTOFORMAT_OLD_ID_NEW  = 4203;
#endif


//  Struct mit Versionsnummern der Items

struct ScAfVersions
{
public:
    USHORT nFontVersion;
    USHORT nFontHeightVersion;
    USHORT nWeightVersion;
    USHORT nPostureVersion;
    USHORT nUnderlineVersion;
    USHORT nCrossedOutVersion;
    USHORT nContourVersion;
    USHORT nShadowedVersion;
    USHORT nColorVersion;
    USHORT nBoxVersion;
    USHORT nBrushVersion;

    USHORT nAdjustVersion;

    USHORT nHorJustifyVersion;
    USHORT nVerJustifyVersion;
    USHORT nOrientationVersion;
    USHORT nMarginVersion;
    USHORT nBoolVersion;
    USHORT nInt32Version;
    USHORT nRotateModeVersion;

    USHORT nNumFmtVersion;

    ScAfVersions();
    void Load( SvStream& rStream, USHORT nVer );
    static void Write(SvStream& rStream);
};

ScAfVersions::ScAfVersions() :
    nFontVersion(0),
    nFontHeightVersion(0),
    nWeightVersion(0),
    nPostureVersion(0),
    nUnderlineVersion(0),
    nCrossedOutVersion(0),
    nContourVersion(0),
    nShadowedVersion(0),
    nColorVersion(0),
    nBoxVersion(0),
    nBrushVersion(0),
    nAdjustVersion(0),
    nHorJustifyVersion(0),
    nVerJustifyVersion(0),
    nOrientationVersion(0),
    nMarginVersion(0),
    nBoolVersion(0),
    nInt32Version(0),
    nRotateModeVersion(0),
    nNumFmtVersion(0)
{
}

void ScAfVersions::Load( SvStream& rStream, USHORT nVer )
{
    rStream >> nFontVersion;
    rStream >> nFontHeightVersion;
    rStream >> nWeightVersion;
    rStream >> nPostureVersion;
    rStream >> nUnderlineVersion;
    rStream >> nCrossedOutVersion;
    rStream >> nContourVersion;
    rStream >> nShadowedVersion;
    rStream >> nColorVersion;
    rStream >> nBoxVersion;
    rStream >> nBrushVersion;
    rStream >> nAdjustVersion;
    rStream >> nHorJustifyVersion;
    rStream >> nVerJustifyVersion;
    rStream >> nOrientationVersion;
    rStream >> nMarginVersion;
    rStream >> nBoolVersion;
    if ( nVer >= AUTOFORMAT_ID_504 )
    {
        rStream >> nInt32Version;
        rStream >> nRotateModeVersion;
    }
    rStream >> nNumFmtVersion;
}

void ScAfVersions::Write(SvStream& rStream)
{
    rStream << SvxFontItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxFontHeightItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxWeightItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxPostureItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxUnderlineItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxCrossedOutItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxContourItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxShadowedItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxColorItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxBoxItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxBrushItem().GetVersion(SOFFICE_FILEFORMAT_40);

    rStream << SvxAdjustItem().GetVersion(SOFFICE_FILEFORMAT_40);

    rStream << SvxHorJustifyItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxVerJustifyItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxOrientationItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxMarginItem().GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SfxBoolItem(ATTR_LINEBREAK).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SfxInt32Item(ATTR_ROTATE_VALUE).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxRotateModeItem(SVX_ROTATE_MODE_STANDARD,0).GetVersion(SOFFICE_FILEFORMAT_40);

    rStream << (USHORT)0;       // Num-Format
}

//  ---------------------------------------------------------------------------

ScAutoFormatData::ScAutoFormatData()
{
    nStrResId = USHRT_MAX;

    bIncludeValueFormat =
    bIncludeFont =
    bIncludeJustify =
    bIncludeFrame =
    bIncludeBackground =
    bIncludeWidthHeight = TRUE;
    for (USHORT i = 0; i < 16; i++)
    {
        pFont[i]            = new SvxFontItem;
        pFontHeight[i]      = new SvxFontHeightItem;
        pFontWeight[i]      = new SvxWeightItem;
        pFontPosture[i]     = new SvxPostureItem;
        pFontUnderline[i]   = new SvxUnderlineItem;
        pFontCrossedOut[i]  = new SvxCrossedOutItem;
        pFontContour[i]     = new SvxContourItem;
        pFontShadowed[i]    = new SvxShadowedItem;
        pFontColor[i]       = new SvxColorItem;
        pBox[i]             = new SvxBoxItem;
        pBackground[i]      = new SvxBrushItem;

        pAdjust[i]          = new SvxAdjustItem;

        pHorJustify[i]      = new SvxHorJustifyItem;
        pVerJustify[i]      = new SvxVerJustifyItem;
        pOrientation[i]     = new SvxOrientationItem;
        pMargin[i]          = new SvxMarginItem;
        pLinebreak[i]       = new SfxBoolItem( ATTR_LINEBREAK );
        pRotateAngle[i]     = new SfxInt32Item( ATTR_ROTATE_VALUE );
        pRotateMode[i]      = new SvxRotateModeItem( SVX_ROTATE_MODE_STANDARD, ATTR_ROTATE_MODE );

        pNumFormat[i]       = new ScNumFormatAbbrev;
    }
}

ScAutoFormatData::ScAutoFormatData( const ScAutoFormatData& rData ) :
        aName( rData.aName ),
        nStrResId( rData.nStrResId ),
        bIncludeValueFormat( rData.bIncludeValueFormat ),
        bIncludeFont( rData.bIncludeFont ),
        bIncludeJustify( rData.bIncludeJustify ),
        bIncludeFrame( rData.bIncludeFrame ),
        bIncludeBackground( rData.bIncludeBackground ),
        bIncludeWidthHeight( rData.bIncludeWidthHeight )
{
    for (USHORT i = 0; i < 16; i++)
    {
        pFont[i]            = new SvxFontItem(*rData.pFont[i]);
        pFontHeight[i]      = new SvxFontHeightItem(*rData.pFontHeight[i]);
        pFontWeight[i]      = new SvxWeightItem(*rData.pFontWeight[i]);
        pFontPosture[i]     = new SvxPostureItem(*rData.pFontPosture[i]);
        pFontUnderline[i]   = new SvxUnderlineItem(*rData.pFontUnderline[i]);
        pFontCrossedOut[i]  = new SvxCrossedOutItem(*rData.pFontCrossedOut[i]);
        pFontContour[i]     = new SvxContourItem(*rData.pFontContour[i]);
        pFontShadowed[i]    = new SvxShadowedItem(*rData.pFontShadowed[i]);
        pFontColor[i]       = new SvxColorItem(*rData.pFontColor[i]);
        pBox[i]             = new SvxBoxItem(*rData.pBox[i]);
        pBackground[i]      = new SvxBrushItem(*rData.pBackground[i]);

        pAdjust[ i ]        = new SvxAdjustItem( *rData.pAdjust[ i ] );

        pHorJustify[i]      = new SvxHorJustifyItem(*rData.pHorJustify[i]);
        pVerJustify[i]      = new SvxVerJustifyItem(*rData.pVerJustify[i]);
        pOrientation[i]     = new SvxOrientationItem(*rData.pOrientation[i]);
        pMargin[i]          = new SvxMarginItem(*rData.pMargin[i]);
        pLinebreak[i]       = new SfxBoolItem(*rData.pLinebreak[i]);
        pRotateAngle[i]     = new SfxInt32Item(*rData.pRotateAngle[i]);
        pRotateMode[i]      = new SvxRotateModeItem(*rData.pRotateMode[i]);

        pNumFormat[i]       = new ScNumFormatAbbrev(*rData.pNumFormat[i]);
    }
}

ScAutoFormatData::~ScAutoFormatData()
{
    for (USHORT i = 0; i < 16; i++)
    {
        delete pFont[i];
        delete pFontHeight[i];
        delete pFontWeight[i];
        delete pFontPosture[i];
        delete pFontUnderline[i];
        delete pFontCrossedOut[i];
        delete pFontContour[i];
        delete pFontShadowed[i];
        delete pFontColor[i];
        delete pBox[i];
        delete pBackground[i];

        delete pAdjust[i];

        delete pHorJustify[i];
        delete pVerJustify[i];
        delete pOrientation[i];
        delete pMargin[i];
        delete pLinebreak[i];
        delete pRotateAngle[i];
        delete pRotateMode[i];

        delete pNumFormat[i];
    }
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetNumFormat(USHORT nIndex, ScNumFormatAbbrev& rNumFormat) const
{
    rNumFormat = *pNumFormat[nIndex];
}

void ScAutoFormatData::SetNumFormat(USHORT nIndex, const ScNumFormatAbbrev& rNumFormat)
{
    *pNumFormat[nIndex] = rNumFormat;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetFont(USHORT nIndex, SvxFontItem& rFont) const
{
    rFont = *pFont[nIndex];
}

void ScAutoFormatData::SetFont(USHORT nIndex, const SvxFontItem& rFont)
{
    *pFont[nIndex] = rFont;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetFontHeight(USHORT nIndex, SvxFontHeightItem& rFontHeight) const
{
    rFontHeight = *pFontHeight[nIndex];
}

void ScAutoFormatData::SetFontHeight(USHORT nIndex, const SvxFontHeightItem& rFontHeight)
{
    *pFontHeight[nIndex] = rFontHeight;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetFontWeight(USHORT nIndex, SvxWeightItem& rFontWeight) const
{
    rFontWeight = *pFontWeight[nIndex];
}

void ScAutoFormatData::SetFontWeight(USHORT nIndex, const SvxWeightItem& rFontWeight)
{
    *pFontWeight[nIndex] = rFontWeight;
}

void ScAutoFormatData::GetFontPosture(USHORT nIndex, SvxPostureItem& rFontPosture) const
{
    rFontPosture = *pFontPosture[nIndex];
}

void ScAutoFormatData::SetFontPosture(USHORT nIndex, const SvxPostureItem& rFontPosture)
{
    *pFontPosture[nIndex] = rFontPosture;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetFontUnderline(USHORT nIndex, SvxUnderlineItem& rFontUnderline) const
{
    rFontUnderline = *pFontUnderline[nIndex];
}

void ScAutoFormatData::SetFontUnderline(USHORT nIndex, const SvxUnderlineItem& rFontUnderline)
{
    *pFontUnderline[nIndex] = rFontUnderline;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetFontCrossedOut(USHORT nIndex, SvxCrossedOutItem& rFontCrossedOut) const
{
    rFontCrossedOut = *pFontCrossedOut[nIndex];
}

void ScAutoFormatData::SetFontCrossedOut(USHORT nIndex, const SvxCrossedOutItem& rFontCrossedOut)
{
    *pFontCrossedOut[nIndex] = rFontCrossedOut;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetFontContour(USHORT nIndex, SvxContourItem& rFontContour) const
{
    rFontContour = *pFontContour[nIndex];
}

void ScAutoFormatData::SetFontContour(USHORT nIndex, const SvxContourItem& rFontContour)
{
    *pFontContour[nIndex] = rFontContour;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetFontShadowed(USHORT nIndex, SvxShadowedItem& rFontShadowed) const
{
    rFontShadowed = *pFontShadowed[nIndex];
}

void ScAutoFormatData::SetFontShadowed(USHORT nIndex, const SvxShadowedItem& rFontShadowed)
{
    *pFontShadowed[nIndex] = rFontShadowed;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetFontColor(USHORT nIndex, SvxColorItem& rFontColor) const
{
    rFontColor = *pFontColor[nIndex];
}

void ScAutoFormatData::SetFontColor(USHORT nIndex, const SvxColorItem& rFontColor)
{
    *pFontColor[nIndex] = rFontColor;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetHorJustify(USHORT nIndex, SvxHorJustifyItem& rHorJustify) const
{
    rHorJustify = *pHorJustify[nIndex];
}

void ScAutoFormatData::SetHorJustify(USHORT nIndex, const SvxHorJustifyItem& rHorJustify)
{
    *pHorJustify[nIndex] = rHorJustify;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetVerJustify(USHORT nIndex, SvxVerJustifyItem& rVerJustify) const
{
    rVerJustify = *pVerJustify[nIndex];
}

void ScAutoFormatData::SetVerJustify(USHORT nIndex, const SvxVerJustifyItem& rVerJustify)
{
    *pVerJustify[nIndex] = rVerJustify;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetOrientation(USHORT nIndex, SvxOrientationItem& rOrientation) const
{
    rOrientation = *pOrientation[nIndex];
}

void ScAutoFormatData::SetOrientation(USHORT nIndex, const SvxOrientationItem& rOrientation)
{
    *pOrientation[nIndex] = rOrientation;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetLinebreak(USHORT nIndex, SfxBoolItem& rLinebreak) const
{
    rLinebreak.SetValue( pLinebreak[nIndex]->GetValue() );
}

void ScAutoFormatData::SetLinebreak(USHORT nIndex, const SfxBoolItem& rLinebreak)
{
    pLinebreak[nIndex]->SetValue( rLinebreak.GetValue() );
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetMargin(USHORT nIndex, SvxMarginItem& rMargin) const
{
    rMargin = *pMargin[nIndex];
}

void ScAutoFormatData::SetMargin(USHORT nIndex, const SvxMarginItem& rMargin)
{
    *pMargin[nIndex] = rMargin;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetBox(USHORT nIndex, SvxBoxItem& rBox) const
{
    rBox = *pBox[nIndex];
}

void ScAutoFormatData::SetBox(USHORT nIndex, const SvxBoxItem& rBox)
{
    *pBox[nIndex] = rBox;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetBackground(USHORT nIndex, SvxBrushItem& rBackground) const
{
    rBackground = *pBackground[nIndex];
}

void ScAutoFormatData::SetBackground(USHORT nIndex, const SvxBrushItem& rBackground)
{
    *pBackground[nIndex] = rBackground;
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetRotateAngle( USHORT nIndex, SfxInt32Item& rRotateAngle ) const
{
    rRotateAngle.SetValue( pRotateAngle[nIndex]->GetValue() );
}

void ScAutoFormatData::SetRotateAngle( USHORT nIndex, const SfxInt32Item& rRotateAngle )
{
    pRotateAngle[nIndex]->SetValue( rRotateAngle.GetValue() );
}

//---------------------------------------------------------------------------------------

void ScAutoFormatData::GetRotateMode( USHORT nIndex, SvxRotateModeItem& rRotateMode ) const
{
    rRotateMode.SetValue( pRotateMode[nIndex]->GetValue() );
}

void ScAutoFormatData::SetRotateMode( USHORT nIndex, const SvxRotateModeItem& rRotateMode )
{
    pRotateMode[nIndex]->SetValue( rRotateMode.GetValue() );
}

//---------------------------------------------------------------------------------------

const SfxPoolItem* ScAutoFormatData::GetItem( USHORT nIndex, USHORT nWhich ) const
{
    switch (nWhich)
    {
        case ATTR_FONT:             return pFont[nIndex];
        case ATTR_FONT_HEIGHT:      return pFontHeight[nIndex];
        case ATTR_FONT_WEIGHT:      return pFontWeight[nIndex];
        case ATTR_FONT_POSTURE:     return pFontPosture[nIndex];
        case ATTR_FONT_UNDERLINE:   return pFontUnderline[nIndex];
        case ATTR_FONT_CROSSEDOUT:  return pFontCrossedOut[nIndex];
        case ATTR_FONT_CONTOUR:     return pFontContour[nIndex];
        case ATTR_FONT_SHADOWED:    return pFontShadowed[nIndex];
        case ATTR_FONT_COLOR:       return pFontColor[nIndex];
        case ATTR_BORDER:           return pBox[nIndex];
        case ATTR_BACKGROUND:       return pBackground[nIndex];
        case ATTR_HOR_JUSTIFY:      return pHorJustify[nIndex];
        case ATTR_VER_JUSTIFY:      return pVerJustify[nIndex];
        case ATTR_ORIENTATION:      return pOrientation[nIndex];
        case ATTR_MARGIN:           return pMargin[nIndex];
        case ATTR_LINEBREAK:        return pLinebreak[nIndex];
        case ATTR_ROTATE_VALUE:     return pRotateAngle[nIndex];
        case ATTR_ROTATE_MODE:      return pRotateMode[nIndex];
    }
    return NULL;
}

void ScAutoFormatData::PutItem( USHORT nIndex, const SfxPoolItem& rItem )
{
    switch (rItem.Which())
    {
        case ATTR_FONT:
            *pFont[nIndex] = (const SvxFontItem&)rItem;
            break;
        case ATTR_FONT_HEIGHT:
            *pFontHeight[nIndex] = (const SvxFontHeightItem&)rItem;
            break;
        case ATTR_FONT_WEIGHT:
            *pFontWeight[nIndex] = (const SvxWeightItem&)rItem;
            break;
        case ATTR_FONT_POSTURE:
            *pFontPosture[nIndex] = (const SvxPostureItem&)rItem;
            break;
        case ATTR_FONT_UNDERLINE:
            *pFontUnderline[nIndex] = (const SvxUnderlineItem&)rItem;
            break;
        case ATTR_FONT_CROSSEDOUT:
            *pFontCrossedOut[nIndex] = (const SvxCrossedOutItem&)rItem;
            break;
        case ATTR_FONT_CONTOUR:
            *pFontContour[nIndex] = (const SvxContourItem&)rItem;
            break;
        case ATTR_FONT_SHADOWED:
            *pFontShadowed[nIndex] = (const SvxShadowedItem&)rItem;
            break;
        case ATTR_FONT_COLOR:
            *pFontColor[nIndex] = (const SvxColorItem&)rItem;
            break;
        case ATTR_BORDER:
            *pBox[nIndex] = (const SvxBoxItem&)rItem;
            break;
        case ATTR_BACKGROUND:
            *pBackground[nIndex] = (const SvxBrushItem&)rItem;
            break;
        case ATTR_HOR_JUSTIFY:
            *pHorJustify[nIndex] = (const SvxHorJustifyItem&)rItem;
            break;
        case ATTR_VER_JUSTIFY:
            *pVerJustify[nIndex] = (const SvxVerJustifyItem&)rItem;
            break;
        case ATTR_ORIENTATION:
            *pOrientation[nIndex] = (const SvxOrientationItem&)rItem;
            break;
        case ATTR_MARGIN:
            *pMargin[nIndex] = (const SvxMarginItem&)rItem;
            break;
        case ATTR_LINEBREAK:
            pLinebreak[nIndex]->SetValue( ((const SfxBoolItem&)rItem).GetValue() );
            break;
        case ATTR_ROTATE_VALUE:
            pRotateAngle[nIndex]->SetValue( ((const SfxInt32Item&)rItem).GetValue() );
            break;
        case ATTR_ROTATE_MODE:
            pRotateMode[nIndex]->SetValue( ((const SvxRotateModeItem&)rItem).GetValue() );
            break;
    }
}

//---------------------------------------------------------------------------------------

BOOL ScAutoFormatData::IsEqualData(USHORT nIndex1, USHORT nIndex2)
{
    BOOL bEqual = TRUE;
    if (bIncludeValueFormat)
    {
        bEqual = (bEqual && (*pNumFormat[nIndex1] == *pNumFormat[nIndex2]));
    }
    if (bIncludeFont)
    {
        bEqual = (bEqual && (*pFont[nIndex1] == *pFont[nIndex2]));
        bEqual = (bEqual && (*pFontHeight[nIndex1] == *pFontHeight[nIndex2]));
        bEqual = (bEqual && (*pFontWeight[nIndex1] == *pFontWeight[nIndex2]));
        bEqual = (bEqual && (*pFontPosture[nIndex1] == *pFontPosture[nIndex2]));
        bEqual = (bEqual && (*pFontUnderline[nIndex1] == *pFontUnderline[nIndex2]));
        bEqual = (bEqual && (*pFontCrossedOut[nIndex1] == *pFontCrossedOut[nIndex2]));
        bEqual = (bEqual && (*pFontContour[nIndex1] == *pFontContour[nIndex2]));
        bEqual = (bEqual && (*pFontShadowed[nIndex1] == *pFontShadowed[nIndex2]));
        bEqual = (bEqual && (*pFontColor[nIndex1] == *pFontColor[nIndex2]));
    }
    if (bIncludeJustify)
    {
        bEqual = (bEqual && (*pHorJustify[nIndex1] == *pHorJustify[nIndex2]));
        bEqual = (bEqual && (*pVerJustify[nIndex1] == *pVerJustify[nIndex2]));
        bEqual = (bEqual && (*pOrientation[nIndex1] == *pOrientation[nIndex2]));
        bEqual = (bEqual && (*pLinebreak[nIndex1] == *pLinebreak[nIndex2]));
        bEqual = (bEqual && (*pMargin[nIndex1] == *pMargin[nIndex2]));
        bEqual = (bEqual && (*pRotateAngle[nIndex1] == *pRotateAngle[nIndex2]));
        bEqual = (bEqual && (*pRotateMode[nIndex1] == *pRotateMode[nIndex2]));
    }
    if (bIncludeFrame)
        bEqual = (bEqual && (*pBox[nIndex1] == *pBox[nIndex2]));
    if (bIncludeBackground)
        bEqual = (bEqual && (*pBackground[nIndex1] == *pBackground[nIndex2]));
    return bEqual;
}

#define READ( i, aItem, aItemType, nVers )\
    pNew = aItem[i]->Create(rStream, nVers ); \
    *aItem[i] = *(aItemType*)pNew; \
    delete pNew;

BOOL ScAutoFormatData::Load(SvStream& rStream, const ScAfVersions& rVersions)
{
    BOOL    bRet = TRUE;
    USHORT  nVal = 0;
    rStream >> nVal;
    bRet = 0 == rStream.GetError();
    if( bRet && (nVal == AUTOFORMAT_DATA_ID_X ||
            (AUTOFORMAT_DATA_ID_504 <= nVal && nVal <= AUTOFORMAT_DATA_ID)) )
    {
        CharSet eSysSet = gsl_getSystemTextEncoding();
        CharSet eSrcSet = rStream.GetStreamCharSet();

        BOOL b;
        rStream.ReadByteString( aName, eSrcSet );
        if( AUTOFORMAT_DATA_ID_552 <= nVal )
        {
            rStream >> nStrResId;
            USHORT nId = RID_SVXSTR_TBLAFMT_BEGIN + nStrResId;
            if( RID_SVXSTR_TBLAFMT_BEGIN <= nId &&
                nId < RID_SVXSTR_TBLAFMT_END )
            {
                aName = SVX_RESSTR( nId );
            }
            else
                nStrResId = USHRT_MAX;
        }

        rStream >> b; bIncludeFont = b;
        rStream >> b; bIncludeJustify = b;
        rStream >> b; bIncludeFrame = b;
        rStream >> b; bIncludeBackground = b;
        rStream >> b; bIncludeValueFormat = b;
        rStream >> b; bIncludeWidthHeight = b;

        SfxPoolItem* pNew;
        bRet = 0 == rStream.GetError();
        for( USHORT i = 0; bRet && i < 16; ++i )
        {
            READ( i, pFont,             SvxFontItem         , rVersions.nFontVersion)
            READ( i, pFontHeight,       SvxFontHeightItem   , rVersions.nFontHeightVersion)
            READ( i, pFontWeight,       SvxWeightItem       , rVersions.nWeightVersion)
            READ( i, pFontPosture,      SvxPostureItem      , rVersions.nPostureVersion)
            READ( i, pFontUnderline,    SvxUnderlineItem    , rVersions.nUnderlineVersion)
            READ( i, pFontCrossedOut,   SvxCrossedOutItem   , rVersions.nCrossedOutVersion)
            READ( i, pFontContour,      SvxContourItem      , rVersions.nContourVersion)
            READ( i, pFontShadowed,     SvxShadowedItem     , rVersions.nShadowedVersion)
            READ( i, pFontColor,        SvxColorItem        , rVersions.nColorVersion)
            READ( i, pBox,              SvxBoxItem          , rVersions.nBoxVersion)
            READ( i, pBackground,       SvxBrushItem        , rVersions.nBrushVersion)

            pNew = pAdjust[i]->Create(rStream, rVersions.nAdjustVersion );
            ((SvxAdjustItem*)pAdjust[i])->SetAdjust( ((SvxAdjustItem*)pNew)->GetAdjust() );
            ((SvxAdjustItem*)pAdjust[i])->SetOneWord( ((SvxAdjustItem*)pNew)->GetOneWord() );
            ((SvxAdjustItem*)pAdjust[i])->SetLastBlock( ((SvxAdjustItem*)pNew)->GetLastBlock() );
            delete pNew;

            READ( i, pHorJustify,   SvxHorJustifyItem,  rVersions.nHorJustifyVersion)
            READ( i, pVerJustify,   SvxVerJustifyItem,  rVersions.nVerJustifyVersion)
            READ( i, pOrientation,  SvxOrientationItem, rVersions.nOrientationVersion)
            READ( i, pMargin,       SvxMarginItem,      rVersions.nMarginVersion)

            pNew = pLinebreak[i]->Create(rStream, rVersions.nBoolVersion );
            pLinebreak[i]->SetValue( ((SfxBoolItem*)pNew)->GetValue() );
            delete pNew;

            if ( nVal >= AUTOFORMAT_DATA_ID_504 )
            {
                pNew = pRotateAngle[i]->Create( rStream, rVersions.nInt32Version );
                pRotateAngle[i]->SetValue( ((SfxInt32Item*)pNew)->GetValue() );
                delete pNew;
                pNew = pRotateMode[i]->Create( rStream, rVersions.nRotateModeVersion );
                pRotateMode[i]->SetValue( ((SvxRotateModeItem*)pNew)->GetValue() );
                delete pNew;
            }

            if( 0 == rVersions.nNumFmtVersion )
                pNumFormat[i]->Load( rStream );

            //  CharSet in Font anpassen
            if ( eSrcSet != eSysSet && pFont[i]->GetCharSet() == eSrcSet )
                pFont[i]->GetCharSet() = eSysSet;

            bRet = 0 == rStream.GetError();
        }
    }
    else
        bRet = FALSE;
    return bRet;
}

#ifdef READ_OLDVERS

BOOL ScAutoFormatData::LoadOld(SvStream& rStream, const ScAfVersions& rVersions)
{
    BOOL    bRet = TRUE;
    USHORT  nVal = 0;
    rStream >> nVal;
    bRet = (rStream.GetError() == 0);
    if (bRet && (nVal == AUTOFORMAT_OLD_DATA_ID))
    {
        rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
        BOOL b;
        rStream >> b; bIncludeFont = b;
        rStream >> b; bIncludeJustify = b;
        rStream >> b; bIncludeFrame = b;
        rStream >> b; bIncludeBackground = b;
        rStream >> b; bIncludeValueFormat = b;
        rStream >> b; bIncludeWidthHeight = b;

        bRet = 0 == rStream.GetError();
        SfxPoolItem* pNew;
        for (USHORT i=0; bRet && i < 16; i++)
        {
            pNumFormat[i]->Load(rStream);

            READ( i, pFont,             SvxFontItem         , rVersions.nFontVersion)
            READ( i, pFontHeight,       SvxFontHeightItem   , rVersions.nFontHeightVersion)
            READ( i, pFontWeight,       SvxWeightItem       , rVersions.nWeightVersion)
            READ( i, pFontPosture,      SvxPostureItem      , rVersions.nPostureVersion)
            READ( i, pFontUnderline,    SvxUnderlineItem    , rVersions.nUnderlineVersion)
            READ( i, pFontCrossedOut,   SvxCrossedOutItem   , rVersions.nCrossedOutVersion)
            READ( i, pFontContour,      SvxContourItem      , rVersions.nContourVersion)
            READ( i, pFontShadowed,     SvxShadowedItem     , rVersions.nShadowedVersion)
            READ( i, pFontColor,        SvxColorItem        , rVersions.nColorVersion)
            READ( i, pHorJustify,       SvxHorJustifyItem,  rVersions.nHorJustifyVersion)
            READ( i, pVerJustify,       SvxVerJustifyItem,  rVersions.nVerJustifyVersion)
            READ( i, pOrientation,      SvxOrientationItem, rVersions.nOrientationVersion)
            pNew = pLinebreak[i]->Create(rStream, rVersions.nBoolVersion );
            pLinebreak[i]->SetValue( ((SfxBoolItem*)pNew)->GetValue() );
            delete pNew;
            READ( i, pMargin,           SvxMarginItem,      rVersions.nMarginVersion)
            READ( i, pBox,              SvxBoxItem          , rVersions.nBoxVersion)
            READ( i, pBackground,       SvxBrushItem        , rVersions.nBrushVersion)

            bRet = (rStream.GetError() == 0);
        }
    }
    else
        bRet = FALSE;
    return bRet;
}

#endif

BOOL ScAutoFormatData::Save(SvStream& rStream)
{
    USHORT nVal = AUTOFORMAT_DATA_ID;
    BOOL b;
    rStream << nVal;
    rStream.WriteByteString( aName, rStream.GetStreamCharSet() );

#if 0
    //  This was an internal flag to allow creating AutoFormats with localized names

    if ( USHRT_MAX == nStrResId )
    {
        String aIniVal( SFX_APP()->GetIniManager()->Get(
            SFX_GROUP_WORKINGSET_IMPL,
            String( RTL_CONSTASCII_USTRINGPARAM( "SaveTableAutoFmtNameId" ))));
        if( 0 != aIniVal.ToInt32() )
        {
            // check Name for ResId
            for( USHORT nId = RID_SVXSTR_TBLAFMT_BEGIN;
                        RID_SVXSTR_TBLAFMT_END > nId; ++nId )
            {
                String s( SVX_RES( nId ) );
                if( s == aName )
                {
                    nStrResId = nId - RID_SVXSTR_TBLAFMT_BEGIN;
                    break;
                }
            }
        }
    }
#endif

    rStream << nStrResId;
    rStream << ( b = bIncludeFont );
    rStream << ( b = bIncludeJustify );
    rStream << ( b = bIncludeFrame );
    rStream << ( b = bIncludeBackground );
    rStream << ( b = bIncludeValueFormat );
    rStream << ( b = bIncludeWidthHeight );
    BOOL bRet = 0 == rStream.GetError();
    for (USHORT i = 0; bRet && (i < 16); i++)
    {
        pFont[i]->Store(rStream, pFont[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pFontHeight[i]->Store(rStream, pFontHeight[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pFontWeight[i]->Store(rStream, pFontWeight[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pFontPosture[i]->Store(rStream, pFontPosture[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pFontUnderline[i]->Store(rStream, pFontUnderline[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pFontCrossedOut[i]->Store(rStream, pFontCrossedOut[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pFontContour[i]->Store(rStream, pFontContour[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pFontShadowed[i]->Store(rStream, pFontShadowed[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pFontColor[i]->Store(rStream, pFontColor[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pBox[i]->Store(rStream, pBox[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pBackground[i]->Store(rStream, pBackground[i]->GetVersion(SOFFICE_FILEFORMAT_40));

        pAdjust[i]->Store(rStream, pAdjust[i]->GetVersion(SOFFICE_FILEFORMAT_40));

        pHorJustify[i]->Store(rStream, pHorJustify[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pVerJustify[i]->Store(rStream, pVerJustify[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pOrientation[i]->Store(rStream, pOrientation[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pMargin[i]->Store(rStream, pMargin[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pLinebreak[i]->Store(rStream, pLinebreak[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        // Rotation ab SO5
        pRotateAngle[i]->Store(rStream, pRotateAngle[i]->GetVersion(SOFFICE_FILEFORMAT_40));
        pRotateMode[i]->Store(rStream, pRotateMode[i]->GetVersion(SOFFICE_FILEFORMAT_40));

        pNumFormat[i]->Save(rStream);
        bRet = (rStream.GetError() == 0);
    }
    return bRet;
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

ScAutoFormat::ScAutoFormat(USHORT nLim, USHORT nDel, BOOL bDup):
    SortedCollection        (nLim, nDel, bDup),
    bSaveLater              (FALSE)
{
    //  Default "Standard" Format anlegen
    ScAutoFormatData* pData = new ScAutoFormatData();
    String aName(ScGlobal::GetRscString(STR_STYLENAME_STANDARD));
    pData->SetName(aName);

    //  Default-Font und Groesse
    Font aStdFont = System::GetStandardFont( STDFONT_SWISS );
    SvxFontItem aFontItem( aStdFont.GetFamily(), aStdFont.GetName(), aStdFont.GetStyleName(),
                            aStdFont.GetPitch(), aStdFont.GetCharSet() );
    SvxFontHeightItem aHeight( 200 );       // 10 pt;

    //  schwarze duenne Umrandung
    Color aBlack( COL_BLACK );
    SvxBorderLine aLine( &aBlack, DEF_LINE_WIDTH_0 );
    SvxBoxItem aBox;
    aBox.SetLine(&aLine, BOX_LINE_LEFT);
    aBox.SetLine(&aLine, BOX_LINE_TOP);
    aBox.SetLine(&aLine, BOX_LINE_RIGHT);
    aBox.SetLine(&aLine, BOX_LINE_BOTTOM);

    Color aWhite(COL_WHITE);
    Color aBlue(COL_BLUE);
    SvxColorItem aWhiteText( aWhite );
    SvxColorItem aBlackText( aBlack );
    SvxBrushItem aBlueBack( aBlue );
    SvxBrushItem aWhiteBack( aWhite );
    SvxBrushItem aGray70Back( Color(0x4d, 0x4d, 0x4d) );
    SvxBrushItem aGray20Back( Color(0xcc, 0xcc, 0xcc) );

    for (USHORT i=0; i<16; i++)
    {
        pData->SetBox( i, aBox );
        pData->SetFont( i, aFontItem );
        pData->SetFontHeight( i, aHeight );
        if (i<4)                                    // oben: weiss auf blau
        {
            pData->SetFontColor( i, aWhiteText );
            pData->SetBackground( i, aBlueBack );
        }
        else if ( i%4 == 0 )                        // links: weiss auf grau70
        {
            pData->SetFontColor( i, aWhiteText );
            pData->SetBackground( i, aGray70Back );
        }
        else if ( i%4 == 3 || i >= 12 )             // rechts & unten: schwarz auf grau20
        {
            pData->SetFontColor( i, aBlackText );
            pData->SetBackground( i, aGray20Back );
        }
        else                                        // Mitte: schwarz auf weiss
        {
            pData->SetFontColor( i, aBlackText );
            pData->SetBackground( i, aWhiteBack );
        }
    }

    Insert(pData);
}

ScAutoFormat::ScAutoFormat(const ScAutoFormat& rAutoFormat) :
    SortedCollection (rAutoFormat),
    bSaveLater       (FALSE)
{}

ScAutoFormat::~ScAutoFormat()
{
    //  Bei Aenderungen per StarOne wird nicht sofort gespeichert, sondern zuerst nur
    //  das SaveLater Flag gesetzt. Wenn das Flag noch gesetzt ist, jetzt speichern.

    if (bSaveLater)
        Save();
}

void ScAutoFormat::SetSaveLater( BOOL bSet )
{
    bSaveLater = bSet;
}

short ScAutoFormat::Compare(DataObject* pKey1, DataObject* pKey2) const
{
    StringCompare eComp;
    String aStr1;
    String aStr2;
    ((ScAutoFormatData*)pKey1)->GetName(aStr1);
    ((ScAutoFormatData*)pKey2)->GetName(aStr2);
    String aStrStandard = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
    if ( ScGlobal::pScInternational->CompareEqual( aStr1, aStrStandard, INTN_COMPARE_IGNORECASE ) )
        return -1;
    if ( ScGlobal::pScInternational->CompareEqual( aStr2, aStrStandard, INTN_COMPARE_IGNORECASE ) )
        return 1;
    eComp = ScGlobal::pScInternational->Compare( aStr2, aStr1, INTN_COMPARE_IGNORECASE );
    if (eComp == COMPARE_EQUAL)
        return 0;
    else if (eComp == COMPARE_LESS)
        return -1;
    else
        return 1;
}

BOOL ScAutoFormat::Load()
{
    BOOL bRet = TRUE;

    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( sAutoTblFmtName ) ) );

    SfxMedium aMedium( aURL.GetMainURL(), STREAM_READ, TRUE );
    SvStream* pStream = aMedium.GetInStream();
    bRet = (pStream && pStream->GetError() == 0);
    if (bRet)
    {
        SvStream& rStream = *pStream;
        // Achtung hier muss ein allgemeiner Header gelesen werden
        USHORT nVal = 0;
        rStream >> nVal;
        bRet = 0 == rStream.GetError();

        ScAfVersions aVersions;

        if (bRet)
        {
            if( nVal == AUTOFORMAT_ID_358 ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                UINT16 nFileVers = SOFFICE_FILEFORMAT_40;
                BYTE nChrSet, nCnt;
                long nPos = rStream.Tell();
                rStream >> nCnt >> nChrSet;
//              if( 4 <= nCnt )
//                  rStream >> nFileVers;
                if( rStream.Tell() != ULONG(nPos + nCnt) )
                {
                    DBG_ERRORFILE( "Der Header enthaelt mehr/neuere Daten" );
                    rStream.Seek( nPos + nCnt );
                }
                rStream.SetStreamCharSet( (CharSet)nChrSet );
                rStream.SetVersion( nFileVers );
            }

            if( nVal == AUTOFORMAT_ID_358 || nVal == AUTOFORMAT_ID_X ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                aVersions.Load( rStream, nVal );        // Item-Versionen

                ScAutoFormatData* pData;
                USHORT nAnz = 0;
                rStream >> nAnz;
                bRet = (rStream.GetError() == 0);
                for (USHORT i=0; bRet && (i < nAnz); i++)
                {
                    pData = new ScAutoFormatData();
                    bRet = pData->Load(rStream, aVersions);
                    Insert(pData);
                }
            }
#ifdef READ_OLDVERS
            else
            {
                if( AUTOFORMAT_OLD_ID_NEW == nVal )
                {
                    // alte Version der Versions laden
                    rStream >> aVersions.nFontVersion;
                    rStream >> aVersions.nFontHeightVersion;
                    rStream >> aVersions.nWeightVersion;
                    rStream >> aVersions.nPostureVersion;
                    rStream >> aVersions.nUnderlineVersion;
                    rStream >> aVersions.nCrossedOutVersion;
                    rStream >> aVersions.nContourVersion;
                    rStream >> aVersions.nShadowedVersion;
                    rStream >> aVersions.nColorVersion;
                    rStream >> aVersions.nHorJustifyVersion;
                    rStream >> aVersions.nVerJustifyVersion;
                    rStream >> aVersions.nOrientationVersion;
                    rStream >> aVersions.nBoolVersion;
                    rStream >> aVersions.nMarginVersion;
                    rStream >> aVersions.nBoxVersion;
                    rStream >> aVersions.nBrushVersion;
                }
                if( AUTOFORMAT_OLD_ID_OLD == nVal ||
                    AUTOFORMAT_OLD_ID_NEW == nVal )
                {
                    ScAutoFormatData* pData;
                    USHORT nAnz = 0;
                    rStream >> nAnz;
                    bRet = 0 == rStream.GetError();
                    for( USHORT i=0; bRet && (i < nAnz); ++i )
                    {
                        pData = new ScAutoFormatData();
                        bRet = pData->LoadOld( rStream, aVersions );
                        Insert( pData );
                    }
                }
                else
                    bRet = FALSE;
            }
#endif
        }
    }
    bSaveLater = FALSE;
    return bRet;
}

BOOL ScAutoFormat::Save()
{
    BOOL bRet = TRUE;

    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( sAutoTblFmtName ) ) );

    SfxMedium aMedium( aURL.GetMainURL(), STREAM_WRITE, TRUE );
    SvStream* pStream = aMedium.GetOutStream();
    bRet = (pStream && pStream->GetError() == 0);
    if (bRet)
    {
        SvStream& rStream = *pStream;
        rStream.SetVersion( SOFFICE_FILEFORMAT_40 );

        // Achtung hier muss ein allgemeiner Header gespeichert werden
        USHORT nVal = AUTOFORMAT_ID;
        rStream << nVal
                << (BYTE)2      // Anzahl von Zeichen des Headers incl. diesem
                << (BYTE)::GetStoreCharSet( gsl_getSystemTextEncoding() );
//              << (BYTE)4      // Anzahl von Zeichen des Headers incl. diesem
//              << (BYTE)::GetStoreCharSet(::GetSystemCharSet())
//              << (UNIT16)SOFFICE_FILEFORMAT_NOW;
        ScAfVersions::Write(rStream);           // Item-Versionen

        bRet = (rStream.GetError() == 0);
        //-----------------------------------------------------------
        rStream << (USHORT)(nCount - 1);
        bRet = (rStream.GetError() == 0);
        for (USHORT i=1; bRet && (i < nCount); i++)
            bRet = ((ScAutoFormatData*)pItems[i])->Save(rStream);
        rStream.Flush();

        aMedium.Commit();
    }
    bSaveLater = FALSE;
    return bRet;
}

USHORT ScAutoFormat::FindIndexPerName( const String& rName ) const
{
    String              aName;

    for( USHORT i=0; i<nCount ; i++ )
    {
        ScAutoFormatData* pItem = (ScAutoFormatData*)pItems[i];
        pItem->GetName( aName );

        if( aName == rName )
            return i;
    }

    return 0;
}




