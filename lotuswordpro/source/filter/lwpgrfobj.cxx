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
* 2005/2  draft code for chart
****************************************************************************/

/**
 * @file
 *  For LWP filter architecture prototype
*/
#include <stdio.h>

#include <boost/scoped_ptr.hpp>

#include "lwp9reader.hxx"
#include "lwpgrfobj.hxx"
#include "lwpsdwfileloader.hxx"
#include "bento.hxx"

#include "lwpglobalmgr.hxx"
#include "xfilter/xfframe.hxx"
#include "xfilter/xfimage.hxx"
#include "xfilter/xfimagestyle.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfparagraph.hxx"
#include "xfilter/xfannotation.hxx"

//For chart
#include "string.h"

#include <osl/thread.h>

#define EF_NONE 0x0000
#define EF_ODMA 0x0002

LwpGraphicObject::LwpGraphicObject(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpGraphicOleObject(objHdr, pStrm)
    , m_nCachedBaseLine(0)
    , m_bIsLinked(0)
    , m_bCompressed(0)
{
    memset(m_sDataFormat, 0, sizeof(m_sDataFormat));
    memset(m_sServerContextFormat, 0, sizeof(m_sServerContextFormat));
}

LwpGraphicObject::~LwpGraphicObject()
{
}

void LwpGraphicObject::Read()
{
    LwpGraphicOleObject::Read();
    m_pObjStrm->QuickReaduInt16(); //disksize
    sal_uInt16 strsize = m_pObjStrm->QuickReaduInt16();
    if (strsize<AFID_MAX_FILE_FORMAT_SIZE)
    {
        m_pObjStrm->QuickRead(m_sDataFormat,strsize);
        m_sDataFormat[strsize] = '\0';
    }
    sal_uInt32 nServerContextSize = m_pObjStrm->QuickReaduInt32();
    unsigned char *pServerContext = NULL;
    if (nServerContextSize > 0)
    {
        pServerContext = new unsigned char[nServerContextSize];
        m_pObjStrm->QuickRead(pServerContext, static_cast<sal_uInt16>(nServerContextSize));
        if (nServerContextSize > 44)
        {
            m_aIPData.nBrightness = pServerContext[14];
            m_aIPData.nContrast = pServerContext[19];
            m_aIPData.nEdgeEnchancement = pServerContext[24];
            m_aIPData.nSmoothing = pServerContext[29];
            m_aIPData.bInvertImage = (sal_Bool)(pServerContext[34] == 0x01);
            m_aIPData.bAutoContrast = (sal_Bool)(pServerContext[44] == 0x00);
        }
    }
    m_pObjStrm->QuickReaduInt16(); //disksize
    strsize = m_pObjStrm->QuickReaduInt16();
    if (strsize<AFID_MAX_FILE_FORMAT_SIZE)
    {
        m_pObjStrm->QuickRead(m_sServerContextFormat,strsize);
        m_sServerContextFormat[strsize] = '\0';
    }
    if (nServerContextSize == 0)
    {
        if (strcmp((char *)m_sServerContextFormat, ".cht") == 0 &&
            strcmp((char *)m_sDataFormat, ".sdw") == 0)
        {
            strcpy((char *)m_sServerContextFormat, ".lch");
            strcpy((char *)m_sDataFormat, ".lch");
        }
    }
    m_nCachedBaseLine = m_pObjStrm->QuickReadInt32();
    m_bIsLinked = m_pObjStrm->QuickReadInt16();
    unsigned char * pFilterContext = NULL;

    if (m_bIsLinked)
    {
        m_LinkedFilePath = m_pObjStrm->QuickReadStringPtr();

        sal_uInt32 nFilterContextSize = m_pObjStrm->QuickReaduInt32();
        if (nFilterContextSize > 0)
        {
            pFilterContext = new unsigned char[nFilterContextSize];
            m_pObjStrm->QuickRead(pFilterContext, static_cast<sal_uInt16>(nFilterContextSize));
        }
        if (LwpFileHeader::m_nFileRevision >= 0x000b)
        {

            // read external file object stuff
            sal_uInt16 type = m_pObjStrm->QuickReaduInt16();
            if ((EF_ODMA != type) && (EF_NONE != type)) // don't know about this
            {
                sal_uInt32 size = m_pObjStrm->QuickReaduInt32();
                m_pObjStrm->SeekRel(static_cast<sal_uInt16>(size));
            }
            // else no external file object
        }
    }

    if (LwpFileHeader::m_nFileRevision >= 0x000b)
    {
        m_bCompressed = m_pObjStrm->QuickReadInt16();
        m_Cache.LinkedFileSize = m_pObjStrm->QuickReaduInt32();
        m_Cache.LinkedFileTime = m_pObjStrm->QuickReaduInt32();
        m_Cache.Width = m_pObjStrm->QuickReadInt32();
        m_Cache.Height = m_pObjStrm->QuickReadInt32();
    }

    if(LwpFileHeader::m_nFileRevision >= 0x000c)
    {
        m_WatermarkName = m_pObjStrm->QuickReadStringPtr();
    }

    if (pServerContext != NULL)
        delete[] pServerContext;

    if (pFilterContext != NULL)
        delete[] pFilterContext;

}

void LwpGraphicObject::XFConvert (XFContentContainer* pCont)
{
    if ((m_sServerContextFormat[1]=='s'&&m_sServerContextFormat[2]=='d'&&m_sServerContextFormat[3]=='w'))
    {
        std::vector <XFFrame*>::iterator iter;
        for (iter = m_vXFDrawObjects.begin(); iter != m_vXFDrawObjects.end(); ++iter)
        {
            pCont->Add(*iter);
        }
    }
    else if (this->IsGrafFormatValid())
    {
        XFImage* pImage = static_cast<XFImage*>(m_vXFDrawObjects.front());

        if (m_bIsLinked)
        {
            OUString fileURL = LwpTools::convertToFileUrl(OUStringToOString(m_LinkedFilePath, osl_getThreadTextEncoding()));
            pImage->SetFileURL(fileURL);
        }
        else
        {
            sal_uInt8* pGrafData = NULL;
            sal_uInt32 nDataLen = this->GetRawGrafData(pGrafData);

            if (pGrafData)
            {
                pImage->SetImageData(pGrafData, nDataLen);

                // delete used image data
                delete [] pGrafData;
                pGrafData = NULL;
            }
        }

        pCont->Add(pImage);
    }
    else if((m_sServerContextFormat[1]=='t'&&m_sServerContextFormat[2]=='e'&&m_sServerContextFormat[3]=='x'))
    {
        XFConvertEquation(pCont);
    }
}

#include "lwpframelayout.hxx"

/**
 * @descr   judge if the graphic format is what we can support: bmp, jpg, wmf, gif, tgf(tif). other format will be filtered to
 *  these formats by Word Pro.
 * @return  sal_True if yes sal_False if not.
 */
sal_Bool LwpGraphicObject::IsGrafFormatValid()
{
    if ((m_sServerContextFormat[1]=='b'&& m_sServerContextFormat[2]=='m' && m_sServerContextFormat[3]=='p')
    || (m_sServerContextFormat[1]=='j' && m_sServerContextFormat[2]=='p' && m_sServerContextFormat[3]=='g')
    || (m_sServerContextFormat[1]=='w' && m_sServerContextFormat[2]=='m' && m_sServerContextFormat[3]=='f')
    || (m_sServerContextFormat[1]=='g' && m_sServerContextFormat[2]=='i' && m_sServerContextFormat[3]=='f')
    || (m_sServerContextFormat[1]=='t' && m_sServerContextFormat[2]=='g' && m_sServerContextFormat[3]=='f')
    || (m_sServerContextFormat[1]=='p' && m_sServerContextFormat[2]=='n' && m_sServerContextFormat[3]=='g')
    || (m_sServerContextFormat[1]=='e' && m_sServerContextFormat[2]=='p' && m_sServerContextFormat[3]=='s'))
    {
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

/**
 * @descr   create drawing object and image object.
 */
void LwpGraphicObject::RegisterStyle()
{
    if (m_sServerContextFormat[1]=='s'&&m_sServerContextFormat[2]=='d'&&m_sServerContextFormat[3]=='w')
    {
        this->CreateDrawObjects();
    }
    // test codes for importing pictures
    else if(this->IsGrafFormatValid())
    {
        this->CreateGrafObject();
    }

    if (m_sServerContextFormat[1]=='l'&&m_sServerContextFormat[2]=='c'&&m_sServerContextFormat[3]=='h')
    {
        LwpVirtualLayout* pMyLayout = GetLayout(NULL);
        if(pMyLayout->IsFrame())
        {
            XFFrameStyle* pXFFrameStyle = new XFFrameStyle();
            pXFFrameStyle->SetXPosType(enumXFFrameXPosFromLeft, enumXFFrameXRelFrame);
            pXFFrameStyle->SetYPosType(enumXFFrameYPosFromTop, enumXFFrameYRelPara);
            XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
            m_strStyleName = pXFStyleManager->AddStyle(pXFFrameStyle)->GetStyleName();
        }
    }

}

/**
 * @descr   create drawing object.
 */
void LwpGraphicObject::CreateDrawObjects()
{
    // if small file, use the compressed stream for BENTO
    LwpSvStream* pStream = m_pStrm->GetCompressedStream() ?  m_pStrm->GetCompressedStream(): m_pStrm;

    OpenStormBento::LtcBenContainer* pBentoContainer;
    sal_uLong ulRet = OpenStormBento::BenOpenContainer(pStream, &pBentoContainer);
    if (ulRet != OpenStormBento::BenErr_OK)
        return;

    SvStream* pDrawObjStream = NULL;

    // get graphic object's bento objet name
    LwpObjectID* pMyID = this->GetObjectID();
    std::string aGrfObjName;
    this->GetBentoNamebyID(pMyID,  aGrfObjName);

    // get bento stream by the name
    pBentoContainer->CreateGraphicStream(pDrawObjStream, aGrfObjName.c_str());
    if (pDrawObjStream)
    {
        LwpSdwFileLoader fileLoader(pDrawObjStream, this);
        fileLoader.CreateDrawObjects(&m_vXFDrawObjects);

        delete pDrawObjStream;
        pDrawObjStream = NULL;
    }
}

/**
 * @descr   create drawing object.
 */
void LwpGraphicObject::GetBentoNamebyID(LwpObjectID* pMyID, std::string& rName)
{
    sal_uInt16 nHigh = pMyID->GetHigh();
    sal_uInt16 nLow = pMyID->GetLow();
    char pTempStr[32];
    rName = std::string("Gr");
    sprintf(pTempStr, "%X,%X", nHigh, nLow);
    rName.append(pTempStr);
}

/**
 * @descr   get the image data read from bento stream according to the VO_GRAPHIC ID.
 * @param   pGrafData   the array to store the image data. the pointer need to be deleted outside.
 * @return  the length of the image data.
 */
sal_uInt32 LwpGraphicObject::GetRawGrafData(sal_uInt8*& pGrafData)
{
    // create graphic object
    // if small file, use the compressed stream for BENTO
    LwpSvStream* pStream = m_pStrm->GetCompressedStream() ?  m_pStrm->GetCompressedStream(): m_pStrm;

    boost::scoped_ptr<OpenStormBento::LtcBenContainer> pBentoContainer;
    {
        OpenStormBento::LtcBenContainer* pTmp(0);
        sal_uLong ulRet = OpenStormBento::BenOpenContainer(pStream, &pTmp);
        pBentoContainer.reset(pTmp);
        if (ulRet != OpenStormBento::BenErr_OK)
            return 0;
    }

    SvStream* pGrafStream = NULL;

    // get graphic object's bento objet name
    LwpObjectID* pMyID = this->GetObjectID();
    std::string aGrfObjName;
    this->GetBentoNamebyID(pMyID,  aGrfObjName);

    // get bento stream by the name
    pBentoContainer->CreateGraphicStream(pGrafStream, aGrfObjName.c_str());
    SvMemoryStream* pMemGrafStream = static_cast<SvMemoryStream*>(pGrafStream);

    if (pMemGrafStream)
    {
        // read image data
        sal_uInt32 nDataLen = pMemGrafStream->GetEndOfData();
        pGrafData = new sal_uInt8 [nDataLen];
        pMemGrafStream->Read(pGrafData, nDataLen);

        delete pMemGrafStream;
        pMemGrafStream = NULL;

        return nDataLen;
    }

    return 0;
}

/**
 * @descr   get the image data (only -D dara) read from bento stream according to the VO_GRAPHIC ID.
 * @param   pGrafData   the array to store the image data. the pointer need to be deleted outside.
 * @return  the length of the image data.
 */
sal_uInt32 LwpGraphicObject::GetGrafData(sal_uInt8*& pGrafData)
{
    // create graphic object
    // if small file, use the compressed stream for BENTO
    LwpSvStream* pStream = m_pStrm->GetCompressedStream() ?  m_pStrm->GetCompressedStream(): m_pStrm;

    OpenStormBento::LtcBenContainer* pBentoContainer;
    sal_uLong ulRet = OpenStormBento::BenOpenContainer(pStream, &pBentoContainer);
    if (ulRet != OpenStormBento::BenErr_OK)
        return 0;

    SvStream* pGrafStream = NULL;

    // get graphic object's bento objet name
    LwpObjectID* pMyID = this->GetObjectID();
    std::string aGrfObjName;
    this->GetBentoNamebyID(pMyID,  aGrfObjName);

    char sDName[64]="";
    sprintf(sDName, "%s-D", aGrfObjName.c_str());

    // get bento stream by the name
    pGrafStream = pBentoContainer->FindValueStreamWithPropertyName(sDName);

    SvMemoryStream* pMemGrafStream = static_cast<SvMemoryStream*>(pGrafStream);

    if (pMemGrafStream)
    {
        // read image data
        sal_uInt32 nPos = pGrafStream->Tell();
        pGrafStream->Seek(STREAM_SEEK_TO_END);
        sal_uInt32 nDataLen = pGrafStream->Tell();
        pGrafStream->Seek(nPos);

        pGrafData = new sal_uInt8 [nDataLen];
        pMemGrafStream->Read(pGrafData, nDataLen);

        delete pMemGrafStream;
        pMemGrafStream = NULL;

        return nDataLen;
    }

    return 0;
}

/**
 * @descr   create xf-image object and save it in the container: m_vXFDrawObjects.
 */
void LwpGraphicObject::CreateGrafObject()
{
    XFImage* pImage = new XFImage();

    // set image processing styles
    XFImageStyle* pImageStyle = new XFImageStyle();
    if (m_sServerContextFormat[1]!='w' || m_sServerContextFormat[2]!='m' || m_sServerContextFormat[3]!='f')
    {
        if (m_aIPData.nBrightness != 50)
        {
            sal_Int32 nSODCBrightness = (sal_Int32)m_aIPData.nBrightness*2 - 100;
            pImageStyle->SetBrightness(nSODCBrightness);
        }
        if (m_aIPData.nContrast != 50)
        {
            sal_Int32 nSODCContrast = (sal_Int32)(80 - (double)m_aIPData.nContrast*1.6);
            pImageStyle->SetContrast(nSODCContrast);
        }
    }

    // set scale and crop styles
    LwpAssociatedLayouts* pLayoutWithMe = GetLayoutsWithMe();
    LwpFrameLayout* pMyFrameLayout =
        static_cast<LwpFrameLayout*>(pLayoutWithMe->GetOnlyLayout()->obj(VO_FRAMELAYOUT));
    if (pMyFrameLayout)
    {
        LwpLayoutScale* pMyScale = pMyFrameLayout->GetLayoutScale();
        LwpLayoutGeometry* pFrameGeo = pMyFrameLayout->GetGeometry();

        // original image size
        double fOrgGrafWidth = (double)m_Cache.Width/TWIPS_PER_CM;
        double fOrgGrafHeight = (double)m_Cache.Height/TWIPS_PER_CM;

        // get margin values
        double fLeftMargin = pMyFrameLayout->GetMarginsValue(MARGIN_LEFT);
        double fRightMargin = pMyFrameLayout->GetMarginsValue(MARGIN_RIGHT);
        double fTopMargin = pMyFrameLayout->GetMarginsValue(MARGIN_TOP);
        double fBottomMargin = pMyFrameLayout->GetMarginsValue(MARGIN_BOTTOM);

        if (pMyScale && pFrameGeo)
        {
            // frame size
            double fFrameWidth = LwpTools::ConvertFromUnitsToMetric(pFrameGeo->GetWidth());
            double fFrameHeight = LwpTools::ConvertFromUnitsToMetric(pFrameGeo->GetHeight());

            // calculate the displayed size of the frame
            double fDisFrameWidth = fFrameWidth - (fLeftMargin+fRightMargin);
            double fDisFrameHeight = fFrameHeight - (fTopMargin+fBottomMargin);

            // scaled image size
            double fSclGrafWidth = fOrgGrafWidth;
            double fSclGrafHeight = fOrgGrafHeight;

            // get scale mode
            sal_uInt16 nScalemode = pMyScale->GetScaleMode();
            if (nScalemode & LwpLayoutScale::CUSTOM)
            {
                fSclGrafWidth = LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleWidth());
                fSclGrafHeight = LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleHeight());
            }
            else if (nScalemode & LwpLayoutScale::PERCENTAGE)
            {
                double fScalePercentage = (double)pMyScale->GetScalePercentage() / 1000;
                fSclGrafWidth = fScalePercentage * fOrgGrafWidth;
                fSclGrafHeight = fScalePercentage * fOrgGrafHeight;
            }
            else if (nScalemode & LwpLayoutScale::FIT_IN_FRAME)
            {
                if (pMyFrameLayout->IsFitGraphic())
                {
                    fSclGrafWidth = fOrgGrafWidth;
                    fSclGrafHeight = fOrgGrafHeight;
                }
                else if (nScalemode & LwpLayoutScale::MAINTAIN_ASPECT_RATIO)
                {
                    if (fOrgGrafWidth/fOrgGrafHeight >= fDisFrameWidth/fDisFrameHeight)
                    {
                        fSclGrafWidth = fDisFrameWidth;
                        fSclGrafHeight = (fDisFrameWidth/fOrgGrafWidth) * fOrgGrafHeight;
                    }
                    else
                    {
                        fSclGrafHeight = fDisFrameHeight;
                        fSclGrafWidth = (fDisFrameHeight/fOrgGrafHeight) * fOrgGrafWidth;
                    }
                }
                else
                {
                    fSclGrafWidth = fDisFrameWidth;
                    fSclGrafHeight = fDisFrameHeight;
                }
            }

            // scaled ratio
            double fXRatio = fSclGrafWidth / fOrgGrafWidth;
            double fYRatio = fSclGrafHeight / fOrgGrafHeight;

            // set image to scaled size.
            pImage->SetWidth(fSclGrafWidth);
            pImage->SetHeight(fSclGrafHeight);

            // placement:centered or tiled. tiled style is not supported so it's processed together with centered.
            if (pMyFrameLayout->GetScaleCenter() || pMyFrameLayout->GetScaleTile())
            {
                // set center alignment
                pImageStyle->SetXPosType(enumXFFrameXPosCenter, enumXFFrameXRelFrame);
                pImageStyle->SetYPosType(enumXFFrameYPosMiddle, enumXFFrameYRelFrame);

                // need horizontal crop?
                double fClipWidth = 0;
                double fClipHeight = 0;
                sal_Bool sal_bCropped = sal_False;
                if (fSclGrafWidth > fDisFrameWidth)
                {
                    fClipWidth = (fSclGrafWidth-fDisFrameWidth ) / 2 / fXRatio;
                    sal_bCropped = sal_True;
                }

                // need vertical crop?
                if (fSclGrafHeight > fDisFrameHeight)
                {
                    fClipHeight = (fSclGrafHeight-fDisFrameHeight ) / 2 / fYRatio;
                    sal_bCropped = sal_True;
                }

                if (sal_bCropped)
                {
                    pImageStyle->SetClip(fClipWidth, fClipWidth, fClipHeight, fClipHeight);
                    pImage->SetWidth(fDisFrameWidth);
                    pImage->SetHeight(fDisFrameHeight);
                }
            }
            // placement:automatic
            else
            {
                // set left-top alignment
                pImageStyle->SetYPosType(enumXFFrameYPosFromTop, enumXFFrameYRelFrame);
                pImageStyle->SetXPosType(enumXFFrameXPosFromLeft, enumXFFrameXRelFrame);

                // get image position offset
                LwpPoint* pOffset = pMyScale->GetOffset();
                double fOffsetX = LwpTools::ConvertFromUnitsToMetric(pOffset->GetX());
                double fOffsetY = LwpTools::ConvertFromUnitsToMetric(pOffset->GetY());

                struct LwpRect
                {
                    double fLeft;
                    double fRight;
                    double fTop;
                    double fBottom;

                    LwpRect()
                    {
                        fLeft = 0.00;
                        fRight = 0.00;
                        fTop = 0.00;
                        fBottom = 0.00;
                    }
                    LwpRect(double fL, double fR, double fT, double fB)
                    {
                        fLeft = fL;
                        fRight = fR;
                        fTop = fT;
                        fBottom = fB;
                    }
                };
                LwpRect aFrameRect(-fOffsetX, (fDisFrameWidth-fOffsetX), (-fOffsetY), ((fDisFrameHeight-fOffsetY)));
                LwpRect aImageRect(0, fSclGrafWidth, 0, fSclGrafHeight);
                LwpRect aCropRect;

                if (aFrameRect.fRight <= aImageRect.fLeft || aFrameRect.fLeft >= aImageRect.fRight
                    ||aFrameRect.fBottom <= aImageRect.fTop|| aFrameRect.fTop >= aImageRect.fBottom)
                {
                    // display blank
                }
                else// need cropped
                {
                    // horizontal crop
                    if (aFrameRect.fLeft > aImageRect.fLeft)
                    {
                        aCropRect.fLeft = (aFrameRect.fLeft - aImageRect.fLeft) / fXRatio;
                    }

                    if (aFrameRect.fRight < aImageRect.fRight)
                    {
                        aCropRect.fRight = (aImageRect.fRight - aFrameRect.fRight) / fXRatio;
                    }

                    // vertical crop
                    if (aFrameRect.fTop > aImageRect.fTop)
                    {
                        aCropRect.fTop = (aFrameRect.fTop - aImageRect.fTop) / fYRatio;
                    }
                    if (aFrameRect.fBottom < aImageRect.fBottom)
                    {
                        aCropRect.fBottom = (aImageRect.fBottom - aFrameRect.fBottom) / fYRatio;
                    }

                    pImageStyle->SetClip(aCropRect.fLeft, aCropRect.fRight, aCropRect.fTop, aCropRect.fBottom);
                    double fPicWidth = fSclGrafWidth - (aCropRect.fLeft+aCropRect.fRight)*fXRatio;
                    double fPicHeight = fSclGrafHeight- (aCropRect.fTop+aCropRect.fBottom)*fYRatio;
                    double fX = fOffsetX > 0 ? fOffsetX : 0.00;
                    double fY = fOffsetY > 0 ? fOffsetY : 0.00;
                    pImage->SetPosition((fX+fLeftMargin), (fY+fTopMargin), fPicWidth, fPicHeight);
                }
            }
        }
    }

    // set style for the image
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pImage->SetStyleName(pXFStyleManager->AddStyle(pImageStyle)->GetStyleName());

    // set archor to frame
    pImage->SetAnchorType(enumXFAnchorFrame);

    // set object name
    LwpAtomHolder* pHolder = this->GetName();
    if ( pHolder && !pHolder->str().isEmpty() )
    {
        pImage->SetName(pHolder->str());
    }

    // insert image object into array
    m_vXFDrawObjects.push_back(pImage);

}

/**
 * @descr   Reserve the equation text in a note in the context.
 */
void LwpGraphicObject::XFConvertEquation(XFContentContainer * pCont)
{
    sal_uInt8* pGrafData = NULL;
    sal_uInt32 nDataLen = this->GetGrafData(pGrafData);
    if(pGrafData)
    {
        //convert equation
        XFParagraph* pXFPara = new XFParagraph;
        pXFPara->Add(A2OUSTR("Formula:"));
        //add notes
        XFAnnotation* pXFNote = new XFAnnotation;
        //add equation to comment notes
        XFParagraph* pXFNotePara = new XFParagraph;
        //equation header text: Times New Roman,
        //                                18,12,0,0,0,0,0.
        //                                 .TCIformat{2}
        //total head length = 45
        sal_uInt32 nBegin = 45;
        sal_uInt32 nEnd = nDataLen -1;

        if(pGrafData[nEnd] == '$' && pGrafData[nEnd-1]!= '\\')
        {
            //equation body is contained by '$';
            nBegin++;
            nEnd--;
        }

        if(nEnd >= nBegin)
        {
            sal_uInt8* pEquData = new sal_uInt8[nEnd - nBegin + 1];
            for(sal_uInt32 nIndex = 0; nIndex < nEnd - nBegin +1 ; nIndex++)
            {
                pEquData[nIndex] = pGrafData[nBegin + nIndex];
            }
            pXFNotePara->Add(rtl::OUString((sal_Char*)pEquData, (nEnd - nBegin + 1), osl_getThreadTextEncoding()));
            delete [] pEquData;
        }
        pXFNote->Add(pXFNotePara);

        pXFPara->Add(pXFNote);
        pCont->Add(pXFPara);

        delete [] pGrafData;
        pGrafData = NULL;
    }

}

void LwpGraphicObject::GetGrafOrgSize(double & rWidth, double & rHeight)
{
    // original image size
    rWidth = (double)m_Cache.Width/TWIPS_PER_CM;
    rHeight = (double)m_Cache.Height/TWIPS_PER_CM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
