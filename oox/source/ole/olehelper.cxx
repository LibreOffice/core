/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <tools/globname.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>

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
using ::com::sun::star::lang::XServiceInfo;

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
const sal_Int16 FORMULAFIELD = -2;

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
        {  FORMULAFIELD,
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
        orBuffer[nCharIdx] = spcHexChars[ nValue & 0xF ];
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
        bool bIsStdFont = importGuid( rInStrm ) == OLE_GUID_STDFONT;
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
        bool bIsStdPic = importGuid( rInStrm ) == OLE_GUID_STDPIC;
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
    virtual ~OleFormCtrlExportHelper() { }
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
            /* psuedo ripped from legacy msocximex:
              "There is a truly horrible thing with EditControls and FormattedField
              Controls, they both pretend to have an EDITBOX ClassId for compability
              reasons, at some stage in the future hopefully there will be a proper
              FormulaField ClassId rather than this piggybacking two controls onto the
              same ClassId, cmc." - when fixed the fake FORMULAFIELD id entry
              and definition above can be removed/replaced
            */
            if ( nClassId == FormComponentType::TEXTFIELD)
            {
                Reference< XServiceInfo > xInfo( xCntrlModel,
                    UNO_QUERY);
                if (xInfo->
                    supportsService( CREATE_OUSTRING( "com.sun.star.form.component.FormattedField" ) ) )
                    nClassId = FORMULAFIELD;
            }
            else if ( nClassId == FormComponentType::COMMANDBUTTON )
            {
                bool bToggle = false;
                aPropSet.getProperty( bToggle, PROP_Toggle );
                if ( bToggle )
                    nClassId = TOGGLEBUTTON;
            }
            else if ( nClassId == FormComponentType::CONTROL )
            {
                Reference< XServiceInfo > xInfo( xCntrlModel,
                    UNO_QUERY);
                if (xInfo->
                    supportsService(OUString( CREATE_OUSTRING( "com.sun.star.form.component.ImageControl" ) ) ) )
                nClassId = FormComponentType::IMAGECONTROL;
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

MSConvertOCXControls::MSConvertOCXControls( const Reference< ::com::sun::star::frame::XModel >& rxModel ) : SvxMSConvertOCXControls( rxModel ), mxCtx( comphelper::getProcessComponentContext() ), maGrfHelper( mxCtx, lcl_getFrame( rxModel ), StorageRef() )
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
            if ( !aName.isEmpty() && xCtlModel.is() )
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

    OleFormCtrlExportHelper exportHelper( comphelper::getProcessComponentContext(), rxModel, rxControlModel );

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
                sName += rtl::OUString::valueOf(static_cast<sal_Int32>(++n));
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
