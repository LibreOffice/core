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
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/

#ifndef LWPGRFOBJECT_HXX_
#define LWPGRFOBJECT_HXX_

#include "lwpoleobject.hxx"
#include "lwpheader.hxx"
#include "xfilter/xfrect.hxx"
#include <tools/gen.hxx>

struct ImageProcessingData
{
    sal_uInt8 nBrightness;
    sal_uInt8 nContrast;
    sal_uInt8 nEdgeEnchancement;
    sal_uInt8 nSmoothing;
    sal_Bool bAutoContrast;
    sal_Bool bInvertImage;

    ImageProcessingData()
    {
        nBrightness = 50;
        nContrast = 50;
        nEdgeEnchancement = 0;
        nSmoothing = 0;
        bAutoContrast = sal_False;
        bInvertImage= sal_False;
    }
};

class XFFrame;
class LwpGraphicObject : public LwpGraphicOleObject
{
public:
    LwpGraphicObject(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpGraphicObject();
private:
    unsigned char m_sDataFormat[AFID_MAX_FILE_FORMAT_SIZE];
    unsigned char m_sServerContextFormat[AFID_MAX_CONTEXT_FORMAT_SIZE];
    sal_Int32 m_nCachedBaseLine;
    sal_Int16 m_bIsLinked;
    AFID_CACHE m_Cache;
    sal_Int16 m_bCompressed;
    OUString m_WatermarkName;
    OUString m_LinkedFilePath;

    ImageProcessingData m_aIPData;

    void GetRect(INT32& nLeft, INT32& nTop, INT32& nRight, INT32& nBottom);
    XFRect GetRectInCM();
    Rectangle GetRectIn100thMM();
    void XFConvertEquation(XFContentContainer* pCont);

    void ParseChart(IXFStream* pOutputStream);
    sal_Bool IsGrafFormatValid();
    // add by , 03/25/2005
    std::vector <XFFrame*> m_vXFDrawObjects;
    // end add

public:
    //inline sal_uInt8* GetBuffer(){return m_DataBuffer;}
    void Read();
    //Added by  for XFConvert refactor, 03/31/2005
    virtual void XFConvert (XFContentContainer* pCont);
    //End of Add
    void RegisterStyle();

    // add by , 03/25/2005
    void CreateDrawObjects();
    void CreateGrafObject();
    void GetBentoNamebyID(LwpObjectID* pMyID, std::string& rName);
    sal_uInt32 GetRawGrafData(sal_uInt8*& pGrafData);
    sal_uInt32 GetGrafData(sal_uInt8*& pGrafData);
    void GetGrafOrgSize(long& rWidth, long& rHeight) { rWidth = m_Cache.Width; rHeight = m_Cache.Height; }
    void GetGrafOrgSize(double& rWidth, double& rHeight);
    // end add

    OUString GetWatermarkName(){ return m_WatermarkName;}
    sal_Int16 IsLinked(){ return m_bIsLinked;}
    OUString GetLinkedFilePath(){ return m_LinkedFilePath;}
};

#endif
