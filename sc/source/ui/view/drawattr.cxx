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
#include "precompiled_sc.hxx"


#include "drawattr.hxx"
#include "global.hxx"

//------------------------------------------------------------------------

String __EXPORT SvxDrawToolItem::GetValueText() const
{
    return GetValueText(GetValue());
}

//------------------------------------------------------------------------

String __EXPORT SvxDrawToolItem::GetValueText( sal_uInt16 nVal ) const
{
    const sal_Char* p;

    switch (nVal)
    {
        case 0  : p = "SVX_SNAP_DRAW_SELECT"    ; break;
        //
        case 1  : p = "SVX_SNAP_DRAW_LINE"      ; break;
        case 2  : p = "SVX_SNAP_DRAW_RECT"      ; break;
        case 3  : p = "SVX_SNAP_DRAW_ELLIPSE"   ; break;
        case 4  : p = "SVX_SNAP_DRAW_POLYGON"   ; break;
        case 5  : p = "SVX_SNAP_DRAW_ARC"       ; break;
        case 6  : p = "SVX_SNAP_DRAW_PIE"       ; break;
        case 7  : p = "SVX_SNAP_DRAW_CIRCLECUT" ; break;
        case 8  : p = "SVX_SNAP_DRAW_TEXT"      ; break;
        default : return EMPTY_STRING;
    }
    return String::CreateFromAscii( p );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxDrawToolItem::Clone( SfxItemPool * ) const
{
    return new SvxDrawToolItem(*this);
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT SvxDrawToolItem::Create( SvStream& rStream, sal_uInt16 nVer ) const
{
    sal_uInt16 nVal;
    rStream >> nVal;
    return new SvxDrawToolItem(nVal);
}



