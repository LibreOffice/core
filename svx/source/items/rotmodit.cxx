/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <tools/stream.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/util/SortField.hpp>
#include <com/sun/star/util/SortFieldType.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include "svx/rotmodit.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

//-----------------------------------------------------------------------
//  SvxRotateModeItem - Ausrichtung bei gedrehtem Text
//-----------------------------------------------------------------------
IMPL_POOLITEM_FACTORY(SvxRotateModeItem)

SvxRotateModeItem::SvxRotateModeItem( SvxRotateMode eMode, sal_uInt16 _nWhich )
    : SfxEnumItem( _nWhich, (sal_uInt16)eMode )
{
}

SvxRotateModeItem::SvxRotateModeItem( const SvxRotateModeItem& rItem )
    : SfxEnumItem( rItem )
{
}

__EXPORT SvxRotateModeItem::~SvxRotateModeItem()
{
}

SfxPoolItem* __EXPORT SvxRotateModeItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStream >> nVal;
    return new SvxRotateModeItem( (SvxRotateMode) nVal,Which() );
}

SfxItemPresentation __EXPORT SvxRotateModeItem::GetPresentation(
                                SfxItemPresentation ePres,
                                SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
                                String& rText, const IntlWrapper * )  const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText.AppendAscii("...");
            rText.AppendAscii(": ");
//          break; // DURCHFALLEN!!!

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += UniString::CreateFromInt32( GetValue() );
            break;
        default: ;//prevent warning
    }

    return ePres;
}

String __EXPORT SvxRotateModeItem::GetValueText( sal_uInt16 nVal ) const
{
    String aText;

    switch ( nVal )
    {
        case SVX_ROTATE_MODE_STANDARD:
        case SVX_ROTATE_MODE_TOP:
        case SVX_ROTATE_MODE_CENTER:
        case SVX_ROTATE_MODE_BOTTOM:
            aText.AppendAscii("...");
            break;
        default:
            DBG_ERROR("SvxRotateModeItem: falscher enum");
            break;
    }
    return aText;
}

sal_uInt16 __EXPORT SvxRotateModeItem::GetValueCount() const
{
    return 4;       // STANDARD, TOP, CENTER, BOTTOM
}

SfxPoolItem* __EXPORT SvxRotateModeItem::Clone( SfxItemPool* ) const
{
    return new SvxRotateModeItem( *this );
}

sal_uInt16 __EXPORT SvxRotateModeItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return 0;
}

//  QueryValue/PutValue: Der ::com::sun::star::table::CellVertJustify enum wird mitbenutzt...

sal_Bool SvxRotateModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    table::CellVertJustify eUno = table::CellVertJustify_STANDARD;
    switch ( (SvxRotateMode)GetValue() )
    {
        case SVX_ROTATE_MODE_STANDARD: eUno = table::CellVertJustify_STANDARD; break;
        case SVX_ROTATE_MODE_TOP:      eUno = table::CellVertJustify_TOP;       break;
        case SVX_ROTATE_MODE_CENTER:   eUno = table::CellVertJustify_CENTER;    break;
        case SVX_ROTATE_MODE_BOTTOM:   eUno = table::CellVertJustify_BOTTOM;    break;
    }
    rVal <<= eUno;
    return sal_True;
}

sal_Bool SvxRotateModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    table::CellVertJustify eUno;
    if(!(rVal >>= eUno))
    {
        sal_Int32 nValue = 0;
        if(!(rVal >>= nValue))
            return sal_False;
        eUno = (table::CellVertJustify)nValue;
    }

    SvxRotateMode eSvx = SVX_ROTATE_MODE_STANDARD;
    switch (eUno)
    {
        case table::CellVertJustify_STANDARD: eSvx = SVX_ROTATE_MODE_STANDARD; break;
        case table::CellVertJustify_TOP:       eSvx = SVX_ROTATE_MODE_TOP;      break;
        case table::CellVertJustify_CENTER:   eSvx = SVX_ROTATE_MODE_CENTER;    break;
        case table::CellVertJustify_BOTTOM:   eSvx = SVX_ROTATE_MODE_BOTTOM;    break;
        default: ;//prevent warning
    }
    SetValue( (sal_uInt16)eSvx );
    return sal_True;
}



