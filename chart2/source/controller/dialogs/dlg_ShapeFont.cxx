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
#include "precompiled_chart2.hxx"

#include "dlg_ShapeFont.hxx"
#include "ViewElementListProvider.hxx"
#include "ResId.hxx"
#include "ResourceIds.hrc"

#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <editeng/flstitem.hxx>

using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

ShapeFontDialog::ShapeFontDialog( Window* pParent, const SfxItemSet* pAttr,
    const ViewElementListProvider* pViewElementListProvider )
    :SfxTabDialog( pParent, SchResId( DLG_SHAPE_FONT ), pAttr )
    ,m_pViewElementListProvider( pViewElementListProvider )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
    AddTabPage( RID_SVXPAGE_CHAR_POSITION );
}

ShapeFontDialog::~ShapeFontDialog()
{
}

void ShapeFontDialog::PageCreated( sal_uInt16 nId, SfxTabPage& rPage )
{
    SfxAllItemSet aSet( *( GetInputSetImpl()->GetPool() ) );
    switch ( nId )
    {
        case RID_SVXPAGE_CHAR_NAME:
            {
                aSet.Put( SvxFontListItem( m_pViewElementListProvider->getFontList(), SID_ATTR_CHAR_FONTLIST ) );
                rPage.PageCreated( aSet );
            }
            break;
        case RID_SVXPAGE_CHAR_EFFECTS:
            {
                aSet.Put( SfxUInt16Item( SID_DISABLE_CTL, DISABLE_CASEMAP ) );
                rPage.PageCreated( aSet );
            }
            break;
        default:
            {
            }
            break;
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
