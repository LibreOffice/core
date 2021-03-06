/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/weld.hxx>

namespace com :: sun :: star :: ucb { class XCommandEnvironment; }

using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;

class FolderTree
{
private:
    std::unique_ptr<weld::TreeView> m_xTreeView;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;
    weld::Window* m_pTopLevel;
    Reference< XCommandEnvironment > m_xEnv;
    ::osl::Mutex m_aMutex;
    Sequence< OUString > m_aDenyList;

    OUString m_sLastUpdatedDir;

    DECL_LINK(RequestingChildrenHdl, const weld::TreeIter&, bool);

public:
    FolderTree(std::unique_ptr<weld::TreeView> xTreeView, weld::Window* pTopLevel);

    void clear() { m_xTreeView->clear(); }

    void connect_changed(const Link<weld::TreeView&, void>& rLink) { m_xTreeView->connect_changed(rLink); }

    void InsertRootEntry(const OUString& rId, const OUString& rRootLabel);
    void FillTreeEntry(const weld::TreeIter& rEntry);
    void FillTreeEntry(const OUString & rUrl, const ::std::vector< std::pair< OUString, OUString > >& rFolders);
    void SetTreePath(OUString const & sUrl);
    void SetDenyList(const css::uno::Sequence< OUString >& rDenyList);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
