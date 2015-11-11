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

#include <com/sun/star/uno/Sequence.hxx>
#include <tools/stream.hxx>
#include "cfgchart.hxx"
#include <dialmgr.hxx>
#include <cuires.hrc>

#define ROW_COLOR_COUNT 12

using namespace com::sun::star;


SvxChartColorTable::SvxChartColorTable()
    : nNextElementNumber(0)
{
}

SvxChartColorTable::SvxChartColorTable(const SvxChartColorTable & _rSource)
    : m_aColorEntries(_rSource.m_aColorEntries)
    , nNextElementNumber(m_aColorEntries.size() + 1)
{
}

// accessors
size_t SvxChartColorTable::size() const
{
    return m_aColorEntries.size();
}

const XColorEntry & SvxChartColorTable::operator[]( size_t _nIndex ) const
{
    if ( _nIndex >= m_aColorEntries.size() )
    {
        SAL_WARN( "cui.options", "SvxChartColorTable::[] invalid index" );
        return m_aColorEntries[ 0 ];
    }

    return m_aColorEntries[ _nIndex ];
}

ColorData SvxChartColorTable::getColorData( size_t _nIndex ) const
{
    if ( _nIndex >= m_aColorEntries.size() )
    {
        SAL_WARN( "cui.options", "SvxChartColorTable::getColorData invalid index" );
        return COL_BLACK;
    }

    return m_aColorEntries[ _nIndex ].GetColor().GetRGBColor();
}

// mutators
void SvxChartColorTable::clear()
{
    m_aColorEntries.clear();
    nNextElementNumber = 1;
}

void SvxChartColorTable::append( const XColorEntry & _rEntry )
{
    m_aColorEntries.push_back( _rEntry );
}

void SvxChartColorTable::remove( size_t _nIndex )
{
    if (m_aColorEntries.size() > 0)
        m_aColorEntries.erase( m_aColorEntries.begin() + _nIndex);

    for (size_t i=0 ; i<m_aColorEntries.size(); i++)
    {
        m_aColorEntries[ i ].SetName( getDefaultName( i ) );
    }
}

void SvxChartColorTable::replace( size_t _nIndex, const XColorEntry & _rEntry )
{
    DBG_ASSERT( _nIndex <= m_aColorEntries.size(),
                "SvxChartColorTable::replace invalid index" );

    m_aColorEntries[ _nIndex ] = _rEntry;
}

void SvxChartColorTable::useDefault()
{
    ColorData aColors[] = {
        RGB_COLORDATA( 0x00, 0x45, 0x86 ),
        RGB_COLORDATA( 0xff, 0x42, 0x0e ),
        RGB_COLORDATA( 0xff, 0xd3, 0x20 ),
        RGB_COLORDATA( 0x57, 0x9d, 0x1c ),
        RGB_COLORDATA( 0x7e, 0x00, 0x21 ),
        RGB_COLORDATA( 0x83, 0xca, 0xff ),
        RGB_COLORDATA( 0x31, 0x40, 0x04 ),
        RGB_COLORDATA( 0xae, 0xcf, 0x00 ),
        RGB_COLORDATA( 0x4b, 0x1f, 0x6f ),
        RGB_COLORDATA( 0xff, 0x95, 0x0e ),
        RGB_COLORDATA( 0xc5, 0x00, 0x0b ),
        RGB_COLORDATA( 0x00, 0x84, 0xd1 )
    };

    clear();

    for( sal_Int32 i=0; i<ROW_COLOR_COUNT; i++ )
    {
        append( XColorEntry( aColors[ i % sizeof( aColors ) ], getDefaultName( i ) ));
    }
}

OUString SvxChartColorTable::getDefaultName( size_t _nIndex )
{
    OUString aName;

    if (sDefaultNamePrefix.getLength() == 0)
    {
        OUString aResName( CUI_RES( RID_SVXSTR_DIAGRAM_ROW ) );
        sal_Int32 nPos = aResName.indexOf( "$(ROW)" );
        if( nPos != -1 )
        {
            sDefaultNamePrefix = aResName.copy( 0, nPos );
            sDefaultNamePostfix = aResName.copy( nPos + sizeof( "$(ROW)" ) - 1 );
        }
        else
        {
            sDefaultNamePrefix = aResName;
        }
    }

    aName = sDefaultNamePrefix + OUString::number(_nIndex + 1) + sDefaultNamePostfix;
    nNextElementNumber++;

    return aName;
}

// comparison
bool SvxChartColorTable::operator==( const SvxChartColorTable & _rOther ) const
{
    // note: XColorEntry has no operator ==
    bool bEqual = ( this->m_aColorEntries.size() == _rOther.m_aColorEntries.size() );

    if( bEqual )
    {
        for( size_t i = 0; i < m_aColorEntries.size(); ++i )
        {
            if( getColorData( i ) != _rOther.getColorData( i ))
            {
                bEqual = false;
                break;
            }
        }
    }

    return bEqual;
}


// class SvxChartOptions


SvxChartOptions::SvxChartOptions() :
    ::utl::ConfigItem( OUString("Office.Chart") ),
    mbIsInitialized( false )
{
    maPropertyNames.realloc( 1 );
    maPropertyNames[ 0 ] = "DefaultColor/Series";
}

SvxChartOptions::~SvxChartOptions()
{
}

const SvxChartColorTable& SvxChartOptions::GetDefaultColors()
{
    if ( !mbIsInitialized )
        mbIsInitialized = RetrieveOptions();
    return maDefColors;
}

void SvxChartOptions::SetDefaultColors( const SvxChartColorTable& aCol )
{
    maDefColors = aCol;
    SetModified();
}

bool SvxChartOptions::RetrieveOptions()
{
    // get sequence containing all properties

    uno::Sequence< OUString > aNames = GetPropertyNames();
    uno::Sequence< uno::Any > aProperties( aNames.getLength());
    aProperties = GetProperties( aNames );

    if( aProperties.getLength() == aNames.getLength())
    {
        // 1. default colors for series
        maDefColors.clear();
        uno::Sequence< sal_Int64 > aColorSeq;
        aProperties[ 0 ] >>= aColorSeq;

        sal_Int32 nCount = aColorSeq.getLength();
        Color aCol;

        // create strings for entry names
        OUString aResName( CUI_RES( RID_SVXSTR_DIAGRAM_ROW ) );
        OUString aPrefix, aPostfix, aName;
        sal_Int32 nPos = aResName.indexOf( "$(ROW)" );
        if( nPos != -1 )
        {
            aPrefix = aResName.copy( 0, nPos );
            sal_Int32 idx = nPos + sizeof( "$(ROW)" ) - 1;
            aPostfix = aResName.copy( idx, aResName.getLength()-idx );
        }
        else
            aPrefix = aResName;

        // set color values
        for( sal_Int32 i=0; i < nCount; i++ )
        {
            aCol.SetColor( (static_cast< ColorData >(aColorSeq[ i ] )));

            aName = aPrefix + OUString::number(i + 1) + aPostfix;

            maDefColors.append( XColorEntry( aCol, aName ));
        }
        return true;
    }
    return false;
}

void SvxChartOptions::ImplCommit()
{
    uno::Sequence< OUString > aNames = GetPropertyNames();
    uno::Sequence< uno::Any > aValues( aNames.getLength());

    if( aValues.getLength() >= 1 )
    {
        // 1. default colors for series
        // convert list to sequence
        const size_t nCount = maDefColors.size();
        uno::Sequence< sal_Int64 > aColors( nCount );
        for( size_t i=0; i < nCount; i++ )
        {
            ColorData aData = maDefColors.getColorData( i );
            aColors[ i ] = aData;
        }

        aValues[ 0 ] <<= aColors;
    }

    PutProperties( aNames, aValues );
}

void SvxChartOptions::Notify( const css::uno::Sequence< OUString >& )
{
}


// class SvxChartColorTableItem


SvxChartColorTableItem::SvxChartColorTableItem( sal_uInt16 nWhich_, const SvxChartColorTable& aTable ) :
    SfxPoolItem( nWhich_ ),
    m_aColorTable( aTable )
{
}

SvxChartColorTableItem::SvxChartColorTableItem( const SvxChartColorTableItem& rOther ) :
    SfxPoolItem( rOther ),
    m_aColorTable( rOther.m_aColorTable )
{
}

SfxPoolItem* SvxChartColorTableItem::Clone( SfxItemPool * ) const
{
    return new SvxChartColorTableItem( *this );
}

bool SvxChartColorTableItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "SvxChartColorTableItem::operator== : types differ" );

    const SvxChartColorTableItem * rCTItem( dynamic_cast<const SvxChartColorTableItem * >( & rAttr ));
    if( rCTItem )
    {
        return (this->m_aColorTable == rCTItem->GetColorList());
    }

    return false;
}

void SvxChartColorTableItem::SetOptions( SvxChartOptions* pOpts ) const
{
    if ( pOpts )
        pOpts->SetDefaultColors( m_aColorTable );
}




void SvxChartColorTableItem::ReplaceColorByIndex( size_t _nIndex, const XColorEntry & _rEntry )
{
    m_aColorTable.replace( _nIndex, _rEntry );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
