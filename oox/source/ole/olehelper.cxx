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

#include "oox/ole/olehelper.hxx"

#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/token/tokens.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/ole/olestorage.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <tools/globname.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>

namespace oox {
namespace ole {

// ============================================================================

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using ::com::sun::star::form::XFormComponent;
using ::com::sun::star::form::XForm;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::awt::Size;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::form::XFormsSupplier;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::drawing::XDrawPageSupplier;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::lang::XMultiServiceFactory;

using namespace ::com::sun::star::form;

// ============================================================================

namespace {

const sal_uInt32 OLE_COLORTYPE_MASK         = 0xFF000000;
const sal_uInt32 OLE_COLORTYPE_CLIENT       = 0x00000000;
const sal_uInt32 OLE_COLORTYPE_PALETTE      = 0x01000000;
const sal_uInt32 OLE_COLORTYPE_BGR          = 0x02000000;
const sal_uInt32 OLE_COLORTYPE_SYSCOLOR     = 0x80000000;

const sal_uInt32 OLE_PALETTECOLOR_MASK      = 0x0000FFFF;
const sal_uInt32 OLE_BGRCOLOR_MASK          = 0x00FFFFFF;
const sal_uInt32 OLE_SYSTEMCOLOR_MASK       = 0x0000FFFF;


/** Swaps the red and blue component of the passed color. */
inline sal_uInt32 lclSwapRedBlue( sal_uInt32 nColor )
{
    return static_cast< sal_uInt32 >( (nColor & 0xFF00FF00) | ((nColor & 0x0000FF) << 16) | ((nColor & 0xFF0000) >> 16) );
}

/** Returns the UNO RGB color from the passed encoded OLE BGR color. */
inline sal_Int32 lclDecodeBgrColor( sal_uInt32 nOleColor )
{
    return static_cast< sal_Int32 >( lclSwapRedBlue( nOleColor ) & 0xFFFFFF );
}

// ----------------------------------------------------------------------------

const sal_Char OLE_GUID_URLMONIKER[] = "{79EAC9E0-BAF9-11CE-8C82-00AA004BA90B}";
const sal_Char OLE_GUID_FILEMONIKER[] = "{00000303-0000-0000-C000-000000000046}";

const sal_uInt32 OLE_STDPIC_ID              = 0x0000746C;

const sal_uInt32 OLE_STDHLINK_VERSION       = 2;
const sal_uInt32 OLE_STDHLINK_HASTARGET     = 0x00000001;   /// Has hyperlink moniker.
const sal_uInt32 OLE_STDHLINK_ABSOLUTE      = 0x00000002;   /// Absolute path.
const sal_uInt32 OLE_STDHLINK_HASLOCATION   = 0x00000008;   /// Has target location.
const sal_uInt32 OLE_STDHLINK_HASDISPLAY    = 0x00000010;   /// Has display string.
const sal_uInt32 OLE_STDHLINK_HASGUID       = 0x00000020;   /// Has identification GUID.
const sal_uInt32 OLE_STDHLINK_HASTIME       = 0x00000040;   /// Has creation time.
const sal_uInt32 OLE_STDHLINK_HASFRAME      = 0x00000080;   /// Has frame.
const sal_uInt32 OLE_STDHLINK_ASSTRING      = 0x00000100;   /// Hyperlink as simple string.

struct GUIDCNamePair
{
    const char* sGUID;
    const char* sName;
};

struct IdCntrlData
{
    sal_Int16 nId;
    GUIDCNamePair aData;
};

const sal_Int16 TOGGLEBUTTON = -1;

typedef std::map< sal_Int16, GUIDCNamePair > GUIDCNamePairMap;
class classIdToGUIDCNamePairMap
{
    GUIDCNamePairMap mnIdToGUIDCNamePairMap;
    classIdToGUIDCNamePairMap();
public:
    static GUIDCNamePairMap& get();
};

classIdToGUIDCNamePairMap::classIdToGUIDCNamePairMap()
{
    IdCntrlData initialCntrlData[] =
    {
        // Command button MUST be at index 0
        { FormComponentType::COMMANDBUTTON,
             { AX_GUID_COMMANDBUTTON, "CommandButton"} ,
        },
        // Toggle button MUST be at index 1
        {  TOGGLEBUTTON,
           { AX_GUID_TOGGLEBUTTON, "ToggleButton"},
        },
        { FormComponentType::FIXEDTEXT,
             { AX_GUID_LABEL, "Label"},
        },
        {  FormComponentType::TEXTFIELD,
             { AX_GUID_TEXTBOX, "TextBox"},
        },
        {  FormComponentType::LISTBOX,
             { AX_GUID_LISTBOX, "ListBox"},
        },
        {  FormComponentType::COMBOBOX,
             { AX_GUID_COMBOBOX, "ComboBox"},
        },
        {  FormComponentType::CHECKBOX,
             { AX_GUID_CHECKBOX, "CheckBox"},
        },
        {  FormComponentType::RADIOBUTTON,
             { AX_GUID_OPTIONBUTTON, "OptionButton"},
        },
        {  FormComponentType::IMAGECONTROL,
             { AX_GUID_IMAGE, "Image"},
        },
        {  FormComponentType::DATEFIELD,
             { AX_GUID_TEXTBOX, "TextBox"},
        },
        {  FormComponentType::TIMEFIELD,
             { AX_GUID_TEXTBOX, "TextBox"},
        },
        {  FormComponentType::NUMERICFIELD,
             { AX_GUID_TEXTBOX, "TextBox"},
        },
        {  FormComponentType::CURRENCYFIELD,
             { AX_GUID_TEXTBOX, "TextBox"},
        },
        {  FormComponentType::PATTERNFIELD,
             { AX_GUID_TEXTBOX, "TextBox"},
        },
        {  FormComponentType::IMAGEBUTTON,
             { AX_GUID_COMMANDBUTTON, "CommandButton"},
        },
        {  FormComponentType::SPINBUTTON,
             { AX_GUID_SPINBUTTON, "SpinButton"},
        },
        {  FormComponentType::SCROLLBAR,
             { AX_GUID_SCROLLBAR, "ScrollBar"},
        }
    };
    int length = SAL_N_ELEMENTS( initialCntrlData );
    IdCntrlData* pData = initialCntrlData;
    for ( int index = 0; index < length; ++index, ++pData )
        mnIdToGUIDCNamePairMap[ pData->nId ] = pData->aData;
};

GUIDCNamePairMap& classIdToGUIDCNamePairMap::get()
{
    static classIdToGUIDCNamePairMap theInst;
    return theInst.mnIdToGUIDCNamePairMap;
}

// ----------------------------------------------------------------------------

template< typename Type >
void lclAppendHex( OUStringBuffer& orBuffer, Type nValue )
{
    const sal_Int32 nWidth = 2 * sizeof( Type );
    static const sal_Unicode spcHexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    orBuffer.setLength( orBuffer.getLength() + nWidth );
    for( sal_Int32 nCharIdx = orBuffer.getLength() - 1, nCharEnd = nCharIdx - nWidth; nCharIdx > nCharEnd; --nCharIdx, nValue >>= 4 )
        orBuffer.setCharAt( nCharIdx, spcHexChars[ nValue & 0xF ] );
}

OUString lclReadStdHlinkString( BinaryInputStream& rInStrm, bool bUnicode )
{
    OUString aRet;
    sal_Int32 nChars = rInStrm.readInt32();
    if( nChars > 0 )
    {
        sal_Int32 nReadChars = getLimitedValue< sal_Int32, sal_Int32 >( nChars, 0, SAL_MAX_UINT16 );
        // byte strings are always in ANSI (Windows 1252) encoding
        aRet = bUnicode ? rInStrm.readUnicodeArray( nReadChars, true ) : rInStrm.readCharArrayUC( nReadChars, RTL_TEXTENCODING_MS_1252, true );
        // strings are NUL terminated, remove trailing NUL and possible other garbage
        sal_Int32 nNulPos = aRet.indexOf( '\0' );
        if( nNulPos >= 0 )
            aRet = aRet.copy( 0, nNulPos );
        // skip remaining chars
        rInStrm.skip( (bUnicode ? 2 : 1) * (nChars - nReadChars) );
    }
    return aRet;
}

} // namespace

// ============================================================================

StdFontInfo::StdFontInfo() :
    mnHeight( 0 ),
    mnWeight( OLE_STDFONT_NORMAL ),
    mnCharSet( WINDOWS_CHARSET_ANSI ),
    mnFlags( 0 )
{
}

StdFontInfo::StdFontInfo( const ::rtl::OUString& rName, sal_uInt32 nHeight,
        sal_uInt16 nWeight, sal_uInt16 nCharSet, sal_uInt8 nFlags ) :
    maName( rName ),
    mnHeight( nHeight ),
    mnWeight( nWeight ),
    mnCharSet( nCharSet ),
    mnFlags( nFlags )
{
}

// ============================================================================

/*static*/ sal_Int32 OleHelper::decodeOleColor(
        const GraphicHelper& rGraphicHelper, sal_uInt32 nOleColor, bool bDefaultColorBgr )
{
    static const sal_Int32 spnSystemColors[] =
    {
        XML_scrollBar,      XML_background,     XML_activeCaption,  XML_inactiveCaption,
        XML_menu,           XML_window,         XML_windowFrame,    XML_menuText,
        XML_windowText,     XML_captionText,    XML_activeBorder,   XML_inactiveBorder,
        XML_appWorkspace,   XML_highlight,      XML_highlightText,  XML_btnFace,
        XML_btnShadow,      XML_grayText,       XML_btnText,        XML_inactiveCaptionText,
        XML_btnHighlight,   XML_3dDkShadow,     XML_3dLight,        XML_infoText,
        XML_infoBk
    };

    switch( nOleColor & OLE_COLORTYPE_MASK )
    {
        case OLE_COLORTYPE_CLIENT:
            return bDefaultColorBgr ? lclDecodeBgrColor( nOleColor ) : rGraphicHelper.getPaletteColor( nOleColor & OLE_PALETTECOLOR_MASK );

        case OLE_COLORTYPE_PALETTE:
            return rGraphicHelper.getPaletteColor( nOleColor & OLE_PALETTECOLOR_MASK );

        case OLE_COLORTYPE_BGR:
            return lclDecodeBgrColor( nOleColor );

        case OLE_COLORTYPE_SYSCOLOR:
            return rGraphicHelper.getSystemColor( STATIC_ARRAY_SELECT( spnSystemColors, nOleColor & OLE_SYSTEMCOLOR_MASK, XML_TOKEN_INVALID ), API_RGB_WHITE );
    }
    OSL_FAIL( "OleHelper::decodeOleColor - unknown color type" );
    return API_RGB_BLACK;
}

/*static*/ sal_uInt32 OleHelper::encodeOleColor( sal_Int32 nRgbColor )
{
    return OLE_COLORTYPE_BGR | lclSwapRedBlue( static_cast< sal_uInt32 >( nRgbColor & 0xFFFFFF ) );
}

/*static*/ OUString OleHelper::importGuid( BinaryInputStream& rInStrm )
{
    OUStringBuffer aBuffer;
    aBuffer.append( sal_Unicode( '{' ) );
    lclAppendHex( aBuffer, rInStrm.readuInt32() );
    aBuffer.append( sal_Unicode( '-' ) );
    lclAppendHex( aBuffer, rInStrm.readuInt16() );
    aBuffer.append( sal_Unicode( '-' ) );
    lclAppendHex( aBuffer, rInStrm.readuInt16() );
    aBuffer.append( sal_Unicode( '-' ) );
    lclAppendHex( aBuffer, rInStrm.readuInt8() );
    lclAppendHex( aBuffer, rInStrm.readuInt8() );
    aBuffer.append( sal_Unicode( '-' ) );
    for( int nIndex = 0; nIndex < 6; ++nIndex )
        lclAppendHex( aBuffer, rInStrm.readuInt8() );
    aBuffer.append( sal_Unicode( '}' ) );
    return aBuffer.makeStringAndClear();
}

/*static*/ bool OleHelper::importStdFont( StdFontInfo& orFontInfo, BinaryInputStream& rInStrm, bool bWithGuid )
{
    if( bWithGuid )
    {
        bool bIsStdFont = importGuid( rInStrm ).equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(OLE_GUID_STDFONT));
        OSL_ENSURE( bIsStdFont, "OleHelper::importStdFont - unexpected header GUID, expected StdFont" );
        if( !bIsStdFont )
            return false;
    }

    sal_uInt8 nVersion, nNameLen;
    rInStrm >> nVersion >> orFontInfo.mnCharSet >> orFontInfo.mnFlags >> orFontInfo.mnWeight >> orFontInfo.mnHeight >> nNameLen;
    // according to spec the name is ASCII
    orFontInfo.maName = rInStrm.readCharArrayUC( nNameLen, RTL_TEXTENCODING_ASCII_US );
    OSL_ENSURE( nVersion <= 1, "OleHelper::importStdFont - wrong version" );
    return !rInStrm.isEof() && (nVersion <= 1);
}

/*static*/ bool OleHelper::importStdPic( StreamDataSequence& orGraphicData, BinaryInputStream& rInStrm, bool bWithGuid )
{
    if( bWithGuid )
    {
        bool bIsStdPic = importGuid( rInStrm ).equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(OLE_GUID_STDPIC));
        OSL_ENSURE( bIsStdPic, "OleHelper::importStdPic - unexpected header GUID, expected StdPic" );
        if( !bIsStdPic )
            return false;
    }

    sal_uInt32 nStdPicId;
    sal_Int32 nBytes;
    rInStrm >> nStdPicId >> nBytes;
    OSL_ENSURE( nStdPicId == OLE_STDPIC_ID, "OleHelper::importStdPic - unexpected header version" );
    return !rInStrm.isEof() && (nStdPicId == OLE_STDPIC_ID) && (nBytes > 0) && (rInStrm.readData( orGraphicData, nBytes ) == nBytes);
}

/*static*/ bool OleHelper::importStdHlink( StdHlinkInfo& orHlinkInfo, BinaryInputStream& rInStrm, bool bWithGuid )
{
    if( bWithGuid )
    {
        bool bIsStdHlink = importGuid( rInStrm ).equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(OLE_GUID_STDHLINK));
        OSL_ENSURE( bIsStdHlink, "OleHelper::importStdHlink - unexpected header GUID, expected StdHlink" );
        if( !bIsStdHlink )
            return false;
    }

    sal_uInt32 nVersion, nFlags;
    rInStrm >> nVersion >> nFlags;
    OSL_ENSURE( nVersion == OLE_STDHLINK_VERSION, "OleHelper::importStdHlink - unexpected header version" );
    if( rInStrm.isEof() || (nVersion != OLE_STDHLINK_VERSION) )
        return false;

    // display string
    if( getFlag( nFlags, OLE_STDHLINK_HASDISPLAY ) )
        orHlinkInfo.maDisplay = lclReadStdHlinkString( rInStrm, true );
    // frame string
    if( getFlag( nFlags, OLE_STDHLINK_HASFRAME ) )
        orHlinkInfo.maFrame = lclReadStdHlinkString( rInStrm, true );

    // target
    if( getFlag( nFlags, OLE_STDHLINK_HASTARGET ) )
    {
        if( getFlag( nFlags, OLE_STDHLINK_ASSTRING ) )
        {
            OSL_ENSURE( getFlag( nFlags, OLE_STDHLINK_ABSOLUTE ), "OleHelper::importStdHlink - link not absolute" );
            orHlinkInfo.maTarget = lclReadStdHlinkString( rInStrm, true );
        }
        else // hyperlink moniker
        {
            OUString aGuid = importGuid( rInStrm );
            if( aGuid.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(OLE_GUID_FILEMONIKER)) )
            {
                // file name, maybe relative and with directory up-count
                sal_Int16 nUpLevels;
                rInStrm >> nUpLevels;
                OSL_ENSURE( (nUpLevels == 0) || !getFlag( nFlags, OLE_STDHLINK_ABSOLUTE ), "OleHelper::importStdHlink - absolute filename with upcount" );
                orHlinkInfo.maTarget = lclReadStdHlinkString( rInStrm, false );
                rInStrm.skip( 24 );
                sal_Int32 nBytes = rInStrm.readInt32();
                if( nBytes > 0 )
                {
                    sal_Int64 nEndPos = rInStrm.tell() + ::std::max< sal_Int32 >( nBytes, 0 );
                    sal_uInt16 nChars = getLimitedValue< sal_uInt16, sal_Int32 >( rInStrm.readInt32() / 2, 0, SAL_MAX_UINT16 );
                    rInStrm.skip( 2 );  // key value
                    orHlinkInfo.maTarget = rInStrm.readUnicodeArray( nChars );  // NOT null terminated
                    rInStrm.seek( nEndPos );
                }
                if( !getFlag( nFlags, OLE_STDHLINK_ABSOLUTE ) )
                    for( sal_Int16 nLevel = 0; nLevel < nUpLevels; ++nLevel )
                        orHlinkInfo.maTarget = CREATE_OUSTRING( "../" ) + orHlinkInfo.maTarget;
            }
            else if( aGuid.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(OLE_GUID_URLMONIKER)) )
            {
                // URL, maybe relative and with leading '../'
                sal_Int32 nBytes = rInStrm.readInt32();
                sal_Int64 nEndPos = rInStrm.tell() + ::std::max< sal_Int32 >( nBytes, 0 );
                orHlinkInfo.maTarget = rInStrm.readNulUnicodeArray();
                rInStrm.seek( nEndPos );
            }
            else
            {
                OSL_FAIL( "OleHelper::importStdHlink - unsupported hyperlink moniker" );
                return false;
            }
        }
    }

    // target location
    if( getFlag( nFlags, OLE_STDHLINK_HASLOCATION ) )
        orHlinkInfo.maLocation = lclReadStdHlinkString( rInStrm, true );

    return !rInStrm.isEof();
}

Reference< ::com::sun::star::frame::XFrame >
lcl_getFrame( const  Reference< ::com::sun::star::frame::XModel >& rxModel )
{
    Reference< ::com::sun::star::frame::XFrame > xFrame;
    if ( rxModel.is() )
    {
        Reference< ::com::sun::star::frame::XController > xController =  rxModel->getCurrentController();
        xFrame =  xController.is() ? xController->getFrame() : NULL;
    }
    return xFrame;
}

Reference< XComponentContext >
lcl_getUnoCtx()
{
    comphelper::ComponentContext aCtx( ::comphelper::getProcessServiceFactory() );
    return aCtx.getUNOContext();
}



class OleFormCtrlExportHelper
{
    ::oox::ole::EmbeddedControl maControl;
    ::oox::ole::ControlModelBase* mpModel;
    ::oox::GraphicHelper maGrfHelper;
    Reference< XModel > mxDocModel;
    Reference< XControlModel > mxControlModel;

    ::rtl::OUString maName;
    ::rtl::OUString maTypeName;
    ::rtl::OUString maFullName;
    ::rtl::OUString maGUID;
public:
    OleFormCtrlExportHelper( const Reference< XComponentContext >& rxCtx, const Reference< XModel >& xDocModel, const Reference< XControlModel >& xModel );
    virtual ::rtl::OUString getGUID()
    {
        rtl::OUString sResult;
        if ( maGUID.getLength() > 2 )
            sResult = maGUID.copy(1, maGUID.getLength() - 2 );
        return sResult;
    }
    ::rtl::OUString getFullName() { return maFullName; }
    ::rtl::OUString getTypeName() { return maTypeName; }
    bool isValid() { return mpModel != NULL; }
    void exportName( const Reference< XOutputStream >& rxOut );
    void exportCompObj( const Reference< XOutputStream >& rxOut );
    void exportControl( const Reference< XOutputStream >& rxOut, const ::com::sun::star::awt::Size& rSize );
};
OleFormCtrlExportHelper::OleFormCtrlExportHelper(  const Reference< XComponentContext >& rxCtx, const Reference< XModel >& rxDocModel, const Reference< XControlModel >& xCntrlModel ) : maControl( CREATE_OUSTRING( "Unknown" ) ), mpModel( NULL ), maGrfHelper( rxCtx, lcl_getFrame( rxDocModel ), StorageRef() ), mxDocModel( rxDocModel ), mxControlModel( xCntrlModel )
{
    // try to get the guid
    Reference< com::sun::star::beans::XPropertySet > xProps( xCntrlModel, UNO_QUERY );
    if ( xProps.is() )
    {
        sal_Int16 nClassId = 0;
        PropertySet aPropSet( mxControlModel );
        if ( aPropSet.getProperty( nClassId, PROP_ClassId ) )
        {
            // psuedo ripped from legacy msocximex
            if ( nClassId == FormComponentType::COMMANDBUTTON )
            {
                bool bToggle = false;
                aPropSet.getProperty( bToggle, PROP_Toggle );
                if ( bToggle )
                    nClassId = TOGGLEBUTTON;
            }

            GUIDCNamePairMap& cntrlMap = classIdToGUIDCNamePairMap::get();
            GUIDCNamePairMap::iterator it = cntrlMap.find( nClassId );
            if ( it != cntrlMap.end() )
            {
                aPropSet.getProperty(maName, PROP_Name );
                maTypeName = OUString::createFromAscii( it->second.sName );
                maFullName = CREATE_OUSTRING( "Microsoft Forms 2.0 " ) + maTypeName;
                maControl =  EmbeddedControl( maName );
                maGUID = OUString::createFromAscii( it->second.sGUID );
                mpModel = maControl.createModelFromGuid( maGUID );
            }
        }
    }
}

void OleFormCtrlExportHelper::exportName( const Reference< XOutputStream >& rxOut )
{
    oox::BinaryXOutputStream aOut( rxOut, false );
    aOut.writeUnicodeArray( maName );
    aOut << sal_Int32(0);
}

void OleFormCtrlExportHelper::exportCompObj( const Reference< XOutputStream >& rxOut )
{
    oox::BinaryXOutputStream aOut( rxOut, false );
    if ( mpModel && mpModel->getControlType() == API_CONTROL_BUTTON )
        mpModel->exportCompObj( aOut );
}

void OleFormCtrlExportHelper::exportControl( const Reference< XOutputStream >& rxOut, const Size& rSize )
{
    oox::BinaryXOutputStream aOut( rxOut, false );
    if ( mpModel )
    {
        ::oox::ole::ControlConverter aConv(  mxDocModel, maGrfHelper );
        maControl.convertFromProperties( mxControlModel, aConv );
        mpModel->maSize.first = rSize.Width;
        mpModel->maSize.second = rSize.Height;
        mpModel->exportBinaryModel( aOut );
    }
}

MSConvertOCXControls::MSConvertOCXControls( const Reference< ::com::sun::star::frame::XModel >& rxModel ) : SvxMSConvertOCXControls( rxModel ), mxCtx( lcl_getUnoCtx() ), maGrfHelper( mxCtx, lcl_getFrame( rxModel ), StorageRef() )
{
}

MSConvertOCXControls::~MSConvertOCXControls()
{
}

bool
MSConvertOCXControls::importControlFromStream( ::oox::BinaryInputStream& rInStrm, Reference< XFormComponent >& rxFormComp, const ::rtl::OUString& rGuidString )
{
    ::oox::ole::EmbeddedControl aControl( CREATE_OUSTRING( "Unknown" ) );
    if( ::oox::ole::ControlModelBase* pModel = aControl.createModelFromGuid( rGuidString  ) )
    {
        pModel->importBinaryModel( rInStrm );
        rxFormComp.set( mxCtx->getServiceManager()->createInstanceWithContext( pModel->getServiceName(), mxCtx ), UNO_QUERY );
        Reference< XControlModel > xCtlModel( rxFormComp, UNO_QUERY );
        ::oox::ole::ControlConverter aConv(  mxModel, maGrfHelper );
        aControl.convertProperties( xCtlModel, aConv );
    }
    return rxFormComp.is();
}

sal_Bool
MSConvertOCXControls::ReadOCXCtlsStream( SotStorageStreamRef& rSrc1, Reference< XFormComponent > & rxFormComp,
                                   sal_Int32 nPos,
                                   sal_Int32 nStreamSize)
{
    if ( rSrc1.Is()  )
    {
        BinaryXInputStream aCtlsStrm( Reference< XInputStream >( new utl::OSeekableInputStreamWrapper( *rSrc1 ) ), true );
        aCtlsStrm.seek( nPos );
        OUString aStrmClassId = ::oox::ole::OleHelper::importGuid( aCtlsStrm );
        return  importControlFromStream( aCtlsStrm, rxFormComp, aStrmClassId, nStreamSize );
    }
    return sal_False;
}

bool MSConvertOCXControls::importControlFromStream( ::oox::BinaryInputStream& rInStrm, Reference< XFormComponent >& rxFormComp, const rtl::OUString& rStrmClassId,
                                   sal_Int32 nStreamSize)
{
    if ( !rInStrm.isEof() )
    {
        // Special processing for those html controls
        bool bOneOfHtmlControls = false;
        if ( rStrmClassId.toAsciiUpperCase().equalsAscii( HTML_GUID_SELECT )
          || rStrmClassId.toAsciiUpperCase().equalsAscii( HTML_GUID_TEXTBOX ) )
            bOneOfHtmlControls = false;

        if ( bOneOfHtmlControls )
        {
            // html controls don't seem have a handy record length following the GUID
            // in the binary stream.
            // Given the control stream length create a stream of nStreamSize bytes starting from
            // nPos ( offset by the guid already read in )
            if ( !nStreamSize )
                return false;
            const int nGuidSize = 0x10;
            StreamDataSequence aDataSeq;
            sal_Int32 nBytesToRead = nStreamSize - nGuidSize;
            while ( nBytesToRead )
                nBytesToRead -= rInStrm.readData( aDataSeq, nBytesToRead );
            SequenceInputStream aInSeqStream( aDataSeq );
            importControlFromStream( aInSeqStream, rxFormComp, rStrmClassId );
        }
        else
        {
            importControlFromStream( rInStrm, rxFormComp, rStrmClassId );
        }
    }
    return rxFormComp.is();
}

sal_Bool MSConvertOCXControls::ReadOCXStorage( SotStorageRef& xOleStg,
                                  Reference< XFormComponent > & rxFormComp )
{
    if ( xOleStg.Is() )
    {
        SvStorageStreamRef pNameStream = xOleStg->OpenSotStream( CREATE_OUSTRING("\3OCXNAME"));
        BinaryXInputStream aNameStream( Reference< XInputStream >( new utl::OSeekableInputStreamWrapper( *pNameStream ) ), true );

        SvStorageStreamRef pContents = xOleStg->OpenSotStream( CREATE_OUSTRING("contents"));
        BinaryXInputStream aInStrm(  Reference< XInputStream >( new utl::OSeekableInputStreamWrapper( *pContents ) ), true );


        SvStorageStreamRef pClsStrm = xOleStg->OpenSotStream( CREATE_OUSTRING("\1CompObj") );
        BinaryXInputStream aClsStrm( Reference< XInputStream >( new utl::OSeekableInputStreamWrapper(*pClsStrm ) ), true );
        aClsStrm.skip(12);

        OUString aStrmClassId = ::oox::ole::OleHelper::importGuid( aClsStrm );
        if ( importControlFromStream(  aInStrm,  rxFormComp, aStrmClassId, aInStrm.size() ) )
        {
            OUString aName = aNameStream.readNulUnicodeArray();
            Reference< XControlModel > xCtlModel( rxFormComp, UNO_QUERY );
            if ( aName.getLength() && xCtlModel.is() )
            {
                PropertyMap aPropMap;
                aPropMap.setProperty( PROP_Name, aName );
                PropertySet aPropSet( xCtlModel );
                aPropSet.setProperties( aPropMap );
            }
            return rxFormComp.is();
        }
    }
    return  sal_False;
}

sal_Bool MSConvertOCXControls::WriteOCXStream( const Reference< XModel >& rxModel, SotStorageRef &xOleStg,
    const Reference< XControlModel > &rxControlModel,
    const com::sun::star::awt::Size& rSize, rtl::OUString &rName)
{
    SvGlobalName aName;

    OleFormCtrlExportHelper exportHelper( lcl_getUnoCtx(), rxModel, rxControlModel );

    if ( !exportHelper.isValid() )
        return sal_False;

    rtl::OUString sId = exportHelper.getGUID();
    aName.MakeId(sId);

    rtl::OUString sFullName = exportHelper.getFullName();
    rName = exportHelper.getTypeName();
    xOleStg->SetClass( aName,0x5C,sFullName);
    {
        SvStorageStreamRef pNameStream = xOleStg->OpenSotStream( CREATE_OUSTRING("\3OCXNAME"));
        Reference< XOutputStream > xOut = new utl::OSeekableOutputStreamWrapper( *pNameStream );
        exportHelper.exportName( xOut );
    }
    {
        SvStorageStreamRef pObjStream = xOleStg->OpenSotStream( CREATE_OUSTRING("\1CompObj"));
        Reference< XOutputStream > xOut = new utl::OSeekableOutputStreamWrapper( *pObjStream );
        exportHelper.exportCompObj( xOut );
    }
    {
        SvStorageStreamRef pContents = xOleStg->OpenSotStream( CREATE_OUSTRING("contents"));
        Reference< XOutputStream > xOut = new utl::OSeekableOutputStreamWrapper( *pContents );
        exportHelper.exportControl( xOut, rSize );
    }
    return sal_True;
}

#if SvxMSConvertOCXControlsRemoved
const Reference< com::sun::star::lang::XMultiServiceFactory > & MSConvertOCXControls::GetServiceFactory()
{
    if ( !mxServiceFactory.is() && mxModel.is() )
        mxServiceFactory.set( mxModel, UNO_QUERY );
    return mxServiceFactory;
}

const Reference< XIndexContainer >&
    MSConvertOCXControls::GetFormComps()
{
    if( !mxFormComps.is() )
    {
        GetDrawPage();
        if( mxDrawPage.is() )
        {
            Reference< XFormsSupplier > xFormsSupplier( mxDrawPage,
                UNO_QUERY );
            OSL_ENSURE( xFormsSupplier.is(),
                    "not able to get XFormsSupplier from XDrawPage" );

            Reference< XNameContainer >  xNameCont =
                xFormsSupplier->getForms();

            rtl::OUString sStdName = CREATE_OUSTRING( "WW-Standard" );
            rtl::OUString sName( sStdName );
            sal_uInt16 n = 0;

            while( xNameCont->hasByName( sName ) )
            {
                sName = sStdName;
                sName += String::CreateFromInt32( ++n );
            }

            const Reference< XMultiServiceFactory > &rServiceFactory
                = GetServiceFactory();
            if( !rServiceFactory.is() )
                return mxFormComps;

            Reference< XInterface >  xCreate =
                rServiceFactory->createInstance( CREATE_OUSTRING(
                    "com.sun.star.form.component.Form"));
            if( xCreate.is() )
            {
                Reference< XPropertySet > xFormPropSet( xCreate,
                    UNO_QUERY );

                Any aTmp(&sName,getCppuType((OUString *)0));
                xFormPropSet->setPropertyValue( CREATE_OUSTRING("Name"), aTmp );

                Reference< XForm > xForm( xCreate, UNO_QUERY );
                OSL_ENSURE(xForm.is(), "No Form?");

                Reference< XIndexContainer > xForms( xNameCont,
                    UNO_QUERY );
                OSL_ENSURE( xForms.is(), "XForms not available" );

                aTmp.setValue( &xForm,
                    ::getCppuType((Reference < XForm >*)0));
                xForms->insertByIndex( xForms->getCount(), aTmp );

                mxFormComps = Reference< XIndexContainer >
                    (xCreate, UNO_QUERY);
            }
        }
    }

    return mxFormComps;
}
const Reference< XDrawPage >& MSConvertOCXControls::GetDrawPage()
{
    if( !mxDrawPage.is() && mxModel.is() )
    {
        Reference< XDrawPageSupplier > xTxtDoc(mxModel, UNO_QUERY);
        OSL_ENSURE( xTxtDoc.is(),"no XDrawPageSupplier from XModel");
        mxDrawPage = xTxtDoc->getDrawPage();
        OSL_ENSURE( mxDrawPage.is(), "no XDrawPage" );
    }
    return mxDrawPage;
}

const Reference< XShapes >& MSConvertOCXControls::GetShapes()
{
    if( !mxShapes.is() )
    {
        GetDrawPage();
        if( mxDrawPage.is() )
        {

            mxShapes = Reference< XShapes >(mxDrawPage,
                UNO_QUERY);
            OSL_ENSURE( mxShapes.is(), "no XShapes from XDrawPage" );
        }
    }
    return mxShapes;
}
#endif
// ============================================================================

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
