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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <comphelper/extract.hxx>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/objsh.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <filter/msfilter/msocximex.hxx>
#include <osl/file.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/processfactory.hxx> // shouldn't be needed
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <algorithm>
#include <memory>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/componentcontext.hxx>
#include <unotools/streamwrap.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <svtools/filterutils.hxx>
// #TODO remove this when oox is used for control/userform import
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>

#ifndef C2S
#define C2S(cChar)	String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))
#endif
#ifndef C2U
#define C2U(cChar)	rtl::OUString::createFromAscii(cChar)
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace cppu;


#define WW8_ASCII2STR(s) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(s))
#define GRAPHOBJ_URLPREFIX "vnd.sun.star.GraphicObject:"


// #FIXME remove when oox is used for control import
// convertion class lifted from oox ( yes, duplication I know but
// should be very short term )
class GraphicHelper
{
public:
   GraphicHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    ~GraphicHelper();

    /** Returns information about the output device. */
    const ::com::sun::star::awt::DeviceInfo& getDeviceInfo() const;

    /** Converts the passed value from horizontal screen pixels to 1/100 mm. */
    sal_Int32           convertScreenPixelXToHmm( double fPixelX ) const;
    /** Converts the passed value from vertical screen pixels to 1/100 mm. */
    sal_Int32           convertScreenPixelYToHmm( double fPixelY ) const;
    /** Converts the passed point from screen pixels to 1/100 mm. */
    ::com::sun::star::awt::Point convertScreenPixelToHmm( const ::com::sun::star::awt::Point& rPixel ) const;
    /** Converts the passed size from screen pixels to 1/100 mm. */
    ::com::sun::star::awt::Size convertScreenPixelToHmm( const ::com::sun::star::awt::Size& rPixel ) const;

    /** Converts the passed value from 1/100 mm to horizontal screen pixels. */
    double              convertHmmToScreenPixelX( sal_Int32 nHmmX ) const;
    /** Converts the passed value from 1/100 mm to vertical screen pixels. */
    double              convertHmmToScreenPixelY( sal_Int32 nHmmY ) const;
    /** Converts the passed point from 1/100 mm to screen pixels. */
    ::com::sun::star::awt::Point convertHmmToScreenPixel( const ::com::sun::star::awt::Point& rHmm ) const;
    /** Converts the passed size from 1/100 mm to screen pixels. */
    ::com::sun::star::awt::Size convertHmmToScreenPixel( const ::com::sun::star::awt::Size& rHmm ) const;

    /** Converts the passed point from AppFont units to 1/100 mm. */
    ::com::sun::star::awt::Point convertAppFontToHmm( const ::com::sun::star::awt::Point& rAppFont ) const;
    /** Converts the passed point from AppFont units to 1/100 mm. */
    ::com::sun::star::awt::Size convertAppFontToHmm( const ::com::sun::star::awt::Size& rAppFont ) const;

    /** Converts the passed point from 1/100 mm to AppFont units. */
    ::com::sun::star::awt::Point convertHmmToAppFont( const ::com::sun::star::awt::Point& rHmm ) const;
    /** Converts the passed size from 1/100 mm to AppFont units. */
    ::com::sun::star::awt::Size convertHmmToAppFont( const ::com::sun::star::awt::Size& rHmm ) const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxGlobalFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxCompContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XUnitConversion > mxUnitConversion;
    ::com::sun::star::awt::DeviceInfo maDeviceInfo; /// Current output device info.
    double              mfPixelPerHmmX;             /// Number of screen pixels per 1/100 mm in X direction.
    double              mfPixelPerHmmY;             /// Number of screen pixels per 1/100 mm in Y direction.
};

inline sal_Int32 lclConvertScreenPixelToHmm( double fPixel, double fPixelPerHmm )
{
    return static_cast< sal_Int32 >( (fPixelPerHmm > 0.0) ? (fPixel / fPixelPerHmm + 0.5) : 0.0 );
}

GraphicHelper::GraphicHelper( const uno::Reference< frame::XModel >& rxModel )
{
    mxGlobalFactory = comphelper::getProcessServiceFactory();
    ::comphelper::ComponentContext aContext( mxGlobalFactory );
    mxCompContext = aContext.getUNOContext();

    // if no target frame has been passed (e.g. OLE objects), try to fallback to the active frame
    // TODO: we need some mechanism to keep and pass the parent frame
    uno::Reference< frame::XFrame > xFrame;
    if ( rxModel.is() )
    {
        uno::Reference< frame::XController > xController = rxModel->getCurrentController();
        xFrame = xController.is() ? xController->getFrame() : NULL;
    }
    if( !xFrame.is() && mxGlobalFactory.is() ) try
    {
        uno::Reference< frame::XFramesSupplier > xFramesSupp( mxGlobalFactory->createInstance( WW8_ASCII2STR( "com.sun.star.frame.Desktop" ) ), uno::UNO_QUERY_THROW );
        xFrame = xFramesSupp->getActiveFrame();
    }
    catch( uno::Exception& )
    {
    }

    // get the metric of the output device
    OSL_ENSURE( xFrame.is(), "GraphicHelper::GraphicHelper - cannot get target frame" );
    maDeviceInfo.PixelPerMeterX = maDeviceInfo.PixelPerMeterY = 3500.0; // some default just in case
    if( xFrame.is() ) try
    {
        uno::Reference< awt::XDevice > xDevice( xFrame->getContainerWindow(), uno::UNO_QUERY_THROW );
        mxUnitConversion.set( xDevice, uno::UNO_QUERY );
        OSL_ENSURE( mxUnitConversion.is(), "GraphicHelper::GraphicHelper - cannot get unit converter" );
        maDeviceInfo = xDevice->getInfo();
    }
    catch( uno::Exception& )
    {
        OSL_ENSURE( false, "GraphicHelper::GraphicHelper - cannot get output device info" );
    }
    mfPixelPerHmmX = maDeviceInfo.PixelPerMeterX / 100000.0;
    mfPixelPerHmmY = maDeviceInfo.PixelPerMeterY / 100000.0;
}

GraphicHelper::~GraphicHelper()
{
}

// Device info and device dependent unit conversion ---------------------------

const awt::DeviceInfo& GraphicHelper::getDeviceInfo() const
{
    return maDeviceInfo;
}

sal_Int32 GraphicHelper::convertScreenPixelXToHmm( double fPixelX ) const
{
    return lclConvertScreenPixelToHmm( fPixelX, mfPixelPerHmmX );
}

sal_Int32 GraphicHelper::convertScreenPixelYToHmm( double fPixelY ) const
{
    return lclConvertScreenPixelToHmm( fPixelY, mfPixelPerHmmY );
}

awt::Point GraphicHelper::convertScreenPixelToHmm( const awt::Point& rPixel ) const
{
    return awt::Point( convertScreenPixelXToHmm( rPixel.X ), convertScreenPixelYToHmm( rPixel.Y ) );
}

awt::Size GraphicHelper::convertScreenPixelToHmm( const awt::Size& rPixel ) const
{
    return awt::Size( convertScreenPixelXToHmm( rPixel.Width ), convertScreenPixelYToHmm( rPixel.Height ) );
}

double GraphicHelper::convertHmmToScreenPixelX( sal_Int32 nHmmX ) const
{
    return nHmmX * mfPixelPerHmmX;
}

double GraphicHelper::convertHmmToScreenPixelY( sal_Int32 nHmmY ) const
{
    return nHmmY * mfPixelPerHmmY;
}

awt::Point GraphicHelper::convertHmmToScreenPixel( const awt::Point& rHmm ) const
{
    return awt::Point(
        static_cast< sal_Int32 >( convertHmmToScreenPixelX( rHmm.X ) + 0.5 ),
        static_cast< sal_Int32 >( convertHmmToScreenPixelY( rHmm.Y ) + 0.5 ) );
}

awt::Size GraphicHelper::convertHmmToScreenPixel( const awt::Size& rHmm ) const
{
    return awt::Size(
        static_cast< sal_Int32 >( convertHmmToScreenPixelX( rHmm.Width ) + 0.5 ),
        static_cast< sal_Int32 >( convertHmmToScreenPixelY( rHmm.Height ) + 0.5 ) );
}

awt::Point GraphicHelper::convertAppFontToHmm( const awt::Point& rAppFont ) const
{
    if( mxUnitConversion.is() ) try
    {
        awt::Point aPixel = mxUnitConversion->convertPointToPixel( rAppFont, ::com::sun::star::util::MeasureUnit::APPFONT );
        return convertScreenPixelToHmm( aPixel );
    }
    catch( uno::Exception& )
    {
    }
    return awt::Point( 0, 0 );
}

awt::Size GraphicHelper::convertAppFontToHmm( const awt::Size& rAppFont ) const
{
    if( mxUnitConversion.is() ) try
    {
        awt::Size aPixel = mxUnitConversion->convertSizeToPixel( rAppFont, ::com::sun::star::util::MeasureUnit::APPFONT );
        return convertScreenPixelToHmm( aPixel );
    }
    catch( uno::Exception& )
    {
    }
    return awt::Size( 0, 0 );
}

awt::Point GraphicHelper::convertHmmToAppFont( const awt::Point& rHmm ) const
{
    if( mxUnitConversion.is() ) try
    {
        awt::Point aPixel = convertHmmToScreenPixel( rHmm );
        return mxUnitConversion->convertPointToLogic( aPixel, ::com::sun::star::util::MeasureUnit::APPFONT );
    }
    catch( uno::Exception& )
    {
    }
    return awt::Point( 0, 0 );
}

awt::Size GraphicHelper::convertHmmToAppFont( const awt::Size& rHmm ) const
{
    if( mxUnitConversion.is() ) try
    {
        awt::Size aPixel = convertHmmToScreenPixel( rHmm );
        return mxUnitConversion->convertSizeToLogic( aPixel, ::com::sun::star::util::MeasureUnit::APPFONT );
    }
    catch( uno::Exception& )
    {
    }
    return awt::Size( 0, 0 );
}


static char sWW8_form[] = "WW-Standard";


struct SortOrderByTabPos
{
    bool operator()( const OCX_Control* a, const OCX_Control* b )
    {
        return a->mnTabPos < b->mnTabPos;
    }
};


sal_uInt8 __READONLY_DATA OCX_Control::aObjInfo[4] = { 0x00, 0x12, 0x03, 0x00 };

long ReadAlign(SvStorageStream *pS, long nPos, int nAmount)
{
    if (long nAlign = nPos % nAmount)
    {

        long nLen = nAmount - nAlign;
        pS->SeekRel(nLen);
        return nLen;
    }
    return 0;
}

// NP - Images in controls in OO2.0/SO8 exist as links, e.g. they are not part of the document so are
// referenced externally. On import from ms document try to save images for controls here.
// Images are stored in directory called temp in the user installation directory. Next version of OO/SO
// hopefully will address this issue and allow a choice e.g. images for controls to be stored as links
// or embeded in the document.
uno::Reference< graphic::XGraphicObject> lcl_readGraphicObject( SotStorageStream *pS )
{
    uno::Reference< graphic::XGraphicObject > xGrfObj;
    uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    if( xServiceManager.is() )
    {
        try
        {
            // use the GraphicProvider service to get the XGraphic
            uno::Reference< graphic::XGraphicProvider > xGraphProvider(
                    xServiceManager->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.graphic.GraphicProvider" ) ), uno::UNO_QUERY );
            if( xGraphProvider.is() )
            {
                uno::Reference< io::XInputStream > xStream( new utl::OInputStreamWrapper( *pS ) );
                if( xStream.is() )
                {
                    uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
                    aMediaProps[0].Name = ::rtl::OUString::createFromAscii( "InputStream" );
                    aMediaProps[0].Value <<= xStream;
                    uno::Reference< graphic::XGraphic > xGraphic = xGraphProvider->queryGraphic( aMediaProps );
                    if( xGraphic.is() )
                    {
                        // create an XGraphicObject
                        ::comphelper::ComponentContext aContext( xServiceManager );
                        xGrfObj = graphic::GraphicObject::create( aContext.getUNOContext() );
                        xGrfObj->setGraphic(xGraphic);
                    }
                }
            }
        }
        catch( uno::Exception& )
        {
        }
    }
    return xGrfObj;
}


long WriteAlign(SvStorageStream *pS, int nAmount)
{
    if (long nAlign = pS->Tell() % nAmount)
    {
        long nLen = nAmount - nAlign;
        for (long i=0; i< nLen; ++i)
            *pS << sal_uInt8(0x00);
        return nLen;
    }
    return 0;
}
// string import/export =======================================================
/** #117832#  import of form control names
* control name is located in stream ("\3OCXNAME")
* a strings in "\3OCXNAME" stream seem to be terminated by 4 trailing bytes of 0's.
*                              ====
* Note: If the string in the stream is overwritten by a shorter string
* some characters from the original string may remain, the new string however
* will still be terminated in the same way e.g. by 4 bytes with value 0.
*/

bool writeOCXNAME( const OUString& sOCXName, SvStorageStream* pStream )
{
    const sal_Unicode* buffer = sOCXName.getStr();
    for ( sal_Int32 index=0; index < sOCXName.getLength(); index++ )
    {
        sal_uInt16 ch = static_cast< sal_uInt16 >( buffer[ index ] );
        *pStream << ch;
    }
    // write
    *pStream << sal_uInt32(0);
    return ( SVSTREAM_OK == pStream->GetError() );

}

bool readOCXNAME( OUString& sCName, SvStorageStream* pStream )
{
    /*
    * Read uniCode until no data or 0 encountered
    */
    OUStringBuffer buf(40);
    do
    {
        sal_uInt16 ch = 0;
        *pStream >> ch;
        sal_Unicode uni = static_cast< sal_Unicode >( ch );
        if ( uni == 0 )
        {
            break;
        }
        buf.append( &uni, 1 );

    } while ( !pStream->IsEof() );

    sCName = buf.makeStringAndClear();
    return ( SVSTREAM_OK == pStream->GetError() );
}


/*  #110435# (DR, 2003-11-12) ** Import of Unicode strings in form controls **

    Strings may be stored either as compressed or uncompressed Unicode
    character array. There are no encoded byte strings anywhere.

    The string length field stores the length of the character array (not the
    character count of the string) in the lower 31 bits, and the compression
    state in the highest bit.

    A set bit means the character array is compressed. This means all Unicode
    characters are <=0xFF. Therefore the high bytes of all characters are left
    out, and the character array size is equal to the string length.

    A cleared bit means the character array is not compressed. The buffer
    contains Little-Endian Unicode characters, and the resulting string length
    is half the buffer size.

    TODO: This implementation of the new string import is a hack to keep
    msocximex.hxx unchanged. A better implementation would replace the char*
    members of all classes by something more reasonable.
 */

namespace {

const sal_uInt32 SVX_MSOCX_SIZEMASK     = 0x7FFFFFFF;   /// Mask for character buffer size.
const sal_uInt32 SVX_MSOCX_COMPRESSED   = 0x80000000;   /// 1 = compressed Unicode array.


/** Returns true, if the passed length field specifies a compressed character array.
 */
inline bool lclIsCompressed( sal_uInt32 nLenFld )
{
    return (nLenFld & SVX_MSOCX_COMPRESSED) != 0;
}


/** Extracts and returns the memory size of the character buffer.
    @return  Character buffer size (may differ from resulting string length!).
 */
inline sal_uInt32 lclGetBufferSize( sal_uInt32 nLenFld )
{
    return nLenFld & SVX_MSOCX_SIZEMASK;
}


// import ---------------------------------------------------------------------

/** Reads the character array of a string in a form control.

    Creates a new character array containing the character data.
    The length field must be read before and passed to this function.
    Aligns stream position to multiple of 4 before.

    @param rStrm
        The input stream.

    @param rpcCharArr
        (out-param) Will point to the created character array,
        or will be 0 if string is empty. The array is NOT null-terminated.
        If the passed pointer points to an old existing array, it will be
        deleted before. Caller must delete the returned array.

    @param nLenFld
        The corresponding string length field read somewhere before.
 */
void lclReadCharArray( SvStorageStream& rStrm, char*& rpcCharArr, sal_uInt32 nLenFld, long nPos )
{
    delete[] rpcCharArr;
    rpcCharArr = 0;
    sal_uInt32 nBufSize = lclGetBufferSize( nLenFld );
    DBG_ASSERT( nBufSize <= 0xFFFF, "lclReadCharArray - possible read error: char array is too big" );
    if( nBufSize && nBufSize <= 0xFFFF )
    {
        rpcCharArr = new char[ nBufSize ];
        if( rpcCharArr )
        {
            ReadAlign( &rStrm, nPos, 4 );
            rStrm.Read( rpcCharArr, nBufSize );
        }
    }
}


/** Creates an OUString from a character array created with lclReadCharArray().

    The passed parameters must match, that means the length field must be the
    same used to create the passed character array.

    @param pcCharArr
        The character array returned by lclReadCharArray(). May be compressed
        or uncompressed, next parameter nLenFld will specify this.

    @param nLenFld
        MUST be the same string length field that has been passed to
        lclReadCharArray() to create the character array in previous parameter
        pcCharArr.

    @return
        An OUString containing the decoded string data. Will be empty if
        pcCharArr is 0.
 */
OUString lclCreateOUString( const char* pcCharArr, sal_uInt32 nLenFld )
{
    sal_uInt32 nBufSize = lclGetBufferSize( nLenFld );
    if( lclIsCompressed( nLenFld ) )
        return svt::BinFilterUtils::CreateOUStringFromStringArray( pcCharArr, nBufSize );

    return svt::BinFilterUtils::CreateOUStringFromUniStringArray( pcCharArr, nBufSize );
}

// export ---------------------------------------------------------------------

/** This class implements writing a character array from a Unicode string.

    Usage:
    1)  Construct an instance, either directly with an OUString, or with an UNO
        Any containing an OUString.
    2)  Check with HasData(), if there is something to write.
    3)  Write the string length field with WriteLenField() at the right place.
    4)  Write the encoded character array with WriteCharArray().
 */
class SvxOcxString
{
public:
    /** Constructs an empty string. String data may be set later by assignment. */
    inline explicit             SvxOcxString() : mnLenFld( 0 ) {}
    /** Constructs the string from the passed OUString. */
    inline explicit             SvxOcxString( const OUString& rStr ) { Init( rStr ); }
    /** Constructs the string from the passed UNO Any. */
    inline explicit             SvxOcxString( const uno::Any& rAny ) { Init( rAny ); }

    /** Assigns the passed string to the object. */
    inline SvxOcxString&        operator=( const OUString& rStr ) { Init( rStr ); return *this; }
    /** Assigns the string in the passed UNO Any to the object. */
    inline SvxOcxString&        operator=( const uno::Any& rAny ) { Init( rAny ); return *this; }

    /** Returns true, if the string contains at least one character to write. */
    inline bool                 HasData() const { return maString.getLength() > 0; }

    /** Writes the encoded 32-bit string length field. Aligns stream position to mult. of 4 before. */
    void                        WriteLenField( SvStorageStream& rStrm ) const;
    /** Writes the encoded character array. Aligns stream position to mult. of 4 before. */
    void                        WriteCharArray( SvStorageStream& rStrm ) const;

private:
    inline void                 Init( const OUString& rStr ) { maString = rStr; Init(); }
    void                        Init( const uno::Any& rAny );
    void                        Init();

    OUString                    maString;       /// The initial string data.
    sal_uInt32                  mnLenFld;       /// The encoded string length field.
};

void SvxOcxString::Init( const uno::Any& rAny )
{
    if( !(rAny >>= maString) )
        maString = OUString();
    Init();
}

void SvxOcxString::Init()
{
    mnLenFld = static_cast< sal_uInt32 >( maString.getLength() );
    bool bCompr = true;
    // try to find a character >= 0x100 -> character array will be stored uncompressed then
    if( const sal_Unicode* pChar = maString.getStr() )
        for( const sal_Unicode* pEnd = pChar + maString.getLength(); bCompr && (pChar < pEnd); ++pChar )
            bCompr = (*pChar < 0x100);
    if( bCompr )
        mnLenFld |= SVX_MSOCX_COMPRESSED;
    else
        mnLenFld *= 2;
}

void SvxOcxString::WriteLenField( SvStorageStream& rStrm ) const
{
    if( HasData() )
    {
        WriteAlign( &rStrm, 4);
        rStrm << mnLenFld;
    }
}

void SvxOcxString::WriteCharArray( SvStorageStream& rStrm ) const
{
    if( HasData() )
    {
        const sal_Unicode* pChar = maString.getStr();
        const sal_Unicode* pEnd = pChar + maString.getLength();
        bool bCompr = lclIsCompressed( mnLenFld );

        WriteAlign( &rStrm, 4);
        for( ; pChar < pEnd; ++pChar )
        {
            // write compressed Unicode (not encoded bytestring), or Little-Endian Unicode
            rStrm << static_cast< sal_uInt8 >( *pChar );
            if( !bCompr )
                rStrm << static_cast< sal_uInt8 >( *pChar >> 8 );
        }
    }
}

const sal_uInt16 USERFORM = (sal_uInt16)0xFF;
const sal_uInt16 STDCONTAINER = (sal_uInt16)0xFE;

const sal_uInt16 PAGE = (sal_uInt16)0x07;

const sal_uInt16 IMAGE = (sal_uInt16)0x0C;
const sal_uInt16 FRAME = (sal_uInt16)0x0E;

const sal_uInt16 SPINBUTTON = (sal_uInt16)0x10;
const sal_uInt16 CMDBUTTON = (sal_uInt16)0x11;
const sal_uInt16 TABSTRIP = (sal_uInt16)0x12;

const sal_uInt16 LABEL = (sal_uInt16)0x15;

const sal_uInt16 TEXTBOX = (sal_uInt16)0x17;
const sal_uInt16 LISTBOX = (sal_uInt16)0x18;
const sal_uInt16 COMBOBOX = (sal_uInt16)0x19;
const sal_uInt16 CHECKBOX = (sal_uInt16)0x1A;

const sal_uInt16 OPTIONBUTTON = (sal_uInt16)0x1B;
const sal_uInt16 TOGGLEBUTTON = (sal_uInt16)0x1C;

const sal_uInt16 SCROLLBAR = (sal_uInt16)0x2F;

const sal_uInt16 MULTIPAGE = (sal_uInt16)0x39;
// The IDs with bit 0x8000 set appear to be generated.
// It looks like these ID's are used with the non-toolbox [1]
// ActiveX controls that can be present in a Userform
// ( note: RefEdit seems to be an exception )
// In UserForm::Read just before the Container record starts
// you will notice there can be sometimes trailing records,
// it seems that these records have a 1:1 relationship with the non-toolbox
// controls present in the Userform. An id in the trailing record
// seems to identify the specific ActiveX control and an artificial nTypeIdent
// e.g. 0x8000, 0x8001 etc. is created so as to be able to associate
// the ActiveX control when referenced later
// [1] Such ActiveX controls are added via Tools/AddionalControls
// menu

// create a fixed set of those special id(s)
// ahem, we can only read one Progress bars at the moment so....
const sal_uInt16 PROGRESSBAR = (sal_uInt16)0x8000;

// A set of IDs from the trailing records mentioned above that seem to
// identify the following ActiveX controls
// Currently we only can process ( in a limited way ) the ProgressBar
// the other ID's are for reference ( & future )

// RefEdit control {00024512-0000-0000-c000-000000000046}
const sal_uInt8 aRefEditID[] =
{
0x12, 0x45, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46,
};

// Microsoft ProgressBar Control, version 6.0 {35053A22-8589-11D1-B16A-00C0F0283628}
const sal_uInt8 aProgressID[] =
{
0x22, 0x3a, 0x05, 0x35, 0x89, 0x85, 0xd1, 0x11,  0xb1, 0x6a, 0x00, 0xc0, 0xf0, 0x28, 0x36, 0x28,
};

// Calendar Control 10.0
const sal_uInt8 aCalendarID[] =
{
0x2b, 0xc9, 0x27, 0x8e, 0x64, 0x12, 0x1c, 0x10, 0x8a, 0x2f, 0x04, 0x02, 0x24, 0x00, 0x9c, 0x02,
};


// Microsoft ImageComboxBox Control, version 6.0 {DD9DA666-8594-11D1-B16A-00C0F0283628}
const sal_uInt8 aImageComboID[] =
{
0x66, 0xa6, 0x9d, 0xdd, 0x94, 0x85, 0xd1, 0x11, 0xb1, 0x6a, 0x00, 0xc0, 0xf0, 0x28, 0x36, 0x28,
};

// Microsoft ImageList Control, version 6.0 {2C247F23-8591-11D1-B16A-00C0F0283628}
const sal_uInt8 aImageListID[] =
{
0x23, 0x7f, 0x24, 0x2c, 0x91, 0x85, 0xd1, 0x11, 0xb1, 0x6a, 0x00, 0xc0, 0xf0, 0x28, 0x36, 0x28,
};

// Microsoft Slider Control, version 6.0 {F08DF954-8592-11D1-B16A-00C0F0283628}
const sal_uInt8 aSliderID[] =
{
0x54, 0xf9, 0x8d, 0xf0, 0x92, 0x85, 0xd1, 0x11, 0xb1, 0x6a, 0x00, 0xc0, 0xf0, 0x28, 0x36, 0x28,
};

// Microsoft StatusBar Control, version 6.0 {8E3867A3-8586-11D1-B16A-00C0F0283628}
const sal_uInt8 aStatusBarID[] =
{
0xa3, 0x67, 0x38, 0x8e, 0x86, 0x85, 0xd1, 0x11, 0xb1, 0x6a, 0x00, 0xc0, 0xf0, 0x28, 0x36, 0x28,
};

// Microsoft Office Chart 10.0
const sal_uInt8 aChartSpaceID[] =
{
0x46, 0xe5, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46,
};

const sal_Int16 ActiveXIDLen = 0x10; // CLSID len
const sal_Int16 ActiveXIDBlockLen = 0x30; // the block len that contains the CLSID

bool lcl_handleActiveXControl(  SvStorageStream *pS, sal_uInt16& nTypeID )
{
    nTypeID = 0; // Illegal ActiveX ID
    bool bRes = false;
    sal_uInt16 nIdentifier, nFixedAreaLen;
    *pS >> nIdentifier;
    *pS >> nFixedAreaLen;
    pS->SeekRel( ( nFixedAreaLen - ActiveXIDBlockLen ) );
    sal_uInt8 aID[ ActiveXIDLen ];
    if ( !pS->IsEof() )
    {
        pS->Read( aID, ActiveXIDLen );
        pS->SeekRel( ActiveXIDBlockLen - ActiveXIDLen ); // read remainer of record
        if ( memcmp( aID, aProgressID, ActiveXIDLen ) == 0 )
        {
            nTypeID = PROGRESSBAR;
            OSL_TRACE("Found supported ***PROGRESSBAR*** ActiveX control");
            bRes = true;
        }
#if (OSL_DEBUG_LEVEL > 0)
        // If we really want to process these more controls we should put them in
        // a list or array and have a single loop testing each id. For the moment
        // as we only can process PROGRESSBAR, not much point doing that until
        // we add support for at least another activex control

        else if ( memcmp( aID, aCalendarID, ActiveXIDLen ) == 0 )
            OSL_TRACE("Found unsupported ***CALENDAR*** ActiveX control");
        else if ( memcmp( aID, aRefEditID, ActiveXIDLen ) == 0 )
            OSL_TRACE("Found unsupported ***REFEDIT*** ActiveX control");
        else if ( memcmp( aID, aImageComboID, ActiveXIDLen ) == 0 )
            OSL_TRACE("Found unsupported ***IMAGECOMBO*** ActiveX control");
        else if ( memcmp( aID, aImageListID, ActiveXIDLen ) == 0 )
            OSL_TRACE("Found unsupported ***IMAGELIST*** ActiveX control");
        else if ( memcmp( aID, aChartSpaceID, ActiveXIDLen ) == 0 )
            OSL_TRACE("Found unsupported ***CHARTSPACE*** ActiveX control");
        else if ( memcmp( aID, aSliderID, ActiveXIDLen ) == 0 )
            OSL_TRACE("Found unsupported ***SLIDER*** ActiveX control");
        else if ( memcmp( aID, aStatusBarID, ActiveXIDLen ) == 0 )
            OSL_TRACE("Found unsupported ***STATUSBAR*** ActiveX control");
#endif
        else
        {
            OSL_TRACE("Unknown activeX ID !");
        }
    }
    return bRes;
}

typedef std::vector< ContainerRecord > ContainerRecordList;

class ContainerRecReader
{
    public:

    virtual ~ContainerRecReader() {}

    virtual bool Read( OCX_ContainerControl* pContainerControl, SvStorageStream *pS)
    {
        *pS >> nNoRecords;
        *pS >> nTotalLen;

        if ( isMultiPage )
        {
            if ( !handleMultiPageHdr( pS ) )
            {
                return false;
            }
        }
        else
        {
            if ( !handleStandardHdr( pS ) )
            {
                return false;
            }
        }

        records.clear();
        for (sal_uInt32 nRecord = 0; nRecord < nNoRecords; ++nRecord)
        {
            // DR #134146# redo loading of FrameChild data

            ContainerRecord rec;

            // record header
            sal_uInt16 nId, nSize;
            *pS >> nId >> nSize;
            sal_Size nStartPos = pS->Tell();

            // content flags
            sal_uInt32 nContentFlags;
            *pS >> nContentFlags;

            // length of control name
            sal_uInt32 nNameLen = 0;
            if( nContentFlags & 0x00000001 )
                *pS >> nNameLen;
            // length of control tag
            sal_uInt32 nTagLen = 0;
            if( nContentFlags & 0x00000002 )
                *pS >> nTagLen;
            // substorage id for frames
            if( nContentFlags & 0x00000004 )
                *pS >> rec.nSubStorageId;
            // help-context id
            if( nContentFlags & 0x00000008 )
                pS->SeekRel( 4 );
            // option flags
            if( nContentFlags & 0x00000010 )
            {
                sal_uInt32 nBitFlags = 0;
                *pS >> nBitFlags;
                rec.bVisible = ( ( nBitFlags & 0x02 ) == 0x02 );
            }
            // substream size
            if( nContentFlags & 0x00000020 )
                *pS >> rec.nSubStreamLen;
            // tabstop position
            if( nContentFlags & 0x00000040 )
                *pS >> rec.nTabPos;
            // control type
            if( nContentFlags & 0x00000080 )
                *pS >> rec.nTypeIdent;
            if( nContentFlags & 0x00000200 )
                pS->SeekRel( 4 ); // Grouping?
            // length of infotip
            sal_uInt32 nTipLen = 0;
            if( nContentFlags & 0x00000800 )
            {
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                *pS >> nTipLen;
            }

            sal_uInt32 nCntrlIdLen = 0;
            if( nContentFlags & 0x00001000 )
                *pS >> nCntrlIdLen;

            // length of control source name
            sal_uInt32 nCtrlSrcLen = 0;
            if( nContentFlags & 0x00002000 )
            {
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                *pS >> nCtrlSrcLen;
            }

            // length of row source name
            sal_uInt32 nRowSrcLen = 0;
            if( nContentFlags & 0x00004000 )
            {
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                *pS >> nRowSrcLen;
            }

            // control name
            sal_Char* pName = 0;
            sal_uInt32 nNameBufSize = lclGetBufferSize( nNameLen );
            if( nNameBufSize > 0 )
            {
                pName = new char[ nNameBufSize ];
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                pS->Read( pName, nNameBufSize );
            }
            // control tag
            sal_uInt32 nTagBufSize = lclGetBufferSize( nTagLen );
            if( nTagBufSize > 0 )
            {
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                pS->SeekRel( nTagBufSize );
            }

            // control position
            if( nContentFlags & 0x00000100 )
            {
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                *pS >> rec.nLeft >> rec.nTop;
            }

            // control infotip
            sal_uInt32 nTipBufSize = lclGetBufferSize( nTipLen );
            if( nTipBufSize > 0 )
            {
                std::auto_ptr< sal_Char > pTipName;
                pTipName.reset( new sal_Char[ nTipBufSize ] );
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                pS->Read( pTipName.get(), nTipBufSize );
                rec.controlTip = lclCreateOUString( pTipName.get(), nTipLen );
            }
            // control id
            sal_uInt32 nCntrlIdSize = lclGetBufferSize( nCntrlIdLen );
            if( nCntrlIdSize > 0 )
            {
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                pS->SeekRel( nCntrlIdSize );
            }
            // control source name
            sal_uInt32 nCtrlSrcBufSize = lclGetBufferSize( nCtrlSrcLen );
            if( nCtrlSrcBufSize > 0 )
            {
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                std::auto_ptr< sal_Char > pCtrlSrcName;
                pCtrlSrcName.reset( new sal_Char[ nCtrlSrcBufSize ] );
                pS->Read( pCtrlSrcName.get(), nCtrlSrcBufSize );
                rec.sCtrlSource = lclCreateOUString( pCtrlSrcName.get(), nCtrlSrcLen );
                OSL_TRACE("*** *** *** ControlSourceName -> %s ", rtl::OUStringToOString( rec.sCtrlSource, RTL_TEXTENCODING_UTF8 ).getStr() );
            }
            // row source name
            sal_uInt32 nRowSrcBufSize = lclGetBufferSize( nRowSrcLen );
            if( nRowSrcBufSize > 0 )
            {
                ReadAlign( pS, pS->Tell() - nStartPos, 4 );
                std::auto_ptr< sal_Char > pRowSrcName;
                pRowSrcName.reset( new sal_Char[ nRowSrcBufSize ] );
                pS->Read( pRowSrcName.get(), nRowSrcBufSize );
                rec.sRowSource =  lclCreateOUString( pRowSrcName.get(), nRowSrcLen );
                OSL_TRACE("*** *** *** RowSourceName -> %s ", rtl::OUStringToOString( rec.sRowSource, RTL_TEXTENCODING_UTF8 ).getStr() );
            }

            // seek to end of data
            pS->Seek( nStartPos + nSize );

            rec.cName = lclCreateOUString(pName, nNameLen);
            delete[] pName;

            OCX_Control* pControl = NULL;
            if( pContainerControl->createFromContainerRecord( rec, pControl ) &&
                pControl )
            {
                // propagate doc shell from parent
                pControl->pDocSh = pContainerControl->pDocSh;
                pContainerControl->ProcessControl( pControl, pS, rec );
            }
            else if ( rec.nTypeIdent & 0x8000 )
            {
                // Skip ActiveX Controls we can't import
                SotStorageStreamRef oStream = pContainerControl->getContainedControlsStream();
                ULONG nStrmPos = oStream->Tell();
                oStream->Seek( nStrmPos + rec.nSubStreamLen );
            }
            else
            {
                DBG_ERROR("Terminating import, unexpected error");
                return false;
            }
        }
        return true;
    }

    protected:
    ContainerRecReader() : isMultiPage(false){}
    bool isMultiPage;
    sal_uInt32 nNoRecords;
    sal_uInt32 nTotalLen;

    private:
    bool handleStandardHdr( SvStorageStream* pS )
    {
        sal_uInt8 aUnknown11[4];
        pS->Read(aUnknown11, sizeof(aUnknown11));
        // discovered a dialog with value of 0xFF for aUnknown11
        // needed an extra 4 bytes to offset correctly  into the control
        // records. Valid test or coincidence ?
        if ( aUnknown11[1] == 0xFF )
           pS->Read( aUnknown11, sizeof(aUnknown11));
        return true;
    }

    bool handleMultiPageHdr( SvStorageStream* pS )
    {
        sal_uInt32 nUnknown_32b; // unknown 32 bit structure, flags ?
        sal_uInt16 nUnknown_16b; // unknown 16 bit structure
        sal_uInt16 nMysteryLen; // lenght of unknown sub record

        *pS >> nUnknown_32b;
        *pS >> nUnknown_16b;
        *pS >> nMysteryLen;

        pS->SeekRel( nMysteryLen );
        return true;
    }
    ContainerRecordList records;
};

class StdContainerRecReader : public ContainerRecReader
{
    public:
    StdContainerRecReader(){}
};

class MultiPageContainerRecReader : public ContainerRecReader
{
    public:
    MultiPageContainerRecReader()
    {
        // NP ( 27-01-05 )
        // Strictly speaking this approach shouldn't be necessary.
        // It should be possible to have a common routine read the
        // container record array and by examining the flags present in
        // the record to determine we expect to read or not.
        // In this case for a MultPage control there is no Top or Left
        // values in the control record array, however time contraints
        // and associated risk prevent further investigation of this
        // at the moment.
        // similar situation exists for the start of the container record
        // which in the case of the MultiPage is different from
        // UserForm & Frame ( the other containers )

        isMultiPage = true; // tell the base class skip
    }
};

class ContainerRecordReaderFac
{
    public:
    static ContainerRecReader* instance( sal_uInt32 containerType )
    {
        switch( containerType )
        {
            case PAGE:
            case FRAME:
            case USERFORM:
            case STDCONTAINER:
                return new StdContainerRecReader();
            case MULTIPAGE:
                return new MultiPageContainerRecReader();
            default:
                DBG_ERROR("Illegal container type for factory");
                return NULL;
        }
    }
    private:
    ContainerRecordReaderFac();
};

} // namespace

// ============================================================================

class OCX_UserFormLabel : public OCX_Label
{
public:
    OCX_UserFormLabel(OCX_Control* pParent ) : OCX_Label( pParent )
    {
        mnForeColor = 0x80000012L;
        mnBackColor = 0x8000000FL;
    }
};


sal_uInt16 OCX_Control::nStandardId(0x0200);
sal_uInt16 OCX_FontData::nStandardId(0x0200);

sal_uInt32 OCX_Control::pColor[25] = {
0xC0C0C0, 0x008080, 0x000080, 0x808080, 0xC0C0C0, 0xFFFFFF, 0x000000,
0x000000, 0x000000, 0xFFFFFF, 0xC0C0C0, 0xC0C0C0, 0x808080, 0x000080,
0xFFFFFF, 0xC0C0C0, 0x808080, 0x808080, 0x000000, 0xC0C0C0, 0xFFFFFF,
0x000000, 0xC0C0C0, 0x000000, 0xFFFFC0 };

void OCX_Control::FillSystemColors()
{
    // overwrite the predefined colors with available system colors
    const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();

    pColor[ 0x00 ] = rSett.GetFaceColor().GetColor();
    pColor[ 0x01 ] = rSett.GetWorkspaceColor().GetColor();
    pColor[ 0x02 ] = rSett.GetActiveColor().GetColor();
    pColor[ 0x03 ] = rSett.GetDeactiveColor().GetColor();
    pColor[ 0x04 ] = rSett.GetMenuBarColor().GetColor();
    pColor[ 0x05 ] = rSett.GetWindowColor().GetColor();
    pColor[ 0x07 ] = rSett.GetMenuTextColor().GetColor();
    pColor[ 0x08 ] = rSett.GetWindowTextColor().GetColor();
    pColor[ 0x09 ] = rSett.GetActiveTextColor().GetColor();
    pColor[ 0x0A ] = rSett.GetActiveBorderColor().GetColor();
    pColor[ 0x0B ] = rSett.GetDeactiveBorderColor().GetColor();
    pColor[ 0x0C ] = rSett.GetWorkspaceColor().GetColor();
    pColor[ 0x0D ] = rSett.GetHighlightColor().GetColor();
    pColor[ 0x0E ] = rSett.GetHighlightTextColor().GetColor();
    pColor[ 0x0F ] = rSett.GetFaceColor().GetColor();
    pColor[ 0x10 ] = rSett.GetShadowColor().GetColor();
    pColor[ 0x12 ] = rSett.GetButtonTextColor().GetColor();
    pColor[ 0x13 ] = rSett.GetDeactiveTextColor().GetColor();
    pColor[ 0x14 ] = rSett.GetHighlightColor().GetColor();
    pColor[ 0x15 ] = rSett.GetDarkShadowColor().GetColor();
    pColor[ 0x16 ] = rSett.GetShadowColor().GetColor();
    pColor[ 0x17 ] = rSett.GetHelpTextColor().GetColor();
    pColor[ 0x18 ] = rSett.GetHelpColor().GetColor();
}

sal_uInt32 OCX_Control::ImportColor(sal_uInt32 nColor) const
{
    sal_uInt8 nUpper = (sal_uInt8)( nColor >> 24 );
    if (nUpper & 0x80) //Palette color, should be switch on bottom 24 bits
    {
        /*Might as well use my systems ones in the absence of any other ideas*/
        nColor = nColor&0x00FFFFFF;
        DBG_ASSERT (nColor <= 24,"Unknown Palette Index");
        if (nColor > 24)
            nColor = 0xFFFFFF;
        else
            nColor = pColor[nColor];
    }
    else
    {
        //Stored in bgr! rather than rgb
        nColor = SwapColor(nColor);
    }
    return nColor;
}

sal_Int16 OCX_FontData::ImportAlign(sal_uInt8 _nJustification) const
{
    sal_Int16 nRet;
    switch (_nJustification)
    {
    default:
    case 1:
        nRet = 0;
        break;
    case 2:
        nRet = 2;
        break;
    case 3:
        nRet = 1;
        break;
    }
    return nRet;
}

sal_uInt8 OCX_FontData::ExportAlign(sal_Int16 nAlign) const
{
    sal_Int8 nRet;
    switch (nAlign)
    {
    default:
    case 0:
        nRet = 1;
        break;
    case 2:
        nRet = 2;
        break;
    case 1:
        nRet = 3;
        break;
    }
    return nRet;
}

sal_uInt32 OCX_Control::SwapColor(sal_uInt32 nColor) const
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nColor&0xFF)),
        g(static_cast<sal_uInt8>(((nColor)>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nColor>>16)&0xFF));
    nColor = (r<<16) + (g<<8) + b;
    return nColor;
}

sal_uInt32 OCX_Control::ExportColor(sal_uInt32 nColor) const
{
    sal_uInt8 nUpper = (sal_uInt8)( nColor >> 24 );
    if (nUpper & 0x80) //Palette color, should be switch on bottom 24 bits
    {
        /*Might as well use my systems ones in the absence of any other ideas*/
        nColor = nColor&0x00FFFFFF;
        DBG_ASSERT (nColor <= 24,"Unknown Palette Index");
        if (nColor > 24)
            nColor = 0xFFFFFF;
        else
            nColor = pColor[nColor];
    }

    //Stored in bgr! rather than rgb
    nColor = SwapColor(nColor);
    return nColor;
}

sal_Bool OCX_Control::Import(
    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory,
    uno::Reference< form::XFormComponent >  &rFComp, awt::Size &rSz)
{

    if(msFormType.getLength() == 0)
        return sal_False;

    rSz.Width = nWidth;
    rSz.Height = nHeight;

    uno::Reference<uno::XInterface> xCreate =
        rServiceFactory->createInstance(msFormType);
    if (!xCreate.is())
        return sal_False;

    rFComp = uno::Reference<form::XFormComponent>(xCreate,uno::UNO_QUERY);
    if (!rFComp.is())
        return sal_False;
    uno::Reference<beans::XPropertySet> xPropSet(xCreate,uno::UNO_QUERY);
    if (!xPropSet.is())
        return sal_False;
    return Import(xPropSet);
}

sal_Bool OCX_Control::Import(uno::Reference<container::XNameContainer> &rDialog
    )
{
    uno::Reference<beans::XPropertySet > xDlgProps( rDialog, uno::UNO_QUERY);


    uno::Reference<lang::XMultiServiceFactory>
        xFactory(rDialog, uno::UNO_QUERY);

    uno::Reference<uno::XInterface> xCreate =
        xFactory->createInstance(msDialogType);
    if (!xCreate.is())
        return sal_False;

    uno::Reference<awt::XControlModel> xModel(xCreate, uno::UNO_QUERY);
    if (!xModel.is())
        return sal_False;

    /*  #147900# sometimes insertion of a control fails due to existing name,
        do not break entire form import then... */
    try
    {
        rDialog->insertByName(sName, uno::makeAny(xModel));
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE(
            ByteString( "OCX_Control::Import - cannot insert control \"" ).
            Append( ByteString( sName, RTL_TEXTENCODING_UTF8 ) ).
            Append( '"' ).GetBuffer() );
    }

    uno::Reference<beans::XPropertySet> xPropSet(xCreate, uno::UNO_QUERY);
    if (!xPropSet.is())
        return sal_False;

    if (!Import(xPropSet))
        return sal_False;

    uno::Any aTmp;

    GraphicHelper gHelper( pDocSh->GetModel() );

    awt::Point aAppFontPos = gHelper.convertHmmToAppFont( awt::Point( mnLeft, mnTop )  );
    aTmp <<= sal_Int32( aAppFontPos.X );
    xPropSet->setPropertyValue(WW8_ASCII2STR("PositionX"), aTmp);
    aTmp <<= sal_Int32( aAppFontPos.Y );
    xPropSet->setPropertyValue(WW8_ASCII2STR("PositionY"), aTmp);

    awt::Size aAppFontSize = gHelper.convertHmmToAppFont( awt::Size( nWidth, nHeight ) );
    aTmp <<= sal_Int32( aAppFontSize.Width ); // 100thmm
    xPropSet->setPropertyValue(WW8_ASCII2STR("Width"), aTmp);
    aTmp <<= sal_Int32( aAppFontSize.Height); //100th mm
    xPropSet->setPropertyValue(WW8_ASCII2STR("Height"), aTmp);
    if ( msToolTip.Len() > 0 )
        xPropSet->setPropertyValue(WW8_ASCII2STR("HelpText"), uno::Any(OUString(msToolTip)));

    if ( mnStep )
    {
        aTmp <<= mnStep;
        xPropSet->setPropertyValue(WW8_ASCII2STR("Step"), aTmp);
    }

    try
    {
        xPropSet->setPropertyValue(WW8_ASCII2STR("EnableVisible"), uno::makeAny( mbVisible ) );
    }
    catch( uno::Exception& )
    {
    }
    return sal_True;
}

sal_Int16 OCX_Control::ImportBorder(sal_uInt16 nSpecialEffect,
    sal_uInt16 nBorderStyle) const
{
    if ((nSpecialEffect == 0) && (nBorderStyle == 0))
        return 0;	//No Border
    else if ((nSpecialEffect == 0) && (nBorderStyle == 1))
        return 2;	//Flat Border
    return 1;	//3D Border
}

sal_uInt8 OCX_Control::ExportBorder(sal_uInt16 nBorder,sal_uInt8 &rBorderStyle)
    const
{
    sal_uInt8 nRet;
    switch(nBorder)
    {
        case 0:
            nRet = rBorderStyle = 0;
            break;
        default:
        case 1:
            nRet = 2;
            rBorderStyle = 0;
            break;
        case 2:
            nRet = 0;
            rBorderStyle = 1;
            break;
    }
    return nRet;
}

sal_Int16 OCX_Control::ImportSpecEffect( sal_uInt8 nSpecialEffect ) const
{
    return (nSpecialEffect == 0) ? 2 : 1;
}

sal_uInt8 OCX_Control::ExportSpecEffect( sal_Int16 nApiEffect ) const
{
    return (nApiEffect == 2) ? 0 : 2;
}

sal_Bool OCX_Control::ReadFontData(SvStorageStream *pS)
{
    return aFontData.Read(pS);
}


const uno::Reference< drawing::XDrawPage >&
    SvxMSConvertOCXControls::GetDrawPage()
{
    if( !xDrawPage.is() && pDocSh )
    {
        uno::Reference< drawing::XDrawPageSupplier > xTxtDoc(pDocSh->GetModel(),
            uno::UNO_QUERY);
        DBG_ASSERT(xTxtDoc.is(),"XDrawPageSupplier nicht vom XModel erhalten");
        xDrawPage = xTxtDoc->getDrawPage();
        DBG_ASSERT( xDrawPage.is(), "XDrawPage nicht erhalten" );
    }

    return xDrawPage;
}


const uno::Reference< lang::XMultiServiceFactory >&
    SvxMSConvertOCXControls::GetServiceFactory()
{
    if( !xServiceFactory.is() && pDocSh )
    {
        xServiceFactory = uno::Reference< lang::XMultiServiceFactory >
            (pDocSh->GetBaseModel(), uno::UNO_QUERY);
        DBG_ASSERT( xServiceFactory.is(),
                "XMultiServiceFactory nicht vom Model erhalten" );
    }

    return xServiceFactory;
}

const uno::Reference< drawing::XShapes >& SvxMSConvertOCXControls::GetShapes()
{
    if( !xShapes.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {

            xShapes = uno::Reference< drawing::XShapes >(xDrawPage,
                uno::UNO_QUERY);
            DBG_ASSERT( xShapes.is(), "XShapes nicht vom XDrawPage erhalten" );
        }
    }
    return xShapes;
}

const uno::Reference< container::XIndexContainer >&
    SvxMSConvertOCXControls::GetFormComps()
{
    if( !xFormComps.is() )
    {
        GetDrawPage();
        if( xDrawPage.is() )
        {
            uno::Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage,
                uno::UNO_QUERY );
            DBG_ASSERT( xFormsSupplier.is(),
                    "XFormsSupplier nicht vom XDrawPage erhalten" );

            uno::Reference< container::XNameContainer >  xNameCont =
                xFormsSupplier->getForms();

            // Das Formular bekommt einen Namen wie "WW-Standard[n]" und
            // wird in jedem Fall neu angelegt.
            UniString sName( sWW8_form, RTL_TEXTENCODING_MS_1252 );
            sal_uInt16 n = 0;

            while( xNameCont->hasByName( sName ) )
            {
                sName.AssignAscii( sWW8_form );
                sName += String::CreateFromInt32( ++n );
            }

            const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory
                = GetServiceFactory();
            if( !rServiceFactory.is() )
                return xFormComps;

            uno::Reference< uno::XInterface >  xCreate =
                rServiceFactory->createInstance(WW8_ASCII2STR(
                    "com.sun.star.form.component.Form"));
            if( xCreate.is() )
            {
                uno::Reference< beans::XPropertySet > xFormPropSet( xCreate,
                    uno::UNO_QUERY );

                uno::Any aTmp(&sName,getCppuType((OUString *)0));
                xFormPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

                uno::Reference< form::XForm > xForm( xCreate, uno::UNO_QUERY );
                DBG_ASSERT(xForm.is(), "keine Form?");

                uno::Reference< container::XIndexContainer > xForms( xNameCont,
                    uno::UNO_QUERY );
                DBG_ASSERT( xForms.is(), "XForms nicht erhalten" );

                aTmp.setValue( &xForm,
                    ::getCppuType((uno::Reference < form::XForm >*)0));
                xForms->insertByIndex( xForms->getCount(), aTmp );

                xFormComps = uno::Reference< container::XIndexContainer >
                    (xCreate, uno::UNO_QUERY);
            }
        }
    }

    return xFormComps;
}

sal_Bool OCX_CommandButton::Import( com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    // fake transparent push button by setting window background color
    if( !fBackStyle )
        mnBackColor = 0x80000005;
    aTmp <<= ImportColor(mnBackColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);

    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTemp = fWordWrap != 0;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        rPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aTmp = bool2any( mbTakeFocus );
    rPropSet->setPropertyValue( WW8_ASCII2STR( "FocusOnClick" ), aTmp );

    aFontData.Import(rPropSet);

    if ( sImageUrl.getLength() )
    {
        aTmp <<= sImageUrl;
        rPropSet->setPropertyValue( WW8_ASCII2STR("ImageURL"), aTmp);
    }
    return sal_True;
}

sal_Bool OCX_GroupBox::Export(SvStorageRef& /* rObj */,
    const uno::Reference< beans::XPropertySet >& /* rPropSet */,
    const awt::Size& /* rSize */ )
{
    sal_Bool bRet=sal_True;
    return bRet;
}

sal_Bool OCX_GroupBox::WriteContents(SvStorageStreamRef& /* rObj */,
    const uno::Reference< beans::XPropertySet >& /* rPropSet */,
    const awt::Size& /* rSize */)
{
    sal_Bool bRet=sal_True;
    return bRet;
}

sal_Bool OCX_CommandButton::WriteContents(SvStorageStreamRef& rContents,
    const uno::Reference< beans::XPropertySet >& rPropSet,
    const awt::Size& rSize )
{
    sal_Bool bRet=sal_True;

    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    if (fBackStyle)
        nTemp |= 0x08;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    nTemp = 0;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fWordWrap = any2bool(aTmp);
    if (fWordWrap)
        nTemp |= 0x80;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    aCaption.WriteLenField( *rContents );
    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);

    *rContents << rSize.Width;
    *rContents << rSize.Height;

    // "take focus on click" is directly in content flags, not in option field...
    mbTakeFocus = any2bool( rPropSet->getPropertyValue( WW8_ASCII2STR( "FocusOnClick" ) ) );

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    sal_uInt8 nTmp = 0x27;
    if (aCaption.HasData())
        nTmp |= 0x08;
    *rContents << nTmp;
    nTmp = 0x00;
    if( !mbTakeFocus )  // flag is set, if option is off
        nTmp |= 0x02;
    *rContents << nTmp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    DBG_ASSERT((rContents.Is() && (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_CommandButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x32, 0x05, 0xD7,
            0x69, 0xCE, 0xCD, 0x11, 0xA7, 0x77, 0x00, 0xDD,
            0x01, 0x14, 0x3C, 0x57, 0x22, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6d, 0x73, 0x20,
            0x32, 0x2e, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x6D,
            0x61, 0x6E, 0x64, 0x42, 0x75, 0x74, 0x74, 0x6F,
            0x6E, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
            0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
            0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x16, 0x00,
            0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
            0x43, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x42,
            0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00,
            0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x6D, 0x00,
        0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x42, 0x00,
        0x75, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));

    return WriteContents(xContents,rPropSet,rSize);
}

sal_Bool OCX_ImageButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);

    uno::Any aTmp=rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=0;//fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    WriteAlign(rContents,4);

    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    sal_uInt8 nTmp = 0x26;
    *rContents << nTmp;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    DBG_ASSERT((rContents.Is() && (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_ImageButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x32, 0x05, 0xD7,
            0x69, 0xCE, 0xCD, 0x11, 0xA7, 0x77, 0x00, 0xDD,
            0x01, 0x14, 0x3C, 0x57, 0x22, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6d, 0x73, 0x20,
            0x32, 0x2e, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x6D,
            0x61, 0x6E, 0x64, 0x42, 0x75, 0x74, 0x74, 0x6F,
            0x6E, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
            0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
            0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x16, 0x00,
            0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
            0x43, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x42,
            0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00,
            0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x6D, 0x00,
        0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x42, 0x00,
        0x75, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6F, 0x00,
        0x6E, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents,rPropSet,rSize);
}

bool lcl_isNamedRange( const rtl::OUString& sAddress, uno::Reference< frame::XModel >& xModel, table::CellRangeAddress& aAddress )
{
    bool bRes = false;
    const static rtl::OUString sNamedRanges( RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< sheet::XCellRangeReferrer > xReferrer;
    try
    {
        uno::Reference< beans::XPropertySet > xPropSet( xModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xNamed( xPropSet->getPropertyValue( sNamedRanges ), uno::UNO_QUERY_THROW );
        xReferrer.set ( xNamed->getByName( sAddress ), uno::UNO_QUERY );
    }
    catch( uno::Exception& /*e*/ )
    {
        // do nothing
    }
    if ( xReferrer.is() )
    {
        uno::Reference< sheet::XCellRangeAddressable > xRangeAddressable( xReferrer->getReferredCells(), uno::UNO_QUERY );
        if ( xRangeAddressable.is() )
        {
            aAddress = xRangeAddressable->getRangeAddress();
            bRes = true;
        }
    }
    return bRes;
}

void lcl_ApplyListSourceAndBindableStuff( uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& rPropSet, const rtl::OUString& rsCtrlSource, const rtl::OUString& rsRowSource )
{
// XBindable etc.
    uno::Reference< lang::XMultiServiceFactory > xFac;
    if ( xModel.is() )
        xFac.set( xModel, uno::UNO_QUERY );
    uno::Reference< form::binding::XBindableValue > xBindable( rPropSet, uno::UNO_QUERY );
    if (  xFac.is() && rsCtrlSource.getLength() && xBindable.is() )
    {

         // OOo address structures
         // RefCell - convert from XL
         // pretend we converted the imported string address into the
         // appropriate address structure
         uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( C2U( "com.sun.star.table.CellAddressConversion" )), uno::UNO_QUERY );
         table::CellAddress aAddress;
         if ( xConvertor.is() )
         {
             // we need this service to properly convert XL notation also
             // Should be easy to extend
             xConvertor->setPropertyValue( C2U( "XL_A1_Representation" ), uno::makeAny( rsCtrlSource ) );
             xConvertor->getPropertyValue( C2U( "Address" ) ) >>= aAddress;
         }

         beans::NamedValue aArg1;
         aArg1.Name = C2U("BoundCell");
         aArg1.Value <<= aAddress;

         uno::Sequence< uno::Any > aArgs(1);
         aArgs[ 0 ]  <<= aArg1;

         uno::Reference< form::binding::XValueBinding > xBinding( xFac->createInstanceWithArguments( C2U("com.sun.star.table.CellValueBinding" ), aArgs ), uno::UNO_QUERY );
         xBindable->setValueBinding( xBinding );
    }
    uno::Reference< form::binding::XListEntrySink > xListEntrySink( rPropSet, uno::UNO_QUERY );
    if (  xFac.is() && rsRowSource.getLength() && xListEntrySink.is() )
    {

         // OOo address structures
         // RefCell - convert from XL
         // pretend we converted the imported string address into the
         // appropriate address structure
         uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( C2U( "com.sun.star.table.CellRangeAddressConversion" )), uno::UNO_QUERY );
         table::CellRangeAddress aAddress;
         if ( xConvertor.is() )
         {
             if ( !lcl_isNamedRange( rsRowSource, xModel, aAddress ) )
             {
                 // we need this service to properly convert XL notation also
                 // Should be easy to extend
                 xConvertor->setPropertyValue( C2U( "XL_A1_Representation" ), uno::makeAny( rsRowSource ) );
                 xConvertor->getPropertyValue( C2U( "Address" ) ) >>= aAddress;
             }
         }

         beans::NamedValue aArg1;
         aArg1.Name = C2U("CellRange");
         aArg1.Value <<= aAddress;

         uno::Sequence< uno::Any > aArgs(1);
         aArgs[ 0 ]  <<= aArg1;

         uno::Reference< form::binding::XListEntrySource > xSource( xFac->createInstanceWithArguments( C2U("com.sun.star.table.CellRangeListSource" ), aArgs ), uno::UNO_QUERY );
         xListEntrySink->setListEntrySource( xSource );
    }
}

sal_Bool OCX_OptionButton::Import(com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet> &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    // background color: fBackStyle==0 -> transparent
    if( fBackStyle )
        aTmp <<= ImportColor(mnBackColor);
    else
        aTmp = uno::Any();
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTemp = fWordWrap != 0;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportSpecEffect( nSpecialEffect );
    rPropSet->setPropertyValue( WW8_ASCII2STR("VisualEffect"), aTmp);

    if (pValue)
    {
        INT16 nTmp = pValue[0]-0x30;
        aTmp <<= nTmp;
        if (!bSetInDialog)
            rPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
        else
        {
            // dialog ( but we might be using the form model )
            if ( rPropSet->getPropertySetInfo()->hasPropertyByName( WW8_ASCII2STR("DefaultState") ) )
                rPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
            else
                rPropSet->setPropertyValue( WW8_ASCII2STR("State"), aTmp);
        }
    }
    // If this is a dialog control then we need to  set a groupname *always*
    rtl::OUString sGroupName = lclCreateOUString( pGroupName, nGroupNameLen );
    if ( GetInDialog() ) // Userform/Dialog
    {
        // By default groupnames are not set in Excel, it's not unusual to have
        // a number of groups of radiobuttons located inside frame ( or other container
        // controls ) where there is *no* specific groupname set for the radiobuttons.
        // But... there is implicit grouping for radio buttons in seperate containers
        // e.g. radio buttons in a frame are by default in the same group.
        // Unfortunately in openoffice there are no containers below the dialog itself :-(
        // To ensure correct grouping for imported radiobuttons either with no groupname
        // or identical groupnames that are in separate containers we *must* ensure
        // that a suitable groupname is applied.
        // Because controlNames are unique even across different containers we can use the
        // controls container (e.g. parent) name as a prefix for a group name
    rtl::OUString sParentName = msParentName;
        sGroupName = sParentName.concat( C2U( ":" ) ).concat( sGroupName );
    }
    if ( sGroupName.getLength() == 0 )
        sGroupName = rtl::OUString::createFromAscii("DefaultGroup");
    OSL_TRACE("RadioButton %s has groupname %s",
        rtl::OUStringToOString( sName, RTL_TEXTENCODING_UTF8 ).getStr(),  rtl::OUStringToOString( sGroupName, RTL_TEXTENCODING_UTF8 ).getStr() );
        try
        {
            aTmp <<= sGroupName;
            rPropSet->setPropertyValue( WW8_ASCII2STR("GroupName"), aTmp);
        }
        catch( uno::Exception& )
        {
        }

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        rPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    // #i40279# always centered vertically
    aTmp <<= ::com::sun::star::style::VerticalAlignment_MIDDLE;
    rPropSet->setPropertyValue( WW8_ASCII2STR("VerticalAlign"), aTmp );

    uno::Reference< frame::XModel > xModel ( pDocSh ? pDocSh->GetModel() : NULL );
    lcl_ApplyListSourceAndBindableStuff( xModel, rPropSet, msCtrlSource, msRowSource );
    if ( sImageUrl.getLength() )
    {
        aTmp <<= sImageUrl;
        rPropSet->setPropertyValue( WW8_ASCII2STR("ImageURL"), aTmp);
    }

    aFontData.Import(rPropSet);
    return sal_True;
}

sal_Bool OCX_OptionButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;

    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    else
        fBackStyle = 0;

    sal_uInt8 nTemp=0;//=fEnabled;
    if (fEnabled)
        nTemp |= 0x02;
    if (fBackStyle)
        nTemp |= 0x08;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    nTemp = 0;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fWordWrap = any2bool(aTmp);
    if (fWordWrap)
        nTemp |= 0x80;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 5;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    WriteAlign(rContents,4);
    nValueLen = 1|SVX_MSOCX_COMPRESSED;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultState"));
    sal_Int16 nDefault = sal_Int16();
    aTmp >>= nDefault;
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;


    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    if (aCaption.HasData())
        pBlockFlags[2] |= 0x80;
    aCaption.WriteLenField( *rContents );

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("VisualEffect"));
    if (aTmp.hasValue())
    {
        sal_Int16 nApiSpecEffect = sal_Int16();
        aTmp >>= nApiSpecEffect;
        nSpecialEffect = ExportSpecEffect( nApiSpecEffect );
    }
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nDefault += 0x30;
    *rContents << sal_uInt8(nDefault);
    *rContents << sal_uInt8(0x00);

    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);
    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_OptionButton::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x21, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4F, 0x70, 0x74, 0x69,
        0x6F, 0x6E, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E,
        0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D, 0x62,
        0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F, 0x62,
        0x6A, 0x65, 0x63, 0x74, 0x00, 0x15, 0x00, 0x00,
        0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E, 0x4F,
        0x70, 0x74, 0x69, 0x6F, 0x6E, 0x42, 0x75, 0x74,
        0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
            0x4F, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00,
            0x6F, 0x00, 0x6E, 0x00, 0x42, 0x00, 0x75, 0x00,
            0x74, 0x00, 0x74, 0x00, 0x6F, 0x00, 0x6E, 0x00,
            0x31, 0x00, 0x00, 0x00, 0x00, 0x00
            };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}


sal_Bool OCX_TextBox::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    aTmp = bool2any( fEnabled != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp = bool2any( fLocked != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);

    aTmp = bool2any( fHideSelection != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR( "HideInactiveSelection" ), aTmp);

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportColor(mnBackColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    aTmp <<= ImportColor( nBorderColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("BorderColor"), aTmp);

    aTmp = bool2any( fMultiLine != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    sal_uInt16 nTmp = static_cast<sal_uInt16>(nMaxLength);
    aTmp <<= nTmp;
    rPropSet->setPropertyValue( WW8_ASCII2STR("MaxTextLen"), aTmp);


    sal_Bool bTemp1,bTemp2;
    uno::Any aBarsH,aBarsV;
    switch(nScrollBars)
    {
        case 1:
            bTemp1 = sal_True;
            bTemp2 = sal_False;
            break;
        case 2:
            bTemp1 = sal_False;
            bTemp2 = sal_True;
            break;
        case 3:
            bTemp1 = sal_True;
            bTemp2 = sal_True;
            break;
        case 0:
        default:
            bTemp1 = sal_False;
            bTemp2 = sal_False;
            break;
    }

    aBarsH = bool2any(bTemp1);
    aBarsV = bool2any(bTemp2);
    rPropSet->setPropertyValue( WW8_ASCII2STR("HScroll"), aBarsH);
    rPropSet->setPropertyValue( WW8_ASCII2STR("VScroll"), aBarsV);

    nTmp = nPasswordChar;
    aTmp <<= nTmp;
    rPropSet->setPropertyValue( WW8_ASCII2STR("EchoChar"), aTmp);

    if (pValue)
    {
        aTmp <<= lclCreateOUString( pValue, nValueLen );
        // DefaultText seems to no longer be in UnoEditControlModel
        if ( bSetInDialog )
        {
            rPropSet->setPropertyValue( WW8_ASCII2STR("Text"), aTmp);
        }
        else
        {
            rPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
        }
    }

    aFontData.Import(rPropSet);
    return sal_True;
}

sal_Bool OCX_TextBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    fMultiLine = any2bool(rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine")));
    fHideSelection = any2bool(rPropSet->getPropertyValue(WW8_ASCII2STR("HideInactiveSelection")));
    nTemp = 0x0C;
    if (fMultiLine)
        nTemp |= 0x80;
    if( fHideSelection )
        nTemp |= 0x20;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("MaxTextLen"));
    aTmp >>= nMaxLength;
    *rContents << nMaxLength;
    pBlockFlags[0] |= 0x08;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("HScroll"));
    sal_Bool bTemp1 = any2bool(aTmp);
    aTmp = rPropSet->getPropertyValue( WW8_ASCII2STR("VScroll"));
    sal_Bool bTemp2 = any2bool(aTmp);
    if (!bTemp1 && !bTemp2)
        nScrollBars =0;
    else if (bTemp1 && bTemp2)
        nScrollBars = 3;
    else if (!bTemp1 && bTemp2)
        nScrollBars = 2;
    else
        nScrollBars = 1;
    *rContents << nScrollBars;
    pBlockFlags[0] |= 0x20;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("EchoChar"));
    sal_uInt16 nTmp = sal_uInt16();
    aTmp >>= nTmp;
    nPasswordChar = static_cast<sal_uInt8>(nTmp);
    *rContents << nPasswordChar;
    pBlockFlags[1] |= 0x02;

    SvxOcxString aValue( rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    WriteAlign(rContents,4);
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BorderColor"));
    if (aTmp.hasValue())
        aTmp >>= nBorderColor;
    *rContents << ExportColor(nBorderColor);
    pBlockFlags[3] |= 0x02;

    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    aValue.WriteCharArray( *rContents );

    WriteAlign(rContents,4);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK == rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_TextBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x54, 0x65, 0x78, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x54, 0x65, 0x78, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x54, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_FieldControl::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    nTemp = 0x2C;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

#if 0 //Each control has a different Value format, and how to convert each to text has to be found out
    SvxOcxString aValue( rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;
#endif

    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

#if 0
    aValue.WriteCharArray( *rContents );
#endif

    WriteAlign(rContents,4);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}

sal_Bool OCX_FieldControl::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x54, 0x65, 0x78, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x54, 0x65, 0x78, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x54, 0x00, 0x65, 0x00, 0x78, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}



sal_Bool OCX_ToggleButton::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    aTmp = bool2any(true);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Toggle"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTemp = fWordWrap != 0;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    // fake transparent toggle button by setting window background color
    if( !fBackStyle )
        mnBackColor = 0x80000005;
    aTmp <<= ImportColor(mnBackColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    if (pValue)
    {
        INT16 nTmp=pValue[0]-0x30;
        //aTmp <<= nTmp == 1;
        aTmp <<= nTmp;
        rPropSet->setPropertyValue( WW8_ASCII2STR("State"), aTmp);
    }

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        rPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(rPropSet);

    if ( sImageUrl.getLength() )
    {
        aTmp <<= sImageUrl;
        rPropSet->setPropertyValue( WW8_ASCII2STR("ImageURL"), aTmp);
    }
    return sal_True;
}

sal_Bool OCX_ToggleButton::Export(
    SvStorageRef &rObj, const uno::Reference< beans::XPropertySet> &rPropSet,
    const awt::Size& rSize )
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x60, 0x1D, 0xD2, 0x8B,
            0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
            0x00, 0x60, 0x02, 0xF3, 0x21, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
            0x32, 0x2E, 0x30, 0x20, 0x54, 0x6F, 0x67, 0x67,
            0x6C, 0x65, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E,
            0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D, 0x62,
            0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F, 0x62,
            0x6A, 0x65, 0x63, 0x74, 0x00, 0x15, 0x00, 0x00,
            0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E, 0x54,
            0x6F, 0x67, 0x67, 0x6C, 0x65, 0x42, 0x75, 0x74,
            0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00, 0xF4, 0x39,
            0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x54, 0x00, 0x6F, 0x00, 0x67, 0x00, 0x67, 0x00,
        0x6C, 0x00, 0x65, 0x00, 0x42, 0x00, 0x75, 0x00,
        0x74, 0x00, 0x74, 0x00, 0x6F, 0x00, 0x6E, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));

    return WriteContents(xContents,rPropSet,rSize);
}

sal_Bool OCX_ToggleButton::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);

    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    if (fBackStyle)
        nTemp |= 0x08;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    nTemp = 0;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fWordWrap = any2bool(aTmp);
    if (fWordWrap)
        nTemp |= 0x80;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 6;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    WriteAlign(rContents,4);
    nValueLen = 1|SVX_MSOCX_COMPRESSED;
    bool bDefault = false;
    rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultState")) >>= bDefault;
    sal_uInt8 nDefault = static_cast< sal_uInt8 >( bDefault ? '1' : '0' );
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;

    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    aCaption.WriteLenField( *rContents );
    if (aCaption.HasData())
        pBlockFlags[2] |= 0x80;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    *rContents << nDefault;
    *rContents << sal_uInt8(0x00);

    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);
    bRet = aFontData.Export(rContents,rPropSet);
    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}

sal_Bool OCX_Label::Import(uno::Reference< beans::XPropertySet > &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    // background color: fBackStyle==0 -> transparent
    if( fBackStyle )
    {
        aTmp <<= ImportColor(mnBackColor);
    }
    else
    {
        // try fake transparent by using parents backColor
        if ( bSetInDialog && mpParent != NULL )
        {
            aTmp <<= ImportColor( mpParent->mnBackColor );
        }
        else
        {
            aTmp = uno::Any(); // use SO default
        }
    }
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    aTmp <<= ImportColor( nBorderColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("BorderColor"), aTmp);

    bTemp=fWordWrap;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        rPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    aFontData.Import(rPropSet);
    return sal_True;
}

sal_Bool OCX_ComboBox::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    aTmp = bool2any(fEnabled != 0);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp = bool2any(fLocked != 0);
    rPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);

    aTmp = bool2any( nDropButtonStyle != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("Dropdown"), aTmp);

    aTmp = bool2any( fHideSelection != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR( "HideInactiveSelection" ), aTmp);

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    if (pValue)
    {
        aTmp <<= lclCreateOUString( pValue, nValueLen );
        if ( bSetInDialog )
        {
            rPropSet->setPropertyValue( WW8_ASCII2STR("Text"), aTmp);
        }
        else
        {
            rPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
        }
    }

    aTmp <<= ImportColor(mnBackColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    aTmp <<= ImportColor( nBorderColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("BorderColor"), aTmp);

    sal_Int16 nTmp=static_cast<sal_Int16>(nMaxLength);
    aTmp <<= nTmp;
    rPropSet->setPropertyValue( WW8_ASCII2STR("MaxTextLen"), aTmp);

    aFontData.Import(rPropSet);
    uno::Reference< frame::XModel > xModel ( pDocSh ? pDocSh->GetModel() : NULL );
    lcl_ApplyListSourceAndBindableStuff( xModel, rPropSet, msCtrlSource, msRowSource );
    return sal_True;
}

sal_Bool OCX_ComboBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x00;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;


    sal_uInt8 nTemp=0x19;//fEnabled;
    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
        nTemp |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x48);
    *rContents << sal_uInt8(0x80);

    nTemp = 0x0C;
    fHideSelection = any2bool(rPropSet->getPropertyValue(WW8_ASCII2STR("HideInactiveSelection")));
    if( fHideSelection )
        nTemp |= 0x20;
    *rContents << nTemp;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    nStyle = 3;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    WriteAlign(rContents,2);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("LineCount"));
    aTmp >>= nListRows;
    *rContents << nListRows;
    pBlockFlags[1] |= 0x40;

    *rContents << sal_uInt8(1); //DefaultSelected One
    pBlockFlags[2] |= 0x01;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Dropdown"));
    nDropButtonStyle = any2bool(aTmp);
    if (nDropButtonStyle)
        nDropButtonStyle=0x02;
    *rContents << nDropButtonStyle;
    pBlockFlags[2] |= 0x04;

    SvxOcxString aValue( rPropSet->getPropertyValue(WW8_ASCII2STR("Text")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    WriteAlign(rContents,4);
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BorderColor"));
    if (aTmp.hasValue())
        aTmp >>= nBorderColor;
    *rContents << ExportColor(nBorderColor);
    pBlockFlags[3] |= 0x02;

    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    aValue.WriteCharArray( *rContents );

    WriteAlign(rContents,4);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_ComboBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x30, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1D, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x62,
        0x6F, 0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65,
        0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74,
        0x00, 0x11, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72,
        0x6D, 0x73, 0x2E, 0x43, 0x6F, 0x6D, 0x62, 0x6F,
        0x42, 0x6F, 0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x62, 0x00,
        0x6F, 0x00, 0x42, 0x00, 0x6F, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}



sal_Bool OCX_ListBox::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{

    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    sal_Bool bTmp=fEnabled;
    aTmp = bool2any(bTmp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTmp=fLocked;
    aTmp = bool2any(bTmp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    sal_Bool bTemp = nMultiState;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("MultiSelection"), aTmp);

#if 0		//Don't delete this for now until I figure out if I can make this
    if (pValue)
    {
        aTmp <<= lclCreateOUString( pValue, nValueLen );
        xPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
    }
#endif

    aTmp <<= ImportColor(mnBackColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    aTmp <<= ImportColor( nBorderColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("BorderColor"), aTmp);
    uno::Reference< frame::XModel > xModel ( pDocSh ? pDocSh->GetModel() : NULL );
    lcl_ApplyListSourceAndBindableStuff( xModel, rPropSet, msCtrlSource, msRowSource );
    aFontData.Import(rPropSet);
    return sal_True;
}

sal_Bool OCX_ListBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0x01;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ReadOnly"));
    fLocked = any2bool(aTmp);
    if (fLocked)
        nTemp |= 0x04;

    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    WriteAlign(rContents,2);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x10;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiSelection"));
    nMultiState = any2bool(aTmp);

    if (nMultiState)
    {
        *rContents << nMultiState;
        pBlockFlags[0] |= 0x20;
    }

    nStyle = 2;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;


    WriteAlign(rContents,4);

#if 0
    SvxOcxString aValue( rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultText")) );
    aValue.WriteLenField( *rContents );
    if (aValue.HasData())
        pBlockFlags[2] |= 0x40;

    WriteAlign(rContents,4);
#endif

    WriteAlign(rContents,4);
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BorderColor"));
    if (aTmp.hasValue())
        aTmp >>= nBorderColor;
    *rContents << ExportColor(nBorderColor);
    pBlockFlags[3] |= 0x02;

    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

#if 0
    aValue.WriteCharArray( *rContents );
#endif

    WriteAlign(rContents,4);

    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}

sal_Bool OCX_ListBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4C, 0x69, 0x73, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x4C, 0x69, 0x73, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x4C, 0x00, 0x69, 0x00, 0x73, 0x00, 0x74, 0x00,
        0x42, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x31, 0x00,
        0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_Control::Read(SvStorageStream *pS)
{
    sal_uInt16 nIdentifier, nFixedAreaLen;
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
        "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->SeekRel(nFixedAreaLen);
    return true;
}

sal_Bool OCX_ModernControl::Read(SvStorageStream *pS)
{
    long nStart = pS->Tell();
    *pS >> nIdentifier;
    DBG_ASSERT(nIdentifier==nStandardId,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,8);

    if (pBlockFlags[0] & 0x01)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;

        fEnabled = (nTemp & 0x02) >> 1;
        fLocked = (nTemp & 0x04) >> 2;
        fBackStyle = (nTemp & 0x08) >> 3;

        *pS >> nTemp;

        fColumnHeads = (nTemp & 0x04) >> 2;
        fIntegralHeight = (nTemp & 0x08) >> 3;
        fMatchRequired = (nTemp & 0x10) >> 4;
        fAlignment = (nTemp & 0x20) >> 5;

        *pS >> nTemp;

        fDragBehaviour = (nTemp & 0x08) >> 3;
        fEnterKeyBehaviour = (nTemp & 0x10) >> 4;
        fEnterFieldBehaviour = (nTemp & 0x20) >> 5;
        fTabKeyBehaviour = (nTemp & 0x40) >> 6;
        fWordWrap = (nTemp & 0x80) >> 7;

        *pS >> nTemp;
        fSelectionMargin = (nTemp & 0x04) >> 2;
        fAutoWordSelect = (nTemp & 0x08) >> 3;
        fAutoSize = (nTemp & 0x10) >> 4;
        fHideSelection = (nTemp & 0x20) >> 5;
        fAutoTab = (nTemp & 0x40) >> 6;
        fMultiLine = (nTemp & 0x80) >> 7;

    }

    /*If any of these are set they follow eachother in this order one after
    another padded out to the next U32 boundary with 0's
    U8 can abut each other U16 must start on a U16 boundary and are padded to
    that with 0's. A standardish word alignment structure*/

    if (pBlockFlags[0] & 0x02)
        *pS >> mnBackColor;
    if (pBlockFlags[0] & 0x04)
        *pS >> mnForeColor;
    if (pBlockFlags[0] & 0x08)
        *pS >> nMaxLength;

    if (pBlockFlags[0] & 0x10)
        *pS >> nBorderStyle;
    if (pBlockFlags[0] & 0x20)
        *pS >> nScrollBars;
    if (pBlockFlags[0] & 0x40)
        *pS >> nStyle;// (UI 0 == Data 3, UI 2 = Data 7)
    if (pBlockFlags[0] & 0x80)
        *pS >> nMousePointer;

    if (pBlockFlags[1] & 0x02)
        *pS >> nPasswordChar; //HUH ??? always 0 ??? not sure maybe just padding

    if (pBlockFlags[1] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nListWidth;
    }

    if (pBlockFlags[1] & 0x08)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nBoundColumn;
    }
    if (pBlockFlags[1] & 0x10)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nTextColumn;
    }
    if (pBlockFlags[1] & 0x20)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nColumnCount;
    }
    if (pBlockFlags[1] & 0x40)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nListRows;
    }
    if (pBlockFlags[1] & 0x80)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nUnknown8; //something to do with ColumnWidths
    }
    if (pBlockFlags[2] & 0x01)
        *pS >> nMatchEntry;
    if (pBlockFlags[2] & 0x02)
        *pS >> nListStyle;
    if (pBlockFlags[2] & 0x04)
        *pS >> nShowDropButtonWhen;
    if (pBlockFlags[2] & 0x10)
        *pS >> nDropButtonStyle;
    if (pBlockFlags[2] & 0x20)
        *pS >> nMultiState;

    bool bValue = (pBlockFlags[2] & 0x40) != 0;
    if (bValue)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nValueLen;
    }
    bool bCaption = (pBlockFlags[2] & 0x80) != 0;
    if (bCaption)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nCaptionLen;
    }
    if (pBlockFlags[3] & 0x01)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nHorzPos;
        *pS >> nVertPos;
    }
    if (pBlockFlags[3] & 0x02)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nBorderColor;
    }
    if (pBlockFlags[3] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4); // NEW
        *pS >> nSpecialEffect;
        pS->SeekRel( 3 );       // special effect is 32bit, not 8bit
    }
    if (pBlockFlags[3] & 0x08)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nIcon;
        DBG_ASSERT(nIcon == 0xFFFF, "Unexpected nIcon");
    }
    if (pBlockFlags[3] & 0x10)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nPicture;
        DBG_ASSERT(nPicture == 0xFFFF, "Unexpected nIcon");
    }
    if (pBlockFlags[3] & 0x20)
        *pS >> nAccelerator;
    /*
    if (pBlockFlags[3] & 0x80)
        *pS >> nUnknown9;
    */
    bool bGroupName = (pBlockFlags[4] & 0x01) != 0;
    if (bGroupName)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nGroupNameLen;
    }

    //End

    ReadAlign(pS, pS->Tell() - nStart, 4);
    *pS >> nWidth;
    *pS >> nHeight;

    if (bValue)
        lclReadCharArray( *pS, pValue, nValueLen, pS->Tell() - nStart);

    if (bCaption)
        lclReadCharArray( *pS, pCaption, nCaptionLen, pS->Tell() - nStart);

    if (bGroupName)
        lclReadCharArray( *pS, pGroupName, nGroupNameLen, pS->Tell() - nStart);

    ReadAlign(pS, pS->Tell() - nStart, 4);
    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        long imagePos = pS->Tell();
        mxGrfObj = lcl_readGraphicObject( pS );
        if( mxGrfObj.is() )
        {
            sImageUrl = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( GRAPHOBJ_URLPREFIX ) );
            sImageUrl = sImageUrl + mxGrfObj->getUniqueID();
        }
        // make sure the stream position should be pointing after the image
        pS->Seek( imagePos + nPictureLen );
    }

    return sal_True;
}


sal_Bool OCX_CommandButton::Read(SvStorageStream *pS)
{
    long nStart = pS->Tell();
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
        "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);


    if (pBlockFlags[0] & 0x01)
        *pS >> mnForeColor;
    if (pBlockFlags[0] & 0x02)
        *pS >> mnBackColor;

    if (pBlockFlags[0] & 0x04)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x02)>>1;
        fLocked = (nTemp&0x04)>>2;
        fBackStyle = (nTemp&0x08)>>3;
        *pS >> nTemp;
        *pS >> nTemp;
        fWordWrap = (nTemp&0x80)>>7;
        *pS >> nTemp;
        fAutoSize = (nTemp&0x10)>>4;
    }

    bool bCaption = (pBlockFlags[0] & 0x08) != 0;
    if (bCaption)
    {
        *pS >> nCaptionLen;
    }
    if (pBlockFlags[0] & 0x10) /*Picture Position, a strange mechanism here*/
    {
        *pS >> nVertPos;
        *pS >> nHorzPos;
    }

    if (pBlockFlags[0] & 0x40) /*MousePointer*/
        *pS >> nMousePointer;

    if (pBlockFlags[0] & 0x80)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nPicture;
    }

    if (pBlockFlags[1] & 0x01)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nAccelerator;
    }

    // "take focus on click" is directly in content flags, not in option field...
    mbTakeFocus = (pBlockFlags[1] & 0x02) == 0;     // option is on, if flag is not set

    if (pBlockFlags[1] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nIcon;
    }

    if (bCaption)
        lclReadCharArray( *pS, pCaption, nCaptionLen, pS->Tell() - nStart);

    ReadAlign(pS, pS->Tell() - nStart, 4);
    *pS >> nWidth;
    *pS >> nHeight;

    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        long imagePos = pS->Tell();
        mxGrfObj = lcl_readGraphicObject( pS );
        if( mxGrfObj.is() )
        {
            sImageUrl = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( GRAPHOBJ_URLPREFIX ) );
            sImageUrl = sImageUrl + mxGrfObj->getUniqueID();
        }
        // make sure the stream position should be pointing after the image
        pS->Seek( imagePos + nPictureLen );
    }

    return sal_True;
}

sal_Bool OCX_Label::Read(SvStorageStream *pS)
{
    long nStart = pS->Tell();
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);


    if (pBlockFlags[0] & 0x01)
        *pS >> mnForeColor;
    if (pBlockFlags[0] & 0x02)
        *pS >> mnBackColor;


    if (pBlockFlags[0] & 0x04)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x02)>>1;
        fLocked = (nTemp&0x04)>>2;
        fBackStyle = (nTemp&0x08)>>3;
        *pS >> nTemp;
        *pS >> nTemp;
        fWordWrap = (nTemp&0x80)>>7;
        *pS >> nTemp;
        fAutoSize = (nTemp&0x10)>>4;
    }
    bool bCaption = (pBlockFlags[0] & 0x08) != 0;
    if (bCaption)
        *pS >> nCaptionLen;

    if (pBlockFlags[0] & 0x10)
    {
        *pS >> nVertPos;
        *pS >> nHorzPos;
    }

    if (pBlockFlags[0] & 0x40)
        *pS >> nMousePointer;

    if (pBlockFlags[0] & 0x80)
    {
        ReadAlign(pS,pS->Tell() - nStart, 4);
        *pS >> nBorderColor;
    }

    if (pBlockFlags[1] & 0x01)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nBorderStyle;
    }

    if (pBlockFlags[1] & 0x02)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nSpecialEffect;
    }

    if (pBlockFlags[1] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nPicture;
    }

    if (pBlockFlags[1] & 0x08)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nAccelerator;
    }

    if (pBlockFlags[1] & 0x10)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nIcon;
    }

    if (bCaption)
        lclReadCharArray( *pS, pCaption, nCaptionLen, pS->Tell() - nStart);

    ReadAlign(pS, pS->Tell() - nStart, 4);
    *pS >> nWidth;
    *pS >> nHeight;

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        pPicture = new sal_uInt8[nPictureLen];
        pS->Read(pPicture,nPictureLen);
    }
    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    return sal_True;
}

TypeName::TypeName(sal_Char *pName, sal_uInt32 nStoreId, sal_uInt32 nLen, sal_uInt16 nType, sal_Int32 nLeft,
    sal_Int32 nTop)
    : msName(lclCreateOUString(pName, nLen)), mnType(nType), mnLeft(nLeft),
    mnTop(nTop),mnStoreId(nStoreId)
{
}

OCX_ContainerControl::OCX_ContainerControl( SotStorageRef& parent,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const uno::Reference< container::XNameContainer >  &rParent,
            OCX_Control* pParent ) :
                OCX_Control(sN, pParent), mxParent(rParent), nNoRecords(0), nTotalLen(0), containerType( STDCONTAINER )
{

    mContainerStorage = parent->OpenSotStorage(storageName,
        STREAM_READWRITE |
        STREAM_NOCREATE |
        STREAM_SHARE_DENYALL);
    mContainerStream = mContainerStorage->OpenSotStream(
        String(RTL_CONSTASCII_STRINGPARAM("f"),
        RTL_TEXTENCODING_MS_1252),
        STREAM_STD_READ | STREAM_NOCREATE);
    mContainedControlsStream = mContainerStorage->OpenSotStream( String(RTL_CONSTASCII_STRINGPARAM("o"),
        RTL_TEXTENCODING_MS_1252),
        STREAM_STD_READ | STREAM_NOCREATE);
}
OCX_ContainerControl::~OCX_ContainerControl()
{
    CtrlIterator aEnd = mpControls.end();
    for (CtrlIterator aIter = mpControls.begin(); aIter != aEnd; ++ aIter )
    {
        delete *aIter;
    }
}

// Really import should receive the parent e.g. a Userform, Frame or Multi Page
// and call import on its containees with itself  ( up-called from
// the base class ) but... the reality is we have no containment model
// so we make sure rPropSet is always the parent Dialog

sal_Bool OCX_ContainerControl::Import(uno::Reference<beans::XPropertySet>& /* rProps */ )
{
    if ( !mxParent.is() )
    {
        return sal_False;
    }
    CtrlIterator aEnd = mpControls.end();
//    int count = 0;
    for (CtrlIterator aIter = mpControls.begin(); aIter != aEnd; ++ aIter )
    {
        if ( !(*aIter)->Import( mxParent ) )
        {
            return sal_False;
        }
    }
    return sal_True;
}

OUString OCX_ContainerControl::createSubStreamName( const sal_uInt32& subStorageId )
{
    static OUString sI = OUString::createFromAscii("i");
    static OUString sZero = OUString::createFromAscii( "0" );
    OUStringBuffer buf( 6 );
    buf.append( sI );
    // for subStorage id < 10 stream name has leading '0'
    // eg "i07"
    if ( subStorageId < 10 )
    {
        buf.append( sZero );
    }
    buf.append( OUString::valueOf( (sal_Int32)subStorageId ) );
    return buf.makeStringAndClear();
}


bool OCX_ContainerControl::createFromContainerRecord( ContainerRecord& record, OCX_Control*& pControl )
{
    pControl = NULL;
    if (  record.nTypeIdent & 0x8000 )
    {
        std::hash_map<sal_uInt16, sal_uInt16>::iterator it = mActiveXIDMap.find( record.nTypeIdent );
        if ( it == mActiveXIDMap.end() )
            return false;
        // replace the generated id with our hardcoded one
        record.nTypeIdent = it->second;
    }
    switch ( record.nTypeIdent)
        {
            case CMDBUTTON:
                pControl = new OCX_CommandButton;
                break;
            case LABEL:
                pControl = new OCX_UserFormLabel(this);
                break;
            case TEXTBOX:
                pControl = new OCX_TextBox;
                break;
            case LISTBOX:
                pControl = new OCX_ListBox;
                break;
            case COMBOBOX:
                pControl = new OCX_ComboBox;
                break;
            case CHECKBOX:
                pControl =  new OCX_CheckBox;
                break;
            case OPTIONBUTTON:
                pControl = new OCX_OptionButton;
                break;
            case TOGGLEBUTTON:
                pControl = new OCX_ToggleButton;
                break;
            case IMAGE: //Image
            {
                pControl = new OCX_Image;
                break;
            }
            case PAGE: // Page
            {
                OUString sMSStore = createSubStreamName( record.nSubStorageId );
                pControl = new OCX_Page(mContainerStorage, sMSStore,
                    record.cName, mxParent, this);
                break;
            }
            case MULTIPAGE: // MultiPage
            {
                OUString sMSStore = createSubStreamName( record.nSubStorageId );
                pControl = new OCX_MultiPage( mContainerStorage, sMSStore,
                    record.cName, mxParent, this);
                break;
            }
            case FRAME:  //Frame
            {
                OUString sFrameStore = createSubStreamName( record.nSubStorageId );
                pControl = new OCX_Frame(mContainerStorage, sFrameStore,
                    record.cName, mxParent, this);

                break;
            }
            case SPINBUTTON: //SpinButton
            {
                pControl = new OCX_SpinButton;
                break;
            }
            case TABSTRIP: //TabStrip
            {
                pControl = new OCX_TabStrip;
                break;
            }
            case SCROLLBAR: //ScrollBar
                pControl = new OCX_ScrollBar;
                break;
            case PROGRESSBAR: //ProgressBar Active X control
                pControl = new OCX_ProgressBar;
                break;
            default:
                OSL_TRACE( "**** Unknown control 0x%x", record.nTypeIdent );
                DBG_ERROR( "Unknown control");
                return false;
        }
        pControl->sName = record.cName;
        return true;
}


void addSeperator( std::vector< OCX_Control* >& dest )
{
    OCX_Control* seperator = new OCX_CommandButton;
    seperator->SetInDialog(true);
    seperator->sName = C2S("GroupSeperator");
    dest.push_back( seperator );
}

void addRButtons( std::vector< OCX_Control* >& src,
                  std::vector< OCX_Control* >& dest,
                  bool addGroupSeperator )
{
    if ( addGroupSeperator )
    {
        addSeperator( dest );
    }

    for ( CtrlIterator rbIter = src.begin(); rbIter != src.end(); ++rbIter )
    {
        dest.push_back( *rbIter );
    }
}

void OCX_ContainerControl::ProcessControl(OCX_Control* pControl,SvStorageStream* /* pS */,  ContainerRecord& rec )
{
    SotStorageStreamRef oStream = mContainedControlsStream;

    // can insert into OO Dialog (e.g is this a supported dialog control)??
    if ( rec.nTypeIdent == TABSTRIP )
    {
        // skip the record in the stream, discard the control
        oStream->SeekRel( rec.nSubStreamLen );
        delete pControl;
    }
    else
    {
        // A container control needs to read the f stream in
        // the folder ( substorage ) associated with this control
        switch ( rec.nTypeIdent )
        {
            case FRAME:
            case MULTIPAGE:
            case PAGE:
                {
                    OCX_ContainerControl* pContainer =
                        static_cast< OCX_ContainerControl* >( pControl );
                    oStream = pContainer->getContainerStream();
                    break;
                }
            case LISTBOX:
            case OPTIONBUTTON:
            case COMBOBOX:
            case SPINBUTTON:
            case SCROLLBAR:
                {
                    pControl->msCtrlSource = rec.sCtrlSource;
                    pControl->msRowSource = rec.sRowSource;
                }
        }
        pControl->sName = rec.cName;
        pControl->msToolTip = rec.controlTip;
        // Position of controls is relative to the container
        pControl->mnTop = rec.nTop + mnTop;
        pControl->mnLeft = rec.nLeft + mnLeft;
        // MS tabIndex, pretty useless in OpenOffice land
        // as tab indexes in MS are relative to parent container.
        // However we need this value in order to set
        // OpenOffice tab indices in a sensible way to
        // reflect the ms tabbing from orig MS UserForm, see below
        pControl->mnTabPos = rec.nTabPos;
        pControl->SetInDialog(true);
        pControl->mbVisible = rec.bVisible;
        if ( mnStep )
        {
            // If the container has a step then it should be
            // applied to all containees
            pControl->mnStep = mnStep;
        }
        pControl->msParentName = sName;

        // #117490# DR: container records provide size of substream, use it here...

        // remember initial position to set correct stream position
        ULONG nStrmPos = oStream->Tell();
        // import control, may return with invalid stream position
        pControl->FullRead(oStream);
        // set stream to position behind substream of this control
        oStream->Seek( nStrmPos + rec.nSubStreamLen );

        mpControls.push_back( pControl );
    }
}

sal_Bool OCX_ContainerControl::Read(SvStorageStream *pS)
{

    if ( mpParent )
    {
        mnBackColor = mpParent->mnBackColor;
    }

    std::auto_ptr< ContainerRecReader > reader (
        ContainerRecordReaderFac::instance( containerType ) );

    reader->Read( this, pS );
    // Need to honour the MS Tab Indexes. However MS tab indexes are
    // relative to parent, this hack sorts the controls in each container
    // based on the ms tab indexes. When import is called we create the
    // controls in Open/Star office based on the order of the tab indexes,
    // this ensures that the default tab index created by Star/Open office
    // reflects the "flattened" ms tab order.
    ::std::sort( mpControls.begin(), mpControls.end(), SortOrderByTabPos() );
    return true;
}

OCX_MultiPage::OCX_MultiPage( SotStorageRef& parent,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const uno::Reference< container::XNameContainer >  &rDialog,
            OCX_Control* pParent):
        OCX_ContainerControl(parent, storageName, sN, rDialog, pParent ), fUnknown1(0), fEnabled(1),
        fLocked(0), fBackStyle(1), fWordWrap(1), fAutoSize(0), nCaptionLen(0),
        nVertPos(1), nHorzPos(7), nMousePointer(0), nBorderColor(0x80000012),
        nKeepScrollBarsVisible(3), nCycle(0), nBorderStyle(0), nSpecialEffect(0),
        nPicture(0), nPictureAlignment(2), nPictureSizeMode(0),
        bPictureTiling(FALSE), nAccelerator(0), nIcon(0), pCaption(0),
        nScrollWidth(0), nScrollHeight(0), nIconLen(0), pIcon(0), nPictureLen(0),
        pPicture(0)
{
    //msDialogType = C2U("NotSupported");
    msDialogType = C2U("com.sun.star.awt.UnoMultiPageModel");
    mnForeColor = 0x80000012L,
    mnBackColor = 0x8000000FL;
    bSetInDialog = true;// UserForm control only
    aFontData.SetHasAlign(TRUE);
    containerType = MULTIPAGE;
    mnCurrentPageStep = 0;
}

void OCX_MultiPage::ProcessControl(OCX_Control* pControl, SvStorageStream* /* pS */,  ContainerRecord& rec )
{
    SotStorageStreamRef oStream = mContainedControlsStream;

    OCX_Page *pPage = NULL;
    if ( rec.nTypeIdent == PAGE )
        pPage = static_cast< OCX_Page* >( pControl );
    if ( pPage != NULL )
    {
        pPage->mnStep = ++mnCurrentPageStep;

        pPage->mnTop =  mnTop;// move these to Page::import ?
        pPage->mnLeft = mnLeft;
        pPage->mnBackColor = mnBackColor;

        oStream = pPage->getContainerStream();;
        // Position of controls is relative to pos of this MuliPage
        // Control
        pPage->FullRead( oStream );

        mpControls.push_back( pPage );
    }
    else
    {
        OSL_TRACE("!!!! Unsupported Control 0x%x ", rec.nTypeIdent);
        DBG_ERROR("MultiPage error expected Page control");
    }

}

sal_Bool OCX_MultiPage::Read(SvStorageStream *pS)
{
    // Unlike the other containers e.g. UserForm & Frame
    // the o stream is does not contain info for the contained controls
    // ( e.g. the pages themselves ) but seems to be for the MultiPage
    // itself - need to check this in more detail

    // For the moment skip read of the MultiPage specific properties
    // not much point reading these as we can't display the multi page
    // control or in fact any sort of tabbed layout, best we can do is
    // import just the contained controls of the individual pages
    // Note: the record skipped below ( does not contain the expected
    // info on this control, that seems to be contained in the o stream,
    // see comment above)
    OCX_Control skip(C2S("Dummy"));
    skip.Read( pS );
    mnCurrentPageStep = mnStep; //( set step of of pages relative to step
                                //of this MultiPage ( e.g. emulate containment )
    return OCX_ContainerControl::Read(pS);
}


sal_Bool OCX_MultiPage::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    OCX_ContainerControl::Import( rPropSet );
    return sal_True;
}

sal_Bool OCX_MultiPage::Import(com::sun::star::uno::Reference<
        com::sun::star::container::XNameContainer>
        &rDialog)
{
    uno::Reference<beans::XPropertySet> xPropSet( rDialog, uno::UNO_QUERY );

    // Although MultiPage is not represeted by a "real" control we still
    // need to propagate the backcolor of this logical parent
    // ( the dialog or Frame or whatever ) to the children of this control.
    // For example the controls contained in the Page of a
    // MultiPage control use the parents backcolor ( e,g,
    // Pages backcolor ) when trying to fake transparency
    mnBackColor = mpParent->mnBackColor;

    if ( xPropSet.is() )
    {
        uno::Reference<lang::XMultiServiceFactory>
            xFactory(rDialog, uno::UNO_QUERY);
    OSL_TRACE("** MultiPage creating control %s", rtl::OUStringToOString( msDialogType, RTL_TEXTENCODING_UTF8 ).getStr() );
    uno::Reference<uno::XInterface> xCreate = xFactory->createInstance(msDialogType);
    if (!xCreate.is())
        return sal_False;

    uno::Reference<awt::XControlModel> xModel(xCreate, uno::UNO_QUERY);
    if (!xModel.is())
        return sal_False;

        try
        {
        // we should just call MultiPage::Import( XPropertySet )
    		OSL_TRACE("********* MULTIPAGE cName %s", rtl::OUStringToOString( sName, RTL_TEXTENCODING_UTF8 ).getStr() );
        uno::Any aTmp(&sName,getCppuType((OUString *)0));
        uno::Reference<beans::XPropertySet> xPrps(xModel, uno::UNO_QUERY);
        xPrps->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );
        aTmp = uno::makeAny( mnCurrentPageStep );
        xPrps->setPropertyValue( WW8_ASCII2STR("ProgressValueMax"), aTmp );
        // default current page to 0 ( #FIXME, we need to read this value )
        aTmp = uno::makeAny( sal_Int32(0) );
        xPrps->setPropertyValue( WW8_ASCII2STR("ProgressValue"), aTmp );
    		OSL_TRACE("********* MULTIPAGE vomitted out properties");

    // Calls import on contained controls
            rDialog->insertByName(sName, uno::makeAny(xModel));
            OSL_TRACE("*** inserted ***");
        }
        catch( uno::Exception& )
        {
            DBG_ERRORFILE(
                ByteString( "OCX_Control::Import - cannot insert control \"" ).
                Append( ByteString( sName, RTL_TEXTENCODING_UTF8 ) ).
                Append( '"' ).GetBuffer() );
        }

        // Calls import on contained pages
        return OCX_ContainerControl::Import( xPropSet );
    }
    OSL_TRACE("*** Major problem, no dialog to add controls to ");
    DBG_ERROR(" Major problem, no dialog to add controls to ");
    return false;
}



OCX_Page::OCX_Page( SotStorageRef& parent,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const uno::Reference< container::XNameContainer >  &rDialog,
            OCX_Control* pParent):
        OCX_ContainerControl(parent, storageName, sN, rDialog, pParent ),
        fUnknown1(0), fEnabled(1), fLocked(0),
        fBackStyle(1), fWordWrap(1), fAutoSize(0), nCaptionLen(0), nVertPos(1),
        nHorzPos(7), nMousePointer(0), nBorderColor(0x80000012),
        nKeepScrollBarsVisible(3), nCycle(0), nBorderStyle(0), nSpecialEffect(0),
        nPicture(0), nPictureAlignment(2), nPictureSizeMode(0),
        bPictureTiling(FALSE), nAccelerator(0), nIcon(0), pCaption(0),
        nScrollWidth(0), nScrollHeight(0), nIconLen(0), pIcon(0), nPictureLen(0),
        pPicture(0)
{
    msDialogType = C2U("NotSupported");
    mnForeColor = 0x80000012,
    mnBackColor = 0x8000000F,
    bSetInDialog = true;// UserForm control only
    aFontData.SetHasAlign(TRUE);
}


sal_Bool OCX_Page::Read(SvStorageStream *pS)
{
    long nStart = pS->Tell();
    *pS >> nIdentifier;
    DBG_ASSERT(0x400==nIdentifier,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);

    pS->SeekRel( nFixedAreaLen - sizeof( pBlockFlags ) );

    ReadAlign( pS, pS->Tell() - nStart, 4);

    if (pBlockFlags[2] & 0x10)
    {
        //Font Stuff..
        pS->SeekRel(0x1a);
        sal_uInt8 nFontLen;
        *pS >> nFontLen;
        pS->SeekRel(nFontLen);
    }
    return OCX_ContainerControl::Read(pS);

}

sal_Bool OCX_Page::Import(com::sun::star::uno::Reference<
        com::sun::star::container::XNameContainer>
        &rDialog)
{

    uno::Reference<beans::XPropertySet> xPropSet( rDialog, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        // apply Step to contained controls
        CtrlIterator aEnd = mpControls.end();
        for (CtrlIterator aIter = mpControls.begin(); aIter != aEnd; ++ aIter )
        {
            (*aIter)->mnStep = mnStep;
        }
        // Calls import on contained pages
        return OCX_ContainerControl::Import( xPropSet );
    }
    OSL_TRACE("*** Major problem, no dialog to add controls to ");
    DBG_ERROR("*** Major problem, no dialog to add controls to ");
    return sal_False;
}

OCX_Frame::OCX_Frame( SotStorageRef& parent,
            const ::rtl::OUString& storageName,
            const ::rtl::OUString& sN,
            const uno::Reference< container::XNameContainer >  &rDialog, OCX_Control* pParent):
        OCX_ContainerControl(parent, storageName, sN, rDialog, pParent ),fUnknown1(0),fEnabled(1), fLocked(0),
        fBackStyle(1), fWordWrap(1), fAutoSize(0), nCaptionLen(0), nVertPos(1),
        nHorzPos(7), nMousePointer(0), nBorderColor(0x80000012),
        nKeepScrollBarsVisible(3), nCycle(0), nBorderStyle(0), nSpecialEffect(0),
        nPicture(0), nPictureAlignment(2), nPictureSizeMode(0),
        bPictureTiling(FALSE), nAccelerator(0), nIcon(0), pCaption(0),
        nScrollWidth(0), nScrollHeight(0), nScrollLeft(0), nScrollTop(0), nIconLen(0), pIcon(0), nPictureLen(0),
        pPicture(0)
{
    msDialogType = C2U("com.sun.star.awt.UnoControlGroupBoxModel");
    mnForeColor = 0x80000012;
    mnBackColor = 0x8000000F;
    bSetInDialog = true;// UserForm control only
    aFontData.SetHasAlign(TRUE);
}


sal_Bool OCX_Frame::Read(SvStorageStream *pS)
{
    long nStart = pS->Tell();
    *pS >> nIdentifier;
    DBG_ASSERT(0x400==nIdentifier,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);

    if (pBlockFlags[0] & 0x01)
    {
            DBG_ASSERT(!this, "ARSE");
    }
    if (pBlockFlags[0] & 0x02)
            *pS >> mnBackColor;
    if (pBlockFlags[0] & 0x04)
            *pS >> mnForeColor;
    if (pBlockFlags[0] & 0x08)
        *pS >> fUnknown1;
    if (pBlockFlags[0] & 0x40)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x04)>>2;
        fBackStyle = (nTemp&0x08)>>3;
        *pS >> nTemp;
        *pS >> nTemp;
        fWordWrap = (nTemp&0x80)>>7;
        *pS >> nTemp;
        fAutoSize = (nTemp&0x10)>>4;
    }
    if (pBlockFlags[0] & 0x80)
    {
        *pS >> nBorderStyle;
    }

    ReadAlign(pS, pS->Tell() - nStart, 4);

    if (pBlockFlags[1] & 0x01)
        *pS >> nMousePointer;
    if (pBlockFlags[1] & 0x02)
        *pS >> nKeepScrollBarsVisible;
    if (pBlockFlags[1] & 0x20)
        *pS >> fUnknown1; // another unknown 32 bit ( or is 8 or 16 bit with padding ? )

    if (pBlockFlags[1] & 0x80)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nIcon;
        DBG_ASSERT(nIcon == 0xFFFF, "Unexpected nIcon");
    }

    bool bCaption = false;

    if (pBlockFlags[2] & 0x01)
        *pS >> nCycle;
    if (pBlockFlags[2] & 0x02)
        *pS >> nSpecialEffect;

    if (pBlockFlags[2] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nBorderColor;
    }

    if (pBlockFlags[2] & 0x08)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nCaptionLen;
        bCaption = true;
    }

    if (pBlockFlags[2] & 0x10)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        sal_uInt16 nNoIdea;
        *pS >> nNoIdea;
        DBG_ASSERT(nNoIdea == 0xFFFF, "Expected 0xFFFF, (related to font ?)");
    }

    if (pBlockFlags[2] & 0x20)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nPicture;
        DBG_ASSERT(nPicture == 0xFFFF, "Unexpected nIcon");
    }

    if (pBlockFlags[2] & 0x80)
        *pS >> nPictureAlignment;

    if (pBlockFlags[3] & 0x01)
        bPictureTiling = true;

    if (pBlockFlags[3] & 0x02)
        *pS >> nPictureSizeMode;

    if (pBlockFlags[3] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> fUnknown8;
    }

    if (pBlockFlags[3] & 0x08)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> fUnknown9;
    }

    ReadAlign(pS, pS->Tell() - nStart, 4);
    *pS >> nWidth;
    *pS >> nHeight;
    *pS >> nScrollWidth;
    *pS >> nScrollHeight;

    if (pBlockFlags[1] & 0x10)
    {
        *pS >> nScrollLeft;
        *pS >> nScrollTop;
    }

    if ( bCaption )
    {
        lclReadCharArray( *pS, pCaption, nCaptionLen, pS->Tell() - nStart);
    }

    OUString tempCaption =  lclCreateOUString( pCaption, nCaptionLen );

    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        pPicture = new sal_uInt8[nPictureLen];
        pS->Read(pPicture,nPictureLen);
    }

    ReadAlign( pS, pS->Tell() - nStart, 4);

    if (pBlockFlags[2] & 0x10)
    {
        //Font Stuff..
        pS->SeekRel(0x1a);
        sal_uInt8 nFontLen;
        *pS >> nFontLen;
        pS->SeekRel(nFontLen);
    }

    return OCX_ContainerControl::Read( pS );
}

sal_Bool OCX_Frame::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue(
        OUString(RTL_CONSTASCII_USTRINGPARAM("Name")), aTmp);
    if ( pCaption )
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        rPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    // Calls import on contained controls
    OCX_ContainerControl::Import( rPropSet );
    return sal_True;
}
OCX_UserForm::OCX_UserForm( SotStorageRef& parent,
            const OUString& storageName,
            const OUString& sN,
            const ::uno::Reference< container::XNameContainer >  &rDialog,
            const ::uno::Reference< lang::XMultiServiceFactory >& rMsf):
        OCX_ContainerControl(parent, storageName, sN, rDialog),
        nChildrenA(0), fEnabled(1), fLocked(0),
        fBackStyle(1), fWordWrap(1), fAutoSize(0), nCaptionLen(0), nVertPos(1),
        nHorzPos(7), nMousePointer(0), nBorderColor(0x80000012), nChildrenB(0),
        nKeepScrollBarsVisible(3), nCycle(0), nBorderStyle(0), nSpecialEffect(0),
        nPicture(0), nPictureAlignment(2), nPictureSizeMode(0),
        bPictureTiling(FALSE), nAccelerator(0), nIcon(0), pCaption(0),
        nScrollWidth(0), nScrollHeight(0), nScrollLeft(0), nScrollTop(0), nIconLen(0), pIcon(0), nPictureLen(0)
    {
            mnForeColor = 0x80000012;
            mnBackColor = 0x8000000F;
            uno::Reference< beans::XPropertySet> xProps( rMsf, uno::UNO_QUERY);
            if ( xProps.is() )
            {
                xProps->getPropertyValue(C2S("DefaultContext"))  >>= mxCtx;
            }
            aFontData.SetHasAlign(TRUE);
        }
sal_Bool OCX_UserForm::Read(SvStorageStream *pS)
{
    long nStart = pS->Tell();
    *pS >> nIdentifier;
    DBG_ASSERT(0x400==nIdentifier,
            "A control that has a different identifier");
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);

    if (pBlockFlags[0] & 0x01)
    {
            DBG_ASSERT(!this, "ARSE");
    }
    if (pBlockFlags[0] & 0x02)
        *pS >> mnBackColor;
    if (pBlockFlags[0] & 0x04)
        *pS >> mnForeColor;
    if (pBlockFlags[0] & 0x08)
        *pS >> nChildrenA;
    if (pBlockFlags[0] & 0x40)
    {
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x04)>>2;
        fBackStyle = (nTemp&0x08)>>3;
        *pS >> nTemp;
        *pS >> nTemp;
        fWordWrap = (nTemp&0x80)>>7;
        *pS >> nTemp;
        fAutoSize = (nTemp&0x10)>>4;
    }
    if (pBlockFlags[0] & 0x80)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nBorderStyle;
    }
#if 0
        sal_uInt16 nFixedOrAlign;
        *pS >> nFixedOrAlign;
#endif
    if (pBlockFlags[1] & 0x01)
        *pS >> nMousePointer;
    if (pBlockFlags[1] & 0x02)
        *pS >> nKeepScrollBarsVisible;
    if (pBlockFlags[1] & 0x20)
    {
        sal_uInt32 nUnknown32;
        *pS >> nUnknown32;
    }
    if (pBlockFlags[1] & 0x80)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nIcon;
        DBG_ASSERT(nIcon == 0xFFFF, "Unexpected nIcon");
    }
    if (pBlockFlags[2] & 0x01)
        *pS >> nCycle;
    if (pBlockFlags[2] & 0x02)
        *pS >> nSpecialEffect;

    if (pBlockFlags[2] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nBorderColor;
    }

    if (pBlockFlags[2] & 0x10)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        sal_uInt16 nNoIdea;
        *pS >> nNoIdea;
        DBG_ASSERT(nNoIdea == 0xFFFF, "Expected 0xFFFF, (related to font ?)");
    }

    if (pBlockFlags[2] & 0x20)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nPicture;
        DBG_ASSERT(nPicture == 0xFFFF, "Unexpected nIcon");
    }

    if (pBlockFlags[2] & 0x80)
        *pS >> nPictureAlignment;

    if (pBlockFlags[3] & 0x01)
        bPictureTiling = true;

    if (pBlockFlags[3] & 0x02)
        *pS >> nPictureSizeMode;

    if (pBlockFlags[3] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nChildrenB;
    }

    ReadAlign(pS, pS->Tell() - nStart, 4);
    *pS >> nDrawBuffer;

    ReadAlign(pS, pS->Tell() - nStart, 4);
    *pS >> nWidth;
    *pS >> nHeight;
    *pS >> nScrollWidth;
    *pS >> nScrollHeight;

    if (pBlockFlags[1] & 0x10)
    {
        *pS >> nScrollLeft;
        *pS >> nScrollTop;
    }

    if (nIcon)
    {
        pS->Read(pIconHeader,20);
        *pS >> nIconLen;
        pIcon = new sal_uInt8[nIconLen];
        pS->Read(pIcon,nIconLen);
    }

    ReadAlign( pS, pS->Tell() - nStart, 4);
    if (pBlockFlags[2] & 0x10)
    {
        //Font Stuff..
        pS->SeekRel(0x1a);
        sal_uInt8 nFontLen;
        *pS >> nFontLen;
        pS->SeekRel(nFontLen);
    }
    if (nPicture)
    {
        pS->Read(pPictureHeader,20);
        *pS >> nPictureLen;
        long imagePos = pS->Tell();
        // great embedded object
        mxGrfObj = lcl_readGraphicObject( pS );
        if( mxGrfObj.is() )
        {
            sImageUrl = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( GRAPHOBJ_URLPREFIX ) );
            sImageUrl = sImageUrl + mxGrfObj->getUniqueID();
        }
        // make sure the stream position should be pointing after the image.
        pS->Seek( imagePos + nPictureLen );
    }

    sal_Int16 numTrailingRecs = 0;
    *pS >> numTrailingRecs;
    // seems to be no. of trailing records,
    // before container record starts proper
    // ( unknown what these trailing records are for)
    if ( numTrailingRecs )
    {
        for ( sal_Int16 i = 0 ; numTrailingRecs ; --numTrailingRecs, ++i )
        {
            sal_uInt16 nTypeID = 0;
            if ( lcl_handleActiveXControl( pS, nTypeID ) )
            {
                if ( nTypeID & 0x8000 ) // valid ActiveXID
                    mActiveXIDMap[ ( i | 0x8000 ) ] = nTypeID;
            }
        }
    }
    return OCX_ContainerControl::Read( pS );
}

sal_Bool OCX_UserForm::Import(
    uno::Reference<container::XNameContainer> &rLib )
{
    uno::Reference<beans::XPropertySet>
        xDialogPropSet(mxParent, uno::UNO_QUERY);
    if ( !xDialogPropSet.is() )
        return sal_False;
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    xDialogPropSet->setPropertyValue(
        OUString(RTL_CONSTASCII_USTRINGPARAM("Name")), aTmp);
    xDialogPropSet->setPropertyValue(
        OUString(RTL_CONSTASCII_USTRINGPARAM("Title")), aTmp);
    aTmp <<= ImportColor(mnBackColor);
    xDialogPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    GraphicHelper gHelper( pDocSh->GetModel() );

    awt::Size aAppFontSize = gHelper.convertHmmToAppFont( awt::Size( nWidth, nHeight ) );
    aTmp <<= sal_Int32( aAppFontSize.Width ); //
    xDialogPropSet->setPropertyValue(WW8_ASCII2STR("Width"), aTmp);
    aTmp <<= sal_Int32( aAppFontSize.Height ); //100th mm
    xDialogPropSet->setPropertyValue(WW8_ASCII2STR("Height"), aTmp);


    uno::Reference<beans::XPropertySet> xPropSet( mxParent, uno::UNO_QUERY );
    OCX_ContainerControl::Import( xPropSet );

    uno::Reference<io::XInputStreamProvider> xSource =
        xmlscript::exportDialogModel(mxParent, mxCtx, pDocSh->GetModel() );
    uno::Any aSourceAny(uno::makeAny(xSource));
    if (rLib->hasByName(sName))
        rLib->replaceByName(sName, aSourceAny);
    else
        rLib->insertByName(sName, aSourceAny);

    if ( sImageUrl.getLength() )
    {
        aTmp <<= sImageUrl;
        try
        {
        	xDialogPropSet->setPropertyValue( WW8_ASCII2STR("ImageURL"), aTmp);
        }
        catch( uno::Exception& )
        {
            OSL_TRACE("OCX_UserForm::Import, Image fails to import");
        }
    }
    return sal_True;
}

sal_Bool OCX_Label::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet = sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);
    pBlockFlags[0] = 0x20;
    pBlockFlags[1] = 0;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x01;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fWordWrap = any2bool(aTmp);
    nTemp=fWordWrap;
    nTemp = nTemp << 7;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);
    pBlockFlags[0] |= 0x04;

    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    aCaption.WriteLenField( *rContents );
    if (aCaption.HasData())
        pBlockFlags[0] |= 0x08;

    WriteAlign(rContents,4);
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BorderColor"));
    if (aTmp.hasValue())
        aTmp >>= nBorderColor;
    *rContents << ExportColor(nBorderColor);
    pBlockFlags[0] |= 0x80;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    sal_uInt8 nNewBorder;
    nSpecialEffect = ExportBorder(nBorder,nNewBorder);
    nBorderStyle = nNewBorder;
    *rContents << nBorderStyle;
    pBlockFlags[1] |= 0x01;
    *rContents << nSpecialEffect;
    pBlockFlags[1] |= 0x02;

    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    bRet = aFontData.Export(rContents,rPropSet);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}



sal_Bool OCX_Label::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x23, 0x9E, 0x8C, 0x97,
        0xB0, 0xD4, 0xCE, 0x11, 0xBF, 0x2D, 0x00, 0xAA,
        0x00, 0x3F, 0x40, 0xD0, 0x1A, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4C, 0x61, 0x62, 0x65,
        0x6C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x0E, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x4C, 0x61, 0x62, 0x65, 0x6C, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x4C, 0x00, 0x61, 0x00, 0x62, 0x00, 0x65, 0x00,
        0x6C, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }
/*
    static sal_uInt8 __READONLY_DATA aTest[] = {
        0x00, 0x02, 0x20, 0x00, 0x2B, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
        0x06, 0x00, 0x00, 0x80, 0x4C, 0x61, 0x62, 0x65,
        0x6C, 0x31, 0x18, 0x00, 0xEC, 0x09, 0x00, 0x00,
        0x7B, 0x02, 0x00, 0x00, 0x00, 0x02, 0x20, 0x00,
        0x35, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x80,
        0xC3, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x54, 0x69, 0x6D, 0x65, 0x73, 0x20, 0x4E, 0x65,
        0x77, 0x20, 0x52, 0x6F, 0x6D, 0x61, 0x6E, 0x00,
        };
*/
    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

typedef OCX_Control *(*FNCreate)();


struct OCX_map
{
    FNCreate pCreate;
    const char *sId;
    sal_Int16 nId;
    const char *sName;
};

OCX_map aOCXTab[] =
{
    // Command button MUST be at index 0
    {&OCX_CommandButton::Create,"D7053240-CE69-11CD-a777-00dd01143c57",
        form::FormComponentType::COMMANDBUTTON,"CommandButton"},
    // Toggle button MUST be at index 1
    {&OCX_ToggleButton::Create,"8BD21D60-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::COMMANDBUTTON,"ToggleButton"},
    {&OCX_Label::Create, "978C9E23-D4B0-11CE-bf2d-00aa003f40d0",
        form::FormComponentType::FIXEDTEXT,"Label"},
    {&OCX_TextBox::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::TEXTFIELD,"TextBox"},
    {&OCX_ListBox::Create,"8BD21D20-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::LISTBOX,"ListBox"},
    {&OCX_ComboBox::Create,"8BD21D30-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::COMBOBOX,"ComboBox"},
    {&OCX_CheckBox::Create,"8BD21D40-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::CHECKBOX,"CheckBox"},
    {&OCX_OptionButton::Create,"8BD21D50-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::RADIOBUTTON,"OptionButton"},
    {&OCX_Image::Create,"4C599241-6926-101B-9992-00000b65c6f9",
        form::FormComponentType::IMAGECONTROL,"Image"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::DATEFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::TIMEFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::NUMERICFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::CURRENCYFIELD,"TextBox"},
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::PATTERNFIELD,"TextBox"},
#if 0
    {&OCX_FieldControl::Create,"8BD21D10-EC42-11CE-9e0d-00aa006002f3",
        form::FormComponentType::FORMULAFIELD,"TextBox"},
#endif
    {&OCX_ImageButton::Create,"D7053240-CE69-11CD-a777-00dd01143c57",
        form::FormComponentType::IMAGEBUTTON,"CommandButton"},
    {&OCX_SpinButton::Create,"79176FB0-B7F2-11CE-97ef-00aa006d2776",
        form::FormComponentType::SPINBUTTON,"SpinButton"},
    {&OCX_ScrollBar::Create,"DFD181E0-5E2F-11CE-a449-00aa004a803d",
        form::FormComponentType::SCROLLBAR,"ScrollBar"},
    {&OCX_GroupBox::Create,"",
        form::FormComponentType::GROUPBOX,""},
    {&OCX_ProgressBar::Create,"",
        form::FormComponentType::CONTROL,""},
    {&HTML_TextBox::Create,"5512D124-5CC6-11CF-8d67-00aa00bdce1d", form::FormComponentType::TEXTFIELD,"TextBox"},
};

const int NO_OCX = sizeof( aOCXTab ) / sizeof( *aOCXTab );

SvxMSConvertOCXControls::SvxMSConvertOCXControls(SfxObjectShell *pDSh, SwPaM *pP) :
    pDocSh(pDSh), pPaM(pP), nEdit(0), nCheckbox(0)
{
    DBG_ASSERT( pDocSh, "No DocShell, Cannot do Controls" );
    OCX_Control::FillSystemColors();
}

SvxMSConvertOCXControls::~SvxMSConvertOCXControls()
{
}

OCX_Control * SvxMSConvertOCXControls::OCX_Factory(const String &sName)
{
    for (int i=0;i<NO_OCX;i++)
    {
        if ( sName.EqualsIgnoreCaseAscii( aOCXTab[i].sId ))
            return(aOCXTab[i].pCreate());
    }
    return(NULL);
}

OCX_Control * SvxMSConvertOCXControls::OCX_Factory(
    const uno::Reference< awt::XControlModel > &rControlModel,
    String &rId, String &rName)
{
    rName.Erase();
    rId.Erase();

    uno::Reference< beans::XPropertySet > xPropSet(
        rControlModel,uno::UNO_QUERY);

    uno::Any aTmp = xPropSet->getPropertyValue(
        OUString::createFromAscii("ClassId"));
    sal_Int16 nClassId = *(sal_Int16*) aTmp.getValue();

//Begin nasty hack
    /*
    There is a truly horrible thing with EditControls and FormattedField
    Controls, they both pretend to have an EDITBOX ClassId for compability
    reasons, at some stage in the future hopefully there will be a proper
    FormulaField ClassId rather than this piggybacking two controls onto the
    same ClassId, when this happens uncomment the FORMULAFIELD in the OCX_Tab
    and delete this block, cmc.

    And also the nClassId for ImageControls is being reported as "CONTROL"
    rather than IMAGECONTROL
    */
    if (nClassId == form::FormComponentType::TEXTFIELD)
    {
        uno::Reference< lang::XServiceInfo > xInfo(rControlModel,
            uno::UNO_QUERY);
        if (xInfo->
            supportsService(OUString::createFromAscii(
                "com.sun.star.form.component.FormattedField")))
        {
            rId.AppendAscii("8BD21D10-EC42-11CE-9e0d-00aa006002f3");
            rName.AppendAscii("TextBox");
            return new OCX_FieldControl;
        }
    }
    else if (nClassId == form::FormComponentType::CONTROL)
    {
        uno::Reference< lang::XServiceInfo > xInfo(rControlModel,
            uno::UNO_QUERY);
        if (xInfo->
            supportsService(OUString::createFromAscii(
                "com.sun.star.form.component.ImageControl")))
        nClassId = form::FormComponentType::IMAGECONTROL;
    }
//End nasty hack

    const OCX_map* pEntry = 0;

    // distinguish between push button and toggle button
    if( nClassId == form::FormComponentType::COMMANDBUTTON )
    {
        pEntry = any2bool(xPropSet->getPropertyValue(WW8_ASCII2STR("Toggle"))) ?
            (aOCXTab + 1) : aOCXTab;
    }
    else
    {
        for( int i = 2; (i < NO_OCX) && !pEntry; ++i )
            if( nClassId == aOCXTab[ i ].nId )
                pEntry = aOCXTab + i;
    }

    if( pEntry )
    {
        rId.AppendAscii( pEntry->sId );
        rName.AppendAscii( pEntry->sName );
        return pEntry->pCreate();
    }

    return 0;
}


sal_Bool SvxMSConvertOCXControls::ReadOCXStream( SvStorageRef& rSrc1,
        uno::Reference < drawing::XShape > *pShapeRef,BOOL bFloatingCtrl)
{

    SvStorageStreamRef xCrash = rSrc1->OpenSotStream( WW8_ASCII2STR("contents") );
    sal_Bool bRet=sal_False;

    SvStorageStreamRef xSrc2 = rSrc1->OpenSotStream( WW8_ASCII2STR("\3OCXNAME") );
    SvStorageStream* pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    /* #117832# import control name */
    OUString controlName;
    bool hasCName = readOCXNAME( controlName, pSt );

    xSrc2 = rSrc1->OpenSotStream( WW8_ASCII2STR("contents") );
    pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    /*Get Class Id of this object, see if it is one of the types
     *that this importer can handle, call the appropiate handler
     to read that control, and call the appropiate handler to
     insert that control
     */

    SvGlobalName aTest = rSrc1->GetClassName();
    OCX_Control *pObj = OCX_Factory(aTest.GetHexName());
    if (pObj)
    {
        pObj->pDocSh = pDocSh;
        /* #117832# set imported control name */
        if ( hasCName )
        {
            pObj->sName = controlName;
        }
        com::sun::star::awt::Size aSz;
        uno::Reference< form::XFormComponent >  xFComp;
        const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
            GetServiceFactory();
        if(!rServiceFactory.is())
            return(sal_False);
        bRet = pObj->FullRead(pSt);
        if(bRet)
            if (pObj->Import(rServiceFactory,xFComp,aSz))
                bRet = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
        delete pObj;
    }
    return bRet;
}


sal_Bool SvxMSConvertOCXControls::ReadOCXExcelKludgeStream(
    SvStorageStreamRef& rSrc1, uno::Reference < drawing::XShape > *
    pShapeRef,BOOL bFloatingCtrl)
{
    sal_Bool bRet=sal_False;
    /*Get Class Id of this object, see if it is one of the types
     *that this importer can handle, call the appropiate handler
     to read that control, and call the appropiate handler to
     insert that control
     */
    /*The Excel Kludge is to concatenate a class id with a contents
     * stream, and then concatenate all the controls together,
     * This means that you should have the cnts stream wound to the
     * correct location before passing the control stream in here*/
    SvStream *pSt = rSrc1;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    SvGlobalName aTest;
    *pSt >> aTest;
    OCX_Control *pObj = OCX_Factory(aTest.GetHexName());
    if (pObj)
    {

        com::sun::star::awt::Size aSz;
        uno::Reference< form::XFormComponent >  xFComp;
        const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
            GetServiceFactory();
        if(!rServiceFactory.is())
            return(sal_False);
        bRet = pObj->FullRead(rSrc1);
        if(bRet)
            if (pObj->Import(rServiceFactory,xFComp,aSz))
                bRet = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
        delete pObj;
    }
    return bRet;
}


sal_Bool SvxMSConvertOCXControls::WriteOCXStream( SvStorageRef& rSrc1,
    const uno::Reference< awt::XControlModel > &rControlModel,
    const awt::Size &rSize, String &rName)
{
    sal_Bool bRet=sal_False;

    DBG_ASSERT( rControlModel.is(), "UNO-Control missing Model, panic!" );
    if( !rControlModel.is() )
        return sal_False;

#if 0
    uno::Any aTmp = xPropSet->getPropertyValue(
        OUString::createFromAscii("ClassId"));
    sal_Int16 nClassId = *(sal_Int16*) aTmp.getValue();
#endif

    String sId;
    OCX_Control *pObj = OCX_Factory(rControlModel,sId,rName);
    if (pObj != NULL)
    {
        uno::Reference<beans::XPropertySet> xPropSet(rControlModel,
            uno::UNO_QUERY);

        /* #117832# - also enable export of control name  */
        OUString sCName;
        xPropSet->getPropertyValue(C2S("Name")) >>= sCName;
        pObj->sName = sCName;

        SvGlobalName aName;
        aName.MakeId(sId);
        String sFullName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
            "Microsoft Forms 2.0 ")));
        sFullName.Append(rName);
        rSrc1->SetClass(aName,0x5C,sFullName);
        //                    ^^^^, this pathetic 0x5c is the magic number
        // which the lack of originally blocked the implementation of
        // the controls export
        // cmc

        bRet = pObj->Export(rSrc1,xPropSet,rSize);
        SvStorageStreamRef xStor2( rSrc1->OpenSotStream( WW8_ASCII2STR("\3OCXNAME")));
        /* #117832# - also enable export of control name  */
        writeOCXNAME( sCName, xStor2 );
        delete pObj;
    }
    return bRet;
}


//I think this should work for excel documents, create the "Ctls" stream
//and give it here as rContents, we'll append out streams ole id and
//contents here and that appears to be what Excel is doing
sal_Bool SvxMSConvertOCXControls::WriteOCXExcelKludgeStream(
    SvStorageStreamRef& rContents,
    const uno::Reference< awt::XControlModel > &rControlModel,
    const awt::Size &rSize, String &rName)
{
    sal_Bool bRet=sal_False;

    DBG_ASSERT( rControlModel.is(), "UNO-Control missing Model, panic!" );
    if( !rControlModel.is() )
        return sal_False;

    String sId;
    OCX_Control *pObj = OCX_Factory(rControlModel,sId,rName);
    if (pObj != NULL)
    {
        uno::Reference<beans::XPropertySet> xPropSet(rControlModel,
            uno::UNO_QUERY);

        SvGlobalName aName;
        aName.MakeId(sId);
        String sFullName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
            "Microsoft Forms 2.0 ")));
        sFullName.Append(rName);
        SvStream *pS=rContents;
        *pS << aName;
        bRet = pObj->WriteContents(rContents,xPropSet,rSize);
        delete pObj;
        // export needs correct stream position
        rContents->Seek( STREAM_SEEK_TO_END );
    }
    return bRet;
}







sal_Bool OCX_CheckBox::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    // background color: fBackStyle==0 -> transparent
    if( fBackStyle )
        aTmp <<= ImportColor(mnBackColor);
    else
        aTmp = uno::Any();
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    sal_Bool bTemp;
    if ((!(fEnabled)) || (fLocked))
        bTemp = sal_False;
    else
        bTemp = sal_True;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    bTemp = fWordWrap != 0;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    bTemp = nMultiState;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TriState"), aTmp);

    aTmp <<= ImportSpecEffect( nSpecialEffect );
    rPropSet->setPropertyValue( WW8_ASCII2STR("VisualEffect"), aTmp);

    if (pValue)
    {
        INT16 nTmp=pValue[0]-0x30;
        aTmp <<= nTmp;
        if ( !bSetInDialog )
            rPropSet->setPropertyValue( WW8_ASCII2STR("DefaultState"), aTmp);
        else
            rPropSet->setPropertyValue( WW8_ASCII2STR("State"), aTmp);
    }

    if (pCaption)
    {
        aTmp <<= lclCreateOUString( pCaption, nCaptionLen );
        rPropSet->setPropertyValue( WW8_ASCII2STR("Label"), aTmp);
    }

    // #i40279# always centered vertically
    aTmp <<= ::com::sun::star::style::VerticalAlignment_MIDDLE;
    rPropSet->setPropertyValue( WW8_ASCII2STR("VerticalAlign"), aTmp );

    aFontData.Import(rPropSet);
    return(sal_True);
}

sal_Bool OCX_CheckBox::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)

{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(12);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x01;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0x80;
    pBlockFlags[4] = 0;
    pBlockFlags[5] = 0;
    pBlockFlags[6] = 0;
    pBlockFlags[7] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("BackgroundColor"));
    fBackStyle = aTmp.hasValue() ? 1 : 0;
    if (fBackStyle)
        aTmp >>= mnBackColor;

    sal_uInt8 nTemp=fEnabled;
    if (fEnabled)
        nTemp = nTemp << 1;
    if (fBackStyle)
        nTemp |= 0x08;
    *rContents << nTemp;
    pBlockFlags[0] |= 0x01;
    *rContents << sal_uInt8(0x00);
    nTemp = 0;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("MultiLine"));
    fWordWrap = any2bool(aTmp);
    if (fWordWrap)
        nTemp |= 0x80;
    *rContents << nTemp;
    *rContents << sal_uInt8(0x00);

    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x02;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TextColor"));
    if (aTmp.hasValue())
        aTmp >>= mnForeColor;
    *rContents << ExportColor(mnForeColor);
    pBlockFlags[0] |= 0x04;

    nStyle = 4;
    *rContents << nStyle;
    pBlockFlags[0] |= 0x40;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("TriState"));
    nMultiState = any2bool(aTmp);
    *rContents << nMultiState;
    pBlockFlags[2] |= 0x20;

    WriteAlign(rContents,4);
    nValueLen = 1|SVX_MSOCX_COMPRESSED;
    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("DefaultState"));
    sal_Int16 nDefault = sal_Int16();
    aTmp >>= nDefault;
    *rContents << nValueLen;
    pBlockFlags[2] |= 0x40;

    SvxOcxString aCaption( rPropSet->getPropertyValue(WW8_ASCII2STR("Label")) );
    aCaption.WriteLenField( *rContents );
    if (aCaption.HasData())
        pBlockFlags[2] |= 0x80;

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("VisualEffect"));
    if (aTmp.hasValue())
    {
        sal_Int16 nApiSpecEffect = sal_Int16();
        aTmp >>= nApiSpecEffect;
        nSpecialEffect = ExportSpecEffect( nApiSpecEffect );
    }
    WriteAlign(rContents,4);
    *rContents << nSpecialEffect;
    pBlockFlags[3] |= 0x04;

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    nDefault += 0x30;
    *rContents << sal_uInt8(nDefault);
    *rContents << sal_uInt8(0x00);

    aCaption.WriteCharArray( *rContents );

    WriteAlign(rContents,4);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);
    bRet = aFontData.Export(rContents,rPropSet);
    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    *rContents << pBlockFlags[4];
    *rContents << pBlockFlags[5];
    *rContents << pBlockFlags[6];
    *rContents << pBlockFlags[7];

    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_CheckBox::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)

{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1D, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x43, 0x68, 0x65, 0x63,
        0x6B, 0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65,
        0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74,
        0x00, 0x11, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72,
        0x6D, 0x73, 0x2E, 0x43, 0x68, 0x65, 0x63, 0x6B,
        0x42, 0x6F, 0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x43, 0x00, 0x68, 0x00, 0x65, 0x00, 0x63, 0x00,
        0x6B, 0x00, 0x42, 0x00, 0x6F, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }
/*
    static sal_uInt8 __READONLY_DATA aTest[] = {
        0x00, 0x02, 0x34, 0x00, 0x46, 0x01, 0xC0, 0x80,
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x80, 0x09, 0x00, 0x00, 0x80,
        0xE2, 0x0E, 0x00, 0x00, 0x95, 0x02, 0x00, 0x00,
        0x30, 0x69, 0x1D, 0x00, 0x43, 0x68, 0x65, 0x63,
        0x6B, 0x42, 0x6F, 0x78, 0x31, 0x20, 0x52, 0x6F,
        0x00, 0x02, 0x20, 0x00, 0x35, 0x00, 0x00, 0x00,
        0x0F, 0x00, 0x00, 0x80, 0xC3, 0x00, 0x00, 0x00,
        0x00, 0x02, 0x00, 0x00, 0x54, 0x69, 0x6D, 0x65,
        0x73, 0x20, 0x4E, 0x65, 0x77, 0x20, 0x52, 0x6F,
        0x6D, 0x61, 0x6E, 0x00,
    };
*/
    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}


sal_Bool OCX_FontData::Read(SvStorageStream *pS)
{
    long nStart = pS->Tell();
    *pS >> nIdentifier;
    *pS >> nFixedAreaLen;
    pS->Read(pBlockFlags,4);

    bool bFontName = (pBlockFlags[0] & 0x01) != 0;
    if (bFontName)
        *pS >> nFontNameLen;
    if (pBlockFlags[0] & 0x02)
    {
        sal_uInt8 nTmp;
        *pS >> nTmp;
        fBold = nTmp & 0x01;
        fItalic = (nTmp & 0x02) >> 1;
        fUnderline = (nTmp & 0x04) >> 2;
        fStrike = (nTmp & 0x08) >> 3;
        fUnknown1 = (nTmp & 0xF0) >> 4;
        *pS >> nUnknown2;
        *pS >> nUnknown3;
        *pS >> nUnknown4;
    }
    if (pBlockFlags[0] & 0x04)
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        *pS >> nFontSize;
    }
    else
        nFontSize = 240;
    if (pBlockFlags[0] & 0x10)
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nLanguageID;
    }
    if (pBlockFlags[0] & 0x40)
    {
        *pS >> nJustification;
    }
    if (pBlockFlags[0] & 0x80)  // font weight before font name
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nFontWeight;
    }

    if (bFontName)
        lclReadCharArray( *pS, pFontName, nFontNameLen, pS->Tell() - nStart);

    ReadAlign(pS, pS->Tell() - nStart, 4);
    return(TRUE);
}

void OCX_FontData::Import(uno::Reference< beans::XPropertySet > &rPropSet)
{
    uno::Any aTmp;
    if (pFontName)
    {
        aTmp <<= lclCreateOUString( pFontName, nFontNameLen );
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontName"), aTmp);
    }

    if (bHasAlign)
    {
        sal_Int16 nAlign = ImportAlign(nJustification);
        aTmp <<= nAlign;
        rPropSet->setPropertyValue( WW8_ASCII2STR("Align"), aTmp);
    }

    if (fBold)
    {
        aTmp <<= awt::FontWeight::BOLD;
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontWeight"), aTmp);
    }

    if (fItalic)
    {
        aTmp <<= (sal_Int16)awt::FontSlant_ITALIC;
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontSlant"), aTmp);
    }

    if (fUnderline)
    {
        aTmp <<= awt::FontUnderline::SINGLE;
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontUnderline"), aTmp);
    }

    if (fStrike)
    {
        aTmp <<= awt::FontStrikeout::SINGLE;
        rPropSet->setPropertyValue( WW8_ASCII2STR("FontStrikeout"), aTmp);
    }

    // 2004-09-17: very strange way of storing font sizes...
    // 1pt->30, 2pt->45, 3pt->60, 4pt->75, 5pt->105, 6pt->120, 7pt->135
    // 8pt->165, 9pt->180, 10pt->195, 11pt->225, ...
    aTmp <<= sal_Int16( (nFontSize <= 30) ? 1 : ((nFontSize + 10) / 20) );
    rPropSet->setPropertyValue( WW8_ASCII2STR("FontHeight"), aTmp);
}

sal_Bool OCX_FontData::Export(SvStorageStreamRef &rContent,
    const uno::Reference< beans::XPropertySet > &rPropSet)
{
    sal_uInt8 nFlags=0x00;
    sal_uInt32 nOldPos = rContent->Tell();
    rContent->SeekRel(8);
    SvxOcxString aFontName;
    uno::Any aTmp;

    if (bHasFont)
        aFontName = rPropSet->getPropertyValue(WW8_ASCII2STR("FontName"));
    if (!aFontName.HasData())
        aFontName = OUString( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) );
    aFontName.WriteLenField( *rContent );
    nFlags |= 0x01;

    if (bHasFont)
    {
        aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("FontWeight"));
        float nBold = 0;
        aTmp >>= nBold;

        if (nBold >= 150)
        {
            nFlags |= 0x02;
            sal_uInt8 nTmp=0x01;
            *rContent << nTmp;
            nTmp=0x00;
            *rContent << nTmp;
            *rContent << nTmp;
            *rContent << nTmp;
        }

        aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("FontHeight"));
        float nFontHeight = 0;
        aTmp >>= nFontHeight;
        if (nFontHeight)
        {
            nFlags |= 0x04;
            // 2004-09-17: very strange way of storing font sizes:
            // 1pt->30, 2pt->45, 3pt->60, 4pt->75, 5pt->105, 6pt->120, 7pt->135
            // 8pt->165, 9pt->180, 10pt->195, 11pt->225, ...
            nFontSize = (nFontHeight == 1) ? 30 : (static_cast<sal_uInt32>((nFontHeight*4+1)/3)*15);
            *rContent << nFontSize;
        }

        if (bHasAlign)
        {
            *rContent << sal_uInt16(0x0200);
            nFlags |= 0x10;

            nFlags |= 0x20; // ?

            aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Align"));
            nFlags |= 0x40;
            sal_Int16 nAlign(0);
            if (aTmp.hasValue())
                aTmp >>= nAlign;
            nJustification = ExportAlign(nAlign);
            *rContent << nJustification;
        }
    }

    aFontName.WriteCharArray( *rContent );
    WriteAlign(rContent,4);

    sal_uInt16 nFixedAreaLn = static_cast<sal_uInt16>(rContent->Tell()-nOldPos-4);
    rContent->Seek(nOldPos);
    *rContent << nStandardId;
    *rContent << nFixedAreaLn;
    *rContent << nFlags;
    *rContent << sal_uInt8(0x00);
    *rContent << sal_uInt8(0x00);
    *rContent << sal_uInt8(0x00);

    WriteAlign(rContent,4);
    return sal_True;
}


sal_Bool HTML_TextBox::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    aTmp = bool2any( fEnabled != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp = bool2any( fLocked != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("ReadOnly"), aTmp);

    aTmp = bool2any( fHideSelection != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR( "HideInactiveSelection" ), aTmp);

    aTmp <<= ImportColor(mnForeColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("TextColor"), aTmp);

    aTmp <<= ImportColor(mnBackColor);
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp <<= ImportBorder(nSpecialEffect,nBorderStyle);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    aTmp <<= ImportColor( nBorderColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("BorderColor"), aTmp);

    aTmp = bool2any( fMultiLine != 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("MultiLine"), aTmp);

    sal_uInt16 nTmp = static_cast<sal_uInt16>(nMaxLength);
    aTmp <<= nTmp;
    rPropSet->setPropertyValue( WW8_ASCII2STR("MaxTextLen"), aTmp);


    sal_Bool bTemp1,bTemp2;
    uno::Any aBarsH,aBarsV;
    switch(nScrollBars)
    {
        case 1:
            bTemp1 = sal_True;
            bTemp2 = sal_False;
            break;
        case 2:
            bTemp1 = sal_False;
            bTemp2 = sal_True;
            break;
        case 3:
            bTemp1 = sal_True;
            bTemp2 = sal_True;
            break;
        case 0:
        default:
            bTemp1 = sal_False;
            bTemp2 = sal_False;
            break;
    }

    aBarsH = bool2any(bTemp1);
    aBarsV = bool2any(bTemp2);
    rPropSet->setPropertyValue( WW8_ASCII2STR("HScroll"), aBarsH);
    rPropSet->setPropertyValue( WW8_ASCII2STR("VScroll"), aBarsV);

    nTmp = nPasswordChar;
    aTmp <<= nTmp;
    rPropSet->setPropertyValue( WW8_ASCII2STR("EchoChar"), aTmp);

    if (pValue)
    {
        aTmp <<= lclCreateOUString( pValue, nValueLen );
        // DefaultText seems to no longer be in UnoEditControlModel
        if ( bSetInDialog )
        {
            rPropSet->setPropertyValue( WW8_ASCII2STR("Text"), aTmp);
        }
        else
        {
            rPropSet->setPropertyValue( WW8_ASCII2STR("DefaultText"), aTmp);
        }
    }

    //	aFontData.Import(rPropSet);
    return sal_True;
}

sal_Bool HTML_TextBox::Read(SotStorageStream * /*pS*/)
{
  return sal_True;
}

sal_Bool HTML_TextBox::ReadFontData(SotStorageStream * /*pS*/)
{
  return sal_True;
}


// Doesn't really read anything but just skips the
// record.
sal_Bool OCX_TabStrip::Read(SotStorageStream *pS)
{
    const long skipLen = 0x18;
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
        "A control that has a different identifier");
    *pS >> nFixedAreaLen;

    pS->Read(pBlockFlags, sizeof(pBlockFlags));
    pS->SeekRel(skipLen);
    *pS >> nNumTabs;
    // skip to end of control
    pS->SeekRel(nFixedAreaLen - sizeof(pBlockFlags) - sizeof(nNumTabs) - skipLen );
    return sal_True;
}

sal_Bool OCX_TabStrip::ReadFontData(SotStorageStream *pS)
{
    // Seems like there is a real font record followed by
    // a number of blank records ( e.g. nFixedAreaLen = 0 )
    // The number of trailing blank records is equal to the number of tabs
    OCX_Control::ReadFontData(pS);
    for ( sal_uInt16 index = 0; index < nNumTabs; index++ )
    {
        OCX_Control::Read(pS); // read trailing records
    }
    return sal_True;
}

sal_Bool OCX_Image::Read(SotStorageStream *pS)
{
    ULONG nStart = pS->Tell();
    *pS >> nIdentifier;
    DBG_ASSERT(nStandardId==nIdentifier,
        "A control that has a different identifier");
    *pS >> nFixedAreaLen;

    pS->Read(pBlockFlags, sizeof(pBlockFlags));

    bool hasEmbeddedImage = false;

    if ( pBlockFlags[0] & 0x04 )
    {
        bAutoSize = true;
    }
    if ( pBlockFlags[0] & 0x08 )
    {
        *pS >> nBorderColor;
    }
    if ( pBlockFlags[0] & 0x10 )
    {
        *pS >> mnBackColor;
    }
    if ( pBlockFlags[0] & 0x20 )
    {
        *pS >> nBorderStyle;
    }
    if ( pBlockFlags[0] & 0x40 )
    {
        *pS >> nMousePointer;
    }
    if ( pBlockFlags[0] & 0x80 )
    {
        *pS >> nPictureSizeMode;
    }

    if ( pBlockFlags[ 1 ] & 0x1 )
    {
        *pS >> nSpecialEffect;
    }
    // investigate if valid also for formcontrols, although for controls
    // images themselves seem to come through from escher ( as shape, think
    // its probably a preview? )

    // np a bit of a guess ( until proved incorrect )
    if ( pBlockFlags[ 1 ] & 0x4 )
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        hasEmbeddedImage = true;
        sal_uInt16 unknown;
        *pS >> unknown;
    }


    if ( pBlockFlags[ 1 ] & 0x8 )
    {
        ReadAlign(pS, pS->Tell() - nStart, 2);
        *pS >> nPictureAlignment;

    }
    if ( pBlockFlags[ 1 ] & 0x10 )
    {
        bPictureTiling = true;
    }
    if ( pBlockFlags[1] & 0x20 )
    {
        ReadAlign(pS, pS->Tell() - nStart, 4);
        sal_uInt8 nTemp;
        *pS >> nTemp;
        fEnabled = (nTemp&0x02)>>1;
        fBackStyle = (nTemp&0x08)>>3;
        // padding?
        *pS >> nTemp;
        *pS >> nTemp;
        *pS >> nTemp;
    }
    ReadAlign(pS, pS->Tell() - nStart, 4);
    *pS >> nWidth;
    *pS >> nHeight;

    if ( hasEmbeddedImage )
    {
        //image follows this block
        //len of image is 0x14 relative to end of this block
        pS->Seek( pS->Tell() + 0x14 );

        sal_uInt32 nImageLen = 0;
        *pS >> nImageLen;

        long imagePos = pS->Tell();

        mxGrfObj = lcl_readGraphicObject( pS );
        if( mxGrfObj.is() )
        {
            sImageUrl = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( GRAPHOBJ_URLPREFIX ) );
            sImageUrl = sImageUrl + mxGrfObj->getUniqueID();
        }
        // make sure the stream position should be pointing after the image
        pS->Seek( imagePos + nImageLen );
    }
    return sal_True;
}

sal_Bool OCX_Image::Import( uno::Reference< beans::XPropertySet > &rPropSet )
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );

    if( fBackStyle )
        aTmp <<= ImportColor(mnBackColor);
    else
        aTmp = uno::Any();
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    sal_Bool bTemp;
    if (fEnabled)
        bTemp = sal_True;
    else
        bTemp = sal_False;
    aTmp = bool2any(bTemp);
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);
    if ( sImageUrl.getLength() )
    {
        aTmp <<= sImageUrl;
        rPropSet->setPropertyValue( WW8_ASCII2STR("ImageURL"), aTmp);
    }
    return sal_True;
}

sal_Bool OCX_Image::WriteContents(SvStorageStreamRef &rContents,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    sal_Bool bRet=sal_True;
    sal_uInt32 nOldPos = rContents->Tell();
    rContents->SeekRel(8);

    pBlockFlags[0] = 0;
    pBlockFlags[1] = 0x02;
    pBlockFlags[2] = 0;
    pBlockFlags[3] = 0;

    uno::Any aTmp = rPropSet->getPropertyValue(
        WW8_ASCII2STR("BackgroundColor"));
    if (aTmp.hasValue())
        aTmp >>= mnBackColor;
    *rContents << ExportColor(mnBackColor);
    pBlockFlags[0] |= 0x10;


    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Border"));
    sal_Int16 nBorder = sal_Int16();
    aTmp >>= nBorder;
    nSpecialEffect = ExportBorder(nBorder,nBorderStyle);
    *rContents << nBorderStyle;
    pBlockFlags[0] |= 0x20;

    *rContents << nSpecialEffect;
    pBlockFlags[1] |= 0x01;

    WriteAlign(rContents,4);

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("Enabled"));
    fEnabled = any2bool(aTmp);
    if (fEnabled)
    {
        sal_uInt8 nTemp = 0x19;
        *rContents << nTemp;
        pBlockFlags[1] |= 0x20;
        *rContents << sal_uInt8(0x00);
        *rContents << sal_uInt8(0x00);
        *rContents << sal_uInt8(0x00);
    }

    aTmp = rPropSet->getPropertyValue(WW8_ASCII2STR("ImageURL"));
//	OUString *pStr = (OUString *)aTmp.getValue();
    /*Magically fetch that image and turn it into something that
     *we can store in ms controls, wmf,png,jpg are almost certainly
     *the options we have for export...*/

    WriteAlign(rContents,4);
    *rContents << rSize.Width;
    *rContents << rSize.Height;

    WriteAlign(rContents,4);
    nFixedAreaLen = static_cast<sal_uInt16>(rContents->Tell()-nOldPos-4);

    rContents->Seek(nOldPos);
    *rContents << nStandardId;
    *rContents << nFixedAreaLen;

    *rContents << pBlockFlags[0];
    *rContents << pBlockFlags[1];
    *rContents << pBlockFlags[2];
    *rContents << pBlockFlags[3];
    DBG_ASSERT((rContents.Is() &&
        (SVSTREAM_OK==rContents->GetError())),"damn");
    return bRet;
}


sal_Bool OCX_Image::Export(SvStorageRef &rObj,
    const uno::Reference< beans::XPropertySet > &rPropSet,
    const awt::Size &rSize)
{
    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x92, 0x59, 0x4C,
        0x26, 0x69, 0x1B, 0x10, 0x99, 0x92, 0x00, 0x00,
        0x0B, 0x65, 0xC6, 0xF9, 0x1A, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x49, 0x6D, 0x61, 0x67,
        0x65, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x0E, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x49, 0x6D, 0x61, 0x67, 0x65, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));
    DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
    SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
    xStor3->Write(aObjInfo,sizeof(aObjInfo));
    DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] = {
        0x49, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x67, 0x00,
        0x65, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    {
    SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
    xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
    DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

// ============================================================================

OCX_SpinButton::OCX_SpinButton() :
    OCX_Control( OUString( RTL_CONSTASCII_USTRINGPARAM( "SpinButton" ) ) ),
    mnBlockFlags( 0 ),
    mnValue( 0 ),
    mnMin( 0 ),
    mnMax( 100 ),
    mnSmallStep( 1 ),
    mnPageStep( 1 ),
    mnOrient( -1 ),
    mnDelay( 50 ),
    mbEnabled( true ),
    mbLocked( false ),
    mbPropThumb( true )
{
    msFormType = C2U("com.sun.star.form.component.SpinButton");
    msDialogType = C2U("com.sun.star.form.component.SpinButton");
    mnBackColor = 0x8000000F;
    mnForeColor = 0x80000012;
}

OCX_Control* OCX_SpinButton::Create()
{
    return new OCX_SpinButton;
}

sal_Bool OCX_SpinButton::Read( SvStorageStream *pS )
{
    if( !pS ) return sal_False;

    SvStream& rStrm = *pS;
    sal_uInt16 nId, nSize;
    sal_Int32 nIcon = 0;

    rStrm >> nId >> nSize >> mnBlockFlags;

    DBG_ASSERT( nStandardId == nId, "OCX_SpinButton::Read - unknown identifier" );

    if( mnBlockFlags & 0x00000001 )     rStrm >> mnForeColor;
    if( mnBlockFlags & 0x00000002 )     rStrm >> mnBackColor;
    if( mnBlockFlags & 0x00000004 )
    {
        sal_Int32 nFlags;
        rStrm >> nFlags;
        mbEnabled = (nFlags & 0x00000002) != 0;
        mbLocked = (nFlags & 0x00000004) != 0;
    }
    if( mnBlockFlags & 0x00000010 )     rStrm.SeekRel( 4 );     // mouse pointer
    if( mnBlockFlags & 0x00000020 )     rStrm >> mnMin;
    if( mnBlockFlags & 0x00000040 )     rStrm >> mnMax;
    if( mnBlockFlags & 0x00000080 )     rStrm >> mnValue;
    if( mnBlockFlags & 0x00000100 )     rStrm.SeekRel( 4 );     // unknown
    if( mnBlockFlags & 0x00000200 )     rStrm.SeekRel( 4 );     // unknown
    if( mnBlockFlags & 0x00000400 )     rStrm >> mnSmallStep;
    if( mnBlockFlags & 0x00000800 )     rStrm.SeekRel( 4 );     // unknown
    if( mnBlockFlags & 0x00001000 )     rStrm >> mnPageStep;
    if( mnBlockFlags & 0x00002000 )     rStrm >> mnOrient;
    if( mnBlockFlags & 0x00004000 )
    {
        sal_Int32 nThumb;
        *pS >> nThumb;
        mbPropThumb = nThumb != 0;
    }
    if( mnBlockFlags & 0x00008000 )     rStrm >> mnDelay;
    if( mnBlockFlags & 0x00010000 )     rStrm >> nIcon;
    if( mnBlockFlags & 0x00000008 )     rStrm >> nWidth >> nHeight;

    if( nIcon )
    {
        sal_Int32 nIconSize;
        pS->SeekRel( 20 );
        *pS >> nIconSize;
        pS->SeekRel( nIconSize );
    }

    return sal_True;
}

sal_Bool OCX_SpinButton::ReadFontData( SvStorageStream* /* pS */ )
{
    // spin buttons and scroll bars do not support font data
    return sal_True;
}

sal_Bool OCX_SpinButton::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    if( (nWidth < 1) || (nHeight < 1) )
        return sal_False;

    uno::Any aTmp( &sName, getCppuType((OUString *)0) );
    rPropSet->setPropertyValue( WW8_ASCII2STR( "Name" ), aTmp );

    aTmp <<= ImportColor( mnForeColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("SymbolColor"), aTmp);

    aTmp <<= ImportColor( mnBackColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp = bool2any( mbEnabled && !mbLocked );
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= mnValue;
    if ( bSetInDialog )
    {
        rPropSet->setPropertyValue( WW8_ASCII2STR("SpinValue"), aTmp );
    }
    else
    {
        rPropSet->setPropertyValue( WW8_ASCII2STR("DefaultSpinValue"), aTmp );
    }

    aTmp <<= mnMin;
    rPropSet->setPropertyValue( WW8_ASCII2STR("SpinValueMin"), aTmp );

    aTmp <<= mnMax;
    rPropSet->setPropertyValue( WW8_ASCII2STR("SpinValueMax"), aTmp );

    aTmp <<= mnSmallStep;
    rPropSet->setPropertyValue( WW8_ASCII2STR("SpinIncrement"), aTmp );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    switch( mnOrient )
    {
        case 0:     aTmp <<= AwtScrollOrient::VERTICAL;     break;
        case 1:     aTmp <<= AwtScrollOrient::HORIZONTAL;   break;
        default:    aTmp <<= (nWidth < nHeight) ? AwtScrollOrient::VERTICAL : AwtScrollOrient::HORIZONTAL;
    }
    rPropSet->setPropertyValue( WW8_ASCII2STR("Orientation"), aTmp );

    aTmp = bool2any( true );
    rPropSet->setPropertyValue( WW8_ASCII2STR("Repeat"), aTmp );

    aTmp <<= mnDelay;
    rPropSet->setPropertyValue( WW8_ASCII2STR("RepeatDelay"), aTmp );

    aTmp <<= sal_Int16( 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    uno::Reference< frame::XModel > xModel ( pDocSh ? pDocSh->GetModel() : NULL );
    lcl_ApplyListSourceAndBindableStuff( xModel, rPropSet, msCtrlSource, msRowSource );

    return sal_True;
}

sal_Bool OCX_SpinButton::Export(
        SvStorageRef &rObj,
        const uno::Reference< beans::XPropertySet>& rPropSet,
        const awt::Size& rSize )
{
    static sal_uInt8 __READONLY_DATA aCompObj[] =
    {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xB0, 0x6F, 0x17, 0x79,
        0xF2, 0xB7, 0xCE, 0x11, 0x97, 0xEF, 0x00, 0xAA,
        0x00, 0x6D, 0x27, 0x76, 0x1F, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x53, 0x70, 0x69, 0x6E,
        0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E, 0x00, 0x10,
        0x00, 0x00, 0x00, 0x45, 0x6D, 0x62, 0x65, 0x64,
        0x64, 0x65, 0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65,
        0x63, 0x74, 0x00, 0x13, 0x00, 0x00, 0x00, 0x46,
        0x6E, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E,
        0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    {
        SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
        xStor->Write(aCompObj,sizeof(aCompObj));
        DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
        SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
        xStor3->Write(aObjInfo,sizeof(aObjInfo));
        DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] =
    {
        0x53, 0x00, 0x70, 0x00, 0x69, 0x00, 0x6E, 0x00,
        0x42, 0x00, 0x75, 0x00, 0x74, 0x00, 0x74, 0x00,
        0x6F, 0x00, 0x6E, 0x00, 0x31, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    {
        SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
        xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
        DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_SpinButton::WriteContents(
        SvStorageStreamRef &rObj,
        const uno::Reference< beans::XPropertySet> &rPropSet,
        const awt::Size& rSize )
{
    if( !rObj.Is() )
        return sal_False;

    mnBlockFlags = 0x00000008;
    nWidth = rSize.Width;
    nHeight = rSize.Height;

    GetInt32Property( mnForeColor, rPropSet, WW8_ASCII2STR( "SymbolColor" ),     0x00000001 );
    GetInt32Property( mnBackColor, rPropSet, WW8_ASCII2STR( "BackgroundColor" ), 0x00000002 );
    GetBoolProperty(  mbEnabled,   rPropSet, WW8_ASCII2STR( "Enabled" ),         0x00000304 );
    GetInt32Property( mnMin,       rPropSet, WW8_ASCII2STR( "SpinValueMin" ),    0x00000020 );
    GetInt32Property( mnMax,       rPropSet, WW8_ASCII2STR( "SpinValueMax" ),    0x00000040 );
    GetInt32Property( mnValue,     rPropSet, WW8_ASCII2STR( "SpinValue" ),       0x00000080 );
    GetInt32Property( mnSmallStep, rPropSet, WW8_ASCII2STR( "SpinIncrement" ),   0x00000800 );
    GetInt32Property( mnDelay,     rPropSet, WW8_ASCII2STR( "RepeatDelay" ),     0x00008000 );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    sal_Int16 nApiOrient = sal_Int16();
    if( rPropSet->getPropertyValue( WW8_ASCII2STR( "Orientation" ) ) >>= nApiOrient )
        UpdateInt32Property( mnOrient, (nApiOrient == AwtScrollOrient::VERTICAL) ? 0 : 1, 0x00002000 );

    return WriteData( *rObj );
}

void OCX_SpinButton::UpdateInt32Property(
        sal_Int32& rnCoreValue, sal_Int32 nNewValue, sal_Int32 nBlockFlag )
{
    if( nNewValue != rnCoreValue )
    {
        rnCoreValue = nNewValue;
        mnBlockFlags |= nBlockFlag;
    }
}

void OCX_SpinButton::GetInt32Property(
        sal_Int32& rnCoreValue, const uno::Reference< beans::XPropertySet>& rxPropSet,
        const OUString& rPropName, sal_Int32 nBlockFlag )
{
    sal_Int32 nNewValue = 0;
    if( rxPropSet->getPropertyValue( rPropName ) >>= nNewValue )
        UpdateInt32Property( rnCoreValue, nNewValue, nBlockFlag );
}

void OCX_SpinButton::UpdateBoolProperty(
        bool& rbCoreValue, bool bNewValue, sal_Int32 nBlockFlag )
{
    if( bNewValue != rbCoreValue )
    {
        rbCoreValue = bNewValue;
        mnBlockFlags |= nBlockFlag;
    }
}

void OCX_SpinButton::GetBoolProperty(
        bool& rbCoreValue, const uno::Reference< beans::XPropertySet>& rxPropSet,
        const OUString& rPropName, sal_Int32 nBlockFlag )
{
    UpdateBoolProperty( rbCoreValue,
        any2bool( rxPropSet->getPropertyValue( rPropName ) ), nBlockFlag );
}

sal_Bool OCX_SpinButton::WriteData( SvStream& rStrm ) const
{
    sal_Bool bRet = sal_True;
    ULONG nStartPos = rStrm.Tell();

    rStrm << sal_Int32( 0 ) << mnBlockFlags;

    if( mnBlockFlags & 0x00000001 )     rStrm << ExportColor( mnForeColor );
    if( mnBlockFlags & 0x00000002 )     rStrm << ExportColor( mnBackColor );
    if( mnBlockFlags & 0x00000004 )
    {
        sal_Int32 nFlags = 0x00000019;  // always set
        if( mbEnabled ) nFlags |= 0x00000002;
        if( mbLocked )  nFlags |= 0x00000004;
        rStrm << nFlags;
    }
    if( mnBlockFlags & 0x00000020 )     rStrm << mnMin;
    if( mnBlockFlags & 0x00000040 )     rStrm << mnMax;
    if( mnBlockFlags & 0x00000080 )     rStrm << mnValue;
    if( mnBlockFlags & 0x00000100 )     rStrm << sal_Int32( 0 );    // unknown
    if( mnBlockFlags & 0x00000200 )     rStrm << sal_Int32( 0 );    // unknown
    if( mnBlockFlags & 0x00000400 )     rStrm << sal_Int32( 0 );    // unknown
    if( mnBlockFlags & 0x00000800 )     rStrm << mnSmallStep;
    if( mnBlockFlags & 0x00001000 )     rStrm << mnPageStep;
    if( mnBlockFlags & 0x00002000 )     rStrm << mnOrient;
    if( mnBlockFlags & 0x00004000 )     rStrm << sal_Int32( mbPropThumb ? 1 : 0 );
    if( mnBlockFlags & 0x00008000 )     rStrm << mnDelay;
    if( mnBlockFlags & 0x00000008 )     rStrm << nWidth << nHeight;

    sal_uInt16 nSize = static_cast< sal_uInt16 >( rStrm.Tell() - nStartPos - 4 );
    rStrm.Seek( nStartPos );
    rStrm << nStandardId << nSize;

    DBG_ASSERT( rStrm.GetError() == SVSTREAM_OK, "OCX_SpinButton::WriteData - error in stream" );
    return bRet;
}

// ============================================================================

OCX_ScrollBar::OCX_ScrollBar()
{
    sName = OUString( RTL_CONSTASCII_USTRINGPARAM( "ScrollBar" ) );
    mnMax = 32767;
    msFormType = C2U("com.sun.star.form.component.ScrollBar");
    msDialogType = C2U("com.sun.star.form.component.ScrollBar");

}

OCX_Control* OCX_ScrollBar::Create()
{
    return new OCX_ScrollBar;
}

sal_Bool OCX_ScrollBar::Import(com::sun::star::uno::Reference<
    com::sun::star::beans::XPropertySet> &rPropSet)
{
    if( (nWidth < 1) || (nHeight < 1) )
        return sal_False;

    uno::Any aTmp( &sName, getCppuType((OUString *)0) );
    rPropSet->setPropertyValue( WW8_ASCII2STR( "Name" ), aTmp );

    aTmp <<= ImportColor( mnForeColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("SymbolColor"), aTmp);

    aTmp <<= ImportColor( mnBackColor );
    rPropSet->setPropertyValue( WW8_ASCII2STR("BackgroundColor"), aTmp);

    aTmp = bool2any( mbEnabled && !mbLocked );
    rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), aTmp);

    aTmp <<= mnValue;
    if ( bSetInDialog )
    {
        rPropSet->setPropertyValue( WW8_ASCII2STR("ScrollValue"), aTmp );
    }
    else
    {
        rPropSet->setPropertyValue( WW8_ASCII2STR("DefaultScrollValue"), aTmp );
    }

    aTmp <<= mnMin;
    rPropSet->setPropertyValue( WW8_ASCII2STR("ScrollValueMin"), aTmp );

    aTmp <<= mnMax;
    rPropSet->setPropertyValue( WW8_ASCII2STR("ScrollValueMax"), aTmp );

    aTmp <<= mnSmallStep;
    rPropSet->setPropertyValue( WW8_ASCII2STR("LineIncrement"), aTmp );

    aTmp <<= mnPageStep;
    rPropSet->setPropertyValue( WW8_ASCII2STR("BlockIncrement"), aTmp );
    if( mbPropThumb && (mnPageStep > 0) )
        rPropSet->setPropertyValue( WW8_ASCII2STR("VisibleSize"), aTmp );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    switch( mnOrient )
    {
        case 0:     aTmp <<= AwtScrollOrient::VERTICAL;     break;
        case 1:     aTmp <<= AwtScrollOrient::HORIZONTAL;   break;
        default:    aTmp <<= (nWidth < nHeight) ? AwtScrollOrient::VERTICAL : AwtScrollOrient::HORIZONTAL;
    }
    rPropSet->setPropertyValue( WW8_ASCII2STR("Orientation"), aTmp );

    aTmp <<= mnDelay;
    rPropSet->setPropertyValue( WW8_ASCII2STR("RepeatDelay"), aTmp );

    aTmp <<= sal_Int16( 0 );
    rPropSet->setPropertyValue( WW8_ASCII2STR("Border"), aTmp);

    uno::Reference< frame::XModel > xModel ( pDocSh ? pDocSh->GetModel() : NULL );
    lcl_ApplyListSourceAndBindableStuff( xModel, rPropSet, msCtrlSource, msRowSource );

    return sal_True;
}

sal_Bool OCX_ScrollBar::Export(
        SvStorageRef &rObj,
        const uno::Reference< beans::XPropertySet>& rPropSet,
        const awt::Size& rSize )
{
    static sal_uInt8 __READONLY_DATA aCompObj[] =
    {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x81, 0xD1, 0xDF,
        0x2F, 0x5E, 0xCE, 0x11, 0xA4, 0x49, 0x00, 0xAA,
        0x00, 0x4A, 0x80, 0x3D, 0x1E, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x53, 0x63, 0x72, 0x6F,
        0x6C, 0x6C, 0x42, 0x61, 0x72, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64,
        0x65, 0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63,
        0x74, 0x00, 0x12, 0x00, 0x00, 0x00, 0x46, 0x6F,
        0x72, 0x6D, 0x73, 0x2E, 0x53, 0x63, 0x72, 0x6F,
        0x6C, 0x6C, 0x42, 0x61, 0x72, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
        SvStorageStreamRef xStor( rObj->OpenSotStream( C2S("\1CompObj")));
        xStor->Write(aCompObj,sizeof(aCompObj));
        DBG_ASSERT((xStor.Is() && (SVSTREAM_OK == xStor->GetError())),"damn");
    }

    {
        SvStorageStreamRef xStor3( rObj->OpenSotStream( C2S("\3ObjInfo")));
        xStor3->Write(aObjInfo,sizeof(aObjInfo));
        DBG_ASSERT((xStor3.Is() && (SVSTREAM_OK == xStor3->GetError())),"damn");
    }

    static sal_uInt8 __READONLY_DATA aOCXNAME[] =
    {
        0x53, 0x00, 0x63, 0x00, 0x72, 0x00, 0x6F, 0x00,
        0x6C, 0x00, 0x6C, 0x00, 0x42, 0x00, 0x61, 0x00,
        0x72, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    {
        SvStorageStreamRef xStor2( rObj->OpenSotStream( C2S("\3OCXNAME")));
        xStor2->Write(aOCXNAME,sizeof(aOCXNAME));
        DBG_ASSERT((xStor2.Is() && (SVSTREAM_OK == xStor2->GetError())),"damn");
    }

    SvStorageStreamRef xContents( rObj->OpenSotStream( C2S("contents")));
    return WriteContents(xContents, rPropSet, rSize);
}

sal_Bool OCX_ScrollBar::WriteContents(
        SvStorageStreamRef &rObj,
        const uno::Reference< beans::XPropertySet> &rPropSet,
        const awt::Size& rSize )
{
    if( !rObj.Is() )
        return sal_False;

    mnBlockFlags = 0x00000008;
    nWidth = rSize.Width;
    nHeight = rSize.Height;

    GetInt32Property( mnForeColor, rPropSet, WW8_ASCII2STR( "SymbolColor" ),     0x00000001 );
    GetInt32Property( mnBackColor, rPropSet, WW8_ASCII2STR( "BackgroundColor" ), 0x00000002 );
    GetBoolProperty(  mbEnabled,   rPropSet, WW8_ASCII2STR( "Enabled" ),         0x00000304 );
    GetInt32Property( mnMin,       rPropSet, WW8_ASCII2STR( "ScrollValueMin" ),  0x00000020 );
    GetInt32Property( mnMax,       rPropSet, WW8_ASCII2STR( "ScrollValueMax" ),  0x00000040 );
    GetInt32Property( mnValue,     rPropSet, WW8_ASCII2STR( "ScrollValue" ),     0x00000080 );
    GetInt32Property( mnSmallStep, rPropSet, WW8_ASCII2STR( "LineIncrement" ),   0x00000800 );
    GetInt32Property( mnPageStep,  rPropSet, WW8_ASCII2STR( "BlockIncrement" ),  0x00001000 );
    GetInt32Property( mnDelay,     rPropSet, WW8_ASCII2STR( "RepeatDelay" ),     0x00008000 );

    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;
    sal_Int16 nApiOrient = sal_Int16();
    if( rPropSet->getPropertyValue( WW8_ASCII2STR( "Orientation" ) ) >>= nApiOrient )
        UpdateInt32Property( mnOrient, (nApiOrient == AwtScrollOrient::VERTICAL) ? 0 : 1, 0x00002000 );

    UpdateBoolProperty( mbPropThumb, true, 0x00004000 );

    return WriteData( *rObj );
}

OCX_ProgressBar::OCX_ProgressBar() :
    OCX_Control( OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressBar" ) ) ),
    nMin( 0 ),
    nMax( 0 ),
    bFixedSingle(true),
    bEnabled( true ),
    b3d( true )
{
    msDialogType = C2U("com.sun.star.awt.UnoControlProgressBarModel");
    bSetInDialog = true;
}

sal_Bool OCX_ProgressBar::Read( SvStorageStream *pS )
{
    pS->SeekRel( 8 );
    *pS >> nWidth >> nHeight;
    pS->SeekRel( 12 );

    float fMin, fMax;
    *pS >> fMin >> fMax;
    nMin = static_cast< sal_Int32 >( fMin );
    nMax = static_cast< sal_Int32 >( fMax );
    bool bVisible = true;
    sal_uInt8 pUnknownFlags[4];
    pS->Read(pUnknownFlags,4);

    // complete guess, but we don't handle visible anyway
    if ( ( pUnknownFlags[2] & 0x8 ) &&  (  pUnknownFlags[2] & 0x2 ) )
        bVisible = false;

    sal_uInt32 nFlags;
    *pS >> nFlags;

    // seems these work out
    bFixedSingle = (nFlags & 0x01) != 0;
    bEnabled = (nFlags & 0x02) != 0;
    b3d = (nFlags & 0x04) != 0;

    return true;
}

OCX_Control* OCX_ProgressBar::Create()
{
    return new OCX_ProgressBar;
}

sal_Bool OCX_ProgressBar::Import(uno::Reference< beans::XPropertySet > &rPropSet)
{
    uno::Any aTmp(&sName,getCppuType((OUString *)0));
    rPropSet->setPropertyValue( WW8_ASCII2STR("Name"), aTmp );
    aTmp <<= nMax;
    rPropSet->setPropertyValue( WW8_ASCII2STR("ProgressValueMax"), aTmp );
    aTmp <<= nMin;
    rPropSet->setPropertyValue( WW8_ASCII2STR("ProgressValueMin"), aTmp );

    if ( !bEnabled )
        rPropSet->setPropertyValue( WW8_ASCII2STR("Enabled"), uno::makeAny( sal_False ) );
    return sal_True;
}
// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
