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
#ifndef INCLUDED_CUI_SOURCE_INC_CFGUTIL_HXX
#define INCLUDED_CUI_SOURCE_INC_CFGUTIL_HXX

#include <vector>
#include <memory>
#include <rtl/ustring.hxx>

#include <com/sun/star/frame/DispatchInformation.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <vcl/timer.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/image.hxx>
#include <vcl/weld.hxx>

class Button;
class SaveInData;
class SfxMacroInfoItem;

struct SfxStyleInfo_Impl
{
    OUString sFamily;
    OUString sStyle;
    OUString sCommand;
    OUString sLabel;

    SfxStyleInfo_Impl()
    {}

    SfxStyleInfo_Impl(const SfxStyleInfo_Impl& rCopy)
    {
        sFamily  = rCopy.sFamily;
        sStyle   = rCopy.sStyle;
        sCommand = rCopy.sCommand;
        sLabel   = rCopy.sLabel;
    }
};

struct SfxStylesInfo_Impl
{
private:
        OUString m_aModuleName;
        css::uno::Reference< css::frame::XModel > m_xDoc;

public:

        SfxStylesInfo_Impl();
        void init(const OUString& rModuleName, const css::uno::Reference< css::frame::XModel >& xModel);

        static bool parseStyleCommand(SfxStyleInfo_Impl& aStyle);
        void getLabel4Style(SfxStyleInfo_Impl& aStyle);

        std::vector< SfxStyleInfo_Impl > getStyleFamilies();
        std::vector< SfxStyleInfo_Impl > getStyles(const OUString& sFamily);

        static OUString generateCommand(const OUString& sFamily, const OUString& sStyle);
};

enum class SfxCfgKind
{
    GROUP_FUNCTION           = 1,
    FUNCTION_SLOT            = 2,
    GROUP_SCRIPTCONTAINER    = 3,
    FUNCTION_SCRIPT          = 4,
    GROUP_STYLES             = 5,
    GROUP_ALLFUNCTIONS       = 6
};

struct SfxGroupInfo_Impl
{
    SfxCfgKind  nKind;
    sal_uInt16  nUniqueID;
    void*       pObject;
    OUString    sCommand;
    OUString    sLabel;
    OUString    sHelpText;

                SfxGroupInfo_Impl( SfxCfgKind n, sal_uInt16 nr, void* pObj = nullptr ) :
                    nKind( n ), nUniqueID( nr ), pObject( pObj ) {}
};

typedef std::vector<std::unique_ptr<SfxGroupInfo_Impl> > SfxGroupInfoArr_Impl;

class CuiConfigFunctionListBox
{
    friend class CuiConfigGroupListBox;
    SfxGroupInfoArr_Impl aArr;

    std::unique_ptr<weld::TreeView> m_xTreeView;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;

public:
    CuiConfigFunctionListBox(std::unique_ptr<weld::TreeView> xTreeView);
    void set_sensitive(bool bSensitive) { m_xTreeView->set_sensitive(bSensitive); }
    void connect_changed(const Link<weld::TreeView&, void>& rLink) { m_xTreeView->connect_changed(rLink); }
    void connect_row_activated(const Link<weld::TreeView&, void>& rLink) { m_xTreeView->connect_row_activated(rLink); }
    void freeze() { m_xTreeView->freeze(); }
    void thaw() { m_xTreeView->thaw(); }
    void append(const OUString& rId, const OUString& rStr, weld::TreeIter* pParent = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, nullptr, false, nullptr);
    }
    std::unique_ptr<weld::TreeIter> tree_append(const OUString& rId, const OUString& rStr, weld::TreeIter* pParent = nullptr)
    {
        std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, nullptr, false, xIter.get());
        return xIter;
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage, weld::TreeIter* pParent = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, &rImage, false, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, const css::uno::Reference<css::graphic::XGraphic>& rImage, weld::TreeIter* pParent = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, nullptr, false, m_xScratchIter.get());
        m_xTreeView->set_image(*m_xScratchIter, rImage, -1);
    }
    void remove(int nPos) { m_xTreeView->remove(nPos); }
    void scroll_to_row(int pos) { m_xTreeView->scroll_to_row(pos); }
    void remove(weld::TreeIter& rIter) { m_xTreeView->remove(rIter); }
    void expand_row(weld::TreeIter& rIter) { m_xTreeView->expand_row(rIter); }
    int n_children() const { return m_xTreeView->n_children(); }
    std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig = nullptr) const { return m_xTreeView->make_iterator(pOrig); }
    bool get_iter_first(weld::TreeIter& rIter) const { return m_xTreeView->get_iter_first(rIter); }
    // set iter to point to next node, depth first, then sibling
    bool iter_next(weld::TreeIter& rIter) const { return m_xTreeView->iter_next(rIter); }
    bool iter_next_sibling(weld::TreeIter& rIter) const { return m_xTreeView->iter_next_sibling(rIter); }
    bool iter_has_child(const weld::TreeIter& rIter) const { return m_xTreeView->iter_has_child(rIter); }
    OUString get_text(const weld::TreeIter& rIter) const { return m_xTreeView->get_text(rIter); }
    OUString get_text(int nPos) const { return m_xTreeView->get_text(nPos); }
    OUString get_id(const weld::TreeIter& rIter) const { return m_xTreeView->get_id(rIter); }
    OUString get_id(int nPos) const { return m_xTreeView->get_id(nPos); }
    bool get_selected(weld::TreeIter* pIter) const { return m_xTreeView->get_selected(pIter); }
    OUString get_selected_text() const
    {
        if (!m_xTreeView->get_selected(m_xScratchIter.get()))
            return OUString();
        return m_xTreeView->get_text(*m_xScratchIter);
    }
    OUString get_selected_id() const
    {
        if (!m_xTreeView->get_selected(m_xScratchIter.get()))
            return OUString();
        return m_xTreeView->get_id(*m_xScratchIter);
    }
    int get_selected_index() const { return m_xTreeView->get_selected_index(); }
    void select(const weld::TreeIter& rIter) { m_xTreeView->select(rIter); }
    void select(int pos) { m_xTreeView->select(pos); }
    void set_size_request(int nWidth, int nHeight) { m_xTreeView->set_size_request(nWidth, nHeight); }
    Size get_size_request() const { return m_xTreeView->get_size_request(); }
    weld::TreeView& get_widget() { return *m_xTreeView; }

    ~CuiConfigFunctionListBox();

    void          ClearAll();
    OUString      GetSelectedScriptURI();
    OUString      GetHelpText( bool bConsiderParent = true );
    OUString      GetCurCommand();
    OUString      GetCurLabel();
};

struct SvxConfigGroupBoxResource_Impl;
class CuiConfigGroupListBox
{
    std::unique_ptr<SvxConfigGroupBoxResource_Impl> xImp;
    CuiConfigFunctionListBox* m_pFunctionListBox;
    SfxGroupInfoArr_Impl aArr;
    OUString m_sModuleLongName;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::container::XNameAccess > m_xGlobalCategoryInfo;
    css::uno::Reference< css::container::XNameAccess > m_xModuleCategoryInfo;
    css::uno::Reference< css::container::XNameAccess > m_xUICmdDescription;
    SfxStylesInfo_Impl* m_pStylesInfo;
    std::unique_ptr<weld::TreeView> m_xTreeView;

    static OUString GetImage(
        const css::uno::Reference< css::script::browse::XBrowseNode >& node,
        css::uno::Reference< css::uno::XComponentContext > const & xCtx,
        bool bIsRootNode);

    static css::uno::Reference< css::uno::XInterface  > getDocumentModel(
        css::uno::Reference< css::uno::XComponentContext > const & xCtx,
        OUString const & docName);

    void InitModule();
    void FillScriptList(const css::uno::Reference< css::script::browse::XBrowseNode >& xRootNode,
                        const weld::TreeIter* pParentEntry, bool bCheapChildrenOnDemand);
    void FillFunctionsList(const css::uno::Sequence< css::frame::DispatchInformation >& xCommands);
    OUString MapCommand2UIName(const OUString& sCommand);

    DECL_LINK(ExpandingHdl, const weld::TreeIter&, bool);
    DECL_LINK(OpenCurrentHdl, weld::TreeView&, void);

public:
    CuiConfigGroupListBox(std::unique_ptr<weld::TreeView> xTreeView);
    void set_sensitive(bool bSensitive) { m_xTreeView->set_sensitive(bSensitive); }
    void connect_changed(const Link<weld::TreeView&, void>& rLink) { m_xTreeView->connect_changed(rLink); }
    void set_size_request(int nWidth, int nHeight) { m_xTreeView->set_size_request(nWidth, nHeight); }
    weld::TreeView& get_widget() { return *m_xTreeView; }
    ~CuiConfigGroupListBox();
    void                ClearAll();

    void                Init(const css::uno::Reference< css::uno::XComponentContext >& xContext,
                             const css::uno::Reference< css::frame::XFrame >&          xFrame,
                             const OUString&                                        sModuleLongName,
                             bool bEventMode);
    void                SetFunctionListBox( CuiConfigFunctionListBox *pBox )
                        { m_pFunctionListBox = pBox; }
    void                GroupSelected();
    void                SelectMacro(const SfxMacroInfoItem*);
    void                SelectMacro(const OUString&, const OUString&);
    void                SetStylesInfo(SfxStylesInfo_Impl* pStyles);
};

class SvxScriptSelectorDialog : public weld::GenericDialogController
{
    OUString                               m_sDefaultDesc;
    SfxStylesInfo_Impl                     m_aStylesInfo;
    bool                                   m_bShowSlots;

    std::unique_ptr<weld::Label> m_xDialogDescription;
    std::unique_ptr<CuiConfigGroupListBox> m_xCategories;
    std::unique_ptr<CuiConfigFunctionListBox> m_xCommands;
    std::unique_ptr<weld::Label> m_xLibraryFT;
    std::unique_ptr<weld::Label> m_xCategoryFT;
    std::unique_ptr<weld::Label> m_xMacronameFT;
    std::unique_ptr<weld::Label> m_xCommandsFT;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Button> m_xCancelButton;
    std::unique_ptr<weld::TextView> m_xDescriptionText;

    DECL_LINK(ClickHdl, weld::Button&, void);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(FunctionDoubleClickHdl, weld::TreeView&, void);

    void                            UpdateUI();

public:
    SvxScriptSelectorDialog(weld::Window* pParent, bool bShowSlots,
                            const css::uno::Reference< css::frame::XFrame >& xFrame);
    virtual ~SvxScriptSelectorDialog() override;

    OUString    GetScriptURL() const;
    void        SetRunLabel();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
