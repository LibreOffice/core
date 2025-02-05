/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <vector>
#include <basctl/scriptdocument.hxx>
#include <svx/passwd.hxx>
#include <svl/lstner.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/DispatchInformation.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

struct ScriptContainerInfo
{
    css::script::browse::XBrowseNode* pBrowseNode;

    ScriptContainerInfo(css::script::browse::XBrowseNode* pObj)
        : pBrowseNode(pObj)
    {
    }
};

struct ScriptInfo
{
    css::script::browse::XBrowseNode* pBrowseNode;
    OUString sURL;
    OUString sDescription;

    ScriptInfo(css::script::browse::XBrowseNode* pObj, const OUString& rsUrl,
               const OUString& rsDesc)
        : pBrowseNode(pObj)
        , sURL(rsUrl)
        , sDescription(rsDesc)
    {
    }
};

typedef std::vector<std::unique_ptr<ScriptInfo>> ScriptInfoArr;

// inspired by class CuiConfigFunctionListBox
// cui/source/inc/cfgutil.hxx
class ScriptsListBox
{
    friend class ScriptContainersListBox; // for access to aArr

    ScriptInfoArr aArr;

    std::unique_ptr<weld::TreeView> m_xTreeView;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;

    DECL_LINK(QueryTooltip, const weld::TreeIter& rIter, OUString);

public:
    ScriptsListBox(std::unique_ptr<weld::TreeView> xTreeView);
    ~ScriptsListBox();

    void ClearAll();
    static OUString GetDescriptionText(const OUString& rId);
    OUString GetSelectedScriptName();

    void connect_changed(const Link<weld::TreeView&, void>& rLink)
    {
        m_xTreeView->connect_selection_changed(rLink);
    }
    void connect_popup_menu(const Link<const CommandEvent&, bool>& rLink)
    {
        m_xTreeView->connect_popup_menu(rLink);
    }
    void connect_row_activated(const Link<weld::TreeView&, bool>& rLink)
    {
        m_xTreeView->connect_row_activated(rLink);
    }
    void freeze() { m_xTreeView->freeze(); }
    void thaw() { m_xTreeView->thaw(); }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage,
                const weld::TreeIter* pParent = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, false,
                            m_xScratchIter.get());
        m_xTreeView->set_image(*m_xScratchIter, rImage);
    }
    int n_children() const { return m_xTreeView->n_children(); }
    std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig = nullptr) const
    {
        return m_xTreeView->make_iterator(pOrig);
    }
    OUString get_id(const weld::TreeIter& rIter) const { return m_xTreeView->get_id(rIter); }
    bool get_selected(weld::TreeIter* pIter) const { return m_xTreeView->get_selected(pIter); }
    OUString get_selected_id() const
    {
        if (!m_xTreeView->get_selected(m_xScratchIter.get()))
            return OUString();
        return m_xTreeView->get_id(*m_xScratchIter);
    }
    void select(int pos) { m_xTreeView->select(pos); }
    weld::TreeView& get_widget() { return *m_xTreeView; }

    void Remove(const weld::TreeIter& rEntry);
};

enum class ScriptContainerType
{
    LOCATION,
    LANGUAGE,
    LIBRARY,
    MODULEORDIALOG
};

class MacroManagerDialog;

// locations, libraries, modules, and dialogs
class ScriptContainersListBox
{
    friend class MacroManagerDialog;

    ScriptsListBox* m_pScriptsListBox;

    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::frame::XFrame> m_xFrame;

    std::unique_ptr<weld::TreeView> m_xTreeView;

    void Remove(const weld::TreeIter* pEntryIter, bool bRemoveEntryIter);
    void Fill(const weld::TreeIter* pEntryIter);
    basctl::ScriptDocument GetScriptDocument(const weld::TreeIter* pIter = nullptr);

    DECL_LINK(ExpandingHdl, const weld::TreeIter&, bool);
    DECL_LINK(QueryTooltip, const weld::TreeIter& rIter, OUString);

    // for weld::WaitObject which seems not to always behave as expected without the
    // dialog window as a parent
    MacroManagerDialog* m_pMacroManagerDialog;

public:
    ScriptContainersListBox(std::unique_ptr<weld::TreeView> xTreeView,
                            MacroManagerDialog* pMacroManagerDialog);
    ~ScriptContainersListBox();

    void connect_changed(const Link<weld::TreeView&, void>& rLink)
    {
        m_xTreeView->connect_selection_changed(rLink);
    }
    weld::TreeView& get_widget() { return *m_xTreeView; }
    void ClearAll();

    void Init(const css::uno::Reference<css::uno::XComponentContext>& xContext,
              const css::uno::Reference<css::frame::XFrame>& xFrame);
    void SetScriptsListBox(ScriptsListBox* pBox) { m_pScriptsListBox = pBox; }
    void ScriptContainerSelected();

    void Insert(const css::uno::Reference<css::script::browse::XBrowseNode>& xInsertNode,
                const weld::TreeIter* pParentEntry, const OUString& rsUiName,
                const OUString& rsImage, bool bChildOnDemand = false, int nPos = -1,
                weld::TreeIter* pRet = nullptr);

    OUString GetContainerName(const weld::TreeIter& rIter,
                              const ScriptContainerType eScriptContainerType);
    OUString GetSelectedEntryContainerName(ScriptContainerType eScriptContainerType);
};

enum class InputDialogMode;

class MacroManagerDialog : public weld::GenericDialogController, public SfxListener
{
    OUString m_aScriptsListBoxLabelBaseStr;

    // For forwarding to Assign dialog
    css::uno::Reference<css::frame::XFrame> m_xDocumentFrame;

    std::unique_ptr<ScriptContainersListBox> m_xScriptContainersListBox;
    std::unique_ptr<ScriptsListBox> m_xScriptsListBox;
    std::unique_ptr<weld::Label> m_xScriptsListBoxLabel;
    std::unique_ptr<weld::Button> m_xRunButton;
    std::unique_ptr<weld::Button> m_xCloseButton;
    std::unique_ptr<weld::TextView> m_xDescriptionText;
    std::unique_ptr<weld::Frame> m_xDescriptionFrame;
    std::unique_ptr<weld::Button> m_xNewLibraryButton;
    std::unique_ptr<weld::Button> m_xNewModuleButton;
    std::unique_ptr<weld::Button> m_xNewDialogButton;
    std::unique_ptr<weld::Button> m_xLibraryModuleDialogEditButton;
    std::unique_ptr<weld::Button> m_xLibraryModuleDialogRenameButton;
    std::unique_ptr<weld::Button> m_xLibraryModuleDialogDeleteButton;
    std::unique_ptr<weld::Button> m_xLibraryPasswordButton;
    std::unique_ptr<weld::Button> m_xLibraryImportButton;
    std::unique_ptr<weld::Button> m_xLibraryExportButton;
    std::unique_ptr<weld::Button> m_xMacroEditButton;
    std::unique_ptr<weld::Button> m_xMacroDeleteButton;
    std::unique_ptr<weld::Button> m_xMacroCreateButton;
    std::unique_ptr<weld::Button> m_xMacroRenameButton;
    std::unique_ptr<weld::Button> m_xAssignButton;

    DECL_LINK(ClickHdl, weld::Button&, void);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(FunctionDoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(ContextMenuHdl, const CommandEvent&, bool);
    DECL_LINK(CheckPasswordHdl, SvxPasswordDialog*, bool);

    void BasicScriptsCreateLibrary(const basctl::ScriptDocument& rDocument);
    void BasicScriptsCreateModule(const basctl::ScriptDocument& rDocument);
    void BasicScriptsCreateDialog(const basctl::ScriptDocument& rDocument);
    void BasicScriptsLibraryModuleDialogEdit(const basctl::ScriptDocument& rDocument);
    void BasicScriptsLibraryModuleDialogRename(const basctl::ScriptDocument& rDocument);
    void BasicScriptsLibraryModuleDialogDelete(const basctl::ScriptDocument& rDocument);
    void BasicScriptsLibraryPassword(const basctl::ScriptDocument& rDocument);
    void BasicScriptsMacroEdit(const basctl::ScriptDocument& rDocument);
    bool IsLibraryReadOnlyOrFailedPasswordQuery(const basctl::ScriptDocument& rDocument,
                                                weld::TreeIter* pIter);

    void ScriptingFrameworkScriptsCreateEntry(InputDialogMode eInputDialogMode);
    void ScriptingFrameworkScriptsRenameEntry(weld::TreeView& rTreeView,
                                              const weld::TreeIter& rEntry);
    void ScriptingFrameworkScriptsDeleteEntry(weld::TreeView& rTreeView,
                                              const weld::TreeIter& rEntry);
    static bool getBoolProperty(css::uno::Reference<css::beans::XPropertySet> const& xProps,
                                OUString const& propName);
    OUString getListOfChildren(const css::uno::Reference<css::script::browse::XBrowseNode>& node,
                               int depth);

    css::uno::Reference<css::script::browse::XBrowseNode>
    getBrowseNode(const weld::TreeView& rTreeView, const weld::TreeIter& rTreeIter);

    void UpdateUI();
    void CheckButtons();

    virtual void Notify(SfxBroadcaster&, const SfxHint& rHint) override;

public:
    MacroManagerDialog(weld::Window* pParent,
                       const css::uno::Reference<css::frame::XFrame>& xFrame);
    virtual ~MacroManagerDialog() override;

    OUString GetScriptURL() const;

    void SaveLastUsedMacro();
    void LoadLastUsedMacro();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
