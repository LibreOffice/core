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

#include "workbooksettings.hxx"

#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <comphelper/mediadescriptor.hxx>
#include "oox/core/filterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/token/properties.hxx"
#include "biffinputstream.hxx"
#include "unitconverter.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

using ::comphelper::MediaDescriptor;
using ::oox::core::CodecHelper;

// ============================================================================

namespace {

const sal_uInt32 BIFF12_WORKBOOKPR_DATE1904     = 0x00000001;
const sal_uInt32 BIFF12_WORKBOOKPR_STRIPEXT     = 0x00000080;

const sal_uInt16 BIFF12_CALCPR_A1               = 0x0002;
const sal_uInt16 BIFF12_CALCPR_ITERATE          = 0x0004;
const sal_uInt16 BIFF12_CALCPR_FULLPRECISION    = 0x0008;
const sal_uInt16 BIFF12_CALCPR_CALCCOMPLETED    = 0x0010;
const sal_uInt16 BIFF12_CALCPR_CALCONSAVE       = 0x0020;
const sal_uInt16 BIFF12_CALCPR_CONCURRENT       = 0x0040;
const sal_uInt16 BIFF12_CALCPR_MANUALPROC       = 0x0080;

// no predefined constants for show objects mode
const sal_Int16 API_SHOWMODE_SHOW               = 0;        /// Show drawing objects.
const sal_Int16 API_SHOWMODE_HIDE               = 1;        /// Hide drawing objects.
const sal_Int16 API_SHOWMODE_PLACEHOLDER        = 2;        /// Show placeholders for drawing objects.

} // namespace

// ============================================================================

FileSharingModel::FileSharingModel() :
    mnPasswordHash( 0 ),
    mbRecommendReadOnly( false )
{
}

// ============================================================================

WorkbookSettingsModel::WorkbookSettingsModel() :
    mnShowObjectMode( XML_all ),
    mnUpdateLinksMode( XML_userSet ),
    mnDefaultThemeVer( -1 ),
    mbDateMode1904( false ),
    mbSaveExtLinkValues( true )
{
}

void WorkbookSettingsModel::setBiffObjectMode( sal_uInt16 nObjMode )
{
    static const sal_Int32 spnObjModes[] = { XML_all, XML_placeholders, XML_none };
    mnShowObjectMode = STATIC_ARRAY_SELECT( spnObjModes, nObjMode, XML_all );
}

// ============================================================================

CalcSettingsModel::CalcSettingsModel() :
    mfIterateDelta( 0.001 ),
    mnCalcId( -1 ),
    mnRefMode( XML_A1 ),
    mnCalcMode( XML_auto ),
    mnIterateCount( 100 ),
    mnProcCount( -1 ),
    mbCalcOnSave( true ),
    mbCalcCompleted( true ),
    mbFullPrecision( true ),
    mbIterate( false ),
    mbConcurrent( true ),
    mbUseNlr( false )
{
}

// ============================================================================

WorkbookSettings::WorkbookSettings( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void WorkbookSettings::importFileSharing( const AttributeList& rAttribs )
{
    maFileSharing.maUserName          = rAttribs.getXString( XML_userName, OUString() );
    maFileSharing.mnPasswordHash      = CodecHelper::getPasswordHash( rAttribs, XML_reservationPassword );
    maFileSharing.mbRecommendReadOnly = rAttribs.getBool( XML_readOnlyRecommended, false );
}

void WorkbookSettings::importWorkbookPr( const AttributeList& rAttribs )
{
    maBookSettings.maCodeName          = rAttribs.getString( XML_codeName, OUString() );
    maBookSettings.mnShowObjectMode    = rAttribs.getToken( XML_showObjects, XML_all );
    maBookSettings.mnUpdateLinksMode   = rAttribs.getToken( XML_updateLinks, XML_userSet );
    maBookSettings.mnDefaultThemeVer   = rAttribs.getInteger( XML_defaultThemeVersion, -1 );
    maBookSettings.mbSaveExtLinkValues = rAttribs.getBool( XML_saveExternalLinkValues, true );
    setDateMode( rAttribs.getBool( XML_date1904, false ), rAttribs.getBool( XML_dateCompatibility, true ) );
}

void WorkbookSettings::importCalcPr( const AttributeList& rAttribs )
{
    maCalcSettings.mfIterateDelta  = rAttribs.getDouble( XML_iterateDelta, 0.0001 );
    maCalcSettings.mnCalcId        = rAttribs.getInteger( XML_calcId, -1 );
    maCalcSettings.mnRefMode       = rAttribs.getToken( XML_refMode, XML_A1 );
    maCalcSettings.mnCalcMode      = rAttribs.getToken( XML_calcMode, XML_auto );
    maCalcSettings.mnIterateCount  = rAttribs.getInteger( XML_iterateCount, 100 );
    maCalcSettings.mnProcCount     = rAttribs.getInteger( XML_concurrentManualCount, -1 );
    maCalcSettings.mbCalcOnSave    = rAttribs.getBool( XML_calcOnSave, true );
    maCalcSettings.mbCalcCompleted = rAttribs.getBool( XML_calcCompleted, true );
    maCalcSettings.mbFullPrecision = rAttribs.getBool( XML_fullPrecision, true );
    maCalcSettings.mbIterate       = rAttribs.getBool( XML_iterate, false );
    maCalcSettings.mbConcurrent    = rAttribs.getBool( XML_concurrentCalc, true );
}

void WorkbookSettings::importFileSharing( SequenceInputStream& rStrm )
{
    maFileSharing.mbRecommendReadOnly = rStrm.readuInt16() != 0;
    rStrm >> maFileSharing.mnPasswordHash >> maFileSharing.maUserName;
}

void WorkbookSettings::importWorkbookPr( SequenceInputStream& rStrm )
{
    sal_uInt32 nFlags;
    rStrm >> nFlags >> maBookSettings.mnDefaultThemeVer >> maBookSettings.maCodeName;
    maBookSettings.setBiffObjectMode( extractValue< sal_uInt16 >( nFlags, 13, 2 ) );
    // set flag means: strip external link values
    maBookSettings.mbSaveExtLinkValues = !getFlag( nFlags, BIFF12_WORKBOOKPR_STRIPEXT );
    setDateMode( getFlag( nFlags, BIFF12_WORKBOOKPR_DATE1904 ) );
}

void WorkbookSettings::importCalcPr( SequenceInputStream& rStrm )
{
    sal_Int32 nCalcMode, nProcCount;
    sal_uInt16 nFlags;
    rStrm >> maCalcSettings.mnCalcId >> nCalcMode >> maCalcSettings.mnIterateCount >> maCalcSettings.mfIterateDelta >> nProcCount >> nFlags;

    static const sal_Int32 spnCalcModes[] = { XML_manual, XML_auto, XML_autoNoTable };
    maCalcSettings.mnRefMode       = getFlagValue( nFlags, BIFF12_CALCPR_A1, XML_A1, XML_R1C1 );
    maCalcSettings.mnCalcMode      = STATIC_ARRAY_SELECT( spnCalcModes, nCalcMode, XML_auto );
    maCalcSettings.mnProcCount     = getFlagValue< sal_Int32 >( nFlags, BIFF12_CALCPR_MANUALPROC, nProcCount, -1 );
    maCalcSettings.mbCalcOnSave    = getFlag( nFlags, BIFF12_CALCPR_CALCONSAVE );
    maCalcSettings.mbCalcCompleted = getFlag( nFlags, BIFF12_CALCPR_CALCCOMPLETED );
    maCalcSettings.mbFullPrecision = getFlag( nFlags, BIFF12_CALCPR_FULLPRECISION );
    maCalcSettings.mbIterate       = getFlag( nFlags, BIFF12_CALCPR_ITERATE );
    maCalcSettings.mbConcurrent    = getFlag( nFlags, BIFF12_CALCPR_CONCURRENT );
}

void WorkbookSettings::finalizeImport()
{
    // default settings
    PropertySet aPropSet( getDocument() );
    switch( getFilterType() )
    {
        case FILTER_OOXML:
        case FILTER_BIFF:
            aPropSet.setProperty( PROP_IgnoreCase,          true );     // always in Excel
            aPropSet.setProperty( PROP_RegularExpressions,  false );    // not supported in Excel
        break;
        case FILTER_UNKNOWN:
        break;
    }

    // write protection
    if( maFileSharing.mbRecommendReadOnly || (maFileSharing.mnPasswordHash != 0) ) try
    {
        getBaseFilter().getMediaDescriptor()[ "ReadOnly" ] <<= true;

        Reference< XPropertySet > xDocumentSettings( getBaseFilter().getModelFactory()->createInstance(
            "com.sun.star.document.Settings" ), UNO_QUERY_THROW );
        PropertySet aSettingsProp( xDocumentSettings );
        if( maFileSharing.mbRecommendReadOnly )
            aSettingsProp.setProperty( PROP_LoadReadonly, true );
//        if( maFileSharing.mnPasswordHash != 0 )
//            aSettingsProp.setProperty( PROP_ModifyPasswordHash, static_cast< sal_Int32 >( maFileSharing.mnPasswordHash ) );
    }
    catch( Exception& )
    {
    }

    // calculation settings
    css::util::Date aNullDate = getNullDate();

    aPropSet.setProperty( PROP_NullDate,           aNullDate );
    aPropSet.setProperty( PROP_IsIterationEnabled, maCalcSettings.mbIterate );
    aPropSet.setProperty( PROP_IterationCount,     maCalcSettings.mnIterateCount );
    aPropSet.setProperty( PROP_IterationEpsilon,   maCalcSettings.mfIterateDelta );
    aPropSet.setProperty( PROP_CalcAsShown,        !maCalcSettings.mbFullPrecision );
    aPropSet.setProperty( PROP_LookUpLabels,       maCalcSettings.mbUseNlr );

    Reference< XNumberFormatsSupplier > xNumFmtsSupp( getDocument(), UNO_QUERY );
    if( xNumFmtsSupp.is() )
    {
        PropertySet aNumFmtProp( xNumFmtsSupp->getNumberFormatSettings() );
        aNumFmtProp.setProperty( PROP_NullDate, aNullDate );
    }

    Reference< XCalculatable > xCalculatable( getDocument(), UNO_QUERY );
    if( xCalculatable.is() )
        xCalculatable->enableAutomaticCalculation( (maCalcSettings.mnCalcMode == XML_auto) || (maCalcSettings.mnCalcMode == XML_autoNoTable) );

    // VBA code name
    aPropSet.setProperty( PROP_CodeName, maBookSettings.maCodeName );
}

sal_Int16 WorkbookSettings::getApiShowObjectMode() const
{
    switch( maBookSettings.mnShowObjectMode )
    {
        case XML_all:           return API_SHOWMODE_SHOW;
        case XML_none:          return API_SHOWMODE_HIDE;
        // #i80528# placeholders not supported anymore, but this is handled internally in Calc
        case XML_placeholders:  return API_SHOWMODE_PLACEHOLDER;
    }
    return API_SHOWMODE_SHOW;
}

css::util::Date WorkbookSettings::getNullDate() const
{
    static const css::util::Date saDate1900                 ( 30, 12, 1899 );
    static const css::util::Date saDate1904                 ( 1, 1, 1904 );
    static const css::util::Date saDateBackCompatibility1900( 31, 12, 1899 );

    if( getOoxFilter().getVersion() == oox::core::ISOIEC_29500_2008 )
    {
        if( !maBookSettings.mbDateCompatibility )
            return saDate1900;

        return maBookSettings.mbDateMode1904 ? saDate1904 :
                                               saDateBackCompatibility1900;
    }

    return maBookSettings.mbDateMode1904 ? saDate1904 : saDate1900;
}

void WorkbookSettings::setDateMode( bool bDateMode1904, bool bDateCompatibility )
{
    maBookSettings.mbDateMode1904      = bDateMode1904;
    maBookSettings.mbDateCompatibility = bDateCompatibility;

    getUnitConverter().finalizeNullDate( getNullDate() );
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
