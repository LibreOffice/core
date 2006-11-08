/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GraphicImport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2006-11-08 09:50:27 $
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
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICPROVIDER_HPP_
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphicr.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
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
    sal_Int32 nHoriScaling;
    sal_Int32 nVertScaling;
    sal_Int32 nXSize;
    sal_Int32 nYSize;
    sal_Int32 nLeftCrop;
    sal_Int32 nTopCrop;
    sal_Int32 nRightCrop;
    sal_Int32 nBottomCrop;

    GraphicBorderLine   aBorders[4];
    sal_Int32           nCurrentBorderLine;

    bool        bIsBitmap;
    bool        bIsTiff;
    sal_Int32   nBitsPerPixel;

    bool    bHoriFlip;
    bool    bVertFlip;

    GraphicImport_Impl() :
        nHoriScaling(0)
        ,nVertScaling(0)
        ,nXSize(0)
        ,nYSize(0)
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
        {}
};
/*-- 01.11.2006 09:42:42---------------------------------------------------

  -----------------------------------------------------------------------*/
GraphicImport::GraphicImport(uno::Reference < uno::XComponentContext >    xComponentContext,
                             uno::Reference< lang::XMultiServiceFactory > xTextFactory )
: m_pImpl( new GraphicImport_Impl)
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
                    default:;
                }
            doctok::Reference<Properties>::Pointer_t pProperties = val.getProperties();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
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
        case NS_rtf::LN_DXACROPLEFT:    m_pImpl->nLeftCrop  = nIntValue; break;// left crop in twips
        case NS_rtf::LN_DYACROPTOP:     m_pImpl->nTopCrop   = nIntValue; break;// top crop in twips
        case NS_rtf::LN_DXACROPRIGHT:   m_pImpl->nRightCrop = nIntValue; break;// right crop in twips
        case NS_rtf::LN_DYACROPBOTTOM:  m_pImpl->nBottomCrop = nIntValue; break;// bottom crop in twips
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

        case NS_rtf::LN_shppid:     break;
        case NS_rtf::LN_shpfBid:    break;
        case NS_rtf::LN_shpfComplex:break;
        case NS_rtf::LN_shpop:      break;
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

//    case NS_rtf::LN_SPID = 10407;
//    case NS_rtf::LN_XALEFT = 10408;
//    case NS_rtf::LN_YATOP = 10409;
//    case NS_rtf::LN_XARIGHT = 10410;
//    case NS_rtf::LN_YABOTTOM = 10411;
//    case NS_rtf::LN_FHDR = 10412;
//    case NS_rtf::LN_BX = 10413;
//    case NS_rtf::LN_BY = 10414;
//    case NS_rtf::LN_WR = 10415;
//    case NS_rtf::LN_WRK = 10416;
//    case NS_rtf::LN_FRCASIMPLE = 10417;
//    case NS_rtf::LN_FBELOWTEXT = 10418;
//    case NS_rtf::LN_FANCHORLOCK = 10419;
//    case NS_rtf::LN_CTXBX = 10420;
//    case NS_rtf::LN_CH = 10421;
//    case NS_rtf::LN_UNUSED0_5 = 10422;
//    case NS_rtf::LN_FLT = 10423;
//    case NS_rtf::LN_shpLeft = 10424;
//    case NS_rtf::LN_shpTop = 10425;

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
        case 0xf010: //part of 0xf004 -
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

        ::com::sun::star::beans::PropertyValues aMediaProperties( 1 );
        aMediaProperties[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream"));
        aMediaProperties[0].Value <<= xIStream;
        uno::Reference< graphic::XGraphic > xGraphic = xGraphicProvider->queryGraphic( aMediaProperties );
        //
        if(xGraphic.is())
        {
            clog << "Graphic loaded" << endl;

            uno::Reference< beans::XPropertySet > xGraphicProperties(
            m_xTextFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextGraphicObject"))),
            uno::UNO_QUERY_THROW);
            xGraphicProperties->setPropertyValue(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Graphic")), uno::makeAny( xGraphic ));
            xGraphicProperties->setPropertyValue(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AnchorType")), uno::makeAny( text::TextContentAnchorType_AS_CHARACTER ));
            m_xGraphicObject = uno::Reference< text::XTextContent >( xGraphicProperties, uno::UNO_QUERY_THROW );
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
