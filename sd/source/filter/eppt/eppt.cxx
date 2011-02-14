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
#include "precompiled_sd.hxx"
#include <eppt.hxx>
#include "epptdef.hxx"
#include <tools/globname.hxx>
#include <tools/datetime.hxx>
#include <tools/poly.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/gradient.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <svtools/fltcall.hxx>
#include <sfx2/docfile.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/view/PaperFormat.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/office/XAnnotationEnumeration.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <tools/zcodec.hxx>
#include <editeng/svxenum.hxx>
#include <sot/storinfo.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <vcl/virdev.hxx>
#include <svtools/wmf.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/docinf.hxx>

#define PPT_TRANSITION_TYPE_NONE            0
#define PPT_TRANSITION_TYPE_RANDOM          1
#define PPT_TRANSITION_TYPE_BLINDS          2
#define PPT_TRANSITION_TYPE_CHECKER         3
#define PPT_TRANSITION_TYPE_COVER           4
#define PPT_TRANSITION_TYPE_DISSOLVE        5
#define PPT_TRANSITION_TYPE_FADE            6
#define PPT_TRANSITION_TYPE_PULL            7
#define PPT_TRANSITION_TYPE_RANDOM_BARS     8
#define PPT_TRANSITION_TYPE_STRIPS          9
#define PPT_TRANSITION_TYPE_WIPE           10
#define PPT_TRANSITION_TYPE_ZOOM           11
#define PPT_TRANSITION_TYPE_SPLIT          13

// effects, new in xp
#define PPT_TRANSITION_TYPE_DIAMOND         17
#define PPT_TRANSITION_TYPE_PLUS            18
#define PPT_TRANSITION_TYPE_WEDGE           19
#define PPT_TRANSITION_TYPE_PUSH            20
#define PPT_TRANSITION_TYPE_COMB            21
#define PPT_TRANSITION_TYPE_NEWSFLASH       22
#define PPT_TRANSITION_TYPE_SMOOTHFADE      23
#define PPT_TRANSITION_TYPE_WHEEL           26
#define PPT_TRANSITION_TYPE_CIRCLE          27

using namespace com::sun::star;

static PHLayout pPHLayout[] =
{
    { EPP_LAYOUT_TITLESLIDE,            { 0x0d, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x0d, 0x10, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_TITLEANDBODYSLIDE,     { 0x0d, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_TITLEANDBODYSLIDE,     { 0x0d, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x14, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_2COLUMNSANDTITLE,      { 0x0d, 0x0e, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x0d, 0x0e, sal_True, sal_True, sal_True },
    { EPP_LAYOUT_2COLUMNSANDTITLE,      { 0x0d, 0x0e, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x14, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_BLANCSLIDE,            { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x0d, 0x0e, sal_False, sal_False, sal_False },
    { EPP_LAYOUT_2COLUMNSANDTITLE,      { 0x0d, 0x0e, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x16, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_2COLUMNSANDTITLE,      { 0x0d, 0x14, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x14, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_TITLEANDBODYSLIDE,     { 0x0d, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x15, 0x0d, 0x0e, sal_True, sal_False, sal_False },
    { EPP_LAYOUT_2COLUMNSANDTITLE,      { 0x0d, 0x16, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x16, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_2COLUMNSANDTITLE,      { 0x0d, 0x0e, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_TITLEANDBODYSLIDE,     { 0x0d, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_False, sal_False },
    { EPP_LAYOUT_RIGHTCOLUMN2ROWS,      { 0x0d, 0x0e, 0x13, 0x13, 0x00, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_2COLUMNSANDTITLE,      { 0x0d, 0x13, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_2ROWSANDTITLE,         { 0x0d, 0x13, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_LEFTCOLUMN2ROWS,       { 0x0d, 0x13, 0x13, 0x0e, 0x00, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_TOPROW2COLUMN,         { 0x0d, 0x13, 0x13, 0x0e, 0x00, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_2ROWSANDTITLE,         { 0x0d, 0x0e, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_4OBJECTS,              { 0x0d, 0x13, 0x13, 0x13, 0x13, 0x00, 0x00, 0x00 }, 0x13, 0x0d, 0x0e, sal_True, sal_False, sal_False },
    { EPP_LAYOUT_ONLYTITLE,             { 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x0d, 0x0e, sal_True, sal_False, sal_False },
    { EPP_LAYOUT_BLANCSLIDE,            { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x0d, 0x0e, sal_False, sal_False, sal_False },
    { EPP_LAYOUT_TITLERIGHT2BODIESLEFT, { 0x11, 0x12, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x14, 0x11, 0x12, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_TITLERIGHTBODYLEFT,    { 0x11, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x11, 0x12, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_TITLEANDBODYSLIDE,     { 0x0d, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x0d, 0x12, sal_True, sal_True, sal_False },
    { EPP_LAYOUT_2COLUMNSANDTITLE,      { 0x0d, 0x16, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x16, 0x0d, 0x12, sal_True, sal_True, sal_False }
};

//============================ PPTWriter ==================================

PPTWriter::PPTWriter( SvStorageRef& rSvStorage,
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rXModel,
            ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > & rXStatInd,
            SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags ) :
    mbStatus                ( sal_False ),
    mbUseNewAnimations      ( sal_True ),
    mnLatestStatValue       ( 0 ),
    maFraction              ( 1, 576 ),
    maMapModeSrc            ( MAP_100TH_MM ),
    maMapModeDest           ( MAP_INCH, Point(), maFraction, maFraction ),
    meLatestPageType        ( NORMAL ),
    mXModel                 ( rXModel ),
    mXStatusIndicator       ( rXStatInd ),
    mbStatusIndicator       ( sal_False ),
    mpCurUserStrm           ( NULL ),
    mpStrm                  ( NULL ),
    mpPicStrm               ( NULL ),
    mpPptEscherEx           ( NULL ),
    mnVBAOleOfs             ( 0 ),
    mpVBA                   ( pVBA ),
    mnExEmbed               ( 0 ),
    mpExEmbed               ( new SvMemoryStream ),
    mnPagesWritten          ( 0 ),
    mnTxId                  ( 0x7a2f64 )
{
    sal_uInt32 i;
    if ( !ImplInitSOIface() )
        return;

    FontCollectionEntry aDefaultFontDesc( String( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) ),
                                            ::com::sun::star::awt::FontFamily::ROMAN,
                                                ::com::sun::star::awt::FontPitch::VARIABLE,
                                                    RTL_TEXTENCODING_MS_1252 );
    maFontCollection.GetId( aDefaultFontDesc ); // default is always times new roman

    if ( !ImplGetPageByIndex( 0, NOTICE ) )
        return;
    sal_Int32 nWidth = 21000;
    if ( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM(  "Width" ) ) ) )
        mAny >>= nWidth;
    sal_Int32 nHeight = 29700;
    if ( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) )
        mAny >>= nHeight;

    maNotesPageSize = ImplMapSize( ::com::sun::star::awt::Size( nWidth, nHeight ) );

    if ( !ImplGetPageByIndex( 0, MASTER ) )
        return;
    nWidth = 28000;
    if ( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) )
        mAny >>= nWidth;
    nHeight = 21000;
    if ( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) )
        mAny >>= nHeight;
    maDestPageSize = ImplMapSize( ::com::sun::star::awt::Size( nWidth, nHeight ) );

    mrStg = rSvStorage;
    if ( !mrStg.Is() )
        return;

    // MasterPages + Slides und Notizen + NotesMasterPage
    mnDrawings = mnMasterPages + ( mnPages << 1 ) + 1;

    if ( mXStatusIndicator.is() )
    {
        mbStatusIndicator = sal_True;
        mnStatMaxValue = ( mnPages + mnMasterPages ) * 5;
        mXStatusIndicator->start( String( RTL_CONSTASCII_USTRINGPARAM( "PowerPoint Export" ) ),
                                    mnStatMaxValue + ( mnStatMaxValue >> 3 ) );
    }

    SvGlobalName aGName( 0x64818d10L, 0x4f9b, 0x11cf, 0x86, 0xea, 0x00, 0xaa, 0x00, 0xb9, 0x29, 0xe8 );
    mrStg->SetClass( aGName, 0, String( RTL_CONSTASCII_USTRINGPARAM( "MS PowerPoint 97" ) ) );

    if ( !ImplCreateCurrentUserStream() )
        return;

    mpStrm = mrStg->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM( "PowerPoint Document" ) ) );
    if ( !mpStrm )
        return;

    if ( !mpPicStrm )
        mpPicStrm = mrStg->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM( "Pictures" ) ) );

    mpPptEscherEx = new PptEscherEx( *mpStrm );

    if ( !ImplGetStyleSheets() )
        return;

    if ( !ImplCreateDocument() )
        return;

    for ( i = 0; i < mnMasterPages; i++ )
    {
        if ( !ImplCreateMaster( i ) )
            return;
    }
    if ( !ImplCreateMainNotes() )
        return;
    for ( i = 0; i < mnPages; i++ )
    {
        if ( !ImplCreateSlide( i ) )
            return;
    }
    for ( i = 0; i < mnPages; i++ )
    {
        if ( !ImplCreateNotes( i ) )
            return;
    }
    if ( !ImplCloseDocument() )
        return;

    if ( mbStatusIndicator )
    {
        mXStatusIndicator->setText( String( RTL_CONSTASCII_USTRINGPARAM( "PowerPoint Export" ) ) );
        sal_uInt32 nValue = mnStatMaxValue + ( mnStatMaxValue >> 3 );
        if ( nValue > mnLatestStatValue )
        {
            mXStatusIndicator->setValue( nValue );
            mnLatestStatValue = nValue;
        }
    }

    ImplWriteOLE( nCnvrtFlags );

    ImplWriteVBA( pVBA );

    if ( !ImplWriteAtomEnding() )
        return;

    if ( !ImplCreateDocumentSummaryInformation( nCnvrtFlags ) )
        return;

    mbStatus = sal_True;
};


// ---------------------------------------------------------------------------------------------

PPTWriter::~PPTWriter()
{
    void*  pPtr;
    delete mpExEmbed;
    delete mpPptEscherEx;
    delete mpCurUserStrm;
    delete mpPicStrm;
    delete mpStrm;



    std::vector< PPTExStyleSheet* >::iterator aStyleSheetIter( maStyleSheetList.begin() );
    while( aStyleSheetIter < maStyleSheetList.end() )
        delete *aStyleSheetIter++;

    for ( pPtr = maSlideNameList.First(); pPtr; pPtr = maSlideNameList.Next() )
        delete (::rtl::OUString*)pPtr;
    for ( pPtr = maHyperlink.First(); pPtr; pPtr = maHyperlink.Next() )
        delete (EPPTHyperlink*)pPtr;
    for ( pPtr = maExOleObj.First(); pPtr; pPtr = maExOleObj.Next() )
        delete (PPTExOleObjEntry*)pPtr;

    if ( mbStatusIndicator )
        mXStatusIndicator->end();
}

// ---------------------------------------------------------------------------------------------

static inline sal_uInt32 PPTtoEMU( sal_Int32 nPPT )
{
    return (sal_uInt32)( (double)nPPT * 1587.5 );
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateCurrentUserStream()
{
    mpCurUserStrm = mrStg->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM( "Current User" ) ) );
    if ( !mpCurUserStrm )
        return sal_False;
    char pUserName[] = "Current User";
    sal_uInt32 nLenOfUserName = strlen( pUserName );
    sal_uInt32 nSizeOfRecord = 0x14 + ( ( nLenOfUserName + 4 ) & ~ 3 );

    *mpCurUserStrm << (sal_uInt16)0 << (sal_uInt16)EPP_CurrentUserAtom << nSizeOfRecord;
    *mpCurUserStrm << (sal_uInt32)0x14                  // Len
                   << (sal_uInt32)0xe391c05f;           // Magic

    sal_uInt32 nEditPos = mpCurUserStrm->Tell();
    *mpCurUserStrm << (sal_uInt32)0x0                   // OffsetToCurrentEdit;
                   << (sal_uInt16)nLenOfUserName        //
                   << (sal_uInt16)0x3f4                 // DocFileVersion
                   << (sal_uInt8)3                      // MajorVersion
                   << (sal_uInt8)0                      // MinorVersion
                   << (sal_uInt16)0;                    // Pad Word
    pUserName[ nLenOfUserName ] = 8;
    mpCurUserStrm->Write( pUserName, nLenOfUserName + 1 );
    for ( sal_uInt32 i = 0x15 + nLenOfUserName; i < nSizeOfRecord; i++ )
    {
        *mpCurUserStrm << (sal_uInt8)0;                 // pad bytes
    };
    mpCurUserStrm->Seek( nEditPos );
    return sal_True;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateDocumentSummaryInformation( sal_uInt32 nCnvrtFlags )
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        mXModel, uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());

    if (xDocProps.is()) {

        // no idea what this is...
        static sal_uInt8 aGuid[ 0x52 ] =
        {
            0x4e, 0x00, 0x00, 0x00,
            '{',0,'D',0,'B',0,'1',0,'A',0,'C',0,'9',0,'6',0,'4',0,'-',0,
            'E',0,'3',0,'9',0,'C',0,'-',0,'1',0,'1',0,'D',0,'2',0,'-',0,
            'A',0,'1',0,'E',0,'F',0,'-',0,'0',0,'0',0,'6',0,'0',0,'9',0,
            '7',0,'D',0,'A',0,'5',0,'6',0,'8',0,'9',0,'}',0
        };
        uno::Sequence<sal_uInt8> aGuidSeq(aGuid, 0x52);

        SvMemoryStream  aHyperBlob;
        ImplCreateHyperBlob( aHyperBlob );

        uno::Sequence<sal_uInt8> aHyperSeq(aHyperBlob.Tell());
        const sal_uInt8* pBlob(
            static_cast<const sal_uInt8*>(aHyperBlob.GetData()));
        for (sal_Int32 j = 0; j < aHyperSeq.getLength(); ++j) {
            aHyperSeq[j] = pBlob[j];
        }

        if ( nCnvrtFlags & 0x8000 )
        {
            uno::Sequence<sal_uInt8> aThumbSeq;
            if ( ImplGetPageByIndex( 0, NORMAL ) &&
                 ImplGetPropertyValue( mXPagePropSet,
                    String( RTL_CONSTASCII_USTRINGPARAM( "PreviewBitmap" ) ) ) )
            {
                aThumbSeq =
                    *static_cast<const uno::Sequence<sal_uInt8>*>(mAny.getValue());
            }
            sfx2::SaveOlePropertySet( xDocProps, mrStg,
                    &aThumbSeq, &aGuidSeq, &aHyperSeq);
        }
        else
        {
            sfx2::SaveOlePropertySet( xDocProps, mrStg,
                    NULL, &aGuidSeq, &aHyperSeq );
        }
    }

    return sal_True;
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteExtParaHeader( SvMemoryStream& rSt, sal_uInt32 nRef, sal_uInt32 nInstance, sal_uInt32 nSlideId )
{
    if ( rSt.Tell() )
    {
        aBuExOutlineStream << (sal_uInt32)( ( EPP_PST_ExtendedParagraphHeaderAtom << 16 )
                                | ( nRef << 4 ) )
                            << (sal_uInt32)8
                            << (sal_uInt32)nSlideId
                            << (sal_uInt32)nInstance;
        aBuExOutlineStream.Write( rSt.GetData(), rSt.Tell() );
    }
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplCreateHeaderFooterStrings( SvStream& rStrm, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPagePropSet )
{
    if ( rXPagePropSet.is() )
    {
        rtl::OUString aString;
        ::com::sun::star::uno::Any aAny;
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "HeaderText" ) ), sal_True ) )
        {
            if ( aAny >>= aString )
                PPTWriter::WriteCString( rStrm, aString, 1 );
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FooterText" ) ), sal_True ) )
        {
            if ( aAny >>= aString )
                PPTWriter::WriteCString( rStrm, aString, 2 );
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "DateTimeText" ) ), sal_True ) )
        {
            if ( aAny >>= aString )
                PPTWriter::WriteCString( rStrm, aString, 0 );
        }
    }
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplCreateHeaderFooters( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPagePropSet )
{
    if ( rXPagePropSet.is() )
    {
        sal_Bool bVal = sal_False;
        sal_uInt32 nVal = 0;
        ::com::sun::star::uno::Any aAny;
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsHeaderVisible" ) ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && bVal )
                nVal |= 0x100000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFooterVisible" ) ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && bVal )
                nVal |= 0x200000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsDateTimeVisible" ) ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && bVal )
                nVal |= 0x010000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsPageNumberVisible" ) ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && bVal )
                nVal |= 0x080000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsDateTimeFixed" ) ), sal_True ) )
        {
            if ( ( aAny >>= bVal ) && !bVal )
                nVal |= 0x20000;
            else
                nVal |= 0x40000;
        }
        if ( PropValue::GetPropertyValue( aAny, rXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "DateTimeFormat" ) ), sal_True ) )
        {
            sal_Int32 nFormat = *(sal_Int32*)aAny.getValue();
            SvxDateFormat eDateFormat = (SvxDateFormat)( nFormat & 0xf );
            SvxTimeFormat eTimeFormat = (SvxTimeFormat)( ( nFormat >> 4 ) & 0xf );
            switch( eDateFormat )
            {
                case SVXDATEFORMAT_F :
                    nFormat = 1;
                break;
                case SVXDATEFORMAT_D :
                    nFormat = 2;
                break;
                case SVXDATEFORMAT_C :
                    nFormat = 4;
                break;
                default:
                case SVXDATEFORMAT_A :
                    nFormat = 0;
            }
            switch( eTimeFormat )
            {
                case SVXTIMEFORMAT_24_HM :
                    nFormat = 9;
                break;
                case SVXTIMEFORMAT_12_HM :
                    nFormat = 11;
                break;
                case SVXTIMEFORMAT_24_HMS :
                    nFormat = 10;
                break;
                case SVXTIMEFORMAT_12_HMS :
                    nFormat = 12;
                break;
                default:
                    break;
            }
            nVal |= nFormat;
        }

        mpPptEscherEx->OpenContainer( EPP_HeadersFooters, 0 );
        mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
        *mpStrm << nVal;
        ImplCreateHeaderFooterStrings( *mpStrm, rXPagePropSet );
        mpPptEscherEx->CloseContainer();
    }
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateDocument()
{
    sal_uInt32 i;
    sal_uInt16 nSlideType = EPP_SLIDESIZE_TYPECUSTOM;

    sal_uInt32 nWidth = maDestPageSize.Width;
    sal_uInt32 nHeight = maDestPageSize.Height;

    if ( ( nWidth == 0x1680 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPEONSCREEN;
    else if ( ( nWidth == 0x1200 ) && ( nHeight == 0x240 ) )
        nSlideType = EPP_SLIDESIZE_TYPEBANNER;
    else if ( ( nWidth == 0x1950 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPE35MM;
    else if ( ( nWidth == 0x1860 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPEA4PAPER;

    mpPptEscherEx->OpenContainer( EPP_Document );
    // CREATE DOCUMENT ATOM
    mpPptEscherEx->AddAtom( 40, EPP_DocumentAtom, 1 );
    *mpStrm << nWidth                           // Slide Size in Master coordinates X
            << nHeight                          //   "     "   "    "        "      Y
            << (sal_Int32)maNotesPageSize.Width     // Notes Page Size                  X
            << (sal_Int32)maNotesPageSize.Height    //   "     "   "                    Y
            << (sal_Int32)1 << (sal_Int32)2;            // the scale used when the Powerpoint document is embedded. the default is 1:2
    mpPptEscherEx->InsertPersistOffset( EPP_MAINNOTESMASTER_PERSIST_KEY, mpStrm->Tell() );
    *mpStrm << (sal_uInt32)0                        // Reference to NotesMaster ( 0 if none );
            << (sal_uInt32)0                        // Reference to HandoutMaster ( 0 if none );
            << (sal_Int16)1                         // Number of the first slide;
            << nSlideType                           // Size of the document slides ( default: EPP_SLIDESIZETYPEONSCREEN )
            << (sal_uInt8)0                         // bool1 indicates if document was saved with embedded true type fonts
            << (sal_uInt8)0                         // bool1 indicates if the placeholders on the title slide are omitted
            << (sal_uInt8)0                         // bool1 right to left ( flag for Bidi version )
            << (sal_uInt8)1;                            // bool1 visibility of comments shapes

    mpPptEscherEx->PtInsert( EPP_Persist_Document, mpStrm->Tell() );

    mpPptEscherEx->OpenContainer( EPP_HeadersFooters, 3 );  //Master footer (default)
    mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
    *mpStrm << (sal_uInt32)0x25000d;
    if ( ImplGetPageByIndex( 0, MASTER ) )
        ImplCreateHeaderFooterStrings( *mpStrm, mXPagePropSet );
    mpPptEscherEx->CloseContainer();
    mpPptEscherEx->OpenContainer( EPP_HeadersFooters, 4 );  //NotesMaster footer (default)
    mpPptEscherEx->AddAtom( 4, EPP_HeadersFootersAtom );
    *mpStrm << (sal_uInt32)0x3d000d;
    if ( ImplGetPageByIndex( 0, NOTICE ) )
        ImplCreateHeaderFooterStrings( *mpStrm, mXPagePropSet );
    mpPptEscherEx->CloseContainer();

    mpPptEscherEx->OpenContainer( EPP_SlideListWithText );      // Animation info fuer die Slides

    for ( i = 0; i < mnPages; i++ )
    {
        mpPptEscherEx->AddAtom( 20, EPP_SlidePersistAtom );
        mpPptEscherEx->InsertPersistOffset( EPP_MAINSLIDE_PERSIST_KEY | i, mpStrm->Tell() );
        *mpStrm << (sal_uInt32)0                                // psrReference - logical reference to the slide persist object ( EPP_MAINSLIDE_PERSIST_KEY )
                << (sal_uInt32)4                                // flags - only bit 3 used, if set then slide contains shapes other than placeholders
                << (sal_Int32)0                                     // numberTexts - number of placeholder texts stored with the persist object.  Allows to display outline view without loading the slide persist objects
                << (sal_Int32)i + 0x100                             // slideId - Unique slide identifier, used for OLE link monikers for example
                << (sal_uInt32)0;                               // reserved, usualy 0

        if ( !ImplGetPageByIndex( i, NORMAL ) )                 // sehr aufregend: noch einmal ueber alle seiten
            return sal_False;
        ImplSetCurrentStyleSheet( ImplGetMasterIndex( NORMAL ) );

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >
            aXName( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

        if ( aXName.is() )
        {
            ::rtl::OUString aStr = aXName->getName();
            ::rtl::OUString *pUStr = new ::rtl::OUString( aStr );
            maSlideNameList.Insert( pUStr, LIST_APPEND );
        }
        else
            maSlideNameList.Insert( new ::rtl::OUString(), LIST_APPEND );
    }
    mpPptEscherEx->CloseContainer();    // EPP_SlideListWithText

    mpPptEscherEx->OpenContainer( EPP_SlideListWithText, 2 );   // Animation info fuer die notes
    for( i = 0; i < mnPages; i++ )
    {
        mpPptEscherEx->AddAtom( 20, EPP_SlidePersistAtom );
        mpPptEscherEx->InsertPersistOffset( EPP_MAINNOTES_PERSIST_KEY | i, mpStrm->Tell() );
        *mpStrm << (sal_uInt32)0
                << (sal_uInt32)4
                << (sal_Int32)0
                << (sal_Int32)i + 0x100
                << (sal_uInt32)0;
    }
    mpPptEscherEx->CloseContainer();        // EPP_SlideListWithText

    ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentationSupplier >
        aXPresSupplier( mXModel, ::com::sun::star::uno::UNO_QUERY );            ;
    if ( aXPresSupplier.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentation >
            aXPresentation( aXPresSupplier->getPresentation() );
        if ( aXPresentation.is() )
        {
            mXPropSet = ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >
                    ( aXPresentation, ::com::sun::star::uno::UNO_QUERY );
            if ( mXPropSet.is() )
            {
                ::rtl::OUString aCustomShow;
                sal_uInt32  nPenColor = 0x1000000;
                sal_Int32   nRestartTime = 0x7fffffff;
                sal_Int16   nStartSlide = 0;
                sal_Int16   nEndSlide = 0;
                sal_uInt32  nFlags = 0;             // Bit 0:   Auto advance
                                                    // Bit 1    Skip builds ( do not allow slide effects )
                                                    // Bit 2    Use slide range
                                                    // Bit 3    Use named show
                                                    // Bit 4    Browse mode on
                                                    // Bit 5    Kiosk mode on
                                                    // Bit 7    loop continously
                                                    // Bit ?    show scrollbar

                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CustomShow" ) ) ) )
                {
                    aCustomShow = ( *(::rtl::OUString*)mAny.getValue() );
                    if ( aCustomShow.getLength() )
                    {
                        nFlags |= 8;
                    }
                }
                if ( ( nFlags & 8 ) == 0 )
                {
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "FirstPage" ) ) ) )
                    {
                        ::rtl::OUString aSlideName( *(::rtl::OUString*)mAny.getValue() );
                        ::rtl::OUString* pStr;
                        for ( pStr = (::rtl::OUString*)maSlideNameList.First(); pStr;
                                    pStr = (::rtl::OUString*)maSlideNameList.Next(), nStartSlide++ )
                        {
                            if ( *pStr == aSlideName )
                            {
                                nStartSlide++;
                                nFlags |= 4;
                                nEndSlide = (sal_uInt16)mnPages;
                                break;
                            }
                        }
                        if ( !pStr )
                            nStartSlide = 0;
                    }
                }

//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DiaName" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsAlwaysOnTop" ) ) ) )
//              {
//              }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsAutomatic" ) ) ) )
                {
                    sal_Bool bBool = sal_False;
                    mAny >>= bBool;
                    if ( !bBool )
                        nFlags |= 1;
                }

                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsEndless" ) ) ) ) // muesste eigendlich heissen IsNotEndless !=)"§()&
                {
                    sal_Bool bBool = sal_False;
                    mAny >>= bBool;
                    if ( bBool )
                        nFlags |= 0x80;
                }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFullScreen" ) ) ) )
                {
                    sal_Bool bBool = sal_False;
                    mAny >>= bBool;
                    if ( !bBool )
                        nFlags |= 0x11;
                }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsMouseVisible" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "StartWithNavigator" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "UsePen" ) ) ) )
//              {
//              }
                mpPptEscherEx->AddAtom( 80, EPP_SSDocInfoAtom, 1 );
                *mpStrm << nPenColor << nRestartTime << nStartSlide << nEndSlide;

                sal_uInt32 nCustomShowNameLen = aCustomShow.getLength();
                if ( nCustomShowNameLen > 31 )
                    nCustomShowNameLen = 31;
                if ( nCustomShowNameLen )       // named show identifier
                {
                    const sal_Unicode* pCustomShow = aCustomShow.getStr();
                    for ( i = 0; i < nCustomShowNameLen; i++ )
                    {
                        *mpStrm << (sal_uInt16)( pCustomShow[ i ] );
                    }
                }
                for ( i = nCustomShowNameLen; i < 32; i++, *mpStrm << (sal_uInt16)0 ) ;

                *mpStrm << nFlags;
                ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XCustomPresentationSupplier >
                    aXCPSup( mXModel, ::com::sun::star::uno::UNO_QUERY );
                if ( aXCPSup.is() )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        aXCont( aXCPSup->getCustomPresentations() );
                    if ( aXCont.is() )
                    {
                        ::com::sun::star::uno::Sequence< ::rtl::OUString> aNameSeq( aXCont->getElementNames() );
                        const ::rtl::OUString* pUString = aNameSeq.getArray();
                        sal_uInt32 nCount = aNameSeq.getLength();
                        if ( nCount )
                        {
                            mpPptEscherEx->OpenContainer( EPP_NamedShows );
                            sal_uInt32 nCustomShowIndex = 0;
                            for( i = 0; i < nCount; i++ )        // Anzahl der Custom Shows
                            {
                                if ( pUString[ i ].getLength() )
                                {
                                    mpPptEscherEx->OpenContainer( EPP_NamedShow, nCustomShowIndex++ );

                                    sal_uInt32 nNamedShowLen = pUString[ i ].getLength();
                                    if ( nNamedShowLen > 31 )
                                        nNamedShowLen = 31;
                                    mpPptEscherEx->AddAtom( nNamedShowLen << 1, EPP_CString );
                                    const sal_Unicode* pCustomShowName = pUString[ i ].getStr();
                                    for ( sal_uInt32 k = 0; k < nNamedShowLen; *mpStrm << (sal_uInt16)( pCustomShowName[ k++ ] ) ) ;
                                    mAny = aXCont->getByName( pUString[ i ] );
                                    if ( mAny.getValue() )
                                    {

                                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > aXIC;
                                        if ( mAny >>= aXIC )
                                        {
                                            mpPptEscherEx->BeginAtom();

                                            sal_Int32 nSlideCount = aXIC->getCount();
                                            for ( sal_Int32 j = 0; j < nSlideCount; j++ )   // Anzahl der Slides
                                            {
                                                mAny = aXIC->getByIndex( j );
                                                if ( mAny.getValue() )
                                                {
                                                    ::com::sun::star::uno::Reference<
                                                        ::com::sun::star::drawing::XDrawPage > aXDrawPage;
                                                    if ( mAny >>= aXDrawPage )
                                                    {
                                                        ::com::sun::star::uno::Reference<
                                                            ::com::sun::star::container::XNamed >
                                                            aXName( aXDrawPage, ::com::sun::star::uno::UNO_QUERY );
                                                        if ( aXName.is() )
                                                        {
                                                            ::rtl::OUString aSlideName( aXName->getName() );
                                                            sal_uInt32 nPageNumber = 0;
                                                            for ( ::rtl::OUString* pSlideName = (::rtl::OUString*)maSlideNameList.First();
                                                                pSlideName;
                                                                pSlideName = (::rtl::OUString*)maSlideNameList.Next(), nPageNumber++ )
                                                            {
                                                                if ( *pSlideName == aSlideName )
                                                                {
                                                                    *mpStrm << (sal_uInt32)( nPageNumber + 0x100 ); // unique slide id
                                                                    break;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            mpPptEscherEx->EndAtom( EPP_NamedShowSlides );
                                        }
                                    }
                                    mpPptEscherEx->CloseContainer();            // EPP_NamedShow
                                }
                            }
                            mpPptEscherEx->CloseContainer();                // EPP_NamedShows
                        }
                    }
                }
            }
        }
    }
    mpPptEscherEx->AddAtom( 0, EPP_EndDocument );
    mpPptEscherEx->CloseContainer();    // EPP_Document
    return sal_True;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateHyperBlob( SvMemoryStream& rStrm )
{
    sal_uInt32 nCurrentOfs, nParaOfs, nParaCount = 0;
// SfxOlePropertySection does this...
//    rStrm << (sal_uInt32)0x41;      // property type VT_BLOB
    nParaOfs = rStrm.Tell();
    rStrm << (sal_uInt32)0;         // property size
    rStrm << (sal_uInt32)0;         // property count

    for ( EPPTHyperlink* pLink = (EPPTHyperlink*)maHyperlink.First(); pLink; pLink = (EPPTHyperlink*)maHyperlink.Next() )
    {
        nParaCount += 6;
        rStrm   << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)7    // (VTI4 - Private1)
                << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)6    // (VTI4 - Private2)
                << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)0;   // (VTI4 - Private3)

        // INFO
        // HIWORD:  = 0 : do not change anything
        //          = 1 : replace the hyperlink with the target and subadress in the following two VTLPWSTR
        //          = 2 : delete the hyperlink
        // LOWORD:  = 0 : graphic shown as background (link)
        //          = 1 : graphic shown as shape (link)
        //          = 2 : graphic is used to fill a shape
        //          = 3 : graphic used to fill a shape outline (future use)
        //          = 4 : hyperlink attached to a shape
        //          = 5 :    "         "      " " (Word) field
        //          = 6 :    "         "      " " (Excel) range
        //          = 7 :    "         "      " " (PPT) text range
        //          = 8 :    "         "      " " (Project) task

        sal_uInt32 nUrlLen = pLink->aURL.Len();
        const sal_Unicode* pUrl = pLink->aURL.GetBuffer();

        sal_uInt32 nInfo = 7;

        rStrm   << (sal_uInt32)3    // Type VT_I4
                << nInfo;       // Info

        switch( pLink->nType & 0xff )
        {
            case 1 :        // click action to slidenumber
            {
                rStrm << (sal_uInt32)0x1f << (sal_uInt32)1 << (sal_uInt32)0;    // path
                rStrm << (sal_uInt32)0x1f << (sal_uInt32)( nUrlLen + 1 );
                for ( sal_uInt32 i = 0; i < nUrlLen; i++ )
                {
                    rStrm << pUrl[ i ];
                }
                rStrm << (sal_uInt16)0;
            }
            break;
            case 2 :
            {
                sal_uInt32 i;

                rStrm   << (sal_uInt32)0x1f
                        << (sal_uInt32)( nUrlLen + 1 );
                for ( i = 0; i < nUrlLen; i++ )
                {
                    rStrm << pUrl[ i ];
                }
                if ( ! ( i & 1 ) )
                    rStrm << (sal_uInt16)0;
                rStrm   << (sal_uInt16)0
                        << (sal_uInt32)0x1f
                        << (sal_uInt32)1
                        << (sal_uInt32)0;
            }
            break;
        }
    }
    nCurrentOfs = rStrm.Tell();
    rStrm.Seek( nParaOfs );
    rStrm << (sal_uInt32)( nCurrentOfs - ( nParaOfs + 4 ) );
    rStrm << nParaCount;
    rStrm.Seek( nCurrentOfs );
    return sal_True;
}

PHLayout& PPTWriter::ImplGetLayout(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPropSet ) const
{
    ::com::sun::star::uno::Any aAny;
    sal_Int16 nLayout = 20;
    if ( GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Layout" ) ) ), sal_True )
        aAny >>= nLayout;

    if ( ( nLayout >= 21 ) && ( nLayout <= 26 ) )   // NOTES _> HANDOUT6
        nLayout = 20;
    if ( ( nLayout >= 27 ) && ( nLayout <= 30 ) )   // VERTICAL LAYOUT
        nLayout -= 6;
    else if ( nLayout > 30 )
        nLayout = 20;
    return pPHLayout[ nLayout ];
}


// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateMaster( sal_uInt32 nPageNum )
{
    if ( !ImplGetPageByIndex( nPageNum, MASTER ) )
        return sal_False;
    ImplSetCurrentStyleSheet( nPageNum );

    if ( !ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Background" ) ) ) )                // Backgroundshape laden
        return sal_False;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet;
    if ( !( mAny >>= aXBackgroundPropSet ) )
        return sal_False;

    sal_uInt32 nFillColor = 0xffffff;
    sal_uInt32 nFillBackColor = 0x000000;

    ::com::sun::star::drawing::FillStyle aFS = ::com::sun::star::drawing::FillStyle_NONE;
    if ( ImplGetPropertyValue( aXBackgroundPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) ) )
        mAny >>= aFS;
    switch ( aFS )
    {
        case ::com::sun::star::drawing::FillStyle_GRADIENT :
        {
            if ( ImplGetPropertyValue( aXBackgroundPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ) ) ) )
            {
                nFillColor = EscherPropertyContainer::GetGradientColor( (::com::sun::star::awt::Gradient*)mAny.getValue(), 0 );
                nFillBackColor = EscherPropertyContainer::GetGradientColor( (::com::sun::star::awt::Gradient*)mAny.getValue(), 1 );
            }
        }
        break;

        case ::com::sun::star::drawing::FillStyle_SOLID :
        {
            if ( ImplGetPropertyValue( aXBackgroundPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) ) )
            {
                nFillColor = mpPptEscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) );
                nFillBackColor = nFillColor ^ 0xffffff;
            }
        }
        break;

        default:
            break;
    }

    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainMaster | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_MainMaster );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    *mpStrm << (sal_Int32)EPP_LAYOUT_TITLEANDBODYSLIDE  // slide layout -> title and body slide
            << (sal_uInt8)1 << (sal_uInt8)2 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0     // placeholderID
            << (sal_uInt32)0        // master ID ( ist gleich null bei einer masterpage )
            << (sal_uInt32)0        // notes ID ( ist gleich null wenn keine notizen vorhanden )
            << (sal_uInt16)0        // Bit 1: Follow master objects, Bit 2: Follow master scheme, Bit 3: Follow master background
            << (sal_uInt16)0;       // padword

    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xff0000 << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x00ffff << (sal_uInt32)0x0099ff << (sal_uInt32)0xffff00 << (sal_uInt32)0x0000ff << (sal_uInt32)0x969696;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xccffff << (sal_uInt32)0x000000 << (sal_uInt32)0x336666 << (sal_uInt32)0x008080 << (sal_uInt32)0x339933 << (sal_uInt32)0x000080 << (sal_uInt32)0xcc3300 << (sal_uInt32)0x66ccff;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x333333 << (sal_uInt32)0x000000 << (sal_uInt32)0xdddddd << (sal_uInt32)0x808080 << (sal_uInt32)0x4d4d4d << (sal_uInt32)0xeaeaea;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x66ccff << (sal_uInt32)0xff0000 << (sal_uInt32)0xcc00cc << (sal_uInt32)0xc0c0c0;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0xc0c0c0 << (sal_uInt32)0xff6600 << (sal_uInt32)0x0000ff << (sal_uInt32)0x009900;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0xff9933 << (sal_uInt32)0xccff99 << (sal_uInt32)0xcc00cc << (sal_uInt32)0xb2b2b2;

    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        if ( nInstance == EPP_TEXTTYPE_notUsed )
            continue;

        // the auto color is dependent to the page background,so we have to set a page that is in the right context
        if ( nInstance == EPP_TEXTTYPE_Notes )
            ImplGetPageByIndex( 0, NOTICE );
        else
            ImplGetPageByIndex( 0, MASTER );

        mpPptEscherEx->BeginAtom();

        sal_Bool bFirst = sal_True;
        sal_Bool bSimpleText = sal_False;

        *mpStrm << (sal_uInt16)5;                           // paragraph count

        for ( sal_uInt16 nLev = 0; nLev < 5; nLev++ )
        {
            if ( nInstance >= EPP_TEXTTYPE_CenterBody )
            {
                bFirst = sal_False;
                bSimpleText = sal_True;
                *mpStrm << nLev;
            }
            mpStyleSheet->mpParaSheet[ nInstance ]->Write( *mpStrm, mpPptEscherEx, nLev, bFirst, bSimpleText, mXPagePropSet );
            mpStyleSheet->mpCharSheet[ nInstance ]->Write( *mpStrm, mpPptEscherEx, nLev, bFirst, bSimpleText, mXPagePropSet );
            bFirst = sal_False;
        }
        mpPptEscherEx->EndAtom( EPP_TxMasterStyleAtom, 0, nInstance );
    }
    ImplGetPageByIndex( nPageNum, MASTER );

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );

    mpPptEscherEx->EnterGroup(0,0);
    ImplWritePage( pPHLayout[ 0 ], aSolverContainer, MASTER, sal_True );    // Die Shapes der Seite werden im PPT Dok. erzeugt
    mpPptEscherEx->LeaveGroup();

    ImplWriteBackground( aXBackgroundPropSet );

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;

    if ( aBuExMasterStream.Tell() )
    {
        ImplProgTagContainer( mpStrm, &aBuExMasterStream );
    }
    mpPptEscherEx->CloseContainer();    // EPP_MainMaster
    return sal_True;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateMainNotes()
{
    if ( !ImplGetPageByIndex( 0, NOTICE ) )
        return sal_False;
    ImplSetCurrentStyleSheet( 0 );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPageTarget >
        aXMasterPageTarget( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

    if ( !aXMasterPageTarget.is() )
        return sal_False;

    mXDrawPage = aXMasterPageTarget->getMasterPage();
    if ( !mXDrawPage.is() )
        return sal_False;

    mXPropSet = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
    if ( !mXPropSet.is() )
        return sal_False;

    mXShapes = ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShapes >
            ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
    if ( !mXShapes.is() )
        return sal_False;

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainNotes, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Notes );
    mpPptEscherEx->AddAtom( 8, EPP_NotesAtom, 1 );
    *mpStrm << (sal_uInt32)0x80000001                                               // Number that identifies this slide
            << (sal_uInt32)0;                                                       // follow nothing
    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(0,0);

    ImplWritePage( pPHLayout[ 20 ], aSolverContainer, NOTICE, sal_True );

    mpPptEscherEx->LeaveGroup();
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0xffffff );                             // stock valued fill color
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, 0x68bdde );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, 0x8b9f8e );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );                            // if true, this is the background shape
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->CloseContainer();    // EPP_Notes
    return sal_True;
}

// ---------------------------------------------------------------------------------------------

static rtl::OUString getInitials( const rtl::OUString& rName )
{
    rtl::OUString sInitials;

    const sal_Unicode * pStr = rName.getStr();
    sal_Int32 nLength = rName.getLength();

    while( nLength )
    {
        // skip whitespace
        while( nLength && (*pStr <= ' ') )
        {
            nLength--; pStr++;
        }

        // take letter
        if( nLength )
        {
            sInitials += rtl::OUString( *pStr );
            nLength--; pStr++;
        }

        // skip letters until whitespace
        while( nLength && (*pStr > ' ') )
        {
            nLength--; pStr++;
        }
    }

    return sInitials;
}

void ImplExportComments( uno::Reference< drawing::XDrawPage > xPage, SvMemoryStream& rBinaryTagData10Atom )
{
    try
    {
        uno::Reference< office::XAnnotationAccess > xAnnotationAccess( xPage, uno::UNO_QUERY_THROW );
        uno::Reference< office::XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );

        sal_Int32 nIndex = 1;

        while( xAnnotationEnumeration->hasMoreElements() )
        {
            EscherExContainer aComment10( rBinaryTagData10Atom, EPP_Comment10 );
            {
                uno::Reference< office::XAnnotation > xAnnotation( xAnnotationEnumeration->nextElement() );

                geometry::RealPoint2D aRealPoint2D( xAnnotation->getPosition() );
                MapMode aMapDest( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) );
                Point aPoint( OutputDevice::LogicToLogic( Point( static_cast< sal_Int32 >( aRealPoint2D.X * 100.0 ),
                    static_cast< sal_Int32 >( aRealPoint2D.Y * 100.0 ) ), MAP_100TH_MM, aMapDest ) );

                rtl::OUString sAuthor( xAnnotation->getAuthor() );
                uno::Reference< text::XText > xText( xAnnotation->getTextRange() );
                rtl::OUString sText( xText->getString() );
                rtl::OUString sInitials( getInitials( sAuthor ) );
                util::DateTime aDateTime( xAnnotation->getDateTime() );
                if ( sAuthor.getLength() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sAuthor, 0 );
                if ( sText.getLength() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sText, 1 );
                if ( sInitials.getLength() )
                    PPTWriter::WriteCString( rBinaryTagData10Atom, sInitials, 2 );

                sal_Int16 nMilliSeconds = aDateTime.HundredthSeconds * 10;
                EscherExAtom aCommentAtom10( rBinaryTagData10Atom, EPP_CommentAtom10 );
                rBinaryTagData10Atom << nIndex++
                                     << aDateTime.Year
                                     << aDateTime.Month
                                     << aDateTime.Day   // todo: day of week
                                     << aDateTime.Day
                                     << aDateTime.Hours
                                     << aDateTime.Minutes
                                     << aDateTime.Seconds
                                     << nMilliSeconds
                                     << static_cast< sal_Int32 >( aPoint.X() )
                                     << static_cast< sal_Int32 >( aPoint.Y() );
            }
        }
    }
    catch ( uno::Exception& )
    {
    }
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateSlide( sal_uInt32 nPageNum )
{
    ::com::sun::star::uno::Any aAny;

    if ( !ImplGetPageByIndex( nPageNum, NORMAL ) )
        return sal_False;
    sal_uInt32 nMasterID = ImplGetMasterIndex( NORMAL );
    ImplSetCurrentStyleSheet( nMasterID );
    nMasterID |= 0x80000000;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet;
    sal_Bool bHasBackground = GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Background" ) ) );
    if ( bHasBackground )
        bHasBackground = ( aAny >>= aXBackgroundPropSet );

    sal_uInt16 nMode = 7;   // Bit 1: Follow master objects, Bit 2: Follow master scheme, Bit 3: Follow master background
    if ( bHasBackground )
        nMode &=~4;

/* sj: Don't know what's IsBackgroundVisible for, have to ask cl
    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsBackgroundVisible" ) ) ) )
    {
        sal_Bool bBackgroundVisible;
        if ( aAny >>= bBackgroundVisible )
        {
            if ( bBackgroundVisible )
                nMode &= ~4;
        }
    }
*/
    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsBackgroundObjectsVisible" ) ) ) )
    {
        sal_Bool bBackgroundObjectsVisible = sal_False;
        if ( aAny >>= bBackgroundObjectsVisible )
        {
            if ( !bBackgroundObjectsVisible )
                nMode &= ~1;
        }
    }

    const PHLayout& rLayout = ImplGetLayout( mXPagePropSet );
    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_Slide | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Slide );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    *mpStrm << rLayout.nLayout;
    mpStrm->Write( rLayout.nPlaceHolder, 8 );       // placeholderIDs ( 8Stueck )
    *mpStrm << (sal_uInt32)nMasterID                // master ID ( ist gleich 0x80000000 bei einer masterpage   )
            << (sal_uInt32)nPageNum + 0x100         // notes ID ( ist gleich null wenn keine notizen vorhanden )
            << nMode
            << (sal_uInt16)0;                       // padword

    mnDiaMode = 0;
    sal_Bool bVisible = sal_True;
    ::com::sun::star::presentation::FadeEffect eFe = ::com::sun::star::presentation::FadeEffect_NONE;

    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Visible" ) ) ) )
        aAny >>= bVisible;
    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Change" ) ) ) )
    {
        switch ( *(sal_Int32*)aAny.getValue() )
        {
            case 1 :        // automatisch
                mnDiaMode++;
            case 2 :        // halbautomatisch
                mnDiaMode++;
            default :
            case 0 :        // manuell
            break;
        }
    }
    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Effect" ) ) ) )
        aAny >>= eFe;

    sal_uInt32  nSoundRef = 0;
    sal_Bool    bIsSound = sal_False;
    sal_Bool    bStopSound = sal_False;
    sal_Bool    bLoopSound = sal_False;

    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Sound" ) ) ) )
    {
        rtl::OUString aSoundURL;
        if ( aAny >>= aSoundURL )
        {
            nSoundRef = maSoundCollection.GetId( aSoundURL );
            bIsSound = sal_True;
        }
        else
            aAny >>= bStopSound;
    }
    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "LoopSound" ) ) ) )
        aAny >>= bLoopSound;


    sal_Bool bNeedsSSSlideInfoAtom = ( bVisible == sal_False )
                                    || ( mnDiaMode == 2 )
                                    || ( bIsSound )
                                    || ( bStopSound )
                                    || ( eFe != ::com::sun::star::presentation::FadeEffect_NONE );
    if ( bNeedsSSSlideInfoAtom )
    {
        sal_uInt8   nDirection = 0;
        sal_uInt8   nTransitionType = 0;
        sal_uInt16  nBuildFlags = 1;        // advange by mouseclick
        sal_Int32       nSlideTime = 0;         // muss noch !!!
        sal_uInt8   nSpeed = 1;

        if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Speed" ) ) ) )
        {
            ::com::sun::star::presentation::AnimationSpeed aAs;
            aAny >>= aAs;
            nSpeed = (sal_uInt8)aAs;
        }
        sal_Int16 nTT = 0, nTST = 0;
        if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TransitionType" ) ) )
            && ( aAny >>= nTT ) )
        {
            if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TransitionSubtype" ) ) )
                && ( aAny >>= nTST ) )
            {
                switch( nTT )
                {
                    case animations::TransitionType::FADE :
                    {
                        if ( nTST == animations::TransitionSubType::CROSSFADE )
                            nTransitionType = PPT_TRANSITION_TYPE_SMOOTHFADE;
                        else if ( nTST == animations::TransitionSubType::FADEOVERCOLOR )
                            nTransitionType = PPT_TRANSITION_TYPE_FADE;
                    }
                    break;
                    case PPT_TRANSITION_TYPE_COMB :
                    {
                        nTransitionType = PPT_TRANSITION_TYPE_COMB;
                        if ( nTST == animations::TransitionSubType::COMBVERTICAL )
                            nDirection++;
                    }
                    break;
                    case animations::TransitionType::PUSHWIPE :
                    {
                        nTransitionType = PPT_TRANSITION_TYPE_PUSH;
                        switch( nTST )
                        {
                            case animations::TransitionSubType::FROMRIGHT: nDirection = 0; break;
                            case animations::TransitionSubType::FROMBOTTOM: nDirection = 1; break;
                            case animations::TransitionSubType::FROMLEFT: nDirection = 2; break;
                            case animations::TransitionSubType::FROMTOP: nDirection = 3; break;
                        }
                    }
                    break;
                    case animations::TransitionType::PINWHEELWIPE :
                    {
                        nTransitionType = PPT_TRANSITION_TYPE_WHEEL;
                        switch( nTST )
                        {
                            case animations::TransitionSubType::ONEBLADE: nDirection = 1; break;
                            case animations::TransitionSubType::TWOBLADEVERTICAL : nDirection = 2; break;
                            case animations::TransitionSubType::THREEBLADE : nDirection = 3; break;
                            case animations::TransitionSubType::FOURBLADE: nDirection = 4; break;
                            case animations::TransitionSubType::EIGHTBLADE: nDirection = 8; break;
                        }
                    }
                    break;
                    case animations::TransitionType::FANWIPE :
                    {
                        nTransitionType = PPT_TRANSITION_TYPE_WEDGE;
                    }
                    break;
                    case animations::TransitionType::ELLIPSEWIPE :
                    {
                        nTransitionType = PPT_TRANSITION_TYPE_CIRCLE;
                    }
                    break;
                    case animations::TransitionType::FOURBOXWIPE :
                    {
                        nTransitionType = PPT_TRANSITION_TYPE_PLUS;
                    }
                    break;
                    case animations::TransitionType::IRISWIPE :
                    {
                        nTransitionType = PPT_TRANSITION_TYPE_DIAMOND;
                    }
                    break;
                }
            }
        }
        if ( !nTransitionType )
        {
            switch ( eFe )
            {
                default :
                case ::com::sun::star::presentation::FadeEffect_RANDOM :
                    nTransitionType = PPT_TRANSITION_TYPE_RANDOM;
                break;

                case ::com::sun::star::presentation::FadeEffect_HORIZONTAL_STRIPES :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_VERTICAL_STRIPES :
                    nTransitionType = PPT_TRANSITION_TYPE_BLINDS;
                break;

                case ::com::sun::star::presentation::FadeEffect_VERTICAL_CHECKERBOARD :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_HORIZONTAL_CHECKERBOARD :
                    nTransitionType = PPT_TRANSITION_TYPE_CHECKER;
                break;

                case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_UPPERLEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_UPPERRIGHT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LOWERLEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LOWERRIGHT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_TOP :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_BOTTOM :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_RIGHT :
                    nTransitionType = PPT_TRANSITION_TYPE_COVER;
                break;

                case ::com::sun::star::presentation::FadeEffect_DISSOLVE :
                    nTransitionType = PPT_TRANSITION_TYPE_DISSOLVE;
                break;

                case ::com::sun::star::presentation::FadeEffect_VERTICAL_LINES :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_HORIZONTAL_LINES :
                    nTransitionType = PPT_TRANSITION_TYPE_RANDOM_BARS;
                break;

                case ::com::sun::star::presentation::FadeEffect_CLOSE_HORIZONTAL :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_OPEN_HORIZONTAL :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_CLOSE_VERTICAL :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_OPEN_VERTICAL :
                    nTransitionType = PPT_TRANSITION_TYPE_SPLIT;
                break;

                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_UPPERLEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_UPPERRIGHT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_LOWERLEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_LOWERRIGHT :
                    nDirection += 4;
                    nTransitionType = PPT_TRANSITION_TYPE_STRIPS;
                break;

                case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LOWERLEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_UPPERLEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_BOTTOM :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_RIGHT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_TOP :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LEFT :
                    nTransitionType = PPT_TRANSITION_TYPE_PULL;
                break;

                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_TOP :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_LEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_BOTTOM :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_RIGHT :
                    nTransitionType = PPT_TRANSITION_TYPE_WIPE;
                break;

                case ::com::sun::star::presentation::FadeEffect_ROLL_FROM_TOP :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_ROLL_FROM_LEFT :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_ROLL_FROM_BOTTOM :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_ROLL_FROM_RIGHT :
                    nTransitionType = PPT_TRANSITION_TYPE_WIPE;
                break;

                case ::com::sun::star::presentation::FadeEffect_FADE_TO_CENTER :
                    nDirection++;
                case ::com::sun::star::presentation::FadeEffect_FADE_FROM_CENTER :
                    nTransitionType = PPT_TRANSITION_TYPE_ZOOM;
                break;

                case ::com::sun::star::presentation::FadeEffect_NONE :
                    nDirection = 2;
                break;
            }
        }
        if ( mnDiaMode == 2 )                                   // automatic ?
            nBuildFlags |= 0x400;
        if ( bVisible == sal_False )
            nBuildFlags |= 4;
        if ( bIsSound )
            nBuildFlags |= 16;
        if ( bLoopSound )
            nBuildFlags |= 64;
        if ( bStopSound )
            nBuildFlags |= 256;

        if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Duration" ) ) ) )// duration of this slide
            nSlideTime = *(sal_Int32*)aAny.getValue() << 10;        // in ticks


        mpPptEscherEx->AddAtom( 16, EPP_SSSlideInfoAtom );
        *mpStrm << nSlideTime       // standtime in ticks
                << nSoundRef
                << nDirection
                << nTransitionType
                << nBuildFlags
                << nSpeed
                << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0;
    }

    ImplCreateHeaderFooters( mXPagePropSet );

    EscherSolverContainer aSolverContainer;
    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(0,0);
    ImplWritePage( rLayout, aSolverContainer, NORMAL, sal_False, nPageNum );    // Die Shapes der Seite werden im PPT Dok. erzeugt
    mpPptEscherEx->LeaveGroup();

    if ( bHasBackground )
        ImplWriteBackground( aXBackgroundPropSet );
    else
    {
        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
        mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );             // Flags: Connector | Background | HasSpt
        EscherPropertyContainer aPropOpt;
        aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, PPTtoEMU( maDestPageSize.Width ) );
        aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, PPTtoEMU( maDestPageSize.Width ) );
        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
        aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
        aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
        aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );                // if true, this is the background shape
        aPropOpt.Commit( *mpStrm );
        mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
    }

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;

    SvMemoryStream aBinaryTagData10Atom;
    ImplExportComments( mXDrawPage, aBinaryTagData10Atom );
    if ( mbUseNewAnimations )
    {
        SvMemoryStream amsofbtAnimGroup;
        ppt::AnimationExporter aExporter( aSolverContainer, maSoundCollection );
        aExporter.doexport( mXDrawPage, amsofbtAnimGroup );
        sal_uInt32 nmsofbtAnimGroupSize = amsofbtAnimGroup.Tell();
        if ( nmsofbtAnimGroupSize )
        {
            {
                EscherExAtom aMagic2( aBinaryTagData10Atom, 0x2eeb );
                aBinaryTagData10Atom << (sal_uInt32)0x01c45df9
                                     << (sal_uInt32)0xe1471b30;
            }
            {
                EscherExAtom aMagic( aBinaryTagData10Atom, 0x2b00 );
                aBinaryTagData10Atom << (sal_uInt32)0;
            }
            aBinaryTagData10Atom.Write( amsofbtAnimGroup.GetData(), amsofbtAnimGroup.Tell() );
            {
                EscherExContainer aMagic2( aBinaryTagData10Atom, 0x2b02 );
            }
        }
    }
    if ( aBinaryTagData10Atom.Tell() )
    {
        EscherExContainer aProgTags     ( *mpStrm, EPP_ProgTags );
        EscherExContainer aProgBinaryTag( *mpStrm, EPP_ProgBinaryTag );
        {
            EscherExAtom aCString( *mpStrm, EPP_CString );
            *mpStrm << (sal_uInt32)0x5f005f
                    << (sal_uInt32)0x50005f
                    << (sal_uInt32)0x540050
                    << (sal_uInt16)0x31
                    << (sal_uInt16)0x30;
        }
        {
            EscherExAtom aBinaryTagData( *mpStrm, EPP_BinaryTagData );
            mpStrm->Write( aBinaryTagData10Atom.GetData(), aBinaryTagData10Atom.Tell() );
        }
    }
/*
    if ( mbUseNewAnimations )
    {
        SvMemoryStream amsofbtAnimGroup;
        ppt::AnimationExporter aExporter( aSolverContainer, maSoundCollection );
        aExporter.doexport( mXDrawPage, amsofbtAnimGroup );
        sal_uInt32 nmsofbtAnimGroupSize = amsofbtAnimGroup.Tell();
        if ( nmsofbtAnimGroupSize )
        {
            EscherExContainer aProgTags     ( *mpStrm, EPP_ProgTags );
            EscherExContainer aProgBinaryTag( *mpStrm, EPP_ProgBinaryTag );
            {
                EscherExAtom aCString( *mpStrm, EPP_CString );
                *mpStrm << (sal_uInt32)0x5f005f
                        << (sal_uInt32)0x50005f
                        << (sal_uInt32)0x540050
                        << (sal_uInt16)0x31
                        << (sal_uInt16)0x30;
            }
            {
                EscherExAtom aBinaryTagData( *mpStrm, EPP_BinaryTagData );
                {
                    {
                        EscherExAtom aMagic2( *mpStrm, 0x2eeb );
                        *mpStrm << (sal_uInt32)0x01c45df9
                                << (sal_uInt32)0xe1471b30;
                    }
                    {
                        EscherExAtom aMagic( *mpStrm, 0x2b00 );
                        *mpStrm << (sal_uInt32)0;
                    }
                }
                mpStrm->Write( amsofbtAnimGroup.GetData(), amsofbtAnimGroup.Tell() );
                {
                    EscherExContainer aMagic2( *mpStrm, 0x2b02 );
                }
            }
        }
    }
*/
    mpPptEscherEx->CloseContainer();    // EPP_Slide
    return sal_True;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateNotes( sal_uInt32 nPageNum )
{
    if ( !ImplGetPageByIndex( nPageNum, NOTICE ) )
        return sal_False;
    ImplSetCurrentStyleSheet( ImplGetMasterIndex( NORMAL ) );


    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_Notes | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Notes );
    mpPptEscherEx->AddAtom( 8, EPP_NotesAtom, 1 );
    *mpStrm << (sal_uInt32)nPageNum + 0x100
            << (sal_uInt16)3                                        // follow master ....
            << (sal_uInt16)0;

    ImplCreateHeaderFooters( mXPagePropSet );

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup(0,0);

    ImplWritePage( pPHLayout[ 20 ], aSolverContainer, NOTICE, sal_False );  // Die Shapes der Seite werden im PPT Dok. erzeugt

    mpPptEscherEx->LeaveGroup();
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 ); // Flags: Connector | Background | HasSpt
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0xffffff );     // stock valued fill color
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, 0x8b9f8e );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, 0x68bdde );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->CloseContainer();    // EPP_Notes
    return sal_True;
};

void PPTWriter::ImplWriteBackground( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet )
{
    //************************ ******
    //** DEFAULT BACKGROUND SHAPE **
    //******************************

    sal_uInt32 nFillColor = 0xffffff;
    sal_uInt32 nFillBackColor = 0;

    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );                     // Flags: Connector | Background | HasSpt
    Point aEmptyPoint = Point();
    Rectangle aRect( aEmptyPoint, Size( 28000, 21000 ) );
    EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm, aRect );
    aPropOpt.AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );
    ::com::sun::star::drawing::FillStyle aFS( ::com::sun::star::drawing::FillStyle_NONE );
    if ( ImplGetPropertyValue( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) ) )
        mAny >>= aFS;

    switch( aFS )
    {
        case ::com::sun::star::drawing::FillStyle_GRADIENT :
        {
            aPropOpt.CreateGradientProperties( rXPropSet );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x1f001e );
            aPropOpt.GetOpt( ESCHER_Prop_fillColor, nFillColor );
            aPropOpt.GetOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
        }
        break;

        case ::com::sun::star::drawing::FillStyle_BITMAP :
            aPropOpt.CreateGraphicProperties( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapURL" ) ), sal_True );
        break;

        case ::com::sun::star::drawing::FillStyle_HATCH :
            aPropOpt.CreateGraphicProperties( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillHatch" ) ), sal_True );
        break;

        case ::com::sun::star::drawing::FillStyle_SOLID :
        {
            if ( ImplGetPropertyValue( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) ) )
            {
                nFillColor = mpPptEscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) );
                nFillBackColor = nFillColor ^ 0xffffff;
            }
        }   // PASSTHROUGH INTENDED
        case ::com::sun::star::drawing::FillStyle_NONE :
        default:
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
        break;
    }
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, nFillColor );
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, PPTtoEMU( maDestPageSize.Width ) );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, PPTtoEMU( maDestPageSize.Height ) );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_bwWhite );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
}

void PPTWriter::ImplWriteVBA( SvMemoryStream* pVBA )
{
    if ( pVBA )
    {
        pVBA->Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nLen = pVBA->Tell();
        if ( nLen > 8 )
        {
            nLen -= 8;
            mnVBAOleOfs = mpStrm->Tell();
            mpPptEscherEx->BeginAtom();
            mpStrm->Write( (sal_Int8*)pVBA->GetData() + 8, nLen );
            mpPptEscherEx->EndAtom( EPP_ExOleObjStg, 0, 1 );
        }
    }
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteOLE( sal_uInt32 nCnvrtFlags )
{
    PPTExOleObjEntry* pPtr;

    SvxMSExportOLEObjects aOleExport( nCnvrtFlags );

    for ( pPtr = (PPTExOleObjEntry*)maExOleObj.First(); pPtr;
        pPtr = (PPTExOleObjEntry*)maExOleObj.Next() )
    {
        SvMemoryStream* pStrm = NULL;
        pPtr->nOfsB = mpStrm->Tell();
        switch ( pPtr->eType )
        {
            case NORMAL_OLE_OBJECT :
            {
                SdrObject* pSdrObj = GetSdrObjectFromXShape( pPtr->xShape );
                if ( pSdrObj && pSdrObj->ISA( SdrOle2Obj ) )
                {
                    ::uno::Reference < embed::XEmbeddedObject > xObj( ( (SdrOle2Obj*) pSdrObj )->GetObjRef() );
                    if( xObj.is() )
                    {
                        SvStorageRef xTempStorage( new SvStorage( new SvMemoryStream(), sal_True ) );
                        aOleExport.ExportOLEObject( xObj, *xTempStorage );

                        //TODO/MBA: testing
                        String aPersistStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( SVEXT_PERSIST_STREAM ) ) );
                        SvMemoryStream aStream;
                        SvStorageRef xCleanStorage( new SvStorage( sal_False, aStream ) );
                        xTempStorage->CopyTo( xCleanStorage );
                        // SJ: #99809# create a dummy content stream, the dummy content is necessary for ppt, but not for
                        // doc files, so we can't share code.
                        SotStorageStreamRef xStm = xCleanStorage->OpenSotStream( aPersistStream, STREAM_STD_READWRITE );
                        *xStm   << (sal_uInt32)0        // no ClipboardId
                                << (sal_uInt32)4        // no target device
                                << (sal_uInt32)1        // aspect ratio
                                << (sal_Int32)-1        // L-Index
                                << (sal_uInt32)0        // Advanced Flags
                                << (sal_uInt32)0        // compression
                                << (sal_uInt32)0        // Size
                                << (sal_uInt32)0        //  "
                                << (sal_uInt32)0;
                        pStrm = xCleanStorage->CreateMemoryStream();
                    }
                }
            }
            break;

            case OCX_CONTROL :
            {
                if ( pPtr->xControlModel.is() )
                {
                    String aName;
                    ::com::sun::star::awt::Size aSize;
                    SvStorageRef xDest( new SvStorage( new SvMemoryStream(), sal_True ) );
                    sal_Bool bOk = SvxMSConvertOCXControls::WriteOCXStream( xDest, pPtr->xControlModel, aSize, aName );
                    if ( bOk )
                        pStrm = xDest->CreateMemoryStream();
                }
            }
        }
        if ( pStrm )
        {
            mpPptEscherEx->BeginAtom();
            pStrm->Seek( STREAM_SEEK_TO_END );
            sal_uInt32 npStrmSize = pStrm->Tell();
            *mpStrm << npStrmSize;                  // uncompressed size

#ifdef DBG_EXTRACTOLEOBJECTS
            SvFileStream aOut( String::CreateFromAscii( "D:\\OUT.OLE" ), STREAM_TRUNC | STREAM_WRITE );
            pStrm->Seek( 0 );
            aOut.Write( pStrm->GetData(), npStrmSize );
#endif

            pStrm->Seek( 0 );
            ZCodec aZCodec( 0x8000, 0x8000 );
            aZCodec.BeginCompression();
            aZCodec.Compress( *pStrm, *mpStrm );
            aZCodec.EndCompression();
            delete pStrm;
            mpPptEscherEx->EndAtom( EPP_ExOleObjStg, 0, 1 );
        }
    }
}

// ---------------------------------------------------------------------------------------------
// PersistantTable und UserEditAtom schreiben

sal_Bool PPTWriter::ImplWriteAtomEnding()
{

#define EPP_LastViewTypeNone        0
#define EPP_LastViewTypeSlideView   1
#define EPP_LastViewTypeOutlineView 2
#define EPP_LastViewTypeNotes       3


    sal_uInt32  i, nPos, nOfs, nPersistOfs = mpStrm->Tell();
    sal_uInt32  nPersistEntrys = 0;
    *mpStrm << (sal_uInt32)0 << (sal_uInt32)0 << (sal_uInt32)0;         // Record Header und ersten Eintrag ueberspringen

    // Document pesist schreiben
        nPersistEntrys++;
        *mpStrm << (sal_uInt32)0;
    // MasterPages persists schreiben
    for ( i = 0; i < mnMasterPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_MainMaster | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINMASTER_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // MainNotesMaster persist schreiben
    nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_MainNotes );
    if ( nOfs )
    {
        *mpStrm << nOfs;
        mpPptEscherEx->InsertAtPersistOffset( EPP_MAINNOTESMASTER_PERSIST_KEY, ++nPersistEntrys );
    }
    // Slide persists schreiben -> es gilt hier auch den EPP_SlidePersistAtome mit einem gueltigen wert zu beschreiben
    for ( i = 0; i < mnPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Slide | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINSLIDE_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // Notes persists schreiben
    for ( i = 0; i < mnPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Notes | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINNOTES_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // Ole persists
    PPTExOleObjEntry* pPtr;
    for ( pPtr = (PPTExOleObjEntry*)maExOleObj.First(); pPtr; pPtr = (PPTExOleObjEntry*)maExOleObj.Next() )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_ExObj );
        if ( nOfs )
        {
            nPersistEntrys++;
            *mpStrm << pPtr->nOfsB;
            sal_uInt32 nOldPos, nPersOfs = nOfs + pPtr->nOfsA + 16 + 8;     // 8 bytes atom header, +16 to the persist entry
            nOldPos = mpStrm->Tell();
            mpStrm->Seek( nPersOfs );
            *mpStrm << nPersistEntrys;
            mpStrm->Seek( nOldPos );
        }
    }
    // VB persist
    if ( mnVBAOleOfs && mpVBA )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_VBAInfoAtom );
        if ( nOfs )
        {
            nPersistEntrys++;
            sal_uInt32 n1, n2;

            mpVBA->Seek( 0 );
            *mpVBA >> n1
                   >> n2;

            *mpStrm << mnVBAOleOfs;
            sal_uInt32 nOldPos = mpStrm->Tell();
            mpStrm->Seek( nOfs );               // Fill the VBAInfoAtom with the correct index to the persisttable
            *mpStrm << nPersistEntrys
                    << n1
                    << 2;
            mpStrm->Seek( nOldPos );

        }
    }
    nPos = mpStrm->Tell();
    mpStrm->Seek( nPersistOfs );
    mpPptEscherEx->AddAtom( ( nPersistEntrys + 1 ) << 2, EPP_PersistPtrIncrementalBlock );      // Record Header eintragen
    *mpStrm << (sal_uInt32)( ( nPersistEntrys << 20 ) | 1 );
    mpStrm->Seek( nPos );

    *mpCurUserStrm << (sal_uInt32)nPos;             // offset to current edit setzen
    mpPptEscherEx->AddAtom( 28, EPP_UserEditAtom );
    *mpStrm << (sal_Int32)0x100                     // last slide ID
            << (sal_uInt32)0x03000dbc               // minor and major app version that did the save
            << (sal_uInt32)0                        // offset last save, 0 after a full save
            << nPersistOfs                      // File offset to persist pointers for this save operation
            << (sal_uInt32)1                        // Persist reference to the document persist object
            << (sal_uInt32)nPersistEntrys           // max persists written, Seed value for persist object id management
            << (sal_Int16)EPP_LastViewTypeSlideView // last view type
            << (sal_Int16)0x12;                     // padword

    return sal_True;
}

// ---------------------------------------------------------------------------------------------

PPTExCharSheet::PPTExCharSheet( int nInstance )
{
    sal_uInt16 nFontHeight = 24;

    for ( int nDepth = 0; nDepth < 5; nDepth++ )
    {
        PPTExCharLevel& rLev = maCharLevel[ nDepth ];
        switch ( nInstance )
        {
            case EPP_TEXTTYPE_Title :
            case EPP_TEXTTYPE_CenterTitle :
                nFontHeight = 44;
            break;
            case EPP_TEXTTYPE_Body :
            case EPP_TEXTTYPE_CenterBody :
            case EPP_TEXTTYPE_HalfBody :
            case EPP_TEXTTYPE_QuarterBody :
            {
                switch ( nDepth )
                {
                    case 0 : nFontHeight = 32; break;
                    case 1 : nFontHeight = 28; break;
                    case 2 : nFontHeight = 24; break;
                    default :nFontHeight = 20; break;
                }
            }
            break;
            case EPP_TEXTTYPE_Notes :
                nFontHeight = 12;
            break;
            case EPP_TEXTTYPE_notUsed :
            case EPP_TEXTTYPE_Other :
                nFontHeight = 24;
            break;
        }
        rLev.mnFlags = 0;
        rLev.mnFont = 0;
        rLev.mnAsianOrComplexFont = 0xffff;
        rLev.mnFontHeight = nFontHeight;
        rLev.mnFontColor = 0;
        rLev.mnEscapement = 0;
    }
}


void PPTExCharSheet::SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                                    FontCollection& rFontCollection, int nLevel )
{
    PortionObj  aPortionObj( rXPropSet, rFontCollection );

    PPTExCharLevel& rLev = maCharLevel[ nLevel ];

    if ( aPortionObj.meCharColor == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFontColor = aPortionObj.mnCharColor;
    if ( aPortionObj.meCharEscapement == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnEscapement = aPortionObj.mnCharEscapement;
    if ( aPortionObj.meCharHeight == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFontHeight = aPortionObj.mnCharHeight;
    if ( aPortionObj.meFontName == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFont = aPortionObj.mnFont;
    if ( aPortionObj.meAsianOrComplexFont == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnAsianOrComplexFont = aPortionObj.mnAsianOrComplexFont;
    rLev.mnFlags = aPortionObj.mnCharAttr;
}

void PPTExCharSheet::Write( SvStream& rSt, PptEscherEx*, sal_uInt16 nLev, sal_Bool, sal_Bool bSimpleText,
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rPagePropSet )
{
    const PPTExCharLevel& rLev = maCharLevel[ nLev ];

    sal_uInt32 nCharFlags = 0xefffff;
    if ( bSimpleText )
        nCharFlags = 0x7ffff;

    rSt << nCharFlags
        << rLev.mnFlags
        << rLev.mnFont;

    sal_uInt32 nFontColor = rLev.mnFontColor;
    if ( nFontColor == COL_AUTO )
    {
        sal_Bool bIsDark = sal_False;
        ::com::sun::star::uno::Any aAny;
        if ( PropValue::GetPropertyValue( aAny, rPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsBackgroundDark" ) ), sal_True ) )
            aAny >>= bIsDark;
        nFontColor = bIsDark ? 0xffffff : 0x000000;
    }
    nFontColor &= 0xffffff;
    nFontColor |= 0xfe000000;
    if ( bSimpleText )
    {
        rSt << rLev.mnFontHeight
            << nFontColor;
    }
    else
    {
        rSt << rLev.mnAsianOrComplexFont
            << (sal_uInt16)0xffff       // unbekannt
            << (sal_uInt16)0xffff       // unbekannt
            << rLev.mnFontHeight
            << nFontColor
            << rLev.mnEscapement;
    }
}

PPTExParaSheet::PPTExParaSheet( int nInstance, sal_uInt16 nDefaultTab, PPTExBulletProvider& rProv ) :
    rBuProv     ( rProv ),
    mnInstance  ( nInstance )
{
    sal_Bool bHasBullet = sal_False;

    sal_uInt16 nUpperDist = 0;
    sal_uInt16 nBulletChar = 0x2022;
    sal_uInt16 nBulletOfs = 0;
    sal_uInt16 nTextOfs = 0;

    for ( int nDepth = 0; nDepth < 5; nDepth++ )
    {
        PPTExParaLevel& rLev = maParaLevel[ nDepth ];
        switch ( nInstance )
        {
            case EPP_TEXTTYPE_Title :
            case EPP_TEXTTYPE_CenterTitle :
            break;
            case EPP_TEXTTYPE_Body :
            case EPP_TEXTTYPE_CenterBody :
            case EPP_TEXTTYPE_HalfBody :
            case EPP_TEXTTYPE_QuarterBody :
            {
                bHasBullet = sal_True;
                nUpperDist = 0x14;
            }
            break;
            case EPP_TEXTTYPE_Notes :
                nUpperDist = 0x1e;
            break;

//          default :
//          case EPP_TEXTTYPE_notUsed :
//          case EPP_TEXTTYPE_Other :
//          break;
        }
        switch ( nDepth )
        {
            case 0 :
            {
                nBulletChar = 0x2022;
                nBulletOfs = 0;
                nTextOfs = ( bHasBullet ) ? 0xd8 : 0;
            }
            break;
            case 1 :
            {
                nBulletChar = 0x2013;
                nBulletOfs = 0x120;
                nTextOfs = 0x1d4;
            }
            break;
            case 2 :
            {
                nBulletChar = 0x2022;
                nBulletOfs = 0x240;
                nTextOfs = 0x2d0;
            }
            break;
            case 3 :
            {
                nBulletChar = 0x2013;
                nBulletOfs = 0x360;
                nTextOfs = 0x3f0;
            }
            break;
            case 4 :
            {
                nBulletChar = 0xbb;
                nBulletOfs = 0x480;
                nTextOfs = 0x510;
            }
            break;
        }
        rLev.mbIsBullet = bHasBullet;
        rLev.mnBulletChar = nBulletChar;
        rLev.mnBulletFont = 0;
        rLev.mnBulletHeight = 100;
        rLev.mnBulletColor = 0;
        rLev.mnAdjust = 0;
        rLev.mnLineFeed = 100;
        rLev.mnLowerDist = 0;
        rLev.mnUpperDist = nUpperDist;
        rLev.mnTextOfs = nTextOfs;
        rLev.mnBulletOfs = nBulletOfs;
        rLev.mnDefaultTab = nDefaultTab;
        rLev.mnAsianSettings = 2;
        rLev.mnBiDi = 0;

        rLev.mbExtendedBulletsUsed = sal_False;
        rLev.mnBulletId = 0xffff;
        rLev.mnBulletStart = 0;
        rLev.mnMappedNumType = 0;
        rLev.mnNumberingType = 0;
    }
}

void PPTExParaSheet::SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                                        FontCollection& rFontCollection, int nLevel, const PPTExCharLevel& rCharLevel )
{
    ParagraphObj aParagraphObj( rXPropSet, rBuProv );
    aParagraphObj.CalculateGraphicBulletSize( rCharLevel.mnFontHeight );
    PPTExParaLevel& rLev = maParaLevel[ nLevel ];

    if ( aParagraphObj.meTextAdjust == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnAdjust = aParagraphObj.mnTextAdjust;
    if ( aParagraphObj.meLineSpacing == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
    {
        sal_Int16 nLineSpacing = aParagraphObj.mnLineSpacing;
        if ( nLineSpacing > 0 ) // if nLinespacing is < 0 the linespacing is an absolute spacing
        {
            sal_Bool bFixedLineSpacing = sal_False;
            uno::Any aAny = rXPropSet->getPropertyValue( ::rtl::OUString(
                                                             RTL_CONSTASCII_USTRINGPARAM(
                                                                 "FontIndependentLineSpacing" ) ) );
            if( !(aAny >>= bFixedLineSpacing) || !bFixedLineSpacing )
            {
                const FontCollectionEntry* pDesc = rFontCollection.GetById( rCharLevel.mnFont );
                if ( pDesc )
                    nLineSpacing = (sal_Int16)( (double)nLineSpacing * pDesc->Scaling + 0.5 );
            }
        }
        else
        {
            if ( rCharLevel.mnFontHeight > (sal_uInt16)( ((double)-nLineSpacing) * 0.001 * 72.0 / 2.54 ) ) // 1/100mm to point
            {
                const FontCollectionEntry* pDesc = rFontCollection.GetById( rCharLevel.mnFont );
                if ( pDesc )
                     nLineSpacing = (sal_Int16)( (double)100.0 * pDesc->Scaling + 0.5 );
                else
                    nLineSpacing = 100;
            }
            else
                nLineSpacing = (sal_Int16)( (double)nLineSpacing / 4.40972 );
        }
        rLev.mnLineFeed = nLineSpacing;
    }
    if ( aParagraphObj.meLineSpacingBottom == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnLowerDist = aParagraphObj.mnLineSpacingBottom;
    if ( aParagraphObj.meLineSpacingTop == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnUpperDist = aParagraphObj.mnLineSpacingTop;
    if ( aParagraphObj.meForbiddenRules == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
    {
        rLev.mnAsianSettings &=~1;
        if ( aParagraphObj.mbForbiddenRules )
            rLev.mnAsianSettings |= 1;
    }
    if ( aParagraphObj.meParagraphPunctation == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
    {
        rLev.mnAsianSettings &=~4;
        if ( aParagraphObj.mbParagraphPunctation )
            rLev.mnAsianSettings |= 4;
    }

    if ( aParagraphObj.meBiDi == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnBiDi = aParagraphObj.mnBiDi;

    rLev.mbIsBullet = aParagraphObj.mbIsBullet; //( ( aParagraphObj.nBulletFlags & 1 ) != 0 );

    if ( !nLevel )
    {
        if ( ( aParagraphObj.meBullet ==  ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
                    && aParagraphObj.bExtendedParameters )
        {
            for ( sal_Int16 i = 0; i < 5; i++ )
            {
                PPTExParaLevel& rLevel = maParaLevel[ i ];
                if ( i )
                    aParagraphObj.ImplGetNumberingLevel( rBuProv, i, sal_False );
//              rLevel.mbIsBullet = ( ( aParagraphObj.nBulletFlags & 1 ) != 0 );
                rLevel.mnTextOfs = aParagraphObj.nTextOfs;
                rLevel.mnBulletOfs = (sal_uInt16)aParagraphObj.nBulletOfs;
                rLevel.mnBulletChar = aParagraphObj.cBulletId;
                FontCollectionEntry aFontDescEntry( aParagraphObj.aFontDesc.Name, aParagraphObj.aFontDesc.Family,
                                                        aParagraphObj.aFontDesc.Pitch, aParagraphObj.aFontDesc.CharSet );
                rLevel.mnBulletFont = (sal_uInt16)rFontCollection.GetId( aFontDescEntry );
                rLevel.mnBulletHeight = aParagraphObj.nBulletRealSize;
                rLevel.mnBulletColor = aParagraphObj.nBulletColor;

                rLevel.mbExtendedBulletsUsed = aParagraphObj.bExtendedBulletsUsed;
                rLevel.mnBulletId = aParagraphObj.nBulletId;
                rLevel.mnNumberingType = aParagraphObj.nNumberingType;
                rLevel.mnBulletStart = aParagraphObj.nStartWith;
                rLevel.mnMappedNumType = aParagraphObj.nMappedNumType;
            }
        }
    }
}

void PPTExParaSheet::Write( SvStream& rSt, PptEscherEx*, sal_uInt16 nLev, sal_Bool, sal_Bool bSimpleText,
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rPagePropSet )
{
    const PPTExParaLevel& rLev = maParaLevel[ nLev ];

    if ( maParaLevel[ 0 ].mbExtendedBulletsUsed || maParaLevel[ 1 ].mbExtendedBulletsUsed ||
            maParaLevel[ 2 ].mbExtendedBulletsUsed || maParaLevel[ 3 ].mbExtendedBulletsUsed ||
                maParaLevel[ 4 ].mbExtendedBulletsUsed )
    {
        SvStream& rOut = rBuProv.aBuExMasterStream;
        if ( !nLev )
        {
            rOut << (sal_uInt32)( ( EPP_PST_ExtendedParagraphMasterAtom << 16 ) | ( mnInstance << 4 ) )
                 << (sal_uInt32)( 5 * 16 + 2 )
                 << (sal_uInt16)5;              // depth
        }
        sal_uInt16 nBulletId = rLev.mnBulletId;
        if ( rLev.mnNumberingType != SVX_NUM_BITMAP )
            nBulletId = 0xffff;
        rOut << (sal_uInt32)0x03800000
             << (sal_uInt16)nBulletId
             << (sal_uInt32)rLev.mnMappedNumType
             << (sal_uInt16)rLev.mnBulletStart
             << (sal_uInt32)0;
    }

    sal_uInt32 nParaFlags = 0x3ffdff;
    sal_uInt16 nBulletFlags = ( rLev.mbIsBullet ) ? 0xf : 0xe;

    if ( nLev )
        nParaFlags &= 0x207fff;
    if ( bSimpleText )
        nParaFlags &= 0x7fff;
    sal_uInt32 nBulletColor = rLev.mnBulletColor;
    if ( nBulletColor == COL_AUTO )
    {
        sal_Bool bIsDark = sal_False;
        ::com::sun::star::uno::Any aAny;
        if ( PropValue::GetPropertyValue( aAny, rPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsBackgroundDark" ) ), sal_True ) )
            aAny >>= bIsDark;
        nBulletColor = bIsDark ? 0xffffff : 0x000000;
    }
    nBulletColor &= 0xffffff;
    nBulletColor |= 0xfe000000;
    rSt << nParaFlags
        << nBulletFlags
        << rLev.mnBulletChar
        << rLev.mnBulletFont
        << rLev.mnBulletHeight
        << nBulletColor
        << rLev.mnAdjust
        << rLev.mnLineFeed
        << rLev.mnUpperDist
        << rLev.mnLowerDist
        << rLev.mnTextOfs
        << rLev.mnBulletOfs;

    if ( bSimpleText || nLev )
    {
        if ( nParaFlags & 0x200000 )
            rSt << rLev.mnBiDi;
    }
    else
    {
        rSt << rLev.mnDefaultTab
            << (sal_uInt16)0
            << (sal_uInt16)0
            << rLev.mnAsianSettings
            << rLev.mnBiDi;
    }
}


PPTExStyleSheet::PPTExStyleSheet( sal_uInt16 nDefaultTab, PPTExBulletProvider& rBuProv )
{
    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        mpParaSheet[ nInstance ] = ( nInstance == EPP_TEXTTYPE_notUsed ) ? NULL : new PPTExParaSheet( nInstance, nDefaultTab, rBuProv );
        mpCharSheet[ nInstance ] = ( nInstance == EPP_TEXTTYPE_notUsed ) ? NULL : new PPTExCharSheet( nInstance );
    }
}

PPTExStyleSheet::~PPTExStyleSheet()
{
    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        if ( nInstance == EPP_TEXTTYPE_notUsed )
            continue;

        delete mpParaSheet[ nInstance ];
        delete mpCharSheet[ nInstance ];
    }
}

void PPTExStyleSheet::SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                                        FontCollection& rFontCollection, int nInstance, int nLevel )
{
    if ( nInstance == EPP_TEXTTYPE_notUsed )
        return;
    mpCharSheet[ nInstance ]->SetStyleSheet( rXPropSet, rFontCollection, nLevel );
    mpParaSheet[ nInstance ]->SetStyleSheet( rXPropSet, rFontCollection, nLevel, mpCharSheet[ nInstance ]->maCharLevel[ nLevel ] );
}

sal_Bool PPTExStyleSheet::IsHardAttribute( sal_uInt32 nInstance, sal_uInt32 nLevel, PPTExTextAttr eAttr, sal_uInt32 nValue )
{
    const PPTExParaLevel& rPara = mpParaSheet[ nInstance ]->maParaLevel[ nLevel ];
    const PPTExCharLevel& rChar = mpCharSheet[ nInstance ]->maCharLevel[ nLevel ];

    sal_uInt32 nFlag = 0;

    switch ( eAttr )
    {
        case ParaAttr_BulletOn : return ( rPara.mbIsBullet ) ? ( nValue ) ? sal_False : sal_True : ( nValue ) ? sal_True : sal_False;
        case ParaAttr_BuHardFont :
        case ParaAttr_BulletFont : return ( rPara.mnBulletFont != nValue );
        case ParaAttr_BuHardColor :
        case ParaAttr_BulletColor : return ( rPara.mnBulletColor != nValue );
        case ParaAttr_BuHardHeight :
        case ParaAttr_BulletHeight : return ( rPara.mnBulletHeight != nValue );
        case ParaAttr_BulletChar : return ( rPara.mnBulletChar != nValue );
        case ParaAttr_Adjust : return ( rPara.mnAdjust != nValue );
        case ParaAttr_LineFeed : return ( rPara.mnLineFeed != nValue );
        case ParaAttr_UpperDist : return ( rPara.mnUpperDist != nValue );
        case ParaAttr_LowerDist : return ( rPara.mnLowerDist != nValue );
        case ParaAttr_TextOfs : return ( rPara.mnTextOfs != nValue );
        case ParaAttr_BulletOfs : return ( rPara.mnBulletOfs != nValue );
        case ParaAttr_DefaultTab : return ( rPara.mnDefaultTab != nValue );
        case ParaAttr_BiDi : return ( rPara.mnBiDi != nValue );
        case CharAttr_Bold : nFlag = 1; break;
        case CharAttr_Italic : nFlag = 2; break;
        case CharAttr_Underline : nFlag = 4; break;
        case CharAttr_Shadow : nFlag = 16; break;
        case CharAttr_Strikeout : nFlag = 256; break;
        case CharAttr_Embossed : nFlag = 512; break;
        case CharAttr_Font : return ( rChar.mnFont != nValue );
        case CharAttr_AsianOrComplexFont : return ( rChar.mnAsianOrComplexFont != nValue );
        case CharAttr_Symbol : return sal_True;
        case CharAttr_FontHeight : return ( rChar.mnFontHeight != nValue );
        case CharAttr_FontColor : return ( rChar.mnFontColor != nValue );
        case CharAttr_Escapement : return ( rChar.mnEscapement != nValue );
        default:
            break;
    };
    if ( nFlag )
    {
        if ( rChar.mnFlags & nFlag )
            return ( ( nValue & nFlag ) == 0 );
        else
            return ( ( nValue & nFlag ) != 0 );
    }
    return sal_True;
}

sal_uInt32 PPTExStyleSheet::SizeOfTxCFStyleAtom() const
{
    return 24;
}

// the TxCFStyleAtom stores the text properties that are used
// when creating new objects in PowerPoint.

void PPTExStyleSheet::WriteTxCFStyleAtom( SvStream& rSt )
{
    const PPTExCharLevel& rCharStyle = mpCharSheet[ EPP_TEXTTYPE_Other ]->maCharLevel[ 0 ];

    sal_uInt16 nFlags = 0x60        // ??
                      | 0x02        // fontsize;
                      | 0x04;       // fontcolor

    sal_uInt32 nCharFlags = rCharStyle.mnFlags;
    nCharFlags &= CharAttr_Italic | CharAttr_Bold | CharAttr_Underline | CharAttr_Shadow;

    rSt << (sal_uInt32)( EPP_TxCFStyleAtom << 16 )  // recordheader
        << SizeOfTxCFStyleAtom() - 8
        << (sal_uInt16)( 0x80 | nCharFlags )
        << (sal_uInt16)nFlags
        << (sal_uInt16)nCharFlags
        << (sal_Int32)-1                            // ?
        << rCharStyle.mnFontHeight
        << rCharStyle.mnFontColor;
}


// ---------------------------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI ExportPPT( SvStorageRef& rSvStorage,
                    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rXModel,
                        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > & rXStatInd,
                            SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags )
{
    PPTWriter*  pPPTWriter;
    sal_Bool bStatus = sal_False;

    pPPTWriter = new PPTWriter( rSvStorage, rXModel, rXStatInd, pVBA, nCnvrtFlags );
    if ( pPPTWriter )
    {
        bStatus = ( pPPTWriter->IsValid() == sal_True );
        delete pPPTWriter;
    }

    return bStatus;
}

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI SaveVBA( SfxObjectShell& rDocShell, SvMemoryStream*& pBas )
{
    SvStorageRef xDest( new SvStorage( new SvMemoryStream(), sal_True ) );
    SvxImportMSVBasic aMSVBas( rDocShell, *xDest, sal_False, sal_False );
    aMSVBas.SaveOrDelMSVBAStorage( sal_True, String( RTL_CONSTASCII_USTRINGPARAM("_MS_VBA_Overhead") ) );

    SvStorageRef xOverhead = xDest->OpenSotStorage( String( RTL_CONSTASCII_USTRINGPARAM("_MS_VBA_Overhead") ) );
    if ( xOverhead.Is() && ( xOverhead->GetError() == SVSTREAM_OK ) )
    {
        SvStorageRef xOverhead2 = xOverhead->OpenSotStorage( String( RTL_CONSTASCII_USTRINGPARAM("_MS_VBA_Overhead") ) );
        if ( xOverhead2.Is() && ( xOverhead2->GetError() == SVSTREAM_OK ) )
        {
            SvStorageStreamRef xTemp = xOverhead2->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM("_MS_VBA_Overhead2") ) );
            if ( xTemp.Is() && ( xTemp->GetError() == SVSTREAM_OK ) )
            {
                sal_uInt32 nLen = xTemp->GetSize();
                if ( nLen )
                {
                    char* pTemp = new char[ nLen ];
                    if ( pTemp )
                    {
                        xTemp->Seek( STREAM_SEEK_TO_BEGIN );
                        xTemp->Read( pTemp, nLen );
                        pBas = new SvMemoryStream( pTemp, nLen, STREAM_READ );
                        pBas->ObjectOwnsMemory( sal_True );
                        return sal_True;
                    }
                }
            }
        }
    }

    return sal_False;
}

