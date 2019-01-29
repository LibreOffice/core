/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_FOLDERTREE_HXX
#define INCLUDED_SVTOOLS_FOLDERTREE_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <svtools/svtdllapi.h>

#include <tools/wintypes.hxx>
#include <vcl/treelistbox.hxx>

namespace com :: sun :: star :: ucb { class XCommandEnvironment; }

class SvTreeListEntry;

using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::svt;

class SVT_DLLPUBLIC FolderTree : public SvTreeListBox
{
private:
    Reference< XCommandEnvironment > m_xEnv;
    ::osl::Mutex m_aMutex;
    Sequence< OUString > m_aBlackList;

    OUString m_sLastUpdatedDir;

public:
    FolderTree( vcl::Window* pParent, WinBits nBits );

    virtual void RequestingChildren( SvTreeListEntry* pEntry ) override;

    void FillTreeEntry( SvTreeListEntry* pEntry );
    void FillTreeEntry( const OUString & rUrl, const ::std::vector< std::pair< OUString, OUString > >& rFolders );
    void SetTreePath( OUString const & sUrl );
    void SetBlackList( const css::uno::Sequence< OUString >& rBlackList );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
