/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: workbooksettings.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/workbooksettings.hxx"
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/biffinputstream.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::util::Date;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::sheet::XCalculatable;

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

OoxWorkbookPrData::OoxWorkbookPrData() :
    mnShowObjectMode( XML_all ),
    mnUpdateLinksMode( XML_userSet ),
    mnDefaultThemeVer( -1 ),
    mbDateMode1904( false ),
    mbSaveExtLinkValues( true )
{
}

void OoxWorkbookPrData::setBinObjectMode( sal_uInt16 nObjMode )
{
    static const sal_Int32 spnObjModes[] = { XML_all, XML_placeholders, XML_none };
    mnShowObjectMode = STATIC_ARRAY_SELECT( spnObjModes, nObjMode, XML_all );
}

// ============================================================================

OoxCalcPrData::OoxCalcPrData() :
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

void WorkbookSettings::importWorkbookPr( const AttributeList& rAttribs )
{
    maOoxBookData.maCodeName          = rAttribs.getString( XML_codePage );
    maOoxBookData.mnShowObjectMode    = rAttribs.getToken( XML_showObjects, XML_all );
    maOoxBookData.mnUpdateLinksMode   = rAttribs.getToken( XML_updateLinks, XML_userSet );
    maOoxBookData.mnDefaultThemeVer   = rAttribs.getInteger( XML_defaultThemeVersion, -1 );
    maOoxBookData.mbDateMode1904      = rAttribs.getBool( XML_date1904, false );
    maOoxBookData.mbSaveExtLinkValues = rAttribs.getBool( XML_saveExternalLinkValues, true );
}

void WorkbookSettings::importCalcPr( const AttributeList& rAttribs )
{
    maOoxCalcData.mfIterateDelta  = rAttribs.getDouble( XML_iterateDelta, 0.0001 );
    maOoxCalcData.mnCalcId        = rAttribs.getInteger( XML_calcId, -1 );
    maOoxCalcData.mnRefMode       = rAttribs.getToken( XML_refMode, XML_A1 );
    maOoxCalcData.mnCalcMode      = rAttribs.getToken( XML_calcMode, XML_auto );
    maOoxCalcData.mnIterateCount  = rAttribs.getInteger( XML_iterateCount, 100 );
    maOoxCalcData.mnProcCount     = rAttribs.getInteger( XML_concurrentManualCount, -1 );
    maOoxCalcData.mbCalcOnSave    = rAttribs.getBool( XML_calcOnSave, true );
    maOoxCalcData.mbCalcCompleted = rAttribs.getBool( XML_calcCompleted, true );
    maOoxCalcData.mbFullPrecision = rAttribs.getBool( XML_fullPrecision, true );
    maOoxCalcData.mbIterate       = rAttribs.getBool( XML_iterate, false );
    maOoxCalcData.mbConcurrent    = rAttribs.getBool( XML_concurrentCalc, true );
}

void WorkbookSettings::importWorkbookPr( RecordInputStream& rStrm )
{
    sal_uInt32 nFlags;
    rStrm >> nFlags >> maOoxBookData.mnDefaultThemeVer >> maOoxBookData.maCodeName;
    maOoxBookData.setBinObjectMode( extractValue< sal_uInt16 >( nFlags, 13, 2 ) );
    maOoxBookData.mbDateMode1904 = getFlag( nFlags, OOBIN_WORKBOOKPR_DATE1904 );
    // set flag means: strip external link values
    maOoxBookData.mbSaveExtLinkValues = !getFlag( nFlags, OOBIN_WORKBOOKPR_STRIPEXT );
}

void WorkbookSettings::importCalcPr( RecordInputStream& rStrm )
{
    sal_Int32 nCalcMode, nProcCount;
    sal_uInt16 nFlags;
    rStrm >> maOoxCalcData.mnCalcId >> nCalcMode >> maOoxCalcData.mnIterateCount >> maOoxCalcData.mfIterateDelta >> nProcCount >> nFlags;

    static const sal_Int32 spnCalcModes[] = { XML_manual, XML_auto, XML_autoNoTable };
    maOoxCalcData.mnRefMode       = getFlagValue( nFlags, OOBIN_CALCPR_A1, XML_A1, XML_R1C1 );
    maOoxCalcData.mnCalcMode      = STATIC_ARRAY_SELECT( spnCalcModes, nCalcMode, XML_auto );
    maOoxCalcData.mnProcCount     = getFlagValue< sal_Int32 >( nFlags, OOBIN_CALCPR_MANUALPROC, nProcCount, -1 );
    maOoxCalcData.mbCalcOnSave    = getFlag( nFlags, OOBIN_CALCPR_CALCONSAVE );
    maOoxCalcData.mbCalcCompleted = getFlag( nFlags, OOBIN_CALCPR_CALCCOMPLETED );
    maOoxCalcData.mbFullPrecision = getFlag( nFlags, OOBIN_CALCPR_FULLPRECISION );
    maOoxCalcData.mbIterate       = getFlag( nFlags, OOBIN_CALCPR_ITERATE );
    maOoxCalcData.mbConcurrent    = getFlag( nFlags, OOBIN_CALCPR_CONCURRENT );
}

void WorkbookSettings::setSaveExtLinkValues( bool bSaveExtLinks )
{
    maOoxBookData.mbSaveExtLinkValues = bSaveExtLinks;
}

void WorkbookSettings::importBookBool( BiffInputStream& rStrm )
{
    // value of 0 means save external values, value of 1 means strip external values
    maOoxBookData.mbSaveExtLinkValues = rStrm.readuInt16() == 0;
}

void WorkbookSettings::importCalcCount( BiffInputStream& rStrm )
{
    maOoxCalcData.mnIterateCount = rStrm.readuInt16();
}

void WorkbookSettings::importCalcMode( BiffInputStream& rStrm )
{
    sal_Int16 nCalcMode = rStrm.readInt16() + 1;
    static const sal_Int32 spnCalcModes[] = { XML_autoNoTable, XML_manual, XML_auto };
    maOoxCalcData.mnCalcMode = STATIC_ARRAY_SELECT( spnCalcModes, nCalcMode, XML_auto );
}

void WorkbookSettings::importCodeName( BiffInputStream& rStrm )
{
    maOoxBookData.maCodeName = rStrm.readUniString();
}

void WorkbookSettings::importDateMode( BiffInputStream& rStrm )
{
    maOoxBookData.mbDateMode1904 = rStrm.readuInt16() != 0;
}

void WorkbookSettings::importDelta( BiffInputStream& rStrm )
{
    rStrm >> maOoxCalcData.mfIterateDelta;
}

void WorkbookSettings::importHideObj( BiffInputStream& rStrm )
{
    maOoxBookData.setBinObjectMode( rStrm.readuInt16() );
}

void WorkbookSettings::importIteration( BiffInputStream& rStrm )
{
    maOoxCalcData.mbIterate = rStrm.readuInt16() != 0;
}

void WorkbookSettings::importPrecision( BiffInputStream& rStrm )
{
    maOoxCalcData.mbFullPrecision = rStrm.readuInt16() != 0;
}

void WorkbookSettings::importRefMode( BiffInputStream& rStrm )
{
    maOoxCalcData.mnRefMode = (rStrm.readuInt16() == 0) ? XML_R1C1 : XML_A1;
}

void WorkbookSettings::importSaveRecalc( BiffInputStream& rStrm )
{
    maOoxCalcData.mbCalcOnSave = rStrm.readuInt16() != 0;
}

void WorkbookSettings::importUncalced( BiffInputStream& )
{
    // existence of this record indicates incomplete recalc
    maOoxCalcData.mbCalcCompleted = false;
}

void WorkbookSettings::importUsesElfs( BiffInputStream& rStrm )
{
    maOoxCalcData.mbUseNlr = rStrm.readuInt16() != 0;
}

void WorkbookSettings::finalizeImport()
{
    // default settings
    PropertySet aPropSet( getDocument() );
    switch( getFilterType() )
    {
        case FILTER_OOX:
        case FILTER_BIFF:
            aPropSet.setProperty( CREATE_OUSTRING( "IgnoreCase" ),          true );     // always in Excel
            aPropSet.setProperty( CREATE_OUSTRING( "RegularExpressions" ),  false );    // not supported in Excel
        break;
        case FILTER_UNKNOWN:
        break;
    }

    // calculation settings
    Date aNullDate = maOoxBookData.mbDateMode1904 ? Date( 1, 1, 1904 ) : Date( 30, 12, 1899 );

    aPropSet.setProperty( CREATE_OUSTRING( "NullDate" ),           aNullDate );
    aPropSet.setProperty( CREATE_OUSTRING( "IsIterationEnabled" ), maOoxCalcData.mbIterate );
    aPropSet.setProperty( CREATE_OUSTRING( "IterationCount" ),     maOoxCalcData.mnIterateCount );
    aPropSet.setProperty( CREATE_OUSTRING( "IterationEpsilon" ),   maOoxCalcData.mfIterateDelta );
    aPropSet.setProperty( CREATE_OUSTRING( "CalcAsShown" ),        !maOoxCalcData.mbFullPrecision );
    aPropSet.setProperty( CREATE_OUSTRING( "LookUpLabels" ),       maOoxCalcData.mbUseNlr );

    Reference< XNumberFormatsSupplier > xNumFmtsSupp( getDocument(), UNO_QUERY );
    if( xNumFmtsSupp.is() )
    {
        PropertySet aNumFmtProp( xNumFmtsSupp->getNumberFormatSettings() );
        aNumFmtProp.setProperty( CREATE_OUSTRING( "NullDate" ), aNullDate );
    }

    Reference< XCalculatable > xCalculatable( getDocument(), UNO_QUERY );
    if( xCalculatable.is() )
        xCalculatable->enableAutomaticCalculation( (maOoxCalcData.mnCalcMode == XML_auto) || (maOoxCalcData.mnCalcMode == XML_autoNoTable) );
}

sal_Int16 WorkbookSettings::getApiShowObjectMode() const
{
    switch( maOoxBookData.mnShowObjectMode )
    {
        case XML_all:           return API_SHOWMODE_SHOW;
        case XML_none:          return API_SHOWMODE_HIDE;
        // #i80528# placeholders not supported anymore, but this is handled internally in Calc
        case XML_placeholders:  return API_SHOWMODE_PLACEHOLDER;
    }
    return API_SHOWMODE_SHOW;
}

// ============================================================================

} // namespace xls
} // namespace oox

