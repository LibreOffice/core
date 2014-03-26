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
    SdActionDlg(Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView);
};

/**
 * Interaction-Tab-Page
 */
class SdTPAction : public SfxTabPage
{
private:
    ListBox*                m_pLbAction;

    FixedText*              m_pFtTree;                // jump destination controls
    SdPageObjsTLB*          m_pLbTree;
    SdPageObjsTLB*          m_pLbTreeDocument;
    ListBox*                m_pLbOLEAction;

    VclFrame*               m_pFrame;
    Edit*                   m_pEdtSound;
    Edit*                   m_pEdtBookmark;
    Edit*                   m_pEdtDocument;
    Edit*                   m_pEdtProgram;
    Edit*                   m_pEdtMacro;
    PushButton*             m_pBtnSearch;
    PushButton*             m_pBtnSeek;

    const SfxItemSet&       rOutAttrs;
    const ::sd::View*       mpView;
    SdDrawDocument*         mpDoc;
    XColorListRef           pColList;

    sal_Bool                    bTreeUpdated;
    std::vector<com::sun::star::presentation::ClickAction> maCurrentActions;
    OUString                aLastFile;
    ::std::vector< long >   aVerbVector;



    DECL_LINK( ClickSearchHdl, void * );
    DECL_LINK( ClickActionHdl, void * );
    DECL_LINK( SelectTreeHdl, void * );
    DECL_LINK( CheckFileHdl, void * );

    void                    UpdateTree();
    virtual void            OpenFileDialog();
    ::com::sun::star::presentation::ClickAction     GetActualClickAction();
    void                    SetActualClickAction( ::com::sun::star::presentation::ClickAction eCA );
    void                    SetActualAnimationEffect( ::com::sun::star::presentation::AnimationEffect eAE );
    void                    SetEditText( OUString const & rStr );
    OUString                GetEditText( sal_Bool bURL = sal_False );
    sal_uInt16                  GetClickActionSdResId( ::com::sun::star::presentation::ClickAction eCA );
    sal_uInt16                  GetAnimationEffectSdResId( ::com::sun::star::presentation::AnimationEffect eAE );

public:
            SdTPAction( Window* pParent, const SfxItemSet& rInAttrs );
            ~SdTPAction();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );

    virtual bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    Construct();

    void    SetView( const ::sd::View* pSdView );

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_TPACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
