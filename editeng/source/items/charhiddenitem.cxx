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
#include "precompiled_editeng.hxx"


#include <editeng/charhiddenitem.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>

/*-- 16.12.2003 15:24:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxCharHiddenItem::SvxCharHiddenItem( const sal_Bool bHidden, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bHidden )
{
}
/*-- 16.12.2003 15:24:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxPoolItem* SvxCharHiddenItem::Clone( SfxItemPool * ) const
{
    return new SvxCharHiddenItem( *this );
}
/*-- 16.12.2003 15:24:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPresentation SvxCharHiddenItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_CHARHIDDEN_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_CHARHIDDEN_TRUE;
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

