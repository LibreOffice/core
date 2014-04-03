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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaOptions::SwVbaOptions( uno::Reference<uno::XComponentContext >& xContext ) : SwVbaOptions_BASE( uno::Reference< XHelperInterface >(), xContext )
{
}

SwVbaOptions::~SwVbaOptions()
{
}

uno::Any SAL_CALL
SwVbaOptions::DefaultFilePath( sal_Int32 _path ) throw ( uno::RuntimeException, std::exception )
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
            DebugHelper::exception( SbERR_NOT_IMPLEMENTED, OUString() );
            break;
        }
    }
    return uno::makeAny( uno::Reference< XPropValue > ( new ScVbaPropValue( this ) ) );
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
        sNewPathUrl = sOldPathUrl.copy( 0, nIndex + 1 ).concat( sNewPathUrl );
    }
    xPathSettings->setPropertyValue( msDefaultFilePath, uno::makeAny( sNewPathUrl ) );
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
    return uno::makeAny( sPath );
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderLineStyle() throw (uno::RuntimeException, std::exception)
{
    return word::WdLineStyle::wdLineStyleSingle;
}

void SAL_CALL SwVbaOptions::setDefaultBorderLineStyle( ::sal_Int32 /*_defaultborderlinestyle*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderLineWidth() throw (uno::RuntimeException, std::exception)
{
    return word::WdLineWidth::wdLineWidth050pt;
}

void SAL_CALL SwVbaOptions::setDefaultBorderLineWidth( ::sal_Int32 /*_defaultborderlinewidth*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderColorIndex() throw (uno::RuntimeException, std::exception)
{
    return word::WdColorIndex::wdAuto;
}

void SAL_CALL SwVbaOptions::setDefaultBorderColorIndex( ::sal_Int32 /*_defaultbordercolorindex*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getReplaceSelection() throw (uno::RuntimeException, std::exception)
{
    return sal_True;
}

void SAL_CALL SwVbaOptions::setReplaceSelection( sal_Bool /*_replaceselection*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getMapPaperSize() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setMapPaperSize( sal_Bool /*_mappapersize*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyHeadings() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyHeadings( sal_Bool /*_autoformatasyoutypeapplyheadings*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyBulletedLists() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyBulletedLists( sal_Bool /*_autoformatasyoutypeapplybulletedlists*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyNumberedLists() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyNumberedLists( sal_Bool /*_autoformatasyoutypeapplynumberedlists*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeFormatListItemBeginning() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeFormatListItemBeginning( sal_Bool /*_autoformatasyoutypeformatlistitembeginning*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeDefineStyles() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeDefineStyles( sal_Bool /*_autoformatasyoutypedefinestyles*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyHeadings() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyHeadings( sal_Bool /*_autoformatapplyheadings*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyLists() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyLists( sal_Bool /*_autoformatapplylists*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyBulletedLists() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyBulletedLists( sal_Bool /*_autoformatapplybulletedlists*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

OUString
SwVbaOptions::getServiceImplName()
{
    return OUString("SwVbaOptions");
}

uno::Sequence< OUString >
SwVbaOptions::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Options";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
