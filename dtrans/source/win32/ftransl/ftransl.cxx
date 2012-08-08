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

#include <osl/diagnose.h>

#include "ftransl.hxx"
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include "../misc/ImplHelper.hxx"

#if defined _MSC_VER
#pragma warning(push,1)
#pragma warning(disable:4917)
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define IMPL_NAME  "com.sun.star.datatransfer.DataFormatTranslator"

#define MODULE_PRIVATE
#define CPPUTYPE_SEQSALINT8       getCppuType( (const Sequence< sal_Int8 >*) 0 )
#define CPPUTYPE_DEFAULT          CPPUTYPE_SEQSALINT8
#define CPPUTYPE_OUSTR            getCppuType( (const ::rtl::OUString*) 0 )
#define CPPUTYPE_SALINT32         getCppuType( ( sal_Int32 * ) 0 )
#define OUSTR( str )              OUString( #str )
#define EMPTY_OUSTR               OUString()

const rtl::OUString Windows_FormatName ("windows_formatname");
const com::sun::star::uno::Type CppuType_ByteSequence = ::getCppuType((const com::sun::star::uno::Sequence<sal_Int8>*)0);
const com::sun::star::uno::Type CppuType_String       = ::getCppuType((const ::rtl::OUString*)0);

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using ::rtl::OUString;
using namespace osl;
using namespace cppu;
using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::container;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace MODULE_PRIVATE
{
    Sequence< OUString > SAL_CALL DataFormatTranslator_getSupportedServiceNames( )
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString("com.sun.star.datatransfer.DataFormatTranslator");
        return aRet;
    }
}

//------------------------------------------------
//
//------------------------------------------------

FormatEntry::FormatEntry()
{
}

//------------------------------------------------
//
//------------------------------------------------

FormatEntry::FormatEntry(
    const char* mime_content_type,
    const char* human_presentable_name,
    const char* native_format_name,
    CLIPFORMAT std_clipboard_format_id,
    ::com::sun::star::uno::Type const & cppu_type)
{
    aDataFlavor.MimeType             = rtl::OUString::createFromAscii(mime_content_type);
    aDataFlavor.HumanPresentableName = rtl::OUString::createFromAscii(human_presentable_name);
    aDataFlavor.DataType             = cppu_type;

    if (native_format_name)
        aNativeFormatName = rtl::OUString::createFromAscii(native_format_name);
    else
        aNativeFormatName = rtl::OUString::createFromAscii(human_presentable_name);

    aStandardFormatId = std_clipboard_format_id;
}

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CDataFormatTranslator::CDataFormatTranslator( const Reference< XMultiServiceFactory >& rSrvMgr ) :
    m_SrvMgr( rSrvMgr )
{
    initTranslationTable( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

Any SAL_CALL CDataFormatTranslator::getSystemDataTypeFromDataFlavor( const DataFlavor& aDataFlavor )
    throw( RuntimeException )
{
    Any aAny;

    try
    {
        Reference< XMimeContentTypeFactory > refXMimeCntFactory( m_SrvMgr->createInstance(
            OUSTR( com.sun.star.datatransfer.MimeContentTypeFactory ) ), UNO_QUERY );

        if ( !refXMimeCntFactory.is( ) )
            throw RuntimeException( );

        Reference< XMimeContentType >
            refXMimeCntType( refXMimeCntFactory->createMimeContentType( aDataFlavor.MimeType ) );

        OUString fullMediaType = refXMimeCntType->getFullMediaType( );
        if ( isTextPlainMediaType( fullMediaType ) )
        {
            // default is CF_TEXT
            aAny <<= static_cast< sal_Int32 >( CF_TEXT );

            if ( refXMimeCntType->hasParameter( OUSTR( charset ) ) )
            {
                // but maybe it is unicode text or oem text
                OUString charset = refXMimeCntType->getParameterValue( OUSTR( charset ) );
                findStandardFormatIdForCharset( charset, aAny );
            }
        }
        else
        {
            if (refXMimeCntType->hasParameter(Windows_FormatName))
            {
                OUString winfmtname = refXMimeCntType->getParameterValue(Windows_FormatName);
                aAny <<= winfmtname;

                setStandardFormatIdForNativeFormatName( winfmtname, aAny );
            }
            else
                findStdFormatIdOrNativeFormatNameForFullMediaType( refXMimeCntFactory, fullMediaType, aAny );
        }
    }
    catch( IllegalArgumentException& )
    {
        OSL_FAIL( "Invalid content-type detected!" );
    }
    catch( NoSuchElementException& )
    {
        OSL_FAIL( "Illegal content-type parameter" );
    }
    catch( ... )
    {
        OSL_FAIL( "Unexpected error" );
        throw;
    }

    return aAny;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DataFlavor SAL_CALL CDataFormatTranslator::getDataFlavorFromSystemDataType( const Any& aSysDataType )
    throw( RuntimeException )
{
    OSL_PRECOND( aSysDataType.hasValue( ), "Empty system data type delivered" );

    DataFlavor aFlavor = mkDataFlv( EMPTY_OUSTR, EMPTY_OUSTR, CPPUTYPE_SEQSALINT8 );

    if ( aSysDataType.getValueType( ) == CPPUTYPE_SALINT32 )
    {
        sal_Int32 clipformat = CF_INVALID;
        aSysDataType >>= clipformat;
        if ( CF_INVALID != clipformat )
            findDataFlavorForStandardFormatId( clipformat, aFlavor );
    }
    else if ( aSysDataType.getValueType( ) == CPPUTYPE_OUSTR )
    {
        OUString nativeFormatName;
        aSysDataType >>= nativeFormatName;

        findDataFlavorForNativeFormatName( nativeFormatName, aFlavor );
    }
    else
        OSL_FAIL( "Invalid data type received" );

    return aFlavor;
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CDataFormatTranslator::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString( IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CDataFormatTranslator::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = DataFormatTranslator_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CDataFormatTranslator::getSupportedServiceNames( )
    throw( RuntimeException )
{
    return DataFormatTranslator_getSupportedServiceNames( );
}

// -------------------------------------------------
// to optimize searching we add all entries with a
// standard clipboard format number first, in the
// table before the entries with CF_INVALID
// if we are searching for a standard clipboard
// format number we can stop if we find the first
// CF_INVALID
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::initTranslationTable()
{
    //SOT_FORMATSTR_ID_DIF
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-dif;windows_formatname=\"DIF\"", "DIF", "DIF", CF_DIF, CPPUTYPE_DEFAULT));
    // SOT_FORMAT_BITMAP
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", "Bitmap", "Bitmap", CF_DIB, CPPUTYPE_DEFAULT));
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", "Bitmap", "Bitmap", CF_BITMAP, CPPUTYPE_DEFAULT));
    // SOT_FORMAT_STRING
    m_TranslTable.push_back(FormatEntry("text/plain;charset=utf-16", "Unicode-Text", "", CF_UNICODETEXT, CppuType_String));
    // Format Locale - for internal use
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-locale;windows_formatname=\"Locale\"", "Locale", "Locale", CF_LOCALE, CPPUTYPE_DEFAULT));
    // SOT_FORMAT_WMF
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-wmf;windows_formatname=\"Image WMF\"", "Windows MetaFile", "Image WMF", CF_METAFILEPICT, CPPUTYPE_DEFAULT));
    // SOT_FORMAT_EMF
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-emf;windows_formatname=\"Image EMF\"", "Windows Enhanced MetaFile", "Image EMF", CF_ENHMETAFILE, CPPUTYPE_DEFAULT));
    // SOT_FORMAT_FILE_LIST
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-filelist;windows_formatname=\"FileList\"", "FileList", "FileList", CF_HDROP, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SYLK
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sylk;windows_formatname=\"Sylk\"", "Sylk", "Sylk", CF_SYLK, CPPUTYPE_DEFAULT ) );
    // SOT_FORMAT_GDIMETAFILE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"", "GDIMetaFile", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMAT_PRIVATE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-private;windows_formatname=\"Private\"", "Private", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMAT_FILE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-file;windows_formatname=\"FileName\"", "FileName", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMAT_RTF
    m_TranslTable.push_back(FormatEntry("text/richtext", "Rich Text Format", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_DRAWING
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-drawing;windows_formatname=\"Drawing Format\"", "Drawing Format", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_SVXB
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-svbx;windows_formatname=\"SVXB (StarView Bitmap/Animation)\"", "SVXB (StarView Bitmap/Animation)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_SVIM
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-svim;windows_formatname=\"SVIM (StarView ImageMap)\"", "SVIM (StarView ImageMap)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_XFA
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-xfa;windows_formatname=\"XFA (XOutDev FillAttr)\"", "XFA (XOutDev FillAttr)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_EDITENGINE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-editengine;windows_formatname=\"EditEngineFormat\"", "EditEngineFormat", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_INTERNALLINK_STATE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-internallink-state;windows_formatname=\"StatusInfo vom SvxInternalLink\"", "StatusInfo vom SvxInternalLink", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_SOLK
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-solk;windows_formatname=\"SOLK (StarOffice Link)\"", "SOLK (StarOffice Link)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-netscape-bookmark;windows_formatname=\"Netscape Bookmark\"", "Netscape Bookmark", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_TREELISTBOX
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-treelistbox;windows_formatname=\"SV_LBOX_DD_FORMAT\"", "SV_LBOX_DD_FORMAT", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_NATIVE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-native;windows_formatname=\"Native\"", "Native", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_OWNERLINK
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-ownerlink;windows_formatname=\"OwnerLink\"", "OwnerLink", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STARSERVER
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starserver;windows_formatname=\"StarServerFormat\"", "StarServerFormat", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STAROBJECT
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starobject;windows_formatname=\"StarObjectFormat\"", "StarObjectFormat", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_APPLETOBJECT
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-appletobject;windows_formatname=\"Applet Object\"", "Applet Object", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_PLUGIN_OBJECT
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-plugin-object;windows_formatname=\"PlugIn Object\"", "PlugIn Object", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STARWRITER_30
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starwriter-30;windows_formatname=\"StarWriter 3.0\"", "StarWriter 3.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWRITER_40
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starwriter-40;windows_formatname=\"StarWriter 4.0\"", "StarWriter 4.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWRITER_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starwriter-50;windows_formatname=\"StarWriter 5.0\"", "StarWriter 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWRITERWEB_40
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starwriterweb-40;windows_formatname=\"StarWriter/Web 4.0\"", "StarWriter/Web 4.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWRITERWEB_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starwriterweb-50;windows_formatname=\"StarWriter/Web 5.0\"", "StarWriter/Web 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWRITERGLOB_40
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starwriterglob-40;windows_formatname=\"StarWriter/Global 4.0\"", "StarWriter/Global 4.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STARWRITERGLOB_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starwriterglob-50;windows_formatname=\"StarWriter/Global 5.0\"", "StarWriter/Global 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARDRAW
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-stardraw;windows_formatname=\"StarDrawDocument\"", "StarDrawDocument", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARDRAW_40
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-stardraw-40;windows_formatname=\"StarDrawDocument 4.0\"", "StarDrawDocument 4.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARIMPRESS_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starimpress-50;windows_formatname=\"StarImpress 5.0\"", "StarImpress 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STARDRAW_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-stardraw-50;windows_formatname=\"StarDraw 5.0\"", "StarDraw 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARCALC
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starcalc;windows_formatname=\"StarCalcDocument\"", "StarCalcDocument", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARCALC_40
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starcalc-40;windows_formatname=\"StarCalc 4.0\"", "StarCalc 4.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STARCALC_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starcalc-50;windows_formatname=\"StarCalc 5.0\"", "StarCalc 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STARCHART
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starchart;windows_formatname=\"StarChartDocument\"", "StarChartDocument", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STARCHART_40
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starchart-40;windows_formatname=\"StarChartDocument 4.0\"", "StarChartDocument 4.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    // SOT_FORMATSTR_ID_STARCHART_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starchart-50;windows_formatname=\"StarChart 5.0\"", "StarChart 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARIMAGE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starimage;windows_formatname=\"StarImageDocument\"", "StarImageDocument", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARIMAGE_40
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starimage-40;windows_formatname=\"StarImageDocument 4.0\"", "StarImageDocument 4.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARIMAGE_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starimage-50;windows_formatname=\"StarImage 5.0\"",  "StarImage 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARMATH
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starmath;windows_formatname=\"StarMath\"", "StarMath", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARMATH_40
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starmath-40;windows_formatname=\"StarMathDocument 4.0\"", "StarMathDocument 4.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARMATH_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starmath-50;windows_formatname=\"StarMath 5.0\"", "StarMath 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starobject-paintdoc;windows_formatname=\"StarObjectPaintDocument\"", "StarObjectPaintDocument", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_FILLED_AREA
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-filled-area;windows_formatname=\"FilledArea\"", "FilledArea", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_HTML
    m_TranslTable.push_back(FormatEntry("text/html", "HTML (HyperText Markup Language)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_HTML_SIMPLE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-html-simple;windows_formatname=\"HTML Format\"", "HTML Format", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_CHAOS
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-chaos;windows_formatname=\"FORMAT_CHAOS\"", "FORMAT_CHAOS", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_CNT_MSGATTACHFILE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-msgattachfile;windows_formatname=\"CNT_MSGATTACHFILE_FORMAT\"", "CNT_MSGATTACHFILE_FORMAT", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_BIFF_5
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-biff5;windows_formatname=\"Biff5\"", "Biff5", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_BIFF__5
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-biff-5;windows_formatname=\"Biff 5\"", "Biff 5", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_BIFF_8
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-biff-8;windows_formatname=\"Biff8\"", "Biff8", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SYLK_BIGCAPS
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sylk-bigcaps;windows_formatname=\"SYLK\"", "SYLK", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_LINK
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-link;windows_formatname=\"Link\"", "Link", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARDRAW_TABBAR
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-stardraw-tabbar;windows_formatname=\"StarDraw TabBar\"", "StarDraw TabBar", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SONLK
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sonlk;windows_formatname=\"SONLK (StarOffice Navi Link)\"", "SONLK (StarOffice Navi Link)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_MSWORD_DOC
    m_TranslTable.push_back(FormatEntry("application/msword", "MSWordDoc", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STAR_FRAMESET_DOC
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-star-frameset-doc;windows_formatname=\"StarFrameSetDocument\"", "StarFrameSetDocument", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_OFFICE_DOC
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-office-doc;windows_formatname=\"OfficeDocument\"", "OfficeDocument", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_NOTES_DOCINFO
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-notes-docinfo;windows_formatname=\"NotesDocInfo\"", "NotesDocInfo", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_NOTES_HNOTE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-notes-hnote;windows_formatname=\"NoteshNote\"", "NoteshNote", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_NOTES_NATIVE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-notes-native;windows_formatname=\"Native\"", "Native", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SFX_DOC
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sfx-doc;windows_formatname=\"SfxDocument\"", "SfxDocument", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_EVDF
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-evdf;windows_formatname=\"EVDF (Explorer View Dummy Format)\"", "EVDF (Explorer View Dummy Format)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_ESDF
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-esdf;windows_formatname=\"ESDF (Explorer Search Dummy Format)\"", "ESDF (Explorer Search Dummy Format)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_IDF
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-idf;windows_formatname=\"IDF (Iconview Dummy Format)\"", "IDF (Iconview Dummy Format)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_EFTP
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-eftp;windows_formatname=\"EFTP (Explorer Ftp File)\"", "EFTP (Explorer Ftp File)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_EFD
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-efd;windows_formatname=\"EFD (Explorer Ftp Dir)\"", "EFD (Explorer Ftp Dir)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-svx-formfieldexch;windows_formatname=\"SvxFormFieldExch\"", "SvxFormFieldExch", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_EXTENDED_TABBAR
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-extended-tabbar;windows_formatname=\"ExtendedTabBar\"", "ExtendedTabBar", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SBA_DATAEXCHANGE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-dataexchange;windows_formatname=\"SBA-DATAFORMAT\"", "SBA-DATAFORMAT", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-fielddataexchange;windows_formatname=\"SBA-FIELDFORMAT\"", "SBA-FIELDFORMAT", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SBA_PRIVATE_URL
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-private-url;windows_formatname=\"SBA-PRIVATEURLFORMAT\"", "SBA-PRIVATEURLFORMAT", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SBA_TABED
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-tabed;windows_formatname=\"Tabed\"", "Tabed", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SBA_TABID
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-tabid;windows_formatname=\"Tabid\"", "Tabid", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SBA_JOIN
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-join;windows_formatname=\"SBA-JOINFORMAT\"", "SBA-JOINFORMAT", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_OBJECTDESCRIPTOR
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"", "Star Object Descriptor (XML)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\"", "Star Link Source Descriptor (XML)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_EMBED_SOURCE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"", "Star Embed Source (XML)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_LINK_SOURCE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\"", "Star Link Source (XML)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_EMBEDDED_OBJ
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\"", "Star Embedded Object (XML)", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_FILECONTENT
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-filecontent;windows_formatname=\"" CFSTR_FILECONTENTS "\"", CFSTR_FILECONTENTS, NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-filegrpdescriptor;windows_formatname=\"" CFSTR_FILEDESCRIPTOR "\"", CFSTR_FILEDESCRIPTOR, NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_FILENAME
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-filename;windows_formatname=\"" CFSTR_FILENAME "\"", CFSTR_FILENAME, NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SD_OLE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sd-ole;windows_formatname=\"SD-OLE\"", "SD-OLE", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-embedded-obj-ole;windows_formatname=\"Embedded Object\"", "Embedded Object", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_EMBED_SOURCE_OLE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-embed-source-ole;windows_formatname=\"Embed Source\"", "Embed Source", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-objectdescriptor-ole;windows_formatname=\"Object Descriptor\"", "Object Descriptor", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR_OLE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-linkdescriptor-ole;windows_formatname=\"Link Source Descriptor\"", "Link Source Descriptor", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_LINK_SOURCE_OLE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-link-source-ole;windows_formatname=\"Link Source\"", "Link Source", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-ctrldataexchange;windows_formatname=\"SBA-CTRLFORMAT\"", "SBA-CTRLFORMAT", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_OUTPLACE_OBJ
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-outplace-obj;windows_formatname=\"OutPlace Object\"", "OutPlace Object", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_CNT_OWN_CLIP
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-cnt-own-clip;windows_formatname=\"CntOwnClipboard\"", "CntOwnClipboard", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_INET_IMAGE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-inet-image;windows_formatname=\"SO-INet-Image\"", "SO-INet-Image", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_NETSCAPE_IMAGE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-netscape-image;windows_formatname=\"Netscape Image Format\"", "Netscape Image Format", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_SBA_FORMEXCHANGE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-formexchange;windows_formatname=\"SBA_FORMEXCHANGE\"", "SBA_FORMEXCHANGE", NULL, CF_INVALID, CPPUTYPE_DEFAULT));  //SOT_FORMATSTR_ID_SBA_REPORTEXCHANGE
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-sba-reportexchange;windows_formatname=\"SBA_REPORTEXCHANGE\"", "SBA_REPORTEXCHANGE", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-uniformresourcelocator;windows_formatname=\"UniformResourceLocator\"", "UniformResourceLocator", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-starchartdocument-50;windows_formatname=\"StarChartDocument 5.0\"", "StarChartDocument 5.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_GRAPHOBJ
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-graphobj;windows_formatname=\"Graphic Object\"", "Graphic Object", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWRITER_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.writer", "Writer 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWRITERWEB_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.writer.web", "Writer/Web 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWRITERGLOB_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.writer.global", "Writer/Global 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARWDRAW_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.draw", "Draw 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARIMPRESS_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.impress", "Impress 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARCALC_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.calc", "Calc 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARCHART_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.chart", "Chart 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_STARMATH_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.math", "Math 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_DIALOG_60
    m_TranslTable.push_back(FormatEntry("application/vnd.sun.xml.dialog", "Dialog 6.0", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_BMP
    m_TranslTable.push_back(FormatEntry("image/bmp", "Windows Bitmap", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_DUMMY3
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-dummy3;windows_formatname=\"SO_DUMMYFORMAT_3\"", "SO_DUMMYFORMAT_3", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
    //SOT_FORMATSTR_ID_DUMMY4
    m_TranslTable.push_back(FormatEntry("application/x-openoffice-dummy4;windows_formatname=\"SO_DUMMYFORMAT_4\"", "SO_DUMMYFORMAT_4", NULL, CF_INVALID, CPPUTYPE_DEFAULT));
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::findDataFlavorForStandardFormatId( sal_Int32 aStandardFormatId, DataFlavor& aDataFlavor ) const
{
    /*
        we break the for loop if we find the first CF_INVALID
        because in the translation table the entries with a
        standard clipboard format id appear before the other
        entries with CF_INVALID
    */
    vector< FormatEntry >::const_iterator citer_end = m_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = m_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        sal_Int32 stdId = citer->aStandardFormatId;
        if ( aStandardFormatId == stdId )
        {
            aDataFlavor = citer->aDataFlavor;
            break;
        }
        else if ( stdId == CF_INVALID )
            break;
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::findDataFlavorForNativeFormatName( const OUString& aNativeFormatName, DataFlavor& aDataFlavor ) const
{
    vector< FormatEntry >::const_iterator citer_end = m_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = m_TranslTable.begin( );
          citer != citer_end;
          ++citer )
    {
        if ( aNativeFormatName.equalsIgnoreAsciiCase( citer->aNativeFormatName ) )
        {
            aDataFlavor = citer->aDataFlavor;
            break;
        }
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::findStandardFormatIdForCharset( const OUString& aCharset, Any& aAny ) const
{
    if ( aCharset.equalsIgnoreAsciiCase( OUSTR( utf-16 ) ) )
        aAny <<= static_cast< sal_Int32 >( CF_UNICODETEXT );
    else
    {
        sal_Int32 wincp = getWinCPFromMimeCharset( aCharset );
        if ( IsOEMCP ( wincp ) )
            aAny <<= static_cast< sal_Int32 >( CF_OEMTEXT );
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::setStandardFormatIdForNativeFormatName( const OUString& aNativeFormatName, Any& aAny ) const
{
    vector< FormatEntry >::const_iterator citer_end = m_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = m_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        if ( aNativeFormatName.equalsIgnoreAsciiCase( citer->aNativeFormatName ) &&
             (CF_INVALID != citer->aStandardFormatId) )
        {
            aAny <<= citer->aStandardFormatId;
            break;
        }
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::findStdFormatIdOrNativeFormatNameForFullMediaType(
    const Reference< XMimeContentTypeFactory >& aRefXMimeFactory,
    const OUString& aFullMediaType,
    Any& aAny ) const
{
    vector< FormatEntry >::const_iterator citer_end = m_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = m_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        Reference< XMimeContentType >
        refXMime( aRefXMimeFactory->createMimeContentType( citer->aDataFlavor.MimeType ) );
        if ( aFullMediaType.equalsIgnoreAsciiCase( refXMime->getFullMediaType( ) ) )
        {
            sal_Int32 cf = citer->aStandardFormatId;
            if ( CF_INVALID != cf )
                aAny <<= cf;
            else
            {
                OSL_ENSURE( citer->aNativeFormatName.getLength( ),
                    "Invalid standard format id and empty native format name in translation table" );
                aAny <<= citer->aNativeFormatName;
            }
            break;
        }
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

inline sal_Bool CDataFormatTranslator::isTextPlainMediaType( const OUString& fullMediaType ) const
{
    return (fullMediaType.equalsIgnoreAsciiCase(OUSTR(text/plain)));
}

// -------------------------------------------------
//
// -------------------------------------------------

DataFlavor SAL_CALL CDataFormatTranslator::mkDataFlv(const OUString& cnttype, const OUString& hpname, Type dtype)
{
    DataFlavor dflv;
    dflv.MimeType             = cnttype;
    dflv.HumanPresentableName = hpname;
    dflv.DataType             = dtype;
    return dflv;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
