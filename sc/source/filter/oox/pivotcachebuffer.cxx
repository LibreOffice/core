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

#include <pivotcachebuffer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupInfo.hpp>
#include <com/sun/star/sheet/XDataPilotFieldGrouping.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <tools/diagnose_ex.h>
#include <defnamesbuffer.hxx>
#include <pivotcachefragment.hxx>
#include <sheetdatabuffer.hxx>
#include <tablebuffer.hxx>
#include <unitconverter.hxx>
#include <worksheetbuffer.hxx>
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <tools/datetime.hxx>
#include <addressconverter.hxx>
#include <biffhelper.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;

using ::oox::core::Relations;

namespace {

const sal_uInt16 BIFF12_PCDFIELD_SERVERFIELD        = 0x0001;
const sal_uInt16 BIFF12_PCDFIELD_NOUNIQUEITEMS      = 0x0002;
const sal_uInt16 BIFF12_PCDFIELD_DATABASEFIELD      = 0x0004;
const sal_uInt16 BIFF12_PCDFIELD_HASCAPTION         = 0x0008;
const sal_uInt16 BIFF12_PCDFIELD_MEMBERPROPFIELD    = 0x0010;
const sal_uInt16 BIFF12_PCDFIELD_HASFORMULA         = 0x0100;
const sal_uInt16 BIFF12_PCDFIELD_HASPROPERTYNAME    = 0x0200;

const sal_uInt16 BIFF12_PCDFSITEMS_HASSEMIMIXED     = 0x0001;
const sal_uInt16 BIFF12_PCDFSITEMS_HASNONDATE       = 0x0002;
const sal_uInt16 BIFF12_PCDFSITEMS_HASDATE          = 0x0004;
const sal_uInt16 BIFF12_PCDFSITEMS_HASSTRING        = 0x0008;
const sal_uInt16 BIFF12_PCDFSITEMS_HASBLANK         = 0x0010;
const sal_uInt16 BIFF12_PCDFSITEMS_HASMIXED         = 0x0020;
const sal_uInt16 BIFF12_PCDFSITEMS_ISNUMERIC        = 0x0040;
const sal_uInt16 BIFF12_PCDFSITEMS_ISINTEGER        = 0x0080;
const sal_uInt16 BIFF12_PCDFSITEMS_HASLONGTEXT      = 0x0200;

const sal_uInt16 BIFF12_PCITEM_ARRAY_DOUBLE         = 0x0001;
const sal_uInt16 BIFF12_PCITEM_ARRAY_STRING         = 0x0002;
const sal_uInt16 BIFF12_PCITEM_ARRAY_ERROR          = 0x0010;
const sal_uInt16 BIFF12_PCITEM_ARRAY_DATE           = 0x0020;

const sal_uInt8 BIFF12_PCDFRANGEPR_AUTOSTART        = 0x01;
const sal_uInt8 BIFF12_PCDFRANGEPR_AUTOEND          = 0x02;
const sal_uInt8 BIFF12_PCDFRANGEPR_DATEGROUP        = 0x04;

const sal_uInt8 BIFF12_PCDEFINITION_SAVEDATA        = 0x01;
const sal_uInt8 BIFF12_PCDEFINITION_INVALID         = 0x02;
const sal_uInt8 BIFF12_PCDEFINITION_REFRESHONLOAD   = 0x04;
const sal_uInt8 BIFF12_PCDEFINITION_OPTIMIZEMEMORY  = 0x08;
const sal_uInt8 BIFF12_PCDEFINITION_ENABLEREFRESH   = 0x10;
const sal_uInt8 BIFF12_PCDEFINITION_BACKGROUNDQUERY = 0x20;
const sal_uInt8 BIFF12_PCDEFINITION_UPGRADEONREFR   = 0x40;
const sal_uInt8 BIFF12_PCDEFINITION_TUPELCACHE      = 0x80;

const sal_uInt8 BIFF12_PCDEFINITION_HASUSERNAME     = 0x01;
const sal_uInt8 BIFF12_PCDEFINITION_HASRELID        = 0x02;
const sal_uInt8 BIFF12_PCDEFINITION_SUPPORTSUBQUERY = 0x04;
const sal_uInt8 BIFF12_PCDEFINITION_SUPPORTDRILL    = 0x08;

const sal_uInt8 BIFF12_PCDWBSOURCE_HASRELID         = 0x01;
const sal_uInt8 BIFF12_PCDWBSOURCE_HASSHEET         = 0x02;


/** Adjusts the weird date format read from binary streams.

    Dates before 1900-Mar-01 are stored including the non-existing leap day
    1900-02-29. tools::Time values (without date) are stored as times of day
    1900-Jan-00. Nothing has to be done when the workbook is stored in 1904
    date mode (dates before 1904-Jan-01 will not occur in this case).
 */
void lclAdjustBinDateTime( css::util::DateTime& orDateTime )
{
    if( (orDateTime.Year == 1900) && (orDateTime.Month <= 2) )
    {
        OSL_ENSURE( (orDateTime.Month == 1) || ((orDateTime.Month == 2) && (orDateTime.Day > 0)), "lclAdjustBinDateTime - invalid date" );
        switch( orDateTime.Month )
        {
            case 2: if( orDateTime.Day > 1 ) --orDateTime.Day; else { orDateTime.Day += 30; --orDateTime.Month; }                       break;
            case 1: if( orDateTime.Day > 1 ) --orDateTime.Day; else { orDateTime.Day += 30; orDateTime.Month = 12; --orDateTime.Year; } break;
        }
    }
}

} // namespace

PivotCacheItem::PivotCacheItem() :
    mnType( XML_m ), mbUnused( false )
{
}

void PivotCacheItem::readString( const AttributeList& rAttribs )
{
    maValue <<= rAttribs.getXString( XML_v, OUString() );
    mnType = XML_s;
}

void PivotCacheItem::readNumeric( const AttributeList& rAttribs )
{
    maValue <<= rAttribs.getDouble( XML_v, 0.0 );
    mnType = XML_n;
    mbUnused = rAttribs.getBool( XML_u, false );
}

void PivotCacheItem::readDate( const AttributeList& rAttribs )
{
    maValue <<= rAttribs.getDateTime( XML_v, css::util::DateTime() );
    mnType = XML_d;
}

void PivotCacheItem::readBool( const AttributeList& rAttribs )
{
    maValue <<= rAttribs.getBool( XML_v, false );
    mnType = XML_b;
}

void PivotCacheItem::readError( const AttributeList& rAttribs )
{
    maValue <<= rAttribs.getXString( XML_v, OUString() );
    mnType = XML_e;
}

void PivotCacheItem::readIndex( const AttributeList& rAttribs )
{
    maValue <<= rAttribs.getInteger( XML_v, -1 );
    mnType = XML_x;
}

void PivotCacheItem::readString( SequenceInputStream& rStrm )
{
    maValue <<= BiffHelper::readString( rStrm );
    mnType = XML_s;
}

void PivotCacheItem::readDouble( SequenceInputStream& rStrm )
{
    maValue <<= rStrm.readDouble();
    mnType = XML_n;
}

void PivotCacheItem::readDate( SequenceInputStream& rStrm )
{
    css::util::DateTime aDateTime;
    aDateTime.Year = rStrm.readuInt16();
    aDateTime.Month = rStrm.readuInt16();
    aDateTime.Day = rStrm.readuInt8();
    aDateTime.Hours = rStrm.readuInt8();
    aDateTime.Minutes = rStrm.readuInt8();
    aDateTime.Seconds = rStrm.readuInt8();
    lclAdjustBinDateTime( aDateTime );
    maValue <<= aDateTime;
    mnType = XML_d;
}

void PivotCacheItem::readBool( SequenceInputStream& rStrm )
{
    maValue <<= (rStrm.readuInt8() != 0);
    mnType = XML_b;
}

void PivotCacheItem::readError( SequenceInputStream& rStrm )
{
    maValue <<= static_cast< sal_Int32 >( rStrm.readuInt8() );
    mnType = XML_e;
}

void PivotCacheItem::readIndex( SequenceInputStream& rStrm )
{
    maValue <<= rStrm.readInt32();
    mnType = XML_x;
}

void PivotCacheItem::setStringValue( const OUString& sString )
{
    mnType = XML_s;
    maValue <<= sString;
}

OUString PivotCacheItem::getName() const
{
    switch( mnType )
    {
        case XML_m: return OUString();
        case XML_s: return maValue.get< OUString >();
        case XML_n: return OUString::number( maValue.get< double >() );                             // !TODO
        case XML_i: return OUString::number( maValue.get< sal_Int32 >() );
        case XML_d: return OUString();                                                              // !TODO
        case XML_b: return OUString::boolean( maValue.get< bool >() );                              // !TODO
        case XML_e: return OUString();                                                              // !TODO
    }
    OSL_FAIL( "PivotCacheItem::getName - invalid data type" );
    return OUString();
}

OUString PivotCacheItem::getFormattedName(const ScDPSaveDimension& rSaveDim, ScDPObject* pObj, const DateTime& rNullDate) const
{
    switch( mnType )
    {
        case XML_m: return OUString();
        case XML_s: return maValue.get< OUString >();
        case XML_n: return pObj->GetFormattedString(rSaveDim.GetName(), maValue.get<double>());
        case XML_i: return pObj->GetFormattedString(rSaveDim.GetName(), static_cast<double>(maValue.get< sal_Int32 >()));
        case XML_b: return pObj->GetFormattedString(rSaveDim.GetName(), static_cast<double>(maValue.get< bool >()));
        case XML_d: return pObj->GetFormattedString(rSaveDim.GetName(), maValue.get< css::util::DateTime >() - rNullDate);
        case XML_e: return maValue.get< OUString >();
    }
    OSL_FAIL( "PivotCacheItem::getFormattedName - invalid data type" );
    return OUString();
}

PivotCacheItemList::PivotCacheItemList( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void PivotCacheItemList::importItem( sal_Int32 nElement, const AttributeList& rAttribs )
{
    PivotCacheItem& rItem = createItem();
    switch( nElement )
    {
        case XLS_TOKEN( m ):                                                        break;
        case XLS_TOKEN( s ):    rItem.readString( rAttribs );                       break;
        case XLS_TOKEN( n ):    rItem.readNumeric( rAttribs );                      break;
        case XLS_TOKEN( d ):    rItem.readDate( rAttribs );                         break;
        case XLS_TOKEN( b ):    rItem.readBool( rAttribs );                         break;
        case XLS_TOKEN( e ):    rItem.readError( rAttribs );                        break;
        default:    OSL_FAIL( "PivotCacheItemList::importItem - unknown element type" );
    }
}

void PivotCacheItemList::importItem( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    if( nRecId == BIFF12_ID_PCITEM_ARRAY )
    {
        importArray( rStrm );
        return;
    }

    PivotCacheItem& rItem = createItem();
    switch( nRecId )
    {
        case BIFF12_ID_PCITEM_MISSING:
        case BIFF12_ID_PCITEMA_MISSING:                             break;
        case BIFF12_ID_PCITEM_STRING:
        case BIFF12_ID_PCITEMA_STRING:  rItem.readString( rStrm );  break;
        case BIFF12_ID_PCITEM_DOUBLE:
        case BIFF12_ID_PCITEMA_DOUBLE:  rItem.readDouble( rStrm );  break;
        case BIFF12_ID_PCITEM_DATE:
        case BIFF12_ID_PCITEMA_DATE:    rItem.readDate( rStrm );    break;
        case BIFF12_ID_PCITEM_BOOL:
        case BIFF12_ID_PCITEMA_BOOL:    rItem.readBool( rStrm );    break;
        case BIFF12_ID_PCITEM_ERROR:
        case BIFF12_ID_PCITEMA_ERROR:   rItem.readError( rStrm );   break;
        default:    OSL_FAIL( "PivotCacheItemList::importItem - unknown record type" );
    }
}

const PivotCacheItem* PivotCacheItemList::getCacheItem( sal_Int32 nItemIdx ) const
{
    return ContainerHelper::getVectorElement( maItems, nItemIdx );
}

void PivotCacheItemList::applyItemCaptions( const IdCaptionPairList& vCaptions )
{
    for( const auto& [rId, rCaption] : vCaptions )
    {
        if ( static_cast<sal_uInt32>( rId ) < maItems.size() )
            maItems[ rId ].setStringValue( rCaption );
    }
}

void PivotCacheItemList::getCacheItemNames( ::std::vector< OUString >& orItemNames ) const
{
    orItemNames.clear();
    orItemNames.reserve( maItems.size() );
    for( const auto& rItem : maItems )
        orItemNames.push_back( rItem.getName() );
}

// private --------------------------------------------------------------------

PivotCacheItem& PivotCacheItemList::createItem()
{
    maItems.emplace_back();
    return maItems.back();
}

void PivotCacheItemList::importArray( SequenceInputStream& rStrm )
{
    sal_uInt16 nType = rStrm.readuInt16();
    sal_Int32 nCount = rStrm.readInt32();
    for( sal_Int32 nIdx = 0; !rStrm.isEof() && (nIdx < nCount); ++nIdx )
    {
        switch( nType )
        {
            case BIFF12_PCITEM_ARRAY_DOUBLE: createItem().readDouble( rStrm );   break;
            case BIFF12_PCITEM_ARRAY_STRING: createItem().readString( rStrm );   break;
            case BIFF12_PCITEM_ARRAY_ERROR:  createItem().readError( rStrm );    break;
            case BIFF12_PCITEM_ARRAY_DATE:   createItem().readDate( rStrm );     break;
            default:
                OSL_FAIL( "PivotCacheItemList::importArray - unknown data type" );
                return;
        }
    }
}

PCFieldModel::PCFieldModel() :
    mnNumFmtId( 0 ),
    mnSqlType( 0 ),
    mnHierarchy( 0 ),
    mnLevel( 0 ),
    mnMappingCount( 0 ),
    mbDatabaseField( true ),
    mbServerField( false ),
    mbUniqueList( true ),
    mbMemberPropField( false )
{
}

PCSharedItemsModel::PCSharedItemsModel() :
    mbHasSemiMixed( true ),
    mbHasNonDate( true ),
    mbHasDate( false ),
    mbHasString( true ),
    mbHasBlank( false ),
    mbHasMixed( false ),
    mbIsNumeric( false ),
    mbIsInteger( false ),
    mbHasLongText( false )
{
}

PCFieldGroupModel::PCFieldGroupModel() :
    mfStartValue( 0.0 ),
    mfEndValue( 0.0 ),
    mfInterval( 1.0 ),
    mnParentField( -1 ),
    mnBaseField( -1 ),
    mnGroupBy( XML_range ),
    mbRangeGroup( false ),
    mbDateGroup( false ),
    mbAutoStart( true ),
    mbAutoEnd( true )
{
}

void PCFieldGroupModel::setBiffGroupBy( sal_uInt8 nGroupBy )
{
    static const sal_Int32 spnGroupBy[] = { XML_range,
        XML_seconds, XML_minutes, XML_hours, XML_days, XML_months, XML_quarters, XML_years };
    mnGroupBy = STATIC_ARRAY_SELECT( spnGroupBy, nGroupBy, XML_range );
}

PivotCacheField::PivotCacheField( const WorkbookHelper& rHelper, bool bIsDatabaseField ) :
    WorkbookHelper( rHelper ),
    maSharedItems( rHelper ),
    maGroupItems( rHelper )
{
    maFieldModel.mbDatabaseField = bIsDatabaseField;
}

void PivotCacheField::importCacheField( const AttributeList& rAttribs )
{
    maFieldModel.maName            = rAttribs.getXString( XML_name, OUString() );
    maFieldModel.maCaption         = rAttribs.getXString( XML_caption, OUString() );
    maFieldModel.maPropertyName    = rAttribs.getXString( XML_propertyName, OUString() );
    maFieldModel.maFormula         = rAttribs.getXString( XML_formula, OUString() );
    maFieldModel.mnNumFmtId        = rAttribs.getInteger( XML_numFmtId, 0 );
    maFieldModel.mnSqlType         = rAttribs.getInteger( XML_sqlType, 0 );
    maFieldModel.mnHierarchy       = rAttribs.getInteger( XML_hierarchy, 0 );
    maFieldModel.mnLevel           = rAttribs.getInteger( XML_level, 0 );
    maFieldModel.mnMappingCount    = rAttribs.getInteger( XML_mappingCount, 0 );
    maFieldModel.mbDatabaseField   = rAttribs.getBool( XML_databaseField, true );
    maFieldModel.mbServerField     = rAttribs.getBool( XML_serverField, false );
    maFieldModel.mbUniqueList      = rAttribs.getBool( XML_uniqueList, true );
    maFieldModel.mbMemberPropField = rAttribs.getBool( XML_memberPropertyField, false );
}

void PivotCacheField::importSharedItems( const AttributeList& rAttribs )
{
    OSL_ENSURE( maSharedItems.empty(), "PivotCacheField::importSharedItems - multiple shared items elements" );
    maSharedItemsModel.mbHasSemiMixed = rAttribs.getBool( XML_containsSemiMixedTypes, true );
    maSharedItemsModel.mbHasNonDate   = rAttribs.getBool( XML_containsNonDate, true );
    maSharedItemsModel.mbHasDate      = rAttribs.getBool( XML_containsDate, false );
    maSharedItemsModel.mbHasString    = rAttribs.getBool( XML_containsString, true );
    maSharedItemsModel.mbHasBlank     = rAttribs.getBool( XML_containsBlank, false );
    maSharedItemsModel.mbHasMixed     = rAttribs.getBool( XML_containsMixedTypes, false );
    maSharedItemsModel.mbIsNumeric    = rAttribs.getBool( XML_containsNumber, false );
    maSharedItemsModel.mbIsInteger    = rAttribs.getBool( XML_containsInteger, false );
    maSharedItemsModel.mbHasLongText  = rAttribs.getBool( XML_longText, false );
}

void PivotCacheField::importSharedItem( sal_Int32 nElement, const AttributeList& rAttribs )
{
    maSharedItems.importItem( nElement, rAttribs );
}

void PivotCacheField::importFieldGroup( const AttributeList& rAttribs )
{
    maFieldGroupModel.mnParentField = rAttribs.getInteger( XML_par, -1 );
    maFieldGroupModel.mnBaseField   = rAttribs.getInteger( XML_base, -1 );
}

void PivotCacheField::importRangePr( const AttributeList& rAttribs )
{
    maFieldGroupModel.maStartDate    = rAttribs.getDateTime( XML_startDate, css::util::DateTime() );
    maFieldGroupModel.maEndDate      = rAttribs.getDateTime( XML_endDate, css::util::DateTime() );
    maFieldGroupModel.mfStartValue   = rAttribs.getDouble( XML_startNum, 0.0 );
    maFieldGroupModel.mfEndValue     = rAttribs.getDouble( XML_endNum, 0.0 );
    maFieldGroupModel.mfInterval     = rAttribs.getDouble( XML_groupInterval, 1.0 );
    maFieldGroupModel.mnGroupBy      = rAttribs.getToken( XML_groupBy, XML_range );
    maFieldGroupModel.mbRangeGroup   = true;
    maFieldGroupModel.mbDateGroup    = maFieldGroupModel.mnGroupBy != XML_range;
    maFieldGroupModel.mbAutoStart    = rAttribs.getBool( XML_autoStart, true );
    maFieldGroupModel.mbAutoEnd      = rAttribs.getBool( XML_autoEnd, true );
}

void PivotCacheField::importDiscretePrItem( sal_Int32 nElement, const AttributeList& rAttribs )
{
    OSL_ENSURE( nElement == XLS_TOKEN( x ), "PivotCacheField::importDiscretePrItem - unexpected element" );
    if( nElement == XLS_TOKEN( x ) )
        maDiscreteItems.push_back( rAttribs.getInteger( XML_v, -1 ) );
}

void PivotCacheField::importGroupItem( sal_Int32 nElement, const AttributeList& rAttribs )
{
    maGroupItems.importItem( nElement, rAttribs );
}

void PivotCacheField::importPCDField( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags;
    nFlags = rStrm.readuInt16();
    maFieldModel.mnNumFmtId = rStrm.readInt32();
    maFieldModel.mnSqlType = rStrm.readInt16();
    maFieldModel.mnHierarchy = rStrm.readInt32();
    maFieldModel.mnLevel = rStrm.readInt32();
    maFieldModel.mnMappingCount = rStrm.readInt32();
    rStrm >> maFieldModel.maName;
    if( getFlag( nFlags, BIFF12_PCDFIELD_HASCAPTION ) )
        rStrm >> maFieldModel.maCaption;
    if( getFlag( nFlags, BIFF12_PCDFIELD_HASFORMULA ) )
        rStrm.skip( ::std::max< sal_Int32 >( rStrm.readInt32(), 0 ) );
    if( maFieldModel.mnMappingCount > 0 )
        rStrm.skip( ::std::max< sal_Int32 >( rStrm.readInt32(), 0 ) );
    if( getFlag( nFlags, BIFF12_PCDFIELD_HASPROPERTYNAME ) )
        rStrm >> maFieldModel.maPropertyName;

    maFieldModel.mbDatabaseField   = getFlag( nFlags, BIFF12_PCDFIELD_DATABASEFIELD );
    maFieldModel.mbServerField     = getFlag( nFlags, BIFF12_PCDFIELD_SERVERFIELD );
    maFieldModel.mbUniqueList      = !getFlag( nFlags, BIFF12_PCDFIELD_NOUNIQUEITEMS );
    maFieldModel.mbMemberPropField = getFlag( nFlags, BIFF12_PCDFIELD_MEMBERPROPFIELD );
}

void PivotCacheField::importPCDFSharedItems( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags;
    nFlags = rStrm.readuInt16();
    maSharedItemsModel.mbHasSemiMixed = getFlag( nFlags, BIFF12_PCDFSITEMS_HASSEMIMIXED );
    maSharedItemsModel.mbHasNonDate   = getFlag( nFlags, BIFF12_PCDFSITEMS_HASNONDATE );
    maSharedItemsModel.mbHasDate      = getFlag( nFlags, BIFF12_PCDFSITEMS_HASDATE );
    maSharedItemsModel.mbHasString    = getFlag( nFlags, BIFF12_PCDFSITEMS_HASSTRING );
    maSharedItemsModel.mbHasBlank     = getFlag( nFlags, BIFF12_PCDFSITEMS_HASBLANK );
    maSharedItemsModel.mbHasMixed     = getFlag( nFlags, BIFF12_PCDFSITEMS_HASMIXED );
    maSharedItemsModel.mbIsNumeric    = getFlag( nFlags, BIFF12_PCDFSITEMS_ISNUMERIC );
    maSharedItemsModel.mbIsInteger    = getFlag( nFlags, BIFF12_PCDFSITEMS_ISINTEGER );
    maSharedItemsModel.mbHasLongText  = getFlag( nFlags, BIFF12_PCDFSITEMS_HASLONGTEXT );
}

void PivotCacheField::importPCDFSharedItem( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    maSharedItems.importItem( nRecId, rStrm );
}

void PivotCacheField::importPCDFieldGroup( SequenceInputStream& rStrm )
{
    maFieldGroupModel.mnParentField = rStrm.readInt32();
    maFieldGroupModel.mnBaseField = rStrm.readInt32();
}

void PivotCacheField::importPCDFRangePr( SequenceInputStream& rStrm )
{
    sal_uInt8 nGroupBy, nFlags;
    nGroupBy = rStrm.readuChar();
    nFlags = rStrm.readuChar();
    maFieldGroupModel.mfStartValue = rStrm.readDouble();
    maFieldGroupModel.mfEndValue = rStrm.readDouble();
    maFieldGroupModel.mfInterval = rStrm.readDouble();

    maFieldGroupModel.setBiffGroupBy( nGroupBy );
    maFieldGroupModel.mbRangeGroup   = true;
    maFieldGroupModel.mbDateGroup    = getFlag( nFlags, BIFF12_PCDFRANGEPR_DATEGROUP );
    maFieldGroupModel.mbAutoStart    = getFlag( nFlags, BIFF12_PCDFRANGEPR_AUTOSTART );
    maFieldGroupModel.mbAutoEnd      = getFlag( nFlags, BIFF12_PCDFRANGEPR_AUTOEND );

    OSL_ENSURE( maFieldGroupModel.mbDateGroup == (maFieldGroupModel.mnGroupBy != XML_range), "PivotCacheField::importPCDFRangePr - wrong date flag" );
    if( maFieldGroupModel.mbDateGroup )
    {
        maFieldGroupModel.maStartDate = getUnitConverter().calcDateTimeFromSerial( maFieldGroupModel.mfStartValue );
        maFieldGroupModel.maEndDate   = getUnitConverter().calcDateTimeFromSerial( maFieldGroupModel.mfEndValue );
    }
}

void PivotCacheField::importPCDFDiscretePrItem( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    OSL_ENSURE( nRecId == BIFF12_ID_PCITEM_INDEX, "PivotCacheField::importPCDFDiscretePrItem - unexpected record" );
    if( nRecId == BIFF12_ID_PCITEM_INDEX )
        maDiscreteItems.push_back( rStrm.readInt32() );
}

void PivotCacheField::importPCDFGroupItem( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    maGroupItems.importItem( nRecId, rStrm );
}

const PivotCacheItem* PivotCacheField::getCacheItem( sal_Int32 nItemIdx ) const
{
    if( hasGroupItems() )
        return maGroupItems.getCacheItem( nItemIdx );
    if( hasSharedItems() )
        return maSharedItems.getCacheItem( nItemIdx );
    return nullptr;
}

void PivotCacheField::applyItemCaptions( const IdCaptionPairList& vCaptions )
{
    if( hasGroupItems() )
        maGroupItems.applyItemCaptions( vCaptions );
    if( hasSharedItems() )
        maSharedItems.applyItemCaptions( vCaptions );
}

void PivotCacheField::getCacheItemNames( ::std::vector< OUString >& orItemNames ) const
{
    if( hasGroupItems() )
        maGroupItems.getCacheItemNames( orItemNames );
    else if( hasSharedItems() )
        maSharedItems.getCacheItemNames( orItemNames );
}

const PivotCacheItemList& PivotCacheField::getCacheItems() const
{
    if( hasGroupItems() )
        return maGroupItems;
    return maSharedItems;
}

void PivotCacheField::convertNumericGrouping( const Reference< XDataPilotField >& rxDPField ) const
{
    OSL_ENSURE( hasGroupItems() && hasNumericGrouping(), "PivotCacheField::convertNumericGrouping - not a numeric group field" );
    PropertySet aPropSet( rxDPField );
    if( hasGroupItems() && hasNumericGrouping() && aPropSet.is() )
    {
        DataPilotFieldGroupInfo aGroupInfo;
        aGroupInfo.HasAutoStart  = maFieldGroupModel.mbAutoStart;
        aGroupInfo.HasAutoEnd    = maFieldGroupModel.mbAutoEnd;
        aGroupInfo.HasDateValues = false;
        aGroupInfo.Start         = maFieldGroupModel.mfStartValue;
        aGroupInfo.End           = maFieldGroupModel.mfEndValue;
        aGroupInfo.Step          = maFieldGroupModel.mfInterval;
        aGroupInfo.GroupBy       = 0;
        aPropSet.setProperty( PROP_GroupInfo, aGroupInfo );
    }
}

OUString PivotCacheField::createDateGroupField( const Reference< XDataPilotField >& rxBaseDPField ) const
{
    OSL_ENSURE( hasGroupItems() && hasDateGrouping(), "PivotCacheField::createDateGroupField - not a numeric group field" );
    Reference< XDataPilotField > xDPGroupField;
    PropertySet aPropSet( rxBaseDPField );
    if( hasGroupItems() && hasDateGrouping() && aPropSet.is() )
    {
        bool bDayRanges = (maFieldGroupModel.mnGroupBy == XML_days) && (maFieldGroupModel.mfInterval >= 2.0);

        DataPilotFieldGroupInfo aGroupInfo;
        aGroupInfo.HasAutoStart  = maFieldGroupModel.mbAutoStart;
        aGroupInfo.HasAutoEnd    = maFieldGroupModel.mbAutoEnd;
        aGroupInfo.HasDateValues = true;
        aGroupInfo.Start         = getUnitConverter().calcSerialFromDateTime( maFieldGroupModel.maStartDate );
        aGroupInfo.End           = getUnitConverter().calcSerialFromDateTime( maFieldGroupModel.maEndDate );
        aGroupInfo.Step          = bDayRanges ? maFieldGroupModel.mfInterval : 0.0;

        using namespace ::com::sun::star::sheet::DataPilotFieldGroupBy;
        switch( maFieldGroupModel.mnGroupBy )
        {
            case XML_years:     aGroupInfo.GroupBy = YEARS;     break;
            case XML_quarters:  aGroupInfo.GroupBy = QUARTERS;  break;
            case XML_months:    aGroupInfo.GroupBy = MONTHS;    break;
            case XML_days:      aGroupInfo.GroupBy = DAYS;      break;
            case XML_hours:     aGroupInfo.GroupBy = HOURS;     break;
            case XML_minutes:   aGroupInfo.GroupBy = MINUTES;   break;
            case XML_seconds:   aGroupInfo.GroupBy = SECONDS;   break;
            default:    OSL_FAIL( "PivotCacheField::convertRangeGrouping - unknown date/time interval" );
        }

        try
        {
            Reference< XDataPilotFieldGrouping > xDPGrouping( rxBaseDPField, UNO_QUERY_THROW );
            xDPGroupField = xDPGrouping->createDateGroup( aGroupInfo );
        }
        catch( Exception& )
        {
        }
    }

    Reference< XNamed > xFieldName( xDPGroupField, UNO_QUERY );
    return xFieldName.is() ? xFieldName->getName() : OUString();
}

OUString PivotCacheField::createParentGroupField( const Reference< XDataPilotField >& rxBaseDPField, const PivotCacheField& rBaseCacheField, PivotCacheGroupItemVector& orItemNames ) const
{
    SAL_WARN_IF( !hasGroupItems() || maDiscreteItems.empty(), "sc", "PivotCacheField::createParentGroupField - not a group field" );
    SAL_WARN_IF( maDiscreteItems.size() != orItemNames.size(), "sc", "PivotCacheField::createParentGroupField - number of item names does not match grouping info" );
    Reference< XDataPilotFieldGrouping > xDPGrouping( rxBaseDPField, UNO_QUERY );
    if( !xDPGrouping.is() ) return OUString();

    // map the group item indexes from maGroupItems to all item indexes from maDiscreteItems
    std::vector< std::vector<sal_Int32> > aItemMap( maGroupItems.size() );
    sal_Int32 nIndex = -1;
    for( const auto& rDiscreteItem : maDiscreteItems )
    {
        ++nIndex;
        if( std::vector<sal_Int32>* pItems = ContainerHelper::getVectorElementAccess( aItemMap, rDiscreteItem ) )
        {
            if ( const PivotCacheItem* pItem = rBaseCacheField.getCacheItems().getCacheItem( nIndex ) )
            {
                // Skip unspecified or unused entries or errors
                if ( pItem->isUnused() || ( pItem->getType() == XML_m ) ||  ( pItem->getType() == XML_e ) )
                    continue;
            }
            pItems->push_back( nIndex );
        }
    }

    // process all groups
    Reference< XDataPilotField > xDPGroupField;
    nIndex = 0;
    for( const auto& rItems : aItemMap )
    {
        SAL_WARN_IF( rItems.empty(), "sc", "PivotCacheField::createParentGroupField - item/group should not be empty" );
        if( !rItems.empty() )
        {
            /*  Insert the names of the items that are part of this group. Calc
                expects the names of the members of the field whose members are
                grouped (which may be the names of groups too). Excel provides
                the names of the base field items instead (no group names
                involved). Therefore, the passed collection of current item
                names as they are already grouped is used here to resolve the
                item names. */
            ::std::vector< OUString > aMembers;
            for( auto i : rItems )
                if( const PivotCacheGroupItem* pName = ContainerHelper::getVectorElement( orItemNames, i ) )
                    if( ::std::find( aMembers.begin(), aMembers.end(), pName->maGroupName ) == aMembers.end() )
                        aMembers.push_back( pName->maGroupName );

            /*  Check again, that this is not just a group that is not grouped
                further with other items. */
            if( !aMembers.empty() ) try
            {
                // only the first call of createNameGroup() returns the new field
                Reference< XDataPilotField > xDPNewField = xDPGrouping->createNameGroup( ContainerHelper::vectorToSequence( aMembers ) );
                SAL_WARN_IF( xDPGroupField.is() == xDPNewField.is(), "sc", "PivotCacheField::createParentGroupField - missing group field" );
                if( !xDPGroupField.is() )
                    xDPGroupField = xDPNewField;

                // get current grouping info
                DataPilotFieldGroupInfo aGroupInfo;
                PropertySet aPropSet( xDPGroupField );
                aPropSet.getProperty( aGroupInfo, PROP_GroupInfo );

                /*  Find the group object and the auto-generated group name.
                    The returned field contains all groups derived from the
                    previous field if that is grouped too. To find the correct
                    group, the first item used to create the group is searched.
                    Calc provides the original item names of the base field
                    when the group is querried for its members. Its does not
                    provide the names of members that are already groups in the
                    field used to create the new groups. (Is this a bug?)
                    Therefore, a name from the passed list of original item
                    names is used to find the correct group. */
                OUString aFirstItem;
                if( const PivotCacheGroupItem* pName = ContainerHelper::getVectorElement( orItemNames, rItems.front() ) )
                    aFirstItem = pName->maOrigName;
                Reference< XNamed > xGroupName;
                OUString aAutoName;
                Reference< XIndexAccess > xGroupsIA( aGroupInfo.Groups, UNO_QUERY_THROW );
                for( sal_Int32 nIdx = 0, nCount = xGroupsIA->getCount(); (nIdx < nCount) && (aAutoName.isEmpty()); ++nIdx ) try
                {
                    Reference< XNameAccess > xItemsNA( xGroupsIA->getByIndex( nIdx ), UNO_QUERY_THROW );
                    if( xItemsNA->hasByName( aFirstItem ) )
                    {
                        xGroupName.set( xGroupsIA->getByIndex( nIdx ), UNO_QUERY_THROW );
                        aAutoName = xGroupName->getName();
                    }
                }
                catch( Exception const & )
                {
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN("sc", "PivotCacheField::createParentGroupField - exception was thrown " << exceptionToString(ex) );
                }
                SAL_WARN_IF( aAutoName.isEmpty(), "sc", "PivotCacheField::createParentGroupField - cannot find auto-generated group name" );

                // get the real group name from the list of group items
                OUString aGroupName;
                if( const PivotCacheItem* pGroupItem = maGroupItems.getCacheItem( nIndex ) )
                    aGroupName = pGroupItem->getName();
                SAL_WARN_IF( aGroupName.isEmpty(), "sc", "PivotCacheField::createParentGroupField - cannot find group name" );
                if( aGroupName.isEmpty() )
                    aGroupName = aAutoName;

                if( xGroupName.is() && !aGroupName.isEmpty() )
                {
                    // replace the auto-generated group name with the real name
                    if( aAutoName != aGroupName )
                    {
                        xGroupName->setName( aGroupName );
                        aPropSet.setProperty( PROP_GroupInfo, aGroupInfo );
                    }
                    // replace original item names in passed vector with group name
                    for( auto i : rItems )
                        if( PivotCacheGroupItem* pName = ContainerHelper::getVectorElementAccess( orItemNames, i ) )
                            pName->maGroupName = aGroupName;
                }
            }
            catch( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN("sc", "PivotCacheField::createParentGroupField - exception was thrown " << exceptionToString(ex) );
            }
        }
        ++nIndex;
    }

    Reference< XNamed > xFieldName( xDPGroupField, UNO_QUERY );
    return xFieldName.is() ? xFieldName->getName() : OUString();
}

void PivotCacheField::writeSourceHeaderCell( const WorksheetHelper& rSheetHelper, sal_Int32 nCol, sal_Int32 nRow ) const
{
    CellModel aModel;
    aModel.maCellAddr = ScAddress( SCCOL( nCol ), SCROW( nRow ), rSheetHelper.getSheetIndex() );
    rSheetHelper.getSheetData().setStringCell( aModel, maFieldModel.maName );
}

void PivotCacheField::writeSourceDataCell( const WorksheetHelper& rSheetHelper, sal_Int32 nCol, sal_Int32 nRow, const PivotCacheItem& rItem ) const
{
    bool bHasIndex = rItem.getType() == XML_x;
    OSL_ENSURE( bHasIndex != maSharedItems.empty(), "PivotCacheField::writeSourceDataCell - shared items missing or not expected" );
    if( bHasIndex )
        writeSharedItemToSourceDataCell( rSheetHelper, nCol, nRow, rItem.getValue().get< sal_Int32 >() );
    else
        writeItemToSourceDataCell( rSheetHelper, nCol, nRow, rItem );
}

void PivotCacheField::importPCRecordItem( SequenceInputStream& rStrm, const WorksheetHelper& rSheetHelper, sal_Int32 nCol, sal_Int32 nRow ) const
{
    if( hasSharedItems() )
    {
        writeSharedItemToSourceDataCell( rSheetHelper, nCol, nRow, rStrm.readInt32() );
    }
    else
    {
        PivotCacheItem aItem;
        if( maSharedItemsModel.mbIsNumeric )
           aItem.readDouble( rStrm );
        else if( maSharedItemsModel.mbHasDate && !maSharedItemsModel.mbHasString )
           aItem.readDate( rStrm );
        else
           aItem.readString( rStrm );
        writeItemToSourceDataCell( rSheetHelper, nCol, nRow, aItem );
    }
}

// private --------------------------------------------------------------------

void PivotCacheField::writeItemToSourceDataCell( const WorksheetHelper& rSheetHelper,
        sal_Int32 nCol, sal_Int32 nRow, const PivotCacheItem& rItem )
{
    if( rItem.getType() != XML_m )
    {
        CellModel aModel;
        aModel.maCellAddr = ScAddress( SCCOL( nCol ), SCROW( nRow ), rSheetHelper.getSheetIndex() );
        SheetDataBuffer& rSheetData = rSheetHelper.getSheetData();
        switch( rItem.getType() )
        {
            case XML_s: rSheetData.setStringCell( aModel, rItem.getValue().get< OUString >() );                             break;
            case XML_n: rSheetData.setValueCell( aModel, rItem.getValue().get< double >() );                                break;
            case XML_i: rSheetData.setValueCell( aModel, rItem.getValue().get< sal_Int16 >() );                             break;
            case XML_d: rSheetData.setDateTimeCell( aModel, rItem.getValue().get< css::util::DateTime >() );                           break;
            case XML_b: rSheetData.setBooleanCell( aModel, rItem.getValue().get< bool >() );                                break;
            case XML_e: rSheetData.setErrorCell( aModel, static_cast< sal_uInt8 >( rItem.getValue().get< sal_Int32 >() ) ); break;
            default:    OSL_FAIL( "PivotCacheField::writeItemToSourceDataCell - unexpected item data type" );
        }
    }
}

void PivotCacheField::writeSharedItemToSourceDataCell(
        const WorksheetHelper& rSheetHelper, sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nItemIdx ) const
{
    if( const PivotCacheItem* pCacheItem = maSharedItems.getCacheItem( nItemIdx ) )
        writeItemToSourceDataCell( rSheetHelper, nCol, nRow, *pCacheItem );
}

PCDefinitionModel::PCDefinitionModel() :
    mfRefreshedDate( 0.0 ),
    mnRecords( 0 ),
    mnMissItemsLimit( 0 ),
    mbInvalid( false ),
    mbSaveData( true ),
    mbRefreshOnLoad( false ),
    mbOptimizeMemory( false ),
    mbEnableRefresh( true ),
    mbBackgroundQuery( false ),
    mbUpgradeOnRefresh( false ),
    mbTupleCache( false ),
    mbSupportSubquery( false ),
    mbSupportDrill( false )
{
}

PCSourceModel::PCSourceModel() :
    mnSourceType( XML_TOKEN_INVALID ),
    mnConnectionId( 0 )
{
}

PCWorksheetSourceModel::PCWorksheetSourceModel()
{
    maRange.SetInvalid();
}

PivotCache::PivotCache( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnCurrRow( -1 ),
    mbValidSource( false ),
    mbDummySheet( false )
{
}

void PivotCache::importPivotCacheDefinition( const AttributeList& rAttribs )
{
    maDefModel.maRelId            = rAttribs.getString( R_TOKEN( id ), OUString() );
    maDefModel.maRefreshedBy      = rAttribs.getXString( XML_refreshedBy, OUString() );
    maDefModel.mfRefreshedDate    = rAttribs.getDouble( XML_refreshedDate, 0.0 );
    maDefModel.mnRecords          = rAttribs.getInteger( XML_recordCount, 0 );
    maDefModel.mnMissItemsLimit   = rAttribs.getInteger( XML_missingItemsLimit, 0 );
    maDefModel.mbInvalid          = rAttribs.getBool( XML_invalid, false );
    maDefModel.mbSaveData         = rAttribs.getBool( XML_saveData, true );
    maDefModel.mbRefreshOnLoad    = rAttribs.getBool( XML_refreshOnLoad, false );
    maDefModel.mbOptimizeMemory   = rAttribs.getBool( XML_optimizeMemory, false );
    maDefModel.mbEnableRefresh    = rAttribs.getBool( XML_enableRefresh, true );
    maDefModel.mbBackgroundQuery  = rAttribs.getBool( XML_backgroundQuery, false );
    maDefModel.mbUpgradeOnRefresh = rAttribs.getBool( XML_upgradeOnRefresh, false );
    maDefModel.mbTupleCache       = rAttribs.getBool( XML_tupleCache, false );
    maDefModel.mbSupportSubquery  = rAttribs.getBool( XML_supportSubquery, false );
    maDefModel.mbSupportDrill     = rAttribs.getBool( XML_supportAdvancedDrill, false );
}

void PivotCache::importCacheSource( const AttributeList& rAttribs )
{
    maSourceModel.mnSourceType   = rAttribs.getToken( XML_type, XML_TOKEN_INVALID );
    maSourceModel.mnConnectionId = rAttribs.getInteger( XML_connectionId, 0 );
}

void PivotCache::importWorksheetSource( const AttributeList& rAttribs, const Relations& rRelations )
{
    maSheetSrcModel.maRelId   = rAttribs.getString( R_TOKEN( id ), OUString() );
    maSheetSrcModel.maSheet   = rAttribs.getXString( XML_sheet, OUString() );
    maSheetSrcModel.maDefName = rAttribs.getXString( XML_name, OUString() );

    // resolve URL of external document
    maTargetUrl = rRelations.getExternalTargetFromRelId( maSheetSrcModel.maRelId );
    // store range address unchecked with sheet index 0, will be resolved/checked later
    AddressConverter::convertToCellRangeUnchecked( maSheetSrcModel.maRange, rAttribs.getString( XML_ref, OUString() ), 0 );
}

void PivotCache::importPCDefinition( SequenceInputStream& rStrm )
{
    sal_uInt8 nFlags1, nFlags2;
    rStrm.skip( 3 );    // create/refresh version id's
    nFlags1 = rStrm.readuChar();
    maDefModel.mnMissItemsLimit = rStrm.readInt32();
    maDefModel.mfRefreshedDate = rStrm.readDouble();
    nFlags2 = rStrm.readuChar();
    maDefModel.mnRecords = rStrm.readInt32();
    if( getFlag( nFlags2, BIFF12_PCDEFINITION_HASUSERNAME ) )
        rStrm >> maDefModel.maRefreshedBy;
    if( getFlag( nFlags2, BIFF12_PCDEFINITION_HASRELID ) )
        rStrm >> maDefModel.maRelId;

    maDefModel.mbInvalid          = getFlag( nFlags1, BIFF12_PCDEFINITION_INVALID );
    maDefModel.mbSaveData         = getFlag( nFlags1, BIFF12_PCDEFINITION_SAVEDATA );
    maDefModel.mbRefreshOnLoad    = getFlag( nFlags1, BIFF12_PCDEFINITION_REFRESHONLOAD );
    maDefModel.mbOptimizeMemory   = getFlag( nFlags1, BIFF12_PCDEFINITION_OPTIMIZEMEMORY );
    maDefModel.mbEnableRefresh    = getFlag( nFlags1, BIFF12_PCDEFINITION_ENABLEREFRESH );
    maDefModel.mbBackgroundQuery  = getFlag( nFlags1, BIFF12_PCDEFINITION_BACKGROUNDQUERY );
    maDefModel.mbUpgradeOnRefresh = getFlag( nFlags1, BIFF12_PCDEFINITION_UPGRADEONREFR );
    maDefModel.mbTupleCache       = getFlag( nFlags1, BIFF12_PCDEFINITION_TUPELCACHE );
    maDefModel.mbSupportSubquery  = getFlag( nFlags2, BIFF12_PCDEFINITION_SUPPORTSUBQUERY );
    maDefModel.mbSupportDrill     = getFlag( nFlags2, BIFF12_PCDEFINITION_SUPPORTDRILL );
}

void PivotCache::importPCDSource( SequenceInputStream& rStrm )
{
    sal_Int32 nSourceType;
    nSourceType = rStrm.readInt32();
    maSourceModel.mnConnectionId = rStrm.readInt32();
    static const sal_Int32 spnSourceTypes[] = { XML_worksheet, XML_external, XML_consolidation, XML_scenario };
    maSourceModel.mnSourceType = STATIC_ARRAY_SELECT( spnSourceTypes, nSourceType, XML_TOKEN_INVALID );
}

void PivotCache::importPCDSheetSource( SequenceInputStream& rStrm, const Relations& rRelations )
{
    sal_uInt8 nIsDefName, nIsBuiltinName, nFlags;
    nIsDefName = rStrm.readuChar();
    nIsBuiltinName = rStrm.readuChar();
    nFlags = rStrm.readuChar();
    if( getFlag( nFlags, BIFF12_PCDWBSOURCE_HASSHEET ) )
        rStrm >> maSheetSrcModel.maSheet;
    if( getFlag( nFlags, BIFF12_PCDWBSOURCE_HASRELID ) )
        rStrm >> maSheetSrcModel.maRelId;

    // read cell range or defined name
    if( nIsDefName == 0 )
    {
        BinRange aBinRange;
        rStrm >> aBinRange;
        // store range address unchecked with sheet index 0, will be resolved/checked later
        AddressConverter::convertToCellRangeUnchecked( maSheetSrcModel.maRange, aBinRange, 0 );
    }
    else
    {
        rStrm >> maSheetSrcModel.maDefName;
        if( nIsBuiltinName != 0 )
            maSheetSrcModel.maDefName = "_xlnm." + maSheetSrcModel.maDefName;
    }

    // resolve URL of external document
    maTargetUrl = rRelations.getExternalTargetFromRelId( maSheetSrcModel.maRelId );
}

PivotCacheField& PivotCache::createCacheField()
{
    PivotCacheFieldVector::value_type xCacheField( new PivotCacheField( *this, true/*bIsDatabaseField*/ ) );
    maFields.push_back( xCacheField );
    return *xCacheField;
}

void PivotCache::finalizeImport()
{
    // collect all fields that are based on source data (needed to finalize source data below)
    OSL_ENSURE( !maFields.empty(), "PivotCache::finalizeImport - no pivot cache fields found" );
    for( PivotCacheFieldVector::const_iterator aIt = maFields.begin(), aEnd = maFields.end(); aIt != aEnd; ++aIt )
    {
        if( (*aIt)->isDatabaseField() )
        {
            OSL_ENSURE( (aIt == maFields.begin()) || (*(aIt - 1))->isDatabaseField(),
                "PivotCache::finalizeImport - database field follows a calculated field" );
            maDatabaseIndexes.push_back( static_cast< sal_Int32 >( maDatabaseFields.size() ) );
            maDatabaseFields.push_back( *aIt );
        }
        else
        {
            maDatabaseIndexes.push_back( -1 );
        }
    }
    OSL_ENSURE( !maDatabaseFields.empty(), "PivotCache::finalizeImport - no pivot cache source fields found" );

    // finalize source data depending on source type
    switch( maSourceModel.mnSourceType )
    {
        case XML_worksheet:
        {
            // decide whether an external document is used
            bool bInternal = maTargetUrl.isEmpty() && maSheetSrcModel.maRelId.isEmpty();
            bool bExternal = !maTargetUrl.isEmpty();   // relation ID may be empty, e.g. BIFF import
            OSL_ENSURE( bInternal || bExternal, "PivotCache::finalizeImport - invalid external document URL" );
            if( bInternal )
                finalizeInternalSheetSource();
            else if( bExternal )
                finalizeExternalSheetSource();
        }
        break;

        // currently, we only support worksheet data sources
        case XML_external:
        break;
        case XML_consolidation:
        break;
        case XML_scenario:
        break;
    }
}

PivotCacheField* PivotCache::getCacheField( sal_Int32 nFieldIdx )
{
    return maFields.get( nFieldIdx ).get();
}

const PivotCacheField* PivotCache::getCacheField( sal_Int32 nFieldIdx ) const
{
    return maFields.get( nFieldIdx ).get();
}

sal_Int32 PivotCache::getCacheDatabaseIndex( sal_Int32 nFieldIdx ) const
{
    return ContainerHelper::getVectorElement( maDatabaseIndexes, nFieldIdx, -1 );
}

void PivotCache::writeSourceHeaderCells( const WorksheetHelper& rSheetHelper ) const
{
    OSL_ENSURE( static_cast< size_t >( maSheetSrcModel.maRange.aEnd.Col() - maSheetSrcModel.maRange.aStart.Col() + 1 ) == maDatabaseFields.size(),
        "PivotCache::writeSourceHeaderCells - source cell range width does not match number of source fields" );
    SCCOL nCol = maSheetSrcModel.maRange.aStart.Col();
    SCCOL nMaxCol = getAddressConverter().getMaxApiAddress().Col();
    SCROW nRow = maSheetSrcModel.maRange.aStart.Row();
    mnCurrRow = -1;
    updateSourceDataRow( rSheetHelper, nRow );
    for( const auto& rxDatabaseField : maDatabaseFields )
    {
        if (nCol > nMaxCol)
            break;
        rxDatabaseField->writeSourceHeaderCell( rSheetHelper, nCol, nRow );
        ++nCol;
    }
}

void PivotCache::writeSourceDataCell( const WorksheetHelper& rSheetHelper, sal_Int32 nColIdx, sal_Int32 nRowIdx, const PivotCacheItem& rItem ) const
{
    SCCOL nCol = maSheetSrcModel.maRange.aStart.Col() + nColIdx;
    OSL_ENSURE( ( maSheetSrcModel.maRange.aStart.Col() <= nCol ) && ( nCol <= maSheetSrcModel.maRange.aEnd.Col() ), "PivotCache::writeSourceDataCell - invalid column index" );
    SCROW nRow = maSheetSrcModel.maRange.aStart.Row() + nRowIdx;
    OSL_ENSURE( ( maSheetSrcModel.maRange.aStart.Row() < nRow ) && ( nRow <= maSheetSrcModel.maRange.aEnd.Row() ), "PivotCache::writeSourceDataCell - invalid row index" );
    updateSourceDataRow( rSheetHelper, nRow );
    if( const PivotCacheField* pCacheField = maDatabaseFields.get( nColIdx ).get() )
        pCacheField->writeSourceDataCell( rSheetHelper, nCol, nRow, rItem );
}

void PivotCache::importPCRecord( SequenceInputStream& rStrm, const WorksheetHelper& rSheetHelper, sal_Int32 nRowIdx ) const
{
    SCROW nRow = maSheetSrcModel.maRange.aStart.Row() + nRowIdx;
    OSL_ENSURE( ( maSheetSrcModel.maRange.aStart.Row() < nRow ) && ( nRow <= maSheetSrcModel.maRange.aEnd.Row() ), "PivotCache::importPCRecord - invalid row index" );
    SCCOL nCol = maSheetSrcModel.maRange.aStart.Col();
    SCCOL nMaxCol = getAddressConverter().getMaxApiAddress().Col();
    for( const auto& rxDatabaseField : maDatabaseFields )
    {
        if( rStrm.isEof() || (nCol > nMaxCol) )
            break;
        rxDatabaseField->importPCRecordItem( rStrm, rSheetHelper, nCol, nRow );
        ++nCol;
    }
}

// private --------------------------------------------------------------------

void PivotCache::finalizeInternalSheetSource()
{
    // resolve sheet name to sheet index
    sal_Int16 nSheet = getWorksheets().getCalcSheetIndex( maSheetSrcModel.maSheet );

    // if cache is based on a defined name or table, try to resolve to cell range
    if( !maSheetSrcModel.maDefName.isEmpty() )
    {
        // local or global defined name
        if( const DefinedName* pDefName = getDefinedNames().getByModelName( maSheetSrcModel.maDefName, nSheet ).get() )
        {
            mbValidSource = pDefName->getAbsoluteRange( maSheetSrcModel.maRange );
        }
        // table
        else if( const Table* pTable = getTables().getTable( maSheetSrcModel.maDefName ).get() )
        {
            // get original range from table, but exclude the totals row(s)
            maSheetSrcModel.maRange = pTable->getOriginalRange();
            mbValidSource = (pTable->getHeight() - pTable->getTotalsRows()) > 1;
            if( mbValidSource )
                maSheetSrcModel.maRange.aEnd.SetRow( maSheetSrcModel.maRange.aEnd.Row() - pTable->getTotalsRows() );
        }
    }
    // else try the cell range (if the sheet exists)
    else if( nSheet >= 0 )
    {
        // insert sheet index into the range, range address will be checked below
        maSheetSrcModel.maRange.aStart.SetTab( nSheet );
        mbValidSource = true;
    }
    // else sheet has been deleted, generate the source data from cache
    else if( !maSheetSrcModel.maSheet.isEmpty() )
    {
        prepareSourceDataSheet();
        // return here to skip the source range check below
        return;
    }

    // check range location, do not allow ranges that overflow the sheet partly
    mbValidSource = mbValidSource &&
        getAddressConverter().checkCellRange( maSheetSrcModel.maRange, false, true ) &&
            ( maSheetSrcModel.maRange.aStart.Row() < maSheetSrcModel.maRange.aEnd.Row() );
}

void PivotCache::finalizeExternalSheetSource()
{
    /*  If pivot cache is based on external sheet data, try to restore sheet
        data from cache records. No support for external defined names or tables,
        sheet name and path to cache records fragment (OOXML only) are required. */
    bool bHasRelation = !maDefModel.maRelId.isEmpty();
    if( bHasRelation && maSheetSrcModel.maDefName.isEmpty() && !maSheetSrcModel.maSheet.isEmpty() )
        prepareSourceDataSheet();
}

void PivotCache::prepareSourceDataSheet()
{
    ScRange& rRange = maSheetSrcModel.maRange;
    // data will be inserted in top-left cell, sheet index is still set to 0 (will be set below)
    rRange.aEnd.SetCol( rRange.aEnd.Col() - rRange.aStart.Col() );
    rRange.aStart.SetCol( 0 );
    rRange.aEnd.SetRow( rRange.aEnd.Row() - rRange.aStart.Row() );
    rRange.aStart.SetRow( 0 );
    // check range location, do not allow ranges that overflow the sheet partly
    if( getAddressConverter().checkCellRange( rRange, false, true ) )
    {
        maColSpans.insert( ValueRange( rRange.aStart.Col(), rRange.aEnd.Col() ) );
        OUString aSheetName = "DPCache_" + maSheetSrcModel.maSheet;
        rRange.aStart.SetTab( getWorksheets().insertEmptySheet( aSheetName ) );
        mbValidSource = mbDummySheet = rRange.aStart.Tab() >= 0;
    }
}

void PivotCache::updateSourceDataRow( const WorksheetHelper& rSheetHelper, sal_Int32 nRow ) const
{
    if( mnCurrRow != nRow )
    {
        rSheetHelper.getSheetData().setColSpans( nRow, maColSpans );
        mnCurrRow = nRow;
    }
}

PivotCacheBuffer::PivotCacheBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void PivotCacheBuffer::registerPivotCacheFragment( sal_Int32 nCacheId, const OUString& rFragmentPath )
{
    OSL_ENSURE( nCacheId >= 0, "PivotCacheBuffer::registerPivotCacheFragment - invalid pivot cache identifier" );
    OSL_ENSURE( maFragmentPaths.count( nCacheId ) == 0, "PivotCacheBuffer::registerPivotCacheFragment - fragment path exists already" );
    if( (nCacheId >= 0) && !rFragmentPath.isEmpty() )
        maFragmentPaths[ nCacheId ] = rFragmentPath;
}

PivotCache* PivotCacheBuffer::importPivotCacheFragment( sal_Int32 nCacheId )
{
    /*  OOXML/BIFF12 filter: On first call for the cache ID, the pivot
        cache object is created and inserted into maCaches. Then, the cache
        definition fragment is read and the cache is returned. On
        subsequent calls, the created cache will be found in maCaches and
        returned immediately. */
    // try to find an imported pivot cache
    if( PivotCache* pCache = maCaches.get( nCacheId ).get() )
        return pCache;

    // check if a fragment path exists for the passed cache identifier
    FragmentPathMap::iterator aIt = maFragmentPaths.find( nCacheId );
    if( aIt == maFragmentPaths.end() )
        return nullptr;

    /*  Import the cache fragment. This may create a dummy data sheet
        for external sheet sources. */
    PivotCache& rCache = createPivotCache( nCacheId );
    importOoxFragment( new PivotCacheDefinitionFragment( *this, aIt->second, rCache ) );
    return &rCache;
}

PivotCache& PivotCacheBuffer::createPivotCache( sal_Int32 nCacheId )
{
    maCacheIds.push_back( nCacheId );
    PivotCacheMap::mapped_type& rxCache = maCaches[ nCacheId ];
    rxCache.reset( new PivotCache( *this ) );
    return *rxCache;
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
