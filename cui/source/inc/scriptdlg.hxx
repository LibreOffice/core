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

#ifndef INCLUDED_CUI_SOURCE_INC_SCRIPTDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_SCRIPTDLG_HXX

#include <memory>
#include <vcl/treelistbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/weld.hxx>
#include <sfx2/basedlgs.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <unordered_map>

#define OBJTYPE_METHOD          2L
#define OBJTYPE_SCRIPTCONTAINER 3L
#define OBJTYPE_SFROOT          4L

typedef std::unordered_map < OUString, OUString > Selection_hash;

class SFEntry;

enum class InputDialogMode {
    NEWLIB        = 1,
    NEWMACRO      = 2,
    RENAME        = 3,
};

class CuiInputDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdit;
public:
    CuiInputDialog(weld::Window * pParent, InputDialogMode nMode);
    OUString GetObjectName() const { return m_xEdit->get_text(); }
    void SetObjectName(const OUString& rName)
    {
        m_xEdit->set_text(rName);
        m_xEdit->select_region(0, -1);
    }
};

class SFEntry final
{
private:
    bool            loaded;
    css::uno::Reference< css::script::browse::XBrowseNode > nodes;
    css::uno::Reference< css::frame::XModel > model;
public:
                    SFEntry( const css::uno::Reference< css::script::browse::XBrowseNode >& entryNodes ,
                             const css::uno::Reference< css::frame::XModel >& entryModel) { nodes = entryNodes; loaded=false; model = entryModel; }
                    SFEntry( const SFEntry& r ) { nodes = r.nodes; loaded = r.loaded; }
    const css::uno::Reference< css::script::browse::XBrowseNode >& GetNode() { return nodes ;}
    const css::uno::Reference< css::frame::XModel >& GetModel() { return model ;};
    bool            isLoaded() const                    { return loaded; }
    void            setLoaded()                         { loaded=true; }
};

class SvxScriptOrgDialog : public SfxDialogController
{
protected:
    OUString         m_sLanguage;
    static Selection_hash   m_lastSelection;
    const OUString m_delErrStr;
    const OUString m_delErrTitleStr;
    const OUString m_delQueryStr;
    const OUString m_delQueryTitleStr;
    const OUString m_createErrStr;
    const OUString m_createDupStr;
    const OUString m_createErrTitleStr;
    const OUString m_renameErrStr;
    const OUString m_renameErrTitleStr;
    const OUString m_sMyMacros;
    const OUString m_sProdMacros;

    std::unique_ptr<weld::TreeView> m_xScriptsBox;
    std::unique_ptr<weld::Button> m_xRunButton;
    std::unique_ptr<weld::Button> m_xCloseButton;
    std::unique_ptr<weld::Button> m_xCreateButton;
    std::unique_ptr<weld::Button> m_xEditButton;
    std::unique_ptr<weld::Button> m_xRenameButton;
    std::unique_ptr<weld::Button> m_xDelButton;

    DECL_LINK( ScriptSelectHdl, weld::TreeView&, void );
    DECL_LINK( ExpandingHdl, const weld::TreeIter&, bool );
    DECL_LINK( ButtonHdl, weld::Button&, void );
    static bool         getBoolProperty( css::uno::Reference< css::beans::XPropertySet > const & xProps, OUString const & propName );
    void                CheckButtons(  css::uno::Reference< css::script::browse::XBrowseNode > const & node );

    void        createEntry(weld::TreeIter& rEntry);
    void        renameEntry(weld::TreeIter& rEntry);
    void        deleteEntry(weld::TreeIter& rEntry);
    css::uno::Reference<css::script::browse::XBrowseNode> getBrowseNode(const weld::TreeIter& rEntry);
    css::uno::Reference<css::frame::XModel> getModel(const weld::TreeIter& rEntry);
    OUString    getListOfChildren( const css::uno::Reference< css::script::browse::XBrowseNode >& node, int depth );
    void        StoreCurrentSelection();
    void        RestorePreviousSelection();

    void        Init(const OUString& language);
    void        delUserData(weld::TreeIter& rIter);
    void        deleteTree(weld::TreeIter& rIter);
    void        deleteAllTree();
    void        insertEntry(OUString const & rText, OUString const & rBitmap,
                              const weld::TreeIter* pParent,
                              bool bChildrenOnDemand,
                              std::unique_ptr< SFEntry > && aUserData,
                              const OUString& factoryURL, bool bSelect);
    void        insertEntry(OUString const & rText, OUString const & rBitmap,
                              const weld::TreeIter* pParent,
                              bool bChildrenOnDemand,
                              std::unique_ptr< SFEntry > && aUserData,
                              bool bSelect);

    void        RequestSubEntries(const weld::TreeIter& rRootEntry,
                                  css::uno::Reference< css::script::browse::XBrowseNode > const & node,
                                  css::uno::Reference< css::frame::XModel>& model);

    static css::uno::Reference< css::script::browse::XBrowseNode >
        getLangNodeFromRootNode( css::uno::Reference< css::script::browse::XBrowseNode > const & root, OUString const & language );

    static css::uno::Reference< css::uno::XInterface  > getDocumentModel( css::uno::Reference< css::uno::XComponentContext > const & xCtx, OUString const & docName );

public:
    // prob need another arg in the ctor
    // to specify the language or provider
    SvxScriptOrgDialog(weld::Window* pParent, const OUString& language);
    virtual ~SvxScriptOrgDialog() override;

    virtual short run() override;
};

class SvxScriptErrorDialog : public VclAbstractDialog
{
private:

    OUString m_sMessage;

    DECL_STATIC_LINK( SvxScriptErrorDialog, ShowDialog, void*, void );

public:

    SvxScriptErrorDialog( css::uno::Any const & aException );

    virtual ~SvxScriptErrorDialog() override;

    short           Execute() override;
};

#endif // INCLUDED_CUI_SOURCE_INC_SCRIPTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
