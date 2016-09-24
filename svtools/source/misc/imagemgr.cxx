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
#include <svtools/svtools.hrc>
#include <svtools/imagemgr.hrc>
#include <svtools/svtresid.hxx>
#include <osl/mutex.hxx>
#include <vcl/lazydelete.hxx>

// globals *******************************************************************

#define NO_INDEX        (-1)
#define CONTENT_HELPER  ::utl::UCBContentHelper

struct SvtExtensionResIdMapping_Impl
{
    const char*   _pExt;
    bool    _bExt;
    sal_uInt16  _nStrId;
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
    { "lnk",   false, 0,                                     0 },
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
    { "pict",   true,  STR_DESCRIPTION_GRAPHIC_DOC,          IMG_PCT },
    { "pcx",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCX },
    { "pl",    true,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "png",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PNG },
    { "rar",   true,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "rtf",   false, STR_DESCRIPTION_WORD_DOC,              IMG_WRITER },
    { "sbl",   false, 0,                                     0 },
    { "sch",   false, 0,                                     0 },
    { "sda",   false, STR_DESCRIPTION_SDRAW_DOC,             IMG_DRAW },
    { "sdb",   false, STR_DESCRIPTION_SDATABASE_DOC,         IMG_DATABASE },
    { "sdc",   false, STR_DESCRIPTION_SCALC_DOC,             IMG_CALC },
    { "sdd",   false, STR_DESCRIPTION_SIMPRESS_DOC,          IMG_IMPRESS },
    { "sdp",   false, STR_DESCRIPTION_SIMPRESS_DOC,          0 },
    { "sds",   false, STR_DESCRIPTION_SCHART_DOC,            0 },
    { "sdw",   false, STR_DESCRIPTION_SWRITER_DOC,           IMG_WRITER },
    { "sga",   false, 0,                                     0 },
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
    { nullptr, false, 0, 0 }
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
    OUString aPath = aObj.GetURLPath( INetURLObject::NO_DECODE );
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
                        aExtension = OUString( pExtensions[0] );
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
    OUString aExt, sURL = rObject.GetMainURL( INetURLObject::NO_DECODE );
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

static sal_uInt16 GetDescriptionId_Impl( const OUString& rExtension, bool& rbShowExt )
{
    sal_uInt16 nId = 0;
    sal_Int32  nIndex = GetIndexOfExtension_Impl( rExtension );
    if ( nIndex != NO_INDEX )
    {
        nId = ExtensionMap_Impl[ nIndex ]._nStrId;
        rbShowExt = ExtensionMap_Impl[ nIndex ]._bExt;
    }

    return nId;
}

static OUString GetDescriptionByFactory_Impl( const OUString& rFactory )
{
    sal_uInt16 nResId = 0;
    if ( rFactory.startsWithIgnoreAsciiCase( "swriter" ) )
        nResId = STR_DESCRIPTION_FACTORY_WRITER;
    else if ( rFactory.startsWithIgnoreAsciiCase( "scalc" ) )
        nResId = STR_DESCRIPTION_FACTORY_CALC;
    else if ( rFactory.startsWithIgnoreAsciiCase( "simpress" ) )
        nResId = STR_DESCRIPTION_FACTORY_IMPRESS;
    else if ( rFactory.startsWithIgnoreAsciiCase( "sdraw" ) )
        nResId = STR_DESCRIPTION_FACTORY_DRAW;
    else if ( rFactory.startsWithIgnoreAsciiCase( "swriter/web" ) )
        nResId = STR_DESCRIPTION_FACTORY_WRITERWEB;
    else if ( rFactory.startsWithIgnoreAsciiCase( "swriter/globaldocument" ) )
        nResId = STR_DESCRIPTION_FACTORY_GLOBALDOC;
    else if ( rFactory.startsWithIgnoreAsciiCase( "smath" ) )
        nResId = STR_DESCRIPTION_FACTORY_MATH;
    else if ( rFactory.startsWithIgnoreAsciiCase( "sdatabase" ) )
        nResId = STR_DESCRIPTION_FACTORY_DATABASE;

    if ( nResId )
    {
        SolarMutexGuard aGuard;
        return SvtResId(nResId).toString();
    }
    return OUString();
}

static sal_uInt16 GetFolderDescriptionId_Impl( const OUString& rURL )
{
    sal_uInt16 nRet = STR_DESCRIPTION_FOLDER;
    svtools::VolumeInfo aVolumeInfo;
    try
    {
        ::ucbhelper::Content aCnt( rURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        if ( GetVolumeProperties_Impl( aCnt, aVolumeInfo ) )
        {
            if ( aVolumeInfo.m_bIsRemote )
                nRet = STR_DESCRIPTION_REMOTE_VOLUME;
            else if ( aVolumeInfo.m_bIsFloppy )
                nRet = STR_DESCRIPTION_FLOPPY_VOLUME;
            else if ( aVolumeInfo.m_bIsCompactDisc )
                nRet = STR_DESCRIPTION_CDROM_VOLUME;
            else if ( aVolumeInfo.m_bIsRemoveable || aVolumeInfo.m_bIsVolume )
                nRet = STR_DESCRIPTION_LOCALE_VOLUME;
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

static Image GetImageFromList_Impl( sal_uInt16 nImageId, bool bBig )
{
    if ( !bBig && IMG_FOLDER == nImageId )
        // return our new small folder image (256 colors)
        return Image( SvtResId( IMG_SVT_FOLDER ) );

    ImageList* pList = nullptr;

    static vcl::DeleteOnDeinit< ImageList > xSmallImageList( nullptr );
    static vcl::DeleteOnDeinit< ImageList > xBigImageList( nullptr );
    static OUString style = Application::GetSettings().GetStyleSettings().DetermineIconTheme();

    // If the style has been changed, throw away our cache of the older images
    OUString currentStyle = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    if ( style != currentStyle)
    {
        xSmallImageList.reset();
        xBigImageList.reset();
        style = currentStyle;
    }

    if ( bBig )
    {
        if ( !xBigImageList.get() )
            xBigImageList.reset(new ImageList(SvtResId(RID_SVTOOLS_IMAGELIST_BIG)));
        pList = xBigImageList.get();
    }
    else
    {
        if ( !xSmallImageList.get() )
            xSmallImageList.reset(new ImageList(SvtResId(RID_SVTOOLS_IMAGELIST_SMALL)));
        pList = xSmallImageList.get();
    }

    if ( pList->HasImageAtPos( nImageId ) )
        return pList->GetImage( nImageId );
    return Image();
}

OUString SvFileInformationManager::GetDescription_Impl( const INetURLObject& rObject, bool bDetectFolder )
{
    OUString sExtension(rObject.getExtension());
    OUString sDescription, sURL( rObject.GetMainURL( INetURLObject::NO_DECODE ) );
    sal_uInt16 nResId = 0;
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
                nResId = GetDescriptionId_Impl( sExtension, bShowExt );
            }
            if ( !nResId )
            {
                nResId = STR_DESCRIPTION_FILE;
                bOnlyFile = bExt;
            }
        }
    }
    else
        nResId = GetFolderDescriptionId_Impl( sURL );

    if ( nResId > 0 )
    {
        if ( bOnlyFile )
        {
            bShowExt = false;
            sExtension = sExtension.toAsciiUpperCase();
            sDescription = sExtension;
            sDescription += "-";
        }
        SolarMutexGuard aGuard;
        sDescription += SvtResId(nResId).toString();
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
    sal_uInt16 nResId = STR_DESCRIPTION_FOLDER;
    if ( rInfo.m_bIsRemote )
        nResId = STR_DESCRIPTION_REMOTE_VOLUME;
    else if ( rInfo.m_bIsFloppy )
        nResId = STR_DESCRIPTION_FLOPPY_VOLUME;
    else if ( rInfo.m_bIsCompactDisc )
        nResId = STR_DESCRIPTION_CDROM_VOLUME;
    else if ( rInfo.m_bIsRemoveable || rInfo.m_bIsVolume )
        nResId = STR_DESCRIPTION_LOCALE_VOLUME;

    return SvtResId(nResId).toString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
