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
/*************************************************************************
 * Change History
 Feb 2005           Created
 ************************************************************************/
#include <stdio.h>
#include <tools/stream.hxx>
#include "lwpglobalmgr.hxx"
#include "lwpoleobject.hxx"
#include "lwpobjfactory.hxx"
#include "lwpidxmgr.hxx"
#include "lwp9reader.hxx"
#include "xfilter/xfoleobj.hxx"
#include "xfilter/xfparagraph.hxx"
#include "lwpframelayout.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "bento.hxx"

/**
 * @descr:   construction function
 * @param:  objHdr - object header, read before entering this function
 * @param: pStrm - file stream
 * @return:  None
 * @date:    2/22/2005
 */
LwpGraphicOleObject::LwpGraphicOleObject(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpContent(objHdr, pStrm)
{}
/**
 * @descr:   Read GraphicOleObject part
 * @param:  None
 * @return:  None
 * @date:    2/22/2005
 */
void LwpGraphicOleObject::Read()
{
    LwpContent::Read();

    if (LwpFileHeader::m_nFileRevision >= 0x000b)
    {
        // I'm not sure about the read method
        m_pNextObj.ReadIndexed(m_pObjStrm);
        m_pPrevObj.ReadIndexed(m_pObjStrm);
    }
    m_pObjStrm->SkipExtra();

}

void LwpGraphicOleObject::GetGrafOrgSize(double & rWidth, double & rHeight)
{
    rWidth = 0;
    rHeight = 0;
}

void LwpGraphicOleObject::GetGrafScaledSize(double & fWidth, double & fHeight)
{
    GetGrafOrgSize(fWidth, fHeight);
    // scaled image size
    double fSclGrafWidth = fWidth;//LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleWidth());
    double fSclGrafHeight = fHeight;//LwpTools::ConvertFromUnitsToMetric(pMyScale->GetScaleHeight());


    LwpVirtualLayout* pLayout = GetLayout(NULL);
    if (pLayout && pLayout->IsFrame())
    {
        LwpFrameLayout* pMyFrameLayout = static_cast<LwpFrameLayout*>(pLayout);
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
            double fFrameWidth = LwpTools::ConvertFromUnitsToMetric(pFrameGeo->GetWidth());
            double fFrameHeight = LwpTools::ConvertFromUnitsToMetric(pFrameGeo->GetHeight());

            // calculate the displayed size of the frame
            double fDisFrameWidth = fFrameWidth - (fLeftMargin+fRightMargin);
            double fDisFrameHeight = fFrameHeight - (fTopMargin+fBottomMargin);

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
                    if (fWidth/fHeight >= fDisFrameWidth/fDisFrameHeight)
                    {
                        fSclGrafWidth = fDisFrameWidth;
                        fSclGrafHeight = (fDisFrameWidth/fWidth) * fHeight;
                    }
                    else
                    {
                        fSclGrafHeight = fDisFrameHeight;
                        fSclGrafWidth = (fDisFrameHeight/fHeight) * fWidth;
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
    fWidth = fSclGrafWidth ;
    fHeight =  fSclGrafHeight ;

}

/**
 * @descr:   construction function
 * @param:  objHdr - object header, read before entering this function
 * @param: pStrm - file stream
 * @return:  None
 * @date:    2/22/2005
 */
LwpOleObject::LwpOleObject(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpGraphicOleObject(objHdr, pStrm),m_SizeRect(0,0,5,5)
{
}
/**
 * @descr:   Read VO_OLEOBJECT record
 * @param:  None
 * @return:  None
 * @date:    2/22/2005
 */
void LwpOleObject::Read()
{
    LwpGraphicOleObject::Read();

    cPersistentFlags = m_pObjStrm->QuickReaduInt16();

    sal_uInt16 nNonVersionedPersistentFlags = 0;
    sal_uInt16 nNumberOfPages = 0;
    // qCMarker read
    LwpObjectID ID;

    if (LwpFileHeader::m_nFileRevision >= 0x0004)
    {
        nNonVersionedPersistentFlags = m_pObjStrm->QuickReaduInt16();

        OUString sFormat = m_pObjStrm->QuickReadStringPtr();

        if (LwpFileHeader::m_nFileRevision < 0x000B)
        {
            // null pointers have a VO_INVALID type
            //if (VO_INVALID == m_pObjStrm->QuickReaduInt16())
            //  return;

            ID.Read(m_pObjStrm);
            //return m_pObjStrm->Locate(ID);
        }
        else
        {
            ID.ReadIndexed(m_pObjStrm);
            //if (ID.IsNull())
            //  return;

            //return m_pObjStrm->Locate(ID);
        }
    }

    if (m_pObjStrm->CheckExtra())
    {
        nNumberOfPages = m_pObjStrm->QuickReaduInt16();
        m_pObjStrm->SkipExtra();
    }

}
/**
 * @descr:   Construct ole-storage name by ObjectID
 * @param:  pObjName - input&output string of object name, spaces allocated outside and at least length should be MAX_STREAMORSTORAGENAME
 * @return:  None
 * @date:    2/22/2005
 */
void LwpOleObject::GetChildStorageName(char *pObjName)
{
    /*LwpObjectFactory * pObjMgr = LwpObjectFactory::Instance();
    LwpIndexManager * pIdxMgr = pObjMgr->GetIndexManager();
    sal_uInt32 nLowID = pIdxMgr->GetObjTime(static_cast<sal_uInt16>(GetObjectID()->GetLow()));*/

    char sName[MAX_STREAMORSTORAGENAME];
    //LwpObjectID ID(nLowID, GetObjectID()->GetHigh());
    sprintf( sName, "%s%X,%X", "Ole",
      GetObjectID()->GetHigh(), GetObjectID()->GetLow());

    strcpy( pObjName, sName);
    return;
}
/**
 * @descr:   Parse VO_OLEOBJECT and dump to XML stream only on WIN32 platform
 * @param:  pOutputStream - stream to dump OLE object
 * @param:  pFrameLayout -  framlayout object used to dump OLE object
 * @return:  None
 * @date:    2/22/2005
 */
void LwpOleObject::Parse(IXFStream* /*pOutputStream*/)
{
#if 0
#ifdef WIN32

    //
    // Construct OLE object storage name
    //
    char sObjectName[MAX_STREAMORSTORAGENAME];
    GetChildStorageName(sObjectName);
    String aObjName= String::CreateFromAscii(sObjectName);

    //
    // Get OLE objects information
    //
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    LwpObjectFactory* pObjMgr = pGlobal->GetLwpObjFactory();
    SvStorageRef objStor;
    SvStorageInfoList * pInfoList;
    pObjMgr->GetOleObjInfo(objStor, &pInfoList);

    if(pInfoList == NULL)
    {
        assert(sal_False);
        return;
    }

    //
    // Get ole object buffer
    //
    BYTE * pBuf = NULL;
    sal_uInt32 nSize = 0;
    for (sal_uInt32 j=0; j<pInfoList->Count(); j++)
    {

        SvStorageInfo& rInfo = pInfoList->GetObject(j);
        String aName = rInfo.GetName();

        if(aName == aObjName)
        {
            SvStorageRef childStor;
            childStor = objStor->OpenStorage(rInfo.GetName());
            SvStorage * aEleStor;
            aEleStor = objStor->OpenOLEStorage( rInfo.GetName() );
            SvInPlaceObjectRef xInplaceObj( ((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndLoad( childStor ) );
            SvOutPlaceObjectRef xOutplaceObj(xInplaceObj);
            aEleStor->SetVersion( SOFFICE_FILEFORMAT_60 );
            SvStream *pStream=xOutplaceObj->GetOLEObjectStream(aEleStor);

            //Get Ole original size
            m_SizeRect = xOutplaceObj->GetVisSize(xOutplaceObj->GetViewAspect());

            nSize = pStream->Seek( STREAM_SEEK_TO_END );
            pBuf = new BYTE[nSize];
            if (pBuf == NULL)
            {
                assert(sal_False);
                return;
            }
            pStream->Seek(0);
            pStream->Read(pBuf, nSize);

            delete pStream;
            break;
        }
    }

    //
    // dump the buffer by XFilter
    //
    if(pBuf != NULL)
    {
        // set ole buffer
        XFOleObject *pOleObj = new XFOleObject();
        pOleObj->SetOleData(pBuf, nSize);

        // set frame attributes
        pOleObj->SetAnchorType(enumXFAnchorPara);
        double fWidth = 0;
        double fHeight = 0;
        GetGrafScaledSize( fWidth,  fHeight);
        if(fWidth < 0.001 || fHeight < 0.001)
        {
            fWidth = 5.0;
            fHeight = 5.0;
        }
        pOleObj->SetWidth(fWidth);
        pOleObj->SetHeight(fHeight);
        /*
        pOleObj->SetName(A2OUSTR("TestOle"));
        pOleObj->SetX(5);
        pOleObj->SetY(5);
        pOleObj->SetWidth(5);
        pOleObj->SetHeight(5);
        */

        // To XML
        XFParagraph *pPara = new XFParagraph();
        pPara->Add(pOleObj);
        pPara->ToXml(pOutputStream);

        delete pPara;       // pOleObj will be deleted inside

#if 0
        // dump to file
        String aTempDir( SvtPathOptions().GetTempPath() );
        sal_Int32 nLength = aTempDir.Len();
        if ( aTempDir.GetChar(nLength-1 ) !=UChar32( '/' ) )
            aTempDir += String::CreateFromAscii("/");

        aTempDir += aObjName;
        SvFileStream aStream(aTempDir, STREAM_WRITE);
        aStream.Write(pBuf, nSize);
        aStream.Close();
#endif
        delete []pBuf;

        // dump attributes to
    }

    return;
#endif
#endif
}

void LwpOleObject::XFConvert(XFContentContainer * /*pCont*/)
{
#if 0
    //Add by  10/24/2005
    //Get ole object stream with the object name;

    // modified by
    // if small file, use the compressed stream for BENTO
    LwpSvStream* pStream = m_pStrm->GetCompressedStream() ?  m_pStrm->GetCompressedStream(): m_pStrm;
    // end modified by

    OpenStormBento::LtcBenContainer* pBentoContainer;
    ULONG ulRet = OpenStormBento::BenOpenContainer(pStream, &pBentoContainer);

    char sObjectName[MAX_STREAMORSTORAGENAME];
    GetChildStorageName(sObjectName);
    std::string aObjName(sObjectName);
    SotStorageStreamRef xOleObjStm = pBentoContainer->ConvertAswStorageToOLE2Stream(aObjName.c_str());

    //Get stream size and data
    if(!xOleObjStm.Is() || xOleObjStm->GetError())
        return;


    BYTE * pBuf = NULL;
    sal_uInt32 nSize = 0;

    SvStorageRef xOleObjStor = new SvStorage( *xOleObjStm );
    //SvStorageRef xOleObjStor = pBentoContainer->CreateOLEStorageWithObjectName(aObjName.c_str());
    if( !xOleObjStor.Is())
        return ;

    SvInPlaceObjectRef xInplaceObj( ((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndLoad( xOleObjStor ) );


    //when the OLE object is converted into native object.
//  SvOutPlaceObjectRef xOutplaceObj(xInplaceObj);
//  xOutplaceObj->SetVersion( SOFFICE_FILEFORMAT_60 );
//  SvStream *pOleStream=xOutplaceObj->GetOLEObjectStream(xOleObjStor);
    //Get Ole original size
    m_SizeRect = GetOLEObjectSize(xOleObjStor);
    //End by

    nSize = xOleObjStm->Seek( STREAM_SEEK_TO_END );
    pBuf = new BYTE[nSize];
    if (pBuf == NULL)
    {
        assert(sal_False);
        return;
    }
    xOleObjStm->Seek(0);
    xOleObjStm->Read(pBuf, nSize);

    //delete pOleStream;


    //End by

    //
    // dump the buffer by XFilter
    //
    if(pBuf != NULL)
    {
        // set ole buffer
        XFOleObject *pOleObj = new XFOleObject();
        pOleObj->SetOleData(pBuf, nSize);

        // set frame attributes
        pOleObj->SetAnchorType(enumXFAnchorFrame);
        pOleObj->SetStyleName( m_strStyleName);
        LwpFrameLayout* pMyFrameLayout = static_cast<LwpFrameLayout*>(GetLayout(NULL));
        if(pMyFrameLayout)
        {
            pOleObj->SetX(pMyFrameLayout->GetMarginsValue(MARGIN_LEFT));
            pOleObj->SetY(pMyFrameLayout->GetMarginsValue(MARGIN_TOP));
        }

        double fWidth = 0;
        double fHeight = 0;
        GetGrafScaledSize( fWidth,  fHeight);
        if(fWidth < 0.001 || fHeight < 0.001)
        {
            fWidth = 5.0;
            fHeight = 5.0;
        }


        pOleObj->SetWidth(fWidth);
        pOleObj->SetHeight(fHeight);


        //delete pPara;     // pOleObj will be deleted inside
        pCont->Add(pOleObj);

#if 0
        // dump to file
        String aTempDir( SvtPathOptions().GetTempPath() );
        sal_Int32 nLength = aTempDir.Len();
        if ( aTempDir.GetChar(nLength-1 ) !=UChar32( '/' ) )
            aTempDir += String::CreateFromAscii("/");

        aTempDir += aObjName;
        SvFileStream aStream(aTempDir, STREAM_WRITE);
        aStream.Write(pBuf, nSize);
        aStream.Close();
#endif
        delete []pBuf;

        // dump attributes to
    }
#endif
    return;
}

void LwpOleObject::GetGrafOrgSize(double & rWidth, double & rHeight)
{
    rWidth = (double)m_SizeRect.GetWidth()/1000;//cm unit
    rHeight = (double)m_SizeRect.GetHeight()/1000;//cm unit
}

void LwpOleObject::RegisterStyle()
{
#if 0
#ifdef WIN32
    LwpVirtualLayout* pMyLayout = GetLayout(NULL);
    if(pMyLayout->IsFrame())
    {
        XFFrameStyle* pXFFrameStyle = new XFFrameStyle();
        pXFFrameStyle->SetXPosType(enumXFFrameXPosFromLeft, enumXFFrameXRelFrame);
        pXFFrameStyle->SetYPosType(enumXFFrameYPosFromTop, enumXFFrameYRelPara);
        XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
        m_strStyleName = pXFStyleManager->AddStyle(pXFFrameStyle)->GetStyleName();
    }
#endif
#endif
}


#include <sot/exchange.hxx>
#include <sot/storinfo.hxx>
#include <svtools/wmf.hxx>
/**
* @descr:   For SODC_2667, To get the OLE object size by reading OLE object picture information.
*/
Rectangle LwpOleObject::GetOLEObjectSize( SotStorage * pStor ) const
{
    Rectangle aSize(0,0,0,0);
    String aStreamName;
    if( pStor->IsContained( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres000" ) ) ) )
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres000" ) );
    else if( pStor->IsContained( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) ) ) )
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) );

    if( aStreamName.Len() == 0 )
        return aSize;


    for( USHORT i = 1; i < 10; i++ )
    {
        SotStorageStreamRef xStm = pStor->OpenSotStream( aStreamName,
            STREAM_READ | STREAM_NOCREATE );
        if( xStm->GetError() )
            break;

        xStm->SetBufferSize( 8192 );
        LwpOlePres * pEle = new LwpOlePres( 0 );
        if( pEle->Read( *xStm ) && !xStm->GetError() )
        {
            if( pEle->GetFormat() == FORMAT_GDIMETAFILE || pEle->GetFormat() == FORMAT_BITMAP )
            {
                aSize = Rectangle( Point(), pEle->GetSize());
                delete pEle;
                break;
            }
        }
        delete pEle;
        pEle = NULL;
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres00" ) );
        aStreamName += String( i );
    };

    return aSize;
}
/**
* @descr:   Read OLE object picture information
*/
BOOL LwpOlePres::Read( SvStream & /*rStm*/ )
{
#if 0
    ULONG nBeginPos = rStm.Tell();
    INT32 n;
    rStm >> n;
    if( n != -1 )
    {
        pBmp = new Bitmap;
        rStm >> *pBmp;
        if( rStm.GetError() == SVSTREAM_OK )
        {
            nFormat = FORMAT_BITMAP;
            aSize = pBmp->GetPrefSize();
            MapMode aMMSrc;
            if( !aSize.Width() || !aSize.Height() )
            {
                // letzte Chance
                aSize = pBmp->GetSizePixel();
                aMMSrc = MAP_PIXEL;
            }
            else
                aMMSrc = pBmp->GetPrefMapMode();
            MapMode aMMDst( MAP_100TH_MM );
            aSize = OutputDevice::LogicToLogic( aSize, aMMSrc, aMMDst );
            return TRUE;
        }
        else
        {
            delete pBmp;
            pBmp = NULL;

            pMtf = new GDIMetaFile();
            rStm.ResetError();
            rStm >> *pMtf;
            if( rStm.GetError() == SVSTREAM_OK )
            {
                nFormat = FORMAT_GDIMETAFILE;
                aSize = pMtf->GetPrefSize();
                MapMode aMMSrc = pMtf->GetPrefMapMode();
                MapMode aMMDst( MAP_100TH_MM );
                aSize = OutputDevice::LogicToLogic( aSize, aMMSrc, aMMDst );
                return TRUE;
            }
            else
            {
                delete pMtf;
                pMtf = NULL;
            }
        }

    }

    rStm.ResetError();
    rStm.Seek( nBeginPos );
    nFormat = ReadClipboardFormat( rStm );
    // JobSetup, bzw. TargetDevice ueberlesen
    // Information aufnehmen, um sie beim Schreiben nicht zu verlieren
    nJobLen = 0;
    rStm >> nJobLen;
    if( nJobLen >= 4 )
    {
        nJobLen -= 4;
        if( nJobLen )
        {
            pJob = new BYTE[ nJobLen ];
            rStm.Read( pJob, nJobLen );
        }
    }
    else
    {
        rStm.SetError( SVSTREAM_GENERALERROR );
        return FALSE;
    }
    UINT32 nAsp;
    rStm >> nAsp;
    USHORT nSvAsp = USHORT( nAsp );
    SetAspect( nSvAsp );
    rStm.SeekRel( 4 ); //L-Index ueberlesen
    rStm >> nAdvFlags;
    rStm.SeekRel( 4 ); //Compression
    UINT32 nWidth  = 0;
    UINT32 nHeight = 0;
    UINT32 nSize   = 0;
    rStm >> nWidth >> nHeight >> nSize;
    aSize.Width() = nWidth;
    aSize.Height() = nHeight;

    if( nFormat == FORMAT_GDIMETAFILE )
    {
        pMtf = new GDIMetaFile();
        ReadWindowMetafile( rStm, *pMtf );
    }
    else if( nFormat == FORMAT_BITMAP )
    {
        pBmp = new Bitmap();
        rStm >> *pBmp;
    }
    else
    {
        void * p = new BYTE[ nSize ];
        rStm.Read( p, nSize );
        delete p;
        return FALSE;
    }
#endif
    return TRUE;
}

/**
* @descr:   Write OLE object picture information.
*/
void LwpOlePres::Write( SvStream & rStm )
{
    WriteClipboardFormat( rStm, FORMAT_GDIMETAFILE );
    rStm << (INT32)(nJobLen +4);       // immer leeres TargetDevice
    if( nJobLen )
        rStm.Write( pJob, nJobLen );
    rStm << (UINT32)nAspect;
    rStm << (INT32)-1;      //L-Index immer -1
    rStm << (INT32)nAdvFlags;
    rStm << (INT32)0;       //Compression
    rStm << (INT32)aSize.Width();
    rStm << (INT32)aSize.Height();
    ULONG nPos = rStm.Tell();
    rStm << (INT32)0;

    if( GetFormat() == FORMAT_GDIMETAFILE && pMtf )
    {
        // Immer auf 1/100 mm, bis Mtf-Loesung gefunden
        // Annahme (keine Skalierung, keine Org-Verschiebung)
        DBG_ASSERT( pMtf->GetPrefMapMode().GetScaleX() == Fraction( 1, 1 ),
            "X-Skalierung im Mtf" );
            DBG_ASSERT( pMtf->GetPrefMapMode().GetScaleY() == Fraction( 1, 1 ),
            "Y-Skalierung im Mtf" );
            DBG_ASSERT( pMtf->GetPrefMapMode().GetOrigin() == Point(),
            "Origin-Verschiebung im Mtf" );
            MapUnit nMU = pMtf->GetPrefMapMode().GetMapUnit();
        if( MAP_100TH_MM != nMU )
        {
            Size aPrefS( pMtf->GetPrefSize() );
            Size aS( aPrefS );
            aS = OutputDevice::LogicToLogic( aS, nMU, MAP_100TH_MM );

            pMtf->Scale( Fraction( aS.Width(), aPrefS.Width() ),
                Fraction( aS.Height(), aPrefS.Height() ) );
            pMtf->SetPrefMapMode( MAP_100TH_MM );
            pMtf->SetPrefSize( aS );
        }
        WriteWindowMetafileBits( rStm, *pMtf );
    }
    else
    {
        DBG_ERROR( "unknown format" );
    }
    ULONG nEndPos = rStm.Tell();
    rStm.Seek( nPos );
    rStm << (UINT32)(nEndPos - nPos - 4);
    rStm.Seek( nEndPos );
}
//End by

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
