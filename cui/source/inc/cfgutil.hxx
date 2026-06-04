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
#pragma once

#include <variant>
#include <vector>
#include <memory>
#include <string_view>

#include <rtl/ustring.hxx>

#include <com/sun/star/frame/DispatchInformation.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/weld/Button.hxx>
#include <vcl/weld/DialogController.hxx>
#include <vcl/weld/Frame.hxx>
#include <vcl/weld/Label.hxx>
#include <vcl/weld/TextView.hxx>
#include <vcl/weld/TreeView.hxx>
#include <vcl/weld/Window.hxx>

#include <config_features.h>

#if HAVE_FEATURE_SCRIPTING
class SfxMacroInfoItem;
#endif

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

        std::vector< SfxStyleInfo_Impl > getStyleFamilies() const;
        std::vector< SfxStyleInfo_Impl > getStyles(const OUString& sFamily);

        static OUString generateCommand(std::u16string_view sFamily, std::u16string_view sStyle);
};

struct SfxScriptContainer_Data
{
    SfxScriptContainer_Data(
        const css::uno::Reference<css::script::browse::XBrowseNode>& xBrowseNode_ = nullptr,
        const css::uno::Reference<css::frame::XModel>& xModel_ = nullptr)
        : xBrowseNode(xBrowseNode_)
        , xModel(xModel_)
    {
    }

    css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode;
    css::uno::Reference<css::frame::XModel> xModel;
};

struct SfxScriptFunction_Data
{
    SfxScriptFunction_Data(
        const OUString& sUrl_ = OUString(),
        const css::uno::Reference<css::frame::XModel> &xModel_ = nullptr)
        : sUrl(sUrl_)
        , xModel(xModel_)
    {
    }

    OUString sUrl;
    css::uno::Reference<css::frame::XModel> xModel;
};

enum class SfxCfgKind
{
    GROUP_FUNCTION,
    FUNCTION_SLOT,
    GROUP_SCRIPTCONTAINER,
    FUNCTION_SCRIPT,
    GROUP_STYLES,
    GROUP_ALLFUNCTIONS,
    GROUP_SIDEBARDECKS,
};

// Used to construct SfxGroupInfo_Data by selecting the variant as an index tag
template <SfxCfgKind K>
constexpr std::in_place_index_t<size_t(K)> SfxCfgKindAsIndex {};

// The types must match the order of the values of SfxCfgKind
typedef std::variant<
    std::monostate, // GROUP_FUNCTION
    std::monostate, // FUNCTION_SLOT
    SfxScriptContainer_Data, // GROUP_SCRIPTCONTAINER
    SfxScriptFunction_Data, // FUNCTION_SCRIPT
    std::unique_ptr<SfxStyleInfo_Impl>, // GROUP_STYLES
    std::monostate, // GROUP_ALLFUNCTIONS
    std::monostate // GROUP_SIDEBARDECKS
    > SfxGroupInfo_Data;

struct SfxGroupInfo_Impl
{
    SfxGroupInfo_Data aData;
    sal_uInt16  nUniqueID;
    OUString    sCommand;
    OUString    sLabel;
    OUString    sHelpText;
    OUString    sTooltip;

    SfxGroupInfo_Impl( SfxGroupInfo_Data ad, sal_uInt16 nr )
        : aData( std::move(ad) )
        , nUniqueID( nr )
    {
    }

    SfxCfgKind getKind() const { return static_cast<SfxCfgKind>(aData.index()); }
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
    void connect_changed(const Link<weld::ItemView&, void>& rLink)
    {
        m_xTreeView->connect_selection_changed(rLink);
    }
    void connect_command(const Link<const CommandEvent&, bool>& rLink) { m_xTreeView->connect_command(rLink); }
    void connect_item_activated(const Link<const weld::TreeIter&, bool>& rLink)
    {
        m_xTreeView->connect_item_activated(rLink);
    }
    void freeze() { m_xTreeView->freeze(); }
    void thaw() { m_xTreeView->thaw(); }
    void append(const OUString& rId, const OUString& rStr, const weld::TreeIter* pParent = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, false, nullptr);
    }
    std::unique_ptr<weld::TreeIter> tree_append(const OUString& rId, const OUString& rStr, const weld::TreeIter* pParent = nullptr)
    {
        std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, false, xIter.get());
        return xIter;
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage, const weld::TreeIter* pParent = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, false, m_xScratchIter.get());
        m_xTreeView->set_image(*m_xScratchIter, rImage);
    }
    void append(const OUString& rId, const OUString& rStr, const css::uno::Reference<css::graphic::XGraphic>& rImage, const weld::TreeIter* pParent = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, false, m_xScratchIter.get());
        m_xTreeView->set_image(*m_xScratchIter, rImage, -1);
    }
    void remove(int nPos) { m_xTreeView->remove(nPos); }
    void scroll_to_row(int pos) { m_xTreeView->scroll_to_row(pos); }
    void remove(const weld::TreeIter& rIter) { m_xTreeView->remove(rIter); }
    void expand_row(const weld::TreeIter& rIter) { m_xTreeView->expand_row(rIter); }
    int n_children() const { return m_xTreeView->n_children(); }
    bool iter_has_child(const weld::TreeIter& rIter) const { return m_xTreeView->iter_has_child(rIter); }
    OUString get_text(int nPos) const { return m_xTreeView->get_text(nPos); }
    OUString get_id(const weld::TreeIter& rIter) const { return m_xTreeView->get_id(rIter); }
    std::unique_ptr<weld::TreeIter> get_selected() const { return m_xTreeView->get_selected(); }
    OUString get_selected_text() const
    {
        if (std::unique_ptr<weld::TreeIter> pIter = m_xTreeView->get_selected())
            return m_xTreeView->get_text(*pIter);
        return OUString();

    }
    OUString get_selected_id() const
    {
        if (std::unique_ptr<weld::TreeIter> pIter = m_xTreeView->get_selected())
            return m_xTreeView->get_id(*pIter);
        return OUString();
    }
    void select(int pos) { m_xTreeView->select(pos); }
    void set_size_request(int nWidth, int nHeight) { m_xTreeView->set_size_request(nWidth, nHeight); }
    Size get_size_request() const { return m_xTreeView->get_size_request(); }
    weld::TreeView& get_widget() { return *m_xTreeView; }

    ~CuiConfigFunctionListBox();

    void          ClearAll();
    OUString      GetSelectedScriptURI() const;
    OUString      GetCommandHelpText();
    OUString      GetCurCommand() const;
    OUString      GetCurLabel() const;

    DECL_LINK(QueryTooltip, const weld::TreeIter& rIter, OUString);
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
    std::unique_ptr<weld::TreeIter> m_xScratchIter;

    sal_Int32 InitModule();
    void FillScriptList(const css::uno::Reference< css::script::browse::XBrowseNode >& xRootNode,
                        const css::uno::Reference< css::frame::XModel >& xParentDocumentModel,
                        const weld::TreeIter* pParentEntry);
    void FillFunctionsList(const css::uno::Sequence< css::frame::DispatchInformation >& xCommands);
    OUString MapCommand2UIName(const OUString& sCommand);

    DECL_LINK(ExpandingHdl, const weld::TreeIter&, bool);

public:
    CuiConfigGroupListBox(std::unique_ptr<weld::TreeView> xTreeView);
    void set_sensitive(bool bSensitive) { m_xTreeView->set_sensitive(bSensitive); }
    void connect_changed(const Link<weld::ItemView&, void>& rLink)
    {
        m_xTreeView->connect_selection_changed(rLink);
    }
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
#if HAVE_FEATURE_SCRIPTING
    void                SelectMacro(const SfxMacroInfoItem*);
#endif
    void                SetStylesInfo(SfxStylesInfo_Impl* pStyles);

    static OUString GetImage(
        const css::uno::Reference< css::script::browse::XBrowseNode >& node,
        css::uno::Reference< css::uno::XComponentContext > const & xCtx,
        bool bIsRootNode);
};

class SvxScriptSelectorDialog : public weld::GenericDialogController
{
    OUString                               m_sDefaultDesc;
    SfxStylesInfo_Impl                     m_aStylesInfo;

    std::unique_ptr<weld::Label> m_xDialogDescription;
    std::unique_ptr<CuiConfigGroupListBox> m_xCategories;
    std::unique_ptr<CuiConfigFunctionListBox> m_xCommands;
    std::unique_ptr<weld::Label> m_xLibraryFT;
    std::unique_ptr<weld::Label> m_xMacronameFT;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Button> m_xCancelButton;
    std::unique_ptr<weld::TextView> m_xDescriptionText;
    std::unique_ptr<weld::Frame> m_xDescriptionFrame;

    DECL_LINK(ClickHdl, weld::Button&, void);
    DECL_LINK(SelectHdl, weld::ItemView&, void);
    DECL_LINK(FunctionDoubleClickHdl, const weld::TreeIter&, bool);
    DECL_LINK(ContextMenuHdl, const CommandEvent&, bool);

    void                            UpdateUI();

public:
    SvxScriptSelectorDialog(weld::Window* pParent,
                            const css::uno::Reference< css::frame::XFrame >& xFrame);
    virtual ~SvxScriptSelectorDialog() override;

    OUString    GetScriptURL() const;
    css::uno::Reference<css::frame::XModel> GetScriptModel() const;
    void        SetRunLabel();
    void        SaveLastUsedMacro();
    void        LoadLastUsedMacro();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
