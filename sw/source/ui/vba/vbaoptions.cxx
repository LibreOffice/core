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
#include "vbaoptions.hxx"
#include <vbahelper/vbahelper.hxx>
#include <ooo/vba/word/WdDefaultFilePath.hpp>
#include <ooo/vba/word/WdLineStyle.hpp>
#include <ooo/vba/word/WdLineWidth.hpp>
#include <ooo/vba/word/WdColorIndex.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <comphelper/processfactory.hxx>
#include <basic/sberrors.hxx>
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaOptions::SwVbaOptions( uno::Reference<uno::XComponentContext > const & xContext ) : SwVbaOptions_BASE( uno::Reference< XHelperInterface >(), xContext )
{
}

SwVbaOptions::~SwVbaOptions()
{
}

uno::Any SAL_CALL
SwVbaOptions::DefaultFilePath( sal_Int32 _path )
{
    switch( _path )
    {
        case word::WdDefaultFilePath::wdDocumentsPath:
        {
            msDefaultFilePath = "Work";
            break;
        }
        case word::WdDefaultFilePath::wdPicturesPath:
        {
            msDefaultFilePath = "Gallery";
            break;
        }
        case word::WdDefaultFilePath::wdUserTemplatesPath:
        case word::WdDefaultFilePath::wdWorkgroupTemplatesPath:
        {
            msDefaultFilePath = "Template";
            break;
        }
        case word::WdDefaultFilePath::wdStartupPath:
        {
            msDefaultFilePath = "Addin";
            break;
        }
        case word::WdDefaultFilePath::wdUserOptionsPath:
        {
            msDefaultFilePath = "UserConfig";
            break;
        }
        case word::WdDefaultFilePath::wdToolsPath:
        case word::WdDefaultFilePath::wdProgramPath:
        {
            msDefaultFilePath = "Module";
            break;
        }
        case word::WdDefaultFilePath::wdTempFilePath:
        {
            msDefaultFilePath = "Temp";
            break;
        }
        default:
        {
            DebugHelper::basicexception( ERRCODE_BASIC_NOT_IMPLEMENTED, {} );
            break;
        }
    }
    return uno::Any( uno::Reference< XPropValue > ( new ScVbaPropValue( this ) ) );
}

void SwVbaOptions::setValueEvent( const uno::Any& value )
{
    OUString sNewPath;
    value >>= sNewPath;
    OUString sNewPathUrl;
    ::osl::File::getFileURLFromSystemPath( sNewPath, sNewPathUrl );
    uno::Reference< util::XPathSettings > xPathSettings = util::thePathSettings::get( comphelper::getProcessComponentContext() );
    OUString sOldPathUrl;
    xPathSettings->getPropertyValue( msDefaultFilePath ) >>= sOldPathUrl;
    // path could be a multipath, Microsoft doesn't support this feature in Word currently
    // only the last path is from interest.
    sal_Int32 nIndex = sOldPathUrl.lastIndexOf( ';' );
    if( nIndex != -1 )
    {
        sNewPathUrl = sOldPathUrl.subView( 0, nIndex + 1 ) + sNewPathUrl;
    }
    xPathSettings->setPropertyValue( msDefaultFilePath, uno::Any( sNewPathUrl ) );
}

uno::Any SwVbaOptions::getValueEvent()
{
    uno::Reference< util::XPathSettings > xPathSettings = util::thePathSettings::get( comphelper::getProcessComponentContext() );
    OUString sPathUrl;
    xPathSettings->getPropertyValue( msDefaultFilePath ) >>= sPathUrl;
    // path could be a multipath, Microsoft doesn't support this feature in Word currently
    // only the last path is from interest.
    sal_Int32 nIndex = sPathUrl.lastIndexOf( ';' );
    if( nIndex != -1 )
    {
        sPathUrl = sPathUrl.copy( nIndex + 1 );
    }
    OUString sPath;
    ::osl::File::getSystemPathFromFileURL( sPathUrl, sPath );
    return uno::Any( sPath );
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderLineStyle()
{
    return word::WdLineStyle::wdLineStyleSingle;
}

void SAL_CALL SwVbaOptions::setDefaultBorderLineStyle( ::sal_Int32 /*_defaultborderlinestyle*/ )
{
    // not support in Writer
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderLineWidth()
{
    return word::WdLineWidth::wdLineWidth050pt;
}

void SAL_CALL SwVbaOptions::setDefaultBorderLineWidth( ::sal_Int32 /*_defaultborderlinewidth*/ )
{
    // not support in Writer
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderColorIndex()
{
    return word::WdColorIndex::wdAuto;
}

void SAL_CALL SwVbaOptions::setDefaultBorderColorIndex( ::sal_Int32 /*_defaultbordercolorindex*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getReplaceSelection()
{
    return true;
}

void SAL_CALL SwVbaOptions::setReplaceSelection( sal_Bool /*_replaceselection*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getMapPaperSize()
{
    return false;
}

void SAL_CALL SwVbaOptions::setMapPaperSize( sal_Bool /*_mappapersize*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyHeadings()
{
    return false;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyHeadings( sal_Bool /*_autoformatasyoutypeapplyheadings*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyBulletedLists()
{
    return false;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyBulletedLists( sal_Bool /*_autoformatasyoutypeapplybulletedlists*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyNumberedLists()
{
    return false;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyNumberedLists( sal_Bool /*_autoformatasyoutypeapplynumberedlists*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeFormatListItemBeginning()
{
    return false;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeFormatListItemBeginning( sal_Bool /*_autoformatasyoutypeformatlistitembeginning*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeDefineStyles()
{
    return false;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeDefineStyles( sal_Bool /*_autoformatasyoutypedefinestyles*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyHeadings()
{
    return false;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyHeadings( sal_Bool /*_autoformatapplyheadings*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyLists()
{
    return false;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyLists( sal_Bool /*_autoformatapplylists*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyBulletedLists()
{
    return false;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyBulletedLists( sal_Bool /*_autoformatapplybulletedlists*/ )
{
    // not support in Writer
}

OUString
SwVbaOptions::getServiceImplName()
{
    return u"SwVbaOptions"_ustr;
}

uno::Sequence< OUString >
SwVbaOptions::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Options"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
