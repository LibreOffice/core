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
#include "fmtextcontroldialogs.hxx"
#include <svx/dialmgr.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

#ifndef _SVX_CHARDLG_HXX
//#include "chardlg.hxx"
#endif
#ifndef _SVX_PARAGRPH_HXX
//#include "paragrph.hxx"
#endif
#include <editeng/eeitem.hxx>
#ifndef _SVX_TABSTPGE_HXX
//#include "tabstpge.hxx"
#endif

#include "svx/flagsdef.hxx"
#include <svl/intitem.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <svl/cjkoptions.hxx>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= TextControlCharAttribDialog
    //====================================================================
    //--------------------------------------------------------------------
    TextControlCharAttribDialog::TextControlCharAttribDialog( Window* pParent, const SfxItemSet& _rCoreSet, const SvxFontListItem& _rFontList )
        :SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_TEXTCONTROL_CHARATTR ), &_rCoreSet )
        ,m_aFontList( _rFontList )
    {
        FreeResource();

        AddTabPage( RID_SVXPAGE_CHAR_NAME);
        AddTabPage( RID_SVXPAGE_CHAR_EFFECTS);
        AddTabPage( RID_SVXPAGE_CHAR_POSITION);
    }

    //--------------------------------------------------------------------
    TextControlCharAttribDialog::~TextControlCharAttribDialog()
    {
    }

    //--------------------------------------------------------------------
    void TextControlCharAttribDialog::PageCreated( sal_uInt16 _nId, SfxTabPage& _rPage )
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

        switch( _nId )
        {
            case RID_SVXPAGE_CHAR_NAME:
                aSet.Put (m_aFontList);
                _rPage.PageCreated(aSet);
                break;

            case RID_SVXPAGE_CHAR_EFFECTS:
                aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
                _rPage.PageCreated(aSet);
                break;

            case RID_SVXPAGE_CHAR_POSITION:
                aSet.Put( SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER) );
                _rPage.PageCreated(aSet);
                break;
        }
    }

    //====================================================================
    //= TextControlParaAttribDialog
    //====================================================================
    //--------------------------------------------------------------------
    TextControlParaAttribDialog::TextControlParaAttribDialog( Window* _pParent, const SfxItemSet& _rCoreSet )
        :SfxTabDialog( _pParent, SVX_RES( RID_SVXDLG_TEXTCONTROL_PARAATTR ), &_rCoreSet )
    {
        FreeResource();

        AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );
        AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );

        SvtCJKOptions aCJKOptions;
        if( aCJKOptions.IsAsianTypographyEnabled() )
            AddTabPage( RID_SVXPAGE_PARA_ASIAN );
        else
            RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

        AddTabPage( RID_SVXPAGE_TABULATOR );
    }

    //--------------------------------------------------------------------
    TextControlParaAttribDialog::~TextControlParaAttribDialog()
    {
    }

//........................................................................
}   // namespace svx
//........................................................................

