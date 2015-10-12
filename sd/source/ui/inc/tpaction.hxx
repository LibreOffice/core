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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TPACTION_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TPACTION_HXX

#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <svx/dlgctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include "sdtreelb.hxx"

#include <vector>

namespace sd {
    class View;
}
class SdDrawDocument;

/**
 * Effect-SingleTab-Dialog
 */
class SdActionDlg : public SfxSingleTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;
public:
    SdActionDlg(vcl::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView);
};

/**
 * Interaction-Tab-Page
 */
class SdTPAction : public SfxTabPage
{
private:
    VclPtr<ListBox>                m_pLbAction;

    VclPtr<FixedText>              m_pFtTree;                // jump destination controls
    VclPtr<SdPageObjsTLB>          m_pLbTree;
    VclPtr<SdPageObjsTLB>          m_pLbTreeDocument;
    VclPtr<ListBox>                m_pLbOLEAction;

    VclPtr<VclFrame>               m_pFrame;
    VclPtr<Edit>                   m_pEdtSound;
    VclPtr<Edit>                   m_pEdtBookmark;
    VclPtr<Edit>                   m_pEdtDocument;
    VclPtr<Edit>                   m_pEdtProgram;
    VclPtr<Edit>                   m_pEdtMacro;
    VclPtr<PushButton>             m_pBtnSearch;
    VclPtr<PushButton>             m_pBtnSeek;

    const ::sd::View*       mpView;
    SdDrawDocument*         mpDoc;
    XColorListRef           pColList;

    bool                    bTreeUpdated;
    std::vector<com::sun::star::presentation::ClickAction> maCurrentActions;
    OUString                aLastFile;
    ::std::vector< long >   aVerbVector;

    DECL_LINK_TYPED( ClickSearchHdl, Button*, void );
    DECL_LINK_TYPED( ClickActionHdl, ListBox&, void );
    DECL_LINK_TYPED( SelectTreeHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( CheckFileHdl, Control&, void );

    void                    UpdateTree();
    void                    OpenFileDialog();
    ::com::sun::star::presentation::ClickAction     GetActualClickAction();
    void                    SetActualClickAction( ::com::sun::star::presentation::ClickAction eCA );
    void                    SetEditText( OUString const & rStr );
    OUString                GetEditText( bool bURL = false );
    static sal_uInt16       GetClickActionSdResId( ::com::sun::star::presentation::ClickAction eCA );

public:
            SdTPAction( vcl::Window* pParent, const SfxItemSet& rInAttrs );
            virtual ~SdTPAction();
    virtual void dispose() override;

    static  VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet& );

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual sfxpg DeactivatePage( SfxItemSet* pSet ) override;

    void    Construct();

    void    SetView( const ::sd::View* pSdView );

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_TPACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
