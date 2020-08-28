/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/**
 * @file
 * Implementation file of LwpDrawObjcts and associated class like LwpDrawGroup, LwpDrawRectangle
 *  and so on.
 */
#include <osl/thread.h>
#include <lwpglobalmgr.hxx>
#include "lwpdrawobj.hxx"
#include <lwptools.hxx>
#include <tools/stream.hxx>

#include <xfilter/xfframe.hxx>

#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xfdrawstyle.hxx>
#include <xfilter/xfparastyle.hxx>
#include <xfilter/xfimagestyle.hxx>
#include <xfilter/xftextboxstyle.hxx>
#include <xfilter/xfparagraph.hxx>
#include <xfilter/xffont.hxx>

#include <xfilter/xfdrawrect.hxx>
#include <xfilter/xfdrawpath.hxx>
#include <xfilter/xfdrawline.hxx>
#include <xfilter/xfdrawpolygon.hxx>
#include <xfilter/xfimage.hxx>

#include "lwpcharsetmgr.hxx"
#include "lwpsdwrect.hxx"

/**
 * @descr   Constructor of class LwpDrawObj
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawObj::LwpDrawObj(SvStream * pStream, DrawingOffsetAndScale* pTransData)
    : m_eType(OT_UNDEFINED)
    , m_pStream(pStream)
    , m_pTransData(pTransData)
{
    ReadObjHeaderRecord();
}

/**
 * @descr   read the header record data of lwp-sdw-object
 */
void LwpDrawObj::ReadObjHeaderRecord()
{
    //flags
    m_pStream->SeekRel(1);

    //record Length
    m_pStream->ReadUInt16( m_aObjHeader.nRecLen );

    //boundrect
    m_pStream->ReadInt16( m_aObjHeader.nLeft );
    m_pStream->ReadInt16( m_aObjHeader.nTop );
    m_pStream->ReadInt16( m_aObjHeader.nRight );
    m_pStream->ReadInt16( m_aObjHeader.nBottom );

    //nextObj, prevObj
    m_pStream->SeekRel(4);
}

/**
 * @descr   read the style of closed lwp-sdw-object like: rectangle, ellipse and so on.
 */
void LwpDrawObj::ReadClosedObjStyle()
{
    if ((m_eType != OT_POLYGON) && (m_eType != OT_TEXTART))
    {
        m_pStream->SeekRel(8);
    }

    m_pStream->ReadUChar( m_aClosedObjStyleRec.nLineWidth );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.nLineStyle );

    // pen color
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aPenColor.nR );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aPenColor.nG );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aPenColor.nB );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aPenColor.unused );

    // foreground color
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aForeColor.nR );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aForeColor.nG );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aForeColor.nB );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aForeColor.unused );

    // background color
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aBackColor.nR );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aBackColor.nG );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aBackColor.nB );
    m_pStream->ReadUChar( m_aClosedObjStyleRec.aBackColor.unused );

    // fill style
    m_pStream->ReadUInt16( m_aClosedObjStyleRec.nFillType );
    m_pStream->ReadBytes(m_aClosedObjStyleRec.pFillPattern,
        sizeof(m_aClosedObjStyleRec.pFillPattern));
}

/**
 * @descr   set fill style of drawing objects
 * @param   pStyle   pointer of the drawing xfstyle.
 */
void LwpDrawObj::SetFillStyle(XFDrawStyle* pStyle)
{
    if (!pStyle)
    {
        assert(false);
        return;
    }

    XFColor aForeColor(m_aClosedObjStyleRec.aForeColor.nR,
        m_aClosedObjStyleRec.aForeColor.nG, m_aClosedObjStyleRec.aForeColor.nB);
    XFColor aBackColor(m_aClosedObjStyleRec.aBackColor.nR,
        m_aClosedObjStyleRec.aBackColor.nG, m_aClosedObjStyleRec.aBackColor.nB);

    switch (m_aClosedObjStyleRec.nFillType)
    {
    default: //fall through!
    case FT_TRANSPARENT:
        {
            // set fill style : none
            break;
        }
    case FT_SOLID:
        {
            pStyle->SetAreaColor(aForeColor);
            break;
        }
    case FT_HORZHATCH:
        {
            pStyle->SetAreaColor(aBackColor);
            pStyle->SetAreaLineStyle(enumXFAreaLineSingle, 0, 0.12, aForeColor);
            break;
        }
    case FT_VERTHATCH:
        {
            pStyle->SetAreaColor(aBackColor);
            pStyle->SetAreaLineStyle(enumXFAreaLineSingle, 90, 0.12, aForeColor);
            break;
        }
    case FT_FDIAGHATCH:
        {
            pStyle->SetAreaColor(aBackColor);
            pStyle->SetAreaLineStyle(enumXFAreaLineSingle, 135, 0.09, aForeColor);
            break;
        }
    case FT_BDIAGHATCH:
        {
            pStyle->SetAreaColor(aBackColor);
            pStyle->SetAreaLineStyle(enumXFAreaLineSingle, 45, 0.09, aForeColor);
            break;
        }
    case FT_CROSSHATCH:
        {
            pStyle->SetAreaColor(aBackColor);
            pStyle->SetAreaLineStyle(enumXFAreaLineCrossed, 0, 0.12, aForeColor);
            break;
        }
    case FT_DIAGCROSSHATCH:
        {
            pStyle->SetAreaColor(aBackColor);
            pStyle->SetAreaLineStyle(enumXFAreaLineCrossed, 45, 0.095, aForeColor);
            break;
        }
    }
}

/**
 * @descr   set line/border style of drawing objects.
 * @param   pStyle   pointer of the drawing xfstyle.
 * @param   nWidth   width of line/border.
 * @param   nLineStyle   flag of line/border style: none, dot or solid.
 * @param   rColor   color of line/border.
 */
void LwpDrawObj::SetLineStyle(XFDrawStyle* pStyle, sal_uInt8 nWidth, sal_uInt8 nLineStyle,
    const SdwColor& rColor)
{
    if (!pStyle)
    {
        assert(false);
        return;
    }
    if (nWidth == 0)
    {
        nLineStyle = LS_NULL;
    }

    if (nLineStyle == LS_NULL)
    {
        // set stroke:none
        return;
    }

    if (nLineStyle == LS_DOT)
    {
        pStyle->SetLineDashStyle(enumXFLineDash, 0.05, 0.05, 0.05);
    }

    // line width
    double fWidth = static_cast<double>(nWidth)/TWIPS_PER_CM;

    // line color
    XFColor aXFColor(rColor.nR, rColor.nG, rColor.nB);

    pStyle->SetLineStyle(fWidth, aXFColor);
}

/**
 * @descr   set position of a drawing object in the frame.
 * @param   pObj   pointer of the xf-drawing object
 */
void LwpDrawObj::SetPosition(XFFrame* pObj)
{
    double fOffsetX = 0.00, fOffsetY = 0.00;
    double fScaleX = 1.00, fScaleY = 1.00;
    if (m_pTransData)
    {
        fOffsetX = m_pTransData->fOffsetX;
        fOffsetY = m_pTransData->fOffsetY;
        fScaleX = m_pTransData->fScaleX;
        fScaleY = m_pTransData->fScaleY;
    }

    pObj->SetPosition(static_cast<double>(m_aObjHeader.nLeft)/TWIPS_PER_CM * fScaleX+ fOffsetX,
        static_cast<double>(m_aObjHeader.nTop)/TWIPS_PER_CM * fScaleY + fOffsetY,
        static_cast<double>(m_aObjHeader.nRight-m_aObjHeader.nLeft)/TWIPS_PER_CM * fScaleX,
        static_cast<double>(m_aObjHeader.nBottom-m_aObjHeader.nTop)/TWIPS_PER_CM * fScaleY);
}

/**
 * @descr   set arrowhead of a xf-drawing object. only opened drawing objects can be assigned arrowheads
 * @param   pOpenedObjStyle   the xf-drawing object which will be set arrowhead.
 * @param   nArrowFlag   arrowhead flags of the object.
 */
void LwpDrawObj::SetArrowHead(XFDrawStyle* pOpenedObjStyle, sal_uInt8 nArrowFlag, sal_uInt8 nLineWidth)
{
    // no arrowhead
    if (!nArrowFlag)
    {
        return;
    }

    if (!pOpenedObjStyle)
    {
        assert(false);
        return;
    }

    // arrowhead flag of an object's start side
    sal_uInt8 nLeftArrow = nArrowFlag & 0x0F;

    // arrowhead flag of an object's end side
    sal_uInt8 nRightArrow = (nArrowFlag & 0xF0) >> 4;

    double fWidth_inch = static_cast<double>(nLineWidth)/TWIPS_PER_CM;
    double fArrowSize_inch = fWidth_inch + 0.08;
    double fArrowSize = fArrowSize_inch * CM_PER_INCH;

    if (nLeftArrow)
    {
        pOpenedObjStyle->SetArrowStart( GetArrowName(nLeftArrow), fArrowSize);
    }
    if (nRightArrow)
    {
        pOpenedObjStyle->SetArrowEnd( GetArrowName(nRightArrow), fArrowSize);
    }

}

/**
 * @descr   get arrow style name according to the flag.
 * @param   nArrowStyle   style of the arrowhead.
 * @return   nWhichSide   style name of the arrowhead.
 */
OUString LwpDrawObj::GetArrowName(sal_uInt8 nArrowStyle)
{
    // style name of arrowhead
    OUString aArrowName;

    switch(nArrowStyle)
    {
    default:
    case AH_ARROW_FULLARROW:
        aArrowName = "Symmetric arrow";
        break;
    case AH_ARROW_HALFARROW:
        aArrowName = "Arrow concave";
        break;
    case AH_ARROW_LINEARROW:
        aArrowName = "arrow100";
        break;
    case AH_ARROW_INVFULLARROW:
        aArrowName = "reverse arrow";
        break;
    case AH_ARROW_INVHALFARROW:
        aArrowName = "reverse concave arrow";
        break;
    case AH_ARROW_INVLINEARROW:
        aArrowName = "reverse line arrow";
        break;
    case AH_ARROW_TEE:
        aArrowName = "Dimension lines";
        break;
    case AH_ARROW_SQUARE:
        aArrowName = "Square";
        break;
    case AH_ARROW_CIRCLE:
        aArrowName = "Circle";
        break;
    }

    return aArrowName;
}

/**
 * @descr   template method of creating drawing object from Lwp-Model to XF-Model
 * @return   pointer of the drawing object of XF-Model.
 */
XFFrame* LwpDrawObj::CreateXFDrawObject()
{
    // read records
    Read();

    // register style
    OUString aStyleName = RegisterStyle();

    // create XF-Objects
    XFFrame* pXFObj = nullptr;
    if (m_pTransData
        && FABS(m_pTransData->fOffsetX - m_pTransData->fLeftMargin) < THRESHOLD
        && FABS(m_pTransData->fOffsetY - m_pTransData->fTopMargin) < THRESHOLD
        && FABS(m_pTransData->fScaleX - 1.0) < THRESHOLD
        && FABS(m_pTransData->fScaleY - 1.0) < THRESHOLD)
    {
        pXFObj = CreateStandardDrawObj(aStyleName);
    }
    else
    {
        pXFObj = CreateDrawObj(aStyleName);
    }

    // set anchor type
    if (pXFObj)
    {
        pXFObj->SetAnchorType(enumXFAnchorFrame);
    }

    return pXFObj;
}

/**
 * @descr   Constructor of class LwpDrawLine
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawLine::LwpDrawLine(SvStream * pStream, DrawingOffsetAndScale* pTransData)
: LwpDrawObj(pStream, pTransData)
{
}

/**
 * @descr   reading function of class LwpDrawLine
 */
void LwpDrawLine::Read()
{
    m_pStream->ReadInt16( m_aLineRec.nStartX );
    m_pStream->ReadInt16( m_aLineRec.nStartY );
    m_pStream->ReadInt16( m_aLineRec.nEndX );
    m_pStream->ReadInt16( m_aLineRec.nEndY );
    m_pStream->ReadUChar( m_aLineRec.nLineWidth );
    m_pStream->ReadUChar( m_aLineRec.nLineEnd );
    m_pStream->ReadUChar( m_aLineRec.nLineStyle );

    m_pStream->ReadUChar( m_aLineRec.aPenColor.nR );
    m_pStream->ReadUChar( m_aLineRec.aPenColor.nG );
    m_pStream->ReadUChar( m_aLineRec.aPenColor.nB );
    m_pStream->ReadUChar( m_aLineRec.aPenColor.unused );
}

OUString LwpDrawLine::RegisterStyle()
{
    std::unique_ptr<XFDrawStyle> pStyle( new XFDrawStyle() );

    // set line style
    SetLineStyle(pStyle.get(), m_aLineRec.nLineWidth, m_aLineRec.nLineStyle, m_aLineRec.aPenColor);

    // set arrow head
    SetArrowHead(pStyle.get(), m_aLineRec.nLineEnd, m_aLineRec.nLineWidth);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawLine::CreateDrawObj(const OUString& rStyleName )
{
    XFDrawPath* pLine = new XFDrawPath();
    pLine->MoveTo(XFPoint(static_cast<double>(m_aLineRec.nStartX)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aLineRec.nStartY)/TWIPS_PER_CM * m_pTransData->fScaleY));
    pLine->LineTo(XFPoint(static_cast<double>(m_aLineRec.nEndX)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aLineRec.nEndY)/TWIPS_PER_CM * m_pTransData->fScaleY));
    SetPosition(pLine);

    pLine->SetStyleName(rStyleName);

    return pLine;
}

XFFrame* LwpDrawLine::CreateStandardDrawObj(const  OUString& rStyleName)
{
    XFDrawLine* pLine = new XFDrawLine();
    pLine->SetStartPoint(static_cast<double>(m_aLineRec.nStartX)/TWIPS_PER_CM,static_cast<double>(m_aLineRec.nStartY)/TWIPS_PER_CM);
    pLine->SetEndPoint(static_cast<double>(m_aLineRec.nEndX)/TWIPS_PER_CM,static_cast<double>(m_aLineRec.nEndY)/TWIPS_PER_CM);

    pLine->SetStyleName(rStyleName);
    return pLine;
}

/**
 * @descr   Constructor of class LwpDrawPolyLine
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawPolyLine::LwpDrawPolyLine(SvStream * pStream, DrawingOffsetAndScale* pTransData)
: LwpDrawObj(pStream, pTransData)
{
}
LwpDrawPolyLine::~LwpDrawPolyLine()
{
}

/**
 * @descr   reading function of class LwpDrawPolyLine
 */
void LwpDrawPolyLine::Read()
{
    m_pStream->ReadUChar( m_aPolyLineRec.nLineWidth );
    m_pStream->ReadUChar( m_aPolyLineRec.nLineEnd );
    m_pStream->ReadUChar( m_aPolyLineRec.nLineStyle );
    m_pStream->ReadUChar( m_aPolyLineRec.aPenColor.nR );
    m_pStream->ReadUChar( m_aPolyLineRec.aPenColor.nG );
    m_pStream->ReadUChar( m_aPolyLineRec.aPenColor.nB );
    m_pStream->ReadUChar( m_aPolyLineRec.aPenColor.unused );
    m_pStream->ReadUInt16( m_aPolyLineRec.nNumPoints );

    if (m_aPolyLineRec.nNumPoints > m_pStream->remainingSize() / 4)
        throw BadRead();

    m_pVector.reset( new SdwPoint[m_aPolyLineRec.nNumPoints] );

    for (sal_uInt16 nC = 0; nC < m_aPolyLineRec.nNumPoints; nC++)
    {
        m_pStream->ReadInt16( m_pVector[nC].x );
        m_pStream->ReadInt16( m_pVector[nC].y );
    }
}

OUString LwpDrawPolyLine::RegisterStyle()
{
    std::unique_ptr<XFDrawStyle> pStyle(new XFDrawStyle());

    // set line style
    SetLineStyle(pStyle.get(), m_aPolyLineRec.nLineWidth, m_aPolyLineRec.nLineStyle,
        m_aPolyLineRec.aPenColor);

    // set arrow head
    SetArrowHead(pStyle.get(), m_aPolyLineRec.nLineEnd, m_aPolyLineRec.nLineWidth);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawPolyLine::CreateDrawObj(const OUString& rStyleName )
{
    XFDrawPath* pPolyline = new XFDrawPath();
    pPolyline->MoveTo(XFPoint(static_cast<double>(m_pVector[0].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
            static_cast<double>(m_pVector[0].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
    for (sal_uInt16 nC = 1; nC < m_aPolyLineRec.nNumPoints; nC++)
    {
        pPolyline->LineTo(XFPoint(static_cast<double>(m_pVector[nC].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
            static_cast<double>(m_pVector[nC].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
    }
    SetPosition(pPolyline);

    pPolyline->SetStyleName(rStyleName);

    return pPolyline;
}

XFFrame* LwpDrawPolyLine::CreateStandardDrawObj(const  OUString& rStyleName)
{
    XFDrawPolyline* pPolyline = new XFDrawPolyline();
    for (sal_uInt16 nC = 0; nC < m_aPolyLineRec.nNumPoints; nC++)
    {
        pPolyline->AddPoint(static_cast<double>(m_pVector[nC].x)/TWIPS_PER_CM,
            static_cast<double>(m_pVector[nC].y)/TWIPS_PER_CM);
    }

    pPolyline->SetStyleName(rStyleName);

    return pPolyline;
}

/**
 * @descr   Constructor of class LwpDrawPolygon
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawPolygon::LwpDrawPolygon(SvStream * pStream, DrawingOffsetAndScale* pTransData)
    : LwpDrawObj(pStream, pTransData)
    , m_nNumPoints(0)
{
}

LwpDrawPolygon::~LwpDrawPolygon()
{
}

/**
 * @descr   reading function of class LwpDrawPolygon
 */
void LwpDrawPolygon::Read()
{
    ReadClosedObjStyle();
    m_pStream->ReadUInt16( m_nNumPoints );

    if (m_nNumPoints > m_pStream->remainingSize() / 4)
        throw BadRead();

    m_pVector.reset( new SdwPoint[m_nNumPoints] );

    for (sal_uInt16 nC = 0; nC < m_nNumPoints; nC++)
    {
        m_pStream->ReadInt16( m_pVector[nC].x );
        m_pStream->ReadInt16( m_pVector[nC].y );
    }
}

OUString LwpDrawPolygon::RegisterStyle()
{
    std::unique_ptr<XFDrawStyle> pStyle(new XFDrawStyle());

    // set line style
    SetLineStyle(pStyle.get(), m_aClosedObjStyleRec.nLineWidth, m_aClosedObjStyleRec.nLineStyle,
        m_aClosedObjStyleRec.aPenColor);

    // set fill style
    SetFillStyle(pStyle.get());

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawPolygon::CreateDrawObj(const OUString& rStyleName)
{
    XFDrawPath* pPolygon = new XFDrawPath();
    pPolygon->MoveTo(XFPoint(static_cast<double>(m_pVector[0].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
            static_cast<double>(m_pVector[0].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
    for (sal_uInt16 nC = 1; nC < m_nNumPoints; nC++)
    {
        pPolygon->LineTo(XFPoint(static_cast<double>(m_pVector[nC].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
            static_cast<double>(m_pVector[nC].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
    }
    pPolygon->ClosePath();
    SetPosition(pPolygon);
    pPolygon->SetStyleName(rStyleName);

    return pPolygon;
}

XFFrame* LwpDrawPolygon::CreateStandardDrawObj(const  OUString& rStyleName)
{
    XFDrawPolygon* pPolygon = new XFDrawPolygon();
    for (sal_uInt16 nC = 0; nC < m_nNumPoints; nC++)
    {
        pPolygon->AddPoint(static_cast<double>(m_pVector[nC].x)/TWIPS_PER_CM,
            static_cast<double>(m_pVector[nC].y)/TWIPS_PER_CM);
    }

    pPolygon->SetStyleName(rStyleName);

    return pPolygon;
}

/**
 * @descr   Constructor of class LwpDrawRectangle
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawRectangle::LwpDrawRectangle(SvStream * pStream, DrawingOffsetAndScale* pTransData)
    : LwpDrawObj(pStream, pTransData)
{
}

/**
 * @descr   reading function of class LwpDrawRectangle
 */
void LwpDrawRectangle::Read()
{
    ReadClosedObjStyle();

    sal_uInt8 nPointsCount;
    if (m_eType == OT_RNDRECT)
    {
        nPointsCount = 16;
        m_pStream->SeekRel(4); // corner points
    }
    else
    {
        nPointsCount = 4;
    }

    for (sal_uInt8 nC = 0; nC < nPointsCount; nC++)
    {
        m_pStream->ReadInt16( m_aVector[nC].x );
        m_pStream->ReadInt16( m_aVector[nC].y );
    }
}

OUString LwpDrawRectangle::RegisterStyle()
{
    std::unique_ptr<XFDrawStyle> pStyle(new XFDrawStyle());

    // set line style
    SetLineStyle(pStyle.get(), m_aClosedObjStyleRec.nLineWidth, m_aClosedObjStyleRec.nLineStyle,
        m_aClosedObjStyleRec.aPenColor);

    // set fill style
    SetFillStyle(pStyle.get());

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawRectangle::CreateDrawObj(const OUString& rStyleName)
{
    if (m_eType == OT_RNDRECT)
    {
        return CreateRoundedRect(rStyleName);
    }
    else
    {
        XFDrawPath* pRect = new XFDrawPath();
        pRect->MoveTo(XFPoint(static_cast<double>(m_aVector[0].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
            static_cast<double>(m_aVector[0].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
        for (sal_uInt8 nC = 1; nC < 4; nC++)
        {
            pRect->LineTo(XFPoint(static_cast<double>(m_aVector[nC].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
                static_cast<double>(m_aVector[nC].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
        }
        pRect->LineTo(XFPoint(static_cast<double>(m_aVector[0].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
                static_cast<double>(m_aVector[0].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
        pRect->ClosePath();
        SetPosition(pRect);

        pRect->SetStyleName(rStyleName);

        return pRect;
    }
}

XFFrame* LwpDrawRectangle::CreateRoundedRect(const OUString& rStyleName)
{
    XFDrawPath* pRoundedRect = new XFDrawPath();
    pRoundedRect->MoveTo(XFPoint(static_cast<double>(m_aVector[0].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
    static_cast<double>(m_aVector[0].y)/TWIPS_PER_CM * m_pTransData->fScaleY));

    sal_uInt8 nPtIndex = 1;
    for (sal_uInt8 nC = 0; nC < 7; nC++)
    {
        if (nC%2 == 0)
        {
            XFPoint aCtrl1(static_cast<double>(m_aVector[nPtIndex].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
                static_cast<double>(m_aVector[nPtIndex].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
            nPtIndex++;
            XFPoint aCtrl2(static_cast<double>(m_aVector[nPtIndex].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
                static_cast<double>(m_aVector[nPtIndex].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
            nPtIndex++;
            XFPoint aDest(static_cast<double>(m_aVector[nPtIndex].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
                static_cast<double>(m_aVector[nPtIndex].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
            nPtIndex++;

            pRoundedRect->CurveTo(aDest, aCtrl1, aCtrl2);
        }
        else
        {
            XFPoint aDest(static_cast<double>(m_aVector[nPtIndex].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
                static_cast<double>(m_aVector[nPtIndex].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
            nPtIndex++;

            pRoundedRect->LineTo(aDest);
        }
    }

    pRoundedRect->LineTo(XFPoint(static_cast<double>(m_aVector[0].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
                static_cast<double>(m_aVector[0].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
    pRoundedRect->ClosePath();
    SetPosition(pRoundedRect);

    pRoundedRect->SetStyleName(rStyleName);

    return pRoundedRect;
}

XFFrame* LwpDrawRectangle::CreateStandardDrawObj(const  OUString& rStyleName)
{
    if (m_eType == OT_RNDRECT)
    {
        return CreateRoundedRect(rStyleName);
    }
    else
    {
        XFDrawRect* pRect = new XFDrawRect();
        double fStartX, fStartY, fWidth, fHeight;
        double fRotAngle = 0.0;
        SdwRectangle aSdwRect;
        tools::Rectangle aOriginalRect;
        Point aPt0, aPt1, aPt2, aPt3;

        aPt0.setX(m_aVector[0].x); aPt0.setY(m_aVector[0].y);
        aPt1.setX(m_aVector[1].x); aPt1.setY(m_aVector[1].y);
        aPt2.setX(m_aVector[2].x); aPt2.setY(m_aVector[2].y);
        aPt3.setX(m_aVector[3].x); aPt3.setY(m_aVector[3].y);

        aSdwRect = SdwRectangle(aPt0, aPt1, aPt2, aPt3);
        if (aSdwRect.IsRectRotated())
        {
            aOriginalRect = aSdwRect.GetOriginalRect();
            fRotAngle = aSdwRect.GetRotationAngle();
        }
        else
        {
            aOriginalRect = tools::Rectangle(aPt0, aPt2);
        }

        fStartX = aOriginalRect.TopLeft().X();
        fStartY = aOriginalRect.TopLeft().Y();
        fWidth = aOriginalRect.GetWidth();
        fHeight = aOriginalRect.GetHeight();

        pRect->SetStartPoint(XFPoint(fStartX/TWIPS_PER_CM + m_pTransData->fOffsetX,
            fStartY/TWIPS_PER_CM + m_pTransData->fOffsetY));
        pRect->SetSize(fWidth/TWIPS_PER_CM, fHeight/TWIPS_PER_CM);

        if (aSdwRect.IsRectRotated())
        {
            pRect->SetRotate( fRotAngle / PI * 180);// aXFCenter);
        }

        pRect->SetStyleName(rStyleName);

        return pRect;
    }
}

/**
 * @descr   Constructor of class LwpDrawEllipse
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawEllipse::LwpDrawEllipse(SvStream * pStream, DrawingOffsetAndScale* pTransData)
: LwpDrawObj(pStream, pTransData)
{
}

/**
 * @descr   reading function of class LwpDrawEllipse
 */
void LwpDrawEllipse::Read()
{
    ReadClosedObjStyle();

    for (SdwPoint & nC : m_aVector)
    {
        m_pStream->ReadInt16( nC.x );
        m_pStream->ReadInt16( nC.y );
    }
}

OUString LwpDrawEllipse::RegisterStyle()
{
    std::unique_ptr<XFDrawStyle> pStyle(new XFDrawStyle());

    // set line style
    SetLineStyle(pStyle.get(), m_aClosedObjStyleRec.nLineWidth, m_aClosedObjStyleRec.nLineStyle,
        m_aClosedObjStyleRec.aPenColor);

    // set fill style
    SetFillStyle(pStyle.get());

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawEllipse::CreateDrawObj(const OUString& rStyleName )
{
    XFDrawPath* pEllipse = new XFDrawPath();
    pEllipse->MoveTo(XFPoint(static_cast<double>(m_aVector[0].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aVector[0].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
    sal_uInt8 nPtIndex = 1;
    for (sal_uInt8 nC = 0; nC < 4; nC++)
    {
        XFPoint aCtrl1(static_cast<double>(m_aVector[nPtIndex].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aVector[nPtIndex].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
        nPtIndex++;
        XFPoint aCtrl2(static_cast<double>(m_aVector[nPtIndex].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aVector[nPtIndex].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
        nPtIndex++;
        XFPoint aDest(static_cast<double>(m_aVector[nPtIndex].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aVector[nPtIndex].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
        nPtIndex++;

        pEllipse->CurveTo(aDest, aCtrl1, aCtrl2);
    }
    pEllipse->ClosePath();
    SetPosition(pEllipse);

    pEllipse->SetStyleName(rStyleName);

    return pEllipse;
}

XFFrame* LwpDrawEllipse::CreateStandardDrawObj(const  OUString& rStyleName)
{
    return CreateDrawObj(rStyleName);
}

/**
 * @descr   Constructor of class LwpDrawArc
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawArc::LwpDrawArc(SvStream * pStream, DrawingOffsetAndScale* pTransData)
: LwpDrawObj(pStream, pTransData)
{
}

/**
 * @descr   reading function of class LwpDrawArc
 */
void LwpDrawArc::Read()
{
    m_pStream->SeekRel(16);// arcRect, startPt, endPt

    m_pStream->ReadUChar( m_aArcRec.nLineWidth );
    m_pStream->ReadUChar( m_aArcRec.nLineStyle );
    m_pStream->ReadUChar( m_aArcRec.aPenColor.nR );
    m_pStream->ReadUChar( m_aArcRec.aPenColor.nG );
    m_pStream->ReadUChar( m_aArcRec.aPenColor.nB );
    m_pStream->ReadUChar( m_aArcRec.aPenColor.unused );
    m_pStream->ReadUChar( m_aArcRec.nLineEnd );

    for (SdwPoint & nC : m_aVector)
    {
        m_pStream->ReadInt16( nC.x );
        m_pStream->ReadInt16( nC.y );
    }
}

OUString LwpDrawArc::RegisterStyle()
{
    std::unique_ptr<XFDrawStyle> pStyle(new XFDrawStyle());

    // set line style
    SetLineStyle(pStyle.get(), m_aArcRec.nLineWidth, m_aArcRec.nLineStyle,
        m_aArcRec.aPenColor);

    // set arrow head
    SetArrowHead(pStyle.get(), m_aArcRec.nLineEnd, m_aArcRec.nLineWidth);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawArc::CreateDrawObj(const OUString& rStyleName )
{
    XFDrawPath* pArc = new XFDrawPath();
    pArc->MoveTo(XFPoint(static_cast<double>(m_aVector[0].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aVector[0].y)/TWIPS_PER_CM * m_pTransData->fScaleY));
    XFPoint aDest(static_cast<double>(m_aVector[3].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aVector[3].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
    XFPoint aCtl1(static_cast<double>(m_aVector[1].x)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(m_aVector[1].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
    XFPoint aCtl2(static_cast<double>(m_aVector[2].x)/TWIPS_PER_CM* m_pTransData->fScaleX,
        static_cast<double>(m_aVector[2].y)/TWIPS_PER_CM * m_pTransData->fScaleY);
    pArc->CurveTo(aDest, aCtl1, aCtl2);

    SetPosition(pArc);

    pArc->SetStyleName(rStyleName);

    return pArc;
}

XFFrame* LwpDrawArc::CreateStandardDrawObj(const  OUString& rStyleName)
{
    return CreateDrawObj(rStyleName);
}

/**
 * @descr   Constructor of class LwpDrawTextBox
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawTextBox::LwpDrawTextBox(SvStream* pStream) : LwpDrawObj(pStream)
{
}

LwpDrawTextBox::~LwpDrawTextBox()
{
    if (m_aTextRec.pTextString)
    {
        delete [] m_aTextRec.pTextString;
        m_aTextRec.pTextString = nullptr;
    }
}

void LwpDrawTextBox::SetFontStyle(rtl::Reference<XFFont> const & pFont, SdwTextBoxRecord const * pRec)
{
    // color
    XFColor aXFColor(pRec->aTextColor.nR, pRec->aTextColor.nG,
            pRec->aTextColor.nB);
    pFont->SetColor(aXFColor);
    //size
    pFont->SetFontSize(pRec->nTextSize/20);
    // bold
    pFont->SetBold((pRec->nTextAttrs & TA_BOLD) != 0);
    // italic
    pFont->SetItalic((pRec->nTextAttrs & TA_ITALIC) != 0);
    // strike-through
    if (pRec->nTextAttrs & TA_STRIKETHRU)
    {
        pFont->SetCrossout(enumXFCrossoutSignel);
    }
    else
    {
        pFont->SetCrossout(enumXFCrossoutNone);
    }
    // underline
    if (pRec->nTextAttrs & TA_UNDERLINE)
    {
        pFont->SetUnderline(enumXFUnderlineSingle);
    }
    else if (pRec->nTextAttrs & TA_WORDUNDERLINE)
    {
        pFont->SetUnderline(enumXFUnderlineSingle, true);
    }
    else if (pRec->nTextAttrs & TA_DOUBLEUNDER)
    {
        pFont->SetUnderline(enumXFUnderlineDouble);
    }
    else
    {
        pFont->SetUnderline(enumXFUnderlineNone);
    }
    // small-caps
    if (pRec->nTextAttrs & TA_SMALLCAPS)
    {
        pFont->SetTransform(enumXFTransformSmallCaps);
    }
}

/**
 * @descr   reading function of class LwpDrawTextBox
 */
void LwpDrawTextBox::Read()
{
    m_pStream->ReadInt16( m_aVector.x );
    m_pStream->ReadInt16( m_aVector.y );
    m_pStream->ReadInt16( m_aTextRec.nTextWidth );

    if (m_aTextRec.nTextWidth == 0)
        m_aTextRec.nTextWidth = 1;

    m_pStream->ReadInt16( m_aTextRec.nTextHeight );
    m_pStream->ReadBytes(m_aTextRec.tmpTextFaceName, DRAW_FACESIZE);
    m_pStream->SeekRel(1);// PitchAndFamily

    m_pStream->ReadInt16( m_aTextRec.nTextSize );

    if (m_aTextRec.nTextSize < 0)
        m_aTextRec.nTextSize = -m_aTextRec.nTextSize;

    //color
    m_pStream->ReadUChar( m_aTextRec.aTextColor.nR );
    m_pStream->ReadUChar( m_aTextRec.aTextColor.nG );
    m_pStream->ReadUChar( m_aTextRec.aTextColor.nB );
    m_pStream->ReadUChar( m_aTextRec.aTextColor.unused );

    m_pStream->ReadUInt16( m_aTextRec.nTextAttrs );
    m_pStream->ReadUInt16( m_aTextRec.nTextCharacterSet );
    m_pStream->ReadInt16( m_aTextRec.nTextRotation );
    m_pStream->ReadInt16( m_aTextRec.nTextExtraSpacing );

    // some draw files in version 1.2 have an extra byte following '\0'.
    // can't rely on that, so read in the whole string into memory.

    // the 71 is the fixed length before text content in textbox record
    sal_Int16 TextLength = m_aObjHeader.nRecLen - 71;
    m_aTextRec.pTextString = new sal_uInt8 [TextLength];

    m_pStream->ReadBytes(m_aTextRec.pTextString, TextLength);
}

OUString LwpDrawTextBox::RegisterStyle()
{
    std::unique_ptr<XFParaStyle> pStyle(new XFParaStyle());

    // font style
    // the pFont need to be deleted myself?
    rtl::Reference<XFFont> pFont = new XFFont();

    OUString aFontName(reinterpret_cast<char*>(m_aTextRec.tmpTextFaceName),
        strlen(reinterpret_cast<char*>(m_aTextRec.tmpTextFaceName)), RTL_TEXTENCODING_MS_1252);
    pFont->SetFontName(aFontName);

    SetFontStyle(pFont, &m_aTextRec);

    pStyle->SetFont(pFont);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawTextBox::CreateDrawObj(const OUString& rStyleName )
{
    XFFrame* pTextBox = new XFFrame(true);

    sal_Int16 TextLength = m_aObjHeader.nRecLen - 71;
    rtl_TextEncoding aEncoding;
    if (!m_aTextRec.nTextCharacterSet)
    {
        aEncoding = osl_getThreadTextEncoding();
    }
    else
    {
        // temporary code, need to create Encoding from the value of nTextCharacterSet
        aEncoding = LwpCharSetMgr::GetTextCharEncoding();
    }

    XFParagraph* pXFPara = new XFParagraph();
    pXFPara->Add(OUString(reinterpret_cast<char*>(m_aTextRec.pTextString), (TextLength-2), aEncoding));
    pXFPara->SetStyleName(rStyleName);

    pTextBox->Add(pXFPara);
    SetPosition(pTextBox);

    std::unique_ptr<XFTextBoxStyle> pBoxStyle(new XFTextBoxStyle());

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    OUString sName = pXFStyleManager->AddStyle(std::move(pBoxStyle)).m_pStyle->GetStyleName();
    pTextBox->SetStyleName(sName);

    //todo: add the interface for rotating textbox
//  if (m_aTextRec.nTextRotation)
//  {
//      double fAngle = double(3600-m_aTextRec.nTextRotation)/10;
//      pTextBox->SetRotate(fAngle);
//  }

    return pTextBox;
}

XFFrame* LwpDrawTextBox::CreateStandardDrawObj(const  OUString& rStyleName)
{
    return CreateDrawObj(rStyleName);
}

/**
 * @descr   Constructor of class LwpDrawTextBox
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawTextArt::LwpDrawTextArt(SvStream* pStream, DrawingOffsetAndScale* pTransData)
: LwpDrawObj(pStream, pTransData)
{
}

LwpDrawTextArt::~LwpDrawTextArt()
{
    if (m_aTextArtRec.aPath[0].pPts)
    {
        delete [] m_aTextArtRec.aPath[0].pPts;
        m_aTextArtRec.aPath[0].pPts = nullptr;
    }
    if (m_aTextArtRec.aPath[1].pPts)
    {
        delete [] m_aTextArtRec.aPath[1].pPts;
        m_aTextArtRec.aPath[1].pPts = nullptr;
    }
    if (m_aTextArtRec.pTextString)
    {
        delete [] m_aTextArtRec.pTextString;
        m_aTextArtRec.pTextString = nullptr;
    }

}

void LwpDrawTextArt::CreateFWPath(XFDrawPath* pPath)
{
    sal_Int16 nX, nY;
    nX = (m_aTextArtRec.aPath[0].pPts[0].x + m_aTextArtRec.aPath[1].pPts[0].x) / 2;
    nY = (m_aTextArtRec.aPath[0].pPts[0].y + m_aTextArtRec.aPath[1].pPts[0].y) / 2;
    XFPoint aStart(static_cast<double>(nX)/TWIPS_PER_CM * m_pTransData->fScaleX,
        static_cast<double>(nY)/TWIPS_PER_CM * m_pTransData->fScaleY);
    pPath->MoveTo(aStart);

    sal_uInt8 nPtIndex = 1;
    for (sal_uInt16 nC = 1; nC <= m_aTextArtRec.aPath[0].n; nC++)
    {
        nX = (m_aTextArtRec.aPath[0].pPts[nPtIndex].x + m_aTextArtRec.aPath[1].pPts[nPtIndex].x) / 2;
        nY = (m_aTextArtRec.aPath[0].pPts[nPtIndex].y + m_aTextArtRec.aPath[1].pPts[nPtIndex].y) / 2;
        XFPoint aCtrl1(static_cast<double>(nX)/TWIPS_PER_CM * m_pTransData->fScaleX,
            static_cast<double>(nY)/TWIPS_PER_CM * m_pTransData->fScaleY);

        nPtIndex++;
        nX = (m_aTextArtRec.aPath[0].pPts[nPtIndex].x + m_aTextArtRec.aPath[1].pPts[nPtIndex].x) / 2;
        nY = (m_aTextArtRec.aPath[0].pPts[nPtIndex].y + m_aTextArtRec.aPath[1].pPts[nPtIndex].y) / 2;
        XFPoint aCtrl2(static_cast<double>(nX)/TWIPS_PER_CM * m_pTransData->fScaleX,
            static_cast<double>(nY)/TWIPS_PER_CM * m_pTransData->fScaleY);

        nPtIndex++;
        nX = (m_aTextArtRec.aPath[0].pPts[nPtIndex].x + m_aTextArtRec.aPath[1].pPts[nPtIndex].x) / 2;
        nY = (m_aTextArtRec.aPath[0].pPts[nPtIndex].y + m_aTextArtRec.aPath[1].pPts[nPtIndex].y) / 2;
        XFPoint aDest(static_cast<double>(nX)/TWIPS_PER_CM * m_pTransData->fScaleX,
            static_cast<double>(nY)/TWIPS_PER_CM * m_pTransData->fScaleY);

        pPath->CurveTo(aDest, aCtrl1, aCtrl2);
    }
}

void LwpDrawTextArt::Read()
{
    for (SdwPoint & nC : m_aVector)
    {
        m_pStream->ReadInt16( nC.x );
        m_pStream->ReadInt16( nC.y );
    }

    ReadClosedObjStyle();
    m_aTextArtRec.aTextColor = m_aClosedObjStyleRec.aForeColor;

    m_pStream->ReadUChar( m_aTextArtRec.nIndex );
    m_pStream->ReadInt16( m_aTextArtRec.nRotation );

    sal_uInt16 nPointNumber;
    sal_Int16 nX, nY;
    m_pStream->ReadUInt16( nPointNumber );

    size_t nPoints = nPointNumber*3+1;
    if (nPoints > m_pStream->remainingSize() / 4)
        throw BadRead();

    m_aTextArtRec.aPath[0].n = nPointNumber;
    m_aTextArtRec.aPath[0].pPts = new SdwPoint[nPoints];
    for (size_t nPt = 0; nPt < nPoints; ++nPt)
    {
        m_pStream->ReadInt16( nX );
        m_pStream->ReadInt16( nY );
        m_aTextArtRec.aPath[0].pPts[nPt].x = nX;
        m_aTextArtRec.aPath[0].pPts[nPt].y = nY;
    }

    m_pStream->ReadUInt16( nPointNumber );

    nPoints = nPointNumber*3+1;
    if (nPoints > m_pStream->remainingSize() / 4)
        throw BadRead();

    m_aTextArtRec.aPath[1].n = nPointNumber;
    m_aTextArtRec.aPath[1].pPts = new SdwPoint[nPoints];
    for (size_t nPt = 0; nPt < nPoints; ++nPt)
    {
        m_pStream->ReadInt16( nX );
        m_pStream->ReadInt16( nY );
        m_aTextArtRec.aPath[1].pPts[nPt].x = nX;
        m_aTextArtRec.aPath[1].pPts[nPt].y = nY;
    }

    m_pStream->SeekRel(1);

    m_pStream->ReadBytes(m_aTextArtRec.tmpTextFaceName, DRAW_FACESIZE);
    m_pStream->SeekRel(1);// PitchAndFamily

    m_pStream->ReadInt16( m_aTextArtRec.nTextSize );

    if (m_aTextArtRec.nTextSize < 0)
        m_aTextArtRec.nTextSize = -m_aTextArtRec.nTextSize;

    m_pStream->ReadUInt16( m_aTextArtRec.nTextAttrs );
    m_pStream->ReadUInt16( m_aTextArtRec.nTextCharacterSet );
    m_aTextArtRec.nTextRotation = 0;
    m_pStream->ReadInt16( m_aTextArtRec.nTextExtraSpacing );

    const sal_uInt16 nTextArtFixedLength = 105;
    m_aTextArtRec.nTextLen = m_aObjHeader.nRecLen - nTextArtFixedLength
                                                    - (m_aTextArtRec.aPath[0].n*3 + 1)*4
                                                    - (m_aTextArtRec.aPath[1].n*3 + 1)*4;


    if (m_aTextArtRec.nTextLen > m_pStream->remainingSize())
        throw BadRead();

    m_aTextArtRec.pTextString = new sal_uInt8 [m_aTextArtRec.nTextLen];
    m_pStream->ReadBytes(m_aTextArtRec.pTextString, m_aTextArtRec.nTextLen);
    m_aTextArtRec.pTextString[m_aTextArtRec.nTextLen-1] = 0;

}

OUString LwpDrawTextArt::RegisterStyle()
{
    std::unique_ptr<XFParaStyle> pStyle(new XFParaStyle());

    // font style
    // the pFont need to be deleted myself?
    rtl::Reference<XFFont> pFont = new XFFont();

    OUString aFontName(reinterpret_cast<char*>(m_aTextArtRec.tmpTextFaceName),
        strlen(reinterpret_cast<char*>(m_aTextArtRec.tmpTextFaceName)), RTL_TEXTENCODING_MS_1252);
    pFont->SetFontName(aFontName);

    LwpDrawTextBox::SetFontStyle(pFont, &m_aTextArtRec);

    pStyle->SetFont(pFont);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawTextArt::CreateDrawObj(const OUString& rStyleName)
{
    XFFrame* pRetObj = nullptr;
    std::unique_ptr<XFDrawStyle> pStyle(new XFDrawStyle());

    pRetObj = new XFDrawPath();
    XFDrawPath* pFWPath = static_cast<XFDrawPath*>(pRetObj);
    CreateFWPath(pFWPath);
    pStyle->SetFontWorkStyle(enumXFFWSlantY, enumXFFWAdjustAutosize);

    SetPosition(pRetObj);

    rtl_TextEncoding aEncoding;
    if (!m_aTextArtRec.nTextCharacterSet)
    {
        aEncoding = osl_getThreadTextEncoding();
    }
    else
    {
        // temporary code, need to create Encoding from the value of nTextCharacterSet
        aEncoding = LwpCharSetMgr::GetTextCharEncoding();
    }

    XFParagraph* pXFPara = new XFParagraph();
    pXFPara->Add(OUString(reinterpret_cast<char*>(m_aTextArtRec.pTextString), (m_aTextArtRec.nTextLen-1), aEncoding));
    pXFPara->SetStyleName(rStyleName);
    pRetObj->Add(pXFPara);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pRetObj->SetStyleName(pXFStyleManager->AddStyle(std::move(pStyle)).m_pStyle->GetStyleName());

    return pRetObj;
}

XFFrame* LwpDrawTextArt::CreateStandardDrawObj(const OUString& rStyleName )
{
    return CreateDrawObj(rStyleName);
}

/**
 * @descr   Constructor of class LwpDrawMetafile
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawMetafile::LwpDrawMetafile(SvStream* pStream) : LwpDrawObj(pStream)
{
}

/**
 * @descr   reading function of class LwpDrawMetafile
 */
void LwpDrawMetafile::Read()
{
    m_pStream->SeekRel(m_aObjHeader.nRecLen -16);
}

/**
 * @descr   Constructor of class LwpDrawBitmap
 * @param   pStream   The memory stream which contains the lwp-sdw draw objects
 */
LwpDrawBitmap::LwpDrawBitmap(SvStream* pStream) : LwpDrawObj(pStream)
{
}

LwpDrawBitmap::~LwpDrawBitmap()
{
}

/**
 * @descr   reading function of class LwpDrawBitmap
 */
void LwpDrawBitmap::Read()
{
    m_pStream->ReadUInt16( m_aBmpRec.nTranslation );
    m_pStream->ReadUInt16( m_aBmpRec.nRotation );

    // 20 == length of draw-specific fields.
    // 14 == length of bmp file header.
    m_aBmpRec.nFileSize = m_aObjHeader.nRecLen - 20 + 14;
    m_pImageData.reset( new sal_uInt8 [m_aBmpRec.nFileSize] );

    BmpInfoHeader2 aInfoHeader2;
    m_pStream->ReadUInt32( aInfoHeader2.nHeaderLen );

    sal_uInt32 N;
    sal_uInt32 rgbTableSize;

    if (aInfoHeader2.nHeaderLen == sizeof(BmpInfoHeader))
    {
        m_pStream->ReadUInt32( aInfoHeader2.nWidth );
        m_pStream->ReadUInt32( aInfoHeader2.nHeight );
        m_pStream->ReadUInt16( aInfoHeader2.nPlanes );
        m_pStream->ReadUInt16( aInfoHeader2.nBitCount );

        N = aInfoHeader2.nPlanes * aInfoHeader2.nBitCount;
        if (N == 24)
        {
            rgbTableSize = 0;
        }
        else
        {
            rgbTableSize = 3 * (1 << N);
        }
    }
    else
    {
        m_pStream->ReadUInt32( aInfoHeader2.nWidth );
        m_pStream->ReadUInt32( aInfoHeader2.nHeight );
        m_pStream->ReadUInt16( aInfoHeader2.nPlanes );
        m_pStream->ReadUInt16( aInfoHeader2.nBitCount );
        N = aInfoHeader2.nPlanes * aInfoHeader2.nBitCount;
        if (N == 24)
        {
            rgbTableSize = 0;
        }
        else
        {
            rgbTableSize = 4 * (1 << N);
        }

    }

    sal_uInt32 nOffBits = 14 + aInfoHeader2.nHeaderLen + rgbTableSize;
    m_pImageData[0] = 'B';
    m_pImageData[1] = 'M';
    m_pImageData[2] = static_cast<sal_uInt8>(m_aBmpRec.nFileSize);
    m_pImageData[3] = static_cast<sal_uInt8>(m_aBmpRec.nFileSize >> 8);
    m_pImageData[4] = static_cast<sal_uInt8>(m_aBmpRec.nFileSize >> 16);
    m_pImageData[5] = static_cast<sal_uInt8>(m_aBmpRec.nFileSize >> 24);
    m_pImageData[6] = 0;
    m_pImageData[7] = 0;
    m_pImageData[8] = 0;
    m_pImageData[9] = 0;
    m_pImageData[10] = static_cast<sal_uInt8>(nOffBits);
    m_pImageData[11] = static_cast<sal_uInt8>(nOffBits >> 8);
    m_pImageData[12] = static_cast<sal_uInt8>(nOffBits >> 16);
    m_pImageData[13] = static_cast<sal_uInt8>(nOffBits >> 24);

    sal_uInt32 nDIBRemaining;
    sal_uInt8* pPicData = m_pImageData.get();
    if (aInfoHeader2.nHeaderLen== sizeof(BmpInfoHeader))
    {
        m_pImageData[14] = static_cast<sal_uInt8>(aInfoHeader2.nHeaderLen);
        m_pImageData[15] = static_cast<sal_uInt8>(aInfoHeader2.nHeaderLen >> 8);
        m_pImageData[16] = static_cast<sal_uInt8>(aInfoHeader2.nHeaderLen >> 16);
        m_pImageData[17] = static_cast<sal_uInt8>(aInfoHeader2.nHeaderLen >> 24);
        m_pImageData[18] = static_cast<sal_uInt8>(aInfoHeader2.nWidth);
        m_pImageData[19] = static_cast<sal_uInt8>(aInfoHeader2.nWidth >> 8);
        m_pImageData[20] = static_cast<sal_uInt8>(aInfoHeader2.nHeight);
        m_pImageData[21] = static_cast<sal_uInt8>(aInfoHeader2.nHeight >> 8);
        m_pImageData[22] = static_cast<sal_uInt8>(aInfoHeader2.nPlanes);
        m_pImageData[23] = static_cast<sal_uInt8>(aInfoHeader2.nPlanes >> 8);
        m_pImageData[24] = static_cast<sal_uInt8>(aInfoHeader2.nBitCount);
        m_pImageData[25] = static_cast<sal_uInt8>(aInfoHeader2.nBitCount >> 8);

        nDIBRemaining = m_aBmpRec.nFileSize - 26;
        pPicData += 26*sizeof(sal_uInt8);
    }
    else
    {
        m_pImageData[14] = static_cast<sal_uInt8>(aInfoHeader2.nHeaderLen);
        m_pImageData[15] = static_cast<sal_uInt8>(aInfoHeader2.nHeaderLen >> 8);
        m_pImageData[16] = static_cast<sal_uInt8>(aInfoHeader2.nHeaderLen >> 16);
        m_pImageData[17] = static_cast<sal_uInt8>(aInfoHeader2.nHeaderLen >> 24);
        m_pImageData[18] = static_cast<sal_uInt8>(aInfoHeader2.nWidth);
        m_pImageData[19] = static_cast<sal_uInt8>(aInfoHeader2.nWidth >> 8);
        m_pImageData[20] = static_cast<sal_uInt8>(aInfoHeader2.nWidth >> 16);
        m_pImageData[21] = static_cast<sal_uInt8>(aInfoHeader2.nWidth >> 24);
        m_pImageData[22] = static_cast<sal_uInt8>(aInfoHeader2.nHeight);
        m_pImageData[23] = static_cast<sal_uInt8>(aInfoHeader2.nHeight >> 8);
        m_pImageData[24] = static_cast<sal_uInt8>(aInfoHeader2.nHeight >> 16);
        m_pImageData[25] = static_cast<sal_uInt8>(aInfoHeader2.nHeight >> 24);
        m_pImageData[26] = static_cast<sal_uInt8>(aInfoHeader2.nPlanes);
        m_pImageData[27] = static_cast<sal_uInt8>(aInfoHeader2.nPlanes >> 8);
        m_pImageData[28] = static_cast<sal_uInt8>(aInfoHeader2.nBitCount);
        m_pImageData[29] = static_cast<sal_uInt8>(aInfoHeader2.nBitCount >> 8);

        nDIBRemaining = m_aBmpRec.nFileSize - 30;
        pPicData += 30*sizeof(sal_uInt8);
    }

    m_pStream->ReadBytes(pPicData, nDIBRemaining);
}

OUString LwpDrawBitmap::RegisterStyle()
{
    std::unique_ptr<XFImageStyle> pBmpStyle(new XFImageStyle());
    pBmpStyle->SetYPosType(enumXFFrameYPosFromTop, enumXFFrameYRelFrame);
    pBmpStyle->SetXPosType(enumXFFrameXPosFromLeft, enumXFFrameXRelFrame);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->AddStyle(std::move(pBmpStyle)).m_pStyle->GetStyleName();
}

XFFrame* LwpDrawBitmap::CreateDrawObj(const OUString& rStyleName)
{
    XFImage* pImage = new XFImage();
    pImage->SetImageData(m_pImageData.get(), m_aBmpRec.nFileSize);
    SetPosition(pImage);

    pImage->SetStyleName(rStyleName);

    return pImage;
}

XFFrame* LwpDrawBitmap::CreateStandardDrawObj(const  OUString& rStyleName)
{
    return CreateDrawObj(rStyleName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
