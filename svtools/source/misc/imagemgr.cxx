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
#include "vcl/image.hxx"
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

#define NO_INDEX        ((sal_uInt16)0xFFFF)
#define CONTENT_HELPER  ::utl::UCBContentHelper

struct SvtExtensionResIdMapping_Impl
{
    const char*   _pExt;
    sal_Bool    _bExt;
    sal_uInt16  _nStrId;
    sal_uInt16  _nImgId;
};

static SvtExtensionResIdMapping_Impl const ExtensionMap_Impl[] =
{
    { "awk",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "bas",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "bat",   sal_True,  STR_DESCRIPTION_BATCHFILE,             0 },
    { "bmk",   sal_False, STR_DESCRIPTION_BOOKMARKFILE,          0 },
    { "bmp",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_BITMAP },
    { "c",     sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "cfg",   sal_False, STR_DESCRIPTION_CFGFILE,               0 },
    { "cmd",   sal_True,  STR_DESCRIPTION_BATCHFILE,             0 },
    { "cob",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "com",   sal_True,  STR_DESCRIPTION_APPLICATION,           0 },
    { "cxx",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "dbf",   sal_True,  STR_DESCRIPTION_DATABASE_TABLE,        IMG_TABLE },
    { "def",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "dll",   sal_True,  STR_DESCRIPTION_SYSFILE,               0 },
    { "doc",   sal_False, STR_DESCRIPTION_WORD_DOC,              IMG_WRITER },
    { "dot",   sal_False, STR_DESCRIPTION_WORD_DOC,              IMG_WRITERTEMPLATE },
    { "docx",  sal_False, STR_DESCRIPTION_WORD_DOC,              IMG_WRITER },
    { "dxf",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_DXF },
    { "exe",   sal_True,  STR_DESCRIPTION_APPLICATION,           0 },
    { "gif",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_GIF },
    { "h",     sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "hlp",   sal_False, STR_DESCRIPTION_HELP_DOC,              0 },
    { "hrc",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "htm",   sal_False, STR_DESCRIPTION_HTMLFILE,              IMG_HTML },
    { "html",  sal_False, STR_DESCRIPTION_HTMLFILE,              IMG_HTML },
    { "asp",   sal_False, STR_DESCRIPTION_HTMLFILE,              IMG_HTML },
    { "hxx",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "ini",   sal_False, STR_DESCRIPTION_CFGFILE,               0 },
    { "java",  sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "jpeg",  sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_JPG },
    { "jpg",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_JPG },
    { "lha",   sal_True,  STR_DESCRIPTION_ARCHIVFILE,            0 },
#ifdef WNT
    { "lnk",   sal_False, 0,                                     0 },
#endif
    { "log",   sal_True,  STR_DESCRIPTION_LOGFILE,               0 },
    { "lst",   sal_True,  STR_DESCRIPTION_LOGFILE,               0 },
    { "met",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_MET },
    { "mml",   sal_False, STR_DESCRIPTION_MATHML_DOC,            IMG_MATH },
    { "mod",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "odb",   sal_False, STR_DESCRIPTION_OO_DATABASE_DOC,       IMG_OO_DATABASE_DOC },
    { "odg",   sal_False, STR_DESCRIPTION_OO_DRAW_DOC,           IMG_OO_DRAW_DOC },
    { "odf",   sal_False, STR_DESCRIPTION_OO_MATH_DOC,           IMG_OO_MATH_DOC },
    { "odm",   sal_False, STR_DESCRIPTION_OO_GLOBAL_DOC,         IMG_OO_GLOBAL_DOC },
    { "odp",   sal_False, STR_DESCRIPTION_OO_IMPRESS_DOC,        IMG_OO_IMPRESS_DOC },
    { "ods",   sal_False, STR_DESCRIPTION_OO_CALC_DOC,           IMG_OO_CALC_DOC },
    { "odt",   sal_False, STR_DESCRIPTION_OO_WRITER_DOC,         IMG_OO_WRITER_DOC },
    { "otg",   sal_False, STR_DESCRIPTION_OO_DRAW_TEMPLATE,      IMG_OO_DRAW_TEMPLATE },
    { "otp",   sal_False, STR_DESCRIPTION_OO_IMPRESS_TEMPLATE,   IMG_OO_IMPRESS_TEMPLATE },
    { "ots",   sal_False, STR_DESCRIPTION_OO_CALC_TEMPLATE,      IMG_OO_CALC_TEMPLATE },
    { "ott",   sal_False, STR_DESCRIPTION_OO_WRITER_TEMPLATE,    IMG_OO_WRITER_TEMPLATE },
    { "pas",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "pcd",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCD },
    { "pct",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCT },
    { "pict",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,          IMG_PCT },
    { "pcx",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCX },
    { "pl",    sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "png",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PNG },
    { "rar",   sal_True,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "rtf",   sal_False, STR_DESCRIPTION_WORD_DOC,              IMG_WRITER },
    { "sbl",   sal_False, 0,                                     0 },
    { "sch",   sal_False, 0,                                     0 },
    { "sda",   sal_False, STR_DESCRIPTION_SDRAW_DOC,             IMG_DRAW },
    { "sdb",   sal_False, STR_DESCRIPTION_SDATABASE_DOC,         IMG_DATABASE },
    { "sdc",   sal_False, STR_DESCRIPTION_SCALC_DOC,             IMG_CALC },
    { "sdd",   sal_False, STR_DESCRIPTION_SIMPRESS_DOC,          IMG_IMPRESS },
    { "sdp",   sal_False, STR_DESCRIPTION_SIMPRESS_DOC,          0 },
    { "sds",   sal_False, STR_DESCRIPTION_SCHART_DOC,            0 },
    { "sdw",   sal_False, STR_DESCRIPTION_SWRITER_DOC,           IMG_WRITER },
    { "sga",   sal_False, 0,                                     0 },
    { "sgf",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SGF },
    { "sgl",   sal_False, STR_DESCRIPTION_GLOBALDOC,             IMG_GLOBAL_DOC },
    { "sgv",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SGV },
    { "shtml", sal_False, STR_DESCRIPTION_HTMLFILE,              IMG_HTML },
    { "sim",   sal_False, STR_DESCRIPTION_SIMAGE_DOC,            IMG_SIM },
    { "smf",   sal_False, STR_DESCRIPTION_SMATH_DOC,             IMG_MATH },
    { "src",   sal_True,  STR_DESCRIPTION_SOURCEFILE,            0 },
    { "svh",   sal_False, STR_DESCRIPTION_HELP_DOC,              0 },
    { "svm",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SVM },
    { "stc",   sal_False, STR_DESCRIPTION_CALC_TEMPLATE,         IMG_CALCTEMPLATE },
    { "std",   sal_False, STR_DESCRIPTION_DRAW_TEMPLATE,         IMG_DRAWTEMPLATE },
    { "sti",   sal_False, STR_DESCRIPTION_IMPRESS_TEMPLATE,      IMG_IMPRESSTEMPLATE },
    { "stw",   sal_False, STR_DESCRIPTION_WRITER_TEMPLATE,       IMG_WRITERTEMPLATE },
    { "sxc",   sal_False, STR_DESCRIPTION_SXCALC_DOC,            IMG_CALC },
    { "sxd",   sal_False, STR_DESCRIPTION_SXDRAW_DOC,            IMG_DRAW },
    { "sxg",   sal_False, STR_DESCRIPTION_SXGLOBAL_DOC,          IMG_GLOBAL_DOC },
    { "sxi",   sal_False, STR_DESCRIPTION_SXIMPRESS_DOC,         IMG_IMPRESS },
    { "sxm",   sal_False, STR_DESCRIPTION_SXMATH_DOC,            IMG_MATH },
    { "sxs",   sal_False, STR_DESCRIPTION_SXCHART_DOC,           0 },
    { "sxw",   sal_False, STR_DESCRIPTION_SXWRITER_DOC,          IMG_WRITER },
    { "sys",   sal_True,  STR_DESCRIPTION_SYSFILE,               0 },
    { "tif",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_TIFF },
    { "tiff",  sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_TIFF },
    { "txt",   sal_False, STR_DESCRIPTION_TEXTFILE,              IMG_TEXTFILE },
    { "url",   sal_False, STR_DESCRIPTION_LINK,                  0 },
    { "vor",   sal_False, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC,  IMG_WRITERTEMPLATE },
    { "vxd",   sal_True,  STR_DESCRIPTION_SYSFILE,               0 },
    { "wmf",   sal_True,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_WMF },
    { "xls",   sal_False, STR_DESCRIPTION_EXCEL_DOC,             IMG_CALC },
    { "xlt",   sal_False, STR_DESCRIPTION_EXCEL_TEMPLATE_DOC,    IMG_CALCTEMPLATE },
    { "xlsx",  sal_False, STR_DESCRIPTION_EXCEL_DOC,             IMG_CALC },
    { "uu",    sal_True,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "uue",   sal_True,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "z",     sal_True,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "zip",   sal_True,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "zoo",   sal_True,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "gz",    sal_True,  STR_DESCRIPTION_ARCHIVFILE,            0 },
    { "ppt",   sal_False, STR_DESCRIPTION_POWERPOINT,            IMG_IMPRESS },
    { "pot",   sal_False, STR_DESCRIPTION_POWERPOINT_TEMPLATE,   IMG_IMPRESSTEMPLATE },
    { "pps",   sal_False, STR_DESCRIPTION_POWERPOINT_SHOW,       IMG_IMPRESS },
    { "pptx",  sal_False, STR_DESCRIPTION_POWERPOINT,            IMG_IMPRESS },
    { "oxt",   sal_False, STR_DESCRIPTION_EXTENSION,             IMG_EXTENSION },
    { 0, sal_False, 0, 0 }
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
    { NULL, NULL }
};

//****************************************************************************

static String GetImageExtensionByFactory_Impl( const String& rURL )
{
    INetURLObject aObj( rURL );
    String aPath = aObj.GetURLPath( INetURLObject::NO_DECODE );
    String aExtension;

    if ( aPath.Len() )
    {
        sal_uInt16 nIndex = 0;
        while ( Fac2ExtMap_Impl[ nIndex ]._pFactory )
        {
            if ( aPath.EqualsAscii( Fac2ExtMap_Impl[ nIndex ]._pFactory ) )
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
        ::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        ::com::sun::star::uno::Reference < ::com::sun::star::document::XTypeDetection > xTypeDetector(
            xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", xContext),
            ::com::sun::star::uno::UNO_QUERY );

        OUString aInternalType = xTypeDetector->queryTypeByURL( rURL );
        ::com::sun::star::uno::Reference < ::com::sun::star::container::XNameAccess > xAccess( xTypeDetector, ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aTypeProps;
        if ( !aInternalType.isEmpty() && xAccess->hasByName( aInternalType ) )
        {
            xAccess->getByName( aInternalType ) >>= aTypeProps;
            sal_Int32 nProps = aTypeProps.getLength();
            for ( sal_Int32 i = 0; i < nProps; ++i )
            {
                const ::com::sun::star::beans::PropertyValue& rProp = aTypeProps[i];
                if ( rProp.Name.compareToAscii("Extensions") == COMPARE_EQUAL )
                {
                    ::com::sun::star::uno::Sequence < OUString > aExtensions;
                    if ( ( rProp.Value >>= aExtensions ) && aExtensions.getLength() > 0 )
                    {
                        const OUString* pExtensions = aExtensions.getConstArray();
                        aExtension = String( pExtensions[0] );
                        break;
                    }
                }
            }
        }
    }
    catch( const ::com::sun::star::uno::RuntimeException& )
    {
        throw; // dont hide it!
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        // type detection failed -> no extension
    }

    return aExtension;
}

static sal_uInt16 GetIndexOfExtension_Impl( const String& rExtension )
{
    sal_uInt16 nRet = NO_INDEX;
    if ( rExtension.Len() )
    {
        sal_uInt16 nIndex = 0;
        String aExt = rExtension;
        aExt.ToLowerAscii();
        while ( ExtensionMap_Impl[ nIndex ]._pExt )
        {
            if ( aExt.EqualsAscii( ExtensionMap_Impl[ nIndex ]._pExt ) )
            {
                nRet = nIndex;
                break;
            }
            ++nIndex;
        }
    }

    return nRet;
}

static sal_uInt16 GetImageId_Impl( const String& rExtension )
{
    sal_uInt16 nImage = IMG_FILE;
    if ( rExtension.Len()  != NO_INDEX )
    {
        sal_uInt16 nIndex = GetIndexOfExtension_Impl( rExtension );
        if ( nIndex != NO_INDEX )
        {
            nImage = ExtensionMap_Impl[ nIndex ]._nImgId;
            if ( !nImage )
                nImage = IMG_FILE;
        }
    }

    return nImage;
}

static sal_Bool GetVolumeProperties_Impl( ::ucbhelper::Content& rContent, svtools::VolumeInfo& rVolumeInfo )
{
    sal_Bool bRet = sal_False;

    try
    {
        bRet = ( ( rContent.getPropertyValue( "IsVolume" ) >>= rVolumeInfo.m_bIsVolume ) &&
                 ( rContent.getPropertyValue( "IsRemote" ) >>= rVolumeInfo.m_bIsRemote ) &&
                 ( rContent.getPropertyValue( "IsRemoveable" ) >>= rVolumeInfo.m_bIsRemoveable ) &&
                 ( rContent.getPropertyValue( "IsFloppy" ) >>= rVolumeInfo.m_bIsFloppy ) &&
                 ( rContent.getPropertyValue( "IsCompactDisc" ) >>= rVolumeInfo.m_bIsCompactDisc ) );
    }
    catch( const ::com::sun::star::uno::RuntimeException& )
    {
        throw; // dont hide it!
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        // type detection failed -> no extension
    }

    return bRet;
}

static sal_uInt16 GetFolderImageId_Impl( const String& rURL )
{
    sal_uInt16 nRet = IMG_FOLDER;
    ::svtools::VolumeInfo aVolumeInfo;
    try
    {
        ::ucbhelper::Content aCnt( rURL, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
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
    catch( const ::com::sun::star::uno::RuntimeException& )
    {
        throw; // dont hide it!
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        // ...
    }
    return nRet;
}

static sal_uInt16 GetImageId_Impl( const INetURLObject& rObject, sal_Bool bDetectFolder )
{
    String aExt, sURL = rObject.GetMainURL( INetURLObject::NO_DECODE );
    sal_uInt16 nImage = IMG_FILE;

    if ( rObject.GetProtocol() == INET_PROT_PRIVATE )
    {
        String aURLPath = sURL.Copy( URL_PREFIX_PRIV_SOFFICE_LEN );
        String aType = aURLPath.GetToken( 0, INET_PATH_TOKEN );
        if ( aType == String( RTL_CONSTASCII_USTRINGPARAM("factory") ) )
        {
            // detect an image id for our "private:factory" urls
            aExt = GetImageExtensionByFactory_Impl( sURL );
            if ( aExt.Len() > 0 )
                nImage = GetImageId_Impl( aExt );
            return nImage;
        }
        else if ( aType == String( RTL_CONSTASCII_USTRINGPARAM("image") ) )
            nImage = (sal_uInt16)aURLPath.GetToken( 1, INET_PATH_TOKEN ).ToInt32();
    }
    else
    {
        aExt = rObject.getExtension();
        if ( aExt.EqualsAscii( "vor" ) )
        {
            SotStorageRef aStorage = new SotStorage( sURL, STREAM_STD_READ );
            sal_uInt16 nId = IMG_WRITERTEMPLATE;
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

            return nId;
        }
    }

    if ( nImage == IMG_FILE && sURL.Len() )
    {
        if ( bDetectFolder && CONTENT_HELPER::IsFolder( sURL ) )
            nImage = GetFolderImageId_Impl( sURL );
        else if ( aExt.Len() > 0 )
            nImage = GetImageId_Impl( aExt );
    }
    return nImage;
}

static sal_uInt16 GetDescriptionId_Impl( const String& rExtension, sal_Bool& rbShowExt )
{
    sal_uInt16 nId = 0;

    if ( rExtension.Len()  != NO_INDEX )
    {
        sal_uInt16 nIndex = GetIndexOfExtension_Impl( rExtension );
        if ( nIndex != NO_INDEX )
        {
            nId = ExtensionMap_Impl[ nIndex ]._nStrId;
            rbShowExt = ExtensionMap_Impl[ nIndex ]._bExt;
        }
    }

    return nId;
}

static String GetDescriptionByFactory_Impl( const String& rFactory )
{
    sal_uInt16 nResId = 0;
    if ( rFactory.EqualsIgnoreCaseAscii( "swriter", 0, 7 ) )
        nResId = STR_DESCRIPTION_FACTORY_WRITER;
    else if ( rFactory.EqualsIgnoreCaseAscii( "scalc", 0, 5 ) )
        nResId = STR_DESCRIPTION_FACTORY_CALC;
    else if ( rFactory.EqualsIgnoreCaseAscii( "simpress", 0, 8 ) )
        nResId = STR_DESCRIPTION_FACTORY_IMPRESS;
    else if ( rFactory.EqualsIgnoreCaseAscii( "sdraw", 0, 5 ) )
        nResId = STR_DESCRIPTION_FACTORY_DRAW;
    else if ( rFactory.EqualsIgnoreCaseAscii( "swriter/web", 0, 11 ) )
        nResId = STR_DESCRIPTION_FACTORY_WRITERWEB;
    else if ( rFactory.EqualsIgnoreCaseAscii( "swriter/globaldocument", 0, 22 ) )
        nResId = STR_DESCRIPTION_FACTORY_GLOBALDOC;
    else if ( rFactory.EqualsIgnoreCaseAscii( "smath", 0, 5 ) )
        nResId = STR_DESCRIPTION_FACTORY_MATH;
    else if ( rFactory.EqualsIgnoreCaseAscii( "sdatabase", 0, 9 ) )
        nResId = STR_DESCRIPTION_FACTORY_DATABASE;

    if ( nResId )
    {
        SolarMutexGuard aGuard;
        return SvtResId(nResId).toString();
    }
    return OUString();
}

static sal_uInt16 GetFolderDescriptionId_Impl( const String& rURL )
{
    sal_uInt16 nRet = STR_DESCRIPTION_FOLDER;
    svtools::VolumeInfo aVolumeInfo;
    try
    {
        ::ucbhelper::Content aCnt( rURL, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
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
    catch( const ::com::sun::star::uno::RuntimeException& )
    {
        throw; // dont hide it!
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        // ...
    }
    return nRet;
}

static Image GetImageFromList_Impl( sal_uInt16 nImageId, sal_Bool bBig )
{
    if ( !bBig && IMG_FOLDER == nImageId )
        // return our new small folder image (256 colors)
        return Image( SvtResId( IMG_SVT_FOLDER ) );

    ImageList* pList = NULL;

    static vcl::DeleteOnDeinit< ImageList > xSmallImageList( NULL );
    static vcl::DeleteOnDeinit< ImageList > xBigImageList( NULL );
    static sal_uLong nStyle = Application::GetSettings().GetStyleSettings().GetSymbolsStyle();

    // If the style has been changed, throw away our cache of the older images
    if ( nStyle != Application::GetSettings().GetStyleSettings().GetSymbolsStyle() )
    {
        xSmallImageList.reset();
        xBigImageList.reset();
        nStyle = Application::GetSettings().GetStyleSettings().GetSymbolsStyle();
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

OUString SvFileInformationManager::GetDescription_Impl( const INetURLObject& rObject, sal_Bool bDetectFolder )
{
    OUString sExtension(rObject.getExtension());
    String sDescription, sURL( rObject.GetMainURL( INetURLObject::NO_DECODE ) );
    sal_uInt16 nResId = 0;
    sal_Bool bShowExt = sal_False, bDetected = sal_False, bOnlyFile = sal_False;
    sal_Bool bFolder = bDetectFolder ? CONTENT_HELPER::IsFolder( sURL ) : sal_False;
    if ( !bFolder )
    {
        if ( !bDetected )
        {
            if ( rObject.GetProtocol() == INET_PROT_PRIVATE )
            {
                String aURLPath = sURL.Copy( URL_PREFIX_PRIV_SOFFICE_LEN );
                String aType = aURLPath.GetToken( 0, INET_PATH_TOKEN );
                if ( aType == String( RTL_CONSTASCII_USTRINGPARAM("factory") ) )
                {
                    sDescription = GetDescriptionByFactory_Impl( aURLPath.Copy( aURLPath.Search( INET_PATH_TOKEN ) + 1 ) );
                    bDetected = sal_True;
                }
            }

            if ( !bDetected )
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
    }
    else
        nResId = GetFolderDescriptionId_Impl( sURL );

    if ( nResId > 0 )
    {
        if ( bOnlyFile )
        {
            bShowExt = sal_False;
            sExtension = sExtension.toAsciiUpperCase();
            sDescription = sExtension;
            sDescription += '-';
        }
        SolarMutexGuard aGuard;
        sDescription += SvtResId(nResId).toString();
    }

    DBG_ASSERT( sDescription.Len() > 0, "file without description" );

    if ( bShowExt )
    {
        sDescription += String( RTL_CONSTASCII_USTRINGPARAM(" (") );
        sDescription += sExtension;
        sDescription += ')';
    }

    return sDescription;
}

Image SvFileInformationManager::GetImage( const INetURLObject& rObject, sal_Bool bBig )
{
    sal_uInt16 nImage = GetImageId_Impl( rObject, sal_True );
    DBG_ASSERT( nImage, "invalid ImageId" );
    return GetImageFromList_Impl( nImage, bBig );
}

Image SvFileInformationManager::GetFileImage( const INetURLObject& rObject, sal_Bool bBig )
{
    sal_Int16 nImage = GetImageId_Impl( rObject, sal_False );
    DBG_ASSERT( nImage, "invalid ImageId" );
    return GetImageFromList_Impl( nImage, bBig );
}

Image SvFileInformationManager::GetImageNoDefault( const INetURLObject& rObject, sal_Bool bBig )
{
    sal_uInt16 nImage = GetImageId_Impl( rObject, sal_True );
    DBG_ASSERT( nImage, "invalid ImageId" );

    if ( nImage == IMG_FILE )
        return Image();

    return GetImageFromList_Impl( nImage, bBig );
}

Image SvFileInformationManager::GetFolderImage( const svtools::VolumeInfo& rInfo, sal_Bool bBig )
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

    return GetImageFromList_Impl( nImage, bBig );
}

OUString SvFileInformationManager::GetDescription( const INetURLObject& rObject )
{
    return SvFileInformationManager::GetDescription_Impl( rObject, sal_True );
}

OUString SvFileInformationManager::GetFileDescription( const INetURLObject& rObject )
{
    return SvFileInformationManager::GetDescription_Impl( rObject, sal_False );
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
