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
#include <sal/log.hxx>
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
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <ucbhelper/content.hxx>
#include <unotools/configmgr.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <bitmaps.hlst>
#include <strings.hxx>

// globals *******************************************************************

#define NO_INDEX        (-1)
#define CONTENT_HELPER  ::utl::UCBContentHelper

struct SvtExtensionResIdMapping_Impl
{
    const char* _pExt;
    bool const        _bExt;
    const char* pStrId;
    SvImageId const   _nImgId;
};

static SvtExtensionResIdMapping_Impl const ExtensionMap_Impl[] =
{
    { "awk",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "bas",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "bat",   true,  STR_DESCRIPTION_BATCHFILE,             SvImageId::NONE },
    { "bmk",   false, STR_DESCRIPTION_BOOKMARKFILE,          SvImageId::NONE },
    { "bmp",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::Bitmap },
    { "c",     true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "cfg",   false, STR_DESCRIPTION_CFGFILE,               SvImageId::NONE },
    { "cmd",   true,  STR_DESCRIPTION_BATCHFILE,             SvImageId::NONE },
    { "cob",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "com",   true,  STR_DESCRIPTION_APPLICATION,           SvImageId::NONE },
    { "cxx",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "dbf",   true,  STR_DESCRIPTION_DATABASE_TABLE,        SvImageId::Table },
    { "def",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "dll",   true,  STR_DESCRIPTION_SYSFILE,               SvImageId::NONE },
    { "doc",   false, STR_DESCRIPTION_WORD_DOC,              SvImageId::Writer },
    { "dot",   false, STR_DESCRIPTION_WORD_DOC,              SvImageId::WriterTemplate },
    { "docx",  false, STR_DESCRIPTION_WORD_DOC,              SvImageId::Writer },
    { "dxf",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::DXF },
    { "exe",   true,  STR_DESCRIPTION_APPLICATION,           SvImageId::NONE },
    { "gif",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::GIF },
    { "h",     true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "hlp",   false, STR_DESCRIPTION_HELP_DOC,              SvImageId::NONE },
    { "hrc",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "htm",   false, STR_DESCRIPTION_HTMLFILE,              SvImageId::HTML },
    { "html",  false, STR_DESCRIPTION_HTMLFILE,              SvImageId::HTML },
    { "asp",   false, STR_DESCRIPTION_HTMLFILE,              SvImageId::HTML },
    { "hxx",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "ini",   false, STR_DESCRIPTION_CFGFILE,               SvImageId::NONE },
    { "java",  true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "jpeg",  true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::JPG },
    { "jpg",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::JPG },
    { "lha",   true,  STR_DESCRIPTION_ARCHIVFILE,            SvImageId::NONE },
#ifdef _WIN32
    { "lnk",   false, nullptr,                               SvImageId::NONE },
#endif
    { "log",   true,  STR_DESCRIPTION_LOGFILE,               SvImageId::NONE },
    { "lst",   true,  STR_DESCRIPTION_LOGFILE,               SvImageId::NONE },
    { "met",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::MET },
    { "mml",   false, STR_DESCRIPTION_MATHML_DOC,            SvImageId::Math },
    { "mod",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "odb",   false, STR_DESCRIPTION_OO_DATABASE_DOC,       SvImageId::OO_DatabaseDoc },
    { "odg",   false, STR_DESCRIPTION_OO_DRAW_DOC,           SvImageId::OO_DrawDoc },
    { "odf",   false, STR_DESCRIPTION_OO_MATH_DOC,           SvImageId::OO_MathDoc },
    { "odm",   false, STR_DESCRIPTION_OO_GLOBAL_DOC,         SvImageId::OO_GlobalDoc },
    { "odp",   false, STR_DESCRIPTION_OO_IMPRESS_DOC,        SvImageId::OO_ImpressDoc },
    { "ods",   false, STR_DESCRIPTION_OO_CALC_DOC,           SvImageId::OO_CalcDoc },
    { "odt",   false, STR_DESCRIPTION_OO_WRITER_DOC,         SvImageId::OO_WriterDoc },
    { "otg",   false, STR_DESCRIPTION_OO_DRAW_TEMPLATE,      SvImageId::OO_DrawTemplate },
    { "otp",   false, STR_DESCRIPTION_OO_IMPRESS_TEMPLATE,   SvImageId::OO_ImpressTemplate },
    { "ots",   false, STR_DESCRIPTION_OO_CALC_TEMPLATE,      SvImageId::OO_CalcTemplate },
    { "ott",   false, STR_DESCRIPTION_OO_WRITER_TEMPLATE,    SvImageId::OO_WriterTemplate },
    { "pas",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "pcd",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::PCD },
    { "pct",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::PCT },
    { "pict",  true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::PCT },
    { "pcx",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::PCX },
    { "pl",    true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "png",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::PNG },
    { "rar",   true,  STR_DESCRIPTION_ARCHIVFILE,            SvImageId::NONE },
    { "rtf",   false, STR_DESCRIPTION_WORD_DOC,              SvImageId::Writer },
    { "sbl",   false, nullptr,                               SvImageId::NONE },
    { "sch",   false, nullptr,                               SvImageId::NONE },
    { "sda",   false, STR_DESCRIPTION_SDRAW_DOC,             SvImageId::Draw },
    { "sdb",   false, STR_DESCRIPTION_SDATABASE_DOC,         SvImageId::Database },
    { "sdc",   false, STR_DESCRIPTION_SCALC_DOC,             SvImageId::Calc },
    { "sdd",   false, STR_DESCRIPTION_SIMPRESS_DOC,          SvImageId::Impress },
    { "sdp",   false, STR_DESCRIPTION_SIMPRESS_DOC,          SvImageId::NONE },
    { "sds",   false, STR_DESCRIPTION_SCHART_DOC,            SvImageId::NONE },
    { "sdw",   false, STR_DESCRIPTION_SWRITER_DOC,           SvImageId::Writer },
    { "sga",   false, nullptr,                               SvImageId::NONE },
    { "sgl",   false, STR_DESCRIPTION_GLOBALDOC,             SvImageId::GlobalDoc },
    { "shtml", false, STR_DESCRIPTION_HTMLFILE,              SvImageId::HTML },
    { "sim",   false, STR_DESCRIPTION_SIMAGE_DOC,            SvImageId::SIM },
    { "smf",   false, STR_DESCRIPTION_SMATH_DOC,             SvImageId::Math },
    { "src",   true,  STR_DESCRIPTION_SOURCEFILE,            SvImageId::NONE },
    { "svh",   false, STR_DESCRIPTION_HELP_DOC,              SvImageId::NONE },
    { "svm",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::SVM },
    { "stc",   false, STR_DESCRIPTION_CALC_TEMPLATE,         SvImageId::CalcTemplate },
    { "std",   false, STR_DESCRIPTION_DRAW_TEMPLATE,         SvImageId::DrawTemplate },
    { "sti",   false, STR_DESCRIPTION_IMPRESS_TEMPLATE,      SvImageId::ImpressTemplate },
    { "stw",   false, STR_DESCRIPTION_WRITER_TEMPLATE,       SvImageId::WriterTemplate },
    { "sxc",   false, STR_DESCRIPTION_SXCALC_DOC,            SvImageId::Calc },
    { "sxd",   false, STR_DESCRIPTION_SXDRAW_DOC,            SvImageId::Draw },
    { "sxg",   false, STR_DESCRIPTION_SXGLOBAL_DOC,          SvImageId::GlobalDoc },
    { "sxi",   false, STR_DESCRIPTION_SXIMPRESS_DOC,         SvImageId::Impress },
    { "sxm",   false, STR_DESCRIPTION_SXMATH_DOC,            SvImageId::Math },
    { "sxs",   false, STR_DESCRIPTION_SXCHART_DOC,           SvImageId::NONE },
    { "sxw",   false, STR_DESCRIPTION_SXWRITER_DOC,          SvImageId::Writer },
    { "sys",   true,  STR_DESCRIPTION_SYSFILE,               SvImageId::NONE },
    { "tif",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::TIFF },
    { "tiff",  true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::TIFF },
    { "txt",   false, STR_DESCRIPTION_TEXTFILE,              SvImageId::TextFile },
    { "url",   false, STR_DESCRIPTION_LINK,                  SvImageId::NONE },
    { "vor",   false, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC,  SvImageId::WriterTemplate },
    { "vxd",   true,  STR_DESCRIPTION_SYSFILE,               SvImageId::NONE },
    { "wmf",   true,  STR_DESCRIPTION_GRAPHIC_DOC,           SvImageId::WMF },
    { "xls",   false, STR_DESCRIPTION_EXCEL_DOC,             SvImageId::Calc },
    { "xlt",   false, STR_DESCRIPTION_EXCEL_TEMPLATE_DOC,    SvImageId::CalcTemplate },
    { "xlsx",  false, STR_DESCRIPTION_EXCEL_DOC,             SvImageId::Calc },
    { "uu",    true,  STR_DESCRIPTION_ARCHIVFILE,            SvImageId::NONE },
    { "uue",   true,  STR_DESCRIPTION_ARCHIVFILE,            SvImageId::NONE },
    { "z",     true,  STR_DESCRIPTION_ARCHIVFILE,            SvImageId::NONE },
    { "zip",   true,  STR_DESCRIPTION_ARCHIVFILE,            SvImageId::NONE },
    { "zoo",   true,  STR_DESCRIPTION_ARCHIVFILE,            SvImageId::NONE },
    { "gz",    true,  STR_DESCRIPTION_ARCHIVFILE,            SvImageId::NONE },
    { "ppt",   false, STR_DESCRIPTION_POWERPOINT,            SvImageId::Impress },
    { "pot",   false, STR_DESCRIPTION_POWERPOINT_TEMPLATE,   SvImageId::ImpressTemplate },
    { "pps",   false, STR_DESCRIPTION_POWERPOINT_SHOW,       SvImageId::Impress },
    { "pptx",  false, STR_DESCRIPTION_POWERPOINT,            SvImageId::Impress },
    { "oxt",   false, STR_DESCRIPTION_EXTENSION,             SvImageId::Extension },
    { nullptr, false, nullptr, SvImageId::NONE }
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

static SvImageId GetImageId_Impl( const OUString& rExtension )
{
    SvImageId nImage = SvImageId::File;
    sal_Int32  nIndex = GetIndexOfExtension_Impl( rExtension );
    if ( nIndex != NO_INDEX )
    {
        nImage = ExtensionMap_Impl[ nIndex ]._nImgId;
        if ( nImage == SvImageId::NONE )
            nImage = SvImageId::File;
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

static SvImageId GetFolderImageId_Impl( const OUString& rURL )
{
    SvImageId nRet = SvImageId::Folder;
    ::svtools::VolumeInfo aVolumeInfo;
    try
    {
        ::ucbhelper::Content aCnt( rURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        if ( GetVolumeProperties_Impl( aCnt, aVolumeInfo ) )
        {
            if ( aVolumeInfo.m_bIsRemote )
                nRet = SvImageId::NetworkDevice;
            else if ( aVolumeInfo.m_bIsCompactDisc )
                nRet = SvImageId::CDRomDevice;
            else if ( aVolumeInfo.m_bIsRemoveable )
                nRet = SvImageId::RemoveableDevice;
            else if ( aVolumeInfo.m_bIsVolume )
                nRet = SvImageId::FixedDevice;
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

static SvImageId GetImageId_Impl( const INetURLObject& rObject, bool bDetectFolder )
{
    OUString aExt, sURL = rObject.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    SvImageId nImage = SvImageId::File;

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
            nImage = static_cast<SvImageId>(aURLPath.getToken( 1, '/' ).toInt32());
    }
    else
    {
        aExt = rObject.getExtension();
        if ( aExt == "vor" )
        {
            SvImageId nId = SvImageId::WriterTemplate;
            try
            {
                tools::SvRef<SotStorage> aStorage = new SotStorage( sURL, StreamMode::STD_READ );
                if ( !aStorage->GetError() )
                {
                    SvGlobalName aGlobalName = aStorage->GetClassName();
                    if ( aGlobalName == SvGlobalName(SO3_SC_CLASSID_50) || aGlobalName == SvGlobalName(SO3_SC_CLASSID_40) || aGlobalName == SvGlobalName(SO3_SC_CLASSID_30) )
                        nId = SvImageId::CalcTemplate;
                    else if ( aGlobalName == SvGlobalName(SO3_SDRAW_CLASSID_50) )
                        nId = SvImageId::DrawTemplate;
                    else if ( aGlobalName == SvGlobalName(SO3_SIMPRESS_CLASSID_50) ||
                            aGlobalName == SvGlobalName(SO3_SIMPRESS_CLASSID_40) || aGlobalName == SvGlobalName(SO3_SIMPRESS_CLASSID_30) )
                        nId = SvImageId::ImpressTemplate;
                    else if ( aGlobalName == SvGlobalName(SO3_SM_CLASSID_50) || aGlobalName == SvGlobalName(SO3_SM_CLASSID_40) || aGlobalName == SvGlobalName(SO3_SM_CLASSID_30) )
                        nId = SvImageId::MathTemplate;
                }
            }
            catch (const css::ucb::ContentCreationException& e)
            {
                SAL_WARN("svtools.misc", "GetImageId_Impl: Caught " << e);
            }

            return nId;
        }
    }

    if ( nImage == SvImageId::File && !sURL.isEmpty() )
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

static OUString GetImageNameFromList_Impl( SvImageId nImageId, bool bBig )
{
    if (bBig)
    {
        switch (nImageId)
        {
            case SvImageId::Impress:
                return OUString(BMP_IMPRESS_LC);
            case SvImageId::Bitmap:
                return OUString(BMP_BITMAP_LC);
            case SvImageId::Calc:
                return OUString(BMP_CALC_LC);
            case SvImageId::CalcTemplate:
                return OUString(BMP_CALCTEMPLATE_LC);
            case SvImageId::Database:
                return OUString(BMP_DATABASE_LC);
            case SvImageId::ImpressTemplate:
                return OUString(BMP_IMPRESSTEMPLATE_LC);
            case SvImageId::GIF:
                return OUString(BMP_GIF_LC);
            case SvImageId::HTML:
                return OUString(BMP_HTML_LC);
            case SvImageId::JPG:
                return OUString(BMP_JPG_LC);
            case SvImageId::Math:
                return OUString(BMP_MATH_LC);
            case SvImageId::MathTemplate:
                return OUString(BMP_MATHTEMPLATE_LC);
            case SvImageId::File:
                return OUString(BMP_FILE_LC);
            case SvImageId::PCD:
                return OUString(BMP_PCD_LC);
            case SvImageId::PCT:
                return OUString(BMP_PCT_LC);
            case SvImageId::PCX:
                return OUString(BMP_PCX_LC);
            case SvImageId::SIM:
                return OUString(BMP_SIM_LC);
            case SvImageId::TextFile:
                return OUString(BMP_TEXTFILE_LC);
            case SvImageId::TIFF:
                return OUString(BMP_TIFF_LC);
            case SvImageId::WMF:
                return OUString(BMP_WMF_LC);
            case SvImageId::Writer:
                return OUString(BMP_WRITER_LC);
            case SvImageId::WriterTemplate:
                return OUString(BMP_WRITERTEMPLATE_LC);
            case SvImageId::FixedDevice:
                return OUString(BMP_FIXEDDEV_LC);
            case SvImageId::RemoveableDevice:
                return OUString(BMP_REMOVABLEDEV_LC);
            case SvImageId::CDRomDevice:
                return OUString(BMP_CDROMDEV_LC);
            case SvImageId::NetworkDevice:
                return OUString(BMP_NETWORKDEV_LC);
            case SvImageId::Table:
                return OUString(BMP_TABLE_LC);
            case SvImageId::Folder:
                return OUString(BMP_FOLDER_LC);
            case SvImageId::DXF:
                return OUString(BMP_DXF_LC);
            case SvImageId::MET:
                return OUString(BMP_MET_LC);
            case SvImageId::PNG:
                return OUString(BMP_PNG_LC);
            case SvImageId::SVM:
                return OUString(BMP_SVM_LC);
            case SvImageId::GlobalDoc:
                return OUString(BMP_GLOBAL_DOC_LC);
            case SvImageId::Draw:
                return OUString(BMP_DRAW_LC);
            case SvImageId::DrawTemplate:
                return OUString(BMP_DRAWTEMPLATE_LC);
            case SvImageId::OO_DatabaseDoc:
                return OUString(BMP_OO_DATABASE_DOC_LC);
            case SvImageId::OO_DrawDoc:
                return OUString(BMP_OO_DRAW_DOC_LC);
            case SvImageId::OO_MathDoc:
                return OUString(BMP_OO_MATH_DOC_LC);
            case SvImageId::OO_GlobalDoc:
                return OUString(BMP_OO_GLOBAL_DOC_LC);
            case SvImageId::OO_ImpressDoc:
                return OUString(BMP_OO_IMPRESS_DOC_LC);
            case SvImageId::OO_CalcDoc:
                return OUString(BMP_OO_CALC_DOC_LC);
            case SvImageId::OO_WriterDoc:
                return OUString(BMP_OO_WRITER_DOC_LC);
            case SvImageId::OO_DrawTemplate:
                return OUString(BMP_OO_DRAW_TEMPLATE_LC);
            case SvImageId::OO_ImpressTemplate:
                return OUString(BMP_OO_IMPRESS_TEMPLATE_LC);
            case SvImageId::OO_CalcTemplate:
                return OUString(BMP_OO_CALC_TEMPLATE_LC);
            case SvImageId::OO_WriterTemplate:
                return OUString(BMP_OO_WRITER_TEMPLATE_LC);
            case SvImageId::Extension:
                return OUString(BMP_EXTENSION_LC);
            default: break;
        }
    }
    else
    {
        switch (nImageId)
        {
            case SvImageId::Impress:
                return OUString(BMP_IMPRESS_SC);
            case SvImageId::Bitmap:
                return OUString(BMP_BITMAP_SC);
            case SvImageId::Calc:
                return OUString(BMP_CALC_SC);
            case SvImageId::CalcTemplate:
                return OUString(BMP_CALCTEMPLATE_SC);
            case SvImageId::Database:
                return OUString(BMP_DATABASE_SC);
            case SvImageId::ImpressTemplate:
                return OUString(BMP_IMPRESSTEMPLATE_SC);
            case SvImageId::GIF:
                return OUString(BMP_GIF_SC);
            case SvImageId::HTML:
                return OUString(BMP_HTML_SC);
            case SvImageId::JPG:
                return OUString(BMP_JPG_SC);
            case SvImageId::Math:
                return OUString(BMP_MATH_SC);
            case SvImageId::MathTemplate:
                return OUString(BMP_MATHTEMPLATE_SC);
            case SvImageId::File:
                return OUString(BMP_FILE_SC);
            case SvImageId::PCD:
                return OUString(BMP_PCD_SC);
            case SvImageId::PCT:
                return OUString(BMP_PCT_SC);
            case SvImageId::PCX:
                return OUString(BMP_PCX_SC);
            case SvImageId::SIM:
                return OUString(BMP_SIM_SC);
            case SvImageId::TextFile:
                return OUString(BMP_TEXTFILE_SC);
            case SvImageId::TIFF:
                return OUString(BMP_TIFF_SC);
            case SvImageId::WMF:
                return OUString(BMP_WMF_SC);
            case SvImageId::Writer:
                return OUString(BMP_WRITER_SC);
            case SvImageId::WriterTemplate:
                return OUString(BMP_WRITERTEMPLATE_SC);
            case SvImageId::FixedDevice:
                return OUString(BMP_FIXEDDEV_SC);
            case SvImageId::RemoveableDevice:
                return OUString(BMP_REMOVABLEDEV_SC);
            case SvImageId::CDRomDevice:
                return OUString(BMP_CDROMDEV_SC);
            case SvImageId::NetworkDevice:
                return OUString(BMP_NETWORKDEV_SC);
            case SvImageId::Table:
                return OUString(BMP_TABLE_SC);
            case SvImageId::Folder:
                // if not bBig, then return our new small folder image (256 colors)
                return OUString(RID_BMP_FOLDER);
            case SvImageId::DXF:
                return OUString(BMP_DXF_SC);
            case SvImageId::MET:
                return OUString(BMP_MET_SC);
            case SvImageId::PNG:
                return OUString(BMP_PNG_SC);
            case SvImageId::SVM:
                return OUString(BMP_SVM_SC);
            case SvImageId::GlobalDoc:
                return OUString(BMP_GLOBAL_DOC_SC);
            case SvImageId::Draw:
                return OUString(BMP_DRAW_SC);
            case SvImageId::DrawTemplate:
                return OUString(BMP_DRAWTEMPLATE_SC);
            case SvImageId::OO_DatabaseDoc:
                return OUString(BMP_OO_DATABASE_DOC_SC);
            case SvImageId::OO_DrawDoc:
                return OUString(BMP_OO_DRAW_DOC_SC);
            case SvImageId::OO_MathDoc:
                return OUString(BMP_OO_MATH_DOC_SC);
            case SvImageId::OO_GlobalDoc:
                return OUString(BMP_OO_GLOBAL_DOC_SC);
            case SvImageId::OO_ImpressDoc:
                return OUString(BMP_OO_IMPRESS_DOC_SC);
            case SvImageId::OO_CalcDoc:
                return OUString(BMP_OO_CALC_DOC_SC);
            case SvImageId::OO_WriterDoc:
                return OUString(BMP_OO_WRITER_DOC_SC);
            case SvImageId::OO_DrawTemplate:
                return OUString(BMP_OO_DRAW_TEMPLATE_SC);
            case SvImageId::OO_ImpressTemplate:
                return OUString(BMP_OO_IMPRESS_TEMPLATE_SC);
            case SvImageId::OO_CalcTemplate:
                return OUString(BMP_OO_CALC_TEMPLATE_SC);
            case SvImageId::OO_WriterTemplate:
                return OUString(BMP_OO_WRITER_TEMPLATE_SC);
            case SvImageId::Extension:
                return OUString(BMP_EXTENSION_SC);
            default: break;
        }
    }

    return OUString();
}

static Image GetImageFromList_Impl( SvImageId nImageId, bool bBig )
{
    OUString sImageName(GetImageNameFromList_Impl(nImageId, bBig));
    if (!sImageName.isEmpty())
        return Image(StockImage::Yes, sImageName);
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

OUString SvFileInformationManager::GetImageId(const INetURLObject& rObject, bool bBig)
{
    SvImageId nImage = GetImageId_Impl( rObject, true );
    DBG_ASSERT( nImage != SvImageId::NONE, "invalid ImageId" );
    return GetImageNameFromList_Impl(nImage, bBig);
}

Image SvFileInformationManager::GetImage( const INetURLObject& rObject, bool bBig )
{
    SvImageId nImage = GetImageId_Impl( rObject, true );
    DBG_ASSERT( nImage != SvImageId::NONE, "invalid ImageId" );
    return GetImageFromList_Impl( nImage, bBig );
}

OUString SvFileInformationManager::GetFileImageId(const INetURLObject& rObject)
{
    SvImageId nImage = GetImageId_Impl( rObject, false );
    DBG_ASSERT( nImage != SvImageId::NONE, "invalid ImageId" );
    return GetImageNameFromList_Impl(nImage, /*bBig*/false);
}

Image SvFileInformationManager::GetFileImage( const INetURLObject& rObject )
{
    SvImageId nImage = GetImageId_Impl( rObject, false );
    DBG_ASSERT( nImage != SvImageId::NONE, "invalid ImageId" );
    return GetImageFromList_Impl( nImage, false/*bBig*/ );
}

Image SvFileInformationManager::GetImageNoDefault( const INetURLObject& rObject, bool bBig )
{
    SvImageId nImage = GetImageId_Impl( rObject, true );
    DBG_ASSERT( nImage != SvImageId::NONE, "invalid ImageId" );

    if ( nImage == SvImageId::File )
        return Image();

    return GetImageFromList_Impl( nImage, bBig );
}

Image SvFileInformationManager::GetFolderImage( const svtools::VolumeInfo& rInfo )
{
    SvImageId nImage = SvImageId::Folder;
    DBG_ASSERT( nImage != SvImageId::NONE, "invalid ImageId" );

    if ( rInfo.m_bIsRemote )
        nImage = SvImageId::NetworkDevice;
    else if ( rInfo.m_bIsCompactDisc )
        nImage = SvImageId::CDRomDevice;
    else if ( rInfo.m_bIsRemoveable || rInfo.m_bIsFloppy )
        nImage = SvImageId::RemoveableDevice;
    else if ( rInfo.m_bIsVolume )
        nImage = SvImageId::FixedDevice;

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
