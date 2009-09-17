/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: workbooksettings.cxx,v $
 * $Revision: 1.4 $
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

#include "oox/xls/workbooksettings.hxx"
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <comphelper/mediadescriptor.hxx>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::util::Date;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::sheet::XCalculatable;
using ::oox::core::CodecHelper;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_uInt32 OOBIN_WORKBOOKPR_DATE1904  = 0x00000001;
const sal_uInt32 OOBIN_WORKBOOKPR_STRIPEXT  = 0x00000080;

const sal_uInt16 OOBIN_CALCPR_A1            = 0x0002;
const sal_uInt16 OOBIN_CALCPR_ITERATE       = 0x0004;
const sal_uInt16 OOBIN_CALCPR_FULLPRECISION = 0x0008;
const sal_uInt16 OOBIN_CALCPR_CALCCOMPLETED = 0x0010;
const sal_uInt16 OOBIN_CALCPR_CALCONSAVE    = 0x0020;
const sal_uInt16 OOBIN_CALCPR_CONCURRENT    = 0x0040;
const sal_uInt16 OOBIN_CALCPR_MANUALPROC    = 0x0080;

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

void WorkbookSettingsModel::setBinObjectMode( sal_uInt16 nObjMode )
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
    maBookSettings.maCodeName          = rAttribs.getString( XML_codePage, OUString() );
    maBookSettings.mnShowObjectMode    = rAttribs.getToken( XML_showObjects, XML_all );
    maBookSettings.mnUpdateLinksMode   = rAttribs.getToken( XML_updateLinks, XML_userSet );
    maBookSettings.mnDefaultThemeVer   = rAttribs.getInteger( XML_defaultThemeVersion, -1 );
    maBookSettings.mbSaveExtLinkValues = rAttribs.getBool( XML_saveExternalLinkValues, true );
    setDateMode( rAttribs.getBool( XML_date1904, false ) );
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

void WorkbookSettings::importFileSharing( RecordInputStream& rStrm )
{
    maFileSharing.mbRecommendReadOnly = rStrm.readuInt16() != 0;
    rStrm >> maFileSharing.mnPasswordHash >> maFileSharing.maUserName;
}

void WorkbookSettings::importWorkbookPr( RecordInputStream& rStrm )
{
    sal_uInt32 nFlags;
    rStrm >> nFlags >> maBookSettings.mnDefaultThemeVer >> maBookSettings.maCodeName;
    maBookSettings.setBinObjectMode( extractValue< sal_uInt16 >( nFlags, 13, 2 ) );
    // set flag means: strip external link values
    maBookSettings.mbSaveExtLinkValues = !getFlag( nFlags, OOBIN_WORKBOOKPR_STRIPEXT );
    setDateMode( getFlag( nFlags, OOBIN_WORKBOOKPR_DATE1904 ) );
}

void WorkbookSettings::importCalcPr( RecordInputStream& rStrm )
{
    sal_Int32 nCalcMode, nProcCount;
    sal_uInt16 nFlags;
    rStrm >> maCalcSettings.mnCalcId >> nCalcMode >> maCalcSettings.mnIterateCount >> maCalcSettings.mfIterateDelta >> nProcCount >> nFlags;

    static const sal_Int32 spnCalcModes[] = { XML_manual, XML_auto, XML_autoNoTable };
    maCalcSettings.mnRefMode       = getFlagValue( nFlags, OOBIN_CALCPR_A1, XML_A1, XML_R1C1 );
    maCalcSettings.mnCalcMode      = STATIC_ARRAY_SELECT( spnCalcModes, nCalcMode, XML_auto );
    maCalcSettings.mnProcCount     = getFlagValue< sal_Int32 >( nFlags, OOBIN_CALCPR_MANUALPROC, nProcCount, -1 );
    maCalcSettings.mbCalcOnSave    = getFlag( nFlags, OOBIN_CALCPR_CALCONSAVE );
    maCalcSettings.mbCalcCompleted = getFlag( nFlags, OOBIN_CALCPR_CALCCOMPLETED );
    maCalcSettings.mbFullPrecision = getFlag( nFlags, OOBIN_CALCPR_FULLPRECISION );
    maCalcSettings.mbIterate       = getFlag( nFlags, OOBIN_CALCPR_ITERATE );
    maCalcSettings.mbConcurrent    = getFlag( nFlags, OOBIN_CALCPR_CONCURRENT );
}

void WorkbookSettings::setSaveExtLinkValues( bool bSaveExtLinks )
{
    maBookSettings.mbSaveExtLinkValues = bSaveExtLinks;
}

void WorkbookSettings::importFileSharing( BiffInputStream& rStrm )
{
    maFileSharing.mbRecommendReadOnly = rStrm.readuInt16() != 0;
    rStrm >> maFileSharing.mnPasswordHash;
    if( getBiff() == BIFF8 )
    {
        sal_uInt16 nStrLen = rStrm.readuInt16();
        // there is no string flags field if string is empty
        if( nStrLen > 0 )
            maFileSharing.maUserName = rStrm.readUniStringBody( nStrLen );
    }
    else
    {
        maFileSharing.maUserName = rStrm.readByteStringUC( false, getTextEncoding() );
    }
}

void WorkbookSettings::importBookBool( BiffInputStream& rStrm )
{
    // value of 0 means save external values, value of 1 means strip external values
    maBookSettings.mbSaveExtLinkValues = rStrm.readuInt16() == 0;
}

void WorkbookSettings::importCalcCount( BiffInputStream& rStrm )
{
    maCalcSettings.mnIterateCount = rStrm.readuInt16();
}

void WorkbookSettings::importCalcMode( BiffInputStream& rStrm )
{
    sal_Int16 nCalcMode = rStrm.readInt16() + 1;
    static const sal_Int32 spnCalcModes[] = { XML_autoNoTable, XML_manual, XML_auto };
    maCalcSettings.mnCalcMode = STATIC_ARRAY_SELECT( spnCalcModes, nCalcMode, XML_auto );
}

void WorkbookSettings::importCodeName( BiffInputStream& rStrm )
{
    maBookSettings.maCodeName = rStrm.readUniString();
}

void WorkbookSettings::importDateMode( BiffInputStream& rStrm )
{
    setDateMode( rStrm.readuInt16() != 0 );
}

void WorkbookSettings::importDelta( BiffInputStream& rStrm )
{
    rStrm >> maCalcSettings.mfIterateDelta;
}

void WorkbookSettings::importHideObj( BiffInputStream& rStrm )
{
    maBookSettings.setBinObjectMode( rStrm.readuInt16() );
}

void WorkbookSettings::importIteration( BiffInputStream& rStrm )
{
    maCalcSettings.mbIterate = rStrm.readuInt16() != 0;
}

void WorkbookSettings::importPrecision( BiffInputStream& rStrm )
{
    maCalcSettings.mbFullPrecision = rStrm.readuInt16() != 0;
}

void WorkbookSettings::importRefMode( BiffInputStream& rStrm )
{
    maCalcSettings.mnRefMode = (rStrm.readuInt16() == 0) ? XML_R1C1 : XML_A1;
}

void WorkbookSettings::importSaveRecalc( BiffInputStream& rStrm )
{
    maCalcSettings.mbCalcOnSave = rStrm.readuInt16() != 0;
}

void WorkbookSettings::importUncalced( BiffInputStream& )
{
    // existence of this record indicates incomplete recalc
    maCalcSettings.mbCalcCompleted = false;
}

void WorkbookSettings::importUsesElfs( BiffInputStream& rStrm )
{
    maCalcSettings.mbUseNlr = rStrm.readuInt16() != 0;
}

void WorkbookSettings::finalizeImport()
{
    // default settings
    PropertySet aPropSet( getDocument() );
    switch( getFilterType() )
    {
        case FILTER_OOX:
        case FILTER_BIFF:
            aPropSet.setProperty( PROP_IgnoreCase,          true );     // always in Excel
            aPropSet.setProperty( PROP_RegularExpressions,  false );    // not supported in Excel
        break;
        case FILTER_UNKNOWN:
        break;
    }

    // write protection
    if( maFileSharing.mbRecommendReadOnly || (maFileSharing.mnPasswordHash != 0) )
        getBaseFilter().getMediaDescriptor()[ CREATE_OUSTRING( "ReadOnly" ) ] <<= true;

    // calculation settings
    Date aNullDate = getNullDate();

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

Date WorkbookSettings::getNullDate() const
{
    static const Date saDate1900( 30, 12, 1899 ), saDate1904( 1, 1, 1904 );
    return maBookSettings.mbDateMode1904 ? saDate1904 : saDate1900;
}

void WorkbookSettings::setDateMode( bool bDateMode1904 )
{
    maBookSettings.mbDateMode1904 = bDateMode1904;
    getUnitConverter().finalizeNullDate( getNullDate() );
}

// ============================================================================

} // namespace xls
} // namespace oox

