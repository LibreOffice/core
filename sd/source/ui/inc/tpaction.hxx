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
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
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
class SdActionDlg : public SfxSingleTabDialogController
{
private:
    const SfxItemSet&   rOutAttrs;
public:
    SdActionDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View const * pView);
};

/**
 * Interaction-Tab-Page
 */
class SdTPAction : public SfxTabPage
{
private:
    const ::sd::View*       mpView;
    SdDrawDocument*         mpDoc;
    XColorListRef           pColList;

    bool                    bTreeUpdated;
    std::vector<css::presentation::ClickAction> maCurrentActions;
    OUString                aLastFile;
    ::std::vector< long >   aVerbVector;

    std::unique_ptr<weld::ComboBox> m_xLbAction;
    std::unique_ptr<weld::Label> m_xFtTree;                // jump destination controls
    std::unique_ptr<SdPageObjsTLV> m_xLbTree;
    std::unique_ptr<SdPageObjsTLV> m_xLbTreeDocument;
    std::unique_ptr<weld::TreeView> m_xLbOLEAction;
    std::unique_ptr<weld::Frame> m_xFrame;
    std::unique_ptr<weld::Entry> m_xEdtSound;
    std::unique_ptr<weld::Entry> m_xEdtBookmark;
    std::unique_ptr<weld::Entry> m_xEdtDocument;
    std::unique_ptr<weld::Entry> m_xEdtProgram;
    std::unique_ptr<weld::Entry> m_xEdtMacro;
    std::unique_ptr<weld::Button> m_xBtnSearch;
    std::unique_ptr<weld::Button> m_xBtnSeek;

    DECL_LINK( ClickSearchHdl, weld::Button&, void );
    DECL_LINK( ClickActionHdl, weld::ComboBox&, void );
    DECL_LINK( SelectTreeHdl, weld::TreeView&, void );
    DECL_LINK( CheckFileHdl, weld::Widget&, void );

    void                    UpdateTree();
    void                    OpenFileDialog();
    css::presentation::ClickAction     GetActualClickAction();
    void                    SetActualClickAction( css::presentation::ClickAction eCA );
    void                    SetEditText( OUString const & rStr );
    OUString                GetEditText( bool bURL = false );
    static const char*      GetClickActionSdResId(css::presentation::ClickAction eCA);

public:
    SdTPAction(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SdTPAction() override;

    static  VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet& );

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    Construct();

    void    SetView( const ::sd::View* pSdView );

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_TPACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
