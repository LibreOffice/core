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

#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <sot/storage.hxx>
#include <comphelper/classids.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <ucbhelper/content.hxx>
#include <tools/rcid.h>
#include <unotools/configmgr.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <vcl/lazydelete.hxx>
#include "imagemgr.hrc"
#include "bitmaps.hlst"
#include "strings.hxx"

// globals *******************************************************************

#define NO_INDEX        (-1)
#define CONTENT_HELPER  ::utl::UCBContentHelper

struct SvtExtensionResIdMapping_Impl
{
    const char*   _pExt;
    bool    _bExt;
    const char* pStrId;
    sal_uInt16  _nImgId;
};

static SvtExtensionResIdMapping_Impl const ExtensionMap_Impl[] =
{
    { "awk",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "bas",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "bat",   true,  STR_DESCRIPTION_BATCHFILE,             0 },
    { "bmk",   false, STR_DESCRIPTION_BOOKMARKFILE,          0 },
    { "bmp",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_BITMAP },
    { "c",     true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "cfg",   false, STR_DESCRIPTION_CFGFILE,               0 },
    { "cmd",   true,  STR_DESCRIPTION_BATCHFILE,             0 },
    { "cob",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "com",   true,  STR_DESCRIPTION_APPLICATION,           0 },
    { "cxx",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "dbf",   true,  STR_DESCRIPTION_DATABASE_TABLE,        IMG_TABLE },
    { "def",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "dll",   true,  STR_DESCRIPTION_SYSFILE,               0 },
    { "doc",   false, STR_DESCRIPTION_WORD_DOC,              IMG_WRITER },
    { "dot",   false, STR_DESCRIPTION_WORD_DOC,              IMG_WRITERTEMPLATE },
    { "docx",  false, STR_DESCRIPTION_WORD_DOC,              IMG_WRITER },
    { "dxf",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_DXF },
    { "exe",   true,  STR_DESCRIPTION_APPLICATION,           0 },
    { "gif",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_GIF },
    { "h",     true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "hlp",   false, STR_DESCRIPTION_HELP_DOC,              0 },
    { "hrc",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "htm",   false, STR_DESCRIPTION_HTMLFILE,              IMG_HTML },
    { "html",  false, STR_DESCRIPTION_HTMLFILE,              IMG_HTML },
    { "asp",   false, STR_DESCRIPTION_HTMLFILE,              IMG_HTML },
    { "hxx",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "ini",   false, STR_DESCRIPTION_CFGFILE,               0 },
    { "java",  true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "jpeg",  true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_JPG },
    { "jpg",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_JPG },
    { "lha",   true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
#ifdef _WIN32
    { "lnk",   false, nullptr,                               0 },
#endif
    { "log",   true,  STR_DESCRIPTION_LOGFILE,               0 },
    { "lst",   true,  STR_DESCRIPTION_LOGFILE,               0 },
    { "met",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_MET },
    { "mml",   false, STR_DESCRIPTION_MATHML_DOC,            IMG_MATH },
    { "mod",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "odb",   false, STR_DESCRIPTION_OO_DATABASE_DOC,       IMG_OO_DATABASE_DOC },
    { "odg",   false, STR_DESCRIPTION_OO_DRAW_DOC,           IMG_OO_DRAW_DOC },
    { "odf",   false, STR_DESCRIPTION_OO_MATH_DOC,           IMG_OO_MATH_DOC },
    { "odm",   false, STR_DESCRIPTION_OO_GLOBAL_DOC,         IMG_OO_GLOBAL_DOC },
    { "odp",   false, STR_DESCRIPTION_OO_IMPRESS_DOC,        IMG_OO_IMPRESS_DOC },
    { "ods",   false, STR_DESCRIPTION_OO_CALC_DOC,           IMG_OO_CALC_DOC },
    { "odt",   false, STR_DESCRIPTION_OO_WRITER_DOC,         IMG_OO_WRITER_DOC },
    { "otg",   false, STR_DESCRIPTION_OO_DRAW_TEMPLATE,      IMG_OO_DRAW_TEMPLATE },
    { "otp",   false, STR_DESCRIPTION_OO_IMPRESS_TEMPLATE,   IMG_OO_IMPRESS_TEMPLATE },
    { "ots",   false, STR_DESCRIPTION_OO_CALC_TEMPLATE,      IMG_OO_CALC_TEMPLATE },
    { "ott",   false, STR_DESCRIPTION_OO_WRITER_TEMPLATE,    IMG_OO_WRITER_TEMPLATE },
    { "pas",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "pcd",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCD },
    { "pct",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCT },
    { "pict",  true,  STR_DESCRIPTION_GRAPHIC_DOC,          IMG_PCT },
    { "pcx",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCX },
    { "pl",    true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "png",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PNG },
    { "rar",   true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "rtf",   false, STR_DESCRIPTION_WORD_DOC,              IMG_WRITER },
    { "sbl",   false, nullptr,                               0 },
    { "sch",   false, nullptr,                               0 },
    { "sda",   false, STR_DESCRIPTION_SDRAW_DOC,             IMG_DRAW },
    { "sdb",   false, STR_DESCRIPTION_SDATABASE_DOC,         IMG_DATABASE },
    { "sdc",   false, STR_DESCRIPTION_SCALC_DOC,             IMG_CALC },
    { "sdd",   false, STR_DESCRIPTION_SIMPRESS_DOC,          IMG_IMPRESS },
    { "sdp",   false, STR_DESCRIPTION_SIMPRESS_DOC,          0 },
    { "sds",   false, STR_DESCRIPTION_SCHART_DOC,            0 },
    { "sdw",   false, STR_DESCRIPTION_SWRITER_DOC,           IMG_WRITER },
    { "sga",   false, nullptr,                               0 },
    { "sgf",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SGF },
    { "sgl",   false, STR_DESCRIPTION_GLOBALDOC,             IMG_GLOBAL_DOC },
    { "sgv",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SGV },
    { "shtml", false, STR_DESCRIPTION_HTMLFILE,              IMG_HTML },
    { "sim",   false, STR_DESCRIPTION_SIMAGE_DOC,            IMG_SIM },
    { "smf",   false, STR_DESCRIPTION_SMATH_DOC,             IMG_MATH },
    { "src",   true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "svh",   false, STR_DESCRIPTION_HELP_DOC,              0 },
    { "svm",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SVM },
    { "stc",   false, STR_DESCRIPTION_CALC_TEMPLATE,         IMG_CALCTEMPLATE },
    { "std",   false, STR_DESCRIPTION_DRAW_TEMPLATE,         IMG_DRAWTEMPLATE },
    { "sti",   false, STR_DESCRIPTION_IMPRESS_TEMPLATE,      IMG_IMPRESSTEMPLATE },
    { "stw",   false, STR_DESCRIPTION_WRITER_TEMPLATE,       IMG_WRITERTEMPLATE },
    { "sxc",   false, STR_DESCRIPTION_SXCALC_DOC,            IMG_CALC },
    { "sxd",   false, STR_DESCRIPTION_SXDRAW_DOC,            IMG_DRAW },
    { "sxg",   false, STR_DESCRIPTION_SXGLOBAL_DOC,          IMG_GLOBAL_DOC },
    { "sxi",   false, STR_DESCRIPTION_SXIMPRESS_DOC,         IMG_IMPRESS },
    { "sxm",   false, STR_DESCRIPTION_SXMATH_DOC,            IMG_MATH },
    { "sxs",   false, STR_DESCRIPTION_SXCHART_DOC,           0 },
    { "sxw",   false, STR_DESCRIPTION_SXWRITER_DOC,          IMG_WRITER },
    { "sys",   true,  STR_DESCRIPTION_SYSFILE,               0 },
    { "tif",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_TIFF },
    { "tiff",  true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_TIFF },
    { "txt",   false, STR_DESCRIPTION_TEXTFILE,              IMG_TEXTFILE },
    { "url",   false, STR_DESCRIPTION_LINK,                  0 },
    { "vor",   false, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC,  IMG_WRITERTEMPLATE },
    { "vxd",   true,  STR_DESCRIPTION_SYSFILE,               0 },
    { "wmf",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_WMF },
    { "xls",   false, STR_DESCRIPTION_EXCEL_DOC,             IMG_CALC },
    { "xlt",   false, STR_DESCRIPTION_EXCEL_TEMPLATE_DOC,    IMG_CALCTEMPLATE },
    { "xlsx",  false, STR_DESCRIPTION_EXCEL_DOC,             IMG_CALC },
    { "uu",    true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "uue",   true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "z",     true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "zip",   true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "zoo",   true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "gz",    true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "ppt",   false, STR_DESCRIPTION_POWERPOINT,            IMG_IMPRESS },
    { "pot",   false, STR_DESCRIPTION_POWERPOINT_TEMPLATE,   IMG_IMPRESSTEMPLATE },
    { "pps",   false, STR_DESCRIPTION_POWERPOINT_SHOW,       IMG_IMPRESS },
    { "pptx",  false, STR_DESCRIPTION_POWERPOINT,            IMG_IMPRESS },
    { "oxt",   false, STR_DESCRIPTION_EXTENSION,             IMG_EXTENSION },
    { nullptr, false, nullptr, 0 }
};

struct SvtFactory2ExtensionMapping_Impl
{
    const char*   _pFactory;
    const char*   _pExtension;
};

// mapping from "private:factory" url to extension

static SvtFactory2ExtensionMapping_Impl const Fac2ExtMap_Impl[] =
{
    { "swriter",                "odt" },
    { "swriter/web",            "html" },
    { "swriter/GlobalDocument", "odm" },
    { "scalc",                  "ods" },
    { "simpress",               "odp" },
    { "sdraw",                  "odg" },
    { "smath",                  "odf" },
    { "sdatabase",              "odb" },
    { nullptr, nullptr }
};


static OUString GetImageExtensionByFactory_Impl( const OUString& rURL )
{
    INetURLObject aObj( rURL );
    OUString aPath = aObj.GetURLPath( INetURLObject::DecodeMechanism::NONE );
    OUString aExtension;

    if ( !aPath.isEmpty() )
    {
        sal_uInt16 nIndex = 0;
        while ( Fac2ExtMap_Impl[ nIndex ]._pFactory )
        {
            if ( aPath.equalsAscii( Fac2ExtMap_Impl[ nIndex ]._pFactory ) )
            {
                // extension found
                aExtension = OUString::createFromAscii(Fac2ExtMap_Impl[ nIndex ]._pExtension);
                // and return it
                return aExtension;
            }
            ++nIndex;
        }
    }

    // no extension found, so use the type detection (performance brake)

    try
    {
        // get the TypeDetection service to access all registered types
        css::uno::Reference < css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        css::uno::Reference < css::document::XTypeDetection > xTypeDetector(
            xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", xContext),
            css::uno::UNO_QUERY );

        OUString aInternalType = xTypeDetector->queryTypeByURL( rURL );
        css::uno::Reference < css::container::XNameAccess > xAccess( xTypeDetector, css::uno::UNO_QUERY );
        css::uno::Sequence < css::beans::PropertyValue > aTypeProps;
        if ( !aInternalType.isEmpty() && xAccess->hasByName( aInternalType ) )
        {
            xAccess->getByName( aInternalType ) >>= aTypeProps;
            sal_Int32 nProps = aTypeProps.getLength();
            for ( sal_Int32 i = 0; i < nProps; ++i )
            {
                const css::beans::PropertyValue& rProp = aTypeProps[i];
                if (rProp.Name == "Extensions")
                {
                    css::uno::Sequence < OUString > aExtensions;
                    if ( ( rProp.Value >>= aExtensions ) && aExtensions.getLength() > 0 )
                    {
                        const OUString* pExtensions = aExtensions.getConstArray();
                        aExtension = pExtensions[0];
                        break;
                    }
                }
            }
        }
    }
    catch( const css::uno::RuntimeException& )
    {
        throw; // don't hide it!
    }
    catch( const css::uno::Exception& )
    {
        // type detection failed -> no extension
    }

    return aExtension;
}

static sal_Int32 GetIndexOfExtension_Impl( const OUString& rExtension )
{
    sal_Int32 nRet = NO_INDEX;
    if ( !rExtension.isEmpty() )
    {
        sal_Int32 nIndex = 0;
        OUString aExt = rExtension.toAsciiLowerCase();
        while ( ExtensionMap_Impl[ nIndex ]._pExt )
        {
            if ( aExt.equalsAscii( ExtensionMap_Impl[ nIndex ]._pExt ) )
            {
                nRet = nIndex;
                break;
            }
            ++nIndex;
        }
    }

    return nRet;
}

static sal_uInt16 GetImageId_Impl( const OUString& rExtension )
{
    sal_uInt16 nImage = IMG_FILE;
    sal_Int32  nIndex = GetIndexOfExtension_Impl( rExtension );
    if ( nIndex != NO_INDEX )
    {
        nImage = ExtensionMap_Impl[ nIndex ]._nImgId;
        if ( !nImage )
            nImage = IMG_FILE;
    }

    return nImage;
}

static bool GetVolumeProperties_Impl( ::ucbhelper::Content& rContent, svtools::VolumeInfo& rVolumeInfo )
{
    bool bRet = false;

    try
    {
        bRet = ( ( rContent.getPropertyValue( "IsVolume" ) >>= rVolumeInfo.m_bIsVolume ) &&
                 ( rContent.getPropertyValue( "IsRemote" ) >>= rVolumeInfo.m_bIsRemote ) &&
                 ( rContent.getPropertyValue( "IsRemoveable" ) >>= rVolumeInfo.m_bIsRemoveable ) &&
                 ( rContent.getPropertyValue( "IsFloppy" ) >>= rVolumeInfo.m_bIsFloppy ) &&
                 ( rContent.getPropertyValue( "IsCompactDisc" ) >>= rVolumeInfo.m_bIsCompactDisc ) );
    }
    catch( const css::uno::RuntimeException& )
    {
        throw; // don't hide it!
    }
    catch( const css::uno::Exception& )
    {
        // type detection failed -> no extension
    }

    return bRet;
}

static sal_uInt16 GetFolderImageId_Impl( const OUString& rURL )
{
    sal_uInt16 nRet = IMG_FOLDER;
    ::svtools::VolumeInfo aVolumeInfo;
    try
    {
        ::ucbhelper::Content aCnt( rURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        if ( GetVolumeProperties_Impl( aCnt, aVolumeInfo ) )
        {
            if ( aVolumeInfo.m_bIsRemote )
                nRet = IMG_NETWORKDEV;
            else if ( aVolumeInfo.m_bIsCompactDisc )
                nRet = IMG_CDROMDEV;
            else if ( aVolumeInfo.m_bIsRemoveable )
                nRet = IMG_REMOVABLEDEV;
            else if ( aVolumeInfo.m_bIsVolume )
                nRet = IMG_FIXEDDEV;
        }
    }
    catch( const css::uno::RuntimeException& )
    {
        throw; // don't hide it!
    }
    catch( const css::uno::Exception& )
    {

    }
    return nRet;
}

static sal_uInt16 GetImageId_Impl( const INetURLObject& rObject, bool bDetectFolder )
{
    OUString aExt, sURL = rObject.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    sal_uInt16 nImage = IMG_FILE;

    if ( rObject.GetProtocol() == INetProtocol::PrivSoffice )
    {
        OUString aURLPath = sURL.copy( strlen(URL_PREFIX_PRIV_SOFFICE) );
        OUString aType = aURLPath.getToken( 0, '/' );
        if ( aType == "factory" )
        {
            // detect an image id for our "private:factory" urls
            aExt = GetImageExtensionByFactory_Impl( sURL );
            if ( !aExt.isEmpty() )
                nImage = GetImageId_Impl( aExt );
            return nImage;
        }
        else if ( aType == "image" )
            nImage = (sal_uInt16)aURLPath.getToken( 1, '/' ).toInt32();
    }
    else
    {
        aExt = rObject.getExtension();
        if ( aExt == "vor" )
        {
            sal_uInt16 nId = IMG_WRITERTEMPLATE;
            try
            {
                tools::SvRef<SotStorage> aStorage = new SotStorage( sURL, StreamMode::STD_READ );
                if ( !aStorage->GetError() )
                {
                    SvGlobalName aGlobalName = aStorage->GetClassName();
                    if ( aGlobalName == SvGlobalName(SO3_SC_CLASSID_50) || aGlobalName == SvGlobalName(SO3_SC_CLASSID_40) || aGlobalName == SvGlobalName(SO3_SC_CLASSID_30) )
                        nId = IMG_CALCTEMPLATE;
                    else if ( aGlobalName == SvGlobalName(SO3_SDRAW_CLASSID_50) )
                        nId = IMG_DRAWTEMPLATE;
                    else if ( aGlobalName == SvGlobalName(SO3_SIMPRESS_CLASSID_50) ||
                            aGlobalName == SvGlobalName(SO3_SIMPRESS_CLASSID_40) || aGlobalName == SvGlobalName(SO3_SIMPRESS_CLASSID_30) )
                        nId = IMG_IMPRESSTEMPLATE;
                    else if ( aGlobalName == SvGlobalName(SO3_SM_CLASSID_50) || aGlobalName == SvGlobalName(SO3_SM_CLASSID_40) || aGlobalName == SvGlobalName(SO3_SM_CLASSID_30) )
                        nId = IMG_MATHTEMPLATE;
                }
            }
            catch (const css::ucb::ContentCreationException& e)
            {
                SAL_WARN("svtools.misc", "GetImageId_Impl: Caught exception: " << e.Message);
            }

            return nId;
        }
    }

    if ( nImage == IMG_FILE && !sURL.isEmpty() )
    {
        if ( bDetectFolder && CONTENT_HELPER::IsFolder( sURL ) )
            nImage = GetFolderImageId_Impl( sURL );
        else if ( !aExt.isEmpty() )
            nImage = GetImageId_Impl( aExt );
    }
    return nImage;
}

static const char* GetDescriptionId_Impl( const OUString& rExtension, bool& rbShowExt )
{
    const char* pId = nullptr;
    sal_Int32  nIndex = GetIndexOfExtension_Impl( rExtension );
    if ( nIndex != NO_INDEX )
    {
        pId = ExtensionMap_Impl[ nIndex ].pStrId;
        rbShowExt = ExtensionMap_Impl[ nIndex ]._bExt;
    }

    return pId;
}

static OUString GetDescriptionByFactory_Impl( const OUString& rFactory )
{
    const char* pResId = nullptr;
    if ( rFactory.startsWithIgnoreAsciiCase( "swriter" ) )
        pResId = STR_DESCRIPTION_FACTORY_WRITER;
    else if ( rFactory.startsWithIgnoreAsciiCase( "scalc" ) )
        pResId = STR_DESCRIPTION_FACTORY_CALC;
    else if ( rFactory.startsWithIgnoreAsciiCase( "simpress" ) )
        pResId = STR_DESCRIPTION_FACTORY_IMPRESS;
    else if ( rFactory.startsWithIgnoreAsciiCase( "sdraw" ) )
        pResId = STR_DESCRIPTION_FACTORY_DRAW;
    else if ( rFactory.startsWithIgnoreAsciiCase( "swriter/web" ) )
        pResId = STR_DESCRIPTION_FACTORY_WRITERWEB;
    else if ( rFactory.startsWithIgnoreAsciiCase( "swriter/globaldocument" ) )
        pResId = STR_DESCRIPTION_FACTORY_GLOBALDOC;
    else if ( rFactory.startsWithIgnoreAsciiCase( "smath" ) )
        pResId = STR_DESCRIPTION_FACTORY_MATH;
    else if ( rFactory.startsWithIgnoreAsciiCase( "sdatabase" ) )
        pResId = STR_DESCRIPTION_FACTORY_DATABASE;

    if (pResId)
    {
        return SvtResId(pResId);
    }
    return OUString();
}

static const char* GetFolderDescriptionId_Impl( const OUString& rURL )
{
    const char* pRet = STR_DESCRIPTION_FOLDER;
    svtools::VolumeInfo aVolumeInfo;
    try
    {
        ::ucbhelper::Content aCnt( rURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        if ( GetVolumeProperties_Impl( aCnt, aVolumeInfo ) )
        {
            if ( aVolumeInfo.m_bIsRemote )
                pRet = STR_DESCRIPTION_REMOTE_VOLUME;
            else if ( aVolumeInfo.m_bIsFloppy )
                pRet = STR_DESCRIPTION_FLOPPY_VOLUME;
            else if ( aVolumeInfo.m_bIsCompactDisc )
                pRet = STR_DESCRIPTION_CDROM_VOLUME;
            else if ( aVolumeInfo.m_bIsRemoveable || aVolumeInfo.m_bIsVolume )
                pRet = STR_DESCRIPTION_LOCALE_VOLUME;
        }
    }
    catch( const css::uno::RuntimeException& )
    {
        throw; // don't hide it!
    }
    catch( const css::uno::Exception& )
    {

    }
    return pRet;
}

static Image GetImageFromList_Impl( sal_uInt16 nImageId, bool bBig )
{
    if (bBig)
    {
        switch (nImageId)
        {
            case IMG_IMPRESS:
                return Image(BitmapEx(BMP_IMPRESS_LC));
            case IMG_BITMAP:
                return Image(BitmapEx(BMP_BITMAP_LC));
            case IMG_CALC:
                return Image(BitmapEx(BMP_CALC_LC));
            case IMG_CALCTEMPLATE:
                return Image(BitmapEx(BMP_CALCTEMPLATE_LC));
            case IMG_DATABASE:
                return Image(BitmapEx(BMP_DATABASE_LC));
            case IMG_IMPRESSTEMPLATE:
                return Image(BitmapEx(BMP_IMPRESSTEMPLATE_LC));
            case IMG_GIF:
                return Image(BitmapEx(BMP_GIF_LC));
            case IMG_HTML:
                return Image(BitmapEx(BMP_HTML_LC));
            case IMG_JPG:
                return Image(BitmapEx(BMP_JPG_LC));
            case IMG_MATH:
                return Image(BitmapEx(BMP_MATH_LC));
            case IMG_MATHTEMPLATE:
                return Image(BitmapEx(BMP_MATHTEMPLATE_LC));
            case IMG_FILE:
                return Image(BitmapEx(BMP_FILE_LC));
            case IMG_PCD:
                return Image(BitmapEx(BMP_PCD_LC));
            case IMG_PCT:
                return Image(BitmapEx(BMP_PCT_LC));
            case IMG_PCX:
                return Image(BitmapEx(BMP_PCX_LC));
            case IMG_SIM:
                return Image(BitmapEx(BMP_SIM_LC));
            case IMG_TEXTFILE:
                return Image(BitmapEx(BMP_TEXTFILE_LC));
            case IMG_TIFF:
                return Image(BitmapEx(BMP_TIFF_LC));
            case IMG_WMF:
                return Image(BitmapEx(BMP_WMF_LC));
            case IMG_WRITER:
                return Image(BitmapEx(BMP_WRITER_LC));
            case IMG_WRITERTEMPLATE:
                return Image(BitmapEx(BMP_WRITERTEMPLATE_LC));
            case IMG_FIXEDDEV:
                return Image(BitmapEx(BMP_FIXEDDEV_LC));
            case IMG_REMOVABLEDEV:
                return Image(BitmapEx(BMP_REMOVABLEDEV_LC));
            case IMG_CDROMDEV:
                return Image(BitmapEx(BMP_CDROMDEV_LC));
            case IMG_NETWORKDEV:
                return Image(BitmapEx(BMP_NETWORKDEV_LC));
            case IMG_TABLE:
                return Image(BitmapEx(BMP_TABLE_LC));
            case IMG_FOLDER:
                return Image(BitmapEx(BMP_FOLDER_LC));
            case IMG_DXF:
                return Image(BitmapEx(BMP_DXF_LC));
            case IMG_MET:
                return Image(BitmapEx(BMP_MET_LC));
            case IMG_PNG:
                return Image(BitmapEx(BMP_PNG_LC));
            case IMG_SGF:
                return Image(BitmapEx(BMP_SGF_LC));
            case IMG_SGV:
                return Image(BitmapEx(BMP_SGV_LC));
            case IMG_SVM:
                return Image(BitmapEx(BMP_SVM_LC));
            case IMG_GLOBAL_DOC:
                return Image(BitmapEx(BMP_GLOBAL_DOC_LC));
            case IMG_DRAW:
                return Image(BitmapEx(BMP_DRAW_LC));
            case IMG_DRAWTEMPLATE:
                return Image(BitmapEx(BMP_DRAWTEMPLATE_LC));
            case IMG_TEMPLATE:
                return Image(BitmapEx(BMP_TEMPLATE_LC));
            case IMG_OO_DATABASE_DOC:
                return Image(BitmapEx(BMP_OO_DATABASE_DOC_LC));
            case IMG_OO_DRAW_DOC:
                return Image(BitmapEx(BMP_OO_DRAW_DOC_LC));
            case IMG_OO_MATH_DOC:
                return Image(BitmapEx(BMP_OO_MATH_DOC_LC));
            case IMG_OO_GLOBAL_DOC:
                return Image(BitmapEx(BMP_OO_GLOBAL_DOC_LC));
            case IMG_OO_IMPRESS_DOC:
                return Image(BitmapEx(BMP_OO_IMPRESS_DOC_LC));
            case IMG_OO_CALC_DOC:
                return Image(BitmapEx(BMP_OO_CALC_DOC_LC));
            case IMG_OO_WRITER_DOC:
                return Image(BitmapEx(BMP_OO_WRITER_DOC_LC));
            case IMG_OO_DRAW_TEMPLATE:
                return Image(BitmapEx(BMP_OO_DRAW_TEMPLATE_LC));
            case IMG_OO_IMPRESS_TEMPLATE:
                return Image(BitmapEx(BMP_OO_IMPRESS_TEMPLATE_LC));
            case IMG_OO_CALC_TEMPLATE:
                return Image(BitmapEx(BMP_OO_CALC_TEMPLATE_LC));
            case IMG_OO_WRITER_TEMPLATE:
                return Image(BitmapEx(BMP_OO_WRITER_TEMPLATE_LC));
            case IMG_EXTENSION:
                return Image(BitmapEx(BMP_EXTENSION_LC));
        }
    }
    else
    {
        switch (nImageId)
        {
            case IMG_IMPRESS:
                return Image(BitmapEx(BMP_IMPRESS_SC));
            case IMG_BITMAP:
                return Image(BitmapEx(BMP_BITMAP_SC));
            case IMG_CALC:
                return Image(BitmapEx(BMP_CALC_SC));
            case IMG_CALCTEMPLATE:
                return Image(BitmapEx(BMP_CALCTEMPLATE_SC));
            case IMG_DATABASE:
                return Image(BitmapEx(BMP_DATABASE_SC));
            case IMG_IMPRESSTEMPLATE:
                return Image(BitmapEx(BMP_IMPRESSTEMPLATE_SC));
            case IMG_GIF:
                return Image(BitmapEx(BMP_GIF_SC));
            case IMG_HTML:
                return Image(BitmapEx(BMP_HTML_SC));
            case IMG_JPG:
                return Image(BitmapEx(BMP_JPG_SC));
            case IMG_MATH:
                return Image(BitmapEx(BMP_MATH_SC));
            case IMG_MATHTEMPLATE:
                return Image(BitmapEx(BMP_MATHTEMPLATE_SC));
            case IMG_FILE:
                return Image(BitmapEx(BMP_FILE_SC));
            case IMG_PCD:
                return Image(BitmapEx(BMP_PCD_SC));
            case IMG_PCT:
                return Image(BitmapEx(BMP_PCT_SC));
            case IMG_PCX:
                return Image(BitmapEx(BMP_PCX_SC));
            case IMG_SIM:
                return Image(BitmapEx(BMP_SIM_SC));
            case IMG_TEXTFILE:
                return Image(BitmapEx(BMP_TEXTFILE_SC));
            case IMG_TIFF:
                return Image(BitmapEx(BMP_TIFF_SC));
            case IMG_WMF:
                return Image(BitmapEx(BMP_WMF_SC));
            case IMG_WRITER:
                return Image(BitmapEx(BMP_WRITER_SC));
            case IMG_WRITERTEMPLATE:
                return Image(BitmapEx(BMP_WRITERTEMPLATE_SC));
            case IMG_FIXEDDEV:
                return Image(BitmapEx(BMP_FIXEDDEV_SC));
            case IMG_REMOVABLEDEV:
                return Image(BitmapEx(BMP_REMOVABLEDEV_SC));
            case IMG_CDROMDEV:
                return Image(BitmapEx(BMP_CDROMDEV_SC));
            case IMG_NETWORKDEV:
                return Image(BitmapEx(BMP_NETWORKDEV_SC));
            case IMG_TABLE:
                return Image(BitmapEx(BMP_TABLE_SC));
            case IMG_FOLDER:
                // if not bBig, then return our new small folder image (256 colors)
                return Image(BitmapEx(RID_BMP_FOLDER));
            case IMG_DXF:
                return Image(BitmapEx(BMP_DXF_SC));
            case IMG_MET:
                return Image(BitmapEx(BMP_MET_SC));
            case IMG_PNG:
                return Image(BitmapEx(BMP_PNG_SC));
            case IMG_SGF:
                return Image(BitmapEx(BMP_SGF_SC));
            case IMG_SGV:
                return Image(BitmapEx(BMP_SGV_SC));
            case IMG_SVM:
                return Image(BitmapEx(BMP_SVM_SC));
            case IMG_GLOBAL_DOC:
                return Image(BitmapEx(BMP_GLOBAL_DOC_SC));
            case IMG_DRAW:
                return Image(BitmapEx(BMP_DRAW_SC));
            case IMG_DRAWTEMPLATE:
                return Image(BitmapEx(BMP_DRAWTEMPLATE_SC));
            case IMG_TEMPLATE:
                return Image(BitmapEx(BMP_TEMPLATE_SC));
            case IMG_OO_DATABASE_DOC:
                return Image(BitmapEx(BMP_OO_DATABASE_DOC_SC));
            case IMG_OO_DRAW_DOC:
                return Image(BitmapEx(BMP_OO_DRAW_DOC_SC));
            case IMG_OO_MATH_DOC:
                return Image(BitmapEx(BMP_OO_MATH_DOC_SC));
            case IMG_OO_GLOBAL_DOC:
                return Image(BitmapEx(BMP_OO_GLOBAL_DOC_SC));
            case IMG_OO_IMPRESS_DOC:
                return Image(BitmapEx(BMP_OO_IMPRESS_DOC_SC));
            case IMG_OO_CALC_DOC:
                return Image(BitmapEx(BMP_OO_CALC_DOC_SC));
            case IMG_OO_WRITER_DOC:
                return Image(BitmapEx(BMP_OO_WRITER_DOC_SC));
            case IMG_OO_DRAW_TEMPLATE:
                return Image(BitmapEx(BMP_OO_DRAW_TEMPLATE_SC));
            case IMG_OO_IMPRESS_TEMPLATE:
                return Image(BitmapEx(BMP_OO_IMPRESS_TEMPLATE_SC));
            case IMG_OO_CALC_TEMPLATE:
                return Image(BitmapEx(BMP_OO_CALC_TEMPLATE_SC));
            case IMG_OO_WRITER_TEMPLATE:
                return Image(BitmapEx(BMP_OO_WRITER_TEMPLATE_SC));
            case IMG_EXTENSION:
                return Image(BitmapEx(BMP_EXTENSION_SC));
        }
    }

    return Image();
}

OUString SvFileInformationManager::GetDescription_Impl( const INetURLObject& rObject, bool bDetectFolder )
{
    OUString sExtension(rObject.getExtension());
    OUString sDescription, sURL( rObject.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    const char* pResId = nullptr;
    bool bShowExt = false, bOnlyFile = false;
    bool bFolder = bDetectFolder && CONTENT_HELPER::IsFolder( sURL );
    if ( !bFolder )
    {
        bool bDetected = false;

        if ( rObject.GetProtocol() == INetProtocol::PrivSoffice )
        {
            OUString aURLPath = sURL.copy( strlen(URL_PREFIX_PRIV_SOFFICE) );
            OUString aType = aURLPath.getToken( 0, '/' );
            if ( aType == "factory" )
            {
                sDescription = GetDescriptionByFactory_Impl( aURLPath.copy( aURLPath.indexOf( '/' ) + 1 ) );
                bDetected = true;
            }
        }

        if (!bDetected)
        {
            // search a description by extension
            bool bExt = !sExtension.isEmpty();
            if ( bExt )
            {
                sExtension = sExtension.toAsciiLowerCase();
                pResId = GetDescriptionId_Impl( sExtension, bShowExt );
            }
            if (!pResId)
            {
                pResId = STR_DESCRIPTION_FILE;
                bOnlyFile = bExt;
            }
        }
    }
    else
        pResId = GetFolderDescriptionId_Impl( sURL );

    if (pResId)
    {
        if ( bOnlyFile )
        {
            bShowExt = false;
            sExtension = sExtension.toAsciiUpperCase();
            sDescription = sExtension;
            sDescription += "-";
        }
        sDescription += SvtResId(pResId);
    }

    DBG_ASSERT( !sDescription.isEmpty(), "file without description" );

    if ( bShowExt )
    {
        sDescription += " (";
        sDescription += sExtension;
        sDescription += ")";
    }

    return sDescription;
}

Image SvFileInformationManager::GetImage( const INetURLObject& rObject, bool bBig )
{
    sal_uInt16 nImage = GetImageId_Impl( rObject, true );
    DBG_ASSERT( nImage, "invalid ImageId" );
    return GetImageFromList_Impl( nImage, bBig );
}

Image SvFileInformationManager::GetFileImage( const INetURLObject& rObject )
{
    sal_Int16 nImage = GetImageId_Impl( rObject, false );
    DBG_ASSERT( nImage, "invalid ImageId" );
    return GetImageFromList_Impl( nImage, false/*bBig*/ );
}

Image SvFileInformationManager::GetImageNoDefault( const INetURLObject& rObject, bool bBig )
{
    sal_uInt16 nImage = GetImageId_Impl( rObject, true );
    DBG_ASSERT( nImage, "invalid ImageId" );

    if ( nImage == IMG_FILE )
        return Image();

    return GetImageFromList_Impl( nImage, bBig );
}

Image SvFileInformationManager::GetFolderImage( const svtools::VolumeInfo& rInfo )
{
    sal_uInt16 nImage = IMG_FOLDER;
    DBG_ASSERT( nImage, "invalid ImageId" );

    if ( rInfo.m_bIsRemote )
        nImage = IMG_NETWORKDEV;
    else if ( rInfo.m_bIsCompactDisc )
        nImage = IMG_CDROMDEV;
    else if ( rInfo.m_bIsRemoveable || rInfo.m_bIsFloppy )
        nImage = IMG_REMOVABLEDEV;
    else if ( rInfo.m_bIsVolume )
        nImage = IMG_FIXEDDEV;

    return GetImageFromList_Impl( nImage, false/*bBig*/ );
}

OUString SvFileInformationManager::GetDescription( const INetURLObject& rObject )
{
    return SvFileInformationManager::GetDescription_Impl( rObject, true );
}

OUString SvFileInformationManager::GetFileDescription( const INetURLObject& rObject )
{
    return SvFileInformationManager::GetDescription_Impl( rObject, false );
}

OUString SvFileInformationManager::GetFolderDescription( const svtools::VolumeInfo& rInfo )
{
    const char* pResId = STR_DESCRIPTION_FOLDER;
    if ( rInfo.m_bIsRemote )
        pResId = STR_DESCRIPTION_REMOTE_VOLUME;
    else if ( rInfo.m_bIsFloppy )
        pResId = STR_DESCRIPTION_FLOPPY_VOLUME;
    else if ( rInfo.m_bIsCompactDisc )
        pResId = STR_DESCRIPTION_CDROM_VOLUME;
    else if ( rInfo.m_bIsRemoveable || rInfo.m_bIsVolume )
        pResId = STR_DESCRIPTION_LOCALE_VOLUME;

    return SvtResId(pResId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
