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

#include "chartsheetfragment.hxx"

#include <oox/helper/attributelist.hxx>
#include "biffinputstream.hxx"
#include "pagesettings.hxx"
#include "viewsettings.hxx"
#include "workbooksettings.hxx"
#include "worksheetsettings.hxx"

namespace oox {
namespace xls {

using namespace ::oox::core;

ChartsheetFragment::ChartsheetFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef ChartsheetFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( chartsheet ) ) return this;
        break;

        case XLS_TOKEN( chartsheet ):
            switch( nElement )
            {
                case XLS_TOKEN( sheetViews ):       return this;

                case XLS_TOKEN( sheetPr ):          getWorksheetSettings().importChartSheetPr( rAttribs );              break;
                case XLS_TOKEN( sheetProtection ):  getWorksheetSettings().importChartProtection( rAttribs );           break;
                case XLS_TOKEN( pageMargins ):      getPageSettings().importPageMargins( rAttribs );                    break;
                case XLS_TOKEN( pageSetup ):        getPageSettings().importChartPageSetup( getRelations(), rAttribs ); break;
                case XLS_TOKEN( headerFooter ):     getPageSettings().importHeaderFooter( rAttribs );                   return this;
                case XLS_TOKEN( picture ):          getPageSettings().importPicture( getRelations(), rAttribs );        break;
                case XLS_TOKEN( drawing ):          importDrawing( rAttribs );                                          break;
            }
        break;

        case XLS_TOKEN( sheetViews ):
            if( nElement == XLS_TOKEN( sheetView ) ) getSheetViewSettings().importChartSheetView( rAttribs );
        break;

        case XLS_TOKEN( headerFooter ):
            switch( nElement )
            {
                case XLS_TOKEN( firstHeader ):
                case XLS_TOKEN( firstFooter ):
                case XLS_TOKEN( oddHeader ):
                case XLS_TOKEN( oddFooter ):
                case XLS_TOKEN( evenHeader ):
                case XLS_TOKEN( evenFooter ):       return this;    // collect contents in onCharacters()
            }
        break;
    }
    return nullptr;
}

void ChartsheetFragment::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( firstHeader ):
        case XLS_TOKEN( firstFooter ):
        case XLS_TOKEN( oddHeader ):
        case XLS_TOKEN( oddFooter ):
        case XLS_TOKEN( evenHeader ):
        case XLS_TOKEN( evenFooter ):
            getPageSettings().importHeaderFooterCharacters( rChars, getCurrentElement() );
        break;
    }
}

ContextHandlerRef ChartsheetFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_WORKSHEET ) return this;
        break;

        case BIFF12_ID_WORKSHEET:
            switch( nRecId )
            {
                case BIFF12_ID_CHARTSHEETVIEWS:  return this;

                case BIFF12_ID_CHARTSHEETPR:    getWorksheetSettings().importChartSheetPr( rStrm );                 break;
                case BIFF12_ID_CHARTPROTECTION: getWorksheetSettings().importChartProtection( rStrm );              break;
                case BIFF12_ID_PAGEMARGINS:     getPageSettings().importPageMargins( rStrm );                       break;
                case BIFF12_ID_CHARTPAGESETUP:  getPageSettings().importChartPageSetup( getRelations(), rStrm );    break;
                case BIFF12_ID_HEADERFOOTER:    getPageSettings().importHeaderFooter( rStrm );                      break;
                case BIFF12_ID_PICTURE:         getPageSettings().importPicture( getRelations(), rStrm );           break;
                case BIFF12_ID_DRAWING:         importDrawing( rStrm );                                             break;
            }
        break;

        case BIFF12_ID_CHARTSHEETVIEWS:
            if( nRecId == BIFF12_ID_CHARTSHEETVIEW ) getSheetViewSettings().importChartSheetView( rStrm );
        break;
    }
    return nullptr;
}

const RecordInfo* ChartsheetFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_CHARTSHEETVIEW,     BIFF12_ID_CHARTSHEETVIEW + 1    },
        { BIFF12_ID_CHARTSHEETVIEWS,    BIFF12_ID_CHARTSHEETVIEWS + 1   },
        { BIFF12_ID_CUSTOMCHARTVIEW,    BIFF12_ID_CUSTOMCHARTVIEW + 1   },
        { BIFF12_ID_CUSTOMCHARTVIEWS,   BIFF12_ID_CUSTOMCHARTVIEWS + 1  },
        { BIFF12_ID_HEADERFOOTER,       BIFF12_ID_HEADERFOOTER + 1      },
        { BIFF12_ID_WORKSHEET,          BIFF12_ID_WORKSHEET + 1         },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

void ChartsheetFragment::initializeImport()
{
    // initial processing in base class WorksheetHelper
    initializeWorksheetImport();
}

void ChartsheetFragment::finalizeImport()
{
    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
}

// private --------------------------------------------------------------------

void ChartsheetFragment::importDrawing( const AttributeList& rAttribs )
{
    setDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) ) );
}

void ChartsheetFragment::importDrawing( SequenceInputStream& rStrm )
{
    setDrawingPath( getFragmentPathFromRelId( BiffHelper::readString( rStrm ) ) );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
