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

#include "internationaloptions.hxx"
#include "internationaloptions.hrc"
#include <svl/eitem.hxx>
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>
#include <svx/dialogs.hrc>

namespace offapp
{

    struct InternationalOptionsPage::IMPL
    {
        FixedLine           m_aFL_DefaultTextDirection;
        RadioButton         m_aRB_TxtDirLeft2Right;
        RadioButton         m_aRB_TxtDirRight2Left;
        FixedLine           m_aFL_SheetView;
        CheckBox            m_aCB_ShtVwRight2Left;
        CheckBox            m_aCB_ShtVwCurrentDocOnly;

        sal_Bool                m_bEnable_SheetView_Opt : 1;

        inline              IMPL( Window* _pParent );

        inline void         EnableOption_SheetView( sal_Bool _bEnable = sal_True );
        void                ShowOption_SheetView( sal_Bool _bShow = sal_True );

        sal_Bool                FillItemSet( SfxItemSet& _rSet );
        void                Reset( const SfxItemSet& _rSet );
    };

    inline InternationalOptionsPage::IMPL::IMPL( Window* _pParent ) :
        m_aFL_DefaultTextDirection  ( _pParent, CUI_RES( FL_DEFTXTDIRECTION ) )
        ,m_aRB_TxtDirLeft2Right     ( _pParent, CUI_RES( RB_TXTDIR_LEFT2RIGHT ) )
        ,m_aRB_TxtDirRight2Left     ( _pParent, CUI_RES( RB_TXTDIR_RIGHT2LEFT ) )
        ,m_aFL_SheetView            ( _pParent, CUI_RES( FL_SHEETVIEW ) )
        ,m_aCB_ShtVwRight2Left      ( _pParent, CUI_RES( CB_SHTVW_RIGHT2LEFT ) )
        ,m_aCB_ShtVwCurrentDocOnly  ( _pParent, CUI_RES( CB_SHTVW_CURRENTDOCONLY ) )

        ,m_bEnable_SheetView_Opt    ( sal_False )
    {
        ShowOption_SheetView( m_bEnable_SheetView_Opt );
    }

    inline void InternationalOptionsPage::IMPL::EnableOption_SheetView( sal_Bool _bEnable )
    {
        if( m_bEnable_SheetView_Opt != _bEnable )
        {
            ShowOption_SheetView( _bEnable );

            m_bEnable_SheetView_Opt = _bEnable;
        }
    }

    void InternationalOptionsPage::IMPL::ShowOption_SheetView( sal_Bool _bShow )
    {
        m_aFL_SheetView.Show( _bShow );
        m_aCB_ShtVwRight2Left.Show( _bShow );
        m_aCB_ShtVwCurrentDocOnly.Show( _bShow );
    }

    sal_Bool InternationalOptionsPage::IMPL::FillItemSet( SfxItemSet& _rSet )
    {
        DBG_ASSERT( _rSet.GetPool(), "-InternationalOptionsPage::FillItemSet(): no pool gives rums!" );

        // handling of DefaultTextDirection stuff
        _rSet.Put(  SfxBoolItem(    _rSet.GetPool()->GetWhich( SID_ATTR_PARA_LEFT_TO_RIGHT ),
                                    m_aRB_TxtDirLeft2Right.IsChecked() ),
                    SID_ATTR_PARA_LEFT_TO_RIGHT );

        return sal_True;
    }

    void InternationalOptionsPage::IMPL::Reset( const SfxItemSet& _rSet )
    {
        // handling of DefaultTextDirection stuff
        const SfxBoolItem*  pLeft2RightItem = static_cast< const SfxBoolItem* >( GetItem( _rSet, SID_ATTR_PARA_LEFT_TO_RIGHT ) );

        DBG_ASSERT( pLeft2RightItem, "+InternationalOptionsPage::Reset(): SID_ATTR_PARA_LEFT_TO_RIGHT not set!" );

        sal_Bool                bLeft2Right = pLeft2RightItem? pLeft2RightItem->GetValue() : sal_True;
        m_aRB_TxtDirLeft2Right.Check( bLeft2Right );

    }

    InternationalOptionsPage::InternationalOptionsPage( Window* _pParent, const SfxItemSet& _rAttrSet ) :
        SfxTabPage  ( _pParent, CUI_RES( RID_OFA_TP_INTERNATIONAL ), _rAttrSet )

        ,m_pImpl    ( new IMPL( this ) )
    {
        FreeResource();
    }

    SfxTabPage* InternationalOptionsPage::CreateSd( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new InternationalOptionsPage( _pParent, _rAttrSet );
    }

    SfxTabPage* InternationalOptionsPage::CreateSc( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        InternationalOptionsPage*   p = new InternationalOptionsPage( _pParent, _rAttrSet );
        return p;
    }

    InternationalOptionsPage::~InternationalOptionsPage()
    {
        DELETEZ( m_pImpl );
    }

    sal_Bool InternationalOptionsPage::FillItemSet( SfxItemSet& _rSet )
    {
        return m_pImpl->FillItemSet( _rSet );
    }

    void InternationalOptionsPage::Reset( const SfxItemSet& _rSet )
    {
        m_pImpl->Reset( _rSet );
    }

}   // namespace offapp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
