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

#include <externallinkfragment.hxx>

#include <com/sun/star/sheet/XExternalSheetCache.hpp>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <osl/diagnose.h>
#include <addressconverter.hxx>
#include <unitconverter.hxx>
#include <biffhelper.hxx>

namespace oox::xls {

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

ExternalSheetDataContext::ExternalSheetDataContext(
        WorkbookFragmentBase& rFragment, const Reference< XExternalSheetCache >& rxSheetCache )
    : WorkbookContextBase(rFragment)
    , mxSheetCache(rxSheetCache)
    , mnCurrType(XML_TOKEN_INVALID)
{
    OSL_ENSURE( mxSheetCache.is(), "ExternalSheetDataContext::ExternalSheetDataContext - missing sheet cache" );
}

ContextHandlerRef ExternalSheetDataContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( sheetData ):
            if( nElement == XLS_TOKEN( row ) ) return this;
        break;
        case XLS_TOKEN( row ):
            if( nElement == XLS_TOKEN( cell ) ) { importCell( rAttribs ); return this; }
        break;
        case XLS_TOKEN( cell ):
            if( nElement == XLS_TOKEN( v ) ) return this;   // collect characters in onCharacters()
        break;
    }
    return nullptr;
}

void ExternalSheetDataContext::onCharacters( const OUString& rChars )
{
    if( !isCurrentElement( XLS_TOKEN( v ) ) )
        return;

    switch( mnCurrType )
    {
        case XML_b:
        case XML_n:
            setCellValue( Any( rChars.toDouble() ) );
        break;
        case XML_e:
            setCellValue( Any( BiffHelper::calcDoubleFromError( getUnitConverter().calcBiffErrorCode( rChars ) ) ) );
        break;
        case XML_str:
            setCellValue( Any( rChars ) );
        break;
    }
    mnCurrType = XML_TOKEN_INVALID;
}

ContextHandlerRef ExternalSheetDataContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_EXTSHEETDATA:
            if( nRecId == BIFF12_ID_EXTROW ) { maCurrPos.SetRow( rStrm.readInt32() ); return this; }
        break;
        case BIFF12_ID_EXTROW:
            switch( nRecId )
            {
                case BIFF12_ID_EXTCELL_BLANK:   importExtCellBlank( rStrm );    break;
                case BIFF12_ID_EXTCELL_BOOL:    importExtCellBool( rStrm );     break;
                case BIFF12_ID_EXTCELL_DOUBLE:  importExtCellDouble( rStrm );   break;
                case BIFF12_ID_EXTCELL_ERROR:   importExtCellError( rStrm );    break;
                case BIFF12_ID_EXTCELL_STRING:  importExtCellString( rStrm );   break;
            }
        break;
    }
    return nullptr;
}

// private --------------------------------------------------------------------

void ExternalSheetDataContext::importCell( const AttributeList& rAttribs )
{
    if( getAddressConverter().convertToCellAddress( maCurrPos, rAttribs.getString( XML_r, OUString() ), 0, false ) )
        mnCurrType = rAttribs.getToken( XML_t, XML_n );
    else
        mnCurrType = XML_TOKEN_INVALID;
}

void ExternalSheetDataContext::importExtCellBlank( SequenceInputStream& rStrm )
{
    maCurrPos.SetCol( rStrm.readInt32() );
    setCellValue( Any( OUString() ) );
}

void ExternalSheetDataContext::importExtCellBool( SequenceInputStream& rStrm )
{
    maCurrPos.SetCol( rStrm.readInt32() );
    double fValue = (rStrm.readuInt8() == 0) ? 0.0 : 1.0;
    setCellValue( Any( fValue ) );
}

void ExternalSheetDataContext::importExtCellDouble( SequenceInputStream& rStrm )
{
    maCurrPos.SetCol( rStrm.readInt32() );
    setCellValue( Any( rStrm.readDouble() ) );
}

void ExternalSheetDataContext::importExtCellError( SequenceInputStream& rStrm )
{
    maCurrPos.SetCol( rStrm.readInt32() );
    setCellValue( Any( BiffHelper::calcDoubleFromError( rStrm.readuInt8() ) ) );
}

void ExternalSheetDataContext::importExtCellString( SequenceInputStream& rStrm )
{
    maCurrPos.SetCol( rStrm.readInt32() );
    setCellValue( Any( BiffHelper::readString( rStrm ) ) );
}

void ExternalSheetDataContext::setCellValue( const Any& rValue )
{
    if( mxSheetCache.is() && getAddressConverter().checkCellAddress( maCurrPos, false ) ) try
    {
        mxSheetCache->setCellValue( maCurrPos.Col(), maCurrPos.Row(), rValue );
    }
    catch( Exception& )
    {
    }
}

ExternalLinkFragment::ExternalLinkFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, ExternalLink& rExtLink ) :
    WorkbookFragmentBase( rHelper, rFragmentPath ),
    mrExtLink( rExtLink ),
    mnResultType( XML_TOKEN_INVALID )
{
}

ContextHandlerRef ExternalLinkFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( externalLink ) ) return this;
        break;

        case XLS_TOKEN( externalLink ):
            switch( nElement )
            {
                case XLS_TOKEN( externalBook ): mrExtLink.importExternalBook( getRelations(), rAttribs );   return this;
                case XLS_TOKEN( ddeLink ):      mrExtLink.importDdeLink( rAttribs );                        return this;
                case XLS_TOKEN( oleLink ):      mrExtLink.importOleLink( getRelations(), rAttribs );        return this;
            }
        break;

        case XLS_TOKEN( externalBook ):
            switch( nElement )
            {
                case XLS_TOKEN( sheetNames ):
                case XLS_TOKEN( definedNames ):
                case XLS_TOKEN( sheetDataSet ): return this;
            }
        break;

        case XLS_TOKEN( sheetNames ):
            if( nElement == XLS_TOKEN( sheetName ) ) mrExtLink.importSheetName( rAttribs );
        break;
        case XLS_TOKEN( definedNames ):
            if( nElement == XLS_TOKEN( definedName ) ) mrExtLink.importDefinedName( rAttribs );
        break;
        case XLS_TOKEN( sheetDataSet ):
            if( (nElement == XLS_TOKEN( sheetData )) && (mrExtLink.getLinkType() == ExternalLinkType::External) )
                return createSheetDataContext( rAttribs.getInteger( XML_sheetId, -1 ) );
        break;

        case XLS_TOKEN( ddeLink ):
            if( nElement == XLS_TOKEN( ddeItems ) ) return this;
        break;
        case XLS_TOKEN( ddeItems ):
            if( nElement == XLS_TOKEN( ddeItem ) )
            {
                mxExtName = mrExtLink.importDdeItem( rAttribs );
                return this;
            }
        break;
        case XLS_TOKEN( ddeItem ):
            if( nElement == XLS_TOKEN( values ) )
            {
                if( mxExtName ) mxExtName->importValues( rAttribs );
                return this;
            }
        break;
        case XLS_TOKEN( values ):
            if( nElement == XLS_TOKEN( value ) )
            {
                mnResultType = rAttribs.getToken( XML_t, XML_n );
                return this;
            }
        break;
        case XLS_TOKEN( value ):
            if( nElement == XLS_TOKEN( val ) ) return this; // collect value in onCharacters()
        break;

        case XLS_TOKEN( oleLink ):
            if( nElement == XLS_TOKEN( oleItems ) ) return this;
        break;
        case XLS_TOKEN( oleItems ):
            if( nElement == XLS_TOKEN( oleItem ) ) mxExtName = mrExtLink.importOleItem( rAttribs );
        break;
    }
    return nullptr;
}

void ExternalLinkFragment::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( XLS_TOKEN( val ) ) )
        maResultValue = rChars;
}

void ExternalLinkFragment::onEndElement()
{
    if( !(isCurrentElement( XLS_TOKEN( value ) ) && mxExtName) )
        return;

    switch( mnResultType )
    {
        case XML_b:
            mxExtName->appendResultValue( maResultValue.toDouble() );
        break;
        case XML_e:
            mxExtName->appendResultValue( BiffHelper::calcDoubleFromError( getUnitConverter().calcBiffErrorCode( maResultValue ) ) );
        break;
        case XML_n:
            mxExtName->appendResultValue( maResultValue.toDouble() );
        break;
        case XML_str:
            mxExtName->appendResultValue( maResultValue );
        break;
        default:
            mxExtName->appendResultValue( BiffHelper::calcDoubleFromError( BIFF_ERR_NA ) );
    }
}

ContextHandlerRef ExternalLinkFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_EXTERNALBOOK )
            {
                mrExtLink.importExternalBook( getRelations(), rStrm );
                return this;
            }
        break;

        case BIFF12_ID_EXTERNALBOOK:
            switch( nRecId )
            {
                case BIFF12_ID_EXTSHEETDATA:
                    if( mrExtLink.getLinkType() == ExternalLinkType::External )
                        return createSheetDataContext( rStrm.readInt32() );
                break;

                case BIFF12_ID_EXTSHEETNAMES:       mrExtLink.importExtSheetNames( rStrm );                             break;
                case BIFF12_ID_EXTERNALNAME:        mxExtName = mrExtLink.importExternalName( rStrm );                  return this;
            }
        break;

        case BIFF12_ID_EXTERNALNAME:
            switch( nRecId )
            {
                case BIFF12_ID_EXTERNALNAMEFLAGS:   if( mxExtName ) mxExtName->importExternalNameFlags( rStrm );  break;
                case BIFF12_ID_DDEITEMVALUES:       if( mxExtName ) mxExtName->importDdeItemValues( rStrm );      return this;
            }
        break;

        case BIFF12_ID_DDEITEMVALUES:
            switch( nRecId )
            {
                case BIFF12_ID_DDEITEM_BOOL:        if( mxExtName ) mxExtName->importDdeItemBool( rStrm );        break;
                case BIFF12_ID_DDEITEM_DOUBLE:      if( mxExtName ) mxExtName->importDdeItemDouble( rStrm );      break;
                case BIFF12_ID_DDEITEM_ERROR:       if( mxExtName ) mxExtName->importDdeItemError( rStrm );       break;
                case BIFF12_ID_DDEITEM_STRING:      if( mxExtName ) mxExtName->importDdeItemString( rStrm );      break;
            }
        break;
    }
    return nullptr;
}

ContextHandlerRef ExternalLinkFragment::createSheetDataContext( sal_Int32 nSheetId )
{
    return new ExternalSheetDataContext( *this, mrExtLink.getSheetCache( nSheetId ) );
}

const RecordInfo* ExternalLinkFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_DDEITEMVALUES,  BIFF12_ID_DDEITEMVALUES + 1     },
        { BIFF12_ID_EXTERNALBOOK,   BIFF12_ID_EXTERNALBOOK + 228    },
        { BIFF12_ID_EXTERNALNAME,   BIFF12_ID_EXTERNALNAME + 10     },
        { BIFF12_ID_EXTROW,         -1                              },
        { BIFF12_ID_EXTSHEETDATA,   BIFF12_ID_EXTSHEETDATA + 1      },
        { -1,                       -1                              }
    };
    return spRecInfos;
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
