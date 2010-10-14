/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "vbaoptions.hxx"
#include <vbahelper/vbahelper.hxx>
#include <ooo/vba/word/WdDefaultFilePath.hpp>
#include <ooo/vba/word/WdLineStyle.hpp>
#include <ooo/vba/word/WdLineWidth.hpp>
#include <ooo/vba/word/WdColorIndex.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaOptions::SwVbaOptions( uno::Reference<uno::XComponentContext >& xContext ) throw ( uno::RuntimeException ) : SwVbaOptions_BASE( uno::Reference< XHelperInterface >(), xContext )
{
    mxFactory.set( comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
}

SwVbaOptions::~SwVbaOptions()
{
}

uno::Any SAL_CALL
SwVbaOptions::DefaultFilePath( sal_Int32 _path ) throw ( uno::RuntimeException )
{
    switch( _path )
    {
        case word::WdDefaultFilePath::wdDocumentsPath:
        {
            msDefaultFilePath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Work") );
            break;
        }
        case word::WdDefaultFilePath::wdPicturesPath:
        {
            msDefaultFilePath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Gallery") );
            break;
        }
        case word::WdDefaultFilePath::wdUserTemplatesPath:
        case word::WdDefaultFilePath::wdWorkgroupTemplatesPath:
        {
            msDefaultFilePath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Template") );
            break;
        }
        case word::WdDefaultFilePath::wdStartupPath:
        {
            msDefaultFilePath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Addin") );
            break;
        }
        case word::WdDefaultFilePath::wdUserOptionsPath:
        {
            msDefaultFilePath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("UserConfig") );
            break;
        }
        case word::WdDefaultFilePath::wdToolsPath:
        case word::WdDefaultFilePath::wdProgramPath:
        {
            msDefaultFilePath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Module") );
            break;
        }
        case word::WdDefaultFilePath::wdTempFilePath:
        {
            msDefaultFilePath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Temp") );
            break;
        }
        default:
        {
            DebugHelper::exception( SbERR_NOT_IMPLEMENTED, rtl::OUString() );
            break;
        }
    }
    return uno::makeAny( uno::Reference< XPropValue > ( new ScVbaPropValue( this ) ) );
}

void SwVbaOptions::setValueEvent( const uno::Any& value )
{
    rtl::OUString sNewPath;
    value >>= sNewPath;
    rtl::OUString sNewPathUrl;
    ::osl::File::getFileURLFromSystemPath( sNewPath, sNewPathUrl );
    uno::Reference< beans::XPropertySet > xPathSettings( mxFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.PathSettings") ), uno::UNO_QUERY_THROW );
    rtl::OUString sOldPathUrl;
    xPathSettings->getPropertyValue( msDefaultFilePath ) >>= sOldPathUrl;
    // path could be a multipath, Microsoft doesn't support this feature in Word currently
    // only the last path is from interest.
    sal_Int32 nIndex = sOldPathUrl.lastIndexOf( sal_Unicode(';') );
    if( nIndex != -1 )
    {
        sNewPathUrl = sOldPathUrl.copy( 0, nIndex + 1 ).concat( sNewPathUrl );
    }
    xPathSettings->setPropertyValue( msDefaultFilePath, uno::makeAny( sNewPathUrl ) );
}

uno::Any SwVbaOptions::getValueEvent()
{
    uno::Reference< beans::XPropertySet > xPathSettings( mxFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.PathSettings") ), uno::UNO_QUERY_THROW );
    rtl::OUString sPathUrl;
    xPathSettings->getPropertyValue( msDefaultFilePath ) >>= sPathUrl;
    // path could be a multipath, Microsoft doesn't support this feature in Word currently
    // only the last path is from interest.
    sal_Int32 nIndex = sPathUrl.lastIndexOf( sal_Unicode(';') );
    if( nIndex != -1 )
    {
        sPathUrl = sPathUrl.copy( nIndex + 1 );
    }
    rtl::OUString sPath;
    ::osl::File::getSystemPathFromFileURL( sPathUrl, sPath );
    return uno::makeAny( sPath );
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderLineStyle() throw (uno::RuntimeException)
{
    return word::WdLineStyle::wdLineStyleSingle;
}

void SAL_CALL SwVbaOptions::setDefaultBorderLineStyle( ::sal_Int32 /*_defaultborderlinestyle*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderLineWidth() throw (uno::RuntimeException)
{
    return word::WdLineWidth::wdLineWidth050pt;
}

void SAL_CALL SwVbaOptions::setDefaultBorderLineWidth( ::sal_Int32 /*_defaultborderlinewidth*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

sal_Int32 SAL_CALL SwVbaOptions::getDefaultBorderColorIndex() throw (uno::RuntimeException)
{
    return word::WdColorIndex::wdAuto;
}

void SAL_CALL SwVbaOptions::setDefaultBorderColorIndex( ::sal_Int32 /*_defaultbordercolorindex*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getReplaceSelection() throw (uno::RuntimeException)
{
    return sal_True;
}

void SAL_CALL SwVbaOptions::setReplaceSelection( ::sal_Bool /*_replaceselection*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getMapPaperSize() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setMapPaperSize( ::sal_Bool /*_mappapersize*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyHeadings() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyHeadings( ::sal_Bool /*_autoformatasyoutypeapplyheadings*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyBulletedLists() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyBulletedLists( ::sal_Bool /*_autoformatasyoutypeapplybulletedlists*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeApplyNumberedLists() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeApplyNumberedLists( ::sal_Bool /*_autoformatasyoutypeapplynumberedlists*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeFormatListItemBeginning() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeFormatListItemBeginning( ::sal_Bool /*_autoformatasyoutypeformatlistitembeginning*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getAutoFormatAsYouTypeDefineStyles() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatAsYouTypeDefineStyles( ::sal_Bool /*_autoformatasyoutypedefinestyles*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyHeadings() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyHeadings( ::sal_Bool /*_autoformatapplyheadings*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyLists() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyLists( ::sal_Bool /*_autoformatapplylists*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaOptions::getAutoFormatApplyBulletedLists() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SwVbaOptions::setAutoFormatApplyBulletedLists( ::sal_Bool /*_autoformatapplybulletedlists*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}


rtl::OUString&
SwVbaOptions::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaOptions") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaOptions::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Options" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
