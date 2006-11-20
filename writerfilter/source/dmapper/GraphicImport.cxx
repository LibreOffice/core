/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GraphicImport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2006-11-20 12:19:03 $
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
#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICPROVIDER_HPP_
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphicr.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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


#include <iostream>

namespace dmapper
{
using namespace ::std;
using namespace ::com::sun::star;
using namespace writerfilter;

class XInputStreamHelper : public cppu::WeakImplHelper1
<    io::XInputStream   >
{
    const sal_uInt8* pBuffer;
    const sal_Int32  nLength;
    sal_Int32        nPosition;
public:
    XInputStreamHelper(const sal_uInt8* buf, size_t len) :
        pBuffer( buf ),
        nLength( len ),
        nPosition( 0 )
        {
        }
    ~XInputStreamHelper();

    virtual ::sal_Int32 SAL_CALL readBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL available(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException);
};
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
    return readSomeBytes( aData, nBytesToRead);
}
/*-- 01.11.2006 13:56:21---------------------------------------------------

  -----------------------------------------------------------------------*/
::sal_Int32 XInputStreamHelper::readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    sal_Int32 nRet = 0;
    if( nMaxBytesToRead > 0 )
    {
        if( nMaxBytesToRead > nLength - nPosition )
            nRet = nLength - nPosition;
        else
            nRet = nMaxBytesToRead;
        aData.realloc( nRet );
        sal_Int8* pData = aData.getArray();
        memcpy( pData, pBuffer+nPosition, nRet );
        nPosition += nRet;
    }
    return nRet;
}
/*-- 01.11.2006 13:56:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void XInputStreamHelper::skipBytes( ::sal_Int32 nBytesToSkip ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    if( nBytesToSkip < 0 || nPosition + nBytesToSkip > nLength )
        throw io::BufferSizeExceededException();
    nPosition += nBytesToSkip;
}
/*-- 01.11.2006 13:56:22---------------------------------------------------

  -----------------------------------------------------------------------*/
::sal_Int32 XInputStreamHelper::available(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    return nLength - nPosition;
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
    sal_Int32   nLineType;
    sal_Int32   nLineColor;
    sal_Int32   nLineDistance;
    bool        bHasShadow;

    GraphicBorderLine() :
        nLineWidth(0)
        ,nLineType(0)
        ,nLineColor(0)
        ,nLineDistance(0)
        ,bHasShadow(false)
        {}

};
struct GraphicImport_Impl
{
    bool      bIsShapeImport;

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

    sal_Int32 nWrap;
    bool      bContour;
    bool      bIgnoreWRK;

    sal_Int32 nContrast;
    sal_Int32 nBrightness;
    double    fGamma;

    drawing::ColorMode eColorMode;

    GraphicBorderLine   aBorders[4];
    sal_Int32           nCurrentBorderLine;

    bool        bIsBitmap;
    bool        bIsTiff;
    sal_Int32   nBitsPerPixel;

    bool    bHoriFlip;
    bool    bVertFlip;

    bool      bInShapeOptionMode;
    sal_Int32 nShapeOptionType;

    GraphicImport_Impl(bool bIsShape) :
        bIsShapeImport( bIsShape )
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
        ,nCurrentBorderLine(BORDER_TOP)
        ,bIsBitmap(false)
        ,bIsTiff(false)
        ,nBitsPerPixel(0)
        ,bHoriFlip(false)
        ,bVertFlip(false)
        ,bInShapeOptionMode(false)
        ,nShapeOptionType(0)
        ,nWrap(0)
        ,bContour(false)
        ,bIgnoreWRK(true)
        ,nContrast(0)
        ,nBrightness(100)
        ,eColorMode( drawing::ColorMode_STANDARD )
        ,fGamma( -1.0 )
        {}
};
/*-- 01.11.2006 09:42:42---------------------------------------------------

  -----------------------------------------------------------------------*/
GraphicImport::GraphicImport(uno::Reference < uno::XComponentContext >    xComponentContext,
                             uno::Reference< lang::XMultiServiceFactory > xTextFactory,
                             bool bIsShape )
: m_pImpl( new GraphicImport_Impl( bIsShape ))
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
void GraphicImport::attribute(doctok::Id Name, doctok::Value & val)
{
    sal_Int32 nIntValue = val.getInt();
    /* WRITERFILTERSTATUS: table: PICFattribute */
    switch( Name )
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
            {
                switch(Name)
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
            doctok::Reference<Properties>::Pointer_t pProperties = val.getProperties();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
            }
                switch(Name)
                {
                    case NS_rtf::LN_shpopt:
                        m_pImpl->bInShapeOptionMode = false;
                    break;
                    default:;
                }
        }
        break;
        case NS_rtf::LN_BM_RCWINMF: //windows bitmap structure - if it's a bitmap
        break;
        case NS_rtf::LN_DXAGOAL: //x-size in twip
        case NS_rtf::LN_DYAGOAL: //y-size in twip
            break;
        case NS_rtf::LN_MX: m_pImpl->nHoriScaling = nIntValue; break;// hori scaling in 0.001%
        case NS_rtf::LN_MY: m_pImpl->nVertScaling = nIntValue; break;// vert scaling in 0.001%
        case NS_rtf::LN_DXACROPLEFT:    m_pImpl->nLeftCrop  = ConversionHelper::convertToMM100(nIntValue); break;// left crop in twips
        case NS_rtf::LN_DYACROPTOP:     m_pImpl->nTopCrop   = ConversionHelper::convertToMM100(nIntValue); break;// top crop in twips
        case NS_rtf::LN_DXACROPRIGHT:   m_pImpl->nRightCrop = ConversionHelper::convertToMM100(nIntValue); break;// right crop in twips
        case NS_rtf::LN_DYACROPBOTTOM:  m_pImpl->nBottomCrop = ConversionHelper::convertToMM100(nIntValue); break;// bottom crop in twips
        case NS_rtf::LN_BRCL:           break;//border type - legacy -
        case NS_rtf::LN_FFRAMEEMPTY:    break;// picture consists of a single frame
        case NS_rtf::LN_FBITMAP:        break;//1 if it's a bitmap ???
        case NS_rtf::LN_FDRAWHATCH:     break;//1 if it's an active OLE object
        case NS_rtf::LN_FERROR:         break;// 1 if picture is an error message
        case NS_rtf::LN_BPP: m_pImpl->nBitsPerPixel = nIntValue; break;//bits per pixel 0 - unknown, 1- mono, 4 - VGA

        case NS_rtf::LN_DXAORIGIN: //horizontal offset of hand annotation origin
        case NS_rtf::LN_DYAORIGIN: //vertical offset of hand annotation origin
        break;
        case NS_rtf::LN_CPROPS:break;// unknown - ignored
        //metafilepict
        case NS_rtf::LN_MM:
            m_pImpl->bIsBitmap = 99 == nIntValue ? true : false;
            m_pImpl->bIsTiff = 98 == nIntValue ? true : false;

        break; //mapmode
        case NS_rtf::LN_XEXT: m_pImpl->nXSize = nIntValue; break; // x-size
        case NS_rtf::LN_YEXT: m_pImpl->nYSize = nIntValue; break; // y-size
        case NS_rtf::LN_HMF: break; //identifier - ignored

        //sprm 0xf004 and 0xf008, 0xf00b
        case NS_rtf::LN_dfftype:// ignored
        case NS_rtf::LN_dffinstance:// ignored
        case NS_rtf::LN_dffversion://  ignored
        break;

        //sprm 0xf008
        case NS_rtf::LN_shptype:        break;
        case NS_rtf::LN_shpid:          break;
        case NS_rtf::LN_shpfGroup:      break;// This shape is a group shape
        case NS_rtf::LN_shpfChild:      break;// Not a top-level shape
        case NS_rtf::LN_shpfPatriarch:  break;// This is the topmost group shape. Exactly one of these per drawing.
        case NS_rtf::LN_shpfDeleted:    break;// The shape has been deleted
        case NS_rtf::LN_shpfOleShape:   break;// The shape is an OLE object
        case NS_rtf::LN_shpfHaveMaster: break;// Shape has a hspMaster property
        case NS_rtf::LN_shpfFlipH:       // Shape is flipped horizontally
            m_pImpl->bHoriFlip = nIntValue ? true : false;
        break;
        case NS_rtf::LN_shpfFlipV:       // Shape is flipped vertically
            m_pImpl->bVertFlip = nIntValue ? true : false;
        break;
        case NS_rtf::LN_shpfConnector:   break;// Connector type of shape
        case NS_rtf::LN_shpfHaveAnchor:  break;// Shape has an anchor of some kind
        case NS_rtf::LN_shpfBackground:  break;// Background shape
        case NS_rtf::LN_shpfHaveSpt:     break;// Shape has a shape type property

        case NS_rtf::LN_shptypename: break;// shape type name

        case NS_rtf::LN_shppid:     m_pImpl->nShapeOptionType = nIntValue; break; //type of shape option
        case NS_rtf::LN_shpfBid:    break;
        case NS_rtf::LN_shpfComplex:break;
        case NS_rtf::LN_shpop:
        {
            sal_Int32 nTwipValue = ConversionHelper::convertToMM100(nIntValue);
            switch( m_pImpl->nShapeOptionType )
            {
                case 256 : m_pImpl->nTopCrop   = nTwipValue; break;// rtf:shpcropFromTop
                case 257 : m_pImpl->nBottomCrop= nTwipValue; break;// rtf:shpcropFromBottom
                case 258 : m_pImpl->nLeftCrop  = nTwipValue; break;// rtf:shpcropFromLeft
                case 259 : m_pImpl->nRightCrop = nTwipValue;break;// rtf:shpcropFromRight
                case 260:  break;  // rtf:shppib
                case 261:  break;  // rtf:shppibName
                case 262:  // rtf:shppibFlags
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
                case 264: // rtf:shppictureContrast docu: "1<<16"
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
                case 265:  // rtf:shppictureBrightness
                    m_pImpl->nBrightness     = ( (sal_Int32) nIntValue / 327 );
                break;
                case 266: // rtf:shppictureGamma
                    m_pImpl->fGamma = double(nIntValue/655);
                break;
                case 267:  break;  // rtf:shppictureId
                case 268:  break;  // rtf:shppictureDblCrMod
                case 269:  break;  // rtf:shppictureFillCrMod
                case 270:  break;  // rtf:shppictureLineCrMod

                case 319: // rtf:shppictureActive
                    switch( nIntValue & 0x06 )
                    {
                        case 0 : m_pImpl->eColorMode = drawing::ColorMode_STANDARD; break;
                        case 4 : m_pImpl->eColorMode = drawing::ColorMode_GREYS; break;
                        case 6 : m_pImpl->eColorMode = drawing::ColorMode_MONO; break;
                        default:;
                    }
                break;
                case 385:  break;  // rtf:shpfillColor
                case 386:  break;  // rtf:shpfillOpacity
                case 447:  break;  // rtf:shpfNoFillHitTest
                case 511:  break;  // rtf:shpfNoLineDrawDash
                case 899:  break;  // rtf:shppWrapPolygonVertices
                case 959:  break;  // rtf:shpfPrint
                default:;
            }
        }
        break;
        case NS_rtf::LN_shpname:    break;
        case NS_rtf::LN_shpvalue:   break;

        //BLIP store entry
        case NS_rtf::LN_shpbtWin32: break;
        case NS_rtf::LN_shpbtMacOS: break;
        case NS_rtf::LN_shprgbUid:  break;
        case NS_rtf::LN_shptag:     break;
        case NS_rtf::LN_shpsize:    break;
        case NS_rtf::LN_shpcRef:    break;
        case NS_rtf::LN_shpfoDelay: break;
        case NS_rtf::LN_shpusage:   break;
        case NS_rtf::LN_shpcbName:  break;
        case NS_rtf::LN_shpunused2: break;
        case NS_rtf::LN_shpunused3: break;

        //border properties
        case NS_rtf::LN_shpblipbname : break;

        case NS_rtf::LN_DPTLINEWIDTH:  // 0x1759
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineWidth = nIntValue;
        break;
        case NS_rtf::LN_BRCTYPE:   // 0x175a
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineType = nIntValue;
        break;
        case NS_rtf::LN_ICO:   // 0x175b
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineColor = nIntValue;;
        break;
        case NS_rtf::LN_DPTSPACE:  // 0x175c
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineDistance = nIntValue;
        break;
        case NS_rtf::LN_FSHADOW:   // 0x175d
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].bHasShadow = nIntValue ? true : false;
        break;
        case NS_rtf::LN_FFRAME:            // ignored
        case NS_rtf::LN_UNUSED2_15: break;// ignored
        break;

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

    case NS_rtf::LN_SPID:
    case NS_rtf::LN_XALEFT: m_pImpl->nLeftPosition = ConversionHelper::convertToMM100(nIntValue); break; //left position
    case NS_rtf::LN_YATOP:  m_pImpl->nTopPosition = ConversionHelper::convertToMM100(nIntValue); break; //top position
    case NS_rtf::LN_XARIGHT:  m_pImpl->nRightPosition = ConversionHelper::convertToMM100(nIntValue); break; //right position
    case NS_rtf::LN_YABOTTOM: m_pImpl->nBottomPosition = ConversionHelper::convertToMM100(nIntValue); break;//bottom position
    case NS_rtf::LN_FHDR:
    case NS_rtf::LN_BX:
    case NS_rtf::LN_BY: break;
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
    case NS_rtf::LN_FRCASIMPLE:
    case NS_rtf::LN_FBELOWTEXT:
    case NS_rtf::LN_FANCHORLOCK:
    case NS_rtf::LN_CTXBX:
    {
        sal_Int32 nValue1 = val.getInt();
        nValue1++;
    }
    break;
/*    case NS_rtf::LN_CH = 10421;
    case NS_rtf::LN_UNUSED0_5 = 10422;
    case NS_rtf::LN_FLT = 10423;
    case NS_rtf::LN_shpLeft = 10424;
    case NS_rtf::LN_shpTop = 10425;
        break;*/
        default: val.getInt();
    }

}
/*-- 01.11.2006 09:45:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::sprm(doctok::Sprm & sprm_)
{
    sal_uInt32 nId = sprm_.getId();
    /* WRITERFILTERSTATUS: table: PICFsprmdata */
    switch(nId)
    {
        case 0xf004: //dff record
        case 0xf00a: //part of 0xf004 - shape properties
        case 0xf00b: //part of 0xf004
        case 0xf007:
        {
            doctok::Reference<Properties>::Pointer_t pProperties = sprm_.getProps();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
            }
        }
        break;
        case 0x271b:
        {
            doctok::Reference<BinaryObj>::Pointer_t pPictureData = sprm_.getBinary();
            if( pPictureData.get())
                pPictureData->resolve(*this);
        }
        break;
        case 0xf010:
        case 0xf011:
        case 0xf122:
            //ignore - doesn't contain useful members
        break;
        default:
            doctok::Value::Pointer_t pValue = sprm_.getValue();
            if( pValue.get() )
                pValue->getInt();
    }
}
/*-- 01.11.2006 09:45:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::entry(int /*pos*/, doctok::Reference<Properties>::Pointer_t /*ref*/)
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
void GraphicImport::data(const sal_uInt8* buf, size_t len, doctok::Reference<Properties>::Pointer_t /*ref*/)
{
    try
    {
        uno::Reference< graphic::XGraphicProvider > xGraphicProvider(
                            m_xComponentContext->getServiceManager()->createInstanceWithContext(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.graphic.GraphicProvider")),
                                m_xComponentContext),
                            uno::UNO_QUERY_THROW );
        uno::Reference< io::XInputStream > xIStream = new XInputStreamHelper( buf, len );

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
                uno::makeAny( m_pImpl->bIsShapeImport ?
                                    text::TextContentAnchorType_AT_CHARACTER :
                                    text::TextContentAnchorType_AS_CHARACTER ));
            m_xGraphicObject = uno::Reference< text::XTextContent >( xGraphicObjectProperties, uno::UNO_QUERY_THROW );
            if( m_pImpl->bIsShapeImport )
            {
                sal_Int32 nWidth = m_pImpl->nRightPosition - m_pImpl->nLeftPosition;
                sal_Int32 nHeight = m_pImpl->nBottomPosition - m_pImpl->nTopPosition;
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SIZE),
                    uno::makeAny( awt::Size( nWidth, nHeight )));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT          ),
                    uno::makeAny(text::HoriOrientation::NONE));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT_POSITTION),
                    uno::makeAny(m_pImpl->nLeftPosition));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT_RELATION ),
                    uno::makeAny(text::RelOrientation::PRINT_AREA));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT          ),
                    uno::makeAny(text::VertOrientation::NONE));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT_POSITTION),
                    uno::makeAny(m_pImpl->nTopPosition));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT_RELATION ),
                uno::makeAny(text::RelOrientation::PRINT_AREA));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_SURROUND ),
                    uno::makeAny(m_pImpl->nWrap));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_SURROUND_CONTOUR ),
                    uno::makeAny(m_pImpl->bContour));

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
        }
    }
    catch( const uno::Exception& )
    {
        clog << __FUNCTION__ << " failed!" << endl;
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
void GraphicImport::text(const sal_uInt8 * /*data*/, size_t /*len*/)
{
}
/*-- 01.11.2006 09:45:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::utext(const sal_uInt8 * /*data*/, size_t /*len*/)
{
}
/*-- 01.11.2006 09:45:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::props(doctok::Reference<Properties>::Pointer_t /*ref*/)
{
}
/*-- 01.11.2006 09:45:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::table(doctok::Id /*name*/, doctok::Reference<Table>::Pointer_t /*ref*/)
{
}
/*-- 01.11.2006 09:45:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::substream(doctok::Id /*name*/, ::doctok::Reference<Stream>::Pointer_t /*ref*/)
{
}
/*-- 01.11.2006 09:45:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void GraphicImport::info(const string & /*info*/)
{
}

}
