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

#include "stylesfragment.hxx"

#include <oox/helper/attributelist.hxx>

namespace oox {
namespace xls {

using namespace ::oox::core;

IndexedColorsContext::IndexedColorsContext( WorkbookFragmentBase& rFragment ) :
    WorkbookContextBase( rFragment )
{
}

ContextHandlerRef IndexedColorsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( indexedColors ):
            if( nElement == XLS_TOKEN( rgbColor ) ) getStyles().importPaletteColor( rAttribs );
        break;
    }
    return nullptr;
}

ContextHandlerRef IndexedColorsContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_INDEXEDCOLORS:
            if( nRecId == BIFF12_ID_RGBCOLOR ) getStyles().importPaletteColor( rStrm );
        break;
    }
    return nullptr;
}

ContextHandlerRef FontContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( mxFont.get() )
        mxFont->importAttribs( nElement, rAttribs );
    return nullptr;
}

void BorderContext::onStartElement( const AttributeList& rAttribs )
{
    if( mxBorder.get() && (getCurrentElement() == XLS_TOKEN( border )) )
        mxBorder->importBorder( rAttribs );
}

ContextHandlerRef BorderContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( mxBorder.get() ) switch( getCurrentElement() )
    {
        case XLS_TOKEN( border ):
            mxBorder->importStyle( nElement, rAttribs );
            return this;

        default:
            if( nElement == XLS_TOKEN( color ) )
                mxBorder->importColor( getCurrentElement(), rAttribs );
    }
    return nullptr;
}

ContextHandlerRef FillContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( mxFill.get() ) switch( getCurrentElement() )
    {
        case XLS_TOKEN( fill ):
            switch( nElement )
            {
                case XLS_TOKEN( patternFill ):  mxFill->importPatternFill( rAttribs );  return this;
                case XLS_TOKEN( gradientFill ): mxFill->importGradientFill( rAttribs ); return this;
            }
        break;
        case XLS_TOKEN( patternFill ):
            switch( nElement )
            {
                case XLS_TOKEN( fgColor ):      mxFill->importFgColor( rAttribs );      break;
                case XLS_TOKEN( bgColor ):      mxFill->importBgColor( rAttribs );      break;
            }
        break;
        case XLS_TOKEN( gradientFill ):
            if( nElement == XLS_TOKEN( stop ) )
            {
                mfGradPos = rAttribs.getDouble( XML_position, -1.0 );
                return this;
            }
        break;
        case XLS_TOKEN( stop ):
            if( nElement == XLS_TOKEN( color ) )
                mxFill->importColor( rAttribs, mfGradPos );
        break;
    }
    return nullptr;
}

void XfContext::onStartElement( const AttributeList& rAttribs )
{
    if( mxXf.get() && (getCurrentElement() == XLS_TOKEN( xf )) )
        mxXf->importXf( rAttribs, mbCellXf );
}

ContextHandlerRef XfContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( mxXf.get() ) switch( getCurrentElement() )
    {
        case XLS_TOKEN( xf ):
            switch( nElement )
            {
                case XLS_TOKEN( alignment ):    mxXf->importAlignment( rAttribs );  break;
                case XLS_TOKEN( protection ):   mxXf->importProtection( rAttribs ); break;
            }
        break;
    }
    return nullptr;
}

ContextHandlerRef DxfContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( mxDxf.get() ) switch( getCurrentElement() )
    {
        case XLS_TOKEN( dxf ):
            switch( nElement )
            {
                case XLS_TOKEN( font ):         return new FontContext( *this, mxDxf->createFont() );
                case XLS_TOKEN( border ):       return new BorderContext( *this, mxDxf->createBorder() );
                case XLS_TOKEN( fill ):         return new FillContext( *this, mxDxf->createFill() );

                case XLS_TOKEN( numFmt ):       mxDxf->importNumFmt( rAttribs );        break;
#if 0
                case XLS_TOKEN( alignment ):    mxDxf->importAlignment( rAttribs );     break;
                case XLS_TOKEN( protection ):   mxDxf->importProtection( rAttribs );    break;
#endif
            }
        break;
    }
    return nullptr;
}

StylesFragment::StylesFragment( const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    WorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef StylesFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( styleSheet ) ) return this;
        break;

        case XLS_TOKEN( styleSheet ):
            switch( nElement )
            {
                case XLS_TOKEN( colors ):
                case XLS_TOKEN( numFmts ):
                case XLS_TOKEN( fonts ):
                case XLS_TOKEN( borders ):
                case XLS_TOKEN( fills ):
                case XLS_TOKEN( cellXfs ):
                case XLS_TOKEN( cellStyleXfs ):
                case XLS_TOKEN( dxfs ):
                case XLS_TOKEN( cellStyles ):   return this;
            }
        break;

        case XLS_TOKEN( colors ):
            if( nElement == XLS_TOKEN( indexedColors ) ) return new IndexedColorsContext( *this );
        break;
        case XLS_TOKEN( numFmts ):
            if( nElement == XLS_TOKEN( numFmt ) ) getStyles().importNumFmt( rAttribs );
        break;
        case XLS_TOKEN( fonts ):
            if( nElement == XLS_TOKEN( font ) ) return new FontContext( *this, getStyles().createFont() );
        break;
        case XLS_TOKEN( borders ):
            if( nElement == XLS_TOKEN( border ) ) return new BorderContext( *this, getStyles().createBorder() );
        break;
        case XLS_TOKEN( fills ):
            if( nElement == XLS_TOKEN( fill ) ) return new FillContext( *this, getStyles().createFill() );
        break;
        case XLS_TOKEN( cellXfs ):
            if( nElement == XLS_TOKEN( xf ) ) return new XfContext( *this, getStyles().createCellXf(), true );
        break;
        case XLS_TOKEN( cellStyleXfs ):
            if( nElement == XLS_TOKEN( xf ) ) return new XfContext( *this, getStyles().createStyleXf(), false );
        break;
        case XLS_TOKEN( dxfs ):
            if( nElement == XLS_TOKEN( dxf ) ) return new DxfContext( *this, getStyles().createDxf() );
        break;
        case XLS_TOKEN( cellStyles ):
            if( nElement == XLS_TOKEN( cellStyle ) ) getStyles().importCellStyle( rAttribs );
        break;
    }
    return nullptr;
}

ContextHandlerRef StylesFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_STYLESHEET ) return this;
        break;

        case BIFF12_ID_STYLESHEET:
            switch( nRecId )
            {
                case BIFF12_ID_COLORS:
                case BIFF12_ID_NUMFMTS:
                case BIFF12_ID_FONTS:
                case BIFF12_ID_BORDERS:
                case BIFF12_ID_FILLS:
                case BIFF12_ID_CELLXFS:
                case BIFF12_ID_CELLSTYLEXFS:
                case BIFF12_ID_DXFS:
                case BIFF12_ID_CELLSTYLES:      return this;
            }
        break;

        case BIFF12_ID_COLORS:
            if( nRecId == BIFF12_ID_INDEXEDCOLORS ) return new IndexedColorsContext( *this );
        break;
        case BIFF12_ID_NUMFMTS:
            if( nRecId == BIFF12_ID_NUMFMT ) getStyles().importNumFmt( rStrm );
        break;
        case BIFF12_ID_FONTS:
            if( nRecId == BIFF12_ID_FONT ) getStyles().createFont()->importFont( rStrm );
        break;
        case BIFF12_ID_BORDERS:
            if( nRecId == BIFF12_ID_BORDER ) getStyles().createBorder()->importBorder( rStrm );
        break;
        case BIFF12_ID_FILLS:
            if( nRecId == BIFF12_ID_FILL ) getStyles().createFill()->importFill( rStrm );
        break;
        case BIFF12_ID_CELLXFS:
            if( nRecId == BIFF12_ID_XF ) getStyles().createCellXf()->importXf( rStrm, true );
        break;
        case BIFF12_ID_CELLSTYLEXFS:
            if( nRecId == BIFF12_ID_XF ) getStyles().createStyleXf()->importXf( rStrm, false );
        break;
        case BIFF12_ID_DXFS:
            if( nRecId == BIFF12_ID_DXF ) getStyles().createDxf()->importDxf( rStrm );
        break;
        case BIFF12_ID_CELLSTYLES:
            if( nRecId == BIFF12_ID_CELLSTYLE ) getStyles().importCellStyle( rStrm );
        break;
    }
    return nullptr;
}

const RecordInfo* StylesFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_BORDERS,        BIFF12_ID_BORDERS + 1       },
        { BIFF12_ID_CELLSTYLES,     BIFF12_ID_CELLSTYLES + 1    },
        { BIFF12_ID_CELLSTYLEXFS,   BIFF12_ID_CELLSTYLEXFS + 1  },
        { BIFF12_ID_CELLXFS,        BIFF12_ID_CELLXFS + 1       },
        { BIFF12_ID_COLORS,         BIFF12_ID_COLORS + 1        },
        { BIFF12_ID_DXFS,           BIFF12_ID_DXFS + 1          },
        { BIFF12_ID_FILLS,          BIFF12_ID_FILLS + 1         },
        { BIFF12_ID_FONTS,          BIFF12_ID_FONTS + 1         },
        { BIFF12_ID_INDEXEDCOLORS,  BIFF12_ID_INDEXEDCOLORS + 1 },
        { BIFF12_ID_MRUCOLORS,      BIFF12_ID_MRUCOLORS + 1     },
        { BIFF12_ID_NUMFMTS,        BIFF12_ID_NUMFMTS + 1       },
        { BIFF12_ID_STYLESHEET,     BIFF12_ID_STYLESHEET + 1    },
        { BIFF12_ID_TABLESTYLES,    BIFF12_ID_TABLESTYLES + 1   },
        { -1,                       -1                          }
    };
    return spRecInfos;
}

void StylesFragment::finalizeImport()
{
    getStyles().finalizeImport();
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
