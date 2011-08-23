/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "GraphicImport.hxx"
#include "GraphicHelpers.hxx"

#include <dmapper/DomainMapper.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>

#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <rtl/ustrbuf.hxx>


#include <iostream>
#include <resourcemodel/QNameToString.hxx>
#include <string.h>

#ifdef DEBUG_DOMAINMAPPER
#include <resourcemodel/TagLogger.hxx>
#endif

namespace writerfilter {
namespace dmapper
{
using namespace ::std;
using namespace ::com::sun::star;

#ifdef DEBUG_DOMAINMAPPER
extern TagLogger::Pointer_t dmapper_logger;
#endif
    
class XInputStreamHelper : public cppu::WeakImplHelper1
<    io::XInputStream   >
{
    const sal_uInt8* m_pBuffer;
    const sal_Int32  m_nLength;
    sal_Int32        m_nPosition;
    bool             m_bBmp;

    const sal_uInt8* m_pBMPHeader; //default BMP-header
    sal_Int32        m_nHeaderLength;
public:
    XInputStreamHelper(const sal_uInt8* buf, size_t len, bool bBmp);
    ~XInputStreamHelper();

    virtual ::sal_Int32 SAL_CALL readBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL available(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException);
};
/*-- 01.11.2006 13:56:20---------------------------------------------------

  -----------------------------------------------------------------------*/
XInputStreamHelper::XInputStreamHelper(const sal_uInt8* buf, size_t len, bool bBmp) :
        m_pBuffer( buf ),
        m_nLength( len ),
        m_nPosition( 0 ),
        m_bBmp( bBmp )
{
    static const sal_uInt8 aHeader[] =
        {0x42, 0x4d, 0xe6, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
    m_pBMPHeader = aHeader;
    m_nHeaderLength = m_bBmp ? sizeof( aHeader ) / sizeof(sal_uInt8) : 0;

}
/*-- 01.11.2006 13:56:20---------------------------------------------------

  -----------------------------------------------------------------------*/
XInputStreamHelper::~XInputStreamHelper()
{
}
/*-- 01.11.2006 13:56:21---------------------------------------------------

  -----------------------------------------------------------------------*/
::sal_Int32 XInputStreamHelper::readBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    return readSomeBytes( aData, nBytesToRead );
}
/*-- 01.11.2006 13:56:21---------------------------------------------------

  -----------------------------------------------------------------------*/
::sal_Int32 XInputStreamHelper::readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    sal_Int32 nRet = 0;
    if( nMaxBytesToRead > 0 )
    {
        if( nMaxBytesToRead > (m_nLength + m_nHeaderLength) - m_nPosition )
            nRet = (m_nLength + m_nHeaderLength) - m_nPosition;
        else
            nRet = nMaxBytesToRead;
        aData.realloc( nRet );
        sal_Int8* pData = aData.getArray();
        sal_Int32 nHeaderRead = 0;
        if( m_nPosition < m_nHeaderLength)
        {
            //copy header content first
            nHeaderRead = m_nHeaderLength - m_nPosition;
            memcpy( pData, m_pBMPHeader + (m_nPosition ), nHeaderRead );
            nRet -= nHeaderRead;
            m_nPosition += nHeaderRead;
        }
        if( nRet )
        {
            memcpy( pData + nHeaderRead, m_pBuffer + (m_nPosition - m_nHeaderLength), nRet );
            m_nPosition += nRet;
        }
    }
    return nRet;
}
/*-- 01.11.2006 13:56:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void XInputStreamHelper::skipBytes( ::sal_Int32 nBytesToSkip ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    if( nBytesToSkip < 0 || m_nPosition + nBytesToSkip > (m_nLength + m_nHeaderLength))
        throw io::BufferSizeExceededException();
    m_nPosition += nBytesToSkip;
}
/*-- 01.11.2006 13:56:22---------------------------------------------------

  -----------------------------------------------------------------------*/
::sal_Int32 XInputStreamHelper::available(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    return ( m_nLength + m_nHeaderLength ) - m_nPosition;
}
/*-- 01.11.2006 13:56:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void XInputStreamHelper::closeInput(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
}
/*-- 02.11.2006 09:34:29---------------------------------------------------

 -----------------------------------------------------------------------*/
struct GraphicBorderLine
{
    sal_Int32   nLineWidth;
//    sal_Int32   nLineType;
    sal_Int32   nLineColor;
    sal_Int32   nLineDistance;
    bool        bHasShadow;

    GraphicBorderLine() :
        nLineWidth(0)
//        ,nLineType(0)
        ,nLineColor(0)
        ,nLineDistance(0)
        ,bHasShadow(false)
        {}

};

class GraphicImport_Impl
{
private:
    sal_Int32 nXSize;
    bool      bXSizeValid;
    sal_Int32 nYSize;
    bool      bYSizeValid;

public:
    GraphicImportType eGraphicImportType;
    DomainMapper&   rDomainMapper;

    sal_Int32 nHoriScaling;
    sal_Int32 nVertScaling;
    sal_Int32 nLeftPosition;
    sal_Int32 nTopPosition;
    sal_Int32 nRightPosition;
    sal_Int32 nBottomPosition;
    sal_Int32 nLeftCrop;
    sal_Int32 nTopCrop;
    sal_Int32 nRightCrop;
    sal_Int32 nBottomCrop;

    bool      bUseSimplePos;

    sal_Int16 nHoriOrient;
    sal_Int16 nHoriRelation;
    bool      bPageToggle;
    sal_Int16 nVertOrient;
    sal_Int16 nVertRelation;
    sal_Int32 nWrap;
    bool      bOpaque;
    bool      bContour;
    bool      bIgnoreWRK;

    sal_Int32 nLeftMargin;
    sal_Int32 nRightMargin;
    sal_Int32 nTopMargin;
    sal_Int32 nBottomMargin;

    sal_Int32 nContrast;
    sal_Int32 nBrightness;
    double    fGamma;

    sal_Int32 nFillColor;

    drawing::ColorMode eColorMode;

    GraphicBorderLine   aBorders[4];
    sal_Int32           nCurrentBorderLine;

    sal_Int32       nDffType;
    bool            bIsGraphic;
    bool            bIsBitmap;
    bool            bIsTiff;
    sal_Int32       nBitsPerPixel;

    bool            bHoriFlip;
    bool            bVertFlip;

    bool            bSizeProtected;
    bool            bPositionProtected;

    bool            bInShapeOptionMode;
    sal_Int32       nShapeOptionType;

    ::rtl::OUString sName;
    ::rtl::OUString sAlternativeText;

    GraphicImport_Impl(GraphicImportType eImportType, DomainMapper&   rDMapper) :
        nXSize(0)
        ,bXSizeValid(false)
        ,nYSize(0)
        ,bYSizeValid(false)
        ,eGraphicImportType( eImportType )
        ,rDomainMapper( rDMapper )
        ,nHoriScaling(0)
        ,nVertScaling(0)
        ,nLeftPosition(0)
        ,nTopPosition(0)
        ,nRightPosition(0)
        ,nBottomPosition(0)
        ,nLeftCrop(0)
        ,nTopCrop (0)
        ,nRightCrop (0)
        ,nBottomCrop(0)
        ,bUseSimplePos(false)
        ,nHoriOrient(   text::HoriOrientation::NONE )
        ,nHoriRelation( text::RelOrientation::FRAME )
        ,bPageToggle( false )
        ,nVertOrient(  text::VertOrientation::NONE )
        ,nVertRelation( text::RelOrientation::FRAME )
        ,nWrap(0)
        ,bOpaque( true )
        ,bContour(false)
        ,bIgnoreWRK(true)
        ,nLeftMargin(319)
        ,nRightMargin(319)
        ,nTopMargin(0)
        ,nBottomMargin(0)
        ,nContrast(0)
        ,nBrightness(0)
        ,fGamma( -1.0 )
        ,nFillColor( 0xffffffff )
        ,eColorMode( drawing::ColorMode_STANDARD )
        ,nCurrentBorderLine(BORDER_TOP)
        ,nDffType( 0 )
        ,bIsGraphic(false)
        ,bIsBitmap(false)
        ,bIsTiff(false)
        ,nBitsPerPixel(0)
        ,bHoriFlip(false)
        ,bVertFlip(false)
        ,bSizeProtected(false)
        ,bPositionProtected(false)
        ,bInShapeOptionMode(false)
        {}

    void setXSize(sal_Int32 _nXSize) 
    {
        nXSize = _nXSize;
        bXSizeValid = true;
    }
    
    sal_uInt32 getXSize() const 
    {
        return nXSize;
    }

    bool isXSizeValid() const
    {
        return bXSizeValid;
    }

    void setYSize(sal_Int32 _nYSize)
    {
        nYSize = _nYSize;
        bYSizeValid = true;
    }

    sal_uInt32 getYSize() const 
    {
        return nYSize;
    }

    bool isYSizeValis () const
    {
        return bYSizeValid;
    }
};
/*-- 01.11.2006 09:42:42---------------------------------------------------

  -----------------------------------------------------------------------*/
GraphicImport::GraphicImport(uno::Reference < uno::XComponentContext >    xComponentContext,
                             uno::Reference< lang::XMultiServiceFactory > xTextFactory,
                             DomainMapper& rDMapper, 
                             GraphicImportType eImportType )
: m_pImpl( new GraphicImport_Impl( eImportType, rDMapper ))
  ,m_xComponentContext( xComponentContext )
  ,m_xTextFactory( xTextFactory)
{
}
/*-- 01.11.2006 09:42:42---------------------------------------------------

  -----------------------------------------------------------------------*/
GraphicImport::~GraphicImport()
{
    delete m_pImpl;
}
/*-- 01.11.2006 09:45:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::attribute(Id nName, Value & val)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("attribute");
    dmapper_logger->attribute("name", (*QNameToString::Instance())(nName));
#endif
    sal_Int32 nIntValue = val.getInt();
    /* WRITERFILTERSTATUS: table: PICFattribute */
    switch( nName )
    {
        case NS_rtf::LN_LCB: break;//byte count
        case NS_rtf::LN_CBHEADER: break;//ignored
        case NS_rtf::LN_MFP: //MetafilePict
        case NS_rtf::LN_DffRecord: //dff record - expands to an sprm which expands to ...
        case NS_rtf::LN_shpopt: //shape options
        case NS_rtf::LN_shpfbse: //BLIP store entry
        case NS_rtf::LN_BRCTOP: //top border
        case NS_rtf::LN_BRCLEFT: //left border
        case NS_rtf::LN_BRCBOTTOM: //bottom border
        case NS_rtf::LN_BRCRIGHT: //right border
        case NS_rtf::LN_shape: //shape
        case NS_rtf::LN_blip: //the binary graphic data in a shape
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            {
                switch(nName)
                {
                    case NS_rtf::LN_BRCTOP: //top border
                        /* WRITERFILTERSTATUS: */
                        m_pImpl->nCurrentBorderLine = BORDER_TOP;
                    break;
                    case NS_rtf::LN_BRCLEFT: //left border
                        /* WRITERFILTERSTATUS: */
                        m_pImpl->nCurrentBorderLine = BORDER_LEFT;
                    break;
                    case NS_rtf::LN_BRCBOTTOM: //bottom border
                        /* WRITERFILTERSTATUS: */
                        m_pImpl->nCurrentBorderLine = BORDER_BOTTOM;
                    break;
                    case NS_rtf::LN_BRCRIGHT: //right border
                        /* WRITERFILTERSTATUS: */
                        m_pImpl->nCurrentBorderLine = BORDER_RIGHT;
                    break;
                    case NS_rtf::LN_shpopt:
                        /* WRITERFILTERSTATUS: */
                        m_pImpl->bInShapeOptionMode = true;
                    break;
                    default:;
                }
            writerfilter::Reference<Properties>::Pointer_t pProperties = val.getProperties();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
            }
                switch(nName)
                {
                    case NS_rtf::LN_shpopt:
                        /* WRITERFILTERSTATUS: */
                        m_pImpl->bInShapeOptionMode = false;
                    break;
                    default:;
                }
        }
        break;
        case NS_rtf::LN_payload :
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            writerfilter::Reference<BinaryObj>::Pointer_t pPictureData = val.getBinary();
            if( pPictureData.get())
                pPictureData->resolve(*this);
        }
        break;
        case NS_rtf::LN_BM_RCWINMF: //windows bitmap structure - if it's a bitmap
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;
        case NS_rtf::LN_DXAGOAL: //x-size in twip
        case NS_rtf::LN_DYAGOAL: //y-size in twip
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_MX: 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nHoriScaling = nIntValue; 
            break;// hori scaling in 0.001%
        case NS_rtf::LN_MY: 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nVertScaling = nIntValue; 
            break;// vert scaling in 0.001%
        case NS_rtf::LN_DXACROPLEFT:    
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nLeftCrop  = ConversionHelper::convertTwipToMM100(nIntValue); 
            break;// left crop in twips
        case NS_rtf::LN_DYACROPTOP:     
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nTopCrop   = ConversionHelper::convertTwipToMM100(nIntValue); 
            break;// top crop in twips
        case NS_rtf::LN_DXACROPRIGHT:   
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nRightCrop = ConversionHelper::convertTwipToMM100(nIntValue); 
            break;// right crop in twips
        case NS_rtf::LN_DYACROPBOTTOM:  
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nBottomCrop = ConversionHelper::convertTwipToMM100(nIntValue); 
            break;// bottom crop in twips
        case NS_rtf::LN_BRCL:           
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;//border type - legacy -
        case NS_rtf::LN_FFRAMEEMPTY:    
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// picture consists of a single frame
        case NS_rtf::LN_FBITMAP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            m_pImpl->bIsBitmap = nIntValue > 0 ? true : false;
        break;//1 if it's a bitmap ???
        case NS_rtf::LN_FDRAWHATCH:     
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;//1 if it's an active OLE object
        case NS_rtf::LN_FERROR:         
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// 1 if picture is an error message
        case NS_rtf::LN_BPP: 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nBitsPerPixel = nIntValue; 
            break;//bits per pixel 0 - unknown, 1- mono, 4 - VGA

        case NS_rtf::LN_DXAORIGIN: //horizontal offset of hand annotation origin
        case NS_rtf::LN_DYAORIGIN: //vertical offset of hand annotation origin
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        break;
        case NS_rtf::LN_CPROPS:break;// unknown - ignored
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        //metafilepict
        case NS_rtf::LN_MM:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
//      according to the documentation 99 or 98 are provided - but they are not!
//            m_pImpl->bIsBitmap = 99 == nIntValue ? true : false;
//            m_pImpl->bIsTiff = 98 == nIntValue ? true : false;

        break; //mapmode
        case NS_rtf::LN_XEXT: 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->setXSize(nIntValue);
            break; // x-size
        case NS_rtf::LN_YEXT: 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->setYSize(nIntValue);
            break; // y-size
        case NS_rtf::LN_HMF: break; //identifier - ignored
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */

        //sprm 0xf004 and 0xf008, 0xf00b
        case NS_rtf::LN_dfftype://
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            m_pImpl->nDffType = nIntValue;
        break;
        case NS_rtf::LN_dffinstance:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //todo: does this still work for PICF?
            //in case of LN_dfftype == 0xf01f the instance contains the bitmap type:
            if(m_pImpl->nDffType == 0xf01f)
                switch( nIntValue )
                {
                    case 0x216 :            // Metafile header then compressed WMF

                    case 0x3D4 :           // Metafile header then compressed EMF

                    case 0x542 :            // Metafile hd. then compressed PICT

                    {

//                        rBLIPStream.SeekRel( nSkip + 20 );
//                        // read in size of metafile in EMUS
//                        rBLIPStream >> aMtfSize100.Width() >> aMtfSize100.Height();
//                        // scale to 1/100mm
//                        aMtfSize100.Width() /= 360, aMtfSize100.Height() /= 360;
//                        if ( pVisArea )     // seem that we currently are skipping the visarea position
//                            *pVisArea = Rectangle( Point(), aMtfSize100 );
//                        // skip rest of header
//                        nSkip = 6;
//                        bMtfBLIP = bZCodecCompression = TRUE;
                    }

                    break;

                    case 0x46A :            break;// One byte tag then JPEG (= JFIF) data

                    case 0x6E0 :            break;// One byte tag then PNG data

                    case 0x7A8 : m_pImpl->bIsBitmap = true;
//                        nSkip += 1;         // One byte tag then DIB data
                    break;

                }
        break;
        case NS_rtf::LN_dffversion://  ignored
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        break;

        //sprm 0xf008
        case NS_rtf::LN_shptype:        
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpid:          
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpfGroup:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// This shape is a group shape
        case NS_rtf::LN_shpfChild:      
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// Not a top-level shape
        case NS_rtf::LN_shpfPatriarch:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// This is the topmost group shape. Exactly one of these per drawing.
        case NS_rtf::LN_shpfDeleted:    
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// The shape has been deleted
        case NS_rtf::LN_shpfOleShape:   
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// The shape is an OLE object
        case NS_rtf::LN_shpfHaveMaster: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// Shape has a hspMaster property
        case NS_rtf::LN_shpfFlipH:       // Shape is flipped horizontally
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->bHoriFlip = nIntValue ? true : false;
        break;
        case NS_rtf::LN_shpfFlipV:       // Shape is flipped vertically
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->bVertFlip = nIntValue ? true : false;
        break;
        case NS_rtf::LN_shpfConnector:   
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// Connector type of shape
        case NS_rtf::LN_shpfHaveAnchor:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// Shape has an anchor of some kind
        case NS_rtf::LN_shpfBackground:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// Background shape
        case NS_rtf::LN_shpfHaveSpt:     
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// Shape has a shape type property
        case NS_rtf::LN_shptypename: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;// shape type name
        case NS_rtf::LN_shppid:     
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nShapeOptionType = nIntValue; 
            break; //type of shape option
        case NS_rtf::LN_shpfBid:    
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            break; //ignored
        case NS_rtf::LN_shpfComplex:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpop:
            /* WRITERFILTERSTATUS: done: 50, planned: 10, spent: 5 */
        {
            if(NS_dff::LN_shpwzDescription != sal::static_int_cast<Id>(m_pImpl->nShapeOptionType) )
                ProcessShapeOptions( val );
        }
        break;
        case NS_rtf::LN_shpname:    
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpvalue:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        {
            if( NS_dff::LN_shpwzDescription == sal::static_int_cast<Id>(m_pImpl->nShapeOptionType) )
                ProcessShapeOptions( val );
        }
        break;

        //BLIP store entry
        case NS_rtf::LN_shpbtWin32: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpbtMacOS: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shprgbUid:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shptag:     
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpsize:    
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpcRef:    
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpfoDelay: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpusage:   
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpcbName:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpunused2: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_shpunused3: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;

        //border properties
        case NS_rtf::LN_shpblipbname : 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        break;

        case NS_rtf::LN_DPTLINEWIDTH:  // 0x1759
        /* WRITERFILTERSTATUS: done: 100, planned: 1, spent: 1 */
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineWidth = nIntValue;
        break;
        case NS_rtf::LN_BRCTYPE:   // 0x175a
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //graphic borders don't support different line types
            //m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineType = nIntValue;
        break;
        case NS_rtf::LN_ICO:   // 0x175b
        /* WRITERFILTERSTATUS: done: 100, planned: 1, spent: 1 */
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineColor = ConversionHelper::ConvertColor( nIntValue );
        break;
        case NS_rtf::LN_DPTSPACE:  // 0x175c
        /* WRITERFILTERSTATUS: done: 100, planned: 1, spent: 1 */
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineDistance = nIntValue;
        break;
        case NS_rtf::LN_FSHADOW:   // 0x175d
        /* WRITERFILTERSTATUS: done: 0, planned: 1, spent: 0 */
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].bHasShadow = nIntValue ? true : false;
        break;
        case NS_rtf::LN_FFRAME:            // ignored
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_UNUSED2_15: // ignored
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            break;

        case NS_rtf::LN_SPID: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;
        case NS_rtf::LN_XALEFT: 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nLeftPosition = ConversionHelper::convertTwipToMM100(nIntValue); 
            break; //left position
        case NS_rtf::LN_YATOP:  
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nTopPosition = ConversionHelper::convertTwipToMM100(nIntValue); 
            break; //top position
        case NS_rtf::LN_XARIGHT:  
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nRightPosition = ConversionHelper::convertTwipToMM100(nIntValue); 
            break; //right position
        case NS_rtf::LN_YABOTTOM: 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nBottomPosition = ConversionHelper::convertTwipToMM100(nIntValue); 
            break;//bottom position
        case NS_rtf::LN_FHDR:
        case NS_rtf::LN_XAlign:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
/*
        static const SwHoriOrient aHoriOriTab[ nCntXAlign ] =
        {
            HORI_NONE,     // From left position
            HORI_LEFT,     // left
            HORI_CENTER,   // centered
            HORI_RIGHT,    // right
            // --> OD 2004-12-06 #i36649#
            // - inside -> HORI_LEFT and outside -> HORI_RIGHT
            HORI_LEFT,   // inside
            HORI_RIGHT   // outside
*/
            if( nIntValue < 6 && nIntValue > 0 )
            {
                static const sal_Int16 aHoriOrientTab[ 6 ] =
                {
                    text::HoriOrientation::NONE,
                    text::HoriOrientation::LEFT,
                    text::HoriOrientation::CENTER,
                    text::HoriOrientation::RIGHT,
                    text::HoriOrientation::INSIDE,
                    text::HoriOrientation::OUTSIDE
                };
                m_pImpl->nHoriOrient = aHoriOrientTab[nIntValue];
                m_pImpl->bPageToggle = nIntValue > 3;
            }
        break;
        case NS_rtf::LN_YAlign:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
/*
        static const SwVertOrient aVertOriTab[ nCntYAlign ] =
        {
            VERT_NONE,         // From Top position
            VERT_TOP,          // top
            VERT_CENTER,       // centered
            VERT_BOTTOM,       // bottom
            VERT_LINE_TOP,     // inside (obscure)
            VERT_LINE_BOTTOM   // outside (obscure)
        };
        // CMC,OD 24.11.2003 #i22673# - to-line vertical alignment
        static const SwVertOrient aToLineVertOriTab[ nCntYAlign ] =
        {
            VERT_NONE,         // below
            VERT_LINE_BOTTOM,  // top
            VERT_LINE_CENTER,  // centered
            VERT_LINE_TOP,     // bottom
            VERT_LINE_BOTTOM,  // inside (obscure)
            VERT_LINE_TOP      // outside (obscure)
        };
        if ( eVertRel == REL_VERT_LINE ) //m_pImpl->nVertRelation == text::RelOrientation::TEXT_LINE
        {
            eVertOri = aToLineVertOriTab[ nYAlign ];
        }
        else
        {
            eVertOri = aVertOriTab[ nYAlign ];
        }

*/
            if( nIntValue < 6 && nIntValue > 0)
            {
                static const sal_Int16 aVertOrientTab[ 6 ] =
                {
                    text::VertOrientation::NONE,         // From Top position
                    text::VertOrientation::TOP,          // top
                    text::VertOrientation::CENTER,       // centered
                    text::VertOrientation::BOTTOM,       // bottom
                    text::VertOrientation::LINE_TOP,     // inside (obscure)
                    text::VertOrientation::LINE_BOTTOM   // outside (obscure)
                };
                static const sal_Int16 aToLineVertOrientTab[ 6 ] =
                {
                    text::VertOrientation::NONE,         // below
                    text::VertOrientation::LINE_BOTTOM,  // top
                    text::VertOrientation::LINE_CENTER,  // centered
                    text::VertOrientation::LINE_TOP,     // bottom
                    text::VertOrientation::LINE_BOTTOM,  // inside (obscure)
                    text::VertOrientation::LINE_TOP      // outside (obscure)
                };
                m_pImpl->nVertOrient = m_pImpl->nVertRelation == text::RelOrientation::TEXT_LINE ?
                    aToLineVertOrientTab[nIntValue] : aVertOrientTab[nIntValue];
            }
        break;
        case NS_rtf::LN_LayoutInTableCell: break; //currently unknown
        case NS_rtf::LN_XRelTo:
        case NS_rtf::LN_BX: //hori orient relation
            switch( nIntValue )
            {
                case  0: m_pImpl->nHoriRelation = text::RelOrientation::PAGE_PRINT_AREA; break;
                case  1: m_pImpl->nHoriRelation = text::RelOrientation::PAGE_FRAME; break;
                case  2: m_pImpl->nHoriRelation = text::RelOrientation::FRAME; break;
                //case  :
                default:m_pImpl->nHoriRelation = text::RelOrientation::CHAR;
            }
        break;
        case NS_rtf::LN_YRelTo:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_BY: //vert orient relation
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            switch( nIntValue )
            {
                case  0: m_pImpl->nVertRelation = text::RelOrientation::PAGE_PRINT_AREA; break;
                case  1: m_pImpl->nVertRelation = text::RelOrientation::PAGE_FRAME; break;
                case  2: m_pImpl->nVertRelation = text::RelOrientation::FRAME; break;
                //case  :
                default:m_pImpl->nVertRelation = text::RelOrientation::TEXT_LINE;
            }

        break;
        case NS_rtf::LN_WR: //wrapping
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            switch( nIntValue )
            {
                case 0: //0 like 2, but doesn't require absolute object
                    m_pImpl->bIgnoreWRK = false;
                case 2: //2 wrap around absolute object
                    m_pImpl->nWrap = text::WrapTextMode_PARALLEL;
                    break;
                case 1: //1 no text next to shape
                    m_pImpl->nWrap = text::WrapTextMode_NONE;
                    break;
                case 3: //3 wrap as if no object present
                    m_pImpl->nWrap = text::WrapTextMode_THROUGHT;
                    break;
                case 4: //4 wrap tightly around object
                    m_pImpl->bIgnoreWRK = false;
                case 5: //5 wrap tightly, but allow holes
                    m_pImpl->nWrap = text::WrapTextMode_PARALLEL;
                    m_pImpl->bContour = true;
                break;
                default:;
            }
        break;
        case NS_rtf::LN_WRK:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            if( !m_pImpl->bIgnoreWRK )
                switch( nIntValue )
                {
                    case 0: //0 like 2, but doesn't require absolute object
                    case 2: //2 wrap around absolute object
                        m_pImpl->nWrap = text::WrapTextMode_PARALLEL;
                        break;
                    case 1: //1 no text next to shape
                        m_pImpl->nWrap = text::WrapTextMode_NONE;
                        break;
                    case 3: //3 wrap as if no object present
                        m_pImpl->nWrap = text::WrapTextMode_THROUGHT;
                        break;
                    case 4: //4 wrap tightly around object
                    case 5: //5 wrap tightly, but allow holes
                        m_pImpl->nWrap = text::WrapTextMode_PARALLEL;
                        m_pImpl->bContour = true;
                    break;
                    default:;
                }
        break;
        case NS_rtf::LN_FRCASIMPLE:
        case NS_rtf::LN_FBELOWTEXT:
        case NS_rtf::LN_FANCHORLOCK:
        case NS_rtf::LN_CTXBX:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        {
//            sal_Int32 nValue1 = val.getInt();
//            nValue1++;
//        }
        break;
        case NS_rtf::LN_shptxt:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //todo: text content
        break;
    /*    case NS_rtf::LN_CH = 10421;
        case NS_rtf::LN_UNUSED0_5 = 10422;
        case NS_rtf::LN_FLT = 10423;
        case NS_rtf::LN_shpLeft = 10424;
        case NS_rtf::LN_shpTop = 10425;
            break;*/
        case NS_rtf::LN_dffheader: break;
        case NS_ooxml::LN_CT_PositiveSize2D_cx:// 90407;
        case NS_ooxml::LN_CT_PositiveSize2D_cy:// 90408;
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        {
            sal_Int32 nDim = ConversionHelper::convertEMUToMM100( nIntValue );
            if( nName == NS_ooxml::LN_CT_PositiveSize2D_cx )
                m_pImpl->setXSize(nDim);
            else
                m_pImpl->setYSize(nDim);
        }        
        break;
        case NS_ooxml::LN_CT_EffectExtent_l:// 90907;
        case NS_ooxml::LN_CT_EffectExtent_t:// 90908;
        case NS_ooxml::LN_CT_EffectExtent_r:// 90909;
        case NS_ooxml::LN_CT_EffectExtent_b:// 90910;
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            //todo: extends the wrapping size of the object, e.g. if shadow is added
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_id:// 90650;
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //id of the object - ignored
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_name:// 90651;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //name of the object
            m_pImpl->sName = val.getString();
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_descr:// 90652;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //alternative text
            m_pImpl->sAlternativeText = val.getString();
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noChangeAspect://90644;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //disallow aspect ratio change - ignored
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noMove:// 90645;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->bPositionProtected = true;
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noResize: // 90646;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->bSizeProtected = true;
        break;
        case NS_ooxml::LN_CT_Anchor_distT: // 90983;
        case NS_ooxml::LN_CT_Anchor_distB: // 90984;
        case NS_ooxml::LN_CT_Anchor_distL: // 90985;
        case NS_ooxml::LN_CT_Anchor_distR: // 90986;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            //redirect to shape option processing
            switch( nName )
            {
                case NS_ooxml::LN_CT_Anchor_distT: // 90983;
                    /* WRITERFILTERSTATUS: */
                    m_pImpl->nShapeOptionType = NS_dff::LN_shpdyWrapDistTop;
                break;
                case NS_ooxml::LN_CT_Anchor_distB: // 90984;
                    /* WRITERFILTERSTATUS: */
                    m_pImpl->nShapeOptionType = NS_dff::LN_shpdyWrapDistBottom;
                break;
                case NS_ooxml::LN_CT_Anchor_distL: // 90985;
                    /* WRITERFILTERSTATUS: */
                    m_pImpl->nShapeOptionType = NS_dff::LN_shpdxWrapDistLeft;
                break;
                case NS_ooxml::LN_CT_Anchor_distR: // 90986;
                    /* WRITERFILTERSTATUS: */
                    m_pImpl->nShapeOptionType = NS_dff::LN_shpdxWrapDistRight;
                break;
                //m_pImpl->nShapeOptionType = NS_dff::LN_shpcropFromTop
                default: ;
            }
            ProcessShapeOptions(val);
        }        
        break;
        case NS_ooxml::LN_CT_Anchor_simplePos_attr: // 90987;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->bUseSimplePos = nIntValue > 0;
        break;
        case NS_ooxml::LN_CT_Anchor_relativeHeight: // 90988;
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            //z-order
        break;
        case NS_ooxml::LN_CT_Anchor_behindDoc: // 90989; - in background
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if( nIntValue > 0 )
                    m_pImpl->bOpaque = false;
        break;
        case NS_ooxml::LN_CT_Anchor_locked: // 90990; - ignored
        case NS_ooxml::LN_CT_Anchor_layoutInCell: // 90991; - ignored
        case NS_ooxml::LN_CT_Anchor_hidden: // 90992; - ignored
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        break;
        case NS_ooxml::LN_CT_Anchor_allowOverlap: // 90993;
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //enable overlapping - ignored
        break;
        case NS_ooxml::LN_CT_Point2D_x: // 90405;
        case NS_ooxml::LN_CT_Point2D_y: // 90406;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if( m_pImpl->bUseSimplePos )
            {
                //todo: absolute positioning
                NS_ooxml::LN_CT_Point2D_x == nName ? m_pImpl->nLeftPosition = ConversionHelper::convertTwipToMM100(nIntValue) : 
                                                        m_pImpl->nTopPosition = ConversionHelper::convertTwipToMM100(nIntValue);
                
            }    
        break;
        case NS_ooxml::LN_CT_WrapTight_wrapText: // 90934;            
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->bContour = true;
            //no break;
        case NS_ooxml::LN_CT_WrapSquare_wrapText: //90928;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            switch ( val.getInt() )
            {
                case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_bothSides: // 90920; 
                    m_pImpl->nWrap = text::WrapTextMode_PARALLEL;
                break;
                case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_left: // 90921;
                    m_pImpl->nWrap = text::WrapTextMode_LEFT;
                break;
                case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_right: // 90922;
                    m_pImpl->nWrap = text::WrapTextMode_RIGHT;
                break;
                case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_largest: // 90923;
                    m_pImpl->nWrap = text::WrapTextMode_DYNAMIC;
                break;
                default:;
            }
        break;
        case NS_ooxml::LN_shape:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            {
                uno::Reference< drawing::XShape> xShape;
                val.getAny( ) >>= xShape;

                if ( xShape.is( ) )
                {                
                    // Is it a graphic image
                    bool bUseShape = true;
                    try
                    {
                        uno::Reference< beans::XPropertySet > xShapeProps
                            ( xShape, uno::UNO_QUERY_THROW );
    
                        rtl::OUString sUrl;
                        xShapeProps->getPropertyValue( rtl::OUString::createFromAscii( "GraphicURL" ) ) >>= sUrl;
    
                        ::com::sun::star::beans::PropertyValues aMediaProperties( 1 );
                        aMediaProperties[0].Name = rtl::OUString::createFromAscii( "URL" );
                        aMediaProperties[0].Value <<= sUrl;
    
                        m_xGraphicObject = createGraphicObject( aMediaProperties );
    
                        bUseShape = !m_xGraphicObject.is( );

                        if ( !bUseShape )
                        {
                            // Define the object size
                            uno::Reference< beans::XPropertySet > xGraphProps( m_xGraphicObject, 
                                    uno::UNO_QUERY );
                            awt::Size aSize = xShape->getSize( );
                            xGraphProps->setPropertyValue( rtl::OUString::createFromAscii( "Height" ),
                                   uno::makeAny( aSize.Height ) );
                            xGraphProps->setPropertyValue( rtl::OUString::createFromAscii( "Width" ),
                                   uno::makeAny( aSize.Width ) );
                        }
                    }
                    catch( const beans::UnknownPropertyException e )
                    {
                        // It isn't a graphic image
                    }
    
                    if ( bUseShape )
                        m_xShape = xShape;

                   
                    if ( m_xShape.is( ) )
                    { 
                        uno::Reference< beans::XPropertySet > xShapeProps
                            (m_xShape, uno::UNO_QUERY_THROW);
                     
    
                        PropertyNameSupplier& rPropNameSupplier = 
                            PropertyNameSupplier::GetPropertyNameSupplier();
                        xShapeProps->setPropertyValue
                            (rPropNameSupplier.GetName(PROP_ANCHOR_TYPE),
                             uno::makeAny
                             (text::TextContentAnchorType_AS_CHARACTER));
                        xShapeProps->setPropertyValue
                            (rPropNameSupplier.GetName(PROP_TEXT_RANGE),
                             uno::makeAny
                             (m_pImpl->rDomainMapper.GetCurrentTextRange()));
    
                        awt::Point aPoint(m_xShape->getPosition());
                        awt::Size aSize(m_xShape->getSize());
    
                        if (m_pImpl->isXSizeValid())
                            aSize.Width = m_pImpl->getXSize();
                        if (m_pImpl->isYSizeValis())
                            aSize.Height = m_pImpl->getYSize();
    
                        m_xShape->setSize(aSize);
    
                        m_pImpl->bIsGraphic = true;
                    }
                }
            }
        break;
        case NS_ooxml::LN_CT_Inline_distT:
        case NS_ooxml::LN_CT_Inline_distB:
        case NS_ooxml::LN_CT_Inline_distL:
        case NS_ooxml::LN_CT_Inline_distR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            //TODO: need to be handled
        break;
        case NS_ooxml::LN_CT_GraphicalObjectData_uri:
            /* WRITERFILTERSTATUS: done: 50, planned: 0.5, spent: 0 */
            val.getString();
            //TODO: does it need to be handled?
        break;
        default: 
#if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "GraphicImport::attribute() - Id: ");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nName ), 10 );
            sMessage += ::rtl::OString(" / 0x");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nName ), 16 );
            OSL_ENSURE( false, sMessage.getStr())
#endif               
            ;
    }
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("attribute");
#endif
}

uno::Reference<text::XTextContent> GraphicImport::GetGraphicObject()
{
    uno::Reference<text::XTextContent> xResult;

    if (m_xGraphicObject.is())
        xResult = m_xGraphicObject;
    else if (m_xShape.is())
    {
        xResult.set(m_xShape, uno::UNO_QUERY_THROW);
    }

    return xResult;
}

/*-- 22.11.2006 09:46:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::ProcessShapeOptions(Value& val)
{
    sal_Int32 nIntValue = val.getInt();
    sal_Int32 nTwipValue = ConversionHelper::convertTwipToMM100(nIntValue);
    /* WRITERFILTERSTATUS: table: ShapeOptionsAttribute */
    switch( m_pImpl->nShapeOptionType )
    {
        case NS_dff::LN_shpcropFromTop /*256*/ : 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nTopCrop   = nTwipValue; 
            break;// rtf:shpcropFromTop
        case NS_dff::LN_shpcropFromBottom /*257*/ : 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nBottomCrop= nTwipValue; 
            break;// rtf:shpcropFromBottom
        case NS_dff::LN_shpcropFromLeft   /*258*/ :
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nLeftCrop  = nTwipValue; 
            break;// rtf:shpcropFromLeft
        case NS_dff::LN_shpcropFromRight/*259*/ : 
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nRightCrop = nTwipValue;
            break;// rtf:shpcropFromRight
        case NS_dff::LN_shppib/*260*/: 
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shppib
        case NS_dff::LN_shppibName/*261*/:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shppibName
        case NS_dff::LN_shppibFlags/*262*/:  // rtf:shppibFlags
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        /*
         * // MSOBLIPFLAGS ñ flags for pictures
            typedef enum
               {
               msoblipflagDefault = 0,
               msoblipflagComment = 0,   // Blip name is a comment
               msoblipflagFile,          // Blip name is a file name
               msoblipflagURL,           // Blip name is a full URL
               msoblipflagType = 3,      // Mask to extract type
               // Or the following flags with any of the above.
               msoblipflagDontSave = 4,  // A "dont" is the depression in the metal
                                         // body work of an automobile caused when a
                                         // cyclist violently thrusts his or her nose
                                         // at it, thus a DontSave is another name for
                                         // a cycle lane.
               msoblipflagDoNotSave = 4, // For those who prefer English
               msoblipflagLinkToFile = 8,
               };
                             *
         * */
        break;
        case NS_dff::LN_shppictureContrast/*264*/: // rtf:shppictureContrast docu: "1<<16"
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            /*
            0x10000 is msoffice 50%
            < 0x10000 is in units of 1/50th of 0x10000 per 1%
            > 0x10000 is in units where
            a msoffice x% is stored as 50/(100-x) * 0x10000

            plus, a (ui) microsoft % ranges from 0 to 100, OOO
            from -100 to 100, so also normalize into that range
            */
            if ( nIntValue > 0x10000 )
            {
                double fX = nIntValue;
                fX /= 0x10000;
                fX /= 51;   // 50 + 1 to round
                fX = 1/fX;
                m_pImpl->nContrast = static_cast<sal_Int32>(fX);
                m_pImpl->nContrast -= 100;
                m_pImpl->nContrast = -m_pImpl->nContrast;
                m_pImpl->nContrast = (m_pImpl->nContrast-50)*2;
            }
            else if ( nIntValue == 0x10000 )
                m_pImpl->nContrast = 0;
            else
            {
                m_pImpl->nContrast = nIntValue * 101;   //100 + 1 to round
                m_pImpl->nContrast /= 0x10000;
                m_pImpl->nContrast -= 100;
            }
        break;
        case NS_dff::LN_shppictureBrightness/*265*/:  // rtf:shppictureBrightness
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->nBrightness     = ( (sal_Int32) nIntValue / 327 );
        break;
        case NS_dff::LN_shppictureGamma/*266*/: // rtf:shppictureGamma
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
            //todo check gamma value with _real_ document
            m_pImpl->fGamma = double(nIntValue/655);
        break;
        case NS_dff::LN_shppictureId        /*267*/:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shppictureId
        case NS_dff::LN_shppictureDblCrMod  /*268*/:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shppictureDblCrMod
        case NS_dff::LN_shppictureFillCrMod /*269*/:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shppictureFillCrMod
        case NS_dff::LN_shppictureLineCrMod /*270*/:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shppictureLineCrMod

        case NS_dff::LN_shppictureActive/*319*/: // rtf:shppictureActive
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            switch( nIntValue & 0x06 )
            {
                case 0 : m_pImpl->eColorMode = drawing::ColorMode_STANDARD; break;
                case 4 : m_pImpl->eColorMode = drawing::ColorMode_GREYS; break;
                case 6 : m_pImpl->eColorMode = drawing::ColorMode_MONO; break;
                default:;
            }
        break;
        case NS_dff::LN_shpfillColor           /*385*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            m_pImpl->nFillColor = (m_pImpl->nFillColor & 0xff000000) + ConversionHelper::ConvertColor( nIntValue );
        break;
        case NS_dff::LN_shpfillOpacity         /*386*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        {
            sal_Int32 nTrans = 0xff - ( nIntValue * 0xff ) / 0xffff;
            m_pImpl->nFillColor = (nTrans << 0x18 ) + (m_pImpl->nFillColor & 0xffffff);
        }
        break;
        case NS_dff::LN_shpfNoFillHitTest      /*447*/:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shpfNoFillHitTest
        case NS_dff::LN_shplineColor           /*448*/:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineColor = ConversionHelper::ConvertColor( nIntValue );
        break;
        case NS_dff::LN_shplineWidth           /*459*/:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            //1pt == 12700 units
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineWidth = ConversionHelper::convertTwipToMM100(nIntValue / 635);
        break;
        case NS_dff::LN_shplineDashing         /*462*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //graphic borders don't support different dashing
            /*MSOLINEDASHING
                msolineSolid,              // Solid (continuous) pen
                msolineDashSys,            // PS_DASH system   dash style
                msolineDotSys,             // PS_DOT system   dash style
                msolineDashDotSys,         // PS_DASHDOT system dash style
                msolineDashDotDotSys,      // PS_DASHDOTDOT system dash style
                msolineDotGEL,             // square dot style
                msolineDashGEL,            // dash style
                msolineLongDashGEL,        // long dash style
                msolineDashDotGEL,         // dash short dash
                msolineLongDashDotGEL,     // long dash short dash
                msolineLongDashDotDotGEL   // long dash short dash short dash*/
            //m_pImpl->aBorders[nCurrentBorderLine].nLineType = nIntValue;
        break;
        case NS_dff::LN_shpfNoLineDrawDash     /*511*/:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        break;  // rtf:shpfNoLineDrawDash
        case NS_dff::LN_shpwzDescription /*897*/: //alternative text
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            m_pImpl->sAlternativeText = val.getString();
        break;
//        case NS_dff::LN_shppihlShape /*898*/:
        case NS_dff::LN_shppWrapPolygonVertices/*899*/:  
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shppWrapPolygonVertices
        case NS_dff::LN_shpdxWrapDistLeft /*900*/: // contains a twip/635 value
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustLRWrapForWordMargins()
            m_pImpl->nLeftMargin = nIntValue / 360;
        break;
        case NS_dff::LN_shpdyWrapDistTop /*901*/:  // contains a twip/635 value
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustULWrapForWordMargins()
            m_pImpl->nTopMargin = nIntValue / 360;
        break;
        case NS_dff::LN_shpdxWrapDistRight /*902*/:// contains a twip/635 value
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustLRWrapForWordMargins()
            m_pImpl->nRightMargin = nIntValue / 360;
        break;
        case NS_dff::LN_shpdyWrapDistBottom /*903*/:// contains a twip/635 value
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustULWrapForWordMargins()
            m_pImpl->nBottomMargin = nIntValue / 360;
        break;
        case NS_dff::LN_shpfPrint              /*959*/:  
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;  // rtf:shpfPrint
        default:
            OSL_ENSURE( false, "shape option unsupported?");
    }
}
/*-- 01.11.2006 09:45:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::sprm(Sprm & rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("sprm");
    dmapper_logger->chars(rSprm.toString());
#endif

    sal_uInt32 nSprmId = rSprm.getId();
    Value::Pointer_t pValue = rSprm.getValue();

    /* WRITERFILTERSTATUS: table: PICFsprmdata */
    switch(nSprmId)
    {
        case 0xf004: //dff record
        case 0xf00a: //part of 0xf004 - shape properties
        case 0xf00b: //part of 0xf004
        case 0xf007:
        case 0xf122: //udefprop
        case NS_ooxml::LN_CT_Inline_extent: // 90911;
        case NS_ooxml::LN_CT_Inline_effectExtent: // 90912;
        case NS_ooxml::LN_CT_Inline_docPr: // 90913;
        case NS_ooxml::LN_CT_Inline_cNvGraphicFramePr: // 90914;
        case NS_ooxml::LN_CT_NonVisualGraphicFrameProperties_graphicFrameLocks:// 90657
        case NS_ooxml::LN_CT_Inline_a_graphic:// 90915
        case NS_ooxml::LN_CT_Anchor_simplePos_elem: // 90975;
        case NS_ooxml::LN_CT_Anchor_extent: // 90978;
        case NS_ooxml::LN_CT_Anchor_effectExtent: // 90979;
        case NS_ooxml::LN_EG_WrapType_wrapSquare: // 90945;                   
        case NS_ooxml::LN_EG_WrapType_wrapTight: // 90946;                
        case NS_ooxml::LN_CT_Anchor_docPr: // 90980;
        case NS_ooxml::LN_CT_Anchor_cNvGraphicFramePr: // 90981;
        case NS_ooxml::LN_CT_Anchor_a_graphic: // 90982;
        case NS_ooxml::LN_CT_WrapPath_start: // 90924;
        case NS_ooxml::LN_CT_WrapPath_lineTo: // 90925;
        case NS_ooxml::LN_CT_WrapTight_wrapPolygon: // 90933; 
        case NS_ooxml::LN_graphic_graphic:
        case NS_ooxml::LN_pic_pic:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
            }
        }
        break;
        case NS_ooxml::LN_CT_Anchor_positionH: // 90976;
        {
            // Use a special handler for the positionning
            PositionHandlerPtr pHandler( new PositionHandler );
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get( ) )
            {
                pProperties->resolve( *pHandler );

                m_pImpl->nHoriRelation = pHandler->m_nRelation;
                m_pImpl->nHoriOrient = pHandler->m_nOrient;
                m_pImpl->nLeftPosition = pHandler->m_nPosition;
            }
        }
        break;
        case NS_ooxml::LN_CT_Anchor_positionV: // 90977;
        {
            // Use a special handler for the positionning
            PositionHandlerPtr pHandler( new PositionHandler );
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get( ) )
            {
                pProperties->resolve( *pHandler );

                m_pImpl->nVertRelation = pHandler->m_nRelation;
                m_pImpl->nVertOrient = pHandler->m_nOrient;
                m_pImpl->nTopPosition = pHandler->m_nPosition;
            }
        }
        break;
        case 0x271b:
        case 0x271c:
        {
            if( nSprmId != 0x271c || m_pImpl->nDffType == 0xf01f || m_pImpl->nDffType == 0xf01e )
            {
                writerfilter::Reference<BinaryObj>::Pointer_t pPictureData = rSprm.getBinary();
                if( pPictureData.get())
                    pPictureData->resolve(*this);
            }
        }
        break;
        case NS_ooxml::LN_EG_WrapType_wrapNone: // 90944; - doesn't contain attributes
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //depending on the behindDoc attribute text wraps through behind or in fron of the object
            m_pImpl->nWrap = text::WrapTextMode_THROUGHT;
        break;
        case NS_ooxml::LN_EG_WrapType_wrapTopAndBottom: // 90948;         
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->nWrap = text::WrapTextMode_NONE;
        break;
        case NS_ooxml::LN_EG_WrapType_wrapThrough: // 90947;              
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->nWrap = text::WrapTextMode_THROUGHT;
        break;
        case 0xf010:
        case 0xf011:
            //ignore - doesn't contain useful members
        break;
        case NS_ooxml::LN_CT_GraphicalObject_graphicData:// 90660;
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            {
                m_pImpl->bIsGraphic = true;

                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                    pProperties->resolve(*this);
            }
        break;
        default:
#if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "GraphicImport::sprm() - Id: ");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nSprmId ), 10 );
            sMessage += ::rtl::OString(" / 0x");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nSprmId ), 16 );
            OSL_ENSURE( false, sMessage.getStr())
#endif
            ;
    }

    

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("sprm");
#endif
}
/*-- 01.11.2006 09:45:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
}
/*-- 16.11.2006 16:14:32---------------------------------------------------
    crop is stored as "fixed float" as 16.16 fraction value
    related to width/or height
  -----------------------------------------------------------------------*/
void lcl_CalcCrop( sal_Int32& nCrop, sal_Int32 nRef )
{
    nCrop = ((nCrop >> 16   ) * nRef )
       + (((nCrop & 0xffff) * nRef ) >> 16);
}

uno::Reference< text::XTextContent > GraphicImport::createGraphicObject( const beans::PropertyValues& aMediaProperties )
{
    uno::Reference< text::XTextContent > xGraphicObject;
    try
    {
        uno::Reference< graphic::XGraphicProvider > xGraphicProvider(
                            m_xComponentContext->getServiceManager()->createInstanceWithContext(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.graphic.GraphicProvider")),
                                m_xComponentContext),
                            uno::UNO_QUERY_THROW );
        
        uno::Reference< graphic::XGraphic > xGraphic = xGraphicProvider->queryGraphic( aMediaProperties );

        if(xGraphic.is())
        {
            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

            uno::Reference< beans::XPropertySet > xGraphicObjectProperties(
            m_xTextFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextGraphicObject"))),
                uno::UNO_QUERY_THROW);
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_GRAPHIC), uno::makeAny( xGraphic ));
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_ANCHOR_TYPE),
                uno::makeAny( m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE || m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR ?
                                    text::TextContentAnchorType_AT_CHARACTER :
                                    text::TextContentAnchorType_AS_CHARACTER ));
            xGraphicObject = uno::Reference< text::XTextContent >( xGraphicObjectProperties, uno::UNO_QUERY_THROW );

            //shapes have only one border, PICF might have four
            table::BorderLine2 aBorderLine;
            for( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder )
            {
                if( m_pImpl->eGraphicImportType == IMPORT_AS_GRAPHIC || !nBorder )
                {
                    aBorderLine.Color = m_pImpl->aBorders[m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE ? BORDER_TOP : static_cast<BorderPosition>(nBorder) ].nLineColor;
                    aBorderLine.InnerLineWidth = 0;
                    aBorderLine.OuterLineWidth = (sal_Int16)m_pImpl->aBorders[m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE ? BORDER_TOP : static_cast<BorderPosition>(nBorder) ].nLineWidth;
                    aBorderLine.LineDistance = 0;
                }
                PropertyIds aBorderProps[4] =
                {
                    PROP_LEFT_BORDER,
                    PROP_RIGHT_BORDER,
                    PROP_TOP_BORDER,
                    PROP_BOTTOM_BORDER
                };
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( aBorderProps[nBorder]), uno::makeAny(aBorderLine));
            }

            // setting properties for all types
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_TITLE ),
                uno::makeAny( m_pImpl->sAlternativeText ));
            if( m_pImpl->bPositionProtected )
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_POSITION_PROTECTED ), 
                    uno::makeAny(true));
            if( m_pImpl->bSizeProtected )
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_SIZE_PROTECTED ),
                    uno::makeAny(true));
            
            if( m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE || m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR )
            {
                sal_Int32 nWidth = m_pImpl->nRightPosition - m_pImpl->nLeftPosition;
                if( m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE )
                {
                    sal_Int32 nHeight = m_pImpl->nBottomPosition - m_pImpl->nTopPosition;
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SIZE),
                        uno::makeAny( awt::Size( nWidth, nHeight )));
                }
                //adjust margins
                if( (m_pImpl->nHoriOrient == text::HoriOrientation::LEFT &&
                    (m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                        m_pImpl->nHoriRelation == text::RelOrientation::FRAME) ) ||
                     (m_pImpl->nHoriOrient == text::HoriOrientation::INSIDE &&
                       m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ))
                    m_pImpl->nLeftMargin = 0;
                if((m_pImpl->nHoriOrient == text::HoriOrientation::RIGHT &&
                        (m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                            m_pImpl->nHoriRelation == text::RelOrientation::FRAME) ) ||
                    (m_pImpl->nHoriOrient == text::HoriOrientation::INSIDE &&
                        m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ))
                    m_pImpl->nRightMargin = 0;
                // adjust top/bottom margins
                if( m_pImpl->nVertOrient == text::VertOrientation::TOP &&
                        ( m_pImpl->nVertRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                            m_pImpl->nVertRelation == text::RelOrientation::PAGE_FRAME))
                    m_pImpl->nTopMargin = 0;
                if( m_pImpl->nVertOrient == text::VertOrientation::BOTTOM &&
                        ( m_pImpl->nVertRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                            m_pImpl->nVertRelation == text::RelOrientation::PAGE_FRAME))
                    m_pImpl->nBottomMargin = 0;
                if( m_pImpl->nVertOrient == text::VertOrientation::BOTTOM &&
                        m_pImpl->nVertRelation == text::RelOrientation::PAGE_PRINT_AREA )
                    m_pImpl->nBottomMargin = 0;

                //adjust alignment
                if( m_pImpl->nHoriOrient == text::HoriOrientation::INSIDE &&
                        m_pImpl->nHoriRelation == text::RelOrientation::PAGE_FRAME )
                {
                    // convert 'left to page' to 'from left -<width> to page text area'
                    m_pImpl->nHoriOrient = text::HoriOrientation::NONE;
                    m_pImpl->nHoriRelation = text::RelOrientation::PAGE_PRINT_AREA;
                    m_pImpl->nLeftPosition = - nWidth;
                }
                else if( m_pImpl->nHoriOrient == text::HoriOrientation::OUTSIDE &&
                        m_pImpl->nHoriRelation == text::RelOrientation::PAGE_FRAME )
                {
                    // convert 'right to page' to 'from left 0 to right page border'
                    m_pImpl->nHoriOrient = text::HoriOrientation::NONE;
                    m_pImpl->nHoriRelation = text::RelOrientation::PAGE_RIGHT;
                    m_pImpl->nLeftPosition = 0;
                }

                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT          ),
                    uno::makeAny(m_pImpl->nHoriOrient));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT_POSITION),
                    uno::makeAny(m_pImpl->nLeftPosition));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT_RELATION ),
                    uno::makeAny(m_pImpl->nHoriRelation));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_PAGE_TOGGLE ),
                    uno::makeAny(m_pImpl->bPageToggle));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT          ),
                    uno::makeAny(m_pImpl->nVertOrient));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT_POSITION),
                    uno::makeAny(m_pImpl->nTopPosition));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT_RELATION ),
                uno::makeAny(m_pImpl->nVertRelation));
                if( !m_pImpl->bOpaque )
                {
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_OPAQUE ),
                        uno::makeAny(m_pImpl->bOpaque));
                }
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_SURROUND ),
                        uno::makeAny(m_pImpl->nWrap));

                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_SURROUND_CONTOUR ),
                    uno::makeAny(m_pImpl->bContour));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_CONTOUR_OUTSIDE ),
                    uno::makeAny(true));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_LEFT_MARGIN ),
                    uno::makeAny(m_pImpl->nLeftMargin));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_RIGHT_MARGIN ),
                    uno::makeAny(m_pImpl->nRightMargin));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_TOP_MARGIN ),
                    uno::makeAny(m_pImpl->nTopMargin));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_BOTTOM_MARGIN ),
                    uno::makeAny(m_pImpl->nBottomMargin));

                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_CONTOUR_POLY_POLYGON),
                    uno::Any());
                if( m_pImpl->eColorMode == drawing::ColorMode_STANDARD &&
                    m_pImpl->nContrast == -70 &&
                    m_pImpl->nBrightness == 70 )
                {
                    // strange definition of WATERMARK!
                    m_pImpl->nContrast = 0;
                    m_pImpl->nBrightness = 0;
                    m_pImpl->eColorMode = drawing::ColorMode_WATERMARK;
                }

                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_ADJUST_CONTRAST ),
                    uno::makeAny((sal_Int16)m_pImpl->nContrast));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_ADJUST_LUMINANCE ),
                    uno::makeAny((sal_Int16)m_pImpl->nBrightness));
                if(m_pImpl->eColorMode != drawing::ColorMode_STANDARD)
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_GRAPHIC_COLOR_MODE ),
                        uno::makeAny(m_pImpl->eColorMode));
                if(m_pImpl->fGamma > 0. )
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_GAMMA ),
                        uno::makeAny(m_pImpl->fGamma ));
                if(m_pImpl->bHoriFlip)
                {
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_MIRRORED_ON_EVEN_PAGES ),
                        uno::makeAny( m_pImpl->bHoriFlip ));
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_MIRRORED_ON_ODD_PAGES ),
                        uno::makeAny( m_pImpl->bHoriFlip ));
                }
                if( m_pImpl->bVertFlip )
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_MIRRORED ),
                        uno::makeAny( m_pImpl->bVertFlip ));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_BACK_COLOR ),
                    uno::makeAny( m_pImpl->nFillColor ));
                //there seems to be no way to detect the original size via _real_ API
                uno::Reference< beans::XPropertySet > xGraphicProperties( xGraphic, uno::UNO_QUERY_THROW );
                awt::Size aGraphicSize, aGraphicSizePixel;
                xGraphicProperties->getPropertyValue(rPropNameSupplier.GetName( PROP_SIZE100th_M_M )) >>= aGraphicSize;
                xGraphicProperties->getPropertyValue(rPropNameSupplier.GetName( PROP_SIZE_PIXEL )) >>= aGraphicSizePixel;
                if( aGraphicSize.Width && aGraphicSize.Height )
                {
                    //todo: i71651 graphic size is not provided by the GraphicDescriptor
                    lcl_CalcCrop( m_pImpl->nTopCrop, aGraphicSize.Height );
                    lcl_CalcCrop( m_pImpl->nBottomCrop, aGraphicSize.Height );
                    lcl_CalcCrop( m_pImpl->nLeftCrop, aGraphicSize.Width );
                    lcl_CalcCrop( m_pImpl->nRightCrop, aGraphicSize.Width );

                    xGraphicProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_GRAPHIC_CROP ),
                        uno::makeAny(text::GraphicCrop(m_pImpl->nTopCrop, m_pImpl->nBottomCrop, m_pImpl->nLeftCrop, m_pImpl->nRightCrop)));
                }
            }
            
            if(m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_INLINE || m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
            {
                if( m_pImpl->getXSize() && m_pImpl->getYSize() )
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SIZE),
                        uno::makeAny( awt::Size( m_pImpl->getXSize(), m_pImpl->getYSize() )));
                try
                {
                    if( m_pImpl->sName.getLength() )
                    {
                        uno::Reference< container::XNamed > xNamed( xGraphicObjectProperties, uno::UNO_QUERY_THROW );
                        xNamed->setName( m_pImpl->sName );
                    }    
                }
                catch( const uno::Exception& )
                {
                }    
            }    
        }
    }
    catch( const uno::Exception& e )
    {
        clog << __FILE__ << ":" << __LINE__ << " failed. Message :" ;
        clog << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( )  << endl;
    }
    return xGraphicObject;
}

/*-- 01.11.2006 09:45:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::data(const sal_uInt8* buf, size_t len, writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

        ::com::sun::star::beans::PropertyValues aMediaProperties( 1 );
        aMediaProperties[0].Name = rPropNameSupplier.GetName(PROP_INPUT_STREAM);
        
        uno::Reference< io::XInputStream > xIStream = new XInputStreamHelper( buf, len, m_pImpl->bIsBitmap );
        aMediaProperties[0].Value <<= xIStream;

        m_xGraphicObject = createGraphicObject( aMediaProperties );
}
/*-- 01.11.2006 09:45:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::startSectionGroup()
{
}
/*-- 01.11.2006 09:45:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::endSectionGroup()
{
}
/*-- 01.11.2006 09:45:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::startParagraphGroup()
{
}
/*-- 01.11.2006 09:45:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::endParagraphGroup()
{
}
/*-- 01.11.2006 09:45:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::startCharacterGroup()
{
}
/*-- 01.11.2006 09:45:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::endCharacterGroup()
{
}
/*-- 01.11.2006 09:45:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::text(const sal_uInt8 * /*_data*/, size_t /*len*/)
{
}
/*-- 01.11.2006 09:45:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::utext(const sal_uInt8 * /*_data*/, size_t /*len*/)
{
}
/*-- 01.11.2006 09:45:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::props(writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
}
/*-- 01.11.2006 09:45:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::table(Id /*name*/, writerfilter::Reference<Table>::Pointer_t /*ref*/)
{
}
/*-- 01.11.2006 09:45:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::substream(Id /*name*/, ::writerfilter::Reference<Stream>::Pointer_t /*ref*/)
{
}
/*-- 01.11.2006 09:45:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::info(const string & /*info*/)
{
}
    
void GraphicImport::startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > /*xShape*/ )
{
}

void GraphicImport::endShape( )
{
}

/*-- 09.08.2007 10:17:00---------------------------------------------------

  -----------------------------------------------------------------------*/
bool    GraphicImport::IsGraphic() const
{
    return m_pImpl->bIsGraphic;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
