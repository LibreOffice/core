/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xedbdata.hxx"
#include "document.hxx"
#include "address.hxx"
#include "globstr.hrc"

#include <oox/token/tokens.hxx>

using namespace oox;

XclExpXmlDBDataStyleInfo::XclExpXmlDBDataStyleInfo( const XclExpRoot& rRoot, ScDBDataFormatting& rDBDataFormatting )
    : XclExpRoot( rRoot ),
    maDBDataFormatting( rDBDataFormatting )
{
}

void XclExpXmlDBDataStyleInfo::SaveXml( XclExpXmlStream& rStrm )
{
   sax_fastparser::FSHelperPtr& rDBDataTable = rStrm.GetCurrentStream();
   rDBDataTable->singleElement( XML_tableStyleInfo, XML_name, OUStringToOString(maDBDataFormatting.GetTableStyleName(), RTL_TEXTENCODING_UTF8 ).getStr(), XML_showFirstColumn, XclXmlUtils::ToPsz( false ), XML_showLastColumn, XclXmlUtils::ToPsz( false ), XML_showRowStripes, XclXmlUtils::ToPsz( maDBDataFormatting.GetBandedRows() ), XML_showColumnStripes, XclXmlUtils::ToPsz( maDBDataFormatting.GetBandedColumns() ),FSEND );// hardcoded two values for functions not supported yet
}

// ============================================================================
XclExpXmlDBDataColumn::XclExpXmlDBDataColumn( const XclExpRoot& rRoot, int iID, OUString& rName )
    : XclExpRoot( rRoot ),
      maName( rName ),
      miID( iID )
{
}

void XclExpXmlDBDataColumn::SaveXml( XclExpXmlStream& rStrm )
{
   sax_fastparser::FSHelperPtr& rDBDataTable = rStrm.GetCurrentStream();
   rDBDataTable->singleElement( XML_tableColumn, XML_id, OString::number( miID ).getStr(), XML_name, OUStringToOString( maName, RTL_TEXTENCODING_UTF8 ), FSEND );
}

// ============================================================================
XclExpXmlDBDataColumns::XclExpXmlDBDataColumns( const XclExpRoot& rRoot, ScDBData& rDBData )
    : XclExpRoot( rRoot )
{
    ScRange aRange;
    rDBData.GetArea( aRange );
    SCROW anRow1, anRow2;
    SCCOL anCol1, anCol2, anTotalCols;
    SCTAB anTab1, anTab2;
    aRange.GetVars( anCol1, anRow1, anTab1, anCol2, anRow2, anTab2 );
    anTotalCols = (anCol2 - anCol1) + 1; //addressing starts from 0
    // Needs special handling for different tab ranges
    OUString aColName = "Column";
    for( int i = 1; i <= anTotalCols; i++ )
    {
        OUString aStri = aColName + OUString::number( i );
        maDBDataColumnContainer.push_back( new XclExpXmlDBDataColumn( rRoot, i, aStri ) );
    }
}

void XclExpXmlDBDataColumns::SaveXml( XclExpXmlStream& rStrm )
{
   sax_fastparser::FSHelperPtr& rDBDataTable = rStrm.GetCurrentStream();
   rDBDataTable->startElement( XML_tableColumns, XML_count, OString::number( maDBDataColumnContainer.size() ).getStr(), FSEND );
    for ( DBDataColumnContainer::iterator itr = maDBDataColumnContainer.begin(); itr != maDBDataColumnContainer.end(); ++itr )
    {
        itr->SaveXml( rStrm );
    }
    rDBDataTable->endElement( XML_tableColumns );
}

// ============================================================================
XclExpXmlDBDataAutoFilter::XclExpXmlDBDataAutoFilter( const XclExpRoot& rRoot, ScRange& rRange )
    : XclExpRoot( rRoot ),
      maRange( rRange )
{
}

void XclExpXmlDBDataAutoFilter::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rDBDataTable = rStrm.GetCurrentStream();
    rDBDataTable->singleElement( XML_autoFilter, XML_ref, XclXmlUtils::ToOString( maRange ).getStr(), FSEND);
}

// ============================================================================
XclExpXmlDBDataTable::XclExpXmlDBDataTable(const XclExpRoot& rRoot, ScDBData& rDBData, int nTableId )
    : XclExpRoot( rRoot ),
      mnTableId( nTableId )
{
    maTableColumns.reset( new XclExpXmlDBDataColumns( rRoot, rDBData ) );
    ScDBDataFormatting aDBFormatting;
    rDBData.GetTableFormatting( aDBFormatting );
    maName = OUString("Table") + OUString::number( mnTableId );
    maDisplayName = rDBData.GetName();
    mbTotalsRowShown = false; // Not supported yet in ScDBData
    rDBData.GetArea( maRange );
    maStyleInfo.reset( new XclExpXmlDBDataStyleInfo( rRoot, aDBFormatting) );
    maAutoFilter.reset( new XclExpXmlDBDataAutoFilter( rRoot, maRange ) );
}
void XclExpXmlDBDataTable::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rDBDataTable = rStrm.GetCurrentStream();
    rDBDataTable->startElement( XML_table,  XML_id, OString::number( mnTableId ).getStr(), XML_name, OUStringToOString( maName, RTL_TEXTENCODING_UTF8 ).getStr(), XML_displayName, OUStringToOString( maDisplayName, RTL_TEXTENCODING_UTF8 ).getStr(), XML_ref, XclXmlUtils::ToOString( maRange ).getStr() ,FSEND);
    maAutoFilter->SaveXml( rStrm );
    maTableColumns->SaveXml( rStrm );
    maStyleInfo->SaveXml( rStrm );
    rDBDataTable->endElement( XML_table );

}

// =============================================================================
XclExpXmlDBDataTables::XclExpXmlDBDataTables( const XclExpRoot& rRoot )
    : XclExpRoot( rRoot )
{
    ScDBCollection* pDBCollection = rRoot.GetDoc().GetDBCollection();
    if( pDBCollection )
    {
        ScDBCollection::NamedDBs& aNamedDBs = pDBCollection->getNamedDBs();
        ScDBCollection::NamedDBs::iterator itr = aNamedDBs.begin();
        ScDBCollection::NamedDBs::iterator itrEnd = aNamedDBs.end();
        for(; itr!= itrEnd; ++itr)
        {
            maDBDataTableContainer.push_back( new XclExpXmlDBDataTable( rRoot, *itr, (maDBDataTableContainer.size() + 1) ) );// Tables are ID'ed 1 - based
        }
    }
}

void XclExpXmlDBDataTables::SaveXml( XclExpXmlStream& rStrm )
{
    // We only make the table folder is we do have any DB Data to write in
    if( maDBDataTableContainer.size() != 0 )
    {
        //Now parse through each of the DB Datas making an xml for each.
        int i = 1;
        for ( DBDataTableContainer::iterator itr = maDBDataTableContainer.begin(); itr != maDBDataTableContainer.end(); ++itr)
        {
            sax_fastparser::FSHelperPtr aDBDataTable = rStrm.CreateOutputStream(
                OUString( "xl/tables/table" )+ OUString::number( i ) + OUString(".xml" ),
                OUString( "table" ) + OUString::number( i ) + OUString( ".xml" ),
                rStrm.GetCurrentStream()->getOutputStream(),
                "application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" );// Last two parameters are a mystery
            rStrm.PushStream( aDBDataTable );
            //Now the table#.xml file is created, need to pass the stream to that table
            itr->SaveXml( rStrm );
            ++i;// increment counter
            rStrm.PopStream();
        }
    }
}
// =============================================================================
