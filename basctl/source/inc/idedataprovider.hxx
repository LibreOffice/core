/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basctl/idecodecompletiontypes.hxx>

#include <map>
#include <memory>
#include <vector>

#include <com/sun/star/uno/TypeClass.hpp>

namespace basctl
{
using IdeSymbolInfo = basctl::IdeSymbolInfo;
using SymbolInfoList = std::vector<std::shared_ptr<IdeSymbolInfo>>;
using GroupedSymbolInfoList = std::map<IdeSymbolKind, SymbolInfoList>;

enum class IdeBrowserScope
{
    ALL_LIBRARIES,
    CURRENT_DOCUMENT
};

// Data structure for the in-memory UNO type hierarchy
struct UnoApiHierarchy
{
    std::map<OUString, SymbolInfoList> m_hierarchyCache;
    void addNode(std::u16string_view sQualifiedName, css::uno::TypeClass eTypeClass);
    static IdeSymbolKind typeClassToSymbolKind(css::uno::TypeClass eTypeClass);
};

class IdeDataProviderInterface
{
public:
    virtual ~IdeDataProviderInterface() = default;

    virtual void Initialize() = 0;
    virtual bool IsInitialized() const = 0;
    virtual void Reset() = 0;

    // Data Access Methods
    virtual SymbolInfoList GetTopLevelNodes() = 0;
    virtual GroupedSymbolInfoList GetMembers(const IdeSymbolInfo& rNode) = 0;
    virtual SymbolInfoList GetChildNodes(const IdeSymbolInfo& rParent) = 0;
    // Scope Management
    virtual void SetScope(IdeBrowserScope eScope, const OUString& rCurrentDocumentURL) = 0;
};

class IdeDataProvider : public IdeDataProviderInterface
{
public:
    IdeDataProvider();

    void Initialize() override;
    bool IsInitialized() const override { return m_bInitialized; }
    void Reset() override;
    void AddDocumentNodesWithModules();
    void RefreshDocumentNodes();

    SymbolInfoList GetTopLevelNodes() override;
    GroupedSymbolInfoList GetMembers(const IdeSymbolInfo& rNode) override;
    SymbolInfoList GetChildNodes(const IdeSymbolInfo& rParent) override;
    void SetScope(IdeBrowserScope eScope, const OUString& rCurrentDocumentURL) override;

private:
    void performFullUnoScan();
    void addDocumentNodes();

    // Core data
    std::unique_ptr<UnoApiHierarchy> m_pUnoHierarchy;
    SymbolInfoList m_aAllTopLevelNodes;

    // State management
    bool m_bInitialized = false;
    IdeBrowserScope m_eCurrentScope = IdeBrowserScope::ALL_LIBRARIES;
    OUString m_sCurrentDocumentURL;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
