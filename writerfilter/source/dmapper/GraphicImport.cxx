/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GraphicImport.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:49:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_GRAPHICIMPORT_HXX
#include <GraphicImport.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#endif
#include <ooxml/resourceids.hxx>
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#include <com/sun/star/container/XNamed.hpp>
#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif

//#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
//#include <com/sun/star/drawing/PointSequenceSequence.hpp>
//#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICPROVIDER_HPP_
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphicr.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HDL_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HDL_
#include <com/sun/star/text/RelOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HDL_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#include <rtl/ustrbuf.hxx>


#include <iostream>
#include <resourcemodel/QNameToString.hxx>
#include <string.h>

namespace writerfilter {
namespace dmapper
{
using namespace ::std;
using namespace ::com::sun::star;

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
struct GraphicImport_Impl
{
    GraphicImportType eGraphicImportType;

    sal_Int32 nHoriScaling;
    sal_Int32 nVertScaling;
    sal_Int32 nXSize;
    sal_Int32 nYSize;
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

    GraphicImport_Impl(GraphicImportType eImportType) :
        eGraphicImportType( eImportType )
        ,nHoriScaling(0)
        ,nVertScaling(0)
        ,nXSize(0)
        ,nYSize(0)
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
};
/*-- 01.11.2006 09:42:42---------------------------------------------------

  -----------------------------------------------------------------------*/
GraphicImport::GraphicImport(uno::Reference < uno::XComponentContext >    xComponentContext,
                             uno::Reference< lang::XMultiServiceFactory > xTextFactory,
                             GraphicImportType eImportType )
: m_pImpl( new GraphicImport_Impl( eImportType ))
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
    logger("DOMAINMAPPER", string("<attribute name=\"") +
           (*QNameToString::Instance())(nName) + "\">");
    //logger("DOMAINMAPPER", string("<value>") + val.toString() + "</value>");
#endif
    sal_Int32 nIntValue = val.getInt();
    /* WRITERFILTERSTATUS: table: PICFattribute */
    switch( nName )
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_LCB: break;//byte count
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_CBHEADER: break;//ignored
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_MFP: //MetafilePict
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_DffRecord: //dff record - expands to an sprm which expands to ...
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_shpopt: //shape options
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfbse: //BLIP store entry
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCTOP: //top border
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCLEFT: //left border
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCBOTTOM: //bottom border
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCRIGHT: //right border
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_shape: //shape
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_blip: //the binary graphic data in a shape
            {
                switch(nName)
                {
                    case NS_rtf::LN_BRCTOP: //top border
                        m_pImpl->nCurrentBorderLine = BORDER_TOP;
                    break;
                    case NS_rtf::LN_BRCLEFT: //left border
                        m_pImpl->nCurrentBorderLine = BORDER_LEFT;
                    break;
                    case NS_rtf::LN_BRCBOTTOM: //bottom border
                        m_pImpl->nCurrentBorderLine = BORDER_BOTTOM;
                    break;
                    case NS_rtf::LN_BRCRIGHT: //right border
                        m_pImpl->nCurrentBorderLine = BORDER_RIGHT;
                    break;
                    case NS_rtf::LN_shpopt:
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
                        m_pImpl->bInShapeOptionMode = false;
                    break;
                    default:;
                }
        }
        break;
        case NS_rtf::LN_payload :
        {
            writerfilter::Reference<BinaryObj>::Pointer_t pPictureData = val.getBinary();
            if( pPictureData.get())
                pPictureData->resolve(*this);
        }
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_BM_RCWINMF: //windows bitmap structure - if it's a bitmap
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_DXAGOAL: //x-size in twip
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_DYAGOAL: //y-size in twip
            break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_MX: m_pImpl->nHoriScaling = nIntValue; break;// hori scaling in 0.001%
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_MY: m_pImpl->nVertScaling = nIntValue; break;// vert scaling in 0.001%
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_DXACROPLEFT:    m_pImpl->nLeftCrop  = ConversionHelper::convertTwipToMM100(nIntValue); break;// left crop in twips
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_DYACROPTOP:     m_pImpl->nTopCrop   = ConversionHelper::convertTwipToMM100(nIntValue); break;// top crop in twips
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_DXACROPRIGHT:   m_pImpl->nRightCrop = ConversionHelper::convertTwipToMM100(nIntValue); break;// right crop in twips
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_DYACROPBOTTOM:  m_pImpl->nBottomCrop = ConversionHelper::convertTwipToMM100(nIntValue); break;// bottom crop in twips
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCL:           break;//border type - legacy -
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FFRAMEEMPTY:    break;// picture consists of a single frame
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FBITMAP:
            m_pImpl->bIsBitmap = nIntValue > 0 ? true : false;
        break;//1 if it's a bitmap ???
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FDRAWHATCH:     break;//1 if it's an active OLE object
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FERROR:         break;// 1 if picture is an error message
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_BPP: m_pImpl->nBitsPerPixel = nIntValue; break;//bits per pixel 0 - unknown, 1- mono, 4 - VGA

        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_DXAORIGIN: //horizontal offset of hand annotation origin
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_DYAORIGIN: //vertical offset of hand annotation origin
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_CPROPS:break;// unknown - ignored
        //metafilepict
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_MM:
//      according to the documentation 99 or 98 are provided - but they are not!
//            m_pImpl->bIsBitmap = 99 == nIntValue ? true : false;
//            m_pImpl->bIsTiff = 98 == nIntValue ? true : false;

        break; //mapmode
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_XEXT: m_pImpl->nXSize = nIntValue; break; // x-size
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_YEXT: m_pImpl->nYSize = nIntValue; break; // y-size
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_HMF: break; //identifier - ignored

        //sprm 0xf004 and 0xf008, 0xf00b
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_dfftype://
            m_pImpl->nDffType = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_dffinstance:
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
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_dffversion://  ignored
        break;

        //sprm 0xf008
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shptype:        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpid:          break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfGroup:      break;// This shape is a group shape
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfChild:      break;// Not a top-level shape
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfPatriarch:  break;// This is the topmost group shape. Exactly one of these per drawing.
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfDeleted:    break;// The shape has been deleted
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfOleShape:   break;// The shape is an OLE object
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfHaveMaster: break;// Shape has a hspMaster property
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfFlipH:       // Shape is flipped horizontally
            m_pImpl->bHoriFlip = nIntValue ? true : false;
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfFlipV:       // Shape is flipped vertically
            m_pImpl->bVertFlip = nIntValue ? true : false;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfConnector:   break;// Connector type of shape
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfHaveAnchor:  break;// Shape has an anchor of some kind
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfBackground:  break;// Background shape
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfHaveSpt:     break;// Shape has a shape type property
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shptypename: break;// shape type name
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_shppid:     m_pImpl->nShapeOptionType = nIntValue; break; //type of shape option
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfBid:    break; //ignored
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfComplex:break;
        /* WRITERFILTERSTATUS: done: 50, planned: 10, spent: 5 */
        case NS_rtf::LN_shpop:
        {
            if(NS_dff::LN_shpwzDescription != sal::static_int_cast<Id>(m_pImpl->nShapeOptionType) )
                ProcessShapeOptions( val );
        }
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpname:    break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpvalue:
        {
            if( NS_dff::LN_shpwzDescription == sal::static_int_cast<Id>(m_pImpl->nShapeOptionType) )
                ProcessShapeOptions( val );
        }
        break;

        //BLIP store entry
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpbtWin32: break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpbtMacOS: break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shprgbUid:  break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shptag:     break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpsize:    break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpcRef:    break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpfoDelay: break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpusage:   break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpcbName:  break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpunused2: break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpunused3: break;

        //border properties
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shpblipbname : break;

        /* WRITERFILTERSTATUS: done: 100, planned: 1, spent: 1 */
        case NS_rtf::LN_DPTLINEWIDTH:  // 0x1759
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineWidth = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCTYPE:   // 0x175a
            //graphic borders don't support different line types
            //m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineType = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 1, spent: 1 */
        case NS_rtf::LN_ICO:   // 0x175b
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineColor = ConversionHelper::ConvertColor( nIntValue );
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 1, spent: 1 */
        case NS_rtf::LN_DPTSPACE:  // 0x175c
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineDistance = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 1, spent: 0 */
        case NS_rtf::LN_FSHADOW:   // 0x175d
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].bHasShadow = nIntValue ? true : false;
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_FFRAME:            // ignored
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_UNUSED2_15: break;// ignored

//    const QName_t LN_shpoptextraoffset = 20028;
//    const QName_t LN_shptypename = 20029;
//    const QName_t LN_shpblipbname = 20031;
//    const QName_t LN_binary = 20032;



//    case NS_rtf::LN_shpblipbname = 20031;
//    case NS_rtf::LN_binary = 20032;
//    case NS_rtf::LN_shpdgg = 10492;
//    case NS_rtf::LN_shpfbse = 10493;


//    case NS_rtf::LN_CPROPS: //unused



//    case NS_rtf::LN_LINECOLOR = 10372;
//    case NS_rtf::LN_LINEWIDTH = 10373;
//    case NS_rtf::LN_LINETYPE = 10374;

        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_SPID: break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_XALEFT: m_pImpl->nLeftPosition = ConversionHelper::convertTwipToMM100(nIntValue); break; //left position
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_YATOP:  m_pImpl->nTopPosition = ConversionHelper::convertTwipToMM100(nIntValue); break; //top position
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_XARIGHT:  m_pImpl->nRightPosition = ConversionHelper::convertTwipToMM100(nIntValue); break; //right position
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_YABOTTOM: m_pImpl->nBottomPosition = ConversionHelper::convertTwipToMM100(nIntValue); break;//bottom position
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FHDR:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_XAlign:
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
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_YRelTo:
        case NS_rtf::LN_BY: //vert orient relation
            switch( nIntValue )
            {
                case  0: m_pImpl->nVertRelation = text::RelOrientation::PAGE_PRINT_AREA; break;
                case  1: m_pImpl->nVertRelation = text::RelOrientation::PAGE_FRAME; break;
                case  2: m_pImpl->nVertRelation = text::RelOrientation::FRAME; break;
                //case  :
                default:m_pImpl->nVertRelation = text::RelOrientation::TEXT_LINE;
            }

        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_WR: //wrapping
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
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_rtf::LN_WRK:
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
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FRCASIMPLE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FBELOWTEXT:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FANCHORLOCK:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_CTXBX:
//        {
//            sal_Int32 nValue1 = val.getInt();
//            nValue1++;
//        }
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_shptxt:
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
        {
            sal_Int32 nDim = ConversionHelper::convertEMUToMM100( nIntValue );
            if( nName == NS_ooxml::LN_CT_PositiveSize2D_cx )
                m_pImpl->nXSize = nDim;
            else
                m_pImpl->nYSize = nDim;
        }
        break;
        case NS_ooxml::LN_CT_EffectExtent_l:// 90907;
        case NS_ooxml::LN_CT_EffectExtent_t:// 90908;
        case NS_ooxml::LN_CT_EffectExtent_r:// 90909;
        case NS_ooxml::LN_CT_EffectExtent_b:// 90910;
            //todo: extends the wrapping size of the object, e.g. if shadow is added
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_id:// 90650;
            //id of the object - ignored
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_name:// 90651;
            //name of the object
            m_pImpl->sName = val.getString();
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_descr:// 90652;
            //alternative text
            m_pImpl->sAlternativeText = val.getString();
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noChangeAspect://90644;
            //disallow aspect ratio change - ignored
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noMove:// 90645;
            m_pImpl->bPositionProtected = true;
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noResize: // 90646;
            m_pImpl->bSizeProtected = true;
        break;
        case NS_ooxml::LN_CT_Anchor_distT: // 90983;
        case NS_ooxml::LN_CT_Anchor_distB: // 90984;
        case NS_ooxml::LN_CT_Anchor_distL: // 90985;
        case NS_ooxml::LN_CT_Anchor_distR: // 90986;
        {
            //redirect to shape option processing
            switch( nName )
            {
                case NS_ooxml::LN_CT_Anchor_distT: // 90983;
                    m_pImpl->nShapeOptionType = NS_dff::LN_shpdyWrapDistTop;
                break;
                case NS_ooxml::LN_CT_Anchor_distB: // 90984;
                    m_pImpl->nShapeOptionType = NS_dff::LN_shpdyWrapDistBottom;
                break;
                case NS_ooxml::LN_CT_Anchor_distL: // 90985;
                    m_pImpl->nShapeOptionType = NS_dff::LN_shpdxWrapDistLeft;
                break;
                case NS_ooxml::LN_CT_Anchor_distR: // 90986;
                    m_pImpl->nShapeOptionType = NS_dff::LN_shpdxWrapDistRight;
                break;
                //m_pImpl->nShapeOptionType = NS_dff::LN_shpcropFromTop
                default: ;
            }
            ProcessShapeOptions(val);
        }
        break;
        case NS_ooxml::LN_CT_Anchor_simplePos_attr: // 90987;
            m_pImpl->bUseSimplePos = nIntValue > 0;
        break;
        case NS_ooxml::LN_CT_Anchor_relativeHeight: // 90988;
            //z-order
        break;
        case NS_ooxml::LN_CT_Anchor_behindDoc: // 90989; - in background
            if( nIntValue > 0 )
                    m_pImpl->bOpaque = false;
        break;
        case NS_ooxml::LN_CT_Anchor_locked: // 90990; - ignored
        case NS_ooxml::LN_CT_Anchor_layoutInCell: // 90991; - ignored
            //true: inside cell, cell resizes, false: table is resized or relocated, object might be outside of the table
        case NS_ooxml::LN_CT_Anchor_hidden: // 90992; - ignored
        break;
        case NS_ooxml::LN_CT_Anchor_allowOverlap: // 90993;
            //enable overlapping - ignored
        break;
        case NS_ooxml::LN_CT_Point2D_x: // 90405;
        case NS_ooxml::LN_CT_Point2D_y: // 90406;
            if( m_pImpl->bUseSimplePos )
            {
                //todo: absolute positioning
                NS_ooxml::LN_CT_Point2D_x == nName ? m_pImpl->nLeftPosition = ConversionHelper::convertTwipToMM100(nIntValue) :
                                                        m_pImpl->nTopPosition = ConversionHelper::convertTwipToMM100(nIntValue);

            }
        break;
        case NS_ooxml::LN_CT_WrapTight_wrapText: // 90934;
            m_pImpl->bContour = true;
            //no break;
        case NS_ooxml::LN_CT_WrapSquare_wrapText: //90928;
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
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            {
                val.getAny() >>= m_xShape;

                if (m_xShape.is())
                {
                    uno::Reference< beans::XPropertySet > xShapeProps
                        (m_xShape, uno::UNO_QUERY_THROW);

                    PropertyNameSupplier& rPropNameSupplier =
                        PropertyNameSupplier::GetPropertyNameSupplier();
                    xShapeProps->setPropertyValue
                        (rPropNameSupplier.GetName(PROP_ANCHOR_TYPE),
                         uno::makeAny
                         (text::TextContentAnchorType_AS_CHARACTER));

                    awt::Size aSize(m_xShape->getSize());
                    aSize.Width = m_pImpl->nXSize;
                    aSize.Height = m_pImpl->nYSize;

                    m_xShape->setSize(aSize);

#ifdef DEBUG_DOMAINMAPPER
                    char buffer[256];
                    snprintf(buffer, sizeof(buffer),
                             "<shape width=\"%ld\" height=\"%ld\">",
                             aSize.Width, aSize.Height);
                    logger("DOMAINMAPPER", buffer);
                    logger("DOMAINMAPPER", "</shape>");
#endif
                    m_pImpl->bIsGraphic = true;
                }
#ifdef DEBUG_DOMAINMAPPER
                else
                    logger("DOMAINMAPPER", "<shape/>");
#endif

            }
            break;
        default: val.getInt();
    }
#ifdef DEBUG_DOMAINMAPPER
    logger("DOMAINMAPPER", string("</attribute>"));
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
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shprotation /*4*/:              break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockRotation /*119*/:       break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockAspectRatio /*120*/:    break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockPosition /*121*/:       break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockAgainstSelect /*122*/:  break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockCropping /*123*/:       break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockVertices /*124*/:       break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockText /*125*/:          break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockAdjustHandles /*126*/: break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockAgainstGrouping /*127*/: break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockAgainstGrouping /*127*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplTxid /*128*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpdxTextLeft /*129*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpdyTextTop /*130*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpdxTextRight /*131*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpdyTextBottom /*132*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpWrapText /*133*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpscaleText /*134*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpanchorText /*135*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shptxflTextFlow /*136*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpcdirFont /*137*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shphspNext /*138*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shptxdir /*139*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfSelectText /*187*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfAutoTextMargin /*188*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfRotateText /*189*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfFitShapeToText /*190*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfFitTextToShape /*191*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextUNICODE /*192*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextRTF /*193*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextAlign /*194*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextSize /*195*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextSpacing /*196*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFont /*197*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFReverseRows /*240*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfGtext /*241*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFVertical /*242*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFKern /*243*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFTight /*244*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFStretch /*245*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFShrinkFit /*246*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFBestFit /*247*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFNormalize /*248*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFDxMeasure /*249*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFBold /*250*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFItalic /*251*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFUnderline /*252*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFShadow /*253*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFSmallcaps /*254*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgtextFStrikethrough /*255*/:

        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shpcropFromTop /*256*/ : m_pImpl->nTopCrop   = nTwipValue; break;// rtf:shpcropFromTop
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shpcropFromBottom /*257*/ : m_pImpl->nBottomCrop= nTwipValue; break;// rtf:shpcropFromBottom
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shpcropFromLeft   /*258*/ : m_pImpl->nLeftCrop  = nTwipValue; break;// rtf:shpcropFromLeft
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shpcropFromRight/*259*/ : m_pImpl->nRightCrop = nTwipValue;break;// rtf:shpcropFromRight
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shppib/*260*/:  break;  // rtf:shppib
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shppibName/*261*/:  break;  // rtf:shppibName
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shppibFlags/*262*/:  // rtf:shppibFlags
        /*
         * // MSOBLIPFLAGS – flags for pictures
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
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppictureTransparent /*263*/:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shppictureContrast/*264*/: // rtf:shppictureContrast docu: "1<<16"
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
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shppictureBrightness/*265*/:  // rtf:shppictureBrightness
            m_pImpl->nBrightness     = ( (sal_Int32) nIntValue / 327 );
        break;
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
        case NS_dff::LN_shppictureGamma/*266*/: // rtf:shppictureGamma
            //todo check gamma value with _real_ document
            m_pImpl->fGamma = double(nIntValue/655);
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shppictureId        /*267*/:  break;  // rtf:shppictureId
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shppictureDblCrMod  /*268*/:  break;  // rtf:shppictureDblCrMod
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shppictureFillCrMod /*269*/:  break;  // rtf:shppictureFillCrMod
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shppictureLineCrMod /*270*/:  break;  // rtf:shppictureLineCrMod
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppibPrint /*271*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppibPrintName /*272*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppibPrintFlags /*273*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfNoHitTestPicture /*316*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppictureGray /*317*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppictureBiLevel /*318*/:

        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shppictureActive/*319*/: // rtf:shppictureActive
            switch( nIntValue & 0x06 )
            {
                case 0 : m_pImpl->eColorMode = drawing::ColorMode_STANDARD; break;
                case 4 : m_pImpl->eColorMode = drawing::ColorMode_GREYS; break;
                case 6 : m_pImpl->eColorMode = drawing::ColorMode_MONO; break;
                default:;
            }
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgeoLeft /*320*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgeoTop /*321*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgeoRight /*322*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpgeoBottom /*323*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshapePath /*324*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppVertices /*325*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppSegmentInfo /*326*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjustValue /*327*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust2Value /*328*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust3Value /*329*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust4Value /*330*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust5Value /*331*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust6Value /*332*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust7Value /*333*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust8Value /*334*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust9Value /*335*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpadjust10Value /*336*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfShadowOK /*378*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpf3DOK /*379*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLineOK /*380*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfGtextOK /*381*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfFillShadeShapeOK /*382*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfFillOK /*383*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillType /*384*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpfillColor           /*385*/:
            m_pImpl->nFillColor = (m_pImpl->nFillColor & 0xff000000) + ConversionHelper::ConvertColor( nIntValue );
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpfillOpacity         /*386*/:
        {
            sal_Int32 nTrans = 0xff - ( nIntValue * 0xff ) / 0xffff;
            m_pImpl->nFillColor = (nTrans << 0x18 ) + (m_pImpl->nFillColor & 0xffffff);
        }
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillBackColor /*387*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillBackOpacity /*388*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillCrMod /*389*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillBlip /*390*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillBlipName /*391*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillBlipFlags /*392*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillWidth /*393*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillHeight /*394*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillAngle /*395*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillFocus /*396*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillToLeft /*397*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillToTop /*398*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillToRight /*399*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillToBottom /*400*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillRectLeft /*401*/:
          /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillRectTop /*402*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillRectRight /*403*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillRectBottom /*404*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillDztype /*405*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillShadePreset /*406*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillShadeColors /*407*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillOriginX /*408*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillOriginY /*409*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillShapeOriginX /*410*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillShapeOriginY /*411*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillShadeType /*412*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfFilled /*443*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfHitTestFill /*444*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillShape /*445*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfillUseRect /*446*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpfNoFillHitTest      /*447*/:  break;  // rtf:shpfNoFillHitTest
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shplineColor           /*448*/:
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineColor = ConversionHelper::ConvertColor( nIntValue );
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineOpacity /*449*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineBackColor /*450*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineCrMod /*451*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineType /*452*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineFillBlip /*453*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineFillBlipName /*454*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineFillBlipFlags /*455*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineFillWidth /*456*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineFillHeight /*457*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineFillDztype /*458*/:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        case NS_dff::LN_shplineWidth           /*459*/:
            //1pt == 12700 units
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineWidth = ConversionHelper::convertTwipToMM100(nIntValue / 635);
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineMiterLimit /*460*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineStyle /*461*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shplineDashing         /*462*/:
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
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineDashStyle /*463*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineStartArrowhead /*464*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineEndArrowhead /*465*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineStartArrowWidth /*466*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineStartArrowLength /*467*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineEndArrowWidth /*468*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineEndArrowLength /*469*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineJoinStyle /*470*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineEndCapStyle /*471*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfArrowheadsOK /*507*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLine /*508*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfHitTestLine /*509*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplineFillShape /*510*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpfNoLineDrawDash     /*511*/:  break;  // rtf:shpfNoLineDrawDash
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowType /*512*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowColor /*513*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowHighlight /*514*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowCrMod /*515*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowOpacity /*516*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowOffsetX /*517*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowOffsetY /*518*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowSecondOffsetX /*519*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowSecondOffsetY /*520*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowScaleXToX /*521*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowScaleYToX /*522*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowScaleXToY /*523*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowScaleYToY /*524*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowPerspectiveX /*525*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowPerspectiveY /*526*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowWeight /*527*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowOriginX /*528*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpshadowOriginY /*529*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfShadow /*574*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfshadowObscured /*575*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveType /*576*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveOffsetX /*577*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveOffsetY /*578*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveScaleXToX /*579*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveScaleYToX /*580*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveScaleXToY /*581*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveScaleYToY /*582*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectivePerspectiveX /*583*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectivePerspectiveY /*584*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveWeight /*585*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveOriginX /*586*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpperspectiveOriginY /*587*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfPerspective /*639*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DSpecularAmt /*640*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DDiffuseAmt /*641*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DShininess /*642*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DEdgeThickness /*643*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DExtrudeForward /*644*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DExtrudeBackward /*645*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DExtrudePlane /*646*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DExtrusionColor /*647*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DCrMod /*648*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpf3D /*700*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfc3DMetallic /*701*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfc3DUseExtrusionColor /*702*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfc3DLightFace /*703*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DYRotationAngle /*704*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DXRotationAngle /*705*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DRotationAxisX /*706*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DRotationAxisY /*707*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DRotationAxisZ /*708*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DRotationAngle /*709*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DRotationCenterX /*710*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DRotationCenterY /*711*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DRotationCenterZ /*712*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DRenderMode /*713*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DTolerance /*714*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DXViewpoint /*715*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DYViewpoint /*716*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DZViewpoint /*717*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DOriginX /*718*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DOriginY /*719*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DSkewAngle /*720*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DSkewAmount /*721*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DAmbientIntensity /*722*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DKeyX /*723*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DKeyY /*724*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DKeyZ /*725*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DKeyIntensity /*726*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DFillX /*727*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DFillY /*728*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DFillZ /*729*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpc3DFillIntensity /*730*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfc3DConstrainRotation /*763*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfc3DRotationCenterAuto /*764*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfc3DParallel /*765*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfc3DKeyHarsh /*766*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfc3DFillHarsh /*767*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shphspMaster /*769*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpcxstyle /*771*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpbWMode /*772*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpbWModePureBW /*773*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpbWModeBW /*774*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfOleIcon /*826*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfPreferRelativeResize /*827*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfLockShapeType /*828*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfDeleteAttachedObject /*830*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfBackground /*831*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpspcot /*832*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpdxyCalloutGap /*833*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpspcoa /*834*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpspcod /*835*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpdxyCalloutDropSpecified /*836*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpdxyCalloutLengthSpecified /*837*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfCallout /*889*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfCalloutAccentBar /*890*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfCalloutTextBorder /*891*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfCalloutMinusX /*892*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfCalloutMinusY /*893*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfCalloutDropAuto /*894*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfCalloutLengthSpecified /*895*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpwzName /*896*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpwzDescription /*897*/: //alternative text
            m_pImpl->sAlternativeText = val.getString();
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shppihlShape /*898*/:
        case NS_dff::LN_shppWrapPolygonVertices/*899*/:  break;  // rtf:shppWrapPolygonVertices
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpdxWrapDistLeft /*900*/: // contains a twip/635 value
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustLRWrapForWordMargins()
            m_pImpl->nLeftMargin = nIntValue / 360;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpdyWrapDistTop /*901*/:  // contains a twip/635 value
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustULWrapForWordMargins()
            m_pImpl->nTopMargin = nIntValue / 360;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpdxWrapDistRight /*902*/:// contains a twip/635 value
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustLRWrapForWordMargins()
            m_pImpl->nRightMargin = nIntValue / 360;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpdyWrapDistBottom /*903*/:// contains a twip/635 value
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustULWrapForWordMargins()
            m_pImpl->nBottomMargin = nIntValue / 360;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shplidRegroup /*904*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfEditedWrap /*953*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfBehindDocument /*954*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfOnDblClickNotify /*955*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfIsButton /*956*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfOneD /*957*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
//        case NS_dff::LN_shpfHidden /*958*/:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_dff::LN_shpfPrint              /*959*/:  break;  // rtf:shpfPrint

        default:
            OSL_ASSERT("shape option unsupported?");
    }
}
/*-- 01.11.2006 09:45:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::sprm(Sprm & rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    logger("DOMAINMAPPER", string("<sprm>") + rSprm.toString());
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
        case NS_ooxml::LN_CT_Anchor_positionH: // 90976;
        case NS_ooxml::LN_CT_Anchor_positionV: // 90977;
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
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
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
            //depending on the behindDoc attribute text wraps through behind or in fron of the object
            m_pImpl->nWrap = text::WrapTextMode_THROUGHT;
        break;
        case NS_ooxml::LN_EG_WrapType_wrapTopAndBottom: // 90948;
            m_pImpl->nWrap = text::WrapTextMode_NONE;
        break;
        case NS_ooxml::LN_EG_WrapType_wrapThrough: // 90947;
            m_pImpl->nWrap = text::WrapTextMode_THROUGHT;
        break;
        case 0xf010:
        case 0xf011:
            //ignore - doesn't contain useful members
        break;
        case NS_ooxml::LN_CT_GraphicalObject_graphicData:// 90660;
            {
                m_pImpl->bIsGraphic = true;

                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                    pProperties->resolve(*this);
            }
        break;
        default:
            if( pValue.get() )
                pValue->getInt();
    }



#ifdef DEBUG_DOMAINMAPPER
    logger("DOMAINMAPPER", "</sprm>");
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

/*-- 01.11.2006 09:45:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::data(const sal_uInt8* buf, size_t len, writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
    try
    {
        uno::Reference< graphic::XGraphicProvider > xGraphicProvider(
                            m_xComponentContext->getServiceManager()->createInstanceWithContext(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.graphic.GraphicProvider")),
                                m_xComponentContext),
                            uno::UNO_QUERY_THROW );
        uno::Reference< io::XInputStream > xIStream = new XInputStreamHelper( buf, len, m_pImpl->bIsBitmap );

        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

        ::com::sun::star::beans::PropertyValues aMediaProperties( 1 );
        aMediaProperties[0].Name = rPropNameSupplier.GetName(PROP_INPUT_STREAM);
        aMediaProperties[0].Value <<= xIStream;
        uno::Reference< graphic::XGraphic > xGraphic = xGraphicProvider->queryGraphic( aMediaProperties );
        //
        if(xGraphic.is())
        {
            clog << "Graphic loaded" << endl;

            uno::Reference< beans::XPropertySet > xGraphicObjectProperties(
            m_xTextFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextGraphicObject"))),
            uno::UNO_QUERY_THROW);
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_GRAPHIC), uno::makeAny( xGraphic ));
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_ANCHOR_TYPE),
                uno::makeAny( m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE || m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR ?
                                    text::TextContentAnchorType_AT_CHARACTER :
                                    text::TextContentAnchorType_AS_CHARACTER ));
            m_xGraphicObject = uno::Reference< text::XTextContent >( xGraphicObjectProperties, uno::UNO_QUERY_THROW );

            //shapes have only one border, PICF might have four
            table::BorderLine aBorderLine;
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
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_ALTERNATIVE_TEXT ),
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
                if( m_pImpl->nXSize && m_pImpl->nYSize )
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SIZE),
                        uno::makeAny( awt::Size( m_pImpl->nXSize, m_pImpl->nYSize )));
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
    catch( const uno::Exception& )
    {
        clog << __FILE__ << __LINE__ << " failed!" << endl;
    }

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
/*-- 09.08.2007 10:17:00---------------------------------------------------

  -----------------------------------------------------------------------*/
bool    GraphicImport::IsGraphic() const
{
    return m_pImpl->bIsGraphic;
}

}
}
