/*************************************************************************
 *
 *  $RCSfile: imagemgr.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: pb $ $Date: 2001-08-23 10:52:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// includes --------------------------------------------------------------

#include "imagemgr.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _SOT_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif

#pragma hdrstop

#if defined( OS2 ) || defined( MAC )
#ifndef _EXTATTR_HXX //autogen
#include "extattr.hxx"
#endif
#endif

#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include <com/sun/star/uno/Reference.h>

#include "svtools.hrc"
#include "imagemgr.hrc"
#include "svtdata.hxx"

// globals *******************************************************************

#define NO_INDEX    ((USHORT)0xFFFF)

static ImageList* _pSmallImageList = NULL;
static ImageList* _pBigImageList = NULL;

struct SvtExtensionResIdMapping_Impl
{
    char*   _pExt;
    BOOL    _bExt;
    USHORT  _nStrId;
    USHORT  _nImgId;
};

static SvtExtensionResIdMapping_Impl __READONLY_DATA ExtensionMap_Impl[] =
{
    "awk",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "bas",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
#ifndef WNT
    "bat",   TRUE,  STR_DESCRIPTION_BATCHFILE,             IMG_APP,
#endif
    "bmk",   FALSE, STR_DESCRIPTION_BOOKMARKFILE,          0,
    "bmp",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_BITMAP,
    "c",     TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "cfg",   FALSE, STR_DESCRIPTION_CFGFILE,               0,
#ifndef WNT
    "cmd",   TRUE,  STR_DESCRIPTION_BATCHFILE,             IMG_APP,
#endif
    "cob",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "com",   TRUE,  STR_DESCRIPTION_APPLICATION,           IMG_APP,
    "cxx",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "dbf",   TRUE,  STR_DESCRIPTION_DATABASE_TABLE,        IMG_TABLE,
    "def",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "dll",   TRUE,  STR_DESCRIPTION_SYSFILE,               0,
    "doc",   FALSE, STR_DESCRIPTION_WORD_DOC,              IMG_WORD,
    "dxf",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_DXF,
#ifndef WNT
    "exe",   TRUE,  STR_DESCRIPTION_APPLICATION,           IMG_APP,
#endif
    "gif",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_GIF,
    "h",     TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "hlp",   FALSE, STR_DESCRIPTION_HELP_DOC,              IMG_HELP,
    "hrc",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "htm",   FALSE, STR_DESCRIPTION_HTMLFILE,              IMG_HTML,
    "html",  FALSE, STR_DESCRIPTION_HTMLFILE,              IMG_HTML,
    "asp",   FALSE, STR_DESCRIPTION_HTMLFILE,              IMG_HTML,
    "hxx",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "ini",   FALSE, STR_DESCRIPTION_CFGFILE,               0,
    "java",  TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "jpeg",  TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_JPG,
    "jpg",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_JPG,
    "lha",   TRUE,  STR_DESCRIPTION_ARCHIVFILE,            0,
#ifdef WNT
    "lnk",   FALSE, 0,                                     0,
#endif
    "log",   TRUE,  STR_DESCRIPTION_LOGFILE,               0,
    "lst",   TRUE,  STR_DESCRIPTION_LOGFILE,               0,
    "met",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_MET,
    "mml",   FALSE, STR_DESCRIPTION_SMATH_DOC,             IMG_MATH,
    "mod",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "pas",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "pcd",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCD,
    "pct",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCT,
    "pcx",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PCX,
    "pl",    TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "png",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_PNG,
    "rar",   TRUE,  STR_DESCRIPTION_ARCHIVFILE,            0,
    "sbl",   FALSE, 0,                                     IMG_MACROLIB,
    "sch",   FALSE, STR_DESCRIPTION_SCHART_DOC,            0,
    "sda",   FALSE, STR_DESCRIPTION_SDRAW_DOC,             IMG_DRAW,
    "sdb",   FALSE, 0,                                     IMG_DATABASE,
    "sdc",   FALSE, STR_DESCRIPTION_SCALC_DOC,             IMG_CALC,
    "sdd",   FALSE, STR_DESCRIPTION_SIMPRESS_DOC,          IMG_IMPRESS,
    "sdp",   FALSE, STR_DESCRIPTION_SIMPRESS_DOC,          IMG_IMPRESSPACKED,
    "sds",   FALSE, STR_DESCRIPTION_SCHART_DOC,            IMG_CHART,
    "sdw",   FALSE, STR_DESCRIPTION_SWRITER_DOC,           IMG_WRITER,
    "sga",   FALSE, 0,                                     IMG_GALLERY,
    "sgf",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SGF,
    "sgl",   FALSE, STR_DESCRIPTION_GLOBALDOC,             IMG_GLOBAL_DOC,
    "sgv",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SGV,
    "shtml", FALSE, STR_DESCRIPTION_HTMLFILE,              IMG_HTML,
    "sim",   FALSE, STR_DESCRIPTION_SIMAGE_DOC,            IMG_SIM,
    "smf",   FALSE, STR_DESCRIPTION_SMATH_DOC,             IMG_MATH,
    "src",   TRUE,  STR_DESCRIPTION_SOURCEFILE,            IMG_TEXTFILE,
    "svh",   FALSE, STR_DESCRIPTION_HELP_DOC,              IMG_SVHELP,
    "svm",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_SVM,
    "stc",   FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC,  IMG_CALCTEMPLATE,
    "sti",   FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC,  IMG_IMPRESSTEMPLATE,
    "stw",   FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC,  IMG_WRITERTEMPLATE,
    "sxc",   FALSE, STR_DESCRIPTION_SCALC_DOC,             IMG_CALC,
    "sxd",   FALSE, STR_DESCRIPTION_SDRAW_DOC,             IMG_DRAW,
    "sxg",   FALSE, STR_DESCRIPTION_GLOBALDOC,             IMG_GLOBAL_DOC,
    "sxi",   FALSE, STR_DESCRIPTION_SIMPRESS_DOC,          IMG_IMPRESS,
    "sxm",   FALSE, STR_DESCRIPTION_SMATH_DOC,             IMG_MATH,
    "sxs",   FALSE, STR_DESCRIPTION_SCHART_DOC,            IMG_CHART,
    "sxw",   FALSE, STR_DESCRIPTION_SWRITER_DOC,           IMG_WRITER,
    "sys",   TRUE,  STR_DESCRIPTION_SYSFILE,               0,
    "tif",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_TIFF,
    "tiff",  TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_TIFF,
    "txt",   FALSE, STR_DESCRIPTION_TEXTFILE,              IMG_TEXTFILE,
    "url",   FALSE, STR_DESCRIPTION_LINK,                  IMG_HTML,
    "vor",   FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC,  IMG_WRITERTEMPLATE,
    "vxd",   TRUE,  STR_DESCRIPTION_SYSFILE,               0,
    "wmf",   TRUE,  STR_DESCRIPTION_GRAPHIC_DOC,           IMG_WMF,
    "xls",   FALSE, STR_DESCRIPTION_EXCEL_DOC,             IMG_EXCEL,
    "xlt",   FALSE, STR_DESCRIPTION_EXCEL_TEMPLATE_DOC,    IMG_EXCELTEMPLATE,
    "uu",    TRUE,  STR_DESCRIPTION_ARCHIVFILE,            0,
    "uue",   TRUE,  STR_DESCRIPTION_ARCHIVFILE,            0,
    "z",     TRUE,  STR_DESCRIPTION_ARCHIVFILE,            0,
    "zip",   TRUE,  STR_DESCRIPTION_ARCHIVFILE,            0,
    "zoo",   TRUE,  STR_DESCRIPTION_ARCHIVFILE,            0,
    "gz",    TRUE,  STR_DESCRIPTION_ARCHIVFILE,            0,
    0, FALSE, 0, 0
};

#ifdef MAC
    SvtExtensionResIdMapping_Impl Mappings[] =
    {
        "SW/©0", FALSE, STR_DESCRIPTION_SWRITER_DOC,    IMG_WRITER,
        "SVsa0", FALSE, STR_DESCRIPTION_SDRAW_DOC,      IMG_DRAW,
        "SVsd0", FALSE, STR_DESCRIPTION_SIMPRESS_DOC,   IMG_IMPRESS,
        "SVsc0", FALSE, STR_DESCRIPTION_SCALC_DOC,      IMG_CALC,
        "SVch0", FALSE, STR_DESCRIPTION_SCHART_DOC,     IMG_CHART,
        "SVsm0", FALSE, STR_DESCRIPTION_SMATH_DOC,      IMG_MATH,
        "sW/©1", FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC, IMG_WRITERTEMPLATE,
        "sVsa1", FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC, IMG_DRAWTEMPLATE,
        "sVsd1", FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC, IMG_IMPRESSTEMPLATE,
        "sVsc1", FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC, IMG_CALCTEMPLATE,
        "sVsm1", FALSE, STR_DESCRIPTION_SOFFICE_TEMPLATE_DOC, IMG_MATHTEMPLATE,
        "BMPp0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_BITMAP,
        "DXFl0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_SIM,
        "GIFf0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_GIF,
        "JPEG0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_JPG,
        "MET 0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_SIM,
        "PCD 0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_PCD,
        "PICT0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_PCT,
        "PCX 0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_PCX,
        "PNG 0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_SIM,
        "SVM 0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_SIM,
        "TIFF0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_TIFF,
        "WMF 0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_WMF,
        "XBM 0", FALSE, STR_DESCRIPTION_GRAPHIC_DOC,    IMG_BITMAP,
        NULL, 0
    };
#endif
#ifdef OS2
    SvtExtensionResIdMapping_Impl Mappings[] =
    {
        "StarWriter 4.0",           FALSE, STR_DESCRIPTION_SWRITER_DOC, IMG_WRITER,
        "StarWriter 3.0",           FALSE, STR_DESCRIPTION_SWRITER_DOC, IMG_WRITER,
        "StarCalc 4.0",             FALSE, STR_DESCRIPTION_SCALC_DOC,   IMG_CALC,
        "StarCalc 3.0",             FALSE, STR_DESCRIPTION_SCALC_DOC,   IMG_CALC,
        "StarImpress 4.0",          FALSE, STR_DESCRIPTION_SIMPRESS_DOC,IMG_IMPRESS,
        "StarDraw 4.0",             FALSE, STR_DESCRIPTION_SDRAW_DOC,   IMG_DRAW,
        "StarDraw 3.0",             FALSE, STR_DESCRIPTION_SDRAW_DOC,   IMG_DRAW,
        "StarChart 3.0",            FALSE, STR_DESCRIPTION_SCHART_DOC,  IMG_CHART,
        "StarChart 4.0",            FALSE, STR_DESCRIPTION_SCHART_DOC,  IMG_CHART,
        "Bitmap",                   FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_BITMAP,
        "AutoCAD",                  FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_SIM,
        "Gif-File",                 FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_GIF,
        "JPEG-File",                FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_JPG,
        "Metafile ",                FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_SIM,
        "Photo-CD ",                FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_PCD,
        "Mac Pict",                 FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_PCT,
        "PCX-File ",                FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_PCX,
        "PNG-File",                 FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_SIM,
        "SV-Metafile",              FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_SIM,
        "TIFF-File",                FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_TIFF,
        "MS-Metafile",              FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_WMF,
        "XBM-File",                 FALSE, STR_DESCRIPTION_GRAPHIC_DOC, IMG_BITMAP,
        "UniformResourceLocator",   FALSE, STR_DESCRIPTION_LINK,        IMG_URL,
        NULL, 0
    };
#endif

//****************************************************************************

String GetImageExtensionByFactory_Impl( const String& rURL )
{
    String aExtension;

    try
    {
        // get the TypeDetection service to access all registered types
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >  xFac = ::comphelper::getProcessServiceFactory();
        ::com::sun::star::uno::Reference < ::com::sun::star::document::XTypeDetection > xTypeDetector(
            xFac->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.TypeDetection") ) ), ::com::sun::star::uno::UNO_QUERY );

        ::rtl::OUString aInternalType = xTypeDetector->queryTypeByURL( rURL );
        ::com::sun::star::uno::Reference < ::com::sun::star::container::XNameAccess > xAccess( xTypeDetector, ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aTypeProps;
        if ( aInternalType.getLength() > 0 && xAccess->hasByName( aInternalType ) )
        {
            xAccess->getByName( aInternalType ) >>= aTypeProps;
            sal_Int32 nProps = aTypeProps.getLength();
            for ( sal_Int32 i = 0; i < nProps; ++i )
            {
                const ::com::sun::star::beans::PropertyValue& rProp = aTypeProps[i];
                if ( rProp.Name.compareToAscii("Extensions") == COMPARE_EQUAL )
                {
                    ::com::sun::star::uno::Sequence < ::rtl::OUString > aExtensions;
                    if ( ( rProp.Value >>= aExtensions ) && aExtensions.getLength() > 0 )
                    {
                        const ::rtl::OUString* pExtensions = aExtensions.getConstArray();
                        aExtension = String( pExtensions[0] );
                        break;
                    }
                }
            }
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        // type detection failed -> no extension
    }

    return aExtension;
}

USHORT GetIndexOfExtension_Impl( const String& rExtension )
{
    USHORT nRet = NO_INDEX;
    if ( rExtension.Len() )
    {
        USHORT nIndex = 0;
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

USHORT GetImageId_Impl( const String& rExtension )
{
    USHORT nImage = IMG_FILE;
    if ( rExtension.Len()  != NO_INDEX )
    {
        USHORT nIndex = GetIndexOfExtension_Impl( rExtension );
        if ( nIndex != NO_INDEX )
        {
            nImage = ExtensionMap_Impl[ nIndex ]._nImgId;
            if ( !nImage )
                nImage = IMG_FILE;
        }
    }

    return nImage;
}

USHORT GetImageId_Impl( const INetURLObject& rObject )
{
    String aExt;
    USHORT nImage = IMG_FILE;

    if ( rObject.GetProtocol() == INET_PROT_PRIVATE )
    {
        String aURLPath = rObject.GetMainURL().Erase( 0, URL_PREFIX_PRIV_SOFFICE_LEN );
        String aType = aURLPath.GetToken( 0, INET_PATH_TOKEN );
        if ( aType == String( RTL_CONSTASCII_STRINGPARAM("factory") ) )
            aExt = GetImageExtensionByFactory_Impl( rObject.GetMainURL() );
        else if ( aType == String( RTL_CONSTASCII_STRINGPARAM("image") ) )
        {
            nImage = (USHORT)aURLPath.GetToken( 1, INET_PATH_TOKEN ).ToInt32();
        }
    }
    else
    {
        aExt = rObject.getExtension();
        if ( aExt.EqualsAscii( "vor" ) )
        {
            SotStorageRef aStorage = new SotStorage( rObject.GetMainURL(), STREAM_STD_READ );
            USHORT nId = IMG_WRITERTEMPLATE;
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

#if defined( OS2 ) || defined( MAC )
    if ( nImage == IMG_FILE )
    {
        SvEaMgr aMgr( rObject.GetMainURL() );
        String aType;
        if( aMgr.GetFileType( aType ) )
        {
            for( USHORT nIndex = 0; Mappings[ nIndex ]._pExt; nIndex++ )
                if ( Mappings[ nIndex ]._pExt == aType )
                    nImage = Mappings[ nIndex ]._nImgId;
        }
    }
#endif

    if ( nImage == IMG_FILE )
        nImage = GetImageId_Impl( aExt );
    return nImage;
}

USHORT GetDescriptionId_Impl( const String& rExtension, sal_Bool& rbShowExt )
{
    USHORT nId = 0;

    if ( rExtension.Len()  != NO_INDEX )
    {
        USHORT nIndex = GetIndexOfExtension_Impl( rExtension );
        if ( nIndex != NO_INDEX )
        {
            nId = ExtensionMap_Impl[ nIndex ]._nStrId;
            rbShowExt = ExtensionMap_Impl[ nIndex ]._bExt;
        }
    }

    return nId;
}

String GetDescriptionByFactory_Impl( const String& rFactory )
{
    USHORT nResId = 0;
    if ( rFactory.EqualsIgnoreCaseAscii( "swriter" ) )
        nResId = STR_DESCRIPTION_FACTORY_WRITER;
    else if ( rFactory.EqualsIgnoreCaseAscii( "scalc" ) )
        nResId = STR_DESCRIPTION_FACTORY_CALC;
    else if ( rFactory.EqualsIgnoreCaseAscii( "simpress" ) )
        nResId = STR_DESCRIPTION_FACTORY_IMPRESS;
    else if ( rFactory.EqualsIgnoreCaseAscii( "sdraw" ) )
        nResId = STR_DESCRIPTION_FACTORY_DRAW;
    else if ( rFactory.EqualsIgnoreCaseAscii( "swriter/web" ) )
        nResId = STR_DESCRIPTION_FACTORY_WRITERWEB;
    else if ( rFactory.EqualsIgnoreCaseAscii( "swriter/globaldocument" ) )
        nResId = STR_DESCRIPTION_FACTORY_GLOBALDOC;
    else if ( rFactory.EqualsIgnoreCaseAscii( "smath" ) )
        nResId = STR_DESCRIPTION_FACTORY_MATH;

    String aRet;
    if ( nResId )
        aRet = String( SvtResId( nResId ) );
    return aRet;
}

//****************************************************************************


Image SvFileInformationManager::GetImage( const INetURLObject& rObject, BOOL bBig )
{
    USHORT nImage = GetImageId_Impl( rObject );
    DBG_ASSERT( nImage, "invalid ImageId" );

    ImageList* pList = NULL;
    if ( bBig )
    {
        if ( !_pBigImageList )
            _pBigImageList = new ImageList( SvtResId( RID_SVTOOLS_IMAGELIST_BIG ) );
        pList = _pBigImageList;
    }
    else
    {
        if ( !_pSmallImageList )
            _pSmallImageList = new ImageList( SvtResId( RID_SVTOOLS_IMAGELIST_SMALL ) );
        pList = _pSmallImageList;
    }

    return pList->GetImage( nImage );
}

String SvFileInformationManager::GetDescription( const INetURLObject& rObject )
{
    String aDescription( SvtResId( STR_DESCRIPTION_FOLDER ) );
    sal_Bool bShowExt = sal_False;
    sal_Bool bDetected = sal_False;
    sal_Bool bFolder = ::utl::UCBContentHelper::IsFolder( rObject.GetMainURL() );
    if ( !bFolder )
    {
#if defined( OS2 ) || defined( MAC )
        // FileType via EAs
        SvEaMgr aMgr( rObject.GetMainURL() );
        String aType;
        if ( aMgr.GetFileType( aType ) )
        {
            for( USHORT nIndex = 0; !bDetected && Mappings[ nIndex ]._pExt; nIndex++ )
                if ( Mappings[ nIndex ]._pExt == aType )
                {
                    aDescription = SfxResId( Mappings[ nIndex ]._nStrId );
                    bDetected = sal_True;
                }
        }
#endif
        if ( !bDetected )
        {
            if ( rObject.GetProtocol() == INET_PROT_PRIVATE )
            {
                String aURLPath = rObject.GetMainURL().Erase( 0, URL_PREFIX_PRIV_SOFFICE_LEN );
                String aType = aURLPath.GetToken( 0, INET_PATH_TOKEN );
                if ( aType == String( RTL_CONSTASCII_STRINGPARAM("factory") ) )
                {
                    aDescription = GetDescriptionByFactory_Impl( aURLPath.Copy( aURLPath.Search( INET_PATH_TOKEN ) + 1 ) );
                    bDetected = sal_True;
                }
            }

            if ( !bDetected )
            {
                // Bestimmung der Dateiendung
                String aExtension( rObject.getExtension() );
                aExtension.ToLowerAscii();

                // Iteration ueber alle Zuordnungen
                USHORT nResId = 0;
                if ( aExtension.Len() )
                    nResId = GetDescriptionId_Impl( aExtension, bShowExt );
                if ( !nResId )
                    nResId = STR_DESCRIPTION_FILE;

                aDescription = String( SvtResId( nResId ) );
            }

            DBG_ASSERT( aDescription.Len() > 0, "file without description" );
        }
    }

    if ( bShowExt )
    {
        aDescription += String( RTL_CONSTASCII_STRINGPARAM(" (") );
        aDescription += rObject.getExtension().ToLowerAscii();
        aDescription += ')';
    }

    return aDescription;
}

