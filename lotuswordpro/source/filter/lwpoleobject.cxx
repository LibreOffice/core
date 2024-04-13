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
 *  For LWP filter architecture prototype - OLE object
 */

#include <lwpfilehdr.hxx>
#include "lwpoleobject.hxx"
#include "lwpframelayout.hxx"
#include <o3tl/numeric.hxx>

/**
 * @descr:   construction function
 * @param:  objHdr - object header, read before entering this function
 * @param: pStrm - file stream
 */
LwpGraphicOleObject::LwpGraphicOleObject(LwpObjectHeader const& objHdr, LwpSvStream* pStrm)
    : LwpContent(objHdr, pStrm)
{
}
/**
 * @descr:   Read GraphicOleObject part
 */
void LwpGraphicOleObject::Read()
{
    LwpContent::Read();

    if (LwpFileHeader::m_nFileRevision >= 0x000b)
    {
        // I'm not sure about the read method
        m_pNextObj.ReadIndexed(m_pObjStrm.get());
        m_pPrevObj.ReadIndexed(m_pObjStrm.get());
    }
    m_pObjStrm->SkipExtra();
}

void LwpGraphicOleObject::GetGrafOrgSize(double& rWidth, double& rHeight)
{
    rWidth = 0;
    rHeight = 0;
}

void LwpGraphicOleObject::GetGrafScaledSize(double& fWidth, double& fHeight)
{
    GetGrafOrgSize(fWidth, fHeight);
    // scaled image size
    double fSclGrafWidth = fWidth; //LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleWidth());
    double fSclGrafHeight
        = fHeight; //LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleHeight());

    rtl::Reference<LwpVirtualLayout> xLayout(GetLayout(nullptr));
    if (xLayout.is() && xLayout->IsFrame())
    {
        LwpFrameLayout* pMyFrameLayout = static_cast<LwpFrameLayout*>(xLayout.get());
        LwpLayoutScale* pMyScale = pMyFrameLayout->GetLayoutScale();
        LwpLayoutGeometry* pFrameGeo = pMyFrameLayout->GetGeometry();

        // original image size
        //double fOrgGrafWidth = (double)m_Cache.Width/TWIPS_PER_CM;
        //double fOrgGrafHeight = (double)m_Cache.Height/TWIPS_PER_CM;

        // get margin values
        double fLeftMargin = pMyFrameLayout->GetMarginsValue(MARGIN_LEFT);
        double fRightMargin = pMyFrameLayout->GetMarginsValue(MARGIN_RIGHT);
        double fTopMargin = pMyFrameLayout->GetMarginsValue(MARGIN_TOP);
        double fBottomMargin = pMyFrameLayout->GetMarginsValue(MARGIN_BOTTOM);

        if (pMyScale && pFrameGeo)
        {
            // frame size
            double fFrameWidth = LwpTools::ConvertFromUnits(pFrameGeo->GetWidth());
            double fFrameHeight = LwpTools::ConvertFromUnits(pFrameGeo->GetHeight());

            // calculate the displayed size of the frame
            double fDisFrameWidth = fFrameWidth - (fLeftMargin + fRightMargin);
            double fDisFrameHeight = fFrameHeight - (fTopMargin + fBottomMargin);

            // get scale mode
            sal_uInt16 nScalemode = pMyScale->GetScaleMode();
            if (nScalemode & LwpLayoutScale::CUSTOM)
            {
                fSclGrafWidth = LwpTools::ConvertFromUnits(pMyScale->GetScaleWidth());
                fSclGrafHeight = LwpTools::ConvertFromUnits(pMyScale->GetScaleHeight());
            }
            else if (nScalemode & LwpLayoutScale::PERCENTAGE)
            {
                double fScalePercentage
                    = static_cast<double>(pMyScale->GetScalePercentage()) / 1000;
                fSclGrafWidth = fScalePercentage * fWidth;
                fSclGrafHeight = fScalePercentage * fHeight;
            }
            else if (nScalemode & LwpLayoutScale::FIT_IN_FRAME)
            {
                if (pMyFrameLayout->IsFitGraphic())
                {
                    fSclGrafWidth = fWidth;
                    fSclGrafHeight = fHeight;
                }
                else if (nScalemode & LwpLayoutScale::MAINTAIN_ASPECT_RATIO)
                {
                    if (fHeight == 0.0 || fDisFrameHeight == 0.0)
                        throw o3tl::divide_by_zero();
                    if (fWidth / fHeight >= fDisFrameWidth / fDisFrameHeight)
                    {
                        fSclGrafWidth = fDisFrameWidth;
                        if (fWidth == 0.0)
                            throw o3tl::divide_by_zero();
                        fSclGrafHeight = (fDisFrameWidth / fWidth) * fHeight;
                    }
                    else
                    {
                        fSclGrafHeight = fDisFrameHeight;
                        fSclGrafWidth = (fDisFrameHeight / fHeight) * fWidth;
                    }
                }
                else
                {
                    fSclGrafWidth = fDisFrameWidth;
                    fSclGrafHeight = fDisFrameHeight;
                }
            }
        }
    }
    fWidth = fSclGrafWidth;
    fHeight = fSclGrafHeight;
}

/**
 * @descr:   construction function
 * @param:  objHdr - object header, read before entering this function
 * @param: pStrm - file stream
 */
LwpOleObject::LwpOleObject(LwpObjectHeader const& objHdr, LwpSvStream* pStrm)
    : LwpGraphicOleObject(objHdr, pStrm)
    , cPersistentFlags(0)
    , m_SizeRect(0, 0, 5, 5)
{
}
/**
 * @descr:   Read VO_OLEOBJECT record
 */
void LwpOleObject::Read()
{
    LwpGraphicOleObject::Read();

    cPersistentFlags = m_pObjStrm->QuickReaduInt16();

    // qCMarker read

    if (LwpFileHeader::m_nFileRevision >= 0x0004)
    {
        LwpObjectID ID;
        m_pObjStrm->QuickReaduInt16();

        m_pObjStrm->QuickReadStringPtr();

        if (LwpFileHeader::m_nFileRevision < 0x000B)
        {
            // null pointers have a VO_INVALID type
            //if (VO_INVALID == m_pObjStrm->QuickReaduInt16())
            //  return;

            ID.Read(m_pObjStrm.get());
            //return m_pObjStrm->Locate(ID);
        }
        else
        {
            ID.ReadIndexed(m_pObjStrm.get());
            //if (ID.IsNull())
            //  return;

            //return m_pObjStrm->Locate(ID);
        }
    }

    if (m_pObjStrm->CheckExtra())
    {
        m_pObjStrm->QuickReaduInt16();
        m_pObjStrm->SkipExtra();
    }
}

/**
 * @descr:   Parse VO_OLEOBJECT and dump to XML stream only on WIN32 platform
 * @param:  pOutputStream - stream to dump OLE object
 * @param:  pFrameLayout -  framelayout object used to dump OLE object
 */
void LwpOleObject::Parse(IXFStream* /*pOutputStream*/) {}

void LwpOleObject::XFConvert(XFContentContainer* /*pCont*/) {}

void LwpOleObject::GetGrafOrgSize(double& rWidth, double& rHeight)
{
    rWidth = static_cast<double>(m_SizeRect.GetWidth()) / 1000; //cm unit
    rHeight = static_cast<double>(m_SizeRect.GetHeight()) / 1000; //cm unit
}

void LwpOleObject::RegisterStyle() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
