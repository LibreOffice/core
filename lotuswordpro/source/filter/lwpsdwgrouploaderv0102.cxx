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
/*****************************************************************************
 * Change History
 * Mar 2005         revised for new processing procedure.
 * Jan 2005         created
 ****************************************************************************/
#include "lwpsdwgrouploaderv0102.hxx"
#include "lwpdrawobj.hxx"
#include "lwptools.hxx"
#include "lwpcharsetmgr.hxx"
#include "lwpgrfobj.hxx"
#include "lwpframelayout.hxx"

#include "xfilter/xfcolor.hxx"
#include "xfilter/xfdrawline.hxx"
#include "xfilter/xfdrawpath.hxx"
#include "xfilter/xfdrawpolyline.hxx"
#include "xfilter/xfdrawrect.hxx"
#include "xfilter/xfdrawpolygon.hxx"
#include "xfilter/xfdrawgroup.hxx"
#include "xfilter/xfdrawstyle.hxx"
#include "xfilter/xfdrawlinestyle.hxx"
#include "xfilter/xfparagraph.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfdrawgroup.hxx"

LwpSdwGroupLoaderV0102::LwpSdwGroupLoaderV0102(SvStream* pStream, LwpGraphicObject* pGraphicObj)
    : m_pStream(pStream)
    , m_pGraphicObj(pGraphicObj)
{
}
LwpSdwGroupLoaderV0102::~LwpSdwGroupLoaderV0102(void)
{
}

// add by , 03/25/2005
/**
 * @descr   entry of lwp-drawing objects. the functin begins to parse the sdw-drawing bento stream and create
 *      the corresponding drawing objects.
 * @param   pDrawObjVector   a container which will contains the created drawing object of XF-Model.
 */
void LwpSdwGroupLoaderV0102::BeginDrawObjects(std::vector <XFFrame*>* pDrawObjVector)
{
    // save the container
    m_pDrawObjVector = pDrawObjVector;

    //flag
    unsigned char BinSignature[2];
    m_pStream->Read(BinSignature,2);
    if (BinSignature[0] != 'S' || BinSignature[1] != 'M')
    {
        assert(false);
        return;
    }
    //version
    unsigned short nVersion;
    m_pStream->Read(&nVersion,2);
    if (nVersion<0x0102)
    {
        assert(false);
        return;
    }
    // topObj, botObj
    m_pStream->SeekRel(4);
    //record count
    unsigned short nRecCount;
    m_pStream->Read(&nRecCount,2);
    // selCount
    m_pStream->SeekRel(2);
    //boundrect
    unsigned short left,top,right,bottom;
    m_pStream->Read(&left,2);
    m_pStream->Read(&top,2);
    m_pStream->Read(&right,2);
    m_pStream->Read(&bottom,2);
    // fileSize
    m_pStream->SeekRel(2);

    //for calculating transformation params.
    LwpFrameLayout* pMyFrameLayout = static_cast<LwpFrameLayout*>(m_pGraphicObj->GetLayout(NULL));
    if (pMyFrameLayout)
    {
        LwpLayoutScale* pMyScale = pMyFrameLayout->GetLayoutScale();
        LwpLayoutGeometry* pFrameGeo = pMyFrameLayout->GetGeometry();
        if (pMyScale && pFrameGeo)
        {
            // original drawing size
            long nWidth = 0, nHeight = 0;
            m_pGraphicObj->GetGrafOrgSize(nWidth, nHeight);
            double fGrafOrgWidth = (double)nWidth/TWIPS_PER_CM;
            double fGrafOrgHeight = (double)nHeight/TWIPS_PER_CM;

            // get margin values
            double fLeftMargin = pMyFrameLayout->GetMarginsValue(MARGIN_LEFT);
//          double fRightMargin = pMyFrameLayout->GetMarginsValue(MARGIN_RIGHT);
            double fTopMargin = pMyFrameLayout->GetMarginsValue(MARGIN_TOP);
//          double fBottomMargin = pMyFrameLayout->GetMarginsValue(MARGIN_BOTTOM);

            // frame size
            double fFrameWidth = LwpTools::ConvertFromUnitsToMetric(pFrameGeo->GetWidth());
            double fFrameHeight = LwpTools::ConvertFromUnitsToMetric(pFrameGeo->GetHeight());

            // get frame offset
            LwpPoint* pOffset = pMyScale->GetOffset();
            double fOffsetX = LwpTools::ConvertFromUnitsToMetric(pOffset->GetX());
            double fOffsetY = LwpTools::ConvertFromUnitsToMetric(pOffset->GetY());

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
                double fScalePercentage = (double)pMyScale->GetScalePercentage() / 1000;
                m_aTransformData.fScaleX = fScalePercentage;
                m_aTransformData.fScaleY = fScalePercentage;
            }
            else if (nScalemode & LwpLayoutScale::FIT_IN_FRAME)
            {
                double fWidth0 = (double)right / TWIPS_PER_CM;
                double fHeight0 = (double)bottom / TWIPS_PER_CM;

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
            if (pMyFrameLayout->GetScaleCenter())
            {
                Rectangle aBoundRect(static_cast<long>(left*m_aTransformData.fScaleX + fLeftMargin),
                    static_cast<long>(top    * m_aTransformData.fScaleY + fTopMargin),
                    static_cast<long>(right  * m_aTransformData.fScaleX),
                    static_cast<long>(bottom * m_aTransformData.fScaleY));
                Point aCenter = aBoundRect.Center();

                double fNewCenterX = (double(left)/TWIPS_PER_CM + fFrameWidth/*-fOffsetX*/) / 2;
                double fNewCenterY = (double(top)/TWIPS_PER_CM + fFrameHeight/*-fOffsetY*/) / 2;

                m_aTransformData.fOffsetX = fNewCenterX - (double)aCenter.X()/TWIPS_PER_CM;
                m_aTransformData.fOffsetY = fNewCenterY -(double)aCenter.Y()/TWIPS_PER_CM;
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

    //load draw object
    for (unsigned short i = 0; i < nRecCount; i++)
    {
        XFFrame* pXFDrawObj = this->CreateDrawObject();

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
XFDrawGroup* LwpSdwGroupLoaderV0102::CreateDrawGroupObject(void)
{
    //flag
    unsigned char BinSignature[2];
    m_pStream->Read(BinSignature,2);
    if (BinSignature[0] != 'S' || BinSignature[1] != 'M')
    {
        assert(false);
        return NULL;
    }
    //version
    unsigned short nVersion;
    m_pStream->Read(&nVersion,2);
    if (nVersion<0x0102)
    {
        assert(false);
        return NULL;
    }
    // topObj, botObj
    m_pStream->SeekRel(4);
    //record count
    unsigned short nRecCount;
    m_pStream->Read(&nRecCount,2);
    // selCount
    m_pStream->SeekRel(2);
    //boundrect
    unsigned short left,top,right,bottom;
    m_pStream->Read(&left,2);
    m_pStream->Read(&top,2);
    m_pStream->Read(&right,2);
    m_pStream->Read(&bottom,2);
    // fileSize
    m_pStream->SeekRel(2);

    XFDrawGroup* pXFDrawGroup = new XFDrawGroup();

    //load draw object
    for (unsigned short i = 0; i < nRecCount; i++)
    {
        XFFrame* pXFDrawObj = this->CreateDrawObject();

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
XFFrame* LwpSdwGroupLoaderV0102::CreateDrawObject(void)
{
    //record type
    unsigned char recType;
    m_pStream->Read(&recType,1);

    LwpDrawObj* pDrawObj = NULL;
    XFFrame* pRetObjct = NULL;

    switch(recType)
    {
    case OT_PERPLINE://fall-through
    case OT_LINE:
    {
        pDrawObj = new LwpDrawLine(m_pStream, &m_aTransformData);
        break;
    }
    case OT_POLYLINE:
    {
        pDrawObj = new LwpDrawPolyLine(m_pStream, &m_aTransformData);
        break;
    }
    case OT_POLYGON:
    {
        pDrawObj = new LwpDrawPolygon(m_pStream, &m_aTransformData);
        pDrawObj->SetObjectType(OT_POLYGON);
        break;
    }
    case OT_SQUARE://fall-through
    case OT_RECT:
    {
        pDrawObj = new LwpDrawRectangle(m_pStream, &m_aTransformData);
        break;
    }
    case OT_RNDSQUARE://fall-through
    case OT_RNDRECT:
    {
        pDrawObj = new LwpDrawRectangle(m_pStream, &m_aTransformData);
        pDrawObj->SetObjectType(OT_RNDRECT);
        break;
    }
    case OT_CIRCLE://fall-through
    case OT_OVAL:
    {
        pDrawObj = new LwpDrawEllipse(m_pStream, &m_aTransformData);
        break;
    }
    case OT_ARC:
    {
        pDrawObj = new LwpDrawArc(m_pStream, &m_aTransformData);
        break;
    }
    case OT_TEXT:
    {
        pDrawObj = new LwpDrawTextBox(m_pStream);
        break;
    }
    case OT_TEXTART:
    {
        pDrawObj = new LwpDrawTextArt(m_pStream, &m_aTransformData);
        pDrawObj->SetObjectType(OT_TEXTART);
        break;
    }
    case OT_GROUP:
    {
        m_pStream->SeekRel(2);
        // read out the object header
        pDrawObj = new LwpDrawGroup(m_pStream);

        pRetObjct = this->CreateDrawGroupObject();

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
        pDrawObj = new LwpDrawBitmap(m_pStream);
        pDrawObj->SetObjectType(OT_BITMAP);
        break;
    }

    // we don't need create the corresponding XF-object of a group object.
    if (pDrawObj && recType != OT_GROUP)
    {
        pRetObjct = pDrawObj->CreateXFDrawObject();
    }

    if (pDrawObj)
    {
        delete pDrawObj;
        pDrawObj = NULL;
    }

    return pRetObjct;
}
// end add

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
