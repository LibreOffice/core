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
* <<Date>> <<Name of editor>> <<Description>>
2005/2		draft code for implementation of chart stream helpers
****************************************************************************/

/**
 * @file
 *  For LWP filter architecture prototype
*/

#include "lwpchartstreamtools.hxx"
#include "xfilter/xfdrawchart.hxx"
#include "xfilter/xfparagraph.hxx"

#include <com/sun/star/xml/sax/SAXParseException.hpp>

/**
* @short   Get the chart stream from bento layer
* @descr
* @param   pDocStream	the document stream
* @param   pChartName	The name of the requested chart stream
* @return  pStream		The chart stream, may be  NULL
*/
SvStream* LwpChartStreamTools::GetChartStream(LwpSvStream* pDocStream, const char* pChartName)
{
    // if small file, use the compressed stream for BENTO
    LwpSvStream *pSvStream = pDocStream->GetCompressedStream() ?  pDocStream->GetCompressedStream(): pDocStream;

    SvStream* pStream = NULL;
    pSvStream->Seek(0);
    OpenStormBento::LtcBenContainer* pContainer = NULL;;
    ULONG nRet = OpenStormBento::BenOpenContainer(pSvStream,&pContainer);
    if ( 0==nRet )
    {
        pStream = pContainer->FindValueStreamWithPropertyName(pChartName);
    }
    return pStream;
}

/**
* @short   Output the chart into output stream
* @param   aChartRect	the rectangle of the chart
* @param   pXChartStrm	Chart stream
* @param   pOutputStream the output stream
*/
void LwpChartStreamTools::OutputChart(XFRect aChartRect, IXFContent* pXChartStrm, IXFStream* pOutputStream)
{
    XFDrawChart* pXChart = new XFDrawChart;
    pXChart->SetPosition(aChartRect);
    pXChart->Add(pXChartStrm);

    //write to xml
    XFParagraph xpara;
    xpara.Add(pXChart);
    xpara.ToXml(pOutputStream);
}

void LwpChartStreamTools::OutputChart(XFRect aChartRect, IXFContent* pXChartStrm, XFContentContainer* pCont, OUString strStyleName)
{
    XFDrawChart* pXChart = new XFDrawChart;
    pXChart->SetStyleName( strStyleName);
    pXChart->SetPosition(aChartRect);
    pXChart->SetAnchorType(enumXFAnchorFrame);
    pXChart->Add(pXChartStrm);

    pCont->Add(pXChart);

    //write to xml
    /*XFParagraph*  pPara = new XFParagraph();
    pPara->Add(pXChart);
    pCont->Add(pPara);
    */
}

/**
* @short   Get the stream length
* @param   pStream	the stream
* @param   the stream length
*/
INT32 LwpChartStreamTools::GetStreamLen(SvStream* pStream)
{
    UINT32 nPos = pStream->Tell();
    pStream->Seek(STREAM_SEEK_TO_END);
    INT32 nStrmLen = pStream->Tell();
    pStream->Seek(nPos);
    return nStrmLen;
}

/**
* @short   Make the chart storage object
* @param   pStream	the chart stream
* @param   aChartRect	the rectangle of the chart
* @param   xIPObj	[out]the inplace object
* @param   xStorage	[out]the storage object of the chart
*/
#include "lwpchartwrapper.hxx"
void LwpChartStreamTools::MakeChartStorage( SvStream* pStream, Rectangle aChartRect, SvInPlaceObjectRef& xIPObj, SvStorageRef& xStorage)
{
    //			SfxItemSet aItemSet( rDoc.GetAttrPool()); //Style of chart area
    INT32 nStrmLen = GetStreamLen(pStream);
    LtcLwpChartWrapper aChart(pStream,nStrmLen);
    aChart.CreateChart(aChartRect,xIPObj,xStorage);
    aChart.Read();
}

/**
* @short   Translate the storage to XML stream
* @param   xStorage	[out]the storage object of a chart
* @param   xIPObj	[in]the inplace object
*/
#include "schxmlwrapper.hxx"
void LwpChartStreamTools::ChartToXML(SvStorageRef xStorage, SvInPlaceObjectRef xIPObj)
{
    xStorage->SetVersion( SOFFICE_FILEFORMAT_60 );

    SfxInPlaceObjectRef aSfxObj( xIPObj );
    SfxObjectShell* pSh = aSfxObj->GetObjectShell();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel = pSh->GetModel();

    SchXMLWrapper aFilter( xModel, *xStorage, sal_False );

    // update user info before writing
    pSh->UpdateDocInfoForSave();

    aFilter.Export();
}

//void LwpChartStreamTools::ChartToXML(SvStorageRef xStorage, SvInPlaceObjectRef xIPObj)
//{
//	xStorage->SetVersion( SOFFICE_FILEFORMAT_60 );
//
//	SfxInPlaceObjectRef aSfxObj( xIPObj );
//	SfxObjectShell* pSh = aSfxObj->GetObjectShell();
//	pSh->Save();
//}

/**
* @short	Get the xml stream from the storage
* @param	xStorage	the storage object
* @param	aStrmName	the stream name
*/
SvStorageStreamRef LwpChartStreamTools::GetStream(SvStorageRef xStorage, String aStrmName)
{
    SvStorageStreamRef rStream;
    if( ! xStorage->IsStream( aStrmName) )
    {
        assert(false);
    }
    else
    {
        rStream = xStorage->OpenStream( aStrmName, STREAM_READ | STREAM_NOCREATE );
    }
    return rStream;
}

/**
* @short	Dump the xml stream into a file
*/
void LwpChartStreamTools::DumpStream(SvStorageStreamRef rStream, char* aName)
{
    INT32 nLen = GetStreamLen(&rStream);
    char* pBuf = new char[nLen];
    rStream->Read(pBuf, nLen);
    std::ofstream aFile(aName, ios_base::out);
    aFile.write(pBuf,nLen);
    rStream->Seek(0);
}

/**
* @short	Get the name of chart stream
* @param	pID	the id of the VO_Graphic object, which map to a chart stream.
* @return	the name of the chart stream
*/
const char* LwpChartStreamTools::GetChartName(LwpObjectID* pID)
{
#define MAX_STREAMORSTORAGENAME 32 //The value is got from wordpro source
    static char pChartName[MAX_STREAMORSTORAGENAME];

    char aGrStrmPrefix[] = "Gr";
    char aDataStrmSuffix[] = "-D";

    sprintf( (char*)pChartName,"%s%lX,%lX%s", aGrStrmPrefix,pID->GetHigh(),
        pID->GetLow(),aDataStrmSuffix);

    return pChartName;
}

/**
* @short   Parse the chart stream in Bento layer.
* @descr
* @param   pOutputStream the pointer to IXFStream, which output the xml content.
* @return
* @todo any error code return are needed?
*/
void LwpChartStreamTools::ParseChart(	LwpSvStream* pDocStream,
                                        LwpObjectID* pID,
                                        Rectangle aRectIn100thMM,
                                        XFRect aRectInCM,
                                        IXFStream* pOutputStream)
{
    XFChartStream* pXFChartStrm = new XFChartStream;

    try
    {
        //GetChartName
        const char* pChartName = GetChartName(pID);

        //Get chart stream
        SvStream* pStream = GetChartStream( pDocStream, pChartName );
        if (!pStream)
        {
            assert(false);
            return;
        }

        SvInPlaceObjectRef xIPObj;
        SvStorageRef xStorage;

        MakeChartStorage( pStream, aRectIn100thMM, xIPObj, xStorage);
        ChartToXML(xStorage, xIPObj);
//		SvStorageStreamRef rStyleStream = GetStream(xStorage,String::CreateFromAscii( "styles.xml" ));
        SvStorageStreamRef rCntStream = GetStream(xStorage,String::CreateFromAscii( "content.xml" ));
//		DumpStream(rCntStream,"d:\\allinone.xml");
        if (rCntStream.Is())
        {
            pXFChartStrm->SetChartStream(rCntStream);
        }

        OutputChart( aRectInCM, pXFChartStrm, pOutputStream);

        delete pStream;
    }
    catch (com::sun::star::xml::sax::SAXParseException&) {
        assert(false);
    }
    catch( com::sun::star::xml::sax::SAXException& )
    {
        assert(false);
    }
    catch( com::sun::star::io::IOException&)
    {
        assert(false);
    }
    catch( com::sun::star::uno::Exception&)
    {
        assert(false);
    }
}

void LwpChartStreamTools::ParseChart(	LwpSvStream* pDocStream,
                                        LwpObjectID* pID,
                                        Rectangle aRectIn100thMM,
                                        XFRect aRectInCM,
                                        XFContentContainer* pCont,
                                        OUString strStyleName)
{
    XFChartStream* pXFChartStrm = new XFChartStream;

    try
    {
        //GetChartName
        const char* pChartName = GetChartName(pID);

        //Get chart stream
        SvStream* pStream = GetChartStream( pDocStream, pChartName );
        if (!pStream)
        {
            assert(false);
            return;
        }

        SvInPlaceObjectRef xIPObj;
        SvStorageRef xStorage;

        MakeChartStorage( pStream, aRectIn100thMM, xIPObj, xStorage);
        ChartToXML(xStorage, xIPObj);
//		SvStorageStreamRef rStyleStream = GetStream(xStorage,String::CreateFromAscii( "styles.xml" ));
        SvStorageStreamRef rCntStream = GetStream(xStorage,String::CreateFromAscii( "content.xml" ));
//		DumpStream(rCntStream,"d:\\allinone.xml");
        if (rCntStream.Is())
        {
            pXFChartStrm->SetChartStream(rCntStream);
        }

        OutputChart( aRectInCM, pXFChartStrm, pCont, strStyleName);

        delete pStream;
    }
    catch (com::sun::star::xml::sax::SAXParseException& ) {
        assert(false);
    }
    catch( com::sun::star::xml::sax::SAXException& )
    {
        assert(false);
    }
    catch( com::sun::star::io::IOException&)
    {
        assert(false);
    }
    catch( com::sun::star::uno::Exception&)
    {
        assert(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
