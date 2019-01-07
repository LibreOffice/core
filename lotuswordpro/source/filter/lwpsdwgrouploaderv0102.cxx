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

#include <tools/stream.hxx>

#include "lwpsdwgrouploaderv0102.hxx"
#include "lwpdrawobj.hxx"
#include <lwptools.hxx>
#include "lwpcharsetmgr.hxx"
#include "lwpgrfobj.hxx"
#include "lwpframelayout.hxx"

#include <xfilter/xfcolor.hxx>
#include <xfilter/xfdrawline.hxx>
#include <xfilter/xfdrawpath.hxx>
#include <xfilter/xfdrawpolyline.hxx>
#include <xfilter/xfdrawrect.hxx>
#include <xfilter/xfdrawpolygon.hxx>
#include <xfilter/xfdrawgroup.hxx>
#include <xfilter/xfdrawstyle.hxx>
#include <xfilter/xfdrawlinestyle.hxx>
#include <xfilter/xfparagraph.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <sal/log.hxx>

LwpSdwGroupLoaderV0102::LwpSdwGroupLoaderV0102(SvStream* pStream, LwpGraphicObject* pGraphicObj)
    : m_pStream(pStream)
    , m_pGraphicObj(pGraphicObj)
    , m_pDrawObjVector(nullptr)
{
}

LwpSdwGroupLoaderV0102::~LwpSdwGroupLoaderV0102()
{
}

/**
 * @descr   entry of lwp-drawing objects. the function begins to parse the sdw-drawing bento stream and create
 *      the corresponding drawing objects.
 * @param   pDrawObjVector   a container which will contains the created drawing object of XF-Model.
 */
void LwpSdwGroupLoaderV0102::BeginDrawObjects(std::vector< rtl::Reference<XFFrame> >* pDrawObjVector)
{
    // save the container
    m_pDrawObjVector = pDrawObjVector;

    //flag
    unsigned char BinSignature[2];
    m_pStream->ReadBytes(BinSignature, 2);
    if (BinSignature[0] != 'S' || BinSignature[1] != 'M')
    {
        assert(false);
        return;
    }
    //version
    unsigned short nVersion;
    m_pStream->ReadUInt16(nVersion);
    if (nVersion<0x0102)
    {
        assert(false);
        return;
    }
    // topObj, botObj
    m_pStream->SeekRel(4);
    //record count
    unsigned short nRecCount(0);
    m_pStream->ReadUInt16(nRecCount);
    // selCount
    m_pStream->SeekRel(2);
    //boundrect
    unsigned short left(0),top(0),right(0),bottom(0);
    m_pStream->ReadUInt16(left);
    m_pStream->ReadUInt16(top);
    m_pStream->ReadUInt16(right);
    m_pStream->ReadUInt16(bottom);
    // fileSize
    m_pStream->SeekRel(2);

    //for calculating transformation params.
    rtl::Reference<LwpFrameLayout> xMyFrameLayout(dynamic_cast<LwpFrameLayout*>(m_pGraphicObj->GetLayout(nullptr).get()));
    if (xMyFrameLayout.is())
    {
        LwpLayoutScale* pMyScale = xMyFrameLayout->GetLayoutScale();
        LwpLayoutGeometry* pFrameGeo = xMyFrameLayout->GetGeometry();
        if (pMyScale && pFrameGeo)
        {
            // original drawing size
            long nWidth = 0, nHeight = 0;
            m_pGraphicObj->GetGrafOrgSize(nWidth, nHeight);
            double fGrafOrgWidth = static_cast<double>(nWidth)/TWIPS_PER_CM;
            double fGrafOrgHeight = static_cast<double>(nHeight)/TWIPS_PER_CM;

            // get margin values
            double fLeftMargin = xMyFrameLayout->GetMarginsValue(MARGIN_LEFT);
            double fTopMargin = xMyFrameLayout->GetMarginsValue(MARGIN_TOP);

            // frame size
            double fFrameWidth = LwpTools::ConvertFromUnitsToMetric(pFrameGeo->GetWidth());
            double fFrameHeight = LwpTools::ConvertFromUnitsToMetric(pFrameGeo->GetHeight());

            // get frame offset
            LwpPoint& rOffset = pMyScale->GetOffset();
            double fOffsetX = LwpTools::ConvertFromUnitsToMetric(rOffset.GetX());
            double fOffsetY = LwpTools::ConvertFromUnitsToMetric(rOffset.GetY());

            // get scale mode
            sal_uInt16 nScalemode = pMyScale->GetScaleMode();

            if (nScalemode & LwpLayoutScale::CUSTOM)
            {
                m_aTransformData.fScaleX =
                    LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleWidth()) / fGrafOrgWidth;
                m_aTransformData.fScaleY =
                    LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleHeight()) / fGrafOrgHeight;
            }
            else if (nScalemode & LwpLayoutScale::PERCENTAGE)
            {
                double fScalePercentage = static_cast<double>(pMyScale->GetScalePercentage()) / 1000;
                m_aTransformData.fScaleX = fScalePercentage;
                m_aTransformData.fScaleY = fScalePercentage;
            }
            else if (nScalemode & LwpLayoutScale::FIT_IN_FRAME)
            {
                double fWidth0 = static_cast<double>(right) / TWIPS_PER_CM;
                double fHeight0 = static_cast<double>(bottom) / TWIPS_PER_CM;

                double fWidth1 = LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleWidth());
                double fHeight1 = LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleHeight());

                double fScaleX = fWidth1 / fWidth0;
                double fScaleY = fHeight1 / fHeight0;

                if (nScalemode & LwpLayoutScale::MAINTAIN_ASPECT_RATIO)
                {
                    m_aTransformData.fScaleX = std::min(fScaleX, fScaleY);
                    m_aTransformData.fScaleY = m_aTransformData.fScaleX;
                }
                else
                {
                    m_aTransformData.fScaleX = fWidth1 / fWidth0;
                    m_aTransformData.fScaleY = fHeight1 / fHeight0;
                }
            }

            // placement: centered
            if (xMyFrameLayout->GetScaleCenter())
            {
                tools::Rectangle aBoundRect(static_cast<long>(left*m_aTransformData.fScaleX + fLeftMargin),
                    static_cast<long>(top    * m_aTransformData.fScaleY + fTopMargin),
                    static_cast<long>(right  * m_aTransformData.fScaleX),
                    static_cast<long>(bottom * m_aTransformData.fScaleY));
                Point aCenter = aBoundRect.Center();

                double fNewCenterX = (double(left)/TWIPS_PER_CM + fFrameWidth/*-fOffsetX*/) / 2;
                double fNewCenterY = (double(top)/TWIPS_PER_CM + fFrameHeight/*-fOffsetY*/) / 2;

                m_aTransformData.fOffsetX = fNewCenterX - static_cast<double>(aCenter.X())/TWIPS_PER_CM;
                m_aTransformData.fOffsetY = fNewCenterY -static_cast<double>(aCenter.Y())/TWIPS_PER_CM;
            }
            else
            {
                m_aTransformData.fOffsetX = fOffsetX;
                m_aTransformData.fOffsetY = fOffsetY;
            }

            m_aTransformData.fOffsetX += fLeftMargin;
            m_aTransformData.fOffsetY += fTopMargin;
            m_aTransformData.fLeftMargin = fLeftMargin;
            m_aTransformData.fTopMargin = fTopMargin;
        }
    }

    if (nRecCount > m_pStream->remainingSize())
    {
        SAL_WARN("lwp", "stream too short for claimed no of records");
        nRecCount = m_pStream->remainingSize();
    }

    //load draw object
    for (unsigned short i = 0; i < nRecCount; i++)
    {
        XFFrame* pXFDrawObj = CreateDrawObject();

        if (pXFDrawObj)
        {
            pDrawObjVector->push_back(pXFDrawObj);
        }
    }
}

/**
 * @descr   when we encounter a group object the function will be called to create a XF-drawgroup object
 *      the corresponding drawing objects.
 * @param   pDrawObjVector   a container which will contains the created drawing object of XF-Model.
 */
XFDrawGroup* LwpSdwGroupLoaderV0102::CreateDrawGroupObject()
{
    //flag
    unsigned char BinSignature[2];
    m_pStream->ReadBytes(BinSignature, 2);
    if (BinSignature[0] != 'S' || BinSignature[1] != 'M')
    {
        assert(false);
        return nullptr;
    }
    //version
    unsigned short nVersion;
    m_pStream->ReadUInt16(nVersion);
    if (nVersion<0x0102)
    {
        assert(false);
        return nullptr;
    }
    // topObj, botObj
    m_pStream->SeekRel(4);
    //record count
    unsigned short nRecCount(0);
    m_pStream->ReadUInt16(nRecCount);
    // selCount
    m_pStream->SeekRel(2);
    //boundrect
    unsigned short left(0),top(0),right(0),bottom(0);
    m_pStream->ReadUInt16(left);
    m_pStream->ReadUInt16(top);
    m_pStream->ReadUInt16(right);
    m_pStream->ReadUInt16(bottom);
    // fileSize
    m_pStream->SeekRel(2);

    XFDrawGroup* pXFDrawGroup = new XFDrawGroup();

    if (nRecCount > m_pStream->remainingSize())
    {
        SAL_WARN("lwp", "stream too short for claimed no of records");
        nRecCount = m_pStream->remainingSize();
    }

    //load draw object
    for (unsigned short i = 0; i < nRecCount; i++)
    {
        XFFrame* pXFDrawObj = CreateDrawObject();

        if (pXFDrawObj)
        {
            if (pXFDrawObj->GetFrameType() == enumXFFrameImage)
            {
                m_pDrawObjVector->push_back(pXFDrawObj);
            }
            else
            {
                pXFDrawGroup->Add(pXFDrawObj);
            }
        }
    }

    return pXFDrawGroup;
}

/**
 * @descr   Create the XF-drawing objects according to the object type read from bento stream.
 * @return   the created XF-drawing objects.
 */
XFFrame* LwpSdwGroupLoaderV0102::CreateDrawObject()
{
    //record type
    unsigned char recType(0);
    m_pStream->ReadUChar(recType);

    std::unique_ptr<LwpDrawObj> pDrawObj;
    XFFrame* pRetObjct = nullptr;

    switch(recType)
    {
    case OT_PERPLINE://fall-through
    case OT_LINE:
    {
        pDrawObj.reset(new LwpDrawLine(m_pStream, &m_aTransformData));
        break;
    }
    case OT_POLYLINE:
    {
        pDrawObj.reset(new LwpDrawPolyLine(m_pStream, &m_aTransformData));
        break;
    }
    case OT_POLYGON:
    {
        pDrawObj.reset(new LwpDrawPolygon(m_pStream, &m_aTransformData));
        pDrawObj->SetObjectType(OT_POLYGON);
        break;
    }
    case OT_SQUARE://fall-through
    case OT_RECT:
    {
        pDrawObj.reset(new LwpDrawRectangle(m_pStream, &m_aTransformData));
        break;
    }
    case OT_RNDSQUARE://fall-through
    case OT_RNDRECT:
    {
        pDrawObj.reset(new LwpDrawRectangle(m_pStream, &m_aTransformData));
        pDrawObj->SetObjectType(OT_RNDRECT);
        break;
    }
    case OT_CIRCLE://fall-through
    case OT_OVAL:
    {
        pDrawObj.reset(new LwpDrawEllipse(m_pStream, &m_aTransformData));
        break;
    }
    case OT_ARC:
    {
        pDrawObj.reset(new LwpDrawArc(m_pStream, &m_aTransformData));
        break;
    }
    case OT_TEXT:
    {
        pDrawObj.reset(new LwpDrawTextBox(m_pStream));
        break;
    }
    case OT_TEXTART:
    {
        pDrawObj.reset(new LwpDrawTextArt(m_pStream, &m_aTransformData));
        pDrawObj->SetObjectType(OT_TEXTART);
        break;
    }
    case OT_GROUP:
    {
        m_pStream->SeekRel(2);
        // read out the object header
        pDrawObj.reset(new LwpDrawGroup(m_pStream));

        pRetObjct = CreateDrawGroupObject();

        // set anchor type
        pRetObjct->SetAnchorType(enumXFAnchorFrame);
        break;
    }
    case OT_CHART://fall-through
    case OT_METAFILE://fall-through
    case OT_METAFILEIMG:
    {
        LwpDrawMetafile aMeta(m_pStream);
        break;
    }
    case OT_BITMAP:
        pDrawObj.reset(new LwpDrawBitmap(m_pStream));
        pDrawObj->SetObjectType(OT_BITMAP);
        break;
    }

    // we don't need create the corresponding XF-object of a group object.
    if (pDrawObj && recType != OT_GROUP)
    {
        pRetObjct = pDrawObj->CreateXFDrawObject();
    }

    return pRetObjct;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
